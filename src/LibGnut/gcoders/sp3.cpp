/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include "gall/gallprec.h"
#include "gcoders/sp3.h"
#include "gutils/gtriple.h"
#include "gutils/gtypeconv.h"

#include "spdlog/spdlog.h"
using namespace std;

namespace gnut
{

    t_sp3::t_sp3(t_gsetbase *s, string version, int sz)
        : t_gcoder(s, version, sz)
    {

        _start.tsys(t_gtime::GPS);
        _lastepo.tsys(t_gtime::GPS);
        _nrecord = -1;
        _nrecmax = -1;
    }

    int t_sp3::decode_head(char *buff, int sz, vector<string> &errmsg)
    {

        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };

        string tmp;
        int consume = 0;
        int tmpsize = 0;

        while ((tmpsize = t_gcoder::_getline(tmp)) >= 0)
        {

            consume += tmpsize;

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Current Line is : {}", tmp);
            // first information
            if (tmp.substr(0, 1) == "#")
            {
                // first line
                if (tmp.substr(1, 1) == "a" || // 60-columns
                    tmp.substr(1, 1) == "c"    // 80-columns
                )
                {

                    _version = tmp.substr(1, 1);
                    _start.from_str("%Y %m %d  %H %M %S", tmp.substr(3, 30));
                    _nepochs = str2int(tmp.substr(32, 7));
                    _orbrefs = tmp.substr(46, 5);
                    _orbtype = tmp.substr(52, 3);
                    if (tmp.size() > 60)
                    {
                        _agency = tmp.substr(56, 4);
                    }
                    else
                    {
                        _agency = "";
                    }
                    // second line
                }
                else if (tmp.substr(1, 1) == "#")
                {
                    _orbintv = (long)str2dbl(tmp.substr(24, 14)); // [sec]
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "start time = {}", _start.str(" %Y-%m-%d %H:%M:%S"));
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "refs       = {}", _orbrefs);
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "type       = {}", _orbtype);
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "intv       = {}", _orbintv);
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "nepo       = {}", _nepochs);
                }
                else
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_WARN(_spdlog, "unknown record");
                }
            }
            else if (tmp.substr(0, 2) == "+ ")
            {
                ostringstream ltmp("reading satellites:");
                for (unsigned int i = 9; i < 60 && i + 3 < tmp.size(); i = i + 3)
                {
                    string ssat = tmp.substr(i, 3);

                    if (trim(ssat) == "00")
                    {
                        continue;
                    }
                    if (ssat.substr(0, 2) == "  ")
                    {
                        ssat = "G0" + ssat.substr(2, 1);
                    }
                    else if (ssat.substr(0, 1) == " ")
                    {
                        ssat = "G" + ssat.substr(1, 2);
                    }
                    _prn.push_back(ssat);
                    ltmp << " " << tmp.substr(i, 3);
                }
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "decoder for {} ", ltmp.str());
            }
            else if (tmp.substr(0, 2) == "++")
            {
                ostringstream ltmp("reading accuracies:");
                for (unsigned int i = 9; i < 60 && i + 3 < tmp.size(); i = i + 3)
                {
                    _acc.push_back(str2int(tmp.substr(i, 3)));
                    ltmp << " " << tmp.substr(i, 3);
                }
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "decoder for {} ", ltmp.str());
            }
            else if (tmp.substr(0, 2) == "%c")
            {
                _timesys.push_back(tmp.substr(3, 2));
                _timesys.push_back(tmp.substr(6, 2));
                _timesys.push_back(tmp.substr(9, 3));
                _timesys.push_back(tmp.substr(13, 3));
                _timesys.push_back(tmp.substr(17, 4));
                _timesys.push_back(tmp.substr(22, 4));
                _timesys.push_back(tmp.substr(27, 4));
                _timesys.push_back(tmp.substr(32, 4));
                _timesys.push_back(tmp.substr(37, 5));
                _timesys.push_back(tmp.substr(43, 5));
                _timesys.push_back(tmp.substr(49, 5));
                _timesys.push_back(tmp.substr(55, 5));
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "reading satellite systems");
            }
            else if (tmp.substr(0, 2) == "%f")
            {
                _accbase.push_back(str2int(tmp.substr(3, 9)));
                _accbase.push_back(str2int(tmp.substr(14, 9)));
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "reading PV base");
            }
            else if (tmp.substr(0, 2) == "%i")
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "additional info");
            }
            else if (tmp.substr(0, 2) == "/*")
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "comments");
            }
            else if (tmp.substr(0, 2) == "* ")
            { // END OF HEADER !!!
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "END OF HEADER");
                _mutex.unlock();
                return -1;
            }
            else
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "warning: unknown header message");
            }

            t_gcoder::_consume(tmpsize);
        }

        _mutex.unlock();
        return consume;
    }

    int t_sp3::decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {

        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };

        string tmp;
        int tmpsize = 0;
        while ((tmpsize = t_gcoder::_getline(tmp, 0)) >= 0)
        {
            // EPOCH record
            if (tmp.substr(0, 1) == "*" ||
                tmp.substr(0, 3) == "EOF")
            {

                if (tmp.substr(0, 3) == "EOF")
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "EOF found");
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return tmpsize;
                }

                if (_nrecord > 0 && _nrecord != _nrecmax)
                {

                    if (_spdlog)
                        SPDLOG_LOGGER_WARN(_spdlog, "not equal number of satellites _nrecord is {}, _nrecmax is {}!", _nrecord, _nrecmax);
                }

                char dummy;
                int yr, mn, dd, hr, mi;
                double sc;
                stringstream ss(tmp);
                ss >> dummy >> yr >> mn >> dd >> hr >> mi >> sc;

                if (ss.fail())
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_CRITICAL(_spdlog, "incorrect SP3 epoch record : {}!", ss.str().c_str());
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return -1;
                }

                int sod = hr * 3600 + mi * 60 + (int)sc;
                _lastepo.from_ymd(yr, mn, dd, sod);

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "reading EPOCH [{}] - {} ", _nrecord, _lastepo.str(" %Y-%m-%d %H:%M:%S"));
                _nrecord = -1;
            }

            // POSITION reccord
            if (tmp.substr(0, 1) == "P")
            { 

                t_gtriple xyz(0.0, 0.0, 0.0);
                t_gtriple dxyz(0.0, 0.0, 0.0);
                double t = 0.0, dt = 0.9;

                char sat[3 + 1];
                sat[3] = '\0';
                char flg;
                double pos[4] = {0.0, 0.0, 0.0, 0.0};
                stringstream ss(tmp);

                ss >> noskipws >> flg >> sat[0] >> sat[1] >> sat[2] >> skipws >> pos[0] >> pos[1] >> pos[2] >> pos[3];
                string prn;
                prn = t_gsys::eval_sat(string(sat));

                for (int i = 0; i < 3; i++)
                    if (pos[i] == 0.0)
                        xyz[i] = UNDEFVAL_POS; // gephprec
                    else
                        xyz[i] = pos[i] * 1000; // km -> m

                if (pos[3] > 999999)
                    t = UNDEFVAL_CLK; // gephprec
                else
                    t = pos[3] / 1000000; // us -> s
                // fill single data record
                if (!_filter_gnss(prn))
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "skip satellite : {} for you do not set at the xml file", prn);
                }
                else
                {
                    map<string, t_gdata*>::iterator it = _data.begin();
                    while (it != _data.end())
                    {
                        if (it->second->id_type() == t_gdata::ALLPREC)
                        {
                            ((t_gallprec*)it->second)->addpos(prn, _lastepo, xyz, t, dxyz, dt);
                            ((t_gallprec*)it->second)->add_interval(prn, _orbintv);
                            ((t_gallprec*)it->second)->add_agency(_agency);
                        }

                        it++;
                }
            }

                if (ss.fail())
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_ERROR(_spdlog, "incorrect SP3 data record: {}", ss.str());
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return -1;
                }
                cnt++;
            }

            // VELOCITY reccord
            if (tmp.substr(0, 1) == "V")
            { // and epoch_defined ){

                char sat[3 + 1];
                sat[3] = '\0';
                char flg;
                double vel[4] = {0.0, 0.0, 0.0, 0.0};
                double var[4] = {0.0, 0.0, 0.0, 0.0};

                stringstream ss(tmp);
                ss >> noskipws >> flg >> sat[0] >> sat[1] >> sat[2] >> skipws >> vel[0] >> vel[1] >> vel[2] >> vel[3];

                string prn;
                prn = t_gsys::eval_sat(string(sat));

                // fill single data record
                map<string, t_gdata *>::iterator it = _data.begin();
                while (it != _data.end())
                {
                    if (!_filter_gnss(prn))
                    {

                        it++;
                    }
                    else
                    {
                        if (it->second->id_type() == t_gdata::ALLPREC)
                        {
                            ((t_gallprec*)it->second)->addvel(prn, _lastepo, vel, var);
                        }
                        it++;
                        //     }
                    }

                    // fill single data record
                }

                if (ss.fail())
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_ERROR(_spdlog, "incorrect SP3 data record: {}", ss.str());
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return -1;
                }
            }

            _nrecord++;

            if (_nrecord > _nrecmax)
                _nrecmax = _nrecord;

            t_gcoder::_consume(tmpsize);
        }
        _mutex.unlock();
        return tmpsize;
    }


} // namespace
