/**
 * @file         gcombmodel.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        base combine biase model
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gmodels/gcombmodel.h"
#include "gutils/gstring.h"
#include <cmath>
#include <utility>
#include "gproc/gfltmatrix.h"
#include "gmodels/gprecisebias.h"
namespace great
{
    t_gcombmodel::t_gcombmodel(t_gsetbase *setting, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data) : _bias_model(std::move(bias_model)),
                                                                                                             _spdlog(nullptr)
    {
        _band_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::GPS);
        _band_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::GAL);
        _band_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::GLO);
        _band_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::BDS);
        _band_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::QZS);

        _freq_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::GPS);
        _freq_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::GAL);
        _freq_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::GLO);
        _freq_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::BDS);
        _freq_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::QZS);

        // =======================================================================================
        // sigma
        _sigCodeGPS = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(GPS);
        _sigCodeGLO = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(GLO);
        _sigCodeGAL = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(GAL);
        _sigCodeBDS = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(BDS);
        _sigCodeQZS = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(QZS);
        _sigPhaseGPS = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(GPS);
        _sigPhaseGLO = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(GLO);
        _sigPhaseGAL = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(GAL);
        _sigPhaseBDS = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(BDS);
        _sigPhaseQZS = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(QZS);

        _frequency = dynamic_cast<t_gsetproc *>(setting)->frequency();
        _ion_model = dynamic_cast<t_gsetproc *>(setting)->ion_model();
        _ifcb_model = dynamic_cast<t_gsetproc *>(setting)->ifcb_model();
        _observ = dynamic_cast<t_gsetproc *>(setting)->obs_combin();

        _gallbias = dynamic_cast<t_gallbias *>((*data)[t_gdata::ALLBIAS]);
    }

    t_gcombmodel::t_gcombmodel(t_gsetbase *setting, t_spdlog spdlog, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data) : _bias_model(std::move(bias_model)),
                                                                                                                              _spdlog(spdlog)
    {
        _band_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::GPS);
        _band_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::GAL);
        _band_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::GLO);
        _band_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::BDS);
        _band_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(setting)->band_index(gnut::QZS);

        _freq_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::GPS);
        _freq_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::GAL);
        _freq_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::GLO);
        _freq_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::BDS);
        _freq_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(setting)->freq_index(gnut::QZS);

        // =======================================================================================
        // sigma
        _sigCodeGPS = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(GPS);
        _sigCodeGLO = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(GLO);
        _sigCodeGAL = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(GAL);
        _sigCodeBDS = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(BDS);
        _sigCodeQZS = dynamic_cast<t_gsetgnss *>(setting)->sigma_C(QZS);
        _sigPhaseGPS = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(GPS);
        _sigPhaseGLO = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(GLO);
        _sigPhaseGAL = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(GAL);
        _sigPhaseBDS = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(BDS);
        _sigPhaseQZS = dynamic_cast<t_gsetgnss *>(setting)->sigma_L(QZS);

        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigCodeGPS " + format("%16.8f", _sigCodeGPS));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigCodeGLO " + format("%16.8f", _sigCodeGLO));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigCodeGAL " + format("%16.8f", _sigCodeGAL));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigCodeBDS " + format("%16.8f", _sigCodeBDS));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigCodeQZS " + format("%16.8f", _sigCodeQZS));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigPhaseGPS" + format("%16.8f", _sigPhaseGPS));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigPhaseGLO" + format("%16.8f", _sigPhaseGLO));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigPhaseGAL" + format("%16.8f", _sigPhaseGAL));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigPhaseBDS" + format("%16.8f", _sigPhaseBDS));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "sigPhaseQZS" + format("%16.8f", _sigPhaseQZS));


        _frequency = dynamic_cast<t_gsetproc *>(setting)->frequency();
        _ion_model = dynamic_cast<t_gsetproc *>(setting)->ion_model();
        _ifcb_model = dynamic_cast<t_gsetproc *>(setting)->ifcb_model();
        _observ = dynamic_cast<t_gsetproc *>(setting)->obs_combin();

        _gallbias = dynamic_cast<t_gallbias *>((*data)[t_gdata::ALLBIAS]);
    }

    t_gcombmodel::~t_gcombmodel() = default;

    map<GSYS, map<FREQ_SEQ, GOBSBAND>> t_gcombmodel::get_band_index()
    {
        return _band_index;
    }

    map<GSYS, map<GOBSBAND, FREQ_SEQ>> t_gcombmodel::get_freq_index()
    {
        return _freq_index;
    }

    t_gcombIF::t_gcombIF(t_gsetbase *setting, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data) : t_gcombmodel(setting, std::move(bias_model), data)
    {
        _clk_type_index[make_pair(FREQ_3, GPS)] = par_type::CLK13_G;
        _clk_type_index[make_pair(FREQ_4, GPS)] = par_type::CLK14_G;
        _clk_type_index[make_pair(FREQ_5, GPS)] = par_type::CLK15_G;

        _clk_type_index[make_pair(FREQ_3, GAL)] = par_type::CLK13_E;
        _clk_type_index[make_pair(FREQ_4, GAL)] = par_type::CLK14_E;
        _clk_type_index[make_pair(FREQ_5, GAL)] = par_type::CLK15_E;

        _clk_type_index[make_pair(FREQ_3, BDS)] = par_type::CLK13_C;
        _clk_type_index[make_pair(FREQ_4, BDS)] = par_type::CLK14_C;
        _clk_type_index[make_pair(FREQ_5, BDS)] = par_type::CLK15_C;

        _clk_type_index[make_pair(FREQ_3, QZS)] = par_type::CLK13_J;
        _clk_type_index[make_pair(FREQ_4, QZS)] = par_type::CLK14_J;
        _clk_type_index[make_pair(FREQ_5, QZS)] = par_type::CLK15_J;
    }

    t_gcombIF::t_gcombIF(t_gsetbase *setting, t_spdlog spdlog, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data) : t_gcombmodel(setting, spdlog, std::move(bias_model), data)
    {
        _clk_type_index[make_pair(FREQ_3, GPS)] = par_type::CLK13_G;
        _clk_type_index[make_pair(FREQ_4, GPS)] = par_type::CLK14_G;
        _clk_type_index[make_pair(FREQ_5, GPS)] = par_type::CLK15_G;

        _clk_type_index[make_pair(FREQ_3, GAL)] = par_type::CLK13_E;
        _clk_type_index[make_pair(FREQ_4, GAL)] = par_type::CLK14_E;
        _clk_type_index[make_pair(FREQ_5, GAL)] = par_type::CLK15_E;

        _clk_type_index[make_pair(FREQ_3, BDS)] = par_type::CLK13_C;
        _clk_type_index[make_pair(FREQ_4, BDS)] = par_type::CLK14_C;
        _clk_type_index[make_pair(FREQ_5, BDS)] = par_type::CLK15_C;

        _clk_type_index[make_pair(FREQ_3, QZS)] = par_type::CLK13_J;
        _clk_type_index[make_pair(FREQ_4, QZS)] = par_type::CLK14_J;
        _clk_type_index[make_pair(FREQ_5, QZS)] = par_type::CLK15_J;
    }

    t_gcombIF::~t_gcombIF() = default;

    bool t_gcombIF::cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gbaseEquation &result)
    {
        // ========================================================================================================================================
        // check Obs type
        const GOBSBAND &b1 = _band_index[obsdata.gsys()][FREQ_1];
        const GOBSBAND &b2 = _band_index[obsdata.gsys()][FREQ_2];

        // ========================================================================================================================================
        obsdata.apply_bias(_gallbias);

        return this->cmb_equ_IF(epoch, params, obsdata, b1, b2, result);
    }

    bool t_gcombIF::cmb_equ_IF(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, GOBSBAND b1, GOBSBAND b2, t_gbaseEquation &result)
    {
        if (b1 == BAND || b2 == BAND)
            return false;

        t_gobs gobsP1(obsdata.select_range(b1));
        t_gobs gobsP2(obsdata.select_range(b2));
        t_gobs gobsL1(obsdata.select_phase(b1));
        t_gobs gobsL2(obsdata.select_phase(b2));

        vector<pair<t_gobs, t_gobs>> type_list;
        type_list.emplace_back(gobsP1, gobsP2);
        type_list.emplace_back(gobsL1, gobsL2);

        auto gsys = obsdata.gsys();
        // ========================================================================================================
        // IF coef
        double coef1, coef2;
        obsdata.coef_ionofree(b1, coef1, b2, coef2);

        // ========================================================================================================
        t_gfltEquationMatrix equ_IF;
        for (const auto &item : type_list)
        {
            t_gobs gobs1 = item.first;
            t_gobs gobs2 = item.second;

            //combine f1 and f2
            t_gbaseEquation temp_equ;
            if (!_bias_model->cmb_equ(epoch, params, obsdata, gobs1, temp_equ))
                return false;
            if (!_bias_model->cmb_equ(epoch, params, obsdata, gobs2, temp_equ))
                return false;

            // ========================================================================================================
            if (temp_equ.B[0].size() != temp_equ.B[1].size())
                throw logic_error("coeff size is not equal in f1 and f2");

            vector<pair<int, double>> coef_IF;
            double P_IF = 0.0, l_IF = 0.0;
            for (int i = 0; i < temp_equ.B[0].size(); i++)
            {
                if (temp_equ.B[0][i].first != temp_equ.B[1][i].first)
                {
                    throw logic_error("coeff par is not the same in f1 and f2");
                }
                // combine coeff
                coef_IF.emplace_back(temp_equ.B[0][i].first, coef1 * temp_equ.B[0][i].second + coef2 * temp_equ.B[1][i].second);
            }
            // combine P and l
            P_IF = 1.0 / (pow(coef1, 2) / temp_equ.P[0] + pow(coef2, 2) / temp_equ.P[1]);
            l_IF = coef1 * temp_equ.l[0] + coef2 * temp_equ.l[1];

            //jdhuang :
            if (_ifcb_model == IFCB_MODEL::COR && _frequency == 3 && gobs1.is_phase() && _freq_index[gsys][b2] == FREQ_3 && obsdata.gsys() == GPS)
            {
                l_IF += dynamic_cast<t_gprecisebias *>(_bias_model.get())->ifcbDelay(obsdata, nullptr, OBSCOMBIN::IONO_FREE);
            }

            // ========================================================================================================
            // CORRECT AMB
            if (gobs1.is_phase() && (!obsdata.is_carrier_range(b1) || !obsdata.is_carrier_range(b2)))
            {
                par_type amb_type = par_type::NO_DEF;
                if (_freq_index[gsys][b2] == FREQ_2)
                    amb_type = par_type::AMB_IF;
                if (_freq_index[gsys][b2] == FREQ_3)
                    amb_type = par_type::AMB13_IF;
                if (_freq_index[gsys][b2] == FREQ_4)
                    amb_type = par_type::AMB14_IF;
                if (_freq_index[gsys][b2] == FREQ_5)
                    amb_type = par_type::AMB15_IF;

                int idx = params.getParam(obsdata.site(), amb_type, obsdata.sat());
                if (idx < 0)
                    return false;

                // if amb is new  then init value with Obs - Modelobs
                if (double_eq(params[idx].value(), 0.0) || params[idx].beg == epoch)
                {
                    double obs_P3 = obsdata.P3(gobsP1, gobsP2);
                    double obs_L3 = obsdata.L3(gobsL1, gobsL2);

                    params[idx].value(obs_L3 - obs_P3);
                    //params[idx].apriori(1e-9); //xjhan test
                }
                // update B l
                coef_IF.emplace_back(idx + 1, 1.0);
                l_IF -= params[idx].value();
            }

            // ========================================================================================================
            // add 13 CLK
            if (!_add_IF_multi_rec_clk(_freq_index[gsys][b2], obsdata, params, coef_IF))
                return false;

             t_gobscombtype type(gobs1, b1, b2, _freq_index[gsys][b1], _freq_index[gsys][b2], OBSCOMBIN::IONO_FREE);
             equ_IF.add_equ(coef_IF, P_IF, l_IF, obsdata.site(), obsdata.sat(), type, false);
        }

        if (dynamic_cast<t_gfltEquationMatrix *>(&result))
        {
            auto *lsq_result = dynamic_cast<t_gfltEquationMatrix *>(&result);
            lsq_result->add_equ(equ_IF);
        }
        else
        {
            result = result + equ_IF;
        }

        return true;
    }

    bool t_gcombIF::_add_IF_multi_rec_clk(const FREQ_SEQ &freq, t_gsatdata &obsdata, t_gallpar &params, vector<pair<int, double>> &coef_IF)
    {
        auto gsys = obsdata.gsys();
        auto site = obsdata.site();
        if (freq <= FREQ_2)
            return true;

        // ???12???????
        for (auto iter = coef_IF.begin(); iter != coef_IF.end();)
        {
            par_type type = params[iter->first - 1].parType;
            if (type == par_type::CLK ||
                type == par_type::GAL_ISB ||
                type == par_type::BDS_ISB ||
                type == par_type::QZS_ISB)
            {
                iter = coef_IF.erase(iter);
                continue;
            }
            iter++;
        }

        par_type clk_type = par_type::NO_DEF;
        if (_clk_type_index.find(make_pair(freq, gsys)) != _clk_type_index.end())
        {
            clk_type = _clk_type_index[make_pair(freq, gsys)];
        }

        int idx = params.getParam(site, clk_type, "");
        if (idx < 0)
        {
            throw runtime_error(" Can not find clk par in t_gcombIF::_add_IF_multi_rec_clk");
            return false;
        }

        //update_value
        int idx_clk12 = params.getParam(site, par_type::CLK, "");
        params[idx].value(params[idx_clk12].value());
        coef_IF.emplace_back(idx + 1, 1.0 - obsdata.drate());
        return true;
    }

    t_gcombALL::t_gcombALL(t_gsetbase *setting, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data) : t_gcombmodel(setting, std::move(bias_model), data)
    {
        ambtype_list[FREQ_1] = par_type::AMB_L1;
        ambtype_list[FREQ_2] = par_type::AMB_L2;
        ambtype_list[FREQ_3] = par_type::AMB_L3;
        ambtype_list[FREQ_4] = par_type::AMB_L4;
        ambtype_list[FREQ_5] = par_type::AMB_L5;
    }

    t_gcombALL::t_gcombALL(t_gsetbase *setting, t_spdlog spdlog, shared_ptr<t_gbiasmodel> bias_model, t_gallproc *data) : t_gcombmodel(setting, spdlog, std::move(bias_model), data)
    {
        ambtype_list[FREQ_1] = par_type::AMB_L1;
        ambtype_list[FREQ_2] = par_type::AMB_L2;
        ambtype_list[FREQ_3] = par_type::AMB_L3;
        ambtype_list[FREQ_4] = par_type::AMB_L4;
        ambtype_list[FREQ_5] = par_type::AMB_L5;
    }

    t_gcombALL::~t_gcombALL() = default;

    bool t_gcombALL::cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gbaseEquation &result)
    {
        if (_gallbias!=nullptr)
            obsdata.apply_bias(_gallbias);

        // check band_list
        map<FREQ_SEQ, GOBSBAND> &crt_bands = _band_index[obsdata.gsys()];

        string grec = obsdata.site();
        string gsat = obsdata.sat();
        GSYS gsys = obsdata.gsys();

        if (crt_bands.empty())
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "crt bands is empty for ==> " + t_gsys::gsys2str(gsys));
            return false;
        }

        for (const auto &iter : crt_bands)
        {
            t_gfltEquationMatrix equ_ALL;
            vector<pair<int, double>> coefP;
            vector<pair<int, double>> coefL;

            GOBSBAND band = iter.second;
            t_gobs obsP(obsdata.select_range(band, true));
            t_gobs obsL(obsdata.select_phase(band, true));

            // check Obs Valid
            auto freq = iter.first;
            if (freq > _frequency)
                continue; 

            // check Obs Valid
            if (obsP.type() != TYPE_C && obsP.type() != TYPE_P)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, "check your obs file, we have no range obs for " + gobsband2str(band));
                if (freq == FREQ_1 && _observ != gnut::OBSCOMBIN::RAW_MIX)
                    return false; // skip sat without freq_1 observation
                else
                    continue;
            }

            if (obsL.type() != TYPE_L)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, "check your obs file, we have no phase obs for " + gobsband2str(band));
                if (freq == FREQ_1 && _observ != gnut::OBSCOMBIN::RAW_MIX)
                    return false;
                else
                    continue;
            }

            if (_frequency == 1 && freq >= FREQ_2)
                continue;
            if (_frequency == 2 && freq >= FREQ_3)
                continue;
            if (_frequency == 3 && freq >= FREQ_4)
                continue;
            if (_frequency == 4 && freq >= FREQ_5)
                continue;
            if (freq >= FREQ_6)
                continue;

            t_gbaseEquation tempP;
            if (!_bias_model->cmb_equ(epoch, params, obsdata, obsP, tempP))
                continue;
            t_gbaseEquation tempL;
            if (!_bias_model->cmb_equ(epoch, params, obsdata, obsL, tempL))
                continue;

            // add AMB
            int idx = params.getParam(grec, ambtype_list[_freq_index[gsys][band]], gsat);
            if (idx >= 0)
            {
                // if amb is new  then init value with Obs - Modelobs
                if (double_eq(params[idx].value(), 0.0) || params[idx].beg == epoch)
                {
                    double obs_P = obsdata.obs_C(obsP);
                    double obs_L = obsdata.obs_L(obsL);
                    params[idx].value(obs_L - obs_P);
                }

                tempL.B[0].push_back(make_pair(idx + 1, 1.0));
                tempL.l[0] -= params[idx].value();
                //ifcb correction
                if (_ifcb_model == IFCB_MODEL::COR && _frequency >= 3 && freq == FREQ_3)
                {
                    tempL.l[0] += dynamic_cast<t_gprecisebias *>(_bias_model.get())->ifcbDelay(obsdata, nullptr, OBSCOMBIN::RAW_ALL);
                }
            }
            else
            {
                if (_spdlog)
                    SPDLOG_LOGGER_ERROR(_spdlog, "check your spdlog file, the amb idx < 0 for :" + grec + "_" + gsat);
                continue;
            }

            if (idx < 0)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_ERROR(_spdlog, "check your spdlog file, the amb idx < 0 for :" + grec + "_" + gsat);
                return false;
            }

            t_gobscombtype typeP(obsP, band, freq, OBSCOMBIN::RAW_ALL);
            equ_ALL.add_equ(tempP.B[0], tempP.P[0], tempP.l[0], obsdata.site(), obsdata.sat(), typeP, false);
            t_gobscombtype typeL(obsL, band, freq, OBSCOMBIN::RAW_ALL);
            equ_ALL.add_equ(tempL.B[0], tempL.P[0], tempL.l[0], obsdata.site(), obsdata.sat(), typeL, false);

            // add result
            if (dynamic_cast<t_gfltEquationMatrix *>(&result))
            {
                auto *lsq_reuslt = dynamic_cast<t_gfltEquationMatrix *>(&result);
                lsq_reuslt->add_equ(equ_ALL);
            }
            else
            {
                result = result + equ_ALL;
            }
        }
        if (!result.l.size())
            return false; 
        return true;
    }

    t_gcombDD::t_gcombDD(t_gsetbase *setting, const shared_ptr<t_gbiasmodel> &bias_model, t_gallproc *data) : t_gcombmodel(setting, bias_model, data),
                                                                                                              t_gcombALL(setting, bias_model, data),
                                                                                                              t_gcombIF(setting, bias_model, data)
    {
        _data_base = nullptr;
        _site = "";
        _site_base = "";
    }

    t_gcombDD::t_gcombDD(t_gsetbase *setting, t_spdlog spdlog, const shared_ptr<t_gbiasmodel> &bias_model, t_gallproc *data) : t_gcombmodel(setting, spdlog, bias_model, data),
                                                                                                                               t_gcombALL(setting, spdlog, bias_model, data),
                                                                                                                               t_gcombIF(setting, spdlog, bias_model, data)
    {
        _data_base = nullptr;
        _site = "";
        _site_base = "";
    }

    t_gcombDD::~t_gcombDD() = default;

    void t_gcombDD::set_observ(OBSCOMBIN observ)
    {
        _observ = observ;
    }

    void t_gcombDD::set_base_data(vector<t_gsatdata> *data_base)
    {
        _data_base = data_base;
    }

    void t_gcombDD::set_site(const string &site, const string &site_base)
    {
        _site = site;
        _site_base = site_base;
    }

    void t_gcombDD::set_rec_info(const t_gtriple &xyz_base, double clk_rover, double clk_base)
    {
        _crd_base = xyz_base;
        _clk_rover = clk_rover;
        _clk_base = clk_base;
    }

    bool t_gcombDD::cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gbaseEquation &result)
    {
        t_gsatdata obsdata_other(_spdlog);
        if (_data_base == nullptr)
            return false;
        for (auto &i : *_data_base)
        {
            if (obsdata.sat() == i.sat())
                obsdata_other = i;
        }
        if (obsdata_other.sat().empty())
            return false;
        unsigned npar_orig = params.parNumber();
        t_gallpar params_temp;
        _temp_params(params, params_temp);
        if (_observ == OBSCOMBIN::IONO_FREE)
        {
            // check Obs type
            GOBSBAND b1 = _band_index[obsdata.gsys()][FREQ_1];
            GOBSBAND b2 = _band_index[obsdata.gsys()][FREQ_2];

            t_gobs gobsP1(obsdata.select_range(b1));
            t_gobs gobsP2(obsdata.select_range(b2));
            t_gobs gobsL1(obsdata.select_phase(b1));
            t_gobs gobsL2(obsdata.select_phase(b2));

            t_gobs gobsP1_other(obsdata_other.select_range(b1));
            t_gobs gobsP2_other(obsdata_other.select_range(b2));
            t_gobs gobsL1_other(obsdata_other.select_phase(b1));
            t_gobs gobsL2_other(obsdata_other.select_phase(b2));

            vector<pair<t_gobs, t_gobs>> type_list;
            type_list.emplace_back(gobsP1, gobsP2);
            type_list.emplace_back(gobsL1, gobsL2);

            vector<pair<t_gobs, t_gobs>> type_list_other;
            type_list_other.emplace_back(gobsP1_other, gobsP2_other);
            type_list_other.emplace_back(gobsL1_other, gobsL2_other);

            // IF coef
            double coef1, coef2;
            obsdata.coef_ionofree(b1, coef1, b2, coef2);

            t_gfltEquationMatrix equ_IF;

            // meter [Code and Phase]
            for (int i = 0; i < type_list.size(); i++)
            {
                t_gobs gobs1 = type_list[i].first;
                t_gobs gobs2 = type_list[i].second;
                t_gobs gobs1_other = type_list_other[i].first;
                t_gobs gobs2_other = type_list_other[i].second;

                //combine P1 and P2
                t_gbaseEquation temp_equ;
                t_gtime crt = obsdata.epoch();
                if (!_bias_model->cmb_equ(crt, params_temp, obsdata, gobs1, temp_equ))
                {
                    /*return false;*/
                    continue; 
                }
                if (!_bias_model->cmb_equ(crt, params_temp, obsdata, gobs2, temp_equ))
                {
                    //return false;
                    continue; 
                }
                t_gbaseEquation temp_equ_other;
                t_gtime crt_other = obsdata_other.epoch();
                if (!_bias_model->cmb_equ(crt_other, params_temp, obsdata_other, gobs1_other, temp_equ_other))
                {
                    //return false;
                    continue; 
                }
                if (!_bias_model->cmb_equ(crt_other, params_temp, obsdata_other, gobs2_other, temp_equ_other))
                {
                    //return false;
                    continue; 
                }
                vector<pair<int, double>> coef_IF;
                double P_IF = 0.0, l_IF = 0.0;

                if (temp_equ.B[0].size() != temp_equ.B[1].size())
                {
                    throw logic_error("coeff size is not equal in f1 and f2");
                }
                for (int j = 0; j < temp_equ.B[0].size(); j++)
                {
                    if (temp_equ.B[0][j].first != temp_equ.B[1][j].first)
                    {
                        throw logic_error("coeff par is not the same in f1 and f2");
                    }
                    // combine coeff
                    if (temp_equ.B[0][j].first > npar_orig)
                        continue;
                    coef_IF.emplace_back(temp_equ.B[0][j].first, coef1 * temp_equ.B[0][j].second + coef2 * temp_equ.B[1][j].second);
                }

                if (temp_equ_other.B[0].size() != temp_equ_other.B[1].size())
                {
                    throw logic_error("coeff size is not equal in f1 and f2");
                }
                for (int j = 0; j < temp_equ_other.B[0].size(); j++)
                {
                    if (temp_equ_other.B[0][j].first != temp_equ_other.B[1][j].first)
                    {
                        throw logic_error("coeff par is not the same in f1 and f2");
                    }
                    // combine coeff
                    if (temp_equ_other.B[0][j].first > npar_orig)
                        continue;
                    coef_IF.emplace_back(temp_equ_other.B[0][j].first, -(coef1 * temp_equ_other.B[0][j].second + coef2 * temp_equ_other.B[1][j].second));
                }

                // combine P
                P_IF = 1.0 / (pow(coef1, 2) / temp_equ.P[0] + pow(coef2, 2) / temp_equ.P[1]);
                // combine l
                l_IF = coef1 * temp_equ.l[0] + coef2 * temp_equ.l[1] - (coef1 * temp_equ_other.l[0] + coef2 * temp_equ_other.l[1]);

                // CORRECT AMB
                if (gobs1.is_phase())
                {
                    par_type amb_type = par_type::AMB_IF;
                    if (_freq_index[obsdata.gsys()][b2] == FREQ_3)
                    {
                        amb_type = par_type::AMB13_IF;
                    }
                    if (_freq_index[obsdata.gsys()][b2] == FREQ_4)
                    {
                        amb_type = par_type::AMB14_IF;
                    }
                    if (_freq_index[obsdata.gsys()][b2] == FREQ_5)
                    {
                        amb_type = par_type::AMB15_IF;
                    }

                    int idx = params_temp.getParam(obsdata.site(), amb_type, obsdata.sat());

                    if (idx < 0)
                    {
                        return false;
                    }

                    // if amb is new  then init value with Obs - Modelobs
                    if (double_eq(params_temp[idx].value(), 0.0) || params_temp[idx].beg == epoch)
                    {
                        double obs_P3 = obsdata.P3(gobsP1, gobsP2);
                        double obs_L3 = obsdata.L3(gobsL1, gobsL2);
                        params_temp[idx].value(obs_L3 - obs_P3);
                    }

                    // update B l
                    coef_IF.emplace_back(idx + 1, 1.0);
                    l_IF -= params_temp[idx].value();
                }

                // CORRECT CLK
                //remove 12 clk and isb
                if (_freq_index[obsdata.gsys()][b2] >= FREQ_3)
                {
                    for (auto iter = coef_IF.begin(); iter != coef_IF.end();)
                    {
                        par_type type = params_temp[iter->first - 1].parType;
                        if (type == par_type::CLK ||
                            type == par_type::GAL_ISB ||
                            type == par_type::BDS_ISB ||
                            type == par_type::QZS_ISB)
                        {
                            iter = coef_IF.erase(iter);
                            continue;
                        }
                        iter++;
                    }
                }

                // add 13 CLK
                if (_freq_index[obsdata.gsys()][b2] == FREQ_3)
                {
                    par_type clk_type;
                    if (obsdata.gsys() == GPS)
                        clk_type = par_type::CLK13_G;
                    else if (obsdata.gsys() == GAL)
                        clk_type = par_type::CLK13_E;
                    else if (obsdata.gsys() == BDS)
                        clk_type = par_type::CLK13_C;
                    else if (obsdata.gsys() == QZS)
                        clk_type = par_type::CLK13_J;
                    else
                        return false;

                    int idx = params_temp.getParam(obsdata.site(), clk_type, "");
                    if (idx < 0)
                    {
                        return false;
                    }
                    //update_value
                    int idx_clk12 = params_temp.getParam(obsdata.site(), par_type::CLK, obsdata.sat());
                    params_temp[idx] = params_temp[idx_clk12];

                    coef_IF.emplace_back(idx + 1, 1.0 - obsdata.drate());
                }

                // add 14 15 ... clk
                // ....
                t_gobscombtype type(gobs1, OBSCOMBIN::IONO_FREE);
                equ_IF.add_equ(coef_IF, P_IF, l_IF, obsdata.site(), obsdata.sat(), type, false);
            }

            if (dynamic_cast<t_gfltEquationMatrix *>(&result))
            {
                auto *lsq_result = dynamic_cast<t_gfltEquationMatrix *>(&result);
                lsq_result->add_equ(equ_IF);
            }
            else
            {
                result = result + equ_IF;
            }
        }
        else if (_observ == OBSCOMBIN::RAW_ALL || _observ == OBSCOMBIN::RAW_MIX)
        {
            // check band_list
            map<FREQ_SEQ, GOBSBAND> crt_bands = _band_index[obsdata.gsys()];
            map<FREQ_SEQ, par_type> ambtype_list = {
                {FREQ_1, par_type::AMB_L1},
                {FREQ_2, par_type::AMB_L2},
                {FREQ_3, par_type::AMB_L3},
                {FREQ_4, par_type::AMB_L4},
                {FREQ_5, par_type::AMB_L5}};

            if (crt_bands.empty())
            {
                return false;
            }
            t_gfltEquationMatrix equ_ALL;
            t_gbaseEquation tempP, tempL;
            vector<int> index(10, 0); 

            for (int isite = 0; isite < 2; isite++)
            {
                t_gsatdata *satdata_ptr;
                if (isite == 0)
                    satdata_ptr = &obsdata_other;
                else
                    satdata_ptr = &obsdata;

                for (const auto &iter : crt_bands)
                {
                    if (iter.first > _frequency)
                        continue; 

                    GOBSBAND band = iter.second;
                    t_gobs obsP(satdata_ptr->select_range(band));
                    t_gobs obsL(satdata_ptr->select_phase(band));

                    // check Obs Valid
                    // for P code modified by zhshen
                    if (obsP.type() != TYPE_C && obsP.type() != TYPE_P)
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_INFO(_spdlog, "check your obs file, we have no range obs for " + gobsband2str(band));
                        continue; 
                    }

                    if (obsL.type() != TYPE_L)
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_INFO(_spdlog, "check your obs file, we have no phase obs for " + gobsband2str(band));
                        continue; 
                    }

                    // jdhuang add for freq3 check
                    auto freq = iter.first;
                    t_gtime crt = satdata_ptr->epoch();
                    if (!_bias_model->cmb_equ(crt, params_temp, *satdata_ptr, obsP, tempP))
                    {
                        continue; 
                    }

                    if (!_bias_model->cmb_equ(crt, params_temp, *satdata_ptr, obsL, tempL))
                    {
                        continue; 
                    }
                    index[isite * crt_bands.size() + freq - 1] = tempL.l.size();
                    // add AMB for ROVER
                    if (satdata_ptr->site() == _site)
                    {
                        int idx = params_temp.getParam(satdata_ptr->site(), ambtype_list[_freq_index[satdata_ptr->gsys()][band]], satdata_ptr->sat());

                        if (idx < 0)
                        {
                            return false;
                        }
                        // if amb is new  then init value with Obs - Modelobs
                        if (double_eq(params_temp[idx].value(), 0.0) || params_temp[idx].beg == epoch)
                        {
                            //pars[idx].value(obs - ModelObs);
                            double obs_P = satdata_ptr->obs_C(obsP);
                            double obs_L = satdata_ptr->obs_L(obsL);

                            params_temp[idx].value(obs_L - obs_P);
                        }
                        tempL.B.back().push_back(make_pair(idx + 1, 1.0));
                        tempL.l.back() -= params_temp[idx].value();
                    }
                }
            }

            int freq_count = 0;
            for (const auto &iter : crt_bands)
            {
                if (iter.first > _frequency)
                    continue; 

                freq_count++;

                GOBSBAND band = iter.second;
                t_gobs obsP(obsdata.select_range(band));
                t_gobs obsL(obsdata.select_phase(band));

                vector<pair<int, double>> B_P, B_L;
                double P_P, P_L, l_P, l_L;
                int ibase = index[freq_count - 1];
                int irover = index[crt_bands.size() + freq_count - 1];
                if (!ibase || !irover)
                    continue;
                ibase -= 1;
                irover -= 1;

                for (const auto &b : tempP.B[irover])
                {
                    if (b.first > npar_orig)
                        continue;
                    B_P.push_back(b);
                }
                for (const auto &b : tempP.B[ibase])
                {
                    if (b.first > npar_orig)
                        continue;
                    B_P.emplace_back(b.first, -b.second);
                }
                for (const auto &b : tempL.B[irover])
                {
                    if (b.first > npar_orig)
                        continue;
                    B_L.push_back(b);
                }
                for (const auto &b : tempL.B[ibase])
                {
                    if (b.first > npar_orig)
                        continue;
                    B_L.emplace_back(b.first, -b.second);
                }

                P_P = 1 / (1 / tempP.P[irover] + 1 / tempP.P[ibase]);
                l_P = tempP.l[irover] - tempP.l[ibase];
                P_L = 1 / (1 / tempL.P[irover] + 1 / tempL.P[ibase]);
                l_L = tempL.l[irover] - tempL.l[ibase];

                t_gobscombtype typeP(obsP, OBSCOMBIN::RAW_ALL);
                t_gobscombtype typeL(obsL, OBSCOMBIN::RAW_ALL);
                equ_ALL.add_equ(B_P, P_P, l_P, obsdata.site(), obsdata.sat(), typeP, false);
                equ_ALL.add_equ(B_L, P_L, l_L, obsdata.site(), obsdata.sat(), typeL, false);
            }

            // add result
            if (dynamic_cast<t_gfltEquationMatrix *>(&result))
            {
                auto *lsq_reuslt = dynamic_cast<t_gfltEquationMatrix *>(&result);
                lsq_reuslt->add_equ(equ_ALL);
            }
            else
            {
                result = result + equ_ALL;
            }
        }
        for (int ipar = 0; ipar < params.parNumber(); ipar++)
            params[ipar] = params_temp[ipar];
        return true;
    }

    bool t_gcombDD::_temp_params(t_gallpar &params, t_gallpar &params_temp)
    {
        params_temp = params;
        t_gpar par_x_base;
        par_x_base.site = _site_base;
        par_x_base.parType = par_type::CRD_X;
        par_x_base.value(_crd_base[0]);
        t_gpar par_y_base;
        par_y_base.site = _site_base;
        par_y_base.parType = par_type::CRD_Y;
        par_y_base.value(_crd_base[1]);
        t_gpar par_z_base;
        par_z_base.site = _site_base;
        par_z_base.parType = par_type::CRD_Z;
        par_z_base.value(_crd_base[2]);
        t_gpar par_clk_rover;
        par_clk_rover.site = _site;
        par_clk_rover.parType = par_type::CLK;
        par_clk_rover.value(_clk_rover);
        t_gpar par_clk_base;
        par_clk_base.site = _site_base;
        par_clk_base.parType = par_type::CLK;
        par_clk_base.value(_clk_base);
        params_temp.addParam(par_x_base);
        params_temp.addParam(par_y_base);
        params_temp.addParam(par_z_base);
        params_temp.addParam(par_clk_rover);
        params_temp.addParam(par_clk_base);
        params_temp.reIndex();
        return true;
    }




}