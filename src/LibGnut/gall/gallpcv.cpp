/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include <sstream>
#include <stdlib.h>
#include <algorithm>

#include "gall/gallpcv.h"

using namespace std;

namespace gnut
{

    t_gallpcv::t_gallpcv()
        : t_gdata(),
          _overwrite(false)
    {
        id_type(t_gdata::ALLPCV);
    }

    t_gallpcv::t_gallpcv(t_spdlog spdlog)
        : t_gdata(spdlog),

          _overwrite(false)
    {
        id_type(t_gdata::ALLPCV);
    }

    t_gallpcv::~t_gallpcv()
    {
    }

    int t_gallpcv::addpcv(shared_ptr<t_gpcv> pcv)
    {
        _gmutex.lock();

        string key = pcv->pcvkey();
        string typ = pcv->pcvtyp();
        t_gtime beg = pcv->beg();
        t_gtime end = pcv->end();

        pcv->gnote(_gnote); // TRANSFER GNOTE

        if (_mappcv.find(key) == _mappcv.end() ||
            _mappcv[key].find(typ) == _mappcv[key].end() ||
            _mappcv[key][typ].find(beg) == _mappcv[key][typ].end())
        {

            // new instance
            _mappcv[key][typ][beg] = pcv;
        }
        else
        {

            if (_overwrite)
            {
                _mappcv[key][typ][beg] = pcv;
            }
            else
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "already exists, not overwritten !\n");
            }
        }
        if (_spdlog)
            SPDLOG_LOGGER_DEBUG(_spdlog, "add PCV " + key + " " + typ + beg.str_ymdhms(" ") + end.str_ymdhms(" "));

        _gmutex.unlock();
        return 1;
    }

    shared_ptr<t_gpcv> t_gallpcv::gpcv(const string &ant, const string &ser, const t_gtime &t)
    {

        _gmutex.lock();
        shared_ptr<t_gpcv> gpcv = _find(ant, ser, t);
        _gmutex.unlock();
        return gpcv;
    }

    shared_ptr<t_gpcv> t_gallpcv::_find(const string &ant_name, const string &ser_num, const t_gtime &t)
    {
        string ant(ant_name);
        string ser(ser_num);
        string ant0 = ant; // DEFAULT (ANTENNA + RADOME)
        string ser0 = "";  // DEFAULT (TYPE CALLIBRATION)

        // antenna not found in the list
        if (_mappcv.find(ant) == _mappcv.end())
        {

            // try alternative name for receiver antenna (REPLACE RADOME TO NONE!)
            if (ant.size() >= 19)
                ant0 = ant.replace(16, 4, "NONE"); // REPLACE DEFAULT ANT-NAME

            if (_mappcv.find(ant0) == _mappcv.end())
            {
                return _pcvnull;
            }
        }

        // individual antenna (serial number used, '*'=ANY!)
        map<string, t_map_epo>::iterator itser;
        if (ser.find("*") == string::npos)
        { // SPECIAL SERIAL DEFINED (OR TYPE CALLIBRATION "")
            for (itser = _mappcv[ant0].begin(); itser != _mappcv[ant0].end(); ++itser)
            {

                // antenna (serial number) found in the list!
                if (_mappcv[ant0].find(ser) != _mappcv[ant0].end())
                {
                    ser0 = ser; // REPLACE DEFAULT SER-NUMB (INDIVIDUAL CALLIBRATION)
                    break;
                }
            }
        }

        // try to find the approximation (ser0=DEFAULT, ser=NOT FOUND)
        if (ser0 == "" && _mappcv[ant].find(ser) == _mappcv[ant].end())
        {
            ser = ""; // REPLACE ALWAYS WITH TYPE CALLIBRATION !!!
        }

        // check/return individual and check time validity
        if (ser0 == ser)
        { // SERIAL NUMBER FOUND!
            t_map_epo::iterator it = _mappcv[ant][ser0].begin();
            while (it != _mappcv[ant][ser0].end())
            {
                if (((it->second)->beg() < t || (it->second)->beg() == t) &&
                    (t < (it->second)->end() || t == (it->second)->end()))
                {
                    return it->second;
                }
                it++;
            }
        }


        return _pcvnull;
    }

} // namespace
