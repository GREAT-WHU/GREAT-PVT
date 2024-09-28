/**
 * @file         gprecisebiasGPP.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        the class for ppp procrssing
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gmodels/gprecisebiasGPP.h"
#include "gmodels/gprecisebias.h"
#include "spdlog/spdlog.h"
#include "gset/gsetgen.h"
#include "gall/gallprec.h"

namespace great
{
    t_gprecisebiasGPP::t_gprecisebiasGPP(t_gallproc *data, t_gsetbase *setting) : t_gprecisebias(data, setting)
    {
        int base_size = dynamic_cast<t_gsetgen *>(setting)->list_base().size();
        _ddmode = (base_size ? true : false);
    }

    t_gprecisebiasGPP::t_gprecisebiasGPP(t_gallproc *data, t_spdlog spdlog, t_gsetbase *setting) : t_gprecisebias(data, spdlog, setting)
    {
        int base_size = dynamic_cast<t_gsetgen *>(setting)->list_base().size();
        _ddmode = (base_size ? true : false);
    }

    t_gprecisebiasGPP::~t_gprecisebiasGPP()
    {
    }

    bool t_gprecisebiasGPP::cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gobs &gobs, t_gbaseEquation &result)
    {
        // check obs_type valid
        double Obs_value = obsdata.getobs(gobs.gobs());
        if (double_eq(Obs_value, 0.0))
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "Obs_value is 0.0");
            return false;
        }

        tuple<string, string, t_gtime>* flag = &_rec_sat_before;
        if (make_tuple(obsdata.site(), obsdata.sat(), epoch) != *flag)
        {
            bool update_valid = t_gprecisebiasGPP::_update_obs_info_GPP(epoch, _gall_nav, _gallobj, obsdata, params);
            if (!update_valid)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_ERROR(_spdlog, "update obs information failed" + epoch.str_ymdhms("", false));
                return false;
            }

            // prepare Caculate some common bias[sat_pos,rec_pos,relative,rho]
            bool pre_valid = t_gprecisebiasGPP::_prepare_obs_GPP(epoch, _gall_nav, _gallobj, params);
            if (!pre_valid)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_ERROR(_spdlog, "prepare obs information failed" + epoch.str_ymdhms("", false));
                return false;
            }
            *flag = make_tuple(obsdata.site(), obsdata.sat(), epoch);
        }

        // combine equ
        double omc = 0.0, wgt = 0.0;
        vector<pair<int, double>> coef;

        if (!_omc_obs_ALL(epoch, _crt_obs, params, gobs, omc))
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "omc obs failed");
            return false;
        };

        if (!_wgt_obs_ALL(t_gdata::REC, gobs, _crt_obs, 1.0, wgt))
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "weight obs failed");
            return false;
        };

        if (!_prt_obs_ALL(epoch, _crt_obs, params, gobs, coef))
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "partialrange obs failed");
            return false;
        }

        result.B.push_back(coef);
        result.P.push_back(wgt);
        result.l.push_back(omc);

        _update_obs_info(obsdata);
        return true;
    }

    double t_gprecisebiasGPP::cmpObs(t_gtime &epoch, string &sat, string &rec, t_gallpar &param, t_gsatdata &gsatdata, t_gobs &gobs)
    {
        if (_gallobj == nullptr)
        {
            throw logic_error("gallobj is nullptr in precisemodel cmpObs");
        }

        // Cartesian coordinates to ellipsodial coordinates
        t_gtriple xyz, ell;
        if (_is_flt)
        {
            if (param.getCrdParam(rec, xyz) < 0)
            {
                xyz = _crt_obj->crd_arp(epoch);
            }
        }
        else
        {
            if (param.getCrdParam(rec, xyz) < 0)
            {
                xyz = _crt_obj->crd(epoch);
            }
            xyz += _crt_obj->eccxyz(epoch);
        }

        xyz2ell(xyz, ell, false);

        t_gtriple satcrd = gsatdata.satcrd();
        ColumnVector cSat = satcrd.crd_cvect();

        // Tropospheric wet delay correction
        double trpDelay = 0;
        trpDelay = tropoDelay(epoch, rec, param, ell, gsatdata);
        if (fabs(trpDelay) > 50)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "trpDelay > 50");
            return -1;
        }

        // idx for par correction
        int i = -1;

        // Receiver clock correction
        double clkRec = 0.0;
        i = param.getParam(rec, par_type::CLK, "");
        if (i >= 0)
        {
            clkRec = param[i].value();
        }
        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, rec + " ! warning:  Receiver Clock is not included in parameters!");
        }

        // system time offset
        double isb_offset = isbDelay(param, gsatdata, sat, rec, gobs);

        double ifb = ifbDelay(param, gsatdata, sat, rec, gobs);

        // Wind up correction
        double wind = 0.0;
        if (gobs.is_phase())
        {
            wind = windUp(gobs.band(), gsatdata, _trs_rec_crd.crd_cvect_Eigen());
        }

        // ion correction
        double ion = 0.0;
        auto band_1 = _band_index[gsatdata.gsys()][FREQ_1];
        ion = ionoDelay(epoch, param, gsatdata, _ion_model, band_1, gobs);

        // pcv correction
        double pcv = PCV(_corrt_sat_pcv, _corrt_rec_pcv, epoch, _crt_sat_epo, _trs_rec_crd, gsatdata, gobs);
            
        return gsatdata.rho() +
               clkRec -
               gsatdata.clk() +
               trpDelay +
               isb_offset +
               ifb +
               ion +
               wind +
               pcv;
    }

    bool t_gprecisebiasGPP::_prepare_obs_GPP(const t_gtime &crt_epo, t_gallnav *gallnav, t_gallobj *gallobj, t_gallpar &pars)
    {
        if (!gallnav || !gallobj)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "no navgation data or atx data for epoch " + crt_epo.str_ymdhms());
            return false;
        }

        // compute reciver time
        _crt_rec_epo = crt_epo - _crt_rec_clk;
        _crt_obs.addrecTime(_crt_rec_epo);

        // get Rec crd
        bool apply_obj_valid = t_gprecisebias::_apply_rec(crt_epo, _crt_rec_epo, pars);
        if (!apply_obj_valid)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "can not apply site in " + crt_epo.str_ymdhms());
            return false;
        }

        // get sat crd
        bool apply_sat_valid = _apply_sat(_crt_rec_epo, _crt_sat_epo, gallnav);
        if (!apply_sat_valid)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "can not apply sat in " + crt_epo.str_ymdhms());
            return false;
        }
        _crt_obs.addsatTime(_crt_sat_epo);

        // get f01 PCO
        this->_crs_rec_pco = this->_crs_rec_crd;
        this->_crs_sat_pco = this->_crs_sat_crd;

        shared_ptr<t_gobj> sat_obj = this->_gallobj->obj(_crt_sat);
        shared_ptr<t_gobj> rec_obj = this->_gallobj->obj(_crt_rec);

        shared_ptr<t_gpcv> sat_pcv = (sat_obj != 0) ? sat_obj->pcv(crt_epo) : nullptr;
        shared_ptr<t_gpcv> rec_pcv = (rec_obj != 0) ? rec_obj->pcv(crt_epo) : nullptr;

        if (!_isCalSatPCO)
        {
            sat_pcv = nullptr;
        }

        if (sat_pcv) 
        {
            // Satellite phase center offset
            t_gtriple pco(0, 0, 0);
            if (sat_pcv->pcoS(_crt_obs, pco, _observ, _band_index[_crt_sys][FREQ_1], _band_index[_crt_sys][FREQ_2]) > 0)
            {
                Matrix _rot_matrix = _RotMatrix_Ant(_crt_obs, _crt_epo, _crt_sat_epo, sat_obj, true);
                this->_crs_sat_pco += t_gtriple(_rot_matrix * pco.crd_cvect());
            }
        }

        if (rec_pcv)
        {
            // Receiver phase center offset
            t_gtriple pco(0.0, 0.0, 0.0);
            if (rec_pcv->pcoR(_crt_obs, pco, _observ, _band_index[_crt_sys][FREQ_1], _band_index[_crt_sys][FREQ_2]) > 0)
            {
                Matrix _rot_matrix = _RotMatrix_Ant(_crt_obs, _crt_epo, _crt_rec_epo, rec_obj, true);
                this->_crs_rec_pco += t_gtriple(_rot_matrix * pco.crd_cvect());
            }
        }

        // compute satclk[s]
        double sat_clk = _crt_sat_clk;
        // compute reldelay[m]
        double reldelay = relDelay(this->_crs_rec_pco, this->_crs_rec_vel, this->_crs_sat_pco, this->_crs_sat_vel);
        
        _crt_obs.addclk(sat_clk * CLIGHT - reldelay); 
        _crt_obs.addreldelay(reldelay);                 

        // addrho
        double tmp = (_crs_sat_crd - _crs_rec_crd).norm();
        _crt_obs.addrho(tmp);

        // add drate
        _crt_obs.adddrate((DotProduct((_crs_sat_vel - _crs_rec_vel).crd_cvect(), (_crs_sat_pco - _crs_rec_pco).crd_cvect())) / (CLIGHT * tmp)); 

        // add azim && elev
        t_gtriple xyz_rho = _crs_sat_pco - _crs_rec_pco;
        t_gtriple ell_r, neu_s;

        Matrix _rot_ant2crs = _RotMatrix_Ant(_crt_obs, _crt_rec_epo, _crt_rec_epo, _crt_obj, true);
        neu_s = t_gtriple(_rot_ant2crs.t() * xyz_rho.crd_cvect());
        double NE2 = neu_s[0] * neu_s[0] + neu_s[1] * neu_s[1];
        double ele = acos(sqrt(NE2) / _crt_obs.rho());
        if (sqrt(NE2) / _crt_obs.rho() > 1.0)
        {
            _crt_obs.addele(0.0);
        }
        else
        {
            _crt_obs.addele(ele);
        }

        double offnadir = dotproduct(xyz_rho.crd_cvect(), _crs_sat_pco.crd_cvect()) / xyz_rho.norm() / _crs_sat_pco.norm();
        offnadir = acos(offnadir);
        _crt_obs.addnadir(offnadir);
        _crt_obs.addzen_sat(offnadir);

        double azi = atan2(neu_s[1], neu_s[0]);
        if (azi < 0)
        {
            azi += 2 * G_PI;
        }
        _crt_obs.addazi_rec(azi);
        _crt_obs.addzen_rec((G_PI / 2.0 - ele));

        /// for satellite-side azimuth
        Matrix _rot_matrix_scf2crs = _RotMatrix_Ant(_crt_obs, _crt_epo, _crt_sat_epo, sat_obj, true);
        t_gtriple xyz_s2r = t_gtriple(_rot_matrix_scf2crs.t() * ((-1) * xyz_rho.crd_cvect())); // from sat. to rec. in SCF XYZ
        double azi_sat = atan2(xyz_s2r[0], xyz_s2r[1]);
        if (azi_sat < 0)
            azi_sat += 2 * G_PI;
        _crt_obs.addazi_sat(azi_sat);

        ///add for another elev and azi used in calculating weight matric
        t_gtriple xyz_rh = _trs_sat_crd - _trs_rec_crd;
        t_gtriple ell_(0, 0, 0), neu_sa(0, 0, 0), xRec(0, 0, 0), xyz_s(0, 0, 0);
        xyz2ell(_trs_rec_crd, ell_, false);
        xyz2neu(ell_, xyz_rh, neu_sa);
        double rho0 = sqrt(pow(_trs_rec_crd[0] - xyz_s[0], 2) + pow(_trs_rec_crd[1] - xyz_s[1], 2) + pow(_trs_rec_crd[2] - xyz_s[2], 2));
        double dPhi = OMEGA * rho0 / CLIGHT;
        xRec[0] = _trs_rec_crd[0] * cos(dPhi) - _trs_rec_crd[1] * sin(dPhi);
        xRec[1] = _trs_rec_crd[1] * cos(dPhi) + _trs_rec_crd[0] * sin(dPhi);
        xRec[2] = _trs_rec_crd[2];
        double NE2_ = neu_sa[0] * neu_sa[0] + neu_sa[1] * neu_sa[1];
        double ele_ = acos(sqrt(NE2_) / _crt_obs.rho());

        _crt_obs.addele_leo(ele_);
        
        // check elevation cut-off
        if (_crt_obj->id_type() == t_gdata::REC && _crt_obs.ele_deg() < _minElev)
        {
            if (_spdlog)
                SPDLOG_LOGGER_WARN(_spdlog, "Prepare fail! the elevation is too small");
            return false;
        }

        return true;
    }

    bool t_gprecisebiasGPP::_update_obs_info_GPP(const t_gtime &epoch, t_gallnav *nav, t_gallobj *gallobj, t_gsatdata &obsdata, t_gallpar &pars)
    {
        if (!nav || !gallobj)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "no pars");
            return false;
        }

        // Some frequently used variables are also defined here.
        _crt_epo = epoch;
        _crt_obs = obsdata;
        _crt_sat = obsdata.sat();
        _crt_rec = obsdata.site();
        _crt_sys = obsdata.gsys();
        _crt_obj = _gallobj->obj(_crt_rec);
        _trs_sat_crd = obsdata.satcrd(); 

        if(!_corrt_sat_pcv)
        {
            this->reset_SatPCO(false);
        }

        bool rec_clk_valid = _update_obj_clk_GPP("rec" + _crt_rec, epoch, nav, pars, _crt_rec_clk, _obj_clk);
        bool sat_clk_valid = _update_obj_clk_GPP("sat" + _crt_sat, epoch, nav, pars, _crt_sat_clk, _obj_clk);
        if (!rec_clk_valid || !sat_clk_valid)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "no rec or sat clk for epoch " + epoch.str_ymdhms());
            return false;
        }
        return true;
    }

    bool t_gprecisebiasGPP::_update_obj_clk_GPP(const string &obj, const t_gtime &crt_epoch, t_gallnav *nav, t_gallpar &pars, double &clk, map<string, pair<t_gtime, double>> &obj_clk)
    {
        if (!nav || obj.substr(3).empty())
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "no navigation files or obj is NONE");
            return false;
        }

        // clk[s] from clk files
        double clk_rms = 0.0;
        double dclk = 0.0;
        string type = obj.substr(0, 3);
        string name = obj.substr(3);
        int base_size = dynamic_cast<t_gsetgen *>(_gset)->list_base().size();
        int pv_iod = 0;   // to do
        int    clk_iod = 0;

        // get clk from clk files
        //--can not get any clk informtion from file or gallpar
        int idx_clk = -1;
        if (type == "rec")
        {
            idx_clk = pars.getParam(name, par_type::CLK, "");
        }

        // update clk
        if (obj_clk[name].first != crt_epoch)
        {
            int clk_valid;
            clk_valid = _gall_nav->clk(name, crt_epoch, &clk, &clk_rms, &dclk);

            if (clk_valid < 0)
                clk = 0.0;

            obj_clk[name].first = crt_epoch;
            obj_clk[name].second = clk;
        }
        else
        {
            clk = obj_clk[name].second;
        }

        if (double_eq(clk, 0.0) && idx_clk < 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "no clk for " + name + "  " + crt_epoch.str_ymdhms());
            return false;
        }
        //-- no clk file but clk par exist
        if (double_eq(clk, 0.0) && idx_clk >= 0)
        {
            clk = pars[idx_clk].value() / CLIGHT;
        }
        //-- update clk par
        if (idx_clk >= 0)
        {
            pars[idx_clk].value(clk * CLIGHT);
        }

        return true;
    }

    bool t_gprecisebiasGPP::_prt_obs_ALL(const t_gtime &crt_epo, t_gsatdata &obsdata, t_gallpar &pars, t_gobs &gobs, vector<pair<int, double>> &coeff)
    {
        auto par_list = pars.getPartialIndex(_crt_rec, _crt_sat);
        for (const int& ipar : par_list)
        {
            t_gpar par = pars.getPar(ipar);
            double coeff_value = 0.0;
            t_gbiasmodel::_Partial_basic(crt_epo, _crt_obs, gobs, par, coeff_value);
            if (coeff_value != 0.0)
            {
                coeff.push_back(make_pair(ipar + 1, coeff_value));
            }
        }
        return true;
    }

}
