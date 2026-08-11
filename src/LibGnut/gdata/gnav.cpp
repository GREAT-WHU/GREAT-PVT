/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <stdlib.h>
#include <iostream>

#include "gdata/gnav.h"


namespace gnut
{

    t_gnav::t_gnav() :
        t_geph()
    {
        id_type(t_gdata::ID_TYPE::EPH);
        id_group(t_gdata::ID_GROUP::GRP_EPHEM);
    }

    t_gnav::~t_gnav()
    {
    }

    int t_gnav::nav_validity(GSYS gs)
    {
        switch (gs)
        {
            case GSYS::GPS:
                return MAX_GPS_TIMEDIFF;
            case GSYS::GLO:
                return MAX_GLO_TIMEDIFF;
            case GSYS::GAL:
                return MAX_GAL_TIMEDIFF;
            case GSYS::BDS:
                return MAX_BDS_TIMEDIFF;
            case GSYS::QZS:
                return MAX_QZS_TIMEDIFF;
            case GSYS::SBS:
                return MAX_SBS_TIMEDIFF;
            case GSYS::IRN:
                return MAX_IRN_TIMEDIFF;
            case GSYS::GNS:
                return MAX_NAV_TIMEDIFF;
            default:
                return MAX_NAV_TIMEDIFF;
        }
        return MAX_NAV_TIMEDIFF;
    }

    bool t_gnav::healthy() const
    {

        bool tmp = this->_healthy();

        return tmp;
    }

} // namespace gnut
