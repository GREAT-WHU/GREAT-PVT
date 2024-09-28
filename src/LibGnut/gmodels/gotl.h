
/**
*
* @verbatim
    History
    2013-03-29  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gotl.h
* @brief      Purpose: implementation of ocean tide loading
*.
* @author     PV
* @version    1.0.0
* @date       2013-03-29
*
*/

#ifndef GOTL_H
#define GOTL_H

#include <string>

#include "newmat/newmat.h"

#include "gdata/gdata.h"
#include "gutils/gtime.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_gotl based t_gdata. */
    class t_gotl : public t_gdata
    {

    public:
        /** @brief default constructor. */
        t_gotl();

        t_gotl(t_spdlog spdlog);
        /** @brief default destructor. */
        ~t_gotl();

        /** @brief get the site. */
        string site();

        /** @brief get the latitude. */
        double lat();

        /** @brief get the lontitude. */
        double lon();

        /** @brief get the data. */
        Matrix data();

        /** @brief set the data. */
        void setdata(const string &site, const double &lon, const double &lat, const Matrix &data);

    private:
        string _site; ///< site
        Matrix _data; ///< data
        double _lat;  ///< latitude
        double _lon;  ///< lontitude
    };

} // namespace

#endif
