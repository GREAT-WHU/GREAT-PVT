/**
 * @file         gtrs2crs.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        calculate the rotation matrix from TRS to CRS
 * @version      1.0
 * @date         2024-08-29
 *
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 *
 */
#ifndef GTRS2CRS_H
#define GTRS2CRS_H
#include "gexport/ExportLibGREAT.h"
#include <string>

#include "gutils/gtime.h"
#include "gdata/gpoleut1.h"
#include "gutils/gconst.h"
#include "gutils/gtriple.h"
#include "newmat/newmat.h"

using namespace std;

namespace great
{
    /**
     *@brief       Class for storaging pole and ut1 data for one day
     */
    class t_pudaily
    {
    public:
        /** @brief default constructor. */
        t_pudaily();

        /** @brief default destructor. */
        virtual ~t_pudaily() {};

        t_gtime time; ///< epoch time
        double xpole; ///< x pole
        double ypole; ///< y pole
        double ut1;   ///< dut1
    };

    /**
     *@brief       Class for zonaltide data
     */
    class t_zonaltide
    {
    public:
        /** @brief default constructor. */
        t_zonaltide();

        /** @brief default destructor. */
        virtual ~t_zonaltide() {};

        t_gtime time; ///< epoch time
        double ut1;   ///< dut1
    };

    /**
     *@brief       Class for calculating the rotation matrix from TRS to CRS
     */
    class LibGREAT_LIBRARY_EXPORT t_gtrs2crs
    {
    public:
        /** @brief default constructor. */
        t_gtrs2crs();

        /** @brief constructor. */
        t_gtrs2crs(string cver);

        /** @brief constructor. */
        t_gtrs2crs(bool erptab, t_gpoleut1* poleut1);

        /**
         * @brief Construct a new t gtrs2crs object
         * @param[in]  erptab    if ERP table is used or not
         * @param[in]  poleut1   Earth orientation parameters
         * @param[in]  cver      IAU version: 00(default) or 06
         */
        t_gtrs2crs(bool erptab, t_gpoleut1* poleut1, string cver);

        /**
         * @brief define = for t_gtrs2crs
         * @param[in]  Other     other t_gtrs2crs
         * @return t_gtrs2crs& =
         */
        t_gtrs2crs& operator=(const t_gtrs2crs& Other);

        /** @brief default destructor. */
        virtual ~t_gtrs2crs() {};

        /** @brief calculate the rotation matrix using the new CIO based method. */
        void calcRotMat(const t_gtime& epoch);

        /** @brief return rotation matrix. */
        Matrix& getRotMat();

        /** @brief return xpole. */
        double getXpole();
        /** @brief return ypole. */
        double getYpole();

    protected:
        /** @brief interpolate short term tidal corrections(fortran: polut1_ray_interpolation). */
        void _tide_corrections(t_gtime& t, t_gtriple& xyu);

        /** @brief calculate short term tidal corrections(fortran: polut1_ray_calc). */
        t_pudaily& _tideCor1Cal(t_pudaily& b);

        /** @brief calculate the diurnal and semi-diurnal variations in Earth Orientation Parameters (x,y, UT1, LOD)
         * from ocean tides (fortran: interp.f). */
        /** agree with _ORTHO_EOP at the level of a few microarcseconds in polar motion and a few tenths of a
         * microsecond in UT1*/
        /** create by yqyuan from interp.f 20210829; calculate both UT1 and LOD; LOD not outputed antway */
        void _PMUT1_OCEANS(t_gtime& t, t_gtriple& eop);

        /** @brief evaluate the model of polar motion for a nonrigid Earth due to tidal gravitation. */
        void _PMSDNUT2(t_gtime& t, double* pm);

        /** @brief evaluate the model of subdiurnal libration in the axial component of rotation, expressed by UT1 and
         * LOD.. */
        void _UTLIBR(t_gtime& t, double* temp);

        /** @brief calculate zonaltide correction on ut1(fortran: ut1_zonaltide_interpolation). */
        double _tideCor2(const double& dRmjd);

        /** @brief evaluate the effects of zonal Earth tides on the rotation of the Earth. */
        void _RG_ZONT2(const double& dT, double* DUT, double* DLOD, double* DOMEGA);

        /** @brief calculate fund arg. */
        void _FUNDARG(const double& T, double* L, double* LP, double* F, double* D, double* OM);
        /** @brief Mean anomaly of the Moon. */
        double _iauFal03(const double& t);
        /** @brief Mean anomaly of the Sun. */
        double _iauFalp03(const double& t);
        /** @brief Mean argument of the latitude of the Moon. */
        double _iauFaf03(const double& t);
        /** @brief Mean elongation of the Moon from the Sun. */
        double _iauFad03(const double& t);
        /** @brief Mean longitude of the ascending node of the Moon. */
        double _iauFaom03(const double& t);
        /** @brief Planetary longitudes, Mercury. */
        double _iauFame03(const double& t);
        /** @brief Planetary longitudes, Venus. */
        double _iauFave03(const double& t);
        /** @brief Planetary longitudes, Earth. */
        double _iauFae03(const double& t);
        /** @brief Planetary longitudes, Mars. */
        double _iauFama03(const double& t);
        /** @brief Planetary longitudes, Jupiter. */
        double _iauFaju03(const double& t);
        /** @brief Planetary longitudes, Saturn. */
        double _iauFasa03(const double& t);
        /** @brief Planetary longitudes, Uranus. */
        double _iauFaur03(const double& t);
        /** @brief Planetary longitudes, Neptune. */
        double _iauFane03(const double& t);
        /** @brief General accumulated precession in longitude. */
        double _iauFapa03(const double& t);

        /** @brief interpolate pole and UT1 data. */
        void _calPoleut1(t_gtime& t, double* x, t_gpoleut1* poleut1);

        /** @brief interpolation. */
        double _interpolation(const int& iOrder, const int& iPoint, double* pdX, double* pdY, const double& dXin);

        // inner variable
        bool _erptab = false; ///< if ERP table is used or not
        t_gtime _tdt;         ///< dynamic time

        // out variable
        double _xpole = 0.0; ///< x pole of pole shift
        double _ypole = 0.0; ///< y pole of pole shift
        Matrix _rotmat;      ///< rotation matrix from TRS to CRS

        t_gpoleut1* _poleut1; ///< poleut1 data

        // constant values
        const double d2PI = 6.283185307179586476925287; ///< DOUBLE PRECISION DJ0
        const double dJ0 = 51544.5;                     ///< Reference epoch (J2000), JD

        const double dAS2R = 4.848136811095359935899141 * 0.000001; ///<         *  Arcseconds to radians
        const double dJC = 36525;                                   ///<         *  Days per Julian century
        const double dTURNAS = 1296000e0;

        const double rmjd0 = 51544.5;

        // IAU2000A or IAU2006/2000A; added by yqyuan 2021.05.10
        string _cver; ///< IAU version: 00(default) or 06
    };
} // namespace great

#endif
