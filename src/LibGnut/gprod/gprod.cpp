
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include "gprod/gprod.h"


namespace gnut
{
    t_gprod::t_gprod(const t_gtime& t, std::shared_ptr<t_gobj> pt) :
        t_gdata(),
        _epo(t),
        _obj(pt)
    {
        id_group(ID_GROUP::GRP_PRODUCT);
        id_type(ID_TYPE::NONE);
    }

    t_gprod::~t_gprod()
    {
    }

    int t_gprod::set_val(const std::string& str, const double& val, const double& rms)
    {
        _prod[str] = std::make_pair(val, rms);
        return 0;
    }

} // namespace gnut
