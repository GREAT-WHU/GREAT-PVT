
/**
*
* @verbatim
    History
    2011-01-10  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gconst.h
* @brief       Purpose: defines constants
* @author      JD
* @version     1.0.0
* @date        2011-01-10
*
*/

#ifndef CONST_H
#define CONST_H

#include <map>
#include <string>
#include <vector>
#include "gexport/ExportLibGnut.h"

using namespace std;

namespace gnut
{
// function
// get min
#define MIN(x, y) ((x) < (y) ? (x) : (y))

// pow(2)
#define SQR(x) ((x) * (x))

// sqrt with zero
#define SQRT(x) ((x) <= 0.0 ? 0.0 : sqrt(x))

#define SGN(x) ((x) <= 0.0 ? -1.0 : 1.0)
#define SWAP_I(x, y) \
    do               \
    {                \
        int _z = x;  \
        x = y;       \
        y = _z;      \
    } while (0)
#define SWAP_D(x, y)   \
    do                 \
    {                  \
        double _z = x; \
        x = y;         \
        y = _z;        \
    } while (0)
#define ROUND(x) ((int)floor((x) + 0.5))
#define ROUND_U(x) ((unsigned int)floor((x) + 0.5))

// math
#ifndef G_PI
#define G_PI 3.14159265358979311599796346854419e0 ///< pi
#endif

#ifndef D2R
#define D2R (G_PI / 180.0) ///< deg to rad
#endif

#ifndef R2D
#define R2D (180.0 / G_PI) ///< rad to deg
#endif

#ifndef R2AS
#define R2AS (180.0 * 3600.0 / G_PI)
#endif

#ifndef SEC2RAD
#define SEC2RAD (D2R / 3600.0) ///< sec to rad
#endif

#ifndef RAD2SEC
#define RAD2SEC (R2D * 3600.0) ///< rad to sec
#endif

#ifndef RAD2TSEC
#define RAD2TSEC (RAD2SEC / 15.0)
#endif

#ifndef RHO_SEC
#define RHO_SEC 3600.0 * D2R
#endif

#ifndef CLIGHT
#define CLIGHT 2.99792458e+8 ///< speed of light [m/s]
#endif

#ifndef CLIGHT_2
#define CLIGHT_2 8.9875517873681764e16 ///< pow(CLIGTH,2)
#endif

#ifndef OMEGA
#define OMEGA 7292115.1467e-11
#endif

#ifndef Aell
#define Aell 6378137.000 ///< semi major axes [m]
#endif

#ifndef Finv
#define Finv 298.257223563 ///< inverse flattening [-]
#endif

#ifndef MJD_J2000
#define MJD_J2000 51544.5 ///< J2000 MJD date
#endif

#ifndef R_SPHERE
#define R_SPHERE 6371000.000 ///< [m] approx. sphere radius
#endif

#ifndef POST_SHADOW
#define POST_SHADOW 1800.0 ///< satellite post shadow period [s]
#endif

#define P2_5 0.03125                /* 2^-5 */
#define P2_6 0.015625               /* 2^-6 */
#define P2_10 0.0009765625          /* 2^-10 */
#define P2_11 4.882812500000000E-04 /* 2^-11 */
#define P2_15 3.051757812500000E-05 /* 2^-15 */
#define P2_17 7.629394531250000E-06 /* 2^-17 */
#define P2_19 1.907348632812500E-06 /* 2^-19 */
#define P2_20 9.536743164062500E-07 /* 2^-20 */
#define P2_21 4.768371582031250E-07 /* 2^-21 */
#define P2_23 1.192092895507810E-07 /* 2^-23 */
#define P2_24 5.960464477539063E-08 /* 2^-24 */
#define P2_27 7.450580596923828E-09 /* 2^-27 */
#define P2_29 1.862645149230957E-09 /* 2^-29 */
#define P2_30 9.313225746154785E-10 /* 2^-30 */
#define P2_31 4.656612873077393E-10 /* 2^-31 */
#define P2_32 2.328306436538696E-10 /* 2^-32 */
#define P2_33 1.164153218269348E-10 /* 2^-33 */
#define P2_34 5.820766091346740E-11 /* 2^-34 */
#define P2_35 2.910383045673370E-11 /* 2^-35 */
#define P2_38 3.637978807091710E-12 /* 2^-38 */
#define P2_39 1.818989403545856E-12 /* 2^-39 */
#define P2_40 9.094947017729280E-13 /* 2^-40 */
#define P2_43 1.136868377216160E-13 /* 2^-43 */
#define P2_46 1.421085471520200E-14 /* 2^-46 */
#define P2_48 3.552713678800501E-15 /* 2^-48 */
#define P2_50 8.881784197001252E-16 /* 2^-50 */
#define P2_55 2.775557561562891E-17 /* 2^-55 */
#define P2_59 1.734723475976810E-18 /* 2^-59 */

