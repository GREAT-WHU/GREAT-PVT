/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include "gall/gallotl.h"
#include "gutils/gconst.h"
#include <math.h>
namespace gnut
{
    t_gallotl::t_gallotl() : t_gdata()
    {
        id_type(t_gdata::ALLOTL);
    }
    t_gallotl::t_gallotl(t_spdlog spdlog) : t_gdata(spdlog)
    {
        id_type(t_gdata::ALLOTL);
    }

    t_gallotl::~t_gallotl()
    {
        _mapotl.clear();
    }

    int t_gallotl::data(Matrix &otldata, const string &site)
    {
        _gmutex.lock();

        if (_mapotl.find(site) == _mapotl.end() || _mapotl.size() == 0)
        {
            string site_short = site.substr(0, 4);
            if (_mapotl.find(site_short) == _mapotl.end() || _mapotl.size() == 0)
            {
                _gmutex.unlock();
                return -1;
            }
            else
                otldata = _mapotl[site_short].data();
        }
        else
            otldata = _mapotl[site].data();

        _gmutex.unlock();
        return 1;
    }

    int t_gallotl::data(Matrix &otldata, double lon, double lat)
    {
        const double dlon_eps = 1E4 / 6378235.0 * R2D, dlat_eps = dlon_eps / cos(lat * D2R);

        if (lon > 180)
            lon -= 360.0;

        for (auto otl_iter = _mapotl.begin(); otl_iter != _mapotl.end(); otl_iter++)
        {
            double dlon, dlat;
            dlon = fabs(otl_iter->second.lon() - lon);
            dlat = fabs(otl_iter->second.lat() - lat);
            if (dlon < dlon_eps && dlat < dlat_eps)
            {
                otldata = otl_iter->second.data();
                return 1;
            }
        }

        return -1;
    }

    void t_gallotl::add(t_gotl &otl)
    {
        _gmutex.lock();

        string site = otl.site();
        _mapotl[site] = otl;

        _gmutex.unlock();
        return;
    }

} // namespace
