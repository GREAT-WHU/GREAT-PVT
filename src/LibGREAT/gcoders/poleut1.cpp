/**
 * @file         poleut1.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        The base class used to decode poleut1 file information.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include <string>
#include <algorithm>

#include "gcoders/poleut1.h"
#include "gdata/gpoleut1.h"
#include "gall/gallobj.h"

using namespace std;
namespace great
{
    t_poleut1::t_poleut1(t_gsetbase *s, string version, int sz)
        : t_gcoder(s, version, sz), _begtime(0), _endtime(0), _interval(0), _parnum(0)
    {
    }
    int t_poleut1::decode_head(char *buff, int sz, vector<string> &errmsg)
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
        string timetype;
        string time;
        string param;
        try
        {
            while ((tmpsize = t_gcoder::_getline(tmp)) >= 0)
            {

                consume += tmpsize;

                // first information
                if (tmp.substr(0, 2) == "% ")
                {
                    istringstream istr(tmp.substr(2));
                    if (tmp.substr(2, 1) == "U")
                    {
                        istr >> timetype >> timetype >> timetype >> _timetype;
                    }
                    else if (tmp.substr(2, 1) == "S")
                    {
                        istr >> time >> time >> _begtime >> _endtime >> _interval;
                        map<string, t_gdata *>::iterator it = _data.begin();
                        while (it != _data.end())
                        {
                            if (it->second->id_type() == t_gdata::ALLPOLEUT1)
                            {
                                istr >> time >> time >> _begtime >> _endtime >> _interval;
                                dynamic_cast<t_gpoleut1 *>(it->second)->setBegEndTime(_begtime, _endtime);
                            }
                            it++;
                        }
                    }
                    else if (tmp.substr(2, 1) == "N")
                    {
                        istr >> param >> param >> param >> param >> _parnum;
                        string unitstr;
                        while (istr >> unitstr)
                        {
                            replace(unitstr.begin(), unitstr.end(), 'D', 'E');
                            _parunit.push_back(atof(unitstr.c_str()));
                        }
                    }
                }
                else if (tmp.substr(0, 2) == "+p")
                {
                    istringstream istr2(tmp.substr(2));
                }
                else if (tmp.substr(0, 2) == "%%") 
                {
                    if (tmp.substr(3, 1) == "M")
                    {
                        istringstream istr3(tmp.substr(6));
                        string name;
                        int i = 0;
                        while (i < _parnum)
                        {
                            istr3 >> name;
                            _parname.push_back(name);
                            i++;
                        }
                    }
                }
                else
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "END OF HEADER");
                    _mutex.unlock();
                    return -1;
                }

                t_gcoder::_consume(tmpsize);
            }
            _mutex.unlock();
            return consume;
        }
        catch (...)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR : t_poleut1::decode_head throw exception");
            return -1;
        }
    }

    int t_poleut1::decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {
        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };
        string tmp;
        int tmpsize = 0;
        try
        {
            while ((tmpsize = t_gcoder::_getline(tmp, 0)) >= 0)
            {
                if (tmp.substr(0, 1) == " ")
                {
                    double t;
                    istringstream istr(tmp.substr(0, 65));
                    vector<double> par;
                    istr >> t;
                    double pa;
                    while (istr >> pa)
                    {
                        par.push_back(pa);
                    }
                    int mjd = floor(t);
                    int sec = (int)((t - mjd) * 86400 / 3600.0) * 3600;
                    t_gtime T;
                    T.from_mjd(mjd, sec, 0.0);
                    map<string, double> data;
                    for (int i = 0; i < _parname.size(); i++)
                    {
                        data[_parname[i]] = par[i] * _parunit[i];
                    }
                    map<string, t_gdata *>::iterator it = _data.begin();
                    while (it != _data.end())
                    {
                        if (T.mjd() > _endtime)
                            break;
                        if (it->second->id_type() == t_gdata::ALLPOLEUT1)
                            dynamic_cast<t_gpoleut1 *>(it->second)->setEopData(T, data, _timetype, _interval);
                        it++;
                    }
                }
                t_gcoder::_consume(tmpsize);
            }
            _mutex.unlock();
            return tmpsize;
        }
        catch (...)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR : t_poleut1::decode_data throw exception");
            return -1;
        }
    }


}
