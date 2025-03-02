/**
 * @file         gtrs2crs.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        calculate the rotation matrix from TRS to CRS and the corresponding partial derivation matrix
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GTRS2CRS_H
#define GTRS2CRS_H
#include "gexport/ExportLibGREAT.h"
#include <vector>

#include "gutils/gtime.h"
#include "gdata/gpoleut1.h"
#include "gutils/gconst.h"
#include "gutils/gtriple.h"
#include "gmodels/ginterp.h"
#include "gutils/gmatrixconv.h"
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
        virtual ~t_pudaily(){};

        t_gtime time; ///< epoch time
        double xpole; ///< x pole
        double ypole; ///< y pole
        double ut1;   ///< dut1
        double dx;    ///< dx pole
        double dy;    ///< dy pole
    };

    /**
     *@brief       Class for nutation for one epoch
     */
    class t_EpochNutation
    {
    public:
        /** @brief default constructor. */
        t_EpochNutation();

        /** @brief default destructor. */
        virtual ~t_EpochNutation(){};

        double T;  ///< epoch
        double Pi; ///< nutation angle of the longitude(radian)
        double Ep; ///< nutation angle of the obliquity(radian)
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
        virtual ~t_zonaltide(){};

        t_gtime time; ///< epoch time
        double ut1;   ///< dut1
        double lod;   ///< dlod
        double omega; ///< omega
    };

    /**
    *@brief       Class for calculating the rotation matrix from TRS to CRS and the corresponding partial derivation matrix
    */
    class LibGREAT_LIBRARY_EXPORT t_gtrs2crs
    {
    public:
        /** @brief default constructor. */
        t_gtrs2crs();

        /** @brief constructor. */
        t_gtrs2crs(string cver);

        /** @brief constructor. */
        t_gtrs2crs(bool erptab, t_gpoleut1 *poleut1);

        /**
         * @brief Construct a new t gtrs2crs object
         * @param[in]  erptab    if ERP table is used or not
         * @param[in]  poleut1   Earth orientation parameters
         * @param[in]  cver      IAU version: 00(default) or 06
         */
        t_gtrs2crs(bool erptab, t_gpoleut1 *poleut1, string cver);

        /**
          * @brief define = for t_gtrs2crs
          * @param[in]  Other     other t_gtrs2crs
          * @return t_gtrs2crs& =
          */
        t_gtrs2crs &operator=(const t_gtrs2crs &Other);

        /** @brief default destructor. */
        virtual ~t_gtrs2crs(){};

        /** @brief calculte process matrix. */
        void calcProcMat(const bool &partial, const int &axis, const double &angle, vector<Matrix> &rot);

        /** @brief calculate the rotation matrix using the new CIO based method. */
        void calcRotMat(const t_gtime &epoch, const bool &ldxdpole, const bool &ldydpole, const bool &ldudpole, const bool &ldX, const bool &ldY);

        /** @brief return rotation matrix. */
        Matrix &getRotMat();

        /** @brief return du matrix. */
        Matrix &getMatDu();

        /** @brief return dx matrix. */
        Matrix &getMatDx();

        /** @brief return dy matrix. */
        Matrix &getMatDy();

        /** @brief return dy matrix. */
        Matrix &getMatDp();

        /** @brief return dy matrix. */
        Matrix &getMatDe();

        /** @brief return xpole. */
        double getXpole();
        /** @brief return ypole. */
        double getYpole();
        /** @brief return gmst. */
        double getGmst();
        /** @brief return now epoch. */
        t_gtime getCurtEpoch();

    protected:
        /** @brief interpolate short term tidal corrections(fortran: polut1_ray_interpolation). */
        void _tide_corrections(t_gtime &t, t_gtriple &xyu);

        /** @brief calculate short term tidal corrections(fortran: polut1_ray_calc). */
        t_pudaily &_tideCor1Cal(t_pudaily &b);

        /** @brief calculate the diurnal and semi-diurnal variations in Earth Orientation Parameters (x,y, UT1, LOD) from ocean tides (fortran: interp.f). */
        /** agree with _ORTHO_EOP at the level of a few microarcseconds in polar motion and a few tenths of a microsecond in UT1*/
        /** create by yqyuan from interp.f 20210829; calculate both UT1 and LOD; LOD not outputed antway */
        void _PMUT1_OCEANS(t_gtime &t, t_gtriple &eop);

        /** @brief evaluate the model of polar motion for a nonrigid Earth due to tidal gravitation. */
        void _PMSDNUT2(t_gtime &t, double *pm);

        /** @brief evaluate the model of subdiurnal libration in the axial component of rotation, expressed by UT1 and LOD.. */
        void _UTLIBR(t_gtime &t, double *temp);

        /** @brief calculate zonaltide correction on ut1(fortran: ut1_zonaltide_interpolation). */
        double _tideCor2(const double &dRmjd);

        /** @brief evaluate the effects of zonal Earth tides on the rotation of the Earth. */
        void _RG_ZONT2(const double &dT, double *DUT, double *DLOD, double *DOMEGA);
        void _cal_dPole_dut1(t_gtime &t, double *x, t_gpoleut1 *poleut1, string type);
        /** @brief calculate fund arg. */
        void _FUNDARG(const double &T, double *L, double *LP, double *F, double *D, double *OM);
        /** @brief Mean anomaly of the Moon. */
        double _iauFal03(const double &t);
        /** @brief Mean anomaly of the Sun. */
        double _iauFalp03(const double &t);
        /** @brief Mean argument of the latitude of the Moon. */
        double _iauFaf03(const double &t);
        /** @brief Mean elongation of the Moon from the Sun. */
        double _iauFad03(const double &t);
        /** @brief Mean longitude of the ascending node of the Moon. */
        double _iauFaom03(const double &t);
        /** @brief Planetary longitudes, Mercury. */
        double _iauFame03(const double &t);
        /** @brief Planetary longitudes, Venus. */
        double _iauFave03(const double &t);
        /** @brief Planetary longitudes, Earth. */
        double _iauFae03(const double &t);
        /** @brief Planetary longitudes, Mars. */
        double _iauFama03(const double &t);
        /** @brief Planetary longitudes, Jupiter. */
        double _iauFaju03(const double &t);
        /** @brief Planetary longitudes, Saturn. */
        double _iauFasa03(const double &t);
        /** @brief Planetary longitudes, Uranus. */
        double _iauFaur03(const double &t);
        /** @brief Planetary longitudes, Neptune. */
        double _iauFane03(const double &t);
        /** @brief General accumulated precession in longitude. */
        double _iauFapa03(const double &t);

        /** @brief calculate _pudata(fortran: polut1_interpolation). */
        void _calPoleut1(t_gtime &t, double *x, t_gpoleut1 *poleut1, string type = "");

        /** @brief interpolate the nutation(fortran: nutation_interpolation). */
        void _nutInt(const double &dRmjd, double *dpsi, double *deps, const double &step);

        /** @brief calculate nutation according to IAU 2000A model(fortran: nu2000a). */
        void _nutCal(const double &dATE1, const double &dATE2, double *dPSI, double *dEPS);

        /** @brief calculate nutation according to IAU 2000A model adjusted for IAU2006 precession. */
        void _nutCal_06(double dATE1, double dATE2, double *dPSI, double *dEPS);

        /** @brief calculate free-core-nutation (FCN) using a fcn model */
        void _nutFCN(const double &dRmjd, double *dX, double *dY, double *sigdX, double *sigdY);

        /** @brief interpolation. */
        double _interpolation(const int &iOrder, const int &iPoint, double *pdX, double *pdY, const double &dXin);

        /** @brief calculate rotation matrix of Precession and nutation. */
        void _process2000(const double &dRmjd, const double &dpsi, const double &deps, Matrix &qmat, const bool &ldpsi, const bool &ldeps);

        /** @brief calculate rotation matrix of IAU2006/2000A Precession and nutation. */
        void _process2006(const double &dRmjd, const double &dpsi, const double &deps, Matrix &qmat, const bool &ldpsi, const bool &ldeps);

        /** @brief calculate X/Y coordinates of celestial intermediate pole            */
        void _iau_XY00(double date1, double date2, double *x, double *y);

        /** @brief calculate X/Y coordinates of celestial intermediate pole            */
        /** from series based on IAU 2006 precession and IAU 2000A nutation.           */
        void _iau_XY06(double date1, double date2, double *x, double *y);

        /** @brief calculate the CIO locator s, IAU 2000/2006. */
        double _iau_CIO_locator(double date1, double date2, double x, double y);

        /** @brief calculate the angle (unit: radian) according to the current epoch and J2000(fortran: sp2000). */
        double _sp2000(const double &dDATE1, const double &dDATE2);

        /** @brief calculate the earth rotation angle (unit: radian)(fortran: era2000). */
        double _era2000(const double &dJ1, const double &dJ2);

        /** @brief calculate Greenwich sidereal time, IAU 2000. */
        double _gst2000(const double &dRmjd, const double &era, const double &dpsi);

        /** @brief calculate Greenwich sidereal time, IAU 2006. */
        /** called: _iau_S06; _iau_Eors                         */
        double _gst2006(const double &dRmjd, const double &era, const double &dpsi);

        /** @brief extract X/Y from BPN(Q) matrix            */
        void _iau_bpn2xy(Matrix qmat, double *X, double *Y);

        /** @brief Equation of the origins, given the classical NPB matrix and the quantity s. */
        double _iau_Eors(Matrix rnpb, double s);

        /** @brief fmod to (0,2pi). */
        double _iau_anp(const double &dA);

        /** @brief Equation of the equinoxes complementary terms **/
        /*    consistent with IAU 2000 resolutions(return complementary terms (radians)). */
        /* used for both GAST2000 and GAST 2006 */
        double _eect2000(const double &dRmjd);

        /** @brief fmod. */
        double _iau_anpm(const double &dA);

        // inner variable
        bool _erptab = false; ///< if ERP table is used or not
        t_gtime _epo;         ///< cureent epoch
        t_gtriple _pudata;    ///< pole and ut1 data after interpolation(xpole, ypole and ut1-tai)
        t_gtime _tdt;         ///< dynamic time
        double _arg[5];       ///< fundamental arguements of nutation in radius
        Matrix _qmat;         ///< TODO
        double _epsa = 0.0;   ///< TODO

        // out variable
        double _xpole = 0.0; ///< x pole of pole shift
        double _ypole = 0.0; ///< y pole of pole shift
        double _gmst = 0.0;  ///< Greenwich Mean Sidereal Time
        Matrix _rotmat;      ///< rotation matrix from TRS to CRS
        Matrix _rotdu;       ///< partial of rotmat wrt to ut1.
        Matrix _rotdx;       ///< partial of rotmat wrt to xpole
        Matrix _rotdy;       ///< partial of rotmat wrt to ypole

        Matrix _rotdpsi; ///< partial of rotmat wrt to psi of nutation
        Matrix _rotdeps; ///< partial of rotmat wrt to eps of nutation

        t_gpoleut1 *_poleut1; ///< poleut1 data

        // constant values
        const double _rr = 1.00273781191135448e0; ///< r = gmst/ut1 = 1.00273781191135448D0 (ERA2000.f90, IERS20xx)
        const double _djc = 36525.0;              ///< days per julian century

        // === void t_gtrs2crs::_process2000(const double& dRmjd, const double& dpsi, const double& deps) ===
        const double dEps0 = 84381.448 / (RAD2SEC);      ///< J2000 obliquity (Lieske et al. 1977)
        const double dEps0_2006 = 84381.406 / (RAD2SEC); ///< J2000 obliquity in IAU2006
        const double dRa0 = -0.0146 / (RAD2SEC);         ///< The ICRS RA of the J2000 equinox (Chapront et al., 2002)
        const double dPrecor = -0.29965 / (RAD2SEC);     ///< The precession and obliquity corrections (radians per century) (page 43, equ(27))
        const double dOblcor = -0.02524 / (RAD2SEC);     ///< The obliquity corrections (radians per century) (page 43, equ(27))
        const double dPsibi = -0.041775 / (RAD2SEC);     ///< The frame bias corrections in longitude and obliquity (page 43, equ(28) ?)
        const double dEpsbi = -0.0068192 / (RAD2SEC);    ///< The eps bias corrections in longitude and obliquity (page 43, equ(28) ?)

        // === double t_gtrs2crs::_era2000(const double& dJ1, const double& dJ2) ===
        const double dPI = 3.141592653589793238462643;  ///< PI
        const double d2PI = 6.283185307179586476925287; ///< DOUBLE PRECISION DJ0
        const double dJ0 = 51544.5;                     ///< Reference epoch (J2000), JD

        // === double t_gtrs2crs::_eect2000(const double& dRmjd) ===
        const double dAS2R = 4.848136811095359935899141 * 0.000001; ///<         *  Arcseconds to radians
        const double dJC = 36525;                                   ///<         *  Days per Julian century

        // == void  t_gtrs2crs::_nutCal(const double& dATE1, const double& dATE2, double *dPSI, double *dEPS) ===
        const double dDAS2R = 4.848136811095359935899141e-6;
        const double dDMAS2R = dDAS2R / 1e3;
        const double dTURNAS = 1296000e0;
        const double dU2R = dDAS2R / 1e7;
        const double dDJ0 = 51544.5e0;
        const double dDJC = 36525e0;

        // === void t_gtrs2crs::_PMSDNUT2(t_gtime& t,double* pm) ===
        const double _sec2rad = 4.848136811095359935899141e-6;
        const double _turnas = 1296000.0;
        const double rmjd0 = 51544.5;
        const double rad2sec = 86400.0 / d2PI;

        // IAU2000A or IAU2006/2000A; added by yqyuan 2021.05.10
        string _cver; ///< IAU version: 00(default) or 06
    };
} // namespace

#endif