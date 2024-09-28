/**
*
* @verbatim
    History
    2011-02-14  JD: created
    2018-08-13  JD: updated

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file      gnavbds.h
* @brief     bds ephemerides and navigation classes
*.
* @author    JD
* @version   1.0.0
* @date      2011-02-14
*
*/
#ifndef GNAVBDS_H
#define GNAVBDS_H

#include <set>

#include "gdata/gnav.h"
#include "gutils/gtime.h"
#include "gutils/gconst.h"
#include "gutils/gcommon.h"
#include "gutils/gtriple.h"

#define MAX_RINEXN_REC_BDS 29

using namespace std;

namespace gnut
{
///< ICD BeiDou v2
#define SC2R 3.1415926535898 ///< semi-circle to radian

    /** @brief Class for bds navigation data storing. */
    class LibGnut_LIBRARY_EXPORT t_gnavbds : public t_gnav
    {

    public:
        /** @brief default constructor. */
        explicit t_gnavbds();

        explicit t_gnavbds(t_spdlog spdlog);
        /** @brief default destructor. */
        virtual ~t_gnavbds();

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

        /**
         * @brief 
         * 
         * @param ep 
         * @param data 
         * @return int 
         */
        int data2nav(string, const t_gtime &ep, const t_gnavdata &data) override;

        /**
         * @brief 
         * 
         * @param data 
         * @return int 
         */
        int nav2data(t_gnavdata &data) override;

        /**
         * @brief 
         * 
         * @return int 
         */
        int iod() const override;

        /**
         * @brief 
         * 
         * @return int 
         */
        int rec() const override { return MAX_RINEXN_REC_BDS; }

        /**
         * @brief 
         * 
         * @param t 
         * @return true 
         * @return false 
         */
        virtual bool chktot(const t_gtime &t) override;

        /**
         * @brief 
         * 
         * @param n 
         * @return t_timdbl 
         */
        t_timdbl param(const NAVDATA &n) override;

        /**
         * @brief 
         * 
         * @param n 
         * @param val 
         * @return int 
         */
        int param(const NAVDATA &n, double val) override;

        /**
         * @brief 
         * 
         * @return string 
         */
        string line() const override;

        /**
         * @brief 
         * 
         * @return string 
         */
        string linefmt() const override;

    private:
        /**
         * @brief 
         * 
         * @return true 
         * @return false 
         */
        bool _healthy() const override;

        /**
         * @brief 
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

        /**
         * @brief get IODC
         * 
         * @return const int 
         */
        const int _getIODC() const;

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
        double _acc;    ///< sv accuracy index (URA)
        double _tgd[2]; ///< group delay parameters [sec]  0: B1/B3,  1: B2/B3
        double _rel;    ///< relativity correction calculated with ICD
    };

}
#endif
