
/**
*
* @verbatim
Reference:
  J. Boehm, R. Heinkelmann, H. Schuh, Short Note: A Global Model of Pressure
  and Temperature for Geodetic Applications, Journal of Geodesy,
  doi:10.1007/s00190-007-0135-3, 2007.

  input data
  ----------
  dmjd: modified julian date
  dlat: ellipsoidal latitude in radians
  dlon: longitude in radians
  dhgt: ellipsoidal height in m

  output data
  -----------
  pres: pressure in hPa
  temp: temperature in Celsius
  undu: Geoid undulation in m (from a 9x9 EGM based model)

    History
    2011-11-28  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       ggpt.h
* @brief      Purpose: Global Pressure Temperature model
* @author     JD
* @version    1.0.0
* @date       2011-11-28
*
*/

#ifndef GGPT_H
#define GGPT_H

#include <iostream>
#include <string.h>
#include <math.h>
#include "gexport/ExportLibGnut.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_gpt. */
    class LibGnut_LIBRARY_EXPORT t_gpt
    {

    public:
        /** @brief default constructor. */
        t_gpt(){};

        /** @brief default destructor. */
        ~t_gpt(){};

        /**
        *@brief       GPT empirical model v1
        * dlat, dlon --> RADIANS !
        */
        int gpt_v1(double dmjd, double dlat, double dlon, double dhgt,
                   double &pres, double &temp, double &undu);

    protected:
        static double a_geoid[55];
        static double b_geoid[55];
        static double ap_mean[55];
        static double bp_mean[55];
        static double ap_amp[55];
        static double bp_amp[55];
        static double at_mean[55];
        static double bt_mean[55];
        static double at_amp[55];
        static double bt_amp[55];

    private:
    };
}

#endif
