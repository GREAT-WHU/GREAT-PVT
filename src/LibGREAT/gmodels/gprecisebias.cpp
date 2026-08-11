/**
 * @file         gprecisebias.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        mainly about precise bias
 * @version      1.0
 * @date         2024-08-29
 *
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 *
 */
#include "gmodels/gprecisebias.h"
#include "gall/gallnav.h"
#include "gset/gsetgen.h"
#include "gmodels/ggmf.h"
#include "gmodels/gtideIERS.h"
#include "gall/gallprec.h"
#include "gutils/gmatrixconv.h"

#include <cmath>

namespace great
{
    t_gprecisebias::t_gprecisebias(t_gallproc* data, t_gsetbase* setting) :
        t_gbiasmodel(setting)
    {
        _gall_nav = dynamic_cast<t_gallnav*>((*data)[t_gdata::ID_GROUP::GRP_EPHEM]);
        _gallobj = dynamic_cast<t_gallobj*>((*data)[t_gdata::ID_TYPE::ALLOBJ]);
        _gdata_erp = dynamic_cast<t_gpoleut1*>((*data)[t_gdata::ID_TYPE::ALLPOLEUT1]);
        _tide = std::shared_ptr<t_gtide>(new t_gtideIERS(dynamic_cast<t_gallotl*>((*data)[t_gdata::ID_TYPE::ALLOTL])));
        _minElev = dynamic_cast<t_gsetproc*>(setting)->minimum_elev();
        _crd_est = dynamic_cast<t_gsetproc*>(setting)->crd_est();
        _is_flt = (dynamic_cast<t_gsetgen*>(setting)->estimator() == "FLT");
        _trop_est = dynamic_cast<t_gsetproc*>(setting)->tropo();
        if (dynamic_cast<t_gallprec*>(_gall_nav))
        {
            _corrt_sat_pcv = !(dynamic_cast<t_gsetinp*>(_gset)->input_size("sp3") == 0);
            if (dynamic_cast<t_gsetinp*>(_gset)->input_size("orbit") != 0)
            {
                _corrt_sat_pcv = true;
            }
        }
        _gifcb = dynamic_cast<t_gifcb*>((*data)[t_gdata::ID_TYPE::IFCB]);
        _attitudes = dynamic_cast<t_gsetproc*>(setting)->attitudes();
        _opl = dynamic_cast<t_gallopl*>((*data)[t_gdata::ID_TYPE::ALLOPL]);
        _mean_pole_model = dynamic_cast<t_gsetproc*>(setting)->mean_pole_model();
    }

    t_gprecisebias::~t_gprecisebias()
    {
    }

    bool t_gprecisebias::cmb_equ(t_gtime& epoch, t_gallpar& params, t_gsatdata& obsdata, t_gobs& gobs, t_gbaseEquation& result)
    {
        return false;
    }

    void t_gprecisebias::update_obj_clk(const std::string& obj, const t_gtime& epo, double clk)
    {
        _obj_clk[obj].first = epo;
        _obj_clk[obj].second = clk;
        _rec_clk[obj] = clk;
    }

    double t_gprecisebias::get_rec_clk(const std::string& obj)
    {
        return _rec_clk[obj];
    }

