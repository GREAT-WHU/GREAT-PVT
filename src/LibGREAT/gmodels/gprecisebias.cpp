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

#ifndef OMGE_DOT
#define OMGE_DOT 7.2921151467e-5 ///< WGS 84 value of the earth's rotation rate [rad/sec]
#endif // !1

namespace great
{
    t_gprecisebias::t_gprecisebias(t_gallproc *data, t_gsetbase *setting) : t_gbiasmodel(setting)
    {
        _gall_nav = dynamic_cast<t_gallnav *>((*data)[t_gdata::GRP_EPHEM]);
        _gallobj = dynamic_cast<t_gallobj *>((*data)[t_gdata::ALLOBJ]);
        _gdata_erp = dynamic_cast<t_gpoleut1 *>((*data)[t_gdata::ALLPOLEUT1]);
        _gdata_navde = dynamic_cast<t_gnavde *>((*data)[t_gdata::ALLDE]);
        _tide = shared_ptr<t_gtide>(new t_gtideIERS(dynamic_cast<t_gallotl *>((*data)[t_gdata::ALLOTL])));
        _minElev = dynamic_cast<t_gsetproc *>(setting)->minimum_elev();
        _crd_est = dynamic_cast<t_gsetproc *>(setting)->crd_est();
        _is_flt = (dynamic_cast<t_gsetgen *>(setting)->estimator() == "FLT");
        _trop_est = dynamic_cast<t_gsetproc *>(setting)->tropo();
        if (dynamic_cast<t_gallprec *>(_gall_nav))
        {
            _corrt_sat_pcv = !(dynamic_cast<t_gsetinp *>(_gset)->input_size("sp3") == 0);
            if (dynamic_cast<t_gsetinp *>(_gset)->input_size("orbit") != 0)
                _corrt_sat_pcv = true;
        }
        _gifcb = dynamic_cast<t_gifcb *>((*data)[t_gdata::IFCB]);
        _attitudes = dynamic_cast<t_gsetproc *>(setting)->attitudes();
        _opl = dynamic_cast<t_gallopl *>((*data)[t_gdata::ALLOPL]);
        _mean_pole_model = dynamic_cast<t_gsetproc *>(setting)->mean_pole_model();
    }

    t_gprecisebias::t_gprecisebias(t_gallproc *data, t_spdlog spdlog, t_gsetbase *setting) : t_gbiasmodel(spdlog, setting)
    {
        _gall_nav = dynamic_cast<t_gallnav *>((*data)[t_gdata::GRP_EPHEM]);
        _gallobj = dynamic_cast<t_gallobj *>((*data)[t_gdata::ALLOBJ]);
        _gdata_erp = dynamic_cast<t_gpoleut1 *>((*data)[t_gdata::ALLPOLEUT1]);
        _gdata_navde = dynamic_cast<t_gnavde *>((*data)[t_gdata::ALLDE]);
        _tide = shared_ptr<t_gtide>(new t_gtideIERS(spdlog, dynamic_cast<t_gallotl *>((*data)[t_gdata::ALLOTL])));
        _minElev = dynamic_cast<t_gsetproc *>(setting)->minimum_elev();
        _crd_est = dynamic_cast<t_gsetproc *>(setting)->crd_est();
        _is_flt = (dynamic_cast<t_gsetgen *>(setting)->estimator() == "FLT");
        _trop_est = dynamic_cast<t_gsetproc *>(setting)->tropo();
        if (dynamic_cast<t_gallprec *>(_gall_nav))
        {
            _corrt_sat_pcv = !(dynamic_cast<t_gsetinp *>(_gset)->input_size("sp3") == 0);
            if (dynamic_cast<t_gsetinp *>(_gset)->input_size("orbit") != 0)
                _corrt_sat_pcv = true;
        }
        _gifcb = dynamic_cast<t_gifcb *>((*data)[t_gdata::IFCB]);
        _attitudes = dynamic_cast<t_gsetproc *>(setting)->attitudes();
        _opl = dynamic_cast<t_gallopl *>((*data)[t_gdata::ALLOPL]);
        _mean_pole_model = dynamic_cast<t_gsetproc *>(setting)->mean_pole_model();
    }

