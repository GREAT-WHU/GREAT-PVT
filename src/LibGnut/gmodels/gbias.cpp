/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/
#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include "gmodels/gbias.h"


namespace gnut
{
#ifndef DIFF_DCB
#define DIFF_DCB (86400 * 31) // 31 days
#endif

    t_gbias::t_gbias() :
        t_gdata()
    {
        _beg = FIRST_TIME;
        _end = LAST_TIME;

        id_type(t_gdata::ID_TYPE::BIAS);
        id_group(t_gdata::ID_GROUP::GRP_MODEL);
    }

    t_gbias::~t_gbias()
    {
    }

    double t_gbias::bias(bool meter)
    {
        if (meter)
        {
            return _val;
        }
        else
        {
            return (_val / CLIGHT) * 1e9;
        }
    }

    void t_gbias::set(const t_gtime& beg, const t_gtime& end, double val, GOBS obs1, GOBS obs2)
    {

        _beg = beg;
        _end = end;

        _gobs = obs1;
        _ref = obs2;

        _val = val;

        return;
    }

    void t_gbias::set(double val, GOBS obs1, GOBS obs2)
    {

        _gobs = obs1;
        _ref = obs2;

        _val = val;

        return;
    }

    bool t_gbias::valid(const t_gtime& epo)
    {

        bool ret = true;
        ;

        if (epo < _beg || epo > _end)
        {
            ret = false;
        }
        else
        {
            ret = true;
        }

        return ret;
    }

} // namespace gnut
