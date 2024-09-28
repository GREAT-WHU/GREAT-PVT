/**
 * @file         ifcb.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        decode and enconde ifcb file
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gcoders/ifcb.h"

using namespace std;
namespace great
{

    t_ifcb::t_ifcb(t_gsetbase *s, string version, int sz)
        : t_gcoder(s, version, sz)
    {
    }

    t_ifcb::~t_ifcb()
    {
    }

    int t_ifcb::decode_head(char *buff, int sz, vector<string> &errmsg)
    {

        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };

        _mutex.unlock();
        return -1;
    }

    int t_ifcb::decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg)
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
        t_ifcbrec ifcb;
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
                else if (tmp.substr(0, 1) == "x" || tmp.substr(0, 1) == "X")
                {
                    //cerr << "warning: data can not use :" << tmp << endl;
                }
                else if (tmp.substr(0, 1) == " ")
                {
                    istr >> prn >> ifcb.value >> ifcb.sigma >> ifcb.npoint;
                    //fill data loop
                    map<string, t_gdata *>::iterator it = _data.begin();
                    while (it != _data.end())
                    {
                        if (it->second->id_type() == t_gdata::IFCB)
                            ((t_gifcb *)it->second)->add_sat_ifcb(_epoch, prn, ifcb);
                        it++;
                    }
                }
                else if (tmp.substr(0, 3) == "EOF")
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_WARN(_spdlog, "t_ifcb::decode_data End of file {}", tmp);
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return -1;
                }
                else
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_WARN(_spdlog, "t_ifcb::decode_data unknown ifcb-data message {}", tmp);
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
                SPDLOG_LOGGER_ERROR(_spdlog, "t_ifcb::decode_data throw exception");
            return -1;
        }
        _mutex.unlock();
        return consume;
    }


} //namespace