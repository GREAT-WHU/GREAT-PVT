/**
 * @file         gbiasmodel.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        mainly about how to cacultae B P l single
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GBIASEMODEL_H
#define GBIASEMODEL_H
#include "gexport/ExportLibGREAT.h"
#include "gutils/gtime.h"
#include "gall/gallpar.h"
#include "gdata/gsatdata.h"
#include "gdata/gobsgnss.h"
#include "gmodels/gbasemodel.h"
#include "gmodels/gtropo.h"

using namespace std;
using namespace gnut;

namespace great
{
    /**
    *@brief t_gbiasmodel Class for bias model
    */
    class LibGREAT_LIBRARY_EXPORT t_gbiasmodel
    {
    public:
        /** @brief default constructor.
        *
        *param[in]    spdlog            data spdlog
        *param[in]    settings        setbase control
        */
        t_gbiasmodel(t_gsetbase *settings);

        /** @brief default constructor. 
        *
        *param[in]    spdlog            data spdlog    
        *param[in]    settings        setbase control
        */
        t_gbiasmodel(t_spdlog spdlog, t_gsetbase *settings);

        /** @brief default constructor.*/
        t_gbiasmodel();

        /** @brief default destructor. */
        virtual ~t_gbiasmodel();

        /** @brief Combined equation */
        virtual bool cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gobs &gobs, t_gbaseEquation &result) = 0;

        /** @brief empty function */
        virtual void update_obj_clk(const string &obj, const t_gtime &epo, double clk){};

        /** @brief get clk of reciever */
        virtual double get_rec_clk(const string &obj) { return 0.0; }; 

		virtual bool get_omc_obs_ALL(const t_gtime& crt_epo, t_gsatdata& obsdata, t_gallpar& pars, t_gobs& gobs, double& omc);

        t_gtriple _trs_rec_crd; ///< coordinates of reciever in terrestrial reference system
        t_gtriple _crs_rec_crd; ///< coordinates of reciever in coordinate reference system
        t_gtriple _trs_sat_crd; ///< coordinates of satellite in terrestrial reference system
        t_gtriple _crs_sat_crd; ///< coordinates of satellite in coordinate reference system
        t_gtriple _trs_rec_pco; ///< PCO of reciever in terrestrial reference system
        t_gtriple _crs_rec_pco; ///< PCO of reciever in coordinate reference system
        t_gtriple _trs_sat_pco; ///< PCO of satellite in terrestrial reference system
        t_gtriple _crs_sat_pco; ///< PCO of satellite in coordinate reference system
        t_gtriple _trs_rec_vel; ///< velocity of reciever in terrestrial reference system
        t_gtriple _crs_rec_vel; ///< velocity of reciever in coordinate reference system
        t_gtriple _trs_sat_vel; ///< velocity of satellite in terrestrial reference system
        t_gtriple _crs_sat_vel; ///< velocity of satellite in coordinate reference system

        string _crt_sat; ///< current satellite
        string _crt_rec; ///< current reciever
        GSYS _crt_sys;   ///< current system

    protected:
        /** @brief calculate distance of models
        *
        *param[in] epoch            current epoch
        *param[in] sat                satlite name
        *param[in] rec                reciever name
        *param[in] param            reciever name
        *param[in] gsatdata            satlite data
        *param[in] gobs                observation data
        */
        virtual double cmpObs(t_gtime &epoch, string &sat, string &rec, t_gallpar &param, t_gsatdata &gsatdata, t_gobs &gobs) { return 0.0; };

        // wegight obs
        /** @brief weight RAW_ALL observation
        *
        *param[in] type                type of observation
        *param[in] gobs1            observations
        *param[in] obsdata            observation data
        *param[in] factorP            factor P
        *param[in] wgt                weight
        */
        bool _wgt_obs_ALL(const t_gdata::ID_TYPE &type, t_gobs &gobs1, t_gsatdata &obsdata, const double &factorP, double &wgt);

        /** @brief omc RAW_ALL observation
        *
        *param[in] crt_epo            current epoch
        *param[in] obsdata            observation data
        *param[in] pars                parameter
        *param[in] omc                TODO
        */
        bool _omc_obs_ALL(const t_gtime &crt_epo, t_gsatdata &obsdata, t_gallpar &pars, t_gobs &gobs, double &omc);

        /** @brief calculate basic partial
        *
        *param[in] epoch            current epoch
        *param[in] obsdata            observation data
        *param[in] gobs                observations
        *param[in] par                parameter
        *param[in] partial            partial
        */
        bool _Partial_basic(const t_gtime &epoch, t_gsatdata &obsdata, const t_gobs &gobs, const t_gpar &par, double &partial);

        /** @brief get mapping function
        *
        *param[in] par                parameter
        *param[in] satData            data of satellite
        *param[in] crd                coordinate
        *param[in] epoch            epoch
        *param[in] mfw                TODO
        *param[in] mfh                TODO
        *param[in] dmfw                TODO
        *param[in] dmfh                TODO
        */
        void _getmf(const t_gpar &par, t_gsatdata &satData, const t_gtriple &crd, const t_gtime &epoch, double &mfw, double &mfh, double &dmfw, double &dmfh);

        t_spdlog _spdlog;  ///< spdlog data
        t_gsetbase *_gset; ///< set base control

        //======================================================================================================\
        // xml swtting
        // trop
        int _frequency;                      ///< _frequency
        CONSTRPAR _crd_est = CONSTRPAR::FIX; ///< TODO
        shared_ptr<t_gtropo> _tropoModel;    ///< model of tropo
        TROPMODEL _trpModStr;                ///< TODO
        ZTDMPFUNC _mf_ztd;                   ///< mapping function for ZTD
        GRDMPFUNC _mf_grd;                   ///< mapping function for GRD
        IONMODEL _ion_model;                 ///< model of IONO

        OBSWEIGHT _weight; ///< weight calculation method

        double _sigCodeGPS;    ///< code bias of GPS
        double _sigCodeGLO;    ///< code bias of GLO
        double _sigCodeGAL;    ///< code bias of GAL
        double _sigCodeBDS;    ///< code bias of BDS
        double _sigCodeQZS;    ///< code bias of QZS

        double _sigPhaseGPS;    ///< phase bias of GPS
        double _sigPhaseGLO;    ///< phase bias of GLO
        double _sigPhaseGAL;    ///< phase bias of GAL
        double _sigPhaseBDS;    ///< phase bias of BDS
        double _sigPhaseQZS;    ///< phase bias of QZS

        map<GSYS, map<FREQ_SEQ, GOBSBAND>> _band_index; ///< index of band
        map<GSYS, map<GOBSBAND, FREQ_SEQ>> _freq_index; ///< index of frequency
        OBSCOMBIN _observ;                              ///< type of observations
        modeofmeanpole _meanpole_model;
        //======================================================================================================\

    };

} // namespace great

#endif