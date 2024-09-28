/**
 * @file         gprecisebiasGPP.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        the class for ppp procrssing
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef precisebiasGPP_h
#define precisebiasGPP_h

#include "gexport/ExportLibGREAT.h"
#include "gset/gsetbase.h"
#include "gset/gsetgen.h"
#include "gmodels/gbiasmodel.h"
#include "gmodels/gprecisebias.h"
#include "gall/gallproc.h"
#include "gutils/gtrs2crs.h"
#include "gall/gallobj.h"
#include "gmodels/gattitudemodel.h"

namespace great
{
    /**
    *@brief       Class for t_gprecisebiasGPP, derive from t_gprecisebias
    */
    class LibGREAT_LIBRARY_EXPORT t_gprecisebiasGPP : public t_gprecisebias
    {
    public:
        /**
        * @brief constructor.
        *
        * @param[in]  data             the data pointer
        * @param[in]  log              the log pointer
        * @param[in]  setting          the set pointer
        */
        explicit t_gprecisebiasGPP(t_gallproc *data, t_gsetbase *setting);

        t_gprecisebiasGPP(t_gallproc *data, t_spdlog spdlog, t_gsetbase *setting);
        /** @brief default destructor. */
        ~t_gprecisebiasGPP();

        /**
        * @brief combine EQU.
        *
        * @param[in]  epoch            the current time
        * @param[in]  params           the parameters
        * @param[in]  obsdata          the observation data
        * @param[in]  gobs             the observation object
        * @param[in]  result           the EQU result
        * @return     bool             combine EQU mode
        */
        bool cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gobs &gobs, t_gbaseEquation &result) override;

        /**
        * @brief prepare observation for GPP.
        *
        * @param[in]  epoch            the current time
        * @param[in]  nav              the navigation data
        * @param[in]  gallobj          the all object
        * @param[in]  pars             the pars
        * @return     bool             prepare mode
        */
        bool _prepare_obs_GPP(const t_gtime &epoch, t_gallnav *nav, t_gallobj *gallobj, t_gallpar &pars);

        /**
        * @brief update observation information for GPP.
        *
        * @param[in]  epoch            the current time
        * @param[in]  nav              the navigation data
        * @param[in]  gallobj          the all object
        * @param[in]  obsdata          the observation data
        * @param[in]  pars             the pars
        * @return     bool             update mode
        */
        bool _update_obs_info_GPP(const t_gtime &epoch, t_gallnav *nav, t_gallobj *gallobj, t_gsatdata &obsdata, t_gallpar &pars);

        /**
        * @brief update clock obiect for GPP.
        *
        * @param[in]  obj              the obj
        * @param[in]  epoch            the current time
        * @param[in]  nav              the navigation data
        * @param[in]  pars             the pars
        * @param[in]  clk              the clock value
        * @param[in]  obj_clk          clock obj
        * @return     bool             update mode
        */
        bool _update_obj_clk_GPP(const string &obj, const t_gtime &epo, t_gallnav *nav, t_gallpar &par, double &clk, map<string, pair<t_gtime, double>> &obj_clk);
    
    protected:

        /**
        * @brief compute obs.
        *
        * @param[in]  epoch            the current time
        * @param[in]  sat              the satellite name
        * @param[in]  rec              the receiver data
        * @param[in]  param            the paramter
        * @param[in]  gsatdata         the satellite data
        * @param[in]  gobs             the obs
        * @return     double           the compute value
        */
        double cmpObs(t_gtime &epoch, string &sat, string &rec, t_gallpar &param, t_gsatdata &gsatdata, t_gobs &gobs) override;

    private:

        /**
        * @brief prt all observation.
        *
        * @param[in]  crt_epo          the current time
        * @param[in]  obsdata          the satellite data
        * @param[in]  pars             the paramter
        * @param[in]  gobs             the obs
        * @param[in]  coeff            the coeff
        * @return     bool             the prt mode
        */
        bool _prt_obs_ALL(const t_gtime &crt_epo, t_gsatdata &obsdata, t_gallpar &pars, t_gobs &gobs, vector<pair<int, double>> &coeff);

        bool _ddmode;       ///< dd mode
    };
}

#endif