    double t_gprecisebias::tropoDelay(t_gtime& epoch, std::string& rec, t_gallpar& param, t_gtriple site_ell, t_gsatdata& satdata)
    {
        if (site_ell[2] > 1E4)
        {
            return 0.0;
        }
        else
        {
            if (_tropoModel == 0)
            {
                GREAT_DEBUG("Tropo Model setting is not correct. Default used! Check config.");
                _tropoModel = std::make_shared<t_saast>();
            }

            double ele = satdata.ele();
            double azi = satdata.azi();

            double delay = 0.0;
            double zwd = 0.0;
            double zhd = 0.0;
            int i, j, k;
            t_gtriple ell;

            xyz2ell(_trs_rec_crd, ell, false);

            if (ell[2] > 1E4)
            {
                return 100.0;
            }

            i = param.getParam(_crt_rec, par_type::TRP, "");
            j = param.getParam(_crt_rec, par_type::GRD_N, "");
            k = param.getParam(_crt_rec, par_type::GRD_E, "");

            if (i >= 0)
            {
                zwd = param[i].value();
                if (param[i].apriori() > 1E-4 && (zwd == 0.0 || epoch == param[i].beg))
                {
                    zwd = _tropoModel->getZWD(ell, epoch);
                    param[i].value(zwd);
                }
                zhd = _tropoModel->getZHD(ell, epoch);
                param[i].zhd = zhd;
            }
            else
            {
                if (_tropoModel != 0)
                {
                    zwd = _tropoModel->getZWD(ell, epoch);
                    zhd = _tropoModel->getZHD(ell, epoch);
                }
            }

            double mfh, mfw, dmfh, dmfw;
            mfh = mfw = dmfh = dmfw = 0.0;
            if (_mf_ztd == ZTDMPFUNC::GMF)
            {
                t_gmf mf;
                mf.gmf(epoch.mjd(), ell[0], ell[1], ell[2], G_PI / 2.0 - ele, mfh, mfw, dmfh, dmfw);
            }
            else if (_mf_ztd == ZTDMPFUNC::COSZ)
            {
                mfh = mfw = 1 / sin(ele);
                dmfh = dmfw = -(cos(ele)) / (sin(ele) * sin(ele));
            }
            else
            {
                return 0.0;
            }

            satdata.addmfH(mfh);
            satdata.addmfW(mfw);

            delay = mfh * zhd + mfw * zwd;

            if (!_trop_est && satdata.obsWithCorr())
            {
                delay = mfh * zhd;
            }

            double grdN, grdE;
            grdN = grdE = 0.0;

            if (j >= 0 && k >= 0)
            {
                if (_grad_mf == GRDMPFUNC::TILTING)
                {
                    grdN = param[j].value() * dmfw * cos(azi);
                    grdE = param[k].value() * dmfw * sin(azi);
                    satdata.addmfG(dmfw);
                }
                else if (_grad_mf == GRDMPFUNC::CHEN_HERRING)
                {
                    double mfg = 1.0 / (sin(ele) * tan(ele) + 0.0032);
                    grdN = param[j].value() * 1000.0 * mfg * cos(azi);
                    grdN /= 1000.0;
                    grdE = param[k].value() * 1000.0 * mfg * sin(azi);
                    grdE /= 1000.0;
                    satdata.addmfG(mfg);
                }
                else if (_grad_mf == GRDMPFUNC::BAR_SEVER)
                {
                    double mfg = mfw * (1 / tan(ele));
                    grdN = param[j].value() * mfg * cos(azi);
                    grdE = param[k].value() * mfg * sin(azi);
                    satdata.addmfG(mfg);
                }

                delay += grdN + grdE;
            }
            return delay;
        }
    }

    double t_gprecisebias::ionoDelay(t_gtime& epoch,
                                     t_gallpar& param,
                                     t_gsatdata& satdata,
                                     IONMODEL& ion_model,
                                     GOBSBAND& band_1,
                                     t_gobs& gobs)
    {
        if (band_1 == gnut::GOBSBAND::BAND || band_1 == gnut::GOBSBAND::BAND_A || band_1 == gnut::GOBSBAND::BAND_B || band_1 == gnut::GOBSBAND::BAND_C || band_1 == gnut::GOBSBAND::BAND_D)
        {
            return 0.0;
        }

        if (gobs.band() == gnut::GOBSBAND::BAND || gobs.band() == gnut::GOBSBAND::BAND_A || gobs.band() == gnut::GOBSBAND::BAND_B || gobs.band() == gnut::GOBSBAND::BAND_C || gobs.band() == gnut::GOBSBAND::BAND_D)
        {
            return 0.0;
        }

        if (ion_model == IONMODEL::VION)
        {
            throw std::logic_error(" not support VION model, use SION in XML");
        }

        double iono_delay = 0.0;

        double f1 = satdata.frequency(band_1);
        ;
        double fk = satdata.frequency(gobs.band());
        double alfa = 0.0;

        if (gobs.is_phase())
        {
            alfa = -(f1 * f1) / (fk * fk);
        }
        if (gobs.is_code())
        {
            alfa = (f1 * f1) / (fk * fk);
        }

        // ionosphere slant delay parameter
        int i = param.getParam(_crt_rec, par_type::SION, satdata.sat());
        if (i >= 0)
        {
            iono_delay = alfa * param[i].value();
        }

        return iono_delay;
    }

