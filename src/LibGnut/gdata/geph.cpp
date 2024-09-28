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

using namespace std;

namespace gnut
{

    t_geph::t_geph()
        : t_gdata(),
          _sat(""),
          _epoch(t_gtime::GPS),
          _validity(true),
          _gio_ptr(0)
    {
        _type = EPHGPS;
    }

    t_geph::t_geph(t_spdlog spdlog)
        : t_gdata(spdlog),
          _sat(""),
          _epoch(t_gtime::GPS),
          _validity(true),
          _gio_ptr(0)
    {
        _type = EPHGPS;
    }

    t_geph::~t_geph()
    {
    }

    GSYS t_geph::gsys() const
    {
        GSYS tmp = GNS;
        _gmutex.lock();
        if (_valid())
            tmp = t_gsys::char2gsys(_sat[0]);
        _gmutex.unlock();
        return tmp;
    }

    string t_geph::gsat() const
    {
        string tmp = "";
        _gmutex.lock();
        if (_valid())
            tmp = _sat;
        _gmutex.unlock();
        return tmp;
    }

    t_timdbl t_geph::param(const NAVDATA &n)
    {
        t_timdbl tmp;
        return tmp;
    }

    int t_geph::param(const NAVDATA &n, double val)
    {
        return 0;
    }

    bool t_geph::param_cyclic(const NAVDATA &n)
    {
        if (n == 2 || n == 6)
            return true;
        return false;
    }

    bool t_geph::valid()
    {
        _gmutex.lock();
        bool tmp = this->_valid();
        _gmutex.unlock();
        return tmp;
    }

    void t_geph::clear()
    {
        _gmutex.lock();
        this->_clear();
        _gmutex.unlock();
        return;
    }

    void t_geph::_clear()
    {

        _sat.clear();
        _epoch = FIRST_TIME;
    }

    bool t_geph::_valid() const
    {

        if (!_validity ||
            _sat.empty() ||
            _sat == "" ||
            _epoch == FIRST_TIME)
            return false;

        return true;
    }


} // namespace
