/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <stdlib.h>
#include <iostream>

#include "gdata/geph.h"
#include "gutils/gtypeconv.h"
#include "gutils/gcommon.h"


namespace gnut
{

    t_geph::t_geph() :
        t_gdata(),
        _sat(GSAT::NONE),
        _epoch(t_gtime::t_tsys::GPS),
        _validity(true),
        _gio_ptr(0)
    {
        _type = ID_TYPE::EPHGPS;
    }

    t_geph::~t_geph()
    {
    }

    GSYS t_geph::gsys() const
    {
        GSYS tmp = GSYS::GNS;
        if (_valid())
        {
            tmp = gsat2gsys(_sat);
        }
        return tmp;
    }

    std::string t_geph::gsat() const
    {
        std::string tmp = "";
        if (_valid())
        {
            tmp = gsat2str(_sat);
        }
        return tmp;
    }

    t_timdbl t_geph::param(const NAVDATA& n)
    {
        t_timdbl tmp;
        return tmp;
    }

    int t_geph::param(const NAVDATA& n, double val)
    {
        return 0;
    }

    bool t_geph::param_cyclic(const NAVDATA& n)
    {
        if (n == NAVDATA::NAV_E || n == NAVDATA::NAV_OMG)
        {
            return true;
        }
        return false;
    }

    bool t_geph::valid()
    {
        bool tmp = this->_valid();
        return tmp;
    }

    void t_geph::clear()
    {
        this->_clear();
        return;
    }

    void t_geph::_clear()
    {
        _sat = GSAT::NONE;
        _epoch = FIRST_TIME;
    }

    bool t_geph::_valid() const
    {
        if (!_validity || _sat == GSAT::NONE || _epoch == FIRST_TIME)
        {
            return false;
        }

        return true;
    }

} // namespace gnut
