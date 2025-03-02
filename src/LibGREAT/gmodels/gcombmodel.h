/**
 * @file         gcombmodel.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        base combine biase model
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GCOMBMODEL_H
#define GCOMBMODEL_H

#include "gexport/ExportLibGREAT.h"
#include "gmodels/gbasemodel.h"
#include "gmodels/gbiasmodel.h"
#include "gall/gallbias.h"
#include "gall/gallproc.h"
#include "gutils/gcommon.h"
#include <map>
#include <memory>

using namespace gnut;
using namespace std;

namespace great
{
    /** @brief Class for combine biase model */
    class LibGREAT_LIBRARY_EXPORT t_gcombmodel : virtual public t_gbasemodel
    {
    public:
        /** @brief default constructor.
        *
        *param[in]    spdlog                spdlog control
        *param[in]    settings        setbase control
        *param[in]    bias_model        model of bias
        *param[in]    data            all data
        */
        t_gcombmodel(t_gsetbase *setting, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data);

        /** @brief default constructor.
        *
        *param[in]    spdlog                spdlog control
        *param[in]    settings        setbase control
        *param[in]    bias_model        model of bias
        *param[in]    data            all data
        */
        t_gcombmodel(t_gsetbase *setting, t_spdlog spdlog, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data);

        /** @brief default destructor. */
        virtual ~t_gcombmodel();

        /** @brief get index of band */
        map<GSYS, map<FREQ_SEQ, GOBSBAND>> get_band_index();

        /** @brief get index of frequency */
        map<GSYS, map<GOBSBAND, FREQ_SEQ>> get_freq_index();

    protected:


        // ====================================================================================================================
        map<GSYS, map<FREQ_SEQ, GOBSBAND>> _band_index; ///< index of band

        map<GSYS, map<GOBSBAND, FREQ_SEQ>> _freq_index; ///< index of frequency

        shared_ptr<t_gbiasmodel> _bias_model; ///< baise model

        t_gallbias *_gallbias = nullptr; ///< bias setting derive
        t_spdlog _spdlog = nullptr;      ///< implements spdlog class derived

        int _frequency;         ///< frequency
        IFCB_MODEL _ifcb_model; ///< model of IFCB
        IONMODEL _ion_model;    ///< model of ION
        OBSCOMBIN _observ;      ///< observation

    protected:
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
    };
    class LibGREAT_LIBRARY_EXPORT t_gcombIF : virtual public t_gcombmodel
    {
    public:
        /** @brief constructor. */
        t_gcombIF(t_gsetbase *setting, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data);
        t_gcombIF(t_gsetbase *setting, t_spdlog spdlog, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data);

        /** @brief destructor. */
        ~t_gcombIF();

        /** @brief Combine equation */
        bool cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gbaseEquation &result) override;
        bool cmb_equ_IF(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, GOBSBAND b1, GOBSBAND b2, t_gbaseEquation &result);

    private:
        /** @brief add IF multi rec clk */
        bool _add_IF_multi_rec_clk(const FREQ_SEQ &freq, t_gsatdata &obsdata, t_gallpar &params, vector<pair<int, double>> &coef_IF);

        map<pair<FREQ_SEQ, GSYS>, par_type> _clk_type_index;  ///< clk type index
    };

    class LibGREAT_LIBRARY_EXPORT t_gcombALL : virtual public t_gcombmodel
    {
    public:
        /** @brief constructor. */
        t_gcombALL(t_gsetbase *setting, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data);
        t_gcombALL(t_gsetbase *setting, t_spdlog spdlog, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data);

        /** @brief destructor. */
        ~t_gcombALL();

        /** @brief Combine equation */
        bool cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gbaseEquation &result) override;

    private:
        map<FREQ_SEQ, par_type> ambtype_list;
    };

    class LibGREAT_LIBRARY_EXPORT t_gcombDD : virtual public t_gcombALL, virtual public t_gcombIF
    {
    public:
        /** @brief constructor. */
        t_gcombDD(t_gsetbase *setting, const shared_ptr<t_gbiasmodel> &bias_model, t_gallproc *data);
        t_gcombDD(t_gsetbase *setting, t_spdlog spdlog, const shared_ptr<t_gbiasmodel> &bias_model, t_gallproc *data);

        /** @brief destructor. */
        ~t_gcombDD();

        /** @brief Combine equation */
        bool cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gbaseEquation &result) override;

        /** @brief set observation */
        void set_observ(OBSCOMBIN observ);

        /** @brief set base data */
        void set_base_data(vector<t_gsatdata> *data_base);

        /** @brief set site */
        void set_site(const string &site, const string &site_base);

        /** @brief set rec info */
        void set_rec_info(const t_gtriple &xyz_base, double clk_rover, double clk_base);

    protected:
        bool _temp_params(t_gallpar &params, t_gallpar &params_temp);
        OBSCOMBIN _observ;              ///< Combination of observations
        vector<t_gsatdata> *_data_base; ///< base data
        string _site, _site_base;       ///< site
        double _clk_rover, _clk_base;   ///< clk
        t_gtriple _crd_base;            ///< base crd
    };



}
#endif