
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
 
  (c) 2011-2017 Geodetic Observatory Pecny, http://www.pecny.cz (gnss@pecny.cz)
      Research Institute of Geodesy, Topography and Cartography
      Ondrejov 244, 251 65, Czech Republic

  This file is part of the G-Nut C++ library.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3 of
  the License, or (at your option) any later version.
 
  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses>.

-*/

#include "gutils/gmatrixconv.h"
#include "gutils/gconst.h"
#include "gutils/gtypeconv.h"
#include "gmodels/geop80.h"

using namespace std;

namespace gnut
{
    t_geop80::t_geop80()
    {
    }

    t_geop80::~t_geop80()
    {
    }

    Matrix t_geop80::nutMatrix(double mjd)
    {
        const double T = (mjd - MJD_J2000) / 36525.0;

        double ls = 2.0 * G_PI * _frac(0.993133 + 99.997306 * T);
        double D = 2.0 * G_PI * _frac(0.827362 + 1236.853087 * T);
        double F = 2.0 * G_PI * _frac(0.259089 + 1342.227826 * T);
        double N = 2.0 * G_PI * _frac(0.347346 - 5.372447 * T);

        double dpsi = (-17.200 * sin(N) - 1.319 * sin(2 * (F - D + N)) - 0.227 * sin(2 * (F + N)) + 0.206 * sin(2 * N) + 0.143 * sin(ls)) / RHO_SEC;
        double deps = (+9.203 * cos(N) + 0.574 * cos(2 * (F - D + N)) + 0.098 * cos(2 * (F + N)) - 0.090 * cos(2 * N)) / RHO_SEC;

        double eps = 0.4090928 - 2.2696E-4 * T;

        return rotX(-eps - deps) * rotZ(-dpsi) * rotX(+eps);
    }

    // Precession Matrix (Precession theory 1976)
    // ------------------------------------------
    Matrix t_geop80::precMatrix(double mjd)
    {
        // Time interval betwen epoch J2000 and start epoch (in Julian centuries)
        const double T = (mjd - MJD_J2000) / 36525.0;

        // Euler angles
        double zeta = T * (2306.2181 + T * (0.30188 + T * 0.017998)) * SEC2RAD;
        double theta = T * (2004.3109 + T * (-0.42665 - T * 0.041833)) * SEC2RAD;
        double z = T * (2306.2181 + T * (1.09468 + T * 0.018203)) * SEC2RAD;

        return rotZ(-z) * rotY(theta) * rotZ(-zeta);
    }

    double t_geop80::_frac(double x)
    {
        return x - floor(x);
    }

} // namespace
