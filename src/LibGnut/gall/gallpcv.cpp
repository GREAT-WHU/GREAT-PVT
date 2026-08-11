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


namespace gnut
{

    t_gallpcv::t_gallpcv() :
        t_gdata(),
        _overwrite(false)
    {
        id_type(t_gdata::ID_TYPE::ALLPCV);
    }

    t_gallpcv::~t_gallpcv()
    {
    }

    int t_gallpcv::addpcv(std::shared_ptr<t_gpcv> pcv)
    {

        std::string key = pcv->pcvkey();
        std::string typ = pcv->pcvtyp();
        t_gtime beg = pcv->beg();
        t_gtime end = pcv->end();

        if (pcv->is_transmitter())
        {
            GSAT sat = pcv->sat();
            if (sat == GSAT::NONE)
            {
                return -1;
            }
            if (_overwrite || _mapsatpcv[sat].find(beg) == _mapsatpcv[sat].end()) _mapsatpcv[sat][beg] = pcv;
        }
        else if (_mappcv.find(key) == _mappcv.end() || _mappcv[key].find(typ) == _mappcv[key].end() ||
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
                GREAT_DEBUG("already exists, not overwritten !\n");
            }
        }
        GREAT_DEBUG("add PCV " + key + " " + typ + beg.str_ymdhms(" ") + end.str_ymdhms(" "));

        return 1;
    }

    std::shared_ptr<t_gpcv> t_gallpcv::gpcv(const std::string& ant, const std::string& ser, const t_gtime& t)
    {
        GSAT sat = str2gsat(ant);
        std::shared_ptr<t_gpcv> gpcv = sat == GSAT::NONE ? _find(ant, ser, t) : _find(sat, t);
        return gpcv;
    }

    std::shared_ptr<t_gpcv> t_gallpcv::gpcv(GSAT sat, const t_gtime& t)
    {
        std::shared_ptr<t_gpcv> gpcv = _find(sat, t);
        return gpcv;
    }

    std::shared_ptr<t_gpcv> t_gallpcv::_find(GSAT sat, const t_gtime& t)
    {
        auto found = _mapsatpcv.find(sat);
        if (found == _mapsatpcv.end()) return _pcvnull;
        auto it = found->second.upper_bound(t);
        if (it == found->second.begin()) return _pcvnull;
        --it;
        return (it->second->beg() <= t && t <= it->second->end()) ? it->second : _pcvnull;
    }

    std::shared_ptr<t_gpcv> t_gallpcv::_find(const std::string& ant_name, const std::string& ser_num, const t_gtime& t)
    {
        std::string ant(ant_name);
        std::string ser(ser_num);
        std::string ant0 = ant; // DEFAULT (ANTENNA + RADOME)
        std::string ser0 = "";  // DEFAULT (TYPE CALLIBRATION)

        // antenna not found in the list
        if (_mappcv.find(ant) == _mappcv.end())
        {
            // try alternative name for receiver antenna (REPLACE RADOME TO NONE!)
            if (ant.size() >= 19)
            {
                ant0 = ant.replace(16, 4, "NONE"); // REPLACE DEFAULT ANT-NAME
            }

            if (_mappcv.find(ant0) == _mappcv.end())
            {
                return _pcvnull;
            }
        }

        // individual antenna (serial number used, '*'=ANY!)
        std::map<std::string, t_map_epo>::iterator itser;
        if (ser.find("*") == std::string::npos)
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

} // namespace gnut
