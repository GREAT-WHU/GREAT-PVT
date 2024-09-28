
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
 
-*/

#include "gprod/gprod.h"

using namespace std;

namespace gnut
{
    t_gprod::t_gprod(const t_gtime &t, shared_ptr<t_gobj> pt)
        : t_gdata(),
          _epo(t),
          _obj(pt)
    {
        id_group(GRP_PRODUCT);
        id_type(NONE);
    }

    t_gprod::t_gprod(t_spdlog spdlog, const t_gtime &t, shared_ptr<t_gobj> pt)
        : t_gdata(spdlog),
          _epo(t),
          _obj(pt)
    {
        id_group(GRP_PRODUCT);
        id_type(NONE);
    }

    t_gprod::~t_gprod()
    {
    }

    int t_gprod::set_val(const string &str, const double &val, const double &rms)
    {
        _prod[str] = make_pair(val, rms);
        return 0;
    }

} // namespace
