/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <memory>

#include "gcoders/biabernese.h"
#include "gutils/gtypeconv.h"
#include "gmodels/gbias.h"

using namespace std;

namespace gnut
{
    t_biabernese::t_biabernese(t_gsetbase *s, string version, int sz, string id)
        : t_gcoder(s, version, sz, id)
    {

        _allbias = 0;
        _ac = "";
        _beg = FIRST_TIME; 
        _end = LAST_TIME;  
    }

    int t_biabernese::decode_head(char *buff, int sz, vector<string> &errmsg)
    {
        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };

        string line;
        int consume = 0;
        int tmpsize = 0;
        size_t idx = 0;
        string year = "";
        string mon = "";

        while ((tmpsize = t_gcoder::_getline(line)) >= 0)
        {
            consume += tmpsize;
            if (line.find("%=BIA") == 0)
            { // first line
                _is_bias = true;
                _version = str2dbl(line.substr(6, 4));
                _ac = line.substr(11, 3);
                if (_ac == "IGG")
                    _ac = "CAS";
                if (_ac == "CNT" && line[64] == 'P')
                    _is_absolute = true;
                else
                    _is_absolute = line.substr(64, 1) == "A";

                if (_is_absolute)
                    _ac += "_A";
                else
                    _ac += "_R";
            }
            else if (line.find("+BIAS/SOLUTION") != string::npos)
            {
                t_gcoder::_consume(tmpsize);
                _mutex.unlock();
                return -1;
            }
            else if (line.find("CODE'S MONTHLY") != string::npos)
            {
                _ac = "COD_R";
                if ((idx = line.find("YEAR ")) != string::npos)
                {
                    year = line.substr(idx + 5, 4);
                    mon = line.substr(idx + 17, 2);
                }
                if ((idx = line.find("YEAR-MONTH")) != string::npos)
                {
                    if (line.substr(idx + 11, 1) > "8")
                        year = "19" + line.substr(idx + 11, 2);
                    else
                        year = "20" + line.substr(idx + 11, 2);
                    mon = line.substr(idx + 14, 2);
                }
                t_gcoder::_consume(tmpsize);
                _mutex.unlock();
                return -1;
            }
            else if (line.find("CODE'S 30-DAY") != string::npos)
            {
                _ac = "COD_R";
                string end_doy = "";
                if ((idx = line.find("ENDING DAY")) != string::npos)
                {
                    year = line.substr(idx + 16, 4);
                    end_doy = line.substr(idx + 11, 3);
                }
                string time_str = year + ":" + end_doy;
                t_gcoder::_consume(tmpsize);
                _mutex.unlock();
                return -1;
            }
            t_gcoder::_consume(tmpsize);
        }
        _mutex.unlock();
        return consume;
    }

    int t_biabernese::decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {
        if (_is_bias)
        {
            if (_version < 1.0)
                return _decode_data_sinex_0(buff, sz, cnt, errmsg);
            else
                return _decode_data_sinex(buff, sz, cnt, errmsg);
        }
        else if (_ac == "COD_R")
        {
            return _decode_data_CODE(buff, sz, cnt, errmsg);
        }
        else
        {
            return -1;
        }
    }

    int t_biabernese::_decode_data_CODE(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {
        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };
        string line;
        int consume = 0;
        int tmpsize = 0;
        bool complete = false;

        GOBS gobs1, gobs2;
        gobs1 = gobs2 = X;

        while ((tmpsize = t_gcoder::_getline(line)) >= 0)
        {
            complete = false;

            size_t idx = 0;
            string signals = "";
            if ((idx = line.find("DIFFERENTIAL (")) != string::npos)
            {
                signals = line.substr(idx + 14, 5);
                gobs1 = str2gobs(signals.substr(0, 2));
                gobs2 = str2gobs(signals.substr(3, 2));
                consume += t_gcoder::_consume(tmpsize);
            }

            string prn = "";
            double dcb, std;
            dcb = std = 0.0;

            istringstream istr(line);
            istr >> prn >> dcb >> std;
            if (!istr.fail())
            {
                complete = true;
            }
            else
            {
                consume += t_gcoder::_consume(tmpsize);
                continue;
            }

            if (complete)
            {

                map<string, t_gdata *>::iterator it = _data.begin();
                while (it != _data.end())
                {
                    if (it->second->id_type() == t_gdata::ALLBIAS)
                    {
                        shared_ptr<t_gbias> pt_bias = make_shared<t_gbias>(_spdlog);
                        pt_bias->set(_beg, _end, dcb * 1e-9 * CLIGHT, gobs1, gobs2);
                        ((t_gallbias *)it->second)->add(_ac, _beg, prn, pt_bias);
                    }
                    it++;
                }
                consume += t_gcoder::_consume(tmpsize);
            }
        }

        _mutex.unlock();
        return consume;
    }

    int t_biabernese::_decode_data_sinex(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {
        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };

        string line;
        int consume = 0;
        int tmpsize = 0;
        bool complete = false;

        GOBS gobs1, gobs2;
        gobs1 = gobs2 = X;

        while ((tmpsize = t_gcoder::_getline(line)) >= 0)
        {
            complete = false;
            if (line.find("-BIAS/SOLUTION") != line.npos || line.find("%=ENDBIA") == 0)
            {
                _mutex.unlock();
                return 0;
            }
            else if (line.substr(1, 4) == "DSB " || line.substr(1, 4) == "ISB " || line.substr(1, 4) == "OSB ")
            {
                string svn, prn, station, obs1, obs2, units;
                double value,factor;
                if (line.size() >= 104)
                {
                    svn = line.substr(6, 4);
                    prn = line.substr(11, 3);
                    station = trim(line.substr(15, 9));
                    if (station.size() != 0)
                    { // currently station-specific bias is not used
                        consume += t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    obs1 = trim(line.substr(25, 4));
                    obs2 = trim(line.substr(30, 4));
                    gobs1 = str2gobs(obs1);
                    gobs2 = str2gobs(obs2);
                    _beg.from_str("%Y:%j:%s", line.substr(35, 14));
                    _end.from_str("%Y:%j:%s", line.substr(50, 14));
                    units = trim(line.substr(65, 4));
                    if (units == "ns")
                    {
                        factor = 1e-9 * CLIGHT;
                    }
                    else if (units == "cyc")
                    {
                        // may for phase bias, but wavelength is needed, so not recommended
                        consume += t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    else
                    {
                        // other units type currently not considered
                        consume += t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    value = str2dbl(line.substr(70, 21));
                    complete = true;
                }
                else if (line.size() >= 92 && _ac == "CNT")
                {

                    svn = line.substr(6, 4);
                    prn = line.substr(11, 3);
                    station = trim(line.substr(15, 9));
                    if (station.size() != 0)
                    { // currently station-specific bias is not used
                        consume += t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    obs1 = trim(line.substr(25, 4));
                    obs2 = trim(line.substr(30, 4));
                    gobs1 = str2gobs(obs1);
                    gobs2 = str2gobs(obs2);
                    _beg.from_str("%Y:%j:%s", line.substr(35, 14));
                    _end.from_str("%Y:%j:%s", line.substr(50, 14));
                    units = trim(line.substr(65, 4));
                    if (units == "ns")
                    {
                        factor = 1e-9 * CLIGHT;
                    }
                    else if (units == "cyc")
                    {
                        // may for phase bias, but wavelength is needed, so not recommended
                        consume += t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    else
                    {
                        // other units type currently not considered
                        consume += t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    value = str2dbl(line.substr(70, 21));
                    complete = true;
                }
                else
                {
                    consume += t_gcoder::_consume(tmpsize);
                    continue;
                }
                if (complete)
                {
                    map<string, t_gdata *>::iterator it = _data.begin();
                    while (it != _data.end())
                    {
                        if (it->second->id_type() == t_gdata::ALLBIAS)
                        {
                            shared_ptr<t_gbias> pt_bias = make_shared<t_gbias>(_spdlog);
                            pt_bias->set(_beg, _end, value * factor, gobs1, gobs2);
                            ((t_gallbias *)it->second)->add(_ac, _beg, prn, pt_bias);
                        }
                        it++;
                    }
                    consume += t_gcoder::_consume(tmpsize);
                }
            }
            else
            {
                consume += t_gcoder::_consume(tmpsize);
            }
        }

        _mutex.unlock();
        return consume;
    }

    int t_biabernese::_decode_data_sinex_0(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {
        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };
        string line;
        int consume = 0;
        int tmpsize = 0;
        bool complete = false;

        GOBS gobs1, gobs2;
        gobs1 = gobs2 = X;

        while ((tmpsize = t_gcoder::_getline(line)) >= 0)
        {
            complete = false;
            if (line.find("-BIAS/SOLUTION") != line.npos || line.find("%=ENDBIA") == 0)
            {
                _mutex.unlock();
                return 0;
            }
            else if (line.substr(1, 4) == "DSB " || line.substr(1, 4) == "ISB " || line.substr(1, 4) == "OSB ")
            {
                string svn, prn, station, obs1, obs2, units;
                double value, factor;
                if (line.size() >= 105)
                {
                    svn = line.substr(6, 4);
                    prn = line.substr(11, 3);
                    station = trim(line.substr(15, 9));
                    if (station.size() != 0)
                    { // currently station-specific bias is not used
                        consume += t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    obs1 = trim(line.substr(30, 4));
                    obs2 = trim(line.substr(35, 4));
                    gobs1 = str2gobs(obs1);
                    gobs2 = str2gobs(obs2);
                    _beg.from_str("%y:%j:%s", line.substr(40, 12));
                    _end.from_str("%y:%j:%s", line.substr(53, 12));
                    units = trim(line.substr(66, 4));
                    if (units == "ns")
                    {
                        factor = 1e-9 * CLIGHT;
                    }
                    else if (units == "cyc")
                    {
                        // may for phase bias, but wavelength is needed, so not recommended
                        consume += t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    else
                    {
                        // other units type currently not considered
                        consume += t_gcoder::_consume(tmpsize);
                        continue;
                    }
                    value = str2dbl(line.substr(71, 21));
                    complete = true;
                }
                else
                {
                    consume += t_gcoder::_consume(tmpsize);
                    continue;
                }
                if (complete)
                {
                    map<string, t_gdata *>::iterator it = _data.begin();
                    while (it != _data.end())
                    {
                        if (it->second->id_type() == t_gdata::ALLBIAS)
                        {
                            shared_ptr<t_gbias> pt_bias = make_shared<t_gbias>(_spdlog);
                            cout << value << factor << value * factor << endl;
                            pt_bias->set(_beg, _end, value * factor, gobs1, gobs2);
                            ((t_gallbias *)it->second)->add(_ac, _beg, prn, pt_bias);
                        }
                        it++;
                    }
                    consume += t_gcoder::_consume(tmpsize);
                }
            }
            else
            {
                consume += t_gcoder::_consume(tmpsize);
            }
        }

        _mutex.unlock();
        return consume;
    }

} // namespace
