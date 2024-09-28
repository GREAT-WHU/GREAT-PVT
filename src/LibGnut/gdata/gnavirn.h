/**
*
* @verbatim
    History
    2016-06-14  PV: created
    2018-08-13  JD: updated

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gnavirn.h
* @brief      IRN navigation
*.
* @author     PV
* @version    1.0.0
* @date       2016-06-14
*
*/
#ifndef GNAVIRN_H
#define GNAVIRN_H

#include <vector>

#include "gdata/gnav.h"
#include "gutils/gtime.h"
#include "gutils/gconst.h"
#include "gutils/gcommon.h"

#define MAX_RINEXN_REC_IRN 28
#define SYS_IRN 'I'

using namespace std;

namespace gnut
{

    // IRNSS_SPS_ICD
#define GM_WGS84 3.986005e14     ///< WGS 84 value of earth's graviational constant for GPS user [m^3/s^2]
#define OMGE_DOT 7.2921151467e-5 ///< WGS 84 value of the earth's rotation rate [rad/sec]
#define SC2R 3.1415926535898     ///< semi-circle to radian

    /** @brief class for t_gnavirn. */
    class LibGnut_LIBRARY_EXPORT t_gnavirn : public t_gnav
    {

    public:
        /** @brief default constructor. */
        t_gnavirn();

        /**
         * @brief Construct a new t gnavirn object
         * 
         * @param spdlog 
         */
        t_gnavirn(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gnavirn();

        // pointers to support NULL if not requested!
        /**
         * @brief 
         * 
         * @param t 
         * @param xyz 
         * @param var 
         * @param vel 
         * @param chk_health 
         * @return int 
         */
        int pos(const t_gtime &t, double xyz[3], double var[3] = NULL, double vel[3] = NULL, bool chk_health = true) override; //[m]

        /**
         * @brief 
         * 
         * @param t 
         * @param clk 
         * @param var 
         * @param dclk 
         * @param chk_health 
         * @return int 
         */
        int clk(const t_gtime &t, double *clk, double *var = NULL, double *dclk = NULL, bool chk_health = true) override; //[s]

        /**
         * @brief 
         * 
         * @param msg 
         * @return int 
         */
        int chk(set<string> &msg) override;

        /** @brief convert data to nav. */
        int data2nav(string, const t_gtime &ep, const t_gnavdata &data) override;

        /** @brief convert nav to data. */
        int nav2data(t_gnavdata &data) override;

        /** @brief get iod. */
        int iod() const override { return _iode; }

        /** @brief get rec. */
        int rec() const override { return MAX_RINEXN_REC_IRN; }

        /** @brief check tot. */
        virtual bool chktot(const t_gtime &t) override;

        /** @brief get/set parameter. */
        t_timdbl param(const NAVDATA &n) override;

        /**
         * @brief 
         * 
         * @param n 
         * @param val 
         * @return int 
         */
        int param(const NAVDATA &n, double val) override;

        /** @brief get line. */
        string line() const override;

        /** @brief get line format. */
        string linefmt() const override;

    private:
        /** @brief get health state. */
        bool _healthy() const override;

        /**
         * @brief eccentric anomaly
         * 
         * @param dt 
         * @param Ek 
         * @param dEk 
         */
        void _ecc_anomaly(double dt, double &Ek, double &dEk);

        /**
         * @brief corrected mean motion
         * 
         * @return double 
         */
        double _mean_motion();

    private:
        int _iode;      ///< issue of ephemeris
        int _iodc;      ///< issue of clocks
        int _health;    ///< sv health (0:ok)
        t_gtime _toe;   ///< time of ephemerides
        t_gtime _toc;   ///< time of clocks
        t_gtime _tot;   ///< time of transmission
        double _a;      ///< major semiaxis [m]
        double _e;      ///< eccentricity
        double _m;      ///< mean anomaly at t0 [rad]
        double _i;      ///< inclination [rad]
        double _idot;   ///< inclination change [rad/sec]
        double _omega;  ///< argument of perigee [rad]
        double _OMG;    ///< longit. of ascend. node at weekly epoch [rad]
        double _OMGDOT; ///< longit. of ascend. node at weekly epoch's change [rad/sec]
        double _dn;     ///< mean motion difference, delta n [rad/sec]
        double _crc;    ///< amplit. of cos harm. terms - radius [m]
        double _cic;    ///< amplit. of cos harm. terms - inclination [rad]
        double _cuc;    ///< amplit. of cos harm. terms - arg-of-latitude [rad]
        double _crs;    ///< amplit. of sin harm. terms - radius [m]
        double _cis;    ///< amplit. of sin harm. terms - inclination [rad]
        double _cus;    ///< amplit. of sin harm. terms - arg-of-latitude [rad]
        double _f0;     ///< sv clock parameters [sec]
        double _f1;     ///< sv clock parameters [sec/sec]
        double _f2;     ///< sv clock parameters [sec/sec^2]
        double _acc;    ///< sv accuracy [m]
        double _tgd[4]; ///< group delay parameters [sec]
    };

} // namespace

#endif
