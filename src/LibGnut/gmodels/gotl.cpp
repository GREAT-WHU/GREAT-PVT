
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include "gmodels/gotl.h"

namespace gnut
{

    t_gotl::t_gotl() :
        t_gdata()
    {
        id_type(t_gdata::ID_TYPE::OTL);
    }

    t_gotl::~t_gotl()
    {
    }

    std::string t_gotl::site()
    {
        std::string tmp = _site;
        return tmp;
    }

    Eigen::MatrixXd t_gotl::data()
    {
        Eigen::MatrixXd tmp = _data;
        return tmp;
    }

    double t_gotl::lat()
    {
        double tmp = _lat;
        return tmp;
    }

    double t_gotl::lon()
    {
        double tmp = _lon;
        return tmp;
    }

    void t_gotl::setdata(const std::string& site, const double& lon, const double& lat, const Eigen::MatrixXd& data)
    {
        _site = site;
        _lon = lon;
        _lat = lat;
        _data = data;
        return;
    }

} // namespace gnut
