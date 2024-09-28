/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/
#include "gmodels/gsppmodel.h"
namespace gnut
{
    t_gsppmodel::t_gsppmodel()
    {

        _tropoModel = make_shared<t_gtropo>();
        _gallbias = 0;
    }

    t_gsppmodel::t_gsppmodel(string site, t_gsetbase *settings)
        : _observ(gnut::OBSCOMBIN::IONO_FREE)
    {

        _tropoModel = make_shared<t_gtropo>();
        _gallbias = 0;

        _settings = settings;
        _site = site;
        _phase = false;

        set<GSYS> systems = GNSS_SUPPORTED();
        for (set<GSYS>::iterator it = systems.begin(); it != systems.end(); it++)
        {
            _maxres_C[*it] = dynamic_cast<t_gsetgnss *>(_settings)->maxres_C(*it);
            _maxres_L[*it] = dynamic_cast<t_gsetgnss *>(_settings)->maxres_L(*it);
        }

        _maxres_norm = dynamic_cast<t_gsetproc *>(_settings)->max_res_norm();
        _tropo_mf = dynamic_cast<t_gsetproc *>(_settings)->tropo_mf();
        _trpModStr = dynamic_cast<t_gsetproc *>(_settings)->tropo_model();
        _resid_type = dynamic_cast<t_gsetproc *>(_settings)->residuals();
        _observ = dynamic_cast<t_gsetproc *>(_settings)->obs_combin();
        _cbiaschar = dynamic_cast<t_gsetproc *>(_settings)->cbiaschar();

        _band_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::GPS);
        _band_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::GAL);
        _band_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::GLO);
        _band_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::BDS);
        _band_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::QZS);

        _freq_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::GPS);
        _freq_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::GAL);
        _freq_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::GLO);
        _freq_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::BDS);
        _freq_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::QZS);

        if (_trpModStr == gnut::TROPMODEL::SAASTAMOINEN)
            _tropoModel = make_shared<t_saast>();
        else if (_trpModStr == gnut::TROPMODEL::DAVIS)
            _tropoModel = make_shared<t_davis>();
        else if (_trpModStr == gnut::TROPMODEL::HOPFIELD)
            _tropoModel = make_shared<t_hopf>();
    }
    t_gsppmodel::t_gsppmodel(t_spdlog spdlog, string site, t_gsetbase *settings)
        : _observ(gnut::OBSCOMBIN::IONO_FREE)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }
        if (nullptr == settings)
        {
            spdlog::critical("your set pointer is nullptr !");
            throw logic_error("");
        }
        else
        {
            _settings = settings;
        }
        _tropoModel = make_shared<t_gtropo>();
        _gallbias = 0;
        _site = site;
        _phase = false;
        set<GSYS> systems = GNSS_SUPPORTED();
        for (set<GSYS>::iterator it = systems.begin(); it != systems.end(); it++)
        {
            _maxres_C[*it] = dynamic_cast<t_gsetgnss *>(_settings)->maxres_C(*it);
            _maxres_L[*it] = dynamic_cast<t_gsetgnss *>(_settings)->maxres_L(*it);
        }
        _maxres_norm = dynamic_cast<t_gsetproc *>(_settings)->max_res_norm();
        _tropo_mf = dynamic_cast<t_gsetproc *>(_settings)->tropo_mf();
        _trpModStr = dynamic_cast<t_gsetproc *>(_settings)->tropo_model();
        _resid_type = dynamic_cast<t_gsetproc *>(_settings)->residuals();
        _observ = dynamic_cast<t_gsetproc *>(_settings)->obs_combin();
        _cbiaschar = dynamic_cast<t_gsetproc *>(_settings)->cbiaschar();
        _band_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::GPS);
        _band_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::GAL);
        _band_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::GLO);
        _band_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::BDS);
        _band_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(_settings)->band_index(gnut::QZS);
        _freq_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::GPS);
        _freq_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::GAL);
        _freq_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::GLO);
        _freq_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::BDS);
        _freq_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(_settings)->freq_index(gnut::QZS);
        if (_trpModStr == gnut::TROPMODEL::SAASTAMOINEN)
            _tropoModel = make_shared<t_saast>();
        else if (_trpModStr == gnut::TROPMODEL::DAVIS)
            _tropoModel = make_shared<t_davis>();
        else if (_trpModStr == gnut::TROPMODEL::HOPFIELD)
            _tropoModel = make_shared<t_hopf>();
    }

    t_gsppmodel::~t_gsppmodel()
    {
    }

    int t_gsppmodel::outlierDetect(vector<t_gsatdata> &data,
                                   SymmetricMatrix &Qx,
                                   const SymmetricMatrix &Qsav)
    {

        vector<t_gsatdata>::iterator itMaxVcodeNORM = data.end();
        vector<t_gsatdata>::iterator itMaxVphaseNORM = data.end();

        vector<t_gsatdata>::iterator itMaxVcodeORIG = data.end();
        vector<t_gsatdata>::iterator itMaxVphaseORIG = data.end();

        vector<t_gsatdata>::iterator itDataErase = data.end();

        double maxVcodeNORM = 0.0;
        double maxVphaseNORM = 0.0;

        double maxVcodeORIG = 0.0;
        double maxVphaseORIG = 0.0;

        // find maximal code/phase residuals
        maxVcodeNORM = _maxres(false, data, itMaxVcodeNORM, gnut::RESIDTYPE::RES_NORM);
        maxVphaseNORM = _maxres(true, data, itMaxVphaseNORM, gnut::RESIDTYPE::RES_NORM);

        maxVcodeORIG = _maxres(false, data, itMaxVcodeORIG, gnut::RESIDTYPE::RES_ORIG);
        maxVphaseORIG = _maxres(true, data, itMaxVphaseORIG, gnut::RESIDTYPE::RES_ORIG);

        if (_check_outl(true, maxVphaseNORM, itMaxVphaseNORM, maxVphaseORIG, itMaxVphaseORIG, itDataErase, data))
        {
            auto it = find(_outlier_sat.begin(), _outlier_sat.end(), itDataErase->sat());
            if (it != _outlier_sat.end())
            {
                _outlier_sat.push_back(itDataErase->sat());
            }
            data.erase(itDataErase);
            Qx = Qsav;
            return 1;
        }
        if (_check_outl(false, maxVcodeNORM, itMaxVcodeNORM, maxVcodeORIG, itMaxVcodeORIG, itDataErase, data))
        {
            auto it = find(_outlier_sat.begin(), _outlier_sat.end(), itDataErase->sat());
            if (it == _outlier_sat.end())
            {
                _outlier_sat.push_back(itDataErase->sat());
            }
            data.erase(itDataErase);
            Qx = Qsav;
            return 1;
        }

        return 0;
    }

    double t_gsppmodel::cmpObs(t_gtime &epoch, t_gallpar &param, t_gsatdata &gsatdata, t_gobs &gobs, bool com)
    {

        // Cartesian coordinates to ellipsodial coordinates
        t_gtriple xyz, ell;
        string strEst = dynamic_cast<t_gsetgen *>(_settings)->estimator();
        bool isFLT = (strEst == "FLT");
        if (isFLT)
        {
            if (param.getCrdParam(_site, xyz) < 0)
            {
                xyz = _grec->crd_arp(epoch);
            }
        }
        else
        {
            if (param.getCrdParam(_site, xyz) < 0)
            {
                xyz = _grec->crd(epoch);
            }
            xyz += _grec->eccxyz(epoch);
        }

        xyz2ell(xyz, ell, false);

        t_gtriple satcrd = gsatdata.satcrd();
        ColumnVector cSat = satcrd.crd_cvect();

        string sat = gsatdata.sat();
        string rec = gsatdata.site();
        t_gtime epo = gsatdata.epoch();

        // Tropospheric wet delay correction
        double trpDelay = 0;
        trpDelay = tropoDelay(epoch, param, ell, gsatdata);
        if (fabs(trpDelay) > 50)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "cmpObs", "trpDelay > 50");
            return -1;
        }

        // idx for par correction
        int i = -1;

        // Receiver clock correction
        double clkRec = 0.0;
        i = param.getParam(_site, par_type::CLK, "");
        if (i >= 0)
        {
            clkRec = param[i].value();
        }
        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, _site + " ! warning:  Receiver Clock is not included in parameters!");
        }

        // system time offset
        double isb_offset = isbCorrection(param, sat, rec, gobs);

        // Inter frequency code bias FREQ_3 
        double ifb = 0.0;
        i = param.getParam(_site, par_type::IFB_GPS, "");
        if (i >= 0 && gobs.is_code() && _freq_index[gsatdata.gsys()][gobs.band()] == FREQ_3)
        {
            ifb = param[i].value();
        }

        i = param.getParam(_site, par_type::IFB_GAL, "");
        if (i >= 0 && gobs.is_code() && _freq_index[gsatdata.gsys()][gobs.band()] == FREQ_3)
        {
            ifb = param[i].value();
        }
        i = param.getParam(_site, par_type::IFB_BDS, "");
        if (i >= 0 && gobs.is_code() && _freq_index[gsatdata.gsys()][gobs.band()] == FREQ_3)
        {
            ifb = param[i].value();
        }
        i = param.getParam(_site, par_type::IFB_QZS, "");
        if (i >= 0 && gobs.is_code() && _freq_index[gsatdata.gsys()][gobs.band()] == FREQ_3)
        {
            ifb = param[i].value();
        }

        // Return value
        return gsatdata.rho() +
               clkRec -
               gsatdata.clk() +
               trpDelay +
               isb_offset +
               ifb;
    }

    double t_gsppmodel::cmpObsD(t_gtime &epoch, t_gallpar &param, t_gsatdata &gsatdata, t_gobs &gobs)
    {

        // Cartesian coordinates to ellipsodial coordinates
        t_gtriple xyz, ell;
        ColumnVector cRec(3), vRec(3);

        if (param.getCrdParam(_site, xyz) > 0)
        {
            cRec = xyz.crd_cvect();
        }
        else
        {
            xyz = _grec->crd_arp(epoch);
            cRec = xyz.crd_cvect();
        }
        xyz2ell(xyz, ell, false);

        int i = param.getParam(_site, par_type::VEL_X, "");
        int j = param.getParam(_site, par_type::VEL_Y, "");
        int k = param.getParam(_site, par_type::VEL_Z, "");
        int l = param.getParam(_site, par_type::CLK_RAT, "");
        vRec(1) = param[i].value();
        vRec(2) = param[j].value();
        vRec(3) = param[k].value();
        double dclk_Rec = param[l].value();
        double dclk = gsatdata.dclk();
        t_gtriple satcrd = gsatdata.satcrd();
        t_gtriple satvel = gsatdata.satvel();
        ColumnVector cSat = satcrd.crd_cvect();
        ColumnVector vSat = satvel.crd_cvect();
        ColumnVector e = (cSat - cRec) / gsatdata.rho();

        double res = dotproduct(e, vSat - vRec) +
                     OMEGA / CLIGHT * (vSat(2) * cRec(1) + cSat(2) * vRec(1) - vSat(1) * cRec(2) - cSat(1) * vRec(2));

        return res +
               dclk_Rec -
               dclk;
    }

    double t_gsppmodel::tropoDelay(t_gtime &epoch, t_gallpar &param, t_gtriple ell, t_gsatdata &satdata)
    {

        if (_tropoModel == 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "Tropo Model setting is not correct. Default used! Check config.");
            _tropoModel = make_shared<t_saast>();
        }

        double ele = satdata.ele();

        double delay = 0.0;
        double zwd = 0.0;
        double zhd = 0.0;

        if (abs(ell[2]) > 1E4)
        {
            return 0.0;
        }

        int i;
        i = param.getParam(_site, par_type::TRP, "");
        if (i >= 0)
        {
            zwd = param[i].value();
            zhd = param[i].apriori();
        }
        else
        {
            if (_tropoModel != 0)
            {
                zwd = _tropoModel->getZWD(ell, epoch);
                zhd = _tropoModel->getZHD(ell, epoch);
            }
        }

        if (_tropo_mf == gnut::ZTDMPFUNC::GMF)
        {
            double gmfh, gmfw, dgmfh, dgmfw;
            t_gmf mf;
            mf.gmf(epoch.mjd(), ell[0], ell[1], ell[2], G_PI / 2.0 - ele,
                   gmfh, gmfw, dgmfh, dgmfw);
            delay = gmfh * zhd + gmfw * zwd;
        }
        else if (_tropo_mf == gnut::ZTDMPFUNC::COSZ)
        {
            double mf = 1 / sin(ele);
            delay = mf * zhd + mf * zwd;
        }

        return delay;
    }

    double t_gsppmodel::isbCorrection(t_gallpar &param, string &sat, string &rec, t_gobs &gobs)
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
            int i = param.getParam(_site, par_type::QZS_ISB, "");
            if (i >= 0)
            {
                isb_offset = param[i].value();
            }
            break;
        }
        default:
            throw logic_error("can not support such sys " + t_gsys::gsys2str(gsys));
        }

        return isb_offset;
    }

    void t_gsppmodel::reset_observ(OBSCOMBIN observ)
    {
        _observ = observ;
    }

    void t_gsppmodel::setrec(shared_ptr<t_gobj> rec)
    {
        _grec = rec;
    }

    double t_gsppmodel::_maxres(bool phase, vector<t_gsatdata> &data, vector<t_gsatdata>::iterator &itDATA, RESIDTYPE res_type, GSYS gs)
    {

        vector<t_gsatdata>::iterator it;

        double maxres = 0.0;

        for (it = data.begin(); it != data.end(); it++)
        {
            if (it->gsys() != gs && gs != GNS)
                continue;

            vector<double> res;
            if (phase)
                res = it->residuals(res_type, TYPE_L);
            else
                res = it->residuals(res_type, TYPE_C);

            for (auto itRES = res.begin(); itRES != res.end(); itRES++)
            {
                if (maxres == 0.0 || fabs(*itRES) > maxres)
                {
                    maxres = fabs(*itRES);
                    itDATA = it;
                }
            }
        }

        return maxres;
    }

    bool t_gsppmodel::_check_outl(bool phase, double &maxresNORM, vector<t_gsatdata>::iterator &itDataNORM,
                                  double &maxresORIG, vector<t_gsatdata>::iterator &itDataORIG,
                                  vector<t_gsatdata>::iterator &itDataErase, vector<t_gsatdata> &data)
    {
        map<GSYS, double> map_res;
        if (phase)
            map_res = _maxres_L;
        else
            map_res = _maxres_C;

        GSYS gs;
        if (itDataORIG != data.end())
            gs = itDataORIG->gsys();
        else
            return false;

        if (_resid_type == gnut::RESIDTYPE::RES_ORIG)
        {
            if (maxresORIG > map_res[gs])
            {
                itDataErase = itDataORIG;
                if (phase)
                    _logOutl(true, itDataORIG->sat(), data.size(), maxresORIG, itDataORIG->ele_deg(), itDataORIG->epoch(), _resid_type);
                else
                    _logOutl(false, itDataORIG->sat(), data.size(), maxresORIG, itDataORIG->ele_deg(), itDataORIG->epoch(), _resid_type);
                return true;
            }
        }
        else if (_resid_type == gnut::RESIDTYPE::RES_NORM)
        {
            if (maxresNORM > _maxres_norm)
            {
                itDataErase = itDataNORM;
                if (phase)
                    _logOutl(true, itDataNORM->sat(), data.size(), maxresNORM, itDataNORM->ele_deg(), itDataNORM->epoch(), _resid_type);
                else
                    _logOutl(false, itDataNORM->sat(), data.size(), maxresNORM, itDataNORM->ele_deg(), itDataNORM->epoch(), _resid_type);
                return true;
            }
        }
        else if (_resid_type == gnut::RESIDTYPE::RES_ALL)
        {
            if (maxresORIG > map_res[gs] || maxresNORM > _maxres_norm)
            {
                if (itDataORIG == itDataNORM)
                {
                    itDataErase = itDataORIG;
                    if (phase)
                        _logOutl(true, itDataORIG->sat(), data.size(), maxresORIG, itDataORIG->ele_deg(), itDataORIG->epoch(), gnut::RESIDTYPE::RES_ORIG);
                    else
                        _logOutl(false, itDataORIG->sat(), data.size(), maxresORIG, itDataORIG->ele_deg(), itDataORIG->epoch(), gnut::RESIDTYPE::RES_ORIG);
                    return true;
                }
                else
                {
                    double ratioORIG = maxresORIG / map_res[gs];
                    double ratioNORM = maxresNORM / _maxres_norm;
                    if (ratioNORM >= ratioORIG)
                    {
                        itDataErase = itDataNORM;
                        if (phase)
                            _logOutl(true, itDataNORM->sat(), data.size(), maxresNORM, itDataNORM->ele_deg(), itDataNORM->epoch(), gnut::RESIDTYPE::RES_NORM);
                        else
                            _logOutl(false, itDataNORM->sat(), data.size(), maxresNORM, itDataNORM->ele_deg(), itDataNORM->epoch(), gnut::RESIDTYPE::RES_NORM);
                        return true;
                    }
                    else
                    {
                        itDataErase = itDataORIG;
                        if (phase)
                            _logOutl(true, itDataORIG->sat(), data.size(), maxresORIG, itDataORIG->ele_deg(), itDataORIG->epoch(), gnut::RESIDTYPE::RES_ORIG);
                        else
                            _logOutl(false, itDataORIG->sat(), data.size(), maxresORIG, itDataORIG->ele_deg(), itDataORIG->epoch(), gnut::RESIDTYPE::RES_ORIG);
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void t_gsppmodel::_logOutl(bool phase, string prn, int data_size, double maxres, double ele, t_gtime epo, RESIDTYPE resid_type)
    {
        string obsType = "";
        string resType = "";
        if (phase)
            obsType = "phase";
        else
            obsType = "range";

        if (resid_type == gnut::RESIDTYPE::RES_NORM)
            resType = "Norm residual";
        if (resid_type == gnut::RESIDTYPE::RES_ORIG)
            resType = "Orig residual";
        if (resid_type == gnut::RESIDTYPE::RES_ALL)
            resType = "All residual";

        ostringstream os;
        os << _site << " outlier (" << resType << ": " << obsType << ") " << prn
           << " size:" << fixed << setw(2) << data_size
           << " v: " << fixed << setw(16) << right << setprecision(3) << maxres
           << " ele: " << fixed << setw(6) << setprecision(2) << ele
           << " " << epo.str_hms();
        if (_spdlog)
            SPDLOG_LOGGER_DEBUG(_spdlog, os.str());
    }

} // namespace
