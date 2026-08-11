/**
 * @file         gbiasmodel.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        mainly about how to cacultae B P l single
 * @version      1.0
 * @date         2024-08-29
 *
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 *
 */
#include "gmodels/gbiasmodel.h"
#include "gutils/gstring.h"
#include "gmodels/ggmf.h"
#include <gutils/gsysconv.h>

namespace great
{
    t_gbiasmodel::t_gbiasmodel(t_gsetbase* setting)
    {
        // set the setting pointer
        if (nullptr == setting)
        {
            GREAT_CRITICAL("your set pointer is nullptr !");
            throw std::logic_error("");
        }
        else
        {
            _gset = setting;
        }

        _frequency = dynamic_cast<t_gsetproc*>(setting)->frequency();
        _crd_est = dynamic_cast<t_gsetproc*>(setting)->crd_est();
        _mf_ztd = dynamic_cast<t_gsetproc*>(setting)->tropo_mf();
        _mf_grd = dynamic_cast<t_gsetproc*>(setting)->grad_mf();
        _trpModStr = dynamic_cast<t_gsetproc*>(setting)->tropo_model();
        _ion_model = dynamic_cast<t_gsetproc*>(setting)->ion_model();
        _weight = dynamic_cast<t_gsetproc*>(setting)->weighting();
        _observ = dynamic_cast<t_gsetproc*>(setting)->obs_combin();

        if (_trpModStr == gnut::TROPMODEL::SAASTAMOINEN)
        {
            _tropoModel = std::make_shared<t_saast>();
        }
        else if (_trpModStr == gnut::TROPMODEL::DAVIS)
        {
            _tropoModel = std::make_shared<t_davis>();
        }
        else if (_trpModStr == gnut::TROPMODEL::HOPFIELD)
        {
            _tropoModel = std::make_shared<t_hopf>();
        }

        _sigCodeGPS = dynamic_cast<t_gsetgnss*>(setting)->sigma_C(GSYS::GPS);
        _sigCodeGLO = dynamic_cast<t_gsetgnss*>(setting)->sigma_C(GSYS::GLO);
        _sigCodeGAL = dynamic_cast<t_gsetgnss*>(setting)->sigma_C(GSYS::GAL);
        _sigCodeBDS = dynamic_cast<t_gsetgnss*>(setting)->sigma_C(GSYS::BDS);
        _sigCodeQZS = dynamic_cast<t_gsetgnss*>(setting)->sigma_C(GSYS::QZS);
        _sigPhaseGPS = dynamic_cast<t_gsetgnss*>(setting)->sigma_L(GSYS::GPS);
        _sigPhaseGLO = dynamic_cast<t_gsetgnss*>(setting)->sigma_L(GSYS::GLO);
        _sigPhaseGAL = dynamic_cast<t_gsetgnss*>(setting)->sigma_L(GSYS::GAL);
        _sigPhaseBDS = dynamic_cast<t_gsetgnss*>(setting)->sigma_L(GSYS::BDS);
        _sigPhaseQZS = dynamic_cast<t_gsetgnss*>(setting)->sigma_L(GSYS::QZS);

        _band_index[gnut::GSYS::GPS] = dynamic_cast<t_gsetgnss*>(setting)->band_index(gnut::GSYS::GPS);
        _band_index[gnut::GSYS::GAL] = dynamic_cast<t_gsetgnss*>(setting)->band_index(gnut::GSYS::GAL);
        _band_index[gnut::GSYS::GLO] = dynamic_cast<t_gsetgnss*>(setting)->band_index(gnut::GSYS::GLO);
        _band_index[gnut::GSYS::BDS] = dynamic_cast<t_gsetgnss*>(setting)->band_index(gnut::GSYS::BDS);
        _band_index[gnut::GSYS::QZS] = dynamic_cast<t_gsetgnss*>(setting)->band_index(gnut::GSYS::QZS);

        _freq_index[gnut::GSYS::GPS] = dynamic_cast<t_gsetgnss*>(setting)->freq_index(gnut::GSYS::GPS);
        _freq_index[gnut::GSYS::GAL] = dynamic_cast<t_gsetgnss*>(setting)->freq_index(gnut::GSYS::GAL);
        _freq_index[gnut::GSYS::GLO] = dynamic_cast<t_gsetgnss*>(setting)->freq_index(gnut::GSYS::GLO);
        _freq_index[gnut::GSYS::BDS] = dynamic_cast<t_gsetgnss*>(setting)->freq_index(gnut::GSYS::BDS);
        _freq_index[gnut::GSYS::QZS] = dynamic_cast<t_gsetgnss*>(setting)->freq_index(gnut::GSYS::QZS);

        _meanpole_model = dynamic_cast<t_gsetproc*>(setting)->mean_pole_model();
    }

