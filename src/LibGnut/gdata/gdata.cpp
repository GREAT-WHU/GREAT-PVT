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

using namespace std;

namespace gnut
{
    t_gdata::t_gdata() : _type(NONE),
                         _group(GRP_NONE)
    {
        _gnote = nullptr;
    }

    t_gdata::t_gdata(t_spdlog spdlog) : _type(NONE),
                                        _group(GRP_NONE)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }
        _gnote = nullptr;
    }

    t_gdata::t_gdata(const t_gdata &data)
    {
        _spdlog = data.spdlog();
        _gnote = data.gnote();
        _type = data.id_type();
        _group = data.id_group();
    }

    t_gdata::~t_gdata()
    {
    }

    t_gdata &t_gdata::operator=(const t_gdata &data)
    {
        _spdlog = data.spdlog();
        _gnote = data.gnote();
        _type = data.id_type();
        _group = data.id_group();
        return *this;
    }

    int t_gdata::id_type(const ID_TYPE &t)
    {
        unsigned int last = LAST;
        for (unsigned int i = 0; i < last; ++i)
        {
            if (t == ID_TYPE(i))
            {
                return _type = t;
            }
        }

        if (_spdlog)
            SPDLOG_LOGGER_ERROR(_spdlog, "Unknown data id. Reset to nullptr");
        _type = t_gdata::NONE;
        return 0;
    }

    int t_gdata::id_group(const ID_GROUP &g)
    {
        unsigned int last = GRP_LAST;
        for (unsigned int i = 0; i < last; ++i)
        {
            if (g == ID_GROUP(i))
            {
                return _group = g;
            }
        }
        if (_spdlog)
            SPDLOG_LOGGER_ERROR(_spdlog, "Unknown group type. Reset to nullptr");
        _group = t_gdata::GRP_NONE;
        return 0;
    }

    string t_gdata::type2str(ID_TYPE type)
    {
        string str = "";

        switch (type)
        {
        case NONE:
            str = "NONE";
            break;
        case OBS:
            str = "OBS";
            break;
        case OBSGNSS:
            str = "OBSGNSS";
            break;
        case EPH:
            str = "EPH";
            break;
        case EPHGPS:
            str = "EPHGPS";
            break;
        case EPHGLO:
            str = "EPHGLO";
            break;
        case EPHGAL:
            str = "EPHGAL";
            break;
        case EPHQZS:
            str = "EPHQZS";
            break;
        case EPHBDS:
            str = "EPHBDS";
            break;
        case EPHIRN:
            str = "EPHIRN";
            break;
        case EPHSBS:
            str = "EPHSBS";
            break;
        case EPHPREC:
            str = "EPHPREC";
            break;

        case ALLNAV:
            str = "ALLNAV";
            break;
        case ALLPREC:
            str = "ALLPREC";
            break;
        case ALLOBS:
            str = "ALLOBS";
            break;
        case ALLOBJ:
            str = "ALLOBJ";
            break;
        case ALLPCV:
            str = "ALLPCV";
            break;
        case ALLOTL:
            str = "ALLOTL";
            break;
        case POS:
            str = "POS";
            break;
        case PCV:
            str = "PCV";
            break;
        case OTL:
            str = "OTL";
            break;
        case BIAS:
            str = "BIAS";
            break;
        case RSSIMAP:
            str = "RSSIMAP";
            break;
        case LCI_POS:
            str = "LCI_POS";
            break;
        case LAST:
            str = "UNDEF";
            break;
        default:
            str = "UNDEF";
        }
        return str;
    }

    string t_gdata::str_group() const
    {
        string group;
        switch (_group)
        {
        case GRP_NONE:
            group = "GRP_NONE";
            break;
        case GRP_OBSERV:
            group = "GRP_OBSERV";
            break;
        case GRP_EPHEM:
            group = "GRP_EPHEM";
            break;
        case GRP_PRODUCT:
            group = "GRP_PRODUCT";
            break;
        case GRP_MODEL:
            group = "GRP_MODEL";
            break;
        case GRP_LAST:
            group = "GRP_UNDEF";
            break;
        default:
            group = "GRP_UNDEF";
        }
        return group;
    }

    void t_gdata::spdlog(t_spdlog spdlog)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }
    }


} // namespace