    double t_gprecisebias::isbDelay(t_gallpar& param, t_gsatdata& satdata, GSAT sat, std::string& rec, t_gobs& gobs)
    {
        double isb_offset = 0.0;

        auto gsys = gsat2gsys(sat);

        switch (gsys)
        {
            case GSYS::GPS:
            {
                break;
            }
            // GLONASS system time offset
            case GSYS::GLO:
            {
                int idx_isb = param.getParam(rec, par_type::GLO_ISB, "");
                if (idx_isb >= 0)
                {
                    isb_offset = param[idx_isb].value();
                }

                int idx_ifb = param.getParam(rec, par_type::GLO_IFB, sat);
                if (idx_ifb >= 0)
                {
                    isb_offset += param[idx_ifb].value();
                }

                break;
            }
            case GSYS::GAL:
            {
                // Galileo system time offset
                int i = param.getParam(rec, par_type::GAL_ISB, "");
                if (i >= 0)
                {
                    isb_offset = param[i].value();
                }
                break;
            }
            case GSYS::BDS:
            {
                // BaiDou system time offset
                int i = param.getParam(rec, par_type::BDS_ISB, "");
                if (i >= 0)
                {
                    isb_offset = param[i].value();
                }
                break;
            }
            // QZSS system time offset
            case GSYS::QZS:
            {
                int i = param.getParam(_crt_rec, par_type::QZS_ISB, "");
                if (i >= 0)
                {
                    isb_offset = param[i].value();
                }
                break;
            }
            default:
                throw std::logic_error("can not support such sys ： " + t_gsys::gsys2str(gsys));
        }

        return isb_offset;
    }
    double t_gprecisebias::relDelay(t_gtriple& crd_crt_rec, t_gtriple& vel_crt_rec, t_gtriple& crd_sat, t_gtriple& vel_sat)
    {
        double reldelay = 0.0;
        reldelay = 2.0 * (crd_sat.crd_cvect_Eigen().dot(vel_sat.crd_cvect_Eigen())) / CLIGHT;
        Eigen::Vector3d xsat = crd_sat.crd_cvect_Eigen();
        Eigen::Vector3d xsite = crd_crt_rec.crd_cvect_Eigen();

        double r = xsite.norm() + xsat.norm();

        Eigen::Vector3d xsat2site = (xsite - xsat);
        double r_crt_rec2sat = xsat2site.norm();

        reldelay += 2.0 * GM_CGCS / CLIGHT / CLIGHT * log((r + r_crt_rec2sat) / (r - r_crt_rec2sat));
        return reldelay;
    }

    bool t_gprecisebias::_update_obs_info(t_gsatdata& obsdata)
    {
        obsdata = _crt_obs;
        return true;
    }