    t_gbiasmodel::t_gbiasmodel()
    {
    }

    t_gbiasmodel::~t_gbiasmodel()
    {
    }

    bool t_gbiasmodel::get_omc_obs_ALL(const t_gtime& crt_epo, t_gsatdata& obsdata, t_gallpar& pars, t_gobs& gobs, double& omc)
    {
        return _omc_obs_ALL(crt_epo, obsdata, pars, gobs, omc);
    }

    bool t_gbiasmodel::_wgt_obs_ALL(const t_gdata::ID_TYPE& obj_type,
                                    t_gobs& gobs1,
                                    t_gsatdata& obsdata,
                                    const double& factorP,
                                    double& wgt)
    {
        GSYS gsys = obsdata.gsys();
        GSAT gsat = obsdata.sat();

        GOBSTYPE type = gobs1.type();
        if (type != gnut::GOBSTYPE::TYPE_C && type != gnut::GOBSTYPE::TYPE_P && type != gnut::GOBSTYPE::TYPE_L)
        {
            GREAT_ERROR(" type should be TYPE_C/TYPE_P/TYPE_L");
            return false;
        }

        double factor = factorP;
        double sigRange = 0.0;
        double sigPhase = 0.0;

        if (gsys == GSYS::BDS)
        {
            factor = 2.0;
        }
        if (gsys == GSYS::BDS && t_gsys::bds_geo(gsat))
        {
            factor = 5.0;
        }

        // get the sys sigRange
        if (type == gnut::GOBSTYPE::TYPE_C || type == gnut::GOBSTYPE::TYPE_P)
        {
            if (obj_type == t_gdata::ID_TYPE::REC)
            {
                switch (gsys)
                {
                    case GSYS::GPS:
                        sigRange = _sigCodeGPS;
                        break;
                    case GSYS::GLO:
                        sigRange = _sigCodeGLO;
                        break;
                    case GSYS::GAL:
                        sigRange = _sigCodeGAL;
                        break;
                    case GSYS::BDS:
                        sigRange = _sigCodeBDS;
                        break;
                    case GSYS::QZS:
                        sigRange = _sigCodeQZS;
                        break;
                    default:
                        sigRange = 0.0;
                        return false;
                }
            }
            else
            {
                sigRange = 0.0;
                return false;
            }
        }
        else if (type == gnut::GOBSTYPE::TYPE_L)
        {
            if (obj_type == t_gdata::ID_TYPE::REC)
            {
                switch (gsys)
                {
                    case GSYS::GPS:
                        sigPhase = _sigPhaseGPS;
                        break;
                    case GSYS::GLO:
                        sigPhase = _sigPhaseGLO;
                        break;
                    case GSYS::GAL:
                        sigPhase = _sigPhaseGAL;
                        break;
                    case GSYS::BDS:
                        sigPhase = _sigPhaseBDS;
                        break;
                    case GSYS::QZS:
                        sigPhase = _sigPhaseQZS;
                        break;
                    default:
                        sigPhase = 0.0;
                        return false;
                }
            }
            else
            {
                sigPhase = 0.0;
                return false;
            }
        }
        else
        {
            GREAT_DEBUG("no obs type");
            return false;
        }

        if (obj_type == t_gdata::ID_TYPE::REC)
        {
            double leo_ele = obsdata.ele_leo();
            double leo_ele_deg = obsdata.ele_leo_deg();
            double sin_leo_ele = sin(leo_ele);
            double snr = obsdata.getobs(pl2snr(gobs1.gobs()));
            double SSI = floor(snr / 5) <= 9 ? floor(snr / 5) : 9;
            SSI = SSI >= 0 ? SSI : 9;
            switch (_weight)
            {
                case OBSWEIGHT::DEF_OBSWEIGHT:
                    std::cerr << "gspplsq: WeightObs (default) should not happened!\n";
                    break;
                case OBSWEIGHT::EQUAL:
                    factor *= 1;
                    break;
                case OBSWEIGHT::SINEL:
                    factor *= 1.0 / 2.0 / sin(leo_ele);
                    break;
                case OBSWEIGHT::SINEL2:
                    factor *= 1.0 / 2.0 / pow(sin_leo_ele, 2);
                    break;
                case OBSWEIGHT::SINEL4:
                    factor *= 1.0 / 2.0 / pow(sin_leo_ele, 4);
                    break;
                case OBSWEIGHT::PARTELE:
                    factor = (leo_ele_deg <= 30.0) ? factor * (1.0 / 2.0 / sin_leo_ele) : factor;
                    break;
                case OBSWEIGHT::SNR:
                    factor *= sqrt(134.02 * pow(10, -(snr / 17.91)));
                    break;
                case OBSWEIGHT::SNRELE:
                    factor = (leo_ele_deg <= 30.0) ? factor * (1.0 / 2.0 / sin_leo_ele) : factor;
                    factor *= SSI / 9;
                    break;
                default:
                    std::cerr << "gspplsq: we can't deal with this WeightObs method!";
                    return false;
            }
        }

        auto b1 = gobs1.band();
        if (type == gnut::GOBSTYPE::TYPE_C || type == gnut::GOBSTYPE::TYPE_P)
        {
            sigRange = sigRange /** 3*/;
        }
        else if (type == gnut::GOBSTYPE::TYPE_L)
        {
            sigPhase = sigPhase * obsdata.wavelength(b1) /** 3*/;
        }
        else
        {
            sigRange = 0.0;
            sigPhase = 0.0;
        }

        if (sigPhase == 0.0 && sigRange == 0.0)
        {
            return false;
        }

        int obsLevel = obsdata.getobsLevelFlag(gobs1.gobs());

        switch (type)
        {
            case gnut::GOBSTYPE::TYPE_L:
                if (sigPhase == 0.0)
                {
                    return false;
                }
                wgt = 1.0 / pow(factor * sigPhase, 2);
                break;
            case gnut::GOBSTYPE::TYPE_C:
            case gnut::GOBSTYPE::TYPE_P:
                if (sigRange == 0.0)
                {
                    return false;
                }
                wgt = 1.0 / pow(factor * sigRange, 2);
                if (obsdata.getoutliers(gobs1.gobs()) >= 1)
                {
                    wgt *= 0.2;
                }
                break;
            default:
                return false;
        }

        if (obsdata.site() == "COMA")
        {
            wgt *= 0.16;
        }
        return true;
    }

