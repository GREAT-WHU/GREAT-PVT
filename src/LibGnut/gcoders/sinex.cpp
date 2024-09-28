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
#include <algorithm>

#include "gcoders/sinex.h"
#include "gutils/gsys.h"
#include "gutils/gtriple.h"
#include "gutils/gtypeconv.h"
#include "gprod/gprodcrd.h"
#include "gmodels/gpcv.h"
#include "gdata/gdata.h"

using namespace std;

namespace gnut
{
    t_sinex::t_sinex(t_gsetbase *s, string version, int sz, string id)
        : t_gcoder(s, version, sz, id),
          _snx_type(SINEX_GNS),
          _technique('P'),
          _parindex(1), // should start from 1 (SINEX format)
          _tmpsize(0),
          _consume(0),
          _complete(true),
          _estimation(false),
          _code_label("CODE"), // LABEL FOR SITE
          _list_gnss(""),
          _line(""),
          _site(""),
          _block(""),
          _pco_mod("igsXX_XXXX"),
          _ac(""),
          _pt_prod(0),
          _file_beg(t_gtime::GPS),
          _file_end(t_gtime::GPS),
          _file_run(t_gtime::GPS),
          _allobj(0),
          _end_epoch(false),
          _end_estim(false)
    {
    }

    int t_sinex::decode_head(char *buff, int sz, vector<string> &errmsg)
    {

        _mutex.lock();

        t_gcoder::_add2buffer(buff, sz);
        while ((_tmpsize = t_gcoder::_getline(_line)) >= 0)
        {
            if (_tmpsize < 65)
            {
                _mutex.unlock();
                return 0;
            }

            // 1st line
            // -------- "FILE HEADER" --------
            if (_line.find("%=TRO") != string::npos || _line.find("%=BIA") != string::npos || _line.find("%=SNX") != string::npos)
            {
                _version = trim(_line.substr(5, 5));
                substitute(_version, " ", "");
                _ac = trim(_line.substr(11, 3));

                if (_snx_type == SINEX_GNS || _line[17] == ':')
                { // SINEX style (YY instead of YYYY)
                    string file_run = trim(_line.substr(15, 12));
                    substitute(file_run, " ", "");
                    _file_run.from_str("%y:%j:%s", file_run);

                    string file_beg = trim(_line.substr(32, 12));
                    substitute(file_beg, " ", "");
                    _file_beg.from_str("%y:%j:%s", file_beg);
                    if (file_beg.compare("00:000:00000") == 0)
                        _file_beg = FIRST_TIME;

                    string file_end = trim(_line.substr(45, 12));
                    substitute(file_end, " ", "");
                    _file_end.from_str("%y:%j:%s", file_end);
                    if (file_end.compare("00:000:00000") == 0)
                        _file_end = LAST_TIME;

                    string num_est = trim(_line.substr(60, 5));
                    substitute(num_est, " ", "");
                    if (str2int(num_est) > 0)
                        _estimation = true;
                }
                else
                {
                    string file_run = trim(_line.substr(15, 14));
                    substitute(file_run, " ", "");
                    _file_run.from_str("%Y:%j:%s", file_run);

                    string file_beg = trim(_line.substr(34, 14));
                    substitute(file_beg, " ", "");
                    _file_beg.from_str("%Y:%j:%s", file_beg);
                    if (file_beg.compare("00:000:00000") == 0)
                        _file_beg = FIRST_TIME;

                    string file_end = trim(_line.substr(49, 14));
                    substitute(file_end, " ", "");
                    _file_end.from_str("%Y:%j:%s", file_end);
                    if (file_end.compare("00:000:00000") == 0)
                        _file_end = LAST_TIME;
                }

                if (_decode_vers() == 0)
                {
                    t_gcoder::_consume(_tmpsize);
                    _mutex.unlock();
                    return -1;
                }
            }
        }
        _mutex.unlock();
        return 0;
    }

    int t_sinex::decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {

        _mutex.lock();

        t_gcoder::_add2buffer(buff, sz);

        _complete = true;
        _consume = 0;
        _tmpsize = 0;
        _line = "";

        _decode_data();

        _mutex.unlock();
        return _consume;
    }

