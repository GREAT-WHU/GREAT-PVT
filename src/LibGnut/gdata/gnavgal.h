/**
*
* @verbatim
    History
    2013-05-07  PV: created
    2018-08-13  JD: updated

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file      gnavgal.h
* @brief     gal ephemerides and navigation classes
*.
* @author    PV
* @version   1.0.0
* @date      2013-05-07
*
*/
#ifndef GNAVGAL_H
#define GNAVGAL_H

#include <vector>

#include "gdata/gnav.h"
#include "gutils/gtime.h"
#include "gutils/gconst.h"
#include "gutils/gcommon.h"

#define MAX_RINEXN_REC_GAL 28

using namespace std;

namespace gnut
{

    // ICD
#define SC2R 3.1415926535898 ///< Ratio of a circle's circumference to its diameter

    class LibGnut_LIBRARY_EXPORT t_gnavgal : public t_gnav
    {
    public:
        /** @brief default constructor. */
        t_gnavgal();

        /**
         * @brief Construct a new t gnavgal object
         * 
         * @param spdlog 
         */
        t_gnavgal(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gnavgal();

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
         * @param full 
         * @return GNAVTYPE 
         */
        GNAVTYPE gnavtype(bool full = true) const override; ///< distinguish INAV/FNAV or in full details

        /**
         * @brief 
         * 
         * @param full 
         * @return int 
         */
        int src(bool full = true) const override; ///< distinguish INAV/FNAV or in full details

        /** @brief get iod value. */
        int iod() const override { return _iode; }

        /** @brief get rec value. */
        int rec() const override { return MAX_RINEXN_REC_GAL; }

        /** @brief get _rec value. */
        double rel() { return _rel; }

        /** @brief check tot. */
        virtual bool chktot(const t_gtime &t) override;

        /** @brief get parameter. */
        t_timdbl param(const NAVDATA &n) override;

        /** @brief get parameter. */
        int param(const NAVDATA &n, double val) override;

        /** @brief get line. */
        string line() const override;

        /** @brief get line format. */
        string linefmt() const override;

        /** @brief convert health to string. */
        string health_str() const override { return _health_str(); }

    protected:
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
         * @return string 
         */
        string _health_str() const override;

        /**
         * @brief source string
         * 
         * @return string 
         */
        string _source_str() const;

        /**
         * @brief return type of navig. mess.
         * 
         * @param full 
         * @return GNAVTYPE 
         */
        GNAVTYPE _gnavtype(bool full = true) const;

    private:
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

        int _iode;      ///< issue of ephemeris
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
        double _tgd[2]; ///< [seconds]  0:E5a/E1   1:E5b/E1
        int _source;    ///< data source
        double _sisa;   ///< SISA Signal in space accuracy [meters]
        double _rel;    ///<relativity correction calculated with ICD
    };

} // namespace

#endif
