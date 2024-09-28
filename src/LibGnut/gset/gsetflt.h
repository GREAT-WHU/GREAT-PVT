/**
*
* @verbatim
    History

    @endverbatim
*
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

* @file       gsetflt.h
* @brief      implements filter setting
* @author     Jan Dousa
* @version    1.0.0
* @date       2012-10-23
*
*/

#ifndef GSETFLT_H
#define GSETFLT_H

#include <string>
#include <iostream>

#include "gutils/gtypeconv.h"
#include "gset/gsetbase.h"

#define XMLKEY_FLT "filter" ///< The defination of filter node

using namespace std;

namespace gnut
{

    class LibGnut_LIBRARY_EXPORT t_gsetflt : public virtual t_gsetbase
    {
    public:
        /** @brief default constructor. */
        t_gsetflt();

        /** @brief default destructor. */
        ~t_gsetflt();

        /**
        * @brief settings check.
        * @return void
        */
        void check();

        /**
        * @brief settings help.
        * @return void
        */
        void help();

        /** 
         * @brief get the flt method
         * @return string : the method of flt
         */
        string method_flt();

        /**
         * @brief get the noise of clk in flt
         * @return double : the noise of clk in flt
         */
        double noise_clk();

        /**
         * @brief get the noise of crd in flt
         * @return double : the noise of crd in flt
         */
        double noise_crd();

        /**
         * @brief get the noise of dclk in flt
         * @return double : the noise of dclk in flt
         */
        double noise_dclk();

        /**
         * @brief get the noise of vel in flt
         * @return double : the noise of vel in flt
         */
        double noise_vel();

        /**
        * @brief get the random walk of glonass in flt
        * @return double : the random walk of gps in flt
        */
        double rndwk_gps();

        /**
         * @brief get the random walk of glonass in flt
         * @return double : the random walk of glonass in flt
         */
        double rndwk_glo();

        /**
         * @brief get the random walk of galileo in flt
         * @return double : the random walk of galileo in flt
         */
        double rndwk_gal();

        /**
         * @brief get the random walk of Beidou in flt
         * @return double : the random walk of Beidou in flt
         */
        double rndwk_bds();

        /**
         * @brief get the random walk of amb in flt
         * @return double : the random walk of amb in flt
         */
        double rndwk_amb();

        /**
         * @brief get the random walk of QZSS in flt
         * @return double : the random walk of QZSS in flt
         */
        double rndwk_qzs();

        /**
         * @brief get the random walk of ztd in flt
         * @return double : the random walk of ztd in flt
         */
        double rndwk_ztd();

        /**
         * @brief get the noise of vion in flt
         * @return double : the noise of vion in flt
         */
        double noise_vion();

        /**
        * @brief get the random walk of ion in flt
        * @return double : the random walk of ion in flt
        */
        double rndwk_vion();

        /**
         * @brief get the random walk of grd in flt
         * @return double : the random walk of grd in flt
         */
        double rndwk_grd();

        /**
         * @brief get the reset value of amb in flt
         * @return int : the reset value of amb in flt
         */
        int reset_amb();

        /**
         * @brief get the reset value of par in flt
         * @return int : the reset value of par in flt
         */
        int reset_par();        
        /**
         * @brief get the reset value of par in flt
         * @return int : the reset value of par in flt
         */
        int reset_par(double d);

    protected:
        string _method_flt; ///< type of filtering method (kalman, SRCF)
        double _noise_clk;  ///< white noise for receiver clock [m]
        double _noise_crd;  ///< white noise for coordinates [m]
        double _noise_dclk; ///< white noise for receiver clock speed [m/s]
        double _noise_vel;  ///< white noise for velocities [m/s]
        double _rndwk_gps;  ///< random walk process for GPS IFB
        double _rndwk_glo;  ///< random walk process for GLONASS system time offset
        double _rndwk_gal;  ///< random walk process for Galileo system time offset
        double _rndwk_bds;  ///< random walk process for BeiDou system time offset
        double _rndwk_qzs;  ///< random walk process for ZQSS system time offset
        double _rndwk_ztd;  ///< random walk process for ZTD [mm/sqrt(hour)]
        double _rndwk_vion; ///< random walk process for VION [mm/sqrt(hour)]
        double _noise_vion; ///< white noise process for VION [m]
        double _rndwk_grd;  ///< random walk process for tropo grad [mm/sqrt(hour)]
        double _rndwk_amb;  ///< random walk process for ambiguity  [mm/sqrt(hour)]
        int _reset_amb;     ///< interval for reseting ambiguity [s]
        int _reset_par;     ///< interval for reseting CRD, ZTD, AMB [s]

    private:
    };

} // namespace

#endif