    void t_sinex::technique(char c)
    {

        switch (c)
        {
        case 'c':
        case 'C':
            _technique = 'C';
            break; // COMBINED
        case 'd':
        case 'D':
            _technique = 'D';
            break; // DORIS
        case 'p':
        case 'P':
            _technique = 'P';
            break; // GNSS
        case 'r':
        case 'R':
            _technique = 'R';
            break; // VLBI
        case 'w':
        case 'W':
            _technique = 'W';
            break; // RADIOMETER
        case 's':
        case 'S':
            _technique = 'S';
            break; // RADIOSONDE
        case 'f':
        case 'F':
            _technique = 'F';
            break; // NWM FORECAST
        case 'm':
        case 'M':
            _technique = 'M';
            break; // NWM RE-ANALYSIS
        case 'n':
        case 'N':
            _technique = 'N';
            break; // CLIMATE MODEL
        default:
            _technique = 'P';
        }

        switch (_technique)
        {
        case 'C':
        case 'P':
        case 'D':
        case 'R':
            _snx_type = TROSNX_GNS;
            break;
        case 'F':
        case 'M':
        case 'N':
            _snx_type = TROSNX_NWM;
            break;
        case 'W':
        case 'S':
            _snx_type = TROSNX_OTH;
            break;
        default:
            _snx_type = TROSNX_OTH;
        }
    }

    char t_sinex::technique()
    {
        return _technique;
    }

    shared_ptr<t_grec> t_sinex::_get_grec(string site)
    {

        if (_epo_pos.find(site) == _epo_pos.end())
        {

            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "Warning: object not found for site: " + site + " [should not happen]");
            return 0;
        }

