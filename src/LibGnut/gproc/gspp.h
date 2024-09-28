/**
*
* @verbatim
    History
    2014-11-24  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gspp.h
* @brief      Purpose: implements spp client
*.
* @author     PV
* @version    1.0.0
* @date       2014-11-24
*
*/

#ifndef GSPP_H
#define GSPP_H

#include "gio/grtlog.h"
#include "gutils/gtime.h"
#include "gutils/gmutex.h"
#include "gprod/gprodcrd.h"
#include "gset/gsetbase.h"
#include "gset/gsetout.h"
#include "gset/gsetgen.h"
#include "gset/gsetgnss.h"
#include "gset/gsetrec.h"
#include "gset/gsetproc.h"
#include "gio/giof.h"
#include "gall/gallobs.h"
#include "gall/gallnav.h"
#include "gall/gallprod.h"
#include "gall/gallobj.h"
#include "gall/gallbias.h"
#include "gmodels/gbancroft.h"
#include "gmodels/gmodel.h"
#include "gmodels/gpar.h"

namespace gnut
{
    /** @brief class for t_gspp. */
    class LibGnut_LIBRARY_EXPORT t_gspp
    {
    public:
        /** @brief constructor 1. */
        t_gspp(string mark, t_gsetbase *set);

        t_gspp(string mark, t_gsetbase *set, t_spdlog spdlog, string mode = "");
        /** @brief default destructor. */
        virtual ~t_gspp();

        /** @brief for process. */
        virtual int processBatch(const t_gtime &beg, const t_gtime &end) = 0;

        /** @brief set for data, output product, object(like pcv model),bias and so on. */
        virtual void setDAT(t_gallobs *gobs, t_gallnav *gnav);

        /** @brief set output for products. */
        void setOUT(t_gallprod *products);

        /** @brief set OBJ/DCB/FCB. */
        virtual void setOBJ(t_gallobj *gobj);
        virtual void setDCB(t_gallbias *gbias);
        virtual void setFCB(t_gallbias *gbias);

        /** @brief Set general log file. */
        virtual void spdlog(t_spdlog spdlog);

        /** 
        *@brief Setting up types of error correction.
        * Tropo is used or not
        */
        void tropo(bool tropo);

        /** @brief Tropo slants are provided. */
        void tropo_slant(bool slant);

        /** @brief phase. */
        void phase(bool phase);

        /** @brief set gnss. */
        void setgnss(GSYS sys);

        /** @brief get site. */
        string site() { return _site; }

    protected:
        shared_ptr<t_gobj> _grec; ///< Transmitter/receiver object.
        t_gallobj *_gallobj;      ///< Objects transmitter/receiver.
        t_gallbias *_gallbias;    ///< Differential code biases.
        t_gallbias *_gallfcb;     ///< Phase fractional cycle bias.

        CONSTRPAR _crd_est;
        t_gtime _crd_begStat;
        t_gtime _crd_endStat;
        t_gtime _ztd_begStat;
        t_gtime _ztd_endStat; ///< constraining parameters

        // a priory parameters
        double _aprox_ztd_xml; ///< Approximate ztd.
        OBSWEIGHT _weight;     ///< observation weighting
        OBSCOMBIN _observ;     ///< observation type model

        /** @brief Get settings from XML file and set local variables. */
        virtual int _get_settings();

        /** @brief Set output file (empty). */
        virtual void _setOut();

        /** @brief Set processing log file (<ppp>). */
        virtual void _setLog(string mode = "");
        bool _valid_crd_xml;
        bool _valid_ztd_xml;
        t_gmodel *_gModel;    ///< models.
        string _site;         ///< Site internal ID.
        t_gsetbase *_set;     ///< Base setting.
        t_spdlog _spdlog;     ///< Processing spdlog output.
        t_spdlog _gspdlog;    ///< Genereal spdlog output.
        t_spdlog _res;        ///< Genereal spdlog output.
        t_grtlog _grtlog;     ///< great log
        t_grtlog _greslog;    ///< Residuals output.
        t_gallobs *_gobs;     ///< Observation data.
        t_gallnav *_gnav;     ///< Objects for ephemerides.
        t_gallprod *_gmet;    ///< Troposphere products input
        t_gallprod *_gion;    ///< Ionosphere products input
        t_gallprod *_allprod; ///< Products output.
        bool _phase;          ///< Phase is used.
        bool _doppler;        ///< Doppler is used.
        bool _tropo_est;      ///< Tropo is estimated.
        bool _iono_est;       ///< Iono is estimated.
        bool _tropo_grad;     ///< Tropo horizontal gradients.
        bool _tropo_slant;    ///< Tropo slants are produced.
        GSYS _gnss;           ///< GNSS system to be used.
        ZTDMPFUNC _ztd_mf;    ///< ZTD mapping function.
        GRDMPFUNC _grd_mf;    ///< GRD mapping function.
        double _minElev;      ///< Elevation cut-off.
        double _sampling;     ///< Sampling interval.
        double _scale;        ///< Sampling scaling factor.
        bool _initialized;    ///< Initialized status.
        int _nSat;      ///< Number of satellites comming into the processing
        int _nSat_excl; ///< Number of excluded satellites due to various reason

        // init sigma
        double _sig_init_crd;  ///< Initial coordinates sigma.
        double _sig_init_vel;  ///< Initial velocities sigma.
        double _sig_init_ztd;  ///< Initial ZTD sigma.
        double _sig_init_vion; ///< Initial VION sigma.
        double _sig_init_grd;  ///< Initial GRD  sigma.

        // ISB sigma
        double _sig_init_glo; ///< Initial GLONASS ISB sigma.
        double _sig_init_gal; ///< Initial Galileo ISB sigma.
        double _sig_init_bds; ///< Initial BeiDou ISB sigma.
        double _sig_init_qzs; ///< Initial QZSS ISB sigma.

        // observations sigma
        double _sigCodeGPS;    ///< Code sigma.  - GPS
        double _sigPhaseGPS;   ///< Phase sigma. - GPS
        double _sigDopplerGPS; ///< Doppler sigma. - GPS
        double _sigCodeGLO;    ///< Code sigma.  - GLONASS
        double _sigPhaseGLO;   ///< Phase sigma. - GLONASS
        double _sigDopplerGLO; ///< Doppler sigma. - GLONASS
        double _sigCodeGAL;    ///< Code sigma.  - Galileo
        double _sigPhaseGAL;   ///< Phase sigma. - Galileo
        double _sigDopplerGAL; ///< Doppler sigma. - Galileo
        double _sigCodeBDS;    ///< Code sigma.  - BeiDou
        double _sigPhaseBDS;   ///< Phase sigma. - BeiDou
        double _sigDopplerBDS; ///< Doppler sigma. - BeiDou
        double _sigCodeQZS;    ///< Code sigma.  - QZSS
        double _sigPhaseQZS;   ///< Phase sigma. - QZSS
        double _sigDopplerQZS; ///< Doppler sigma. - QZSS

        bool _pos_kin;    ///< pos kin
        bool _extern_log; ///< extern log

        bool _use_ecl; ///< use ecl
        bool _success; ///< success
        t_gmutex _gmutex; ///< gmutex

        // signals used for processing
        map<string, map<GOBSBAND, GOBSATTR>> _signals; ///< signal
    };
} // namespace

#endif
