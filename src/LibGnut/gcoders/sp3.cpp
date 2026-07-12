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
#include <string>

#include "gall/gallprec.h"
#include "gcoders/sp3.h"
#include "gutils/gtriple.h"
#include "gutils/gtypeconv.h"

#include "spdlog/spdlog.h"
using namespace std;

namespace gnut
{

    t_sp3::t_sp3(t_gsetbase* s, string version, int sz) :
        t_gcoder(s, version, sz)
    {
        _start.tsys(t_gtime::GPS);
        _lastepo.tsys(t_gtime::GPS);
        _nrecord = -1;
        _nrecmax = -1;
    }

    int t_sp3::decode_head(char* buff, int sz, vector<string>& errmsg)
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

            GREAT_DEBUG(std::string("Current Line is : ") + tmp);
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
                    GREAT_DEBUG(std::string("start time = ") + _start.str(" %Y-%m-%d %H:%M:%S"));
                    GREAT_DEBUG(std::string("refs       = ") + _orbrefs);
                    GREAT_DEBUG(std::string("type       = ") + _orbtype);
                    GREAT_DEBUG(std::string("intv       = ") + std::to_string(_orbintv));
                    GREAT_DEBUG(std::string("nepo       = ") + std::to_string(_nepochs));
                }
                else
                {
                    GREAT_WARN("unknown record");
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
                GREAT_DEBUG(std::string("decoder for ") + ltmp.str() + " ");
            }
            else if (tmp.substr(0, 2) == "++")
            {
                ostringstream ltmp("reading accuracies:");
                for (unsigned int i = 9; i < 60 && i + 3 < tmp.size(); i = i + 3)
                {
                    _acc.push_back(str2int(tmp.substr(i, 3)));
                    ltmp << " " << tmp.substr(i, 3);
                }
                GREAT_DEBUG(std::string("decoder for ") + ltmp.str() + " ");
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
                GREAT_DEBUG("reading satellite systems");
            }
            else if (tmp.substr(0, 2) == "%f")
            {
                _accbase.push_back(str2int(tmp.substr(3, 9)));
                _accbase.push_back(str2int(tmp.substr(14, 9)));
                GREAT_DEBUG("reading PV base");
            }
            else if (tmp.substr(0, 2) == "%i")
            {
                GREAT_DEBUG("additional info");
            }
            else if (tmp.substr(0, 2) == "/*")
            {
                GREAT_DEBUG("comments");
            }
            else if (tmp.substr(0, 2) == "* ")
            { // END OF HEADER !!!
                GREAT_DEBUG("END OF HEADER");
                _mutex.unlock();
                return -1;
            }
            else
            {
                GREAT_DEBUG("warning: unknown header message");
            }

            t_gcoder::_consume(tmpsize);
        }

        _mutex.unlock();
        return consume;
    }

    int t_sp3::decode_data(char* buff, int sz, int& cnt, vector<string>& errmsg)
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
            if (tmp.substr(0, 1) == "*" || tmp.substr(0, 3) == "EOF")
            {
                if (tmp.substr(0, 3) == "EOF")
                {
                    GREAT_DEBUG("EOF found");
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return tmpsize;
                }

                if (_nrecord > 0 && _nrecord != _nrecmax)
                {
                    GREAT_WARN(std::string("not equal number of satellites _nrecord is ") + std::to_string(_nrecord) + ", _nrecmax is " +
                               std::to_string(_nrecmax) + "!");
                }

                char dummy;
                int yr, mn, dd, hr, mi;
                double sc;
                stringstream ss(tmp);
                ss >> dummy >> yr >> mn >> dd >> hr >> mi >> sc;

                if (ss.fail())
                {
                    GREAT_CRITICAL(std::string("incorrect SP3 epoch record : ") + ss.str() + "!");
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return -1;
                }

                int sod = hr * 3600 + mi * 60 + (int)sc;
                _lastepo.from_ymd(yr, mn, dd, sod);

                GREAT_DEBUG(std::string("reading EPOCH [") + std::to_string(_nrecord) + "] - " + _lastepo.str(" %Y-%m-%d %H:%M:%S") + " ");
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
                {
                    if (pos[i] == 0.0)
                    {
                        xyz[i] = UNDEFVAL_POS; // gephprec
                    }
                    else
                    {
                        xyz[i] = pos[i] * 1000; // km -> m
                    }
                }

                if (pos[3] > 999999)
                {
                    t = UNDEFVAL_CLK; // gephprec
                }
                else
                {
                    t = pos[3] / 1000000; // us -> s
                }
                // fill single data record
                if (!_filter_gnss(prn))
                {
                    GREAT_DEBUG(std::string("skip satellite : ") + prn + " for you do not set at the xml file");
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
                    GREAT_ERROR(std::string("incorrect SP3 data record: ") + ss.str());
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
                map<string, t_gdata*>::iterator it = _data.begin();
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
                    GREAT_ERROR(std::string("incorrect SP3 data record: ") + ss.str());
                    t_gcoder::_consume(tmpsize);
                    _mutex.unlock();
                    return -1;
                }
            }

            _nrecord++;

            if (_nrecord > _nrecmax)
            {
                _nrecmax = _nrecord;
            }

            t_gcoder::_consume(tmpsize);
        }
        _mutex.unlock();
        return tmpsize;
    }

} // namespace gnut