    double t_gprecisebias::ifbDelay(t_gallpar& param, t_gsatdata& satdata, GSAT sat, std::string& rec, t_gobs& gobs)
    {
        double ifb = 0.0;
        auto gsys = gsat2gsys(sat);

        switch (gsys)
        {
            case GSYS::GPS:
            {
                int i = param.getParam(_crt_rec, par_type::IFB_GPS, "");
                if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_3)
                {
                    ifb = param[i].value();
                }
                break;
            }
            case GSYS::GAL:
            {
                int i = param.getParam(_crt_rec, par_type::IFB_GAL, "");
                if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_3)
                {
                    ifb = param[i].value();
                }
                i = param.getParam(_crt_rec, par_type::IFB_GAL_2, "");
                if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_4)
                {
                    ifb = param[i].value();
                }
                i = param.getParam(_crt_rec, par_type::IFB_GAL_3, "");
                if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_5)
                {
                    ifb = param[i].value();
                }
                break;
            }
            case GSYS::BDS:
            {
                int i = param.getParam(_crt_rec, par_type::IFB_BDS, "");
                if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_3)
                {
                    ifb = param[i].value();
                }
                i = param.getParam(_crt_rec, par_type::IFB_BDS_2, "");
                if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_4)
                {
                    ifb = param[i].value();
                }
                i = param.getParam(_crt_rec, par_type::IFB_BDS_3, "");
                if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_5)
                {
                    ifb = param[i].value();
                }
                break;
            }
            // QZSS system time offset
            case GSYS::QZS:
            {
                int i = param.getParam(_crt_rec, par_type::IFB_QZS, "");
                if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == gnut::FREQ_SEQ::FREQ_3)
                {
                    ifb = param[i].value();
                }
                break;
            }
            case GSYS::GLO:
            {
                break;
            }
            default:
                throw std::logic_error("can not support such sys ： " + t_gsys::gsys2str(gsys));
        }
        return ifb;
    }

    double t_gprecisebias::ifcbDelay(t_gsatdata& satdata, t_gifcb* ifcb, OBSCOMBIN obscombin)
    {
        double L = 0.0;
        double ifcb_GPS = 0.0;

        // lvhb added in 202100404 for rtk/nrtk/urtk
        if (ifcb == nullptr && _gifcb == nullptr)
        {
            return false;
        }

        if (ifcb == nullptr)
        {
            ifcb = _gifcb;
        }

        t_gtime epoch = satdata.epoch();
        GSAT sat = satdata.sat();

        GOBSBAND b1 = t_gsys::band_priority(satdata.gsys(), gnut::FREQ_SEQ::FREQ_1);
        GOBSBAND b3 = t_gsys::band_priority(satdata.gsys(), gnut::FREQ_SEQ::FREQ_3);

        double l1 = satdata.wavelength(b1);
        double l3 = satdata.wavelength(b3);

        double c2 = l1 / l3;
        double c1 = 1.0 / (1.0 - c2 * c2);
        c2 = c2 * c2 * c1;

        if (satdata.gsys() == GSYS::GPS)
        {
            // liugege
            one_epoch_ifcb epoch_ifcb = ifcb->get_epo_ifcb(epoch);
            if (epoch_ifcb.find(sat) == epoch_ifcb.end())
            {
                return false;
            }

            if (epoch_ifcb[sat]->sigma > 0.2 || epoch_ifcb[sat]->npoint <= 2)
            {
                return false;
            }
            ifcb_GPS = epoch_ifcb[sat]->value;
            if (obscombin == OBSCOMBIN::IONO_FREE)
            {
                L += ifcb_GPS;
            }
            else if (obscombin == OBSCOMBIN::RAW_ALL)
            {
                L += ifcb_GPS / (-c2);
            }
        }

        return L;
    }

    double t_gprecisebias::windUp(const GOBSBAND& freq_2, t_gsatdata& satdata, const Eigen::Vector3d& rRec)
    {
        double wavelength = 0.0;
        wavelength = satdata.wavelength(freq_2);

        if (double_eq(wavelength, 0.0))
        {
            throw std::runtime_error("wavelength is zero when getting windup");
        }

        if (fabs(satdata.wind()) > 0)
        {
            return satdata.wind() * wavelength;
        }
        else
        {
            t_gtime epoch = satdata.epoch();
            GSAT prn = satdata.sat();
            Eigen::Vector3d rSat = _trs_sat_crd.crd_cvect_Eigen();

            // First time - initialize to zero
            // -------------------------------
            if (_phase_windup.find(_crt_rec) == _phase_windup.end() || _phase_windup[_crt_rec].find(prn) == _phase_windup[_crt_rec].end() ||
                _phase_windup[_crt_rec][prn].size() == 0)
            {
                _phase_windup[_crt_rec][prn][epoch] = 0.0;
            }

            // Compute the correction for new time
            // -----------------------------------
            if (_phase_windup[_crt_rec][prn].find(epoch) == _phase_windup[_crt_rec][prn].end() || _phase_windup[_crt_rec][prn].size() == 1)
            {
                // the last epoch
                double dphi0 = _phase_windup[_crt_rec][prn].rbegin()->second;
                Eigen::Vector3d rho = rRec - rSat;
                rho /= rho.norm();

                Eigen::Vector3d i, j, k;

                // attitude model
                std::string antype = "";
                if (_gallobj != 0)
                {
                    std::shared_ptr<t_gobj> sat_obj = _gallobj->obj(gsat2str(satdata.sat()));
                    std::shared_ptr<t_gpcv> sat_pcv;
                    if (sat_obj != 0)
                    {
                        sat_pcv = sat_obj->pcv(satdata.epoch());
                    }
                    if (sat_pcv != 0)
                    {
                        antype = sat_pcv->anten();
                    }
                }

                _satellite_attitude(satdata, antype, i, j, k);

                if (antype.find("BLOCK IIR") != std::string::npos)
                {
                    i = (-1.0) * i;
                    j = (-1.0) * j;
                }
                double rlength = rho.dot(i);
                Eigen::Vector3d dipSat = i - rlength * rho - rho.cross(j);

                // Receiver unit Vectors rx, ry
                // ----------------------------
                Eigen::Vector3d rx;
                Eigen::Vector3d ry;

                double recEll[3];
                xyz2ell(rRec.data(), recEll, false);
                double neu[3];

                neu[0] = 1.0;
                neu[1] = 0.0;
                neu[2] = 0.0;
                neu2xyz(recEll, neu, rx.data());

                neu[0] = 0.0;
                neu[1] = -1.0;
                neu[2] = 0.0;
                neu2xyz(recEll, neu, ry.data());

                // Effective Dipole of the Receiver Antenna
                // ----------------------------------------
                rlength = rho.dot(rx);
                Eigen::Vector3d dipRec = rx - rlength * rho + rho.cross(ry);

                // Resulting Effect
                // ----------------
                double alpha = dipSat.dot(dipRec) / (dipSat.norm() * dipRec.norm());

                if (alpha > 1.0)
                {
                    alpha = 1.0;
                }
                if (alpha < -1.0)
                {
                    alpha = -1.0;
                }

                double dphi = acos(alpha) / 2.0 / G_PI; // in cycles

                if (rho.dot(dipSat.cross(dipRec)) < 0.0)
                {
                    dphi = -dphi;
                }

                _phase_windup[_crt_rec][prn][epoch] = floor(dphi0 - dphi + 0.5) + dphi;
            }

            satdata.addwind(_phase_windup[_crt_rec][prn][epoch]);
            return _phase_windup[_crt_rec][prn][epoch] * wavelength;
        }
    }

    double t_gprecisebias::PCV(bool corrt_sat,
                               bool corrt_rec,
                               t_gtime& epoch,
                               t_gtime& crt_sat_epo,
                               t_gtriple& trs_rec_crd,
                               t_gsatdata& satdata,
                               t_gobs& gobs)
    {
        // Phase center variation correction
        double pcv_R = 0.0;
        double pcv_S = 0.0;
        double pco_R = 0.0;
        double pco_S = 0.0;

        GOBSBAND band = gobs.band();

        if (_gallobj == nullptr)
        {
            throw std::runtime_error("cannot correct PCV\n");
        }

        std::shared_ptr<t_gobj> sat_obj = _gallobj->obj(gsat2str(satdata.sat()));
        std::shared_ptr<t_gobj> rec_obj = _gallobj->obj(satdata.site());

        std::shared_ptr<t_gpcv> sat_pcv = (sat_obj != nullptr) ? sat_obj->pcv(epoch) : nullptr;
        std::shared_ptr<t_gpcv> rec_pcv = (rec_obj != nullptr) ? rec_obj->pcv(epoch) : nullptr;

        if (!_isCalSatPCO)
        {
            if (sat_pcv != nullptr)
            {
                sat_pcv = nullptr;
            }
        }

        if (sat_pcv != nullptr && corrt_sat)
        {
            // Satellite phase center variation
            // -- Satellite phase center offset
            t_gtriple pco(0, 0, 0);
            if (sat_pcv->pcoS_raw(satdata, pco, band) > 0)
            {
                std::string antenna = sat_pcv->anten();
                t_gtriple dx(0, 0, 0);
                Eigen::Vector3d i, j, k;
                _satellite_attitude(satdata, antenna, i, j, k);
                dx[0] = pco[0] * i(0) + pco[1] * j(0) + pco[2] * k(0);
                dx[1] = pco[0] * i(1) + pco[1] * j(1) + pco[2] * k(1);
                dx[2] = pco[0] * i(2) + pco[1] * j(2) + pco[2] * k(2);
                sat_pcv->pco_proj(pco_S, satdata, trs_rec_crd, dx);
                satdata.addpco(dx);
            }
        }

        if (rec_pcv != nullptr && corrt_rec)
        {
            // Receiver phase center offset
            t_gtriple pco(0.0, 0.0, 0.0);
            if (rec_pcv->pcoR_raw(satdata, pco, band) > 0)
            {
                Matrix _rot_matrix = _RotMatrix_Ant(satdata, _crt_rec_epo, rec_obj);
                t_gtriple dx(_rot_matrix * (pco.crd_cvect()));
                rec_pcv->pco_proj(pco_R, satdata, trs_rec_crd, dx);
            }
            pco_R *= -1;
        }

        if (gobs.is_phase())
        {
            if (sat_pcv != 0 && corrt_sat)
            {
                // Satellite phase center variation
                sat_pcv->pcvS_raw(pcv_S, satdata, band, trs_rec_crd);
            }
            if (rec_pcv != 0 && corrt_rec)
            {
                // Receiver phase center variation
                rec_pcv->pcvR_raw(pcv_R, satdata, band);
            }
        }

        return pcv_R + pcv_S + pco_R + pco_S;
    }

    void t_gprecisebias::_satellite_attitude(t_gsatdata& satdata,
                                             const std::string& antenna,
                                             Eigen::Vector3d& i,
                                             Eigen::Vector3d& j,
                                             Eigen::Vector3d& k)
    {
        // Keep orbital angles based on the ECEF derivative, as in v1.3.0.
        satdata.beta();
        satdata.orb_angle();

        const t_gtriple trs_velocity = satdata.satvel();
        const t_gtriple position = satdata.satcrd();
        t_gtriple inertial_velocity = trs_velocity;
        inertial_velocity[0] -= OMEGA * position[1];
        inertial_velocity[1] += OMEGA * position[0];
        satdata.addvel(inertial_velocity);

        try
        {
            if (_attitudes == ATTITUDES::YAW_NOMI)
            {
                _gattitude_model.attitude(satdata, "", i, j, k);
            }
            else if (_attitudes == ATTITUDES::YAW_RTCM)
            {
                _gattitude_model.attitude(satdata, satdata.yaw(), i, j, k);
            }
            else
            {
                _gattitude_model.attitude(satdata, antenna, i, j, k);
            }
        }
        catch (...)
        {
            satdata.addvel(trs_velocity);
            throw;
        }

        satdata.addvel(trs_velocity);
    }

    Matrix t_gprecisebias::_RotMatrix_Ant(t_gsatdata& obsdata, const t_gtime& epoch, std::shared_ptr<t_gobj> obj)
    {
        t_gdata::ID_TYPE type = obj->id_type();
        Matrix rotmatrix(3, 3);
        t_gtriple ell(0.0, 0.0, 0.0);
        double sinPhi, cosPhi, sinLam, cosLam;

        if (type == t_gdata::ID_TYPE::TRN)
        {
            std::shared_ptr<t_gpcv> sat_pcv = obj->pcv(epoch);
            std::string antenna = (sat_pcv)->anten();
            t_gtriple dx(0, 0, 0);
            Eigen::Vector3d i, j, k;
            _satellite_attitude(obsdata, antenna, i, j, k);
            rotmatrix.Column(1) << i.data();
            rotmatrix.Column(2) << j.data();
            rotmatrix.Column(3) << k.data();
        }
        else if (type == t_gdata::ID_TYPE::REC)
        {
            xyz2ell(_trs_rec_crd, ell, false);
            sinPhi = sin(ell[0]);
            cosPhi = cos(ell[0]);
            sinLam = sin(ell[1]);
            cosLam = cos(ell[1]);
            rotmatrix << -sinPhi * cosLam << -sinLam << +cosPhi * cosLam << -sinPhi * sinLam << +cosLam << +cosPhi * sinLam << +cosPhi
                      << 0.0 << +sinPhi;
        }
        else
        {
            throw std::exception();
        }

        return rotmatrix;
    }

    bool t_gprecisebias::_apply_rec(const t_gtime& crt_epo, const t_gtime& rec_epo, t_gallpar& pars)
    {
        t_gtriple trs_rec_xyz(0.0, 0.0, 0.0);
        if (_crd_est == CONSTRPAR::FIX)
        {
            trs_rec_xyz = _crt_obj->crd(rec_epo);
        }
        else
        {
            int ix = pars.getParam(_crt_rec, par_type::CRD_X, "");
            int iy = pars.getParam(_crt_rec, par_type::CRD_Y, "");
            int iz = pars.getParam(_crt_rec, par_type::CRD_Z, "");

            if (ix >= 0 && iy >= 0 && iz >= 0)
            {
                trs_rec_xyz = t_gtriple(pars.getParValue(ix), pars.getParValue(iy), pars.getParValue(iz));
                if (trs_rec_xyz.zero())
                {
                    trs_rec_xyz = _crt_obj->crd(rec_epo);
                    pars.setParValue(ix, trs_rec_xyz[0]);
                    pars.setParValue(iy, trs_rec_xyz[1]);
                    pars.setParValue(iz, trs_rec_xyz[2]);
                }
            }
            else
            {
                GREAT_DEBUG("can't get the crd of" + _crt_rec + " in time" + rec_epo.str_ymdhms());
                return false;
            }
        }

        if (trs_rec_xyz.zero())
        {
            GREAT_DEBUG("can't get the crd of" + _crt_rec + " in time" + rec_epo.str_ymdhms());
            return false;
        }

        bool tide_valid = _apply_rec_tides(rec_epo, trs_rec_xyz);
        if (!tide_valid)
        {
            GREAT_DEBUG("apply tide failed for " + _crt_rec + " in time" + rec_epo.str_ymdhms());
            return false;
        }

        // ARP Correction
        if (!_is_flt || _crd_est == CONSTRPAR::FIX)
        {
            trs_rec_xyz = trs_rec_xyz + _crt_obj->eccxyz(rec_epo);
        }
        _trs_rec_crd = trs_rec_xyz;
        _trs_rec_vel = t_gtriple(0.0, 0.0, 0.0);
        return true;
    }

    bool t_gprecisebias::_apply_sat(const t_gtime& rec_epo, t_gtime& sat_epo, t_gallnav* nav)
    {
        double delay = 0.0;
        Matrix rot_matrix_sat, rot_matrix_rec;

        while (true)
        {
            sat_epo = rec_epo - delay;
            t_gtriple sat_crd, sat_vel;
            if (!_get_trs_sat_crdvel(sat_epo, _crt_sat, nav, sat_crd, sat_vel) ||
                double_eq(sat_crd[0] * sat_crd[1] * sat_crd[2], 0.0) || abs(sat_crd[0]) >= 1E18)
            {
                GREAT_DEBUG("can not get sat pos or vel for " + _crt_sat);
                return false;
            }

            Matrix earth_rotation = rotZ(OMEGA * delay);
            _trs_sat_crd = t_gtriple(earth_rotation * sat_crd.crd_cvect());
            _trs_sat_vel = t_gtriple(earth_rotation * sat_vel.crd_cvect());

            // PCO corr sat
            double pco_R = 0.0, pco_S = 0.0;
            if (_gallobj != 0)
            {
                _crt_obs.addcrd(_trs_sat_crd);
                _crt_obs.addvel(_trs_sat_vel);
                std::shared_ptr<t_gobj> sat_obj = _gallobj->obj(gsat2str(_crt_sat));
                std::shared_ptr<t_gobj> rec_obj = _gallobj->obj(_crt_rec);

                std::shared_ptr<t_gpcv> sat_pcv;
                std::shared_ptr<t_gpcv> rec_pcv;

                if (sat_obj != 0)
                {
                    sat_pcv = sat_obj->pcv(_crt_epo);
                }
                if (rec_obj != 0)
                {
                    rec_pcv = rec_obj->pcv(_crt_epo);
                }

                GOBS_LC lc = gnut::GOBS_LC::LC_L1;
                if (sat_pcv)
                {
                    // Satellite phase center offset
                    t_gtriple pco(0, 0, 0);

                    if (sat_pcv->pcoS(_crt_obs, pco, lc, _band_index[_crt_sys][gnut::FREQ_SEQ::FREQ_1], _band_index[_crt_sys][gnut::FREQ_SEQ::FREQ_2]) > 0)
                    {
                        rot_matrix_sat = _RotMatrix_Ant(_crt_obs, _crt_epo, sat_obj);
                        t_gtriple dx(rot_matrix_sat * (pco.crd_cvect()));
                        sat_pcv->pco_proj(pco_S, _crt_obs, _trs_rec_crd, dx);
                    }
                }

                if (rec_pcv)
                {
                    // Receiver phase center offset
                    t_gtriple pco(0.0, 0.0, 0.0);
                    if (rec_pcv->pcoR(_crt_obs, pco, lc, _band_index[_crt_sys][gnut::FREQ_SEQ::FREQ_1], _band_index[_crt_sys][gnut::FREQ_SEQ::FREQ_2]) > 0)
                    {
                        rot_matrix_rec = _RotMatrix_Ant(_crt_obs, _crt_epo, rec_obj);
                        t_gtriple dx(rot_matrix_rec * (pco.crd_cvect()));
                        rec_pcv->pco_proj(pco_R, _crt_obs, _trs_rec_crd, dx);
                    }
                    pco_R *= -1;
                }
            }

            double delay_temp;
            delay_temp = (_trs_sat_crd - _trs_rec_crd).norm();
            delay_temp += pco_R + pco_S;

            if (abs(delay_temp / CLIGHT - delay) < 1E-9)
            {
                break;
            }
            delay = delay_temp / CLIGHT;
        }
        _crt_obs.addcrd(_trs_sat_crd);
        _crt_obs.addvel(_trs_sat_vel);

        return true;
    }

    bool t_gprecisebias::_get_trs_sat_crdvel(const t_gtime& sat_epoch,
                                             const GSAT sat,
                                             t_gallnav* nav,
                                             t_gtriple& trs_sat_crd,
                                             t_gtriple& trs_sat_vel)
    {
        if (!nav)
        {
            GREAT_DEBUG("cannot get position or velocity " + sat);
            return false;
        }

        double xyz[3] = {0.0, 0.0, 0.0};
        double var[3] = {0.0, 0.0, 0.0};
        double vel[3] = {0.0, 0.0, 0.0};
        if (nav->pos(sat, sat_epoch, xyz, var, vel) < 0)
        {
            GREAT_DEBUG(sat_epoch.str_ymdhms("can not get sat crd or vel in epoch", false));
            return false;
        }

        trs_sat_crd = t_gtriple(xyz);
        trs_sat_vel = t_gtriple(vel);
        return true;
    }

    bool t_gprecisebias::_apply_rec_tides(const t_gtime& epoch, t_gtriple& rec)
    {
        if (!_tide)
        {
            GREAT_ERROR("The tide ptr is nullptr.");
            return false;
        }

        t_gtriple tide(0.0, 0.0, 0.0);
        try
        {
            if (!_gdata_erp || _gdata_erp->isEmpty())
            {
                GREAT_ERROR("have no poleut1 data, cannot compute tides");
                return false;
            }
            t_gtime epoch_tt = epoch;
            epoch_tt.tsys(t_gtime::t_tsys::TT);
            std::map<std::string, double> eop;
            _gdata_erp->getEopData(epoch_tt, eop);
            const double xpole = eop.at("XPOLE");
            const double ypole = eop.at("YPOLE");
            const double ut1_tai = eop.at("UT1-TAI");

            // solid tide
            t_gtideIERS* solid_ptr = dynamic_cast<t_gtideIERS*>(_tide.get());
            if (!solid_ptr)
            {
                GREAT_ERROR("can not get the solid tide ptr.");
                return false;
            }
            t_gtriple solid_earth = solid_ptr->tide_solid(epoch, rec, xpole, ypole, ut1_tai);

            // ocean load
            t_gtriple load_ocean = _tide->load_ocean(epoch, _crt_rec, rec);

            // pole tide
            t_gtideIERS* pole_ptr = dynamic_cast<t_gtideIERS*>(_tide.get());
            if (!pole_ptr)
            {
                GREAT_ERROR("can not get the pole tide ptr.");
                return false;
            }
            pole_ptr->set_mean_pole_model(_mean_pole_model);
            t_gtriple tide_pole = pole_ptr->tide_pole_pod(epoch, xpole, ypole, rec);

            t_gtriple load_atmosph = t_gtriple();

            t_gtriple load_ocean_pole(0.0, 0.0, 0.0);
            if (_opl)
            {
                t_gtideIERS* opl_ptr =
                    dynamic_cast<t_gtideIERS*>(_tide.get()); // please note that in _tide. no pointer for atmloading is stored.
                opl_ptr->set_opl_grid(_opl);
                opl_ptr->set_mean_pole_model(_mean_pole_model);
                load_ocean_pole = opl_ptr->load_oceanpole(epoch, rec, xpole, ypole);
            }

            tide = solid_earth + load_ocean + tide_pole + load_atmosph + load_ocean_pole;
        }
        catch (...)
        {
            GREAT_ERROR("can not get the tide ptr.");
            return false;
        }

        // unit to m
        rec = rec + tide * 1.e3;
        return true;
    }

} // namespace great
