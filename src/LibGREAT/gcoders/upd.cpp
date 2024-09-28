/**
 * @file         upd.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        decode and enconde upd file
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gcoders/upd.h"

using namespace std;
namespace great
{

    t_upd::t_upd(t_gsetbase *s, string version, int sz)
        : t_gcoder(s, version, sz), _updtype(UPDTYPE::NONE)
    {

        _epoch = FIRST_TIME;
        _updtype = UPDTYPE::NONE;
    }

    t_upd::~t_upd()
    {
    }

    int t_upd::decode_head(char *buff, int sz, vector<string> &errmsg)
    {
        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };
        string tmp;
        int tmpsize = 0;
        int consume = 0;
        string mode_tmp;
        try
        {
            while ((tmpsize = t_gcoder::_getline(tmp, 0)) >= 0)
            {
                consume += tmpsize;
                if (tmp.substr(0, 1) == "%")
                {
                    if (tmp.substr(2, 1) != "U")  //for upd in new format
                    {
                        t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    int pos = tmp.find("_");
                    tmp.erase(0, pos + 1);
                    tmp = cut_crlf(tmp);  // remove "\n" or "\r" at the end of line (WindowsMac/Linux)
                    mode_tmp = trim(tmp); // remove "\t" at the end of line

                    if (mode_tmp == "EWL" || mode_tmp == "ewl" || mode_tmp == "WL" || mode_tmp == "wl" ||
                        mode_tmp == "NL" || mode_tmp == "nl" || mode_tmp == "EWL_epoch" || mode_tmp == "ewl_epoch" || mode_tmp == "RTPPP_SAVE" ||
                        mode_tmp == "EWL24" || mode_tmp == "ewl24" || mode_tmp == "EWL25" || mode_tmp == "ewl25")
                    {
                        if (mode_tmp == "EWL" || mode_tmp == "ewl")
                        {
                            _epoch = t_gtime(EWL_IDENTIFY);
                            _updtype = UPDTYPE::EWL;
                        }
                        else if (mode_tmp == "EWL24" || mode_tmp == "ewl24")
                        {
                            _epoch = t_gtime(EWL24_IDENTIFY);
                            _updtype = UPDTYPE::EWL24;
                        }
                        else if (mode_tmp == "EWL25" || mode_tmp == "ewl25")
                        {
                            _epoch = t_gtime(EWL25_IDENTIFY);
                            _updtype = UPDTYPE::EWL25;
                        }
                        else if (mode_tmp == "WL" || mode_tmp == "wl")
                        {
                            _epoch = t_gtime(WL_IDENTIFY);
                            _updtype = UPDTYPE::WL;
                        }
                        else if (mode_tmp == "NL" || mode_tmp == "nl")
                        {
                            _updtype = UPDTYPE::NL;
                        }
                        

                        t_gcoder::_consume(tmpsize);
                        break;
                    }
                    else
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_ERROR(_spdlog, "ERROR: unknown upd mode");
                        t_gcoder::_consume(tmpsize);
                        _mutex.unlock();
                        return -1;
                    }
                }
                else
                {
                    _mutex.unlock();
                    return -1;
                }
            }
        }
        catch (...)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR: unknown mistake");
            return -1;
        }
        _mutex.unlock();
        return consume;
    }

    int t_upd::decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {
        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };
        string tmp;
        int tmpsize = 0;
        int consume = 0;
        string str; // no use

        int mjd;
        double sod;

        string prn;
        t_updrec upd;
        try
        {
            while ((tmpsize = t_gcoder::_getline(tmp, 0)) >= 0)
            {
                istringstream istr(tmp);
                consume += tmpsize;
                if (tmp.substr(1, 10) == "EPOCH-TIME")
                {
                    istr >> str >> mjd >> sod;
                    _epoch.from_mjd(mjd, sod, sod - floor(sod));
                }
                else if ((tmp.substr(0, 1) == "x" || tmp.substr(0, 1) == "X"))
                {
                    istr >> prn >> upd.value >> upd.sigma >> upd.npoint;
                    upd.sigma = 10000.0;
                    //fill data loop
                    map<string, t_gdata *>::iterator it = _data.begin();
                    while (it != _data.end())
                    {
                        if (it->second->id_type() == t_gdata::UPD)
                            dynamic_cast<t_gupd *>(it->second)->add_sat_upd(_updtype, _epoch, prn.substr(1), upd);
                        ++it;
                    }
                }
                else if (tmp.substr(0, 1) == " ")
                {
                    istr >> prn >> upd.value >> upd.sigma >> upd.npoint;
                    //fill data loop
                    map<string, t_gdata *>::iterator it = _data.begin();
                    while (it != _data.end())
                    {
                        if (it->second->id_type() == t_gdata::UPD)
                            dynamic_cast<t_gupd *>(it->second)->add_sat_upd(_updtype, _epoch, prn, upd);
                        ++it;
                    }
                }
                else if (tmp.substr(0, 3) == "EOF")
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "WARNING: End of file" + tmp);
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return -1;
                }
                else
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_ERROR(_spdlog, "WARNING: unknown upd-data message" + tmp);
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return -1;
                }
                t_gcoder::_consume(tmpsize);
            }
        }
        catch (...)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR: unknown mistake");
            return -1;
        }
        _mutex.unlock();
        return consume;
    }


} //namespace