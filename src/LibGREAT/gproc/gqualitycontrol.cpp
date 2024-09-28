/**
 * @file         gqualitycontrol.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        main about quality control 
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include <iomanip>
#include <memory>

#include "gproc/gqualitycontrol.h"
#include "gmodels/gbancroft.h"

namespace great
{

    t_gsmooth::t_gsmooth(t_gsetbase *settings)
    {
        _sampling = dynamic_cast<t_gsetgen *>(settings)->sampling();

        _band_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::GPS);
        _band_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::GAL);
        _band_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::GLO);
        _band_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::BDS);
        _band_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::QZS);
    }

    t_gbds_codebias_cor::t_gbds_codebias_cor(t_gsetbase *settings)
    {
        // set the setting pointer
        if (nullptr == settings)
        {
            spdlog::critical("your set pointer is nullptr !");
            throw logic_error("");
        }
        else
        {
            _set = settings;
        }

        _correct_bds_code_bias = dynamic_cast<t_gsetproc *>(settings)->bds_code_bias_correction();
        set<string> sys_list = dynamic_cast<t_gsetgen *>(settings)->sys();
        if (sys_list.find("BDS") == sys_list.end())
        {
            _correct_bds_code_bias = false;
        }

        _band_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::BDS);
        _band_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::GPS);
        _band_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::GAL);
        _band_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::GLO);
        _band_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(settings)->band_index(gnut::QZS);

        // set Wanninger & Beer
        _IGSO_MEO_Corr[BAND_2]["IGSO"] = {{0, -0.55}, {1, -0.40}, {2, -0.34}, {3, -0.23}, {4, -0.15}, {5, -0.04}, {6, 0.09}, {7, 0.19}, {8, 0.27}, {9, 0.35}}; // B1
        _IGSO_MEO_Corr[BAND_7]["IGSO"] = {{0, -0.71}, {1, -0.36}, {2, -0.33}, {3, -0.19}, {4, -0.14}, {5, -0.03}, {6, 0.08}, {7, 0.17}, {8, 0.24}, {9, 0.33}}; // B2
        _IGSO_MEO_Corr[BAND_6]["IGSO"] = {{0, -0.27}, {1, -0.23}, {2, -0.21}, {3, -0.15}, {4, -0.11}, {5, -0.04}, {6, 0.05}, {7, 0.14}, {8, 0.19}, {9, 0.32}}; // B3

        _IGSO_MEO_Corr[BAND_2]["MEO"] = {{0, -0.47}, {1, -0.38}, {2, -0.32}, {3, -0.23}, {4, -0.11}, {5, 0.06}, {6, 0.34}, {7, 0.69}, {8, 0.97}, {9, 1.05}}; //B1
        _IGSO_MEO_Corr[BAND_7]["MEO"] = {{0, -0.40}, {1, -0.31}, {2, -0.26}, {3, -0.18}, {4, -0.06}, {5, 0.09}, {6, 0.28}, {7, 0.48}, {8, 0.64}, {9, 0.69}}; //B2
        _IGSO_MEO_Corr[BAND_6]["MEO"] = {{0, -0.22}, {1, -0.15}, {2, -0.13}, {3, -0.10}, {4, -0.04}, {5, 0.05}, {6, 0.14}, {7, 0.27}, {8, 0.36}, {9, 0.47}}; //B3
    }

    void t_gbds_codebias_cor::apply_IGSO_MEO(const string &rec, t_gtriple &rec_crd, t_gallnav *gnav, vector<t_gsatdata> &obsdata)
    {
        if (!this->_correct_bds_code_bias || obsdata.size() == 0)
            return;

        if (rec_crd.zero())
        {
            if (!this->_recAprCoordinate(rec, rec_crd, gnav, obsdata))
                return;
        }

        // Calculate elevation and Correct Obs
        map<GOBSBAND, double> Band_cor;
        string sat_type;

        vector<t_gsatdata>::iterator data;
        for (data = obsdata.begin(); data != obsdata.end();)
        {
            if (data->getrangestate("bds_code_bias")) 
            {
                data++;
                continue;
            }
            if (data->site() != rec || data->gsys() != BDS) 
            {
                data++;
                continue;
            }
            string sat = data->sat();
            if (sat <= "C05" || sat > "C16") // BDS2 - IGSO/MEO
            {
                data++;
                continue;
            }
            if (sat == "C11" || sat == "C12" || sat == "C14")
                sat_type = "MEO";
            else
                sat_type = "IGSO";

            if (data->satcrd().zero())
            {
                if (data->addprd(gnav) < 0)
                {
                    data = obsdata.erase(data);
                    continue;
                }
            }
            t_gtriple sat_crd = data->satcrd();
            t_gtriple rec_sat_vector = sat_crd - rec_crd;
            double distance = rec_sat_vector.norm();
            double elev = (rec_crd[0] * rec_sat_vector[0] + rec_crd[1] * rec_sat_vector[1] + rec_crd[2] * rec_sat_vector[2]) / rec_crd.norm() / distance;
            elev = 90.0 - acos(elev) * 180.0 / G_PI;

            // get correction
            double elev0 = elev / 10.0;
            int elev0_int = floor(elev0);
            if (elev0_int < 0)
            {
                Band_cor[BAND_2] = _IGSO_MEO_Corr[BAND_2][sat_type][0];
                Band_cor[BAND_7] = _IGSO_MEO_Corr[BAND_7][sat_type][0];
                Band_cor[BAND_6] = _IGSO_MEO_Corr[BAND_6][sat_type][0];
            }
            else if (elev0_int >= 9)
            {
                Band_cor[BAND_2] = _IGSO_MEO_Corr[BAND_2][sat_type][9];
                Band_cor[BAND_7] = _IGSO_MEO_Corr[BAND_7][sat_type][9];
                Band_cor[BAND_6] = _IGSO_MEO_Corr[BAND_6][sat_type][9];
            }
            else
            {
                Band_cor[BAND_2] = _IGSO_MEO_Corr[BAND_2][sat_type][elev0_int] * (1.0 - elev0 + elev0_int) + _IGSO_MEO_Corr[BAND_2][sat_type][elev0_int + 1] * (elev0 - elev0_int);
                Band_cor[BAND_7] = _IGSO_MEO_Corr[BAND_7][sat_type][elev0_int] * (1.0 - elev0 + elev0_int) + _IGSO_MEO_Corr[BAND_7][sat_type][elev0_int + 1] * (elev0 - elev0_int);
                Band_cor[BAND_6] = _IGSO_MEO_Corr[BAND_6][sat_type][elev0_int] * (1.0 - elev0 + elev0_int) + _IGSO_MEO_Corr[BAND_6][sat_type][elev0_int + 1] * (elev0 - elev0_int);
            }

            vector<GOBS> obs_vec = data->obs();
            for (auto obs_type : obs_vec)
            {
                if (!t_gobs(obs_type).is_code())
                {
                    continue;
                }

                GOBSBAND b = t_gobs(obs_type).band();
                double obs_P = data->obs_C(obs_type);

                obs_P += Band_cor[b];
                data->resetobs(obs_type, obs_P);
                data->setrangestate("bds_code_bias", true);
            }

            data++;
        }
        return;
    }

    bool t_gbds_codebias_cor::_recAprCoordinate(const string &rec, t_gtriple &rec_crd, t_gallnav *gnav, vector<t_gsatdata> &obsdata)
    {
        Matrix BB;

        BB.ReSize(obsdata.size(), 4);
        BB = 0.0;
        int iobs = 0;

        vector<t_gsatdata>::iterator iter;
        for (iter = obsdata.begin(); iter != obsdata.end();)
        {
            if (iter->site() != rec) 
            {
                iter++;
                continue;
            }

            GSYS gs = iter->gsys();

            GOBSBAND b1 = _band_index[gs][FREQ_1];
            GOBSBAND b2 = _band_index[gs][FREQ_2];

            GOBS l1 = iter->select_phase(b1);
            GOBS l2 = iter->select_phase(b2);
            GOBS p1 = iter->select_range(b1);
            GOBS p2 = iter->select_range(b2);

            if (p1 == X || l1 == X || p2 == X || l2 == X)
            {
                iter++;
                continue;
            }

            double P3 = iter->P3(p1, p2);
            double L3 = iter->L3(l1, l2);

            if (double_eq(L3, 0.0) || double_eq(P3, 0.0))
            {
                iter++;
                continue;
            }

            if (iter->addprd(gnav) < 0)
            {
                iter = obsdata.erase(iter);
                continue;
            }

            iobs++;
            BB(iobs, 1) = iter->satcrd().crd(0);
            BB(iobs, 2) = iter->satcrd().crd(1);
            BB(iobs, 3) = iter->satcrd().crd(2);
            BB(iobs, 4) = P3 + iter->clk();

            iter++;
        }

        if (iobs < 4)
            return false;

        BB = BB.Rows(1, iobs); // delete zero rows

        ColumnVector vBanc;

        gbancroft(BB, vBanc);

        rec_crd[0] = vBanc(1);
        rec_crd[1] = vBanc(2);
        rec_crd[2] = vBanc(3);

        return true;
    }

    t_goutliers_process::t_goutliers_process(t_gsetbase *settings)
    {
        if (1)
        {
            //lvhb shut down the outputting in 202011
            if (_debug_outliers)
            {
                if (_debug_outliers->is_open())
                    _debug_outliers->close();
                delete _debug_outliers;
                _debug_outliers = nullptr;
            }
        }
        else
        {
            _debug_outliers = new t_giof;
            _debug_outliers->tsys(t_gtime::GPS);
            _debug_outliers->mask("debug_outliers.log");
            _debug_outliers->append(false);
        }
        _observ = dynamic_cast<t_gsetproc *>(settings)->obs_combin();
        _frequency = dynamic_cast<t_gsetproc *>(settings)->frequency();

        _freq_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::GPS);
        _freq_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::GAL);
        _freq_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::GLO);
        _freq_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::BDS);
        _freq_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::QZS);

        _single_mix = {gnut::OBSCOMBIN::RAW_MIX};
    }

    t_goutliers_process::t_goutliers_process(t_gsetbase *settings, t_spdlog spdlog) : _spdlog(spdlog)
    {
        if (1)
        {
            //lvhb shut down the outputting in 202011
            if (_debug_outliers)
            {
                if (_debug_outliers->is_open())
                    _debug_outliers->close();
                delete _debug_outliers;
                _debug_outliers = nullptr;
            }
        }
        else
        {
            _debug_outliers = new t_giof;
            _debug_outliers->tsys(t_gtime::GPS);
            _debug_outliers->mask("debug_outliers.log");
            _debug_outliers->append(false);
        }
        _observ = dynamic_cast<t_gsetproc *>(settings)->obs_combin();
        _frequency = dynamic_cast<t_gsetproc *>(settings)->frequency();

        _freq_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::GPS);
        _freq_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::GAL);
        _freq_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::GLO);
        _freq_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::BDS);
        _freq_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(settings)->freq_index(gnut::QZS);

        _single_mix = {gnut::OBSCOMBIN::RAW_MIX};
    }

    t_goutliers_process::~t_goutliers_process()
    {
        if (_debug_outliers)
        {
            if (_debug_outliers->is_open())
                _debug_outliers->close();
            delete _debug_outliers;
            _debug_outliers = nullptr;
        }
    }

    void t_goutliers_process::excludeBadObs(vector<t_gsatdata> &obsdata)
    {
        vector<t_gsatdata>::iterator it;
        bool valid_SNR, valid_Code, valid_Phase, Freq_Lack;
        set<FREQ_SEQ> nFreq;

        for (it = obsdata.begin(); it != obsdata.end();)
        {
            valid_SNR = true;
            valid_Code = true;
            valid_Phase = true;
            Freq_Lack = false;
            nFreq.clear();
            double P1 = 0.0;

            GSYS gsys = it->gsys();
            set<GOBSBAND> band_avail;

            band_avail = it->band_avail(true);

            if (band_avail.size() == 0)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_WARN(_spdlog, "{} Erase {} for [no freq]", it->epoch().str_hms(), it->sat());
                it = obsdata.erase(it);
                continue;
            }
            else
            {
                for (auto band : band_avail)
                {
                    // skip on used band
                    if (_freq_index[gsys].find(band) == _freq_index[gsys].end())
                        continue;

                    // skip exceed frequency setting
                    if (_freq_index[gsys][band] > _frequency)
                        continue;

                    // freq num count
                    if (nFreq.find(_freq_index[gsys][band]) == nFreq.end())
                        nFreq.insert(_freq_index[gsys][band]);

                    GOBS obsP = it->select_range(band);
                    GOBS obsL = it->select_phase(band);
                    GOBS obsS;

                    obsS = pha2snr(obsL);

                    double obs_snr = it->getobs(obsS);
                    if (obs_snr <= 10.0 && !double_eq(obs_snr, 0.0))
                        valid_SNR = false;

                    double obs_code = it->getobs(obsP);

                    double obs_phase = it->obs_L(obsL); // meter
                    if (_observ != OBSCOMBIN::RAW_MIX && double_eq(obs_phase, 0.0))
                        valid_Phase = false;

                    if (_single_mix.find(_observ) == _single_mix.end())
                    {
                        if (_freq_index[gsys][band] == FREQ_1)
                        {
                            P1 = obs_code;
                        }
                        else if (!double_eq(P1, 0.0))
                        {
                            if (abs(obs_code - P1) >= 200)
                                valid_Code = false;
                        }
                    }
                }

                if (nFreq.size() == 0)
                {
                    Freq_Lack = true;
                }
                else if (nFreq.size() == 1)
                {
                    if (_single_mix.find(_observ) == _single_mix.end())
                    {
                        Freq_Lack = true;
                        if (_spdlog)
                            SPDLOG_LOGGER_WARN(_spdlog, "{} frequency missing {}", it->epoch().str_hms(), it->sat());
                    }
                }
                else if (nFreq.size() < _frequency)
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_WARN(_spdlog, "{} frequency missing {}", it->epoch().str_hms(), it->sat());
                }

                if (!valid_SNR || !valid_Phase || !valid_Code || Freq_Lack)
                {
                    vector<GOBS> obss = it->obs();
                    it = obsdata.erase(it);
                    continue;
                }

                it++;
            }
        }
    }

    t_gqualitycontrol::t_gqualitycontrol(t_spdlog spdlog, t_gsetbase *settings, t_gallnav *gnav) : _gnav(gnav),
                                                                                                   _spdlog(spdlog),
                                                                                                   _bds_codebias_cor(settings),
                                                                                                   _smooth_range(settings),
                                                                                                   _outliers_proc(settings, nullptr)
    {
        // set spdlog
        if (nullptr == spdlog)
        {
            throw logic_error("your spdlog in t_gqualitycontrol::t_gqualitycontrol i nullptr");
        }
        _outliers_proc.setLog(spdlog);
    }

    t_gqualitycontrol::t_gqualitycontrol(t_gsetbase *settings, t_gallnav *gnav) : _gnav(gnav),
                                                                                  _bds_codebias_cor(settings),
                                                                                  _smooth_range(settings),
                                                                                  _outliers_proc(settings, nullptr)
    {
    }

    t_gqualitycontrol::~t_gqualitycontrol()
    {
    }

    int t_gqualitycontrol::processOneEpoch(const t_gtime &now, const string &rec, t_gtriple &rec_crd, vector<t_gsatdata> &obsdata)
    {
        this->_outliers_proc.excludeBadObs(obsdata);
        this->_bds_codebias_cor.apply_IGSO_MEO(rec, rec_crd, _gnav, obsdata);

        return 1;
    }

} // namespace