    // GPS
#define A_WGS 6378137.000         ///< [m] WGS84 semi-major axis
#define B_WGS 6356752.300         ///< [m] WGS84 semi-minor axis
#define E_WGS 0.081819            ///< [-] WGS84 eccentricity
#define F_WGS 0.003352811         ///< [-] WGS84 flatenning
#define MUDOT_GPS (0.00836 * D2R) ///< avarage GPS satellite angular velocity [rad]
#define EPS0_GPS (13.5 * D2R)     ///< maximal GPS satellites crossing angle [deg]

// GLONASS
#define GM_PZ90 398.60044e12      ///< [] PZ90 earth's graviational constant
#define Aell_PZ90 6378136.000     ///< [m] PZ90 semi-major axes
#define C20_PZ90 -1082.62575e-6   ///< [] PZ90
#define MUDOT_GLO (0.00888 * D2R) ///< avarage GLO satellite angular velocity [rad]
#define EPS0_GLO (14.2 * D2R)     ///< maximal GLO satellites crossing angle [deg]
#define Finv_GLO 298.25784        ///< inverse flattening [-]
#define OMGE_DOT_GLO 7.292115e-5  ///< Mean angular velocity of the Earth [rad/sec]

// Galileo
#define GM_GAL 3.986004418e14        ///< Geocentric gravitational constant [m^3/s^2]
#define OMGE_DOT_GAL 7.2921151467e-5 ///< Mean angular velocity of the Earth [rad/sec]

// BeiDou
#define GM_CGCS 3.986004418e14    ///< []  CGCS2000 earth's graviational constant
#define OMGE_DOT_BDS 7.2921150e-5 ///< BDS value of the earth's rotation rate [rad/sec]//
#define Aell_CGCS 6378137.000     ///< [m] CGCS2000 semi-major axes

// METEO
#define G_RATIO 0.003449787 ///< [-] gravity ratio
#define G_EQUA 9.7803253359 ///< [m/s2] equatorial gravity
#define G_POLE 9.8321849378 ///< [m/s2] polar gravity
#define G_WMO 9.80665       ///< [m/s2] gravity acceleration (WMO constant for lat=45)
#define KS_SOM 0.001931853  ///< [-] Somigliana's constant
#define Md 28.9644          ///< [g/mol] molar mass (mean molecular weight) of dry air
#define Mw 18.01528         ///< [g/mol] molar mass (mean molecular weight) of water
#define Ru 8.3144621        ///< [J/mol/K] universal gas constant
#define Rd 287.058          ///< [J/kg/K] spec. gas constant for Dry Air (Rd = 287.058)  Ru/(Md/1000)
#define Rv 461.495          ///< [J/kg/K] spec. gas constant for Wat Vap (Rv = 461.495)  Ru/(Mw/1000)
#define Eps 0.62198         ///< [-] molecular weight ratio (Mw/Md)     (Eps = 0.62198)
#define TC2TK 273.15        ///< conversion from deg of Celsius to Kelvins
#define TPOINT 273.16       ///< temperature of triple point [K], i.e. 0.01 Deg C

    typedef map<string, vector<double>> t_map_refr; ///< refractivity coefficients

#define K1_ESS 77.64   ///< [K/hPa]   Essen and Froome (1951)
#define K2_ESS 64.68   ///< [K/hPa]   Essen and Froome (1951)
#define K3_ESS 3.718e5 ///< [K^2/hPa] Essen and Froome (1951)

#define K1_SMW 77.607  ///< [K/hPa]   Smith and Weintraub (1953) // FULL RESOLUTION
#define K2_SMW 71.6    ///< [K/hPa]   Smith and Weintraub (1953) // original paper 3-par formula!
#define K3_SMW 3.747e5 ///< [K^2/hPa] Smith and Weintraub (1953) // original paper 3-par formula!

#define K1_THA 77.604  ///< [K/hPa]   Thayer (1974)
#define K2_THA 64.79   ///< [K/hPa]   Thayer (1974)
#define K3_THA 3.776e5 ///< [K^2/hPa] Thayer (1974)

#define K1_BEV 77.6    ///< [K/hPa]   Bevis (1994)
#define K2_BEV 70.4    ///< [K/hPa]   Bevis (1994)
#define K3_BEV 3.739e5 ///< [K^2/hPa] Bevis (1994)

#define K1_FOE 77.65   ///< [K/hPa]   Foelsche (1999)
#define K2_FOE 65.99   ///< [K/hPa]   Foelsche (1999)
#define K3_FOE 3.777e5 ///< [K^2/hPa] Foelsche (1999)

#define K1_RUE 77.6848   ///< [K/hPa]   Rueger (2002)
#define K2_RUE 71.2952   ///< [K/hPa]   Rueger (2002)
#define K3_RUE 3.75463e5 ///< [K^2/hPa] Rueger (2002)

// for calulate the ocean tide and else
#ifndef EARTH_GM
#define EARTH_GM 398600.4415 ///< GM of Earth
#endif                       // !EARTH_GM

#ifndef EARTH_R
#define EARTH_R 6378.13646 ///< radius of Earth(unit: km)
#endif

#ifndef HP
#define HP sqrt(8 * G_PI / 15) * OMEGA *OMEGA *EARTH_R *EARTH_R *EARTH_R *EARTH_R * 1e12 / EARTH_GM / 1e9
#endif

#ifndef K_OPL
#define K_OPL 4 * G_PI * 6.67428e-11 * EARTH_R * 1000 * 1025 * HP / 3 / 9.7803278
#endif

#ifndef SUN_GM
#define SUN_GM 1.32712440e+11 ///< GM of Sun
#endif                        // !SUN_GM

#ifndef SUN_R
#define SUN_R 696000.0 ///< radius of Sun(unit: km)
#endif                 // !SUN_R

#ifndef MOON_GM
#define MOON_GM 4902.7989 ///< GM of Moon
#endif                    // !MOON_GM

#ifndef MOON_R
#define MOON_R 1738.090 ///< radius of Moon(unit: km)
#endif                  // !MOON_R

} // namespace

#endif
