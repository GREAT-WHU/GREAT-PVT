
/**
* @verbatim
    History
    2011-04-26  PV: created
    2012-04-06  JD: extracted coordinate system conversion utilities only

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gsysconv.h
* @brief       Purpose: reference system conversion utilities
*.
* @author      JD
* @version     1.0.0
* @date        2012-09-26
*
*/

#ifndef GSYSCONV_H
#define GSYSCONV_H

#include <string>

#include "gutils/gtriple.h"
#include "gdata/gsatdata.h"
#include "newmat/newmat.h"
#include "gexport/ExportLibGnut.h"

using namespace std;

namespace gnut
{
    /**@brief Ellipsoidal to XYZ coordinates */
    LibGnut_LIBRARY_EXPORT int ell2xyz(const double *Ell, double *XYZ, bool degrees); // True/False:  LatLon in Degrees/Radians!

    /**@brief XYZ to Ellipsoidal coordinates */
    LibGnut_LIBRARY_EXPORT int xyz2ell(const double *XYZ, double *Ell, bool degrees); // True/False:  LatLon in Degrees/Radians!

    /**@brief XYZ to Topocentric/Local (North, East, Up) coordinates */
    LibGnut_LIBRARY_EXPORT int xyz2neu(const double *XYZ, const double *XYZ_Ref, double *neu);

    /**@brief Topocentric/Local (NEU) to XYZ coordinates */
    LibGnut_LIBRARY_EXPORT int neu2xyz(const double *Ell, const double *neu, double *xyz); // ! Radians only: Ell[0], Ell[1]

    /**@brief rotation matrix for rao2xyz */
    LibGnut_LIBRARY_EXPORT int rao2xyz_rot(const ColumnVector &pos, const ColumnVector &vel, Matrix &R);

    /**@brief Radial/Along-track/Out-of-plane (Satell-system) to XYZ coordinates */
    LibGnut_LIBRARY_EXPORT int rao2xyz(const ColumnVector &pos, const ColumnVector &vel,
                                       const ColumnVector &rao, ColumnVector &xyz);

    /**@brief XYZ coordinates to Radial/Along-track/Out-of-plane (Satell-system) */
    LibGnut_LIBRARY_EXPORT int xyz2rao(const ColumnVector &pos, const ColumnVector &vel,
                                       ColumnVector &xyz, ColumnVector &rao);

    /**@brief ELL coordinates to XYZ coordinates */
    LibGnut_LIBRARY_EXPORT int ell2xyz(const t_gtriple &ell, t_gtriple &xyz, bool degrees);

    /**@brief XYZ coordinates to ELL coordinates */
    LibGnut_LIBRARY_EXPORT int xyz2ell(const t_gtriple &crd, t_gtriple &ell, bool degrees); // True/False:  LatLon in Degrees/Radians!

    /**@brief XYZ coordinates to ELL vlbi */
    LibGnut_LIBRARY_EXPORT int xyz2ell_vlbi(const t_gtriple &crd, t_gtriple &ell);

    /**@brief XYZ coordinates to Topocentric/Local (NEU) */
    LibGnut_LIBRARY_EXPORT void xyz2neu(t_gtriple &ell, t_gtriple &xyz, t_gtriple &neu); // Radians only: Ell[0], Ell[1]

    /**@brief Topocentric/Local (NEU) to XYZ coordinates */
    LibGnut_LIBRARY_EXPORT void neu2xyz(t_gtriple &ell, t_gtriple &neu, t_gtriple &xyz); // Radians only: Ell[0], Ell[1]

    /**@brief XYZ coordinates to Topocentric/Local (NEU) */
    LibGnut_LIBRARY_EXPORT int xyz2neu(t_gtriple &xyz, SymmetricMatrix &Q_xyz, SymmetricMatrix &Q_neu);

    /**@brief ELL coordinates to ipp */
    LibGnut_LIBRARY_EXPORT int ell2ipp(t_gsatdata &satdata, t_gtriple &ell_site, t_gtriple &ell_ipp, bool GPStkflag = false);

    /**@brief rotm */
    LibGnut_LIBRARY_EXPORT Matrix rotm(double angle, int type);

    /**@brief drotm */
    LibGnut_LIBRARY_EXPORT Matrix drotm(double angle, int type);
} // namespace

#endif // GSYSCONV_H
