/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include "gdata/gdata.h"


namespace gnut
{
    t_gdata::t_gdata() :
        _type(ID_TYPE::NONE),
        _group(ID_GROUP::GRP_NONE)
    {
    }

    t_gdata::t_gdata(const t_gdata& data)
    {
        _type = data.id_type();
        _group = data.id_group();
    }

    t_gdata::~t_gdata()
    {
    }

    t_gdata& t_gdata::operator=(const t_gdata& data)
    {
        _type = data.id_type();
        _group = data.id_group();
        return *this;
    }

    int t_gdata::id_type(const ID_TYPE& t)
    {
        unsigned int last = static_cast<unsigned int>(ID_TYPE::LAST);
        for (unsigned int i = 0; i < last; ++i)
        {
            if (t == static_cast<ID_TYPE>(i))
            {
                return static_cast<int>(_type = t);
            }
        }

        GREAT_ERROR("Unknown data id. Reset to nullptr");
        _type = t_gdata::ID_TYPE::NONE;
        return 0;
    }

    int t_gdata::id_group(const ID_GROUP& g)
    {
        unsigned int last = static_cast<unsigned int>(ID_GROUP::GRP_LAST);
        for (unsigned int i = 0; i < last; ++i)
        {
            if (g == static_cast<ID_GROUP>(i))
            {
                return static_cast<int>(_group = g);
            }
        }
        GREAT_ERROR("Unknown group type. Reset to nullptr");
        _group = t_gdata::ID_GROUP::GRP_NONE;
        return 0;
    }

    std::string t_gdata::type2str(ID_TYPE type)
    {
        std::string str = "";

        switch (type)
        {
            case ID_TYPE::NONE:
                str = "NONE";
                break;
            case ID_TYPE::OBS:
                str = "OBS";
                break;
            case ID_TYPE::OBSGNSS:
                str = "OBSGNSS";
                break;
            case ID_TYPE::EPH:
                str = "EPH";
                break;
            case ID_TYPE::EPHGPS:
                str = "EPHGPS";
                break;
            case ID_TYPE::EPHGLO:
                str = "EPHGLO";
                break;
            case ID_TYPE::EPHGAL:
                str = "EPHGAL";
                break;
            case ID_TYPE::EPHQZS:
                str = "EPHQZS";
                break;
            case ID_TYPE::EPHBDS:
                str = "EPHBDS";
                break;
            case ID_TYPE::EPHIRN:
                str = "EPHIRN";
                break;
            case ID_TYPE::EPHSBS:
                str = "EPHSBS";
                break;
            case ID_TYPE::EPHPREC:
                str = "EPHPREC";
                break;

            case ID_TYPE::ALLNAV:
                str = "ALLNAV";
                break;
            case ID_TYPE::ALLPREC:
                str = "ALLPREC";
                break;
            case ID_TYPE::ALLOBS:
                str = "ALLOBS";
                break;
            case ID_TYPE::ALLOBJ:
                str = "ALLOBJ";
                break;
            case ID_TYPE::ALLPCV:
                str = "ALLPCV";
                break;
            case ID_TYPE::ALLOTL:
                str = "ALLOTL";
                break;
            case ID_TYPE::POS:
                str = "POS";
                break;
            case ID_TYPE::PCV:
                str = "PCV";
                break;
            case ID_TYPE::OTL:
                str = "OTL";
                break;
            case ID_TYPE::BIAS:
                str = "BIAS";
                break;
            case ID_TYPE::RSSIMAP:
                str = "RSSIMAP";
                break;
            case ID_TYPE::LCI_POS:
                str = "LCI_POS";
                break;
            case ID_TYPE::LAST:
                str = "UNDEF";
                break;
            default:
                str = "UNDEF";
        }
        return str;
    }

    std::string t_gdata::str_group() const
    {
        std::string group;
        switch (_group)
        {
            case ID_GROUP::GRP_NONE:
                group = "GRP_NONE";
                break;
            case ID_GROUP::GRP_OBSERV:
                group = "GRP_OBSERV";
                break;
            case ID_GROUP::GRP_EPHEM:
                group = "GRP_EPHEM";
                break;
            case ID_GROUP::GRP_PRODUCT:
                group = "GRP_PRODUCT";
                break;
            case ID_GROUP::GRP_MODEL:
                group = "GRP_MODEL";
                break;
            case ID_GROUP::GRP_LAST:
                group = "GRP_UNDEF";
                break;
            default:
                group = "GRP_UNDEF";
        }
        return group;
    }

} // namespace gnut