    t_gprecisebias::~t_gprecisebias()
    {
    }

    bool t_gprecisebias::cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gobs &gobs, t_gbaseEquation &result)
    {
        return false;
    }

    void t_gprecisebias::update_obj_clk(const string &obj, const t_gtime &epo, double clk)
    {

        _obj_clk[obj].first = epo;
        _obj_clk[obj].second = clk;
        _rec_clk[obj] = clk;
    }

    double t_gprecisebias::get_rec_clk(const string &obj)
    {
        return _rec_clk[obj];
    }

    double t_gprecisebias::tropoDelay(t_gtime &epoch, string &rec, t_gallpar &param, t_gtriple site_ell, t_gsatdata &satdata)
    {

        if (site_ell[2] > 1E4)
        {
            return 0.0;
        }
        else
        {
            if (_tropoModel == 0)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Tropo Model setting is not correct. Default used! Check config.");
                _tropoModel = make_shared<t_saast>();
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
                mf.gmf(epoch.mjd(), ell[0], ell[1], ell[2], G_PI / 2.0 - ele,
                       mfh, mfw, dmfh, dmfw);
            }
            else if (_mf_ztd == ZTDMPFUNC::COSZ)
            {
                mfh = mfw = 1 / sin(ele);
                dmfh = dmfw = -(cos(ele)) / (sin(ele) * sin(ele));
            }
            else
                return 0.0;

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

    double t_gprecisebias::ionoDelay(t_gtime &epoch, t_gallpar &param, t_gsatdata &satdata, IONMODEL &ion_model, GOBSBAND &band_1, t_gobs &gobs)
    {

        if (band_1 == BAND || band_1 == BAND_A || band_1 == BAND_B || band_1 == BAND_C || band_1 == BAND_D)
        {
            return 0.0;
        }

        if (gobs.band() == BAND || gobs.band() == BAND_A || gobs.band() == BAND_B || gobs.band() == BAND_C || gobs.band() == BAND_D)
        {
            return 0.0;
        }

        if (ion_model == IONMODEL::VION)
        {
            throw logic_error(" not support VION model, use SION in XML");
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

    double t_gprecisebias::isbDelay(t_gallpar &param, t_gsatdata &satdata, string &sat, string &rec, t_gobs &gobs)
    {
        double isb_offset = 0.0;

        auto gsys = t_gsys::sat2gsys(sat);

        switch (gsys)
        {
        case GPS:
        {
            break;
        }
        // GLONASS system time offset
        case GLO:
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
        case GAL:
        {
            // Galileo system time offset
            int i = param.getParam(rec, par_type::GAL_ISB, "");
            if (i >= 0)
            {
                isb_offset = param[i].value();
            }
            break;
        }
        case BDS:
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
        case QZS:
        {
            int i = param.getParam(_crt_rec, par_type::QZS_ISB, "");
            if (i >= 0)
            {
                isb_offset = param[i].value();
            }
            break;
        }
        default:
            throw logic_error("can not support such sys ： " + t_gsys::gsys2str(gsys));
        }

        return isb_offset;
    }
    double t_gprecisebias::relDelay(t_gtriple &crd_crt_rec, t_gtriple &vel_crt_rec, t_gtriple &crd_sat, t_gtriple &vel_sat)
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

    bool t_gprecisebias::_update_obs_info(t_gsatdata &obsdata)
    {
        obsdata = _crt_obs;
        return true;
    }

    double t_gprecisebias::ifbDelay(t_gallpar& param, t_gsatdata& satdata, string& sat, string& rec, t_gobs& gobs)
    {
        double ifb = 0.0;
        auto gsys = t_gsys::sat2gsys(sat);

        switch (gsys)
        {
        case GPS:
        {
            int i = param.getParam(_crt_rec, par_type::IFB_GPS, "");
            if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == FREQ_3)
            {
                ifb = param[i].value();
            }
            break;
        }
        case GAL:
        {
            int i = param.getParam(_crt_rec, par_type::IFB_GAL, "");
            if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == FREQ_3)
            {
                ifb = param[i].value();
            }
            i = param.getParam(_crt_rec, par_type::IFB_GAL_2, "");
            if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == FREQ_4)
            {
                ifb = param[i].value();
            }
            i = param.getParam(_crt_rec, par_type::IFB_GAL_3, "");
            if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == FREQ_5)
            {
                ifb = param[i].value();
            }
            break;
        }
        case BDS:
        {
            int i = param.getParam(_crt_rec, par_type::IFB_BDS, "");
            if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == FREQ_3)
            {
                ifb = param[i].value();
            }
            i = param.getParam(_crt_rec, par_type::IFB_BDS_2, "");
            if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == FREQ_4)
            {
                ifb = param[i].value();
            }
            i = param.getParam(_crt_rec, par_type::IFB_BDS_3, "");
            if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == FREQ_5)
            {
                ifb = param[i].value();
            }
            break;
        }
        // QZSS system time offset
        case QZS:
        {
            int i = param.getParam(_crt_rec, par_type::IFB_QZS, "");
            if (i >= 0 && gobs.is_code() && _freq_index[satdata.gsys()][gobs.band()] == FREQ_3)
            {
                ifb = param[i].value();
            }
            break;
        }
        case GLO:
        {
            break;
        }
        default:
            throw logic_error("can not support such sys ： " + t_gsys::gsys2str(gsys));
        }      
        return ifb;
    }

    double t_gprecisebias::ifcbDelay(t_gsatdata &satdata, t_gifcb *ifcb, OBSCOMBIN obscombin)
    {
        double L = 0.0;
        double ifcb_GPS = 0.0;

        //lvhb added in 202100404 for rtk/nrtk/urtk
        if (ifcb == nullptr && _gifcb == nullptr)
        {
            return false;
        }

        if (ifcb == nullptr)
        {
            ifcb = _gifcb;
        }

        t_gtime epoch = satdata.epoch();
        string sat = satdata.sat();

        GOBSBAND b1 = t_gsys::band_priority(satdata.gsys(), FREQ_1);
        GOBSBAND b3 = t_gsys::band_priority(satdata.gsys(), FREQ_3);

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
                return false;

            if (epoch_ifcb[sat]->sigma > 0.2 ||
                epoch_ifcb[sat]->npoint <= 2)
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

    double t_gprecisebias::windUp(const GOBSBAND &freq_2, t_gsatdata &satdata, const Eigen::Vector3d &rRec)
    {
        double wavelength = 0.0;
        wavelength = satdata.wavelength(freq_2);

        if (double_eq(wavelength, 0.0))
        {
            throw runtime_error("wavelength is zero when getting windup");
        }

        if (fabs(satdata.wind()) > 0)
        {
            return satdata.wind() * wavelength;
        }
        else
        {
            t_gtime epoch = satdata.epoch();
            string prn = satdata.sat();
            Eigen::Vector3d rSat = _trs_sat_crd.crd_cvect_Eigen();

            // First time - initialize to zero
            // -------------------------------
            if (_phase_windup.find(_crt_rec) == _phase_windup.end() ||
                _phase_windup[_crt_rec].find(prn) == _phase_windup[_crt_rec].end() ||
                _phase_windup[_crt_rec][prn].size() == 0)
            {
                _phase_windup[_crt_rec][prn][epoch] = 0.0;
            }

            // Compute the correction for new time
            // -----------------------------------
            if (_phase_windup[_crt_rec][prn].find(epoch) == _phase_windup[_crt_rec][prn].end() ||
                _phase_windup[_crt_rec][prn].size() == 1)
            {

                // the last epoch
                double dphi0 = _phase_windup[_crt_rec][prn].rbegin()->second;
                Eigen::Vector3d rho = rRec - rSat; 
                rho /= rho.norm();

                Eigen::Vector3d i, j, k;

                // attitude model
                string antype = "";
                if (_gallobj != 0)
                {
                    shared_ptr<t_gobj> sat_obj = _gallobj->obj(satdata.sat());
                    shared_ptr<t_gpcv> sat_pcv;
                    if (sat_obj != 0)
                        sat_pcv = sat_obj->pcv(satdata.epoch());
                    if (sat_pcv != 0)
                        antype = sat_pcv->anten();
                }

                if (_attitudes == ATTITUDES::YAW_NOMI)
                    _gattitude_model.attitude(satdata, "", i, j, k); //nominal modeling
                else if (_attitudes == ATTITUDES::YAW_RTCM)
                    _gattitude_model.attitude(satdata, satdata.yaw(), i, j, k); //value from RTCM used
                else
                    _gattitude_model.attitude(satdata, antype, i, j, k); //default

                if (antype.find("BLOCK IIR") != string::npos)
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
                    alpha = 1.0;
                if (alpha < -1.0)
                    alpha = -1.0;

                double dphi = acos(alpha) / 2.0 / G_PI; // in cycles

                if (rho.dot(dipSat.cross(dipRec)) < 0.0)
                    dphi = -dphi;

                _phase_windup[_crt_rec][prn][epoch] = floor(dphi0 - dphi + 0.5) + dphi;
            }

            satdata.addwind(_phase_windup[_crt_rec][prn][epoch]);
            return _phase_windup[_crt_rec][prn][epoch] * wavelength;
        }
    }

    double t_gprecisebias::PCV(bool corrt_sat, bool corrt_rec, t_gtime &epoch, t_gtime &crt_sat_epo, t_gtriple &trs_rec_crd, t_gsatdata &satdata, t_gobs &gobs)
    {
        // Phase center variation correction
        double pcv_R = 0.0;
        double pcv_S = 0.0;
        double pco_R = 0.0;
        double pco_S = 0.0;

        GOBSBAND band = gobs.band();

        if (_gallobj == nullptr)
        {
            throw runtime_error("cannot correct PCV\n");
        }

        shared_ptr<t_gobj> sat_obj = _gallobj->obj(satdata.sat());
        shared_ptr<t_gobj> rec_obj = _gallobj->obj(satdata.site());

        shared_ptr<t_gpcv> sat_pcv = (sat_obj != nullptr) ? sat_obj->pcv(epoch) : nullptr;
        shared_ptr<t_gpcv> rec_pcv = (rec_obj != nullptr) ? rec_obj->pcv(epoch) : nullptr;

        if (!_isCalSatPCO)
        { 
            if (sat_pcv != nullptr)
                sat_pcv = nullptr;
        }

        if (sat_pcv != nullptr && corrt_sat) 
        {
            // Satellite phase center variation
            // -- Satellite phase center offset
            t_gtriple pco(0, 0, 0);
            if (sat_pcv->pcoS_raw(satdata, pco, band) > 0)
            {
                string antenna = sat_pcv->anten();
                t_gtriple dx(0, 0, 0);
                Eigen::Vector3d i, j, k;
                if (_attitudes == ATTITUDES::YAW_NOMI)
                    _gattitude_model.attitude(satdata, "", i, j, k); //nominal modeling
                else if (_attitudes == ATTITUDES::YAW_RTCM)
                    _gattitude_model.attitude(satdata, satdata.yaw(), i, j, k); //value from RTCM used
                else
                    _gattitude_model.attitude(satdata, antenna, i, j, k); //default
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
                Matrix _rot_matrix = _RotMatrix_Ant(satdata, _crt_rec_epo, crt_sat_epo, rec_obj, false);
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

    Matrix t_gprecisebias::_RotMatrix_Ant(t_gsatdata &obsdata, const t_gtime &receive_epoch, const t_gtime &transmit_epoch, shared_ptr<t_gobj> obj, bool isCRS)
    {
        t_gdata::ID_TYPE type = obj->id_type();
        Matrix rotmatrix(3, 3);
        t_gtriple ell(0.0, 0.0, 0.0);
        double sinPhi, cosPhi, sinLam, cosLam;

        t_gtime t;

        if (type == t_gdata::TRN)
        {
            shared_ptr<t_gpcv> sat_pcv = obj->pcv(receive_epoch);
            string antenna = (sat_pcv)->anten();
            t_gtriple dx(0, 0, 0);
            Eigen::Vector3d i, j, k;
            if (_attitudes == ATTITUDES::YAW_NOMI)
                _gattitude_model.attitude(obsdata, "", i, j, k); 
            else if (_attitudes == ATTITUDES::YAW_RTCM)
                _gattitude_model.attitude(obsdata, obsdata.yaw(), i, j, k); 
            else
                _gattitude_model.attitude(obsdata, antenna, i, j, k); 
            rotmatrix.Column(1) << i.data();
            rotmatrix.Column(2) << j.data();
            rotmatrix.Column(3) << k.data();
            t = transmit_epoch;
        }
        else if (type == t_gdata::REC)
        {
            xyz2ell(_trs_rec_crd, ell, false);
            sinPhi = sin(ell[0]);
            cosPhi = cos(ell[0]);
            sinLam = sin(ell[1]);
            cosLam = cos(ell[1]);
            rotmatrix << -sinPhi * cosLam << -sinLam << +cosPhi * cosLam
                      << -sinPhi * sinLam << +cosLam << +cosPhi * sinLam
                      << +cosPhi << 0.0 << +sinPhi;
            t = receive_epoch;
        }
        else
        {
            throw exception();
        }

        if (isCRS)
        {
            _update_rot_matrix(t); 
            rotmatrix = _trs2crs_2000->getRotMat() * rotmatrix;
        }

        return rotmatrix;
    }

    void t_gprecisebias::_update_rot_matrix(const t_gtime &epoch)
    {
        if (_gdata_erp->isEmpty())
            return;
        if (!_gdata_erp)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "have no poleut1 data,cant' compute trs2crs!!");
            throw exception();
        }

        t_gtime tdt = epoch;
        tdt.tsys(t_gtime::TT);
        t_gtime utc = epoch;
        utc.tsys(t_gtime::UTC);
        auto find_iter = _trs2crs_list.find(tdt);
        if (find_iter == _trs2crs_list.end())
        {
            _trs2crs_2000 = make_shared<t_gtrs2crs>(false, _gdata_erp);
            _trs2crs_2000->calcRotMat(tdt, true, true, true, false, false);
            _trs2crs_list.insert(make_pair(tdt, _trs2crs_2000));

            auto before_iter = _trs2crs_list.lower_bound(tdt - 300.0);
            if (before_iter != _trs2crs_list.begin())
            {
                _trs2crs_list.erase(_trs2crs_list.begin(), --before_iter);
            }
        }
        else
        {
            _trs2crs_2000 = find_iter->second;
        }
        return;
    }


    bool t_gprecisebias::_apply_rec(const t_gtime &crt_epo, const t_gtime &rec_epo, t_gallpar &pars)
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
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "can't get the crd of" + _crt_rec + " in time" + rec_epo.str_ymdhms());
                return false;
            }
        }

        if (trs_rec_xyz.zero())
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "can't get the crd of" + _crt_rec + " in time" + rec_epo.str_ymdhms());
            return false;
        }

        bool tide_valid = _apply_rec_tides(rec_epo, trs_rec_xyz);
        if (!tide_valid)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "apply tide failed for " + _crt_rec + " in time" + rec_epo.str_ymdhms());
            return false;
        }

        // ARP Correction
        if (!_is_flt || _crd_est == CONSTRPAR::FIX)
            trs_rec_xyz = trs_rec_xyz + _crt_obj->eccxyz(rec_epo);
        _trs_rec_crd = trs_rec_xyz;

        // TRS2CRS
        _update_rot_matrix(rec_epo);
        Matrix trs2crs = _trs2crs_2000->getRotMat();
        ColumnVector crs_rec_xyz = trs2crs * trs_rec_xyz.crd_cvect();
        _crs_rec_crd = t_gtriple(crs_rec_xyz);

        // get vel
        Matrix dtrs2crs = _trs2crs_2000->getMatDu() * OMGE_DOT;
        ColumnVector trs_rec_vel = dtrs2crs * trs_rec_xyz.crd_cvect();
        _crs_rec_vel = t_gtriple(trs_rec_vel);
        return true;
    }

    bool t_gprecisebias::_apply_sat(const t_gtime &rec_epo, t_gtime &sat_epo, t_gallnav *nav)
    {
        // ITERATION
        // compute sat coord(CRS)  clk(estimated) (rewrite in orb model)
        double delay = 0.0;
        Matrix rot_matrix_sat, rot_matrix_rec;

        int pv_iod = 0;
        int clk_iod = 0;

        while (true)
        {
            sat_epo = rec_epo - delay;
            // Get CRS
            bool sat_pos_valid = _get_crs_sat_crd(sat_epo, _crt_sat, nav, _crs_sat_crd);
            if (!sat_pos_valid)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "can not get sat pos for " + _crt_sat);
                return false;
            }
            if (double_eq(_crs_sat_crd[0] * _crs_sat_crd[1] * _crs_sat_crd[2], 0.0) || abs(_crs_sat_crd[0]) >= 1E18)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "can not get sat pos for " + _crt_sat);
                return false;
            }
            // SET TRS in epoch TR [include earth rotation]
            _update_rot_matrix(rec_epo);
            _trs_sat_crd = t_gtriple(_trs2crs_2000->getRotMat().t() * _crs_sat_crd.crd_cvect());

            bool sat_vel_valid = _get_crs_sat_vel(sat_epo, _crt_sat, nav, _crs_sat_vel);
            if (!sat_vel_valid)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "can not get sat vel for " + _crt_sat);
                return false;
            }
            // PCO corr sat
            double pco_R = 0.0, pco_S = 0.0;
            if (_gallobj != 0)
            {
                _update_rot_matrix(rec_epo);
                _crt_obs.addcrd(_trs_sat_crd);
                ColumnVector x_earth = (_crs_sat_crd.crd_cvect().t() * _trs2crs_2000->getMatDu() / RAD2TSEC).t();
                t_gtriple vel = t_gtriple(_trs2crs_2000->getRotMat().t() * _crs_sat_vel.crd_cvect() - x_earth);
                _crt_obs.addvel(vel);
                shared_ptr<t_gobj> sat_obj = _gallobj->obj(_crt_sat);
                shared_ptr<t_gobj> rec_obj = _gallobj->obj(_crt_rec);

                shared_ptr<t_gpcv> sat_pcv;
                shared_ptr<t_gpcv> rec_pcv;

                if (sat_obj != 0)
                    sat_pcv = sat_obj->pcv(_crt_epo);
                if (rec_obj != 0)
                    rec_pcv = rec_obj->pcv(_crt_epo);

                GOBS_LC lc = LC_L1;
                if (sat_pcv) 
                {
                    // Satellite phase center offset
                    t_gtriple pco(0, 0, 0);

                    if (sat_pcv->pcoS(_crt_obs, pco, lc, _band_index[_crt_sys][FREQ_1], _band_index[_crt_sys][FREQ_2]) > 0)
                    {
                        rot_matrix_sat = _RotMatrix_Ant(_crt_obs, _crt_epo, _crt_sat_epo, sat_obj, false);
                        t_gtriple dx(rot_matrix_sat * (pco.crd_cvect()));
                        sat_pcv->pco_proj(pco_S, _crt_obs, _trs_rec_crd, dx);
                    }
                }

                if (rec_pcv)
                {
                    // Receiver phase center offset
                    t_gtriple pco(0.0, 0.0, 0.0);
                    if (rec_pcv->pcoR(_crt_obs, pco, lc, _band_index[_crt_sys][FREQ_1], _band_index[_crt_sys][FREQ_2]) > 0)
                    {
                        rot_matrix_rec = _RotMatrix_Ant(_crt_obs, _crt_epo, _crt_sat_epo, rec_obj, false);
                        rot_matrix_rec *(pco.crd_cvect());
                        t_gtriple dx(rot_matrix_rec * (pco.crd_cvect()));
                        rec_pcv->pco_proj(pco_R, _crt_obs, _trs_rec_crd, dx);
                    }
                    pco_R *= -1;
                }
            }

            double delay_temp;
            delay_temp = (_crs_sat_crd - _crs_rec_crd).norm();
            delay_temp += pco_R + pco_S;

            if (abs(delay_temp / CLIGHT - delay) < 1E-9)
                break;
            delay = delay_temp / CLIGHT;
        }
        _crt_obs.addSCF2CRS(_trs2crs_2000->getRotMat() * rot_matrix_sat, rot_matrix_sat);
        _crt_obs.addcrd(_trs_sat_crd);
        ColumnVector x_earth = (_crs_sat_crd.crd_cvect().t() * _trs2crs_2000->getMatDu() / RAD2TSEC).t();
        t_gtriple vel = t_gtriple(_trs2crs_2000->getRotMat().t() * _crs_sat_vel.crd_cvect() - x_earth);
        _crt_obs.addvel(vel);
        _crt_obs.addcrdcrs(_crs_sat_crd);
        _crt_obs.addvel_crs(_crs_sat_vel);

        return true;
    }

    bool t_gprecisebias::_get_crs_sat_vel(const t_gtime &sat_epoch, const string &sat, t_gallnav *nav, t_gtriple &crs_sat_vel)
    {
        if (!nav)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "cannot get vel " + sat);
            return false;
        }

        // get vel in CRS
        t_gtriple satcrd_next(0.0, 0.0, 0.0);
        t_gtriple satcrd_before(0.0, 0.0, 0.0);
        
        bool crd_valid = _get_crs_sat_crd(sat_epoch + 0.0001, sat, nav, satcrd_next);
        if (!crd_valid)
            return false;
        crs_sat_vel = (satcrd_next - _crs_sat_crd) * 10000;
        return true;
    }

    bool t_gprecisebias::_get_crs_sat_crd(const t_gtime &sat_epoch, const string &sat, t_gallnav *nav, t_gtriple &crs_sat_crd)
    {
        bool pos_valid = false;
        if (nav)
        {
            double xyz_sat[3];
            pos_valid = (nav->pos(sat, sat_epoch, xyz_sat) >= 0) ? true : false;

            // TRS2CRS
            _update_rot_matrix(sat_epoch);
            crs_sat_crd = t_gtriple(xyz_sat);
            crs_sat_crd = t_gtriple(_trs2crs_2000->getRotMat() * crs_sat_crd.crd_cvect());
        }

        if (pos_valid)
            return pos_valid;
        else if (_spdlog)
            SPDLOG_LOGGER_DEBUG(_spdlog, sat_epoch.str_ymdhms("can not get sat crd in epoch", false));
        return false;
    }

    bool t_gprecisebias::_apply_rec_tides(const t_gtime &epoch, t_gtriple &rec)
    {
        if (!_tide)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "The tide ptr is nullptr.");
            return false;
        }

        _update_rot_matrix(epoch);
        double xpole = _trs2crs_2000->getXpole();
        double ypole = _trs2crs_2000->getYpole();
        // double gast = _trs2crs_2000->getGmst();
        Matrix rot_trs2crs = _trs2crs_2000->getRotMat();

        t_gtriple tide(0.0, 0.0, 0.0);
        try
        {
            // solid tide
            t_gtideIERS *solid_ptr = dynamic_cast<t_gtideIERS *>(_tide.get());
            if (!solid_ptr)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_ERROR(_spdlog, "can not get the solid tide ptr.");
                return false;
            }
            t_gtriple solid_earth = solid_ptr->tide_solid(epoch, rec, rot_trs2crs, _gdata_navde);

            // ocean load
            t_gtriple load_ocean = _tide->load_ocean(epoch, _crt_rec, rec);

            //pole tide
            t_gtideIERS *pole_ptr = dynamic_cast<t_gtideIERS *>(_tide.get());
            if (!pole_ptr)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_ERROR(_spdlog, "can not get the pole tide ptr.");
                return false;
            }
            pole_ptr->set_mean_pole_model(_mean_pole_model);
            t_gtriple tide_pole = pole_ptr->tide_pole_pod(epoch, xpole, ypole, rec);

            t_gtriple load_atmosph = t_gtriple();

            t_gtriple load_ocean_pole(0.0, 0.0, 0.0);
            if (_opl)
            {
                t_gtideIERS *opl_ptr = dynamic_cast<t_gtideIERS *>(_tide.get()); // please note that in _tide. no pointer for atmloading is stored.
                opl_ptr->set_opl_grid(_opl);
                opl_ptr->set_mean_pole_model(_mean_pole_model);
                load_ocean_pole = opl_ptr->load_oceanpole(epoch, rec, xpole, ypole);
            }

            tide = solid_earth +
                   load_ocean +
                   tide_pole +
                   load_atmosph +
                   load_ocean_pole;

        }
        catch (...)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "can not get the tide ptr.");
            return false;
        }

        //unit to m
        rec = rec + tide * 1.e3;
        return true;
    }

}
