/**
*
* @verbatim
    History
    2011-02-14 /JD: created
    2018-08-13 /JD: updated

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gnavsbs.h
* @brief      SBS navigation
*.
* @author     JD
* @version    1.0.0
* @date       2011-02-14
*
*/
#ifndef GNAVSBS_H
#define GNAVSBS_H

#include <vector>

#include "newmat/newmat.h"
#include "gdata/gnav.h"
#include "gutils/gtime.h"
#include "gutils/gconst.h"
#include "gutils/gcommon.h"
#include "gutils/gtriple.h"

#define MAX_RINEXN_REC_SBS 15

using namespace std;

namespace gnut
{

    /** @brief class for t_gnavsbs. */
    class t_gnavsbs : public t_gnav
    {

    public:
        /** @brief default constructor. */
        explicit t_gnavsbs();

        /**
         * @brief Construct a new t gnavsbs object
         * 
         * @param spdlog 
         */
        explicit t_gnavsbs(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gnavsbs();

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
        virtual int pos(const t_gtime &t, double xyz[3], double var[3] = NULL, double vel[3] = NULL, bool chk_health = true) override;

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
        virtual int clk(const t_gtime &t, double *clk, double *var = NULL, double *dclk = NULL, bool chk_health = true) override;

        /**
         * @brief 
         * 
         * @param msg 
         * @return int 
         */
        int chk(set<string> &msg) override;

        /** @brief convert data to nav. */
        int data2nav(string sat, const t_gtime &ep, const t_gnavdata &data) override;

        /** @brief convert nav to data. */
        int nav2data(t_gnavdata &data) override;

        /** @brief get iod. */
        int iod() const override { return _iod; }

        /** @brief get rec. */
        int rec() const override { return MAX_RINEXN_REC_SBS; }

        /** @brief get/set parameter. */
        virtual t_timdbl param(const NAVDATA &n) override;

        /**
         * @brief 
         * 
         * @param n 
         * @param val 
         * @return int 
         */
        virtual int param(const NAVDATA &n, double val) override;

        /** @brief get line. */
        string line() const override;

        /** @brief get line format. */
        string linefmt() const override;

    protected:
        /** @brief get health. */
        bool _healthy() const override;

    private:
        double _x;      ///< position X [km]
        double _x_d;    ///< velocity X [km/s]
        double _x_dd;   ///< acceleration X [km/s^2]
        double _y;      ///< position Y [km]
        double _y_d;    ///< velocity Y [km/s]
        double _y_dd;   ///< acceleration Y [km/s^2]
        double _z;      ///< position Z [km]
        double _z_d;    ///< velocity Z [km/s]
        double _z_dd;   ///< acceleration Z [km/s^2]
        double _f0;     ///< SV clock bias [s]
        double _tki;    ///< Transmission time of message in GPS seconds of weak
        double _health; ///< health 0 = OK
        double _C_rms;  ///< Accuracy code [m]
        int _iod;       ///< Issue of Data Navigation
        double _f1;     ///< SV relative frequency bias []
        t_gtime _toc;   ///< Epoch of ephemerides
    };

} // namespace

#endif
