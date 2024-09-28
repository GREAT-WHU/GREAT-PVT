
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
 
-*/

#include "gmodels/gotl.h"

namespace gnut
{

    t_gotl::t_gotl() : t_gdata()
    {
        id_type(t_gdata::OTL);
    }

    t_gotl::t_gotl(t_spdlog spdlog) : t_gdata(spdlog)
    {
        id_type(t_gdata::OTL);
    }

    t_gotl::~t_gotl()
    {
    }

    string t_gotl::site()
    {
        _gmutex.lock();
        string tmp = _site;
        _gmutex.unlock();
        return tmp;
    }

    Matrix t_gotl::data()
    {
        _gmutex.lock();
        Matrix tmp = _data;
        _gmutex.unlock();
        return tmp;
    }

    double t_gotl::lat()
    {
        _gmutex.lock();
        double tmp = _lat;
        _gmutex.unlock();
        return tmp;
    }

    double t_gotl::lon()
    {
        _gmutex.lock();
        double tmp = _lon;
        _gmutex.unlock();
        return tmp;
    }

    void t_gotl::setdata(const string &site, const double &lon, const double &lat, const Matrix &data)
    {
        _gmutex.lock();
        _site = site;
        _lon = lon;
        _lat = lat;
        _data = data;
        _gmutex.unlock();
        return;
    }

} // namespace