        if (_epo_pos[site].size() == 0)
        {

            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "Position not found for site: " + site);
            return 0;
        }

        return dynamic_pointer_cast<t_grec>(_mapobj[site]);
    }

    int t_sinex::_decode_vers()
    {

        if (_spdlog)
            SPDLOG_LOGGER_DEBUG(_spdlog, "Read VERS: " + _version);

        return 0;
    }

    int t_sinex::_decode_data()
    {

        while (_complete && (_tmpsize = t_gcoder::_getline(_line)) >= 0)
        {

            _complete = true;

            // -------- "COMMENT" --------
            if (_line.find("*", 0) != string::npos)
            {

                _comment.push_back(_line); // _line.substr(0,60));

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Read COMM: " + _line.substr(1, 25)); // DEFAULT: 3

                this->_decode_comm();
            }

            // -------- "BLOCK START" --------
            else if (_line[0] == '+' && _line[1] != ' ')
            { // sometimes in other block
                _block = cut_crlf(trim(_line.substr(1)));
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "BLOCK beg: " + _block.substr(0, 13)); // DEFAULT: 1
            }

            // -------- "BLOCK END" --------
            else if (_line[0] == '-' && _line.find(_block) != string::npos)
            { // sometimes in other block
                string block_tmp = cut_crlf(trim(_line.substr(1)));
                if (_block != block_tmp)
                {
                    cerr << "uncorectly closed SINEX BLOCK: " + _block << endl;
                    t_gcoder::_consume(_tmpsize);
                    return -1;
                }

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "BLOCK end: " + _block.substr(0, 13)); // DEFAULT: 1

                if (_block.compare("SOLUTION/EPOCHS") == 0)
                    _end_epoch = true;
                if (_block.compare("SOLUTION/ESTIMATE") == 0)
                    _end_estim = true;
                _block = "";
            }

            // -------- "BLOCK READING" --------
            else
                this->_decode_block();

            // -------- CONSUME --------
            if (_complete)
                _consume += t_gcoder::_consume(_tmpsize);
            else
                break;
        }

        // Set object coordinates
        if (_end_epoch && _end_estim)
        {
            _set_rec_crd();
        }

        return _consume;
    }

    int t_sinex::_decode_comm()
    {

        int idx = 0;
        if ((idx = _line.find("SITE")) != string::npos)
            _mapidx["SIT"] = make_pair(idx, 4);
        if ((idx = _line.find("STATION__")) != string::npos)
            _mapidx["SIT"] = make_pair(idx, 9); // NEW TRO-SINEX2

        if ((idx = _line.find("__STA_X_____")) != string::npos)
            _mapidx["X"] = make_pair(idx, 12);
        if ((idx = _line.find("__STA_Y_____")) != string::npos)
            _mapidx["Y"] = make_pair(idx, 12);
        if ((idx = _line.find("__STA_Z_____")) != string::npos)
            _mapidx["Z"] = make_pair(idx, 12);

        return 0;
    }

    int t_sinex::_decode_block()
    {

        // -------- "FILE/REFERENCE" --------
        if (_block.find("FILE/REFERENCE") != string::npos)
        {

            if (_line.find(" DESC", 4) != string::npos)
            {

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Read FILE/DESC: " + _line.substr(20, 60));
            }
            else if (_line.find(" OUTP", 4) != string::npos)
            {

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Read FILE/OUTP: " + _line.substr(20, 60));
            }
            else if (_line.find(" CONT", 4) != string::npos)
            {

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Read FILE/CONT: " + _line.substr(20, 60));
            }
            else if (_line.find(" SOFT", 4) != string::npos)
            {

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Read FILE/SOFT: " + _line.substr(20, 60));
            }
            else if (_line.find(" HARD", 4) != string::npos)
            {

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Read FILE/HARD: " + _line.substr(20, 60));
            }
            else if (_line.find(" INPU", 4) != string::npos)
            {

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Read FILE/INPT: " + _line.substr(20, 60));
            }
        }
        else if (_block.find("SITE/RECEIVER") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Read SITE/RECEIVER");

            if (_snx_type == SINEX_GNS)
            {
                string site = trim(_line.substr(1, 4));
                transform(site.begin(), site.end(), site.begin(), ::toupper);
                string begfmt = trim(_line.substr(16, 12));
                string endfmt = trim(_line.substr(29, 12));
                string rcv = trim(_line.substr(42, 20));
                t_gtime beg(t_gtime::GPS);
                t_gtime end(t_gtime::GPS);

                if (begfmt.compare("00:000:00000") == 0)
                    beg = LAST_TIME;
                else
                    beg.from_str("%y:%j:%s", begfmt);

                if (endfmt.compare("00:000:00000") == 0)
                    end = LAST_TIME;
                else
                    end.from_str("%y:%j:%s", endfmt);

                shared_ptr<t_grec> new_obj = make_shared<t_grec>(_spdlog);
                new_obj->id(site);
                new_obj->spdlog(_spdlog);
                new_obj->rec(rcv, beg, end);

                _complete_obj(new_obj, beg);
            }
        }
        else if (_block.find("SITE/ANTENNA") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Read SITE/ANTENNA");

            if (_snx_type == SINEX_GNS)
            {
                string site = trim(_line.substr(1, 4));
                transform(site.begin(), site.end(), site.begin(), ::toupper);
                string begfmt = trim(_line.substr(16, 12));
                string endfmt = trim(_line.substr(29, 12));
                string ant = trim(_line.substr(42, 20));
                t_gtime beg(t_gtime::GPS);
                t_gtime end(t_gtime::GPS);

                if (begfmt.compare("00:000:00000") == 0)
                    beg = LAST_TIME;
                else
                    beg.from_str("%y:%j:%s", begfmt);

                if (endfmt.compare("00:000:00000") == 0)
                    end = LAST_TIME;
                else
                    end.from_str("%y:%j:%s", endfmt);

                shared_ptr<t_grec> new_obj = make_shared<t_grec>(_spdlog);
                new_obj->id(site);
                new_obj->spdlog(_spdlog);
                new_obj->ant(ant, beg, end);
                _complete_obj(new_obj, beg);
            }
        }
        else if (_block.find("SITE/ECCENTRICITY") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Read SITE/ECCENTRICITY");

            if (_snx_type == SINEX_GNS)
            {
                string site = trim(_line.substr(1, 4));
                transform(site.begin(), site.end(), site.begin(), ::toupper);
                string begfmt = trim(_line.substr(16, 12));
                string endfmt = trim(_line.substr(29, 12));
                t_gtime beg(t_gtime::GPS);
                t_gtime end(t_gtime::GPS);

                if (begfmt.compare("00:000:00000") == 0)
                    beg = LAST_TIME;
                else
                    beg.from_str("%y:%j:%s", begfmt);

                if (endfmt.compare("00:000:00000") == 0)
                    end = LAST_TIME;
                else
                    end.from_str("%y:%j:%s", endfmt);

                string eccsys = trim(_line.substr(42, 3));
                t_gtriple ecc;
                double a = str2dbl(trim(_line.substr(46, 8)));
                double b = str2dbl(trim(_line.substr(55, 8)));
                double c = str2dbl(trim(_line.substr(64, 8)));

                shared_ptr<t_grec> new_obj = make_shared<t_grec>(_spdlog);
                new_obj->id(site);
                new_obj->spdlog(_spdlog);
                if (eccsys.compare("UNE") == 0)
                {
                    ecc[0] = b;
                    ecc[1] = c;
                    ecc[2] = a;
                    new_obj->eccneu(ecc, beg, end);
                }
                else if (eccsys.compare("XYZ") == 0)
                {
                    ecc[0] = a;
                    ecc[1] = b;
                    ecc[2] = c;
                    new_obj->eccxyz(ecc, beg, end);
                }

                _complete_obj(new_obj, beg);
            }
        }
        else if (_block.find("SITE/ID") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Read SITE/ID");

            if (_snx_type == SINEX_GNS)
            {
                string site = trim(_line.substr(1, 4));
                transform(site.begin(), site.end(), site.begin(), ::toupper);
                string domes = trim(_line.substr(9, 9));
                string descr = trim(_line.substr(21, 22));

                int lon_d = str2int(trim(_line.substr(44, 3)));
                int lon_m = str2int(trim(_line.substr(48, 2)));
                double lon_s = str2dbl(trim(_line.substr(51, 4)));
                double lon = double(lon_d) + double(lon_m) / 60 + lon_s / 3600;

                int lat_d = str2int(trim(_line.substr(56, 3)));
                int lat_m = str2int(trim(_line.substr(60, 2)));
                double lat_s = str2dbl(trim(_line.substr(63, 4)));
                double lat = double(lat_d) + double(lat_m) / 60 + lat_s / 3600;

                double height = str2dbl(trim(_line.substr(68, 7)));

                t_gtriple ell(lat, lon, height);
                t_gtriple xyz;
                ell2xyz(ell, xyz, true);

                if (!xyz.zero() && !_estimation)
                {
                    shared_ptr<t_grec> new_obj = make_shared<t_grec>(_spdlog);
                    new_obj->id(site);
                    new_obj->spdlog(_spdlog);
                    t_gtriple std(10, 10, 10);
                    new_obj->crd(xyz, std, _file_beg, _file_end);

                    _complete_obj(new_obj, _file_beg);
                }
            }
        }
        else if (_block.find("SOLUTION/EPOCHS") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Read SOLUTION/EPOCHS");

            string site = trim(_line.substr(1, 4));
            transform(site.begin(), site.end(), site.begin(), ::toupper);

            string begfmt = trim(_line.substr(16, 12));
            string endfmt = trim(_line.substr(29, 12));
            t_gtime beg(t_gtime::GPS);
            t_gtime end(t_gtime::GPS);

            if (begfmt.compare("00:000:00000") == 0)
                beg = LAST_TIME;
            else
                beg.from_str("%y:%j:%s", begfmt);

            if (endfmt.compare("00:000:00000") == 0)
                end = LAST_TIME;
            else
                end.from_str("%y:%j:%s", endfmt);

            _sol_epoch[site].insert(beg);
            _sol_epoch[site].insert(end);
        }
        else if (_block.find("SOLUTION/ESTIMATE") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Read SOLUTION/ESTIMATE");

            string site = trim(_line.substr(14, 4));
            transform(site.begin(), site.end(), site.begin(), ::toupper);

            string param = trim(_line.substr(7, 6));
            transform(param.begin(), param.end(), param.begin(), ::toupper);

            string epofmt = trim(_line.substr(27, 12));
            t_gtime epo(t_gtime::GPS);
            epo.from_str("%y:%j:%s", epofmt);

            double est = str2dbl(trim(_line.substr(47, 21)));
            double std = str2dbl(trim(_line.substr(69, 11)));
            t_gpair val(est, std);

            _sol_estim[site][epo][param] = val;
        }

        return 0;
    }

    void t_sinex::_add_data(string id, t_gdata *pt_data)
    {

        if (pt_data == 0)
            return;

        // ORDER IS IMPORTANT!
        if (pt_data->id_type() == t_gdata::ALLPROD &&
            pt_data->id_group() == t_gdata::GRP_PRODUCT)
        {
            // ALL PRODUCTS (MET) special case (input meteo parameters -> encode only)
            // ----> currently the product needs to be labeled "MET" to be recognized
            // ----> THIS SHOULD BE IN FUTURE CONSOLIDATED using gprodmet instead of gprod
            if (id != "MET")
            {
                _pt_prod = dynamic_cast<t_gallprod *>(pt_data);
            }
        }

        // ALL OBJECTS
        if (pt_data->id_type() == t_gdata::ALLOBJ)
        {
            if (!_allobj)
            {
                _allobj = dynamic_cast<t_gallobj *>(pt_data);
            }
        }

        return;
    }

    t_gtime t_sinex::_site_beg(string site)
    {
        return *_epo_pos[site].begin();
    }

    t_gtime t_sinex::_site_end(string site)
    {
        return *_epo_pos[site].rbegin();
    }

    void t_sinex::_complete_obj(shared_ptr<t_grec> gobj, const t_gtime &epo)
    {
        if (!_allobj)
            return;

        string id = gobj->id();
        bool found = false;
        // 4-CH in Sinex and 9-CH in settings
        for (auto it = _rec.begin(); it != _rec.end(); it++)
        {
            if (it->compare(0, 4, id, 0, 4) == 0)
            {
                found = true;
                gobj->id(*it); // replace id from settings
            }
        }

        id = gobj->id();

        shared_ptr<t_grec> old_obj = dynamic_pointer_cast<t_grec>(_allobj->obj(id));

        if (old_obj == nullptr && found)
        {
            _allobj->add(gobj);
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Object created, using SINEX: " + id + epo.str_ymdhms(" "));
        }
        else if (old_obj != nullptr)
        {
            old_obj->compare(gobj, epo, "SINEX");
        }
    }

    void t_sinex::_set_rec_crd()
    {
        for (auto itSITE = _sol_estim.begin(); itSITE != _sol_estim.end(); itSITE++)
        {
            string site = itSITE->first;
            t_gtriple xyz;
            t_gtriple std;
            t_gtime epo;
            t_gtime beg = FIRST_TIME;
            t_gtime end = LAST_TIME;

            auto itSITE2 = _sol_epoch.find(site);
            if (itSITE2 == _sol_epoch.end())
                continue; // by ZHJ

            for (auto itEPO = itSITE->second.begin(); itEPO != itSITE->second.end(); itEPO++)
            {
                epo = itEPO->first;

                auto itBE = itSITE2->second.upper_bound(epo);
                if (itBE != itSITE2->second.begin() && itBE != itSITE2->second.end())
                {
                    end = *itBE;
                    itBE--;
                    beg = *itBE;
                }

                for (auto itPAR = itEPO->second.begin(); itPAR != itEPO->second.end(); itPAR++)
                {
                    string par = itPAR->first;
                    if (par.compare("STAX") == 0)
                    {
                        xyz.set(0, itPAR->second.crd(0));
                        std.set(0, itPAR->second.crd(1));
                    }
                    else if (par.compare("STAY") == 0)
                    {
                        xyz.set(1, itPAR->second.crd(0));
                        std.set(1, itPAR->second.crd(1));
                    }
                    else if (par.compare("STAZ") == 0)
                    {
                        xyz.set(2, itPAR->second.crd(0));
                        std.set(2, itPAR->second.crd(1));
                    }
                }
            }
            shared_ptr<t_grec> new_obj = make_shared<t_grec>(_spdlog);
            new_obj->id(site);
            new_obj->spdlog(_spdlog);
            new_obj->crd(xyz, std, beg, end);

            _complete_obj(new_obj, epo);
        }
    }

} // namespace