    bool t_gbiasmodel::_omc_obs_ALL(const t_gtime& crt_epo, t_gsatdata& obsdata, t_gallpar& pars, t_gobs& gobs, double& omc)
    {
        if (!gobs.is_code() && !gobs.is_phase())
        {
            throw std::logic_error("only support code and phase observation.");
        }

        double obs = gobs.is_code() ? obsdata.obs_C(gobs) : obsdata.obs_L(gobs);

        if (double_eq(obs, 0.0))
        {
            GREAT_ERROR("obs type is empty, the obs is zero!");
            return false;
        }

        t_gtime epo = crt_epo;
        GSAT sat_name = obsdata.sat();
        std::string site_name = obsdata.site();
        double ModelObs = cmpObs(epo, sat_name, site_name, pars, obsdata, gobs);
        if (ModelObs < 0)
        {
            GREAT_ERROR("Compute ModelObs error!");
            return false;
        }

        double Amb = 0.0;
        omc = (obs - ModelObs - Amb);
        return true;
    }

    bool t_gbiasmodel::_Partial_basic(const t_gtime& epoch, t_gsatdata& obsdata, const t_gobs& gobs, const t_gpar& par, double& partial)
    {
        partial = 0.0;

        switch (par.parType)
        {
            case par_type::CRD_X:
                if (obsdata.site() == par.site)
                {
                    partial = (par.value() - obsdata.satcrd().crd(0)) / obsdata.rho();
                }
                break;
            case par_type::CRD_Y:
                if (obsdata.site() == par.site)
                {
                    partial = (par.value() - obsdata.satcrd().crd(1)) / obsdata.rho();
                }
                break;
            case par_type::CRD_Z:
                if (obsdata.site() == par.site)
                {
                    partial = (par.value() - obsdata.satcrd().crd(2)) / obsdata.rho();
                }
                break;
            case par_type::CLK:
                if (obsdata.site() == par.site)
                {
                    partial = 1.0 - obsdata.drate();
                }
                break;
            case par_type::TRP:
                if (obsdata.site() == par.site)
                {
                    double mfw, dmfw, mfh, dmfh;
                    mfw = dmfw = mfh = dmfh = 0.0;
                    t_gtriple ell(0.0, 0.0, 0.0);
                    xyz2ell(_trs_rec_crd, ell, false);
                    _getmf(par, obsdata, ell, epoch, mfw, mfh, dmfw, dmfh);
                    partial = mfw;
                }
                break;
            case par_type::SION:
                if (obsdata.site() == par.site)
                {
                    auto gsys = obsdata.gsys();
                    double f1 = obsdata.frequency(_band_index[gsys][gnut::FREQ_SEQ::FREQ_1]);
                    double fk = obsdata.frequency(gobs.band());
                    double alfa = 0.0;
                    if (gobs.is_phase() && par.prn == obsdata.sat())
                    {
                        alfa = -(f1 * f1) / (fk * fk);
                    }
                    if (gobs.is_code() && par.prn == obsdata.sat())
                    {
                        alfa = (f1 * f1) / (fk * fk);
                    }
                    partial = alfa;
                }
                break;
            case par_type::VION:
                if (obsdata.site() == par.site)
                {
                    auto gsys = obsdata.gsys();
                    double f1 = obsdata.frequency(_band_index[gsys][gnut::FREQ_SEQ::FREQ_1]);
                    double fk = obsdata.frequency(gobs.band());
                    double mf = 1.0 / sqrt(1.0 - pow(R_SPHERE / (R_SPHERE + 450000.0) * sin(G_PI / 2.0 - obsdata.ele()), 2));
                    double alfa = 0.0;
                    if (gobs.is_phase() && par.prn == obsdata.sat())
                    {
                        alfa = -(f1 * f1) / (fk * fk);
                    }
                    if (gobs.is_code() && par.prn == obsdata.sat())
                    {
                        alfa = (f1 * f1) / (fk * fk);
                    }
                    partial = alfa * mf;
                }
                break;
            case par_type::GLO_ISB:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::GLO)
                {
                    partial = 1.0;
                }
                break;
            case par_type::GLO_IFCB:
                if (obsdata.site() == par.site && !gobs.is_phase() && obsdata.gsys() == GSYS::GLO && par.prn == obsdata.sat())
                {
                    partial = 1.0;
                }
                break;
            case par_type::GLO_IFPB:
                if (obsdata.site() == par.site && gobs.is_phase() && obsdata.gsys() == GSYS::GLO && par.prn == obsdata.sat())
                {
                    partial = 1.0;
                }
                break;
            case par_type::GLO_IFB:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::GLO && par.prn == obsdata.sat() && gobs.is_code())
                {
                    partial = 1.0;
                }
                break;
            case par_type::GAL_ISB:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::GAL)
                {
                    partial = 1.0;
                }
                break;
            case par_type::BDS_ISB:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::BDS)
                {
                    partial = 1.0;
                }
                break;
            case par_type::QZS_ISB:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::QZS)
                {
                    partial = 1.0;
                }
                break;
            case par_type::IFB_QZS:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::QZS && _freq_index[obsdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_3 &&
                    gobs.is_code())
                {
                    partial = 1.0;
                }
                break;
            case par_type::IFB_GPS:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::GPS && _freq_index[obsdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_3 &&
                    gobs.is_code())
                {
                    partial = 1.0;
                }
                break;
            case par_type::IFB_GAL:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::GAL && _freq_index[obsdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_3 &&
                    gobs.is_code())
                {
                    partial = 1.0;
                }
                break;
            case par_type::IFB_GAL_2:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::GAL && _freq_index[obsdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_4 &&
                    gobs.is_code())
                {
                    partial = 1.0;
                }
                break;
            case par_type::IFB_GAL_3:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::GAL && _freq_index[obsdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_5 &&
                    gobs.is_code())
                {
                    partial = 1.0;
                }
                break;
            case par_type::IFB_BDS:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::BDS && _freq_index[obsdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_3 &&
                    gobs.is_code())
                {
                    partial = 1.0;
                }
                break;
            case par_type::IFB_BDS_2:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::BDS && _freq_index[obsdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_4 &&
                    gobs.is_code())
                {
                    partial = 1.0;
                }
                break;
            case par_type::IFB_BDS_3:
                if (obsdata.site() == par.site && obsdata.gsys() == GSYS::BDS && _freq_index[obsdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_5 &&
                    gobs.is_code())
                {
                    partial = 1.0;
                }
                break;
            default:
                return false;
        }
        return true;
    }

    void t_gbiasmodel::_getmf(const t_gpar& par,
                              t_gsatdata& satData,
                              const t_gtriple& crd,
                              const t_gtime& epoch,
                              double& mfw,
                              double& mfh,
                              double& dmfw,
                              double& dmfh)
    {
        if (par.parType != par_type::TRP && par.parType != par_type::GRD_N && par.parType != par_type::GRD_E)
        {
            return;
        }

        double ele = satData.ele();

        if (_mf_ztd == ZTDMPFUNC::COSZ)
        {
            mfw = mfh = 1.0 / sin(ele);
        }
        else if (_mf_ztd == ZTDMPFUNC::GMF)
        {
            t_gmf mf;
            mf.gmf(epoch.mjd(), crd[0], crd[1], crd[2], G_PI / 2.0 - ele, mfh, mfw, dmfh, dmfw);
        }
        else
        {
            std::cerr << "ZTD mapping function is not set up correctly!!!" << std::endl;
        }
    }
} // namespace great
