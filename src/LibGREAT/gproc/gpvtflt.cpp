/**
 * @file         gpvtflt.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        main control class for RTK/PPP Processing
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gpvtflt.h"
#include "gutils/gtimesync.h"
#include "gmodels/gprecisebias.h"
#include "gmodels/gprecisebiasGPP.h"
#include <algorithm>
#include "gproc/gqualitycontrol.h"
#include "gio/grtlog.h"

great::t_gpvtflt::t_gpvtflt(string mark, string mark_base, t_gsetbase *gset, t_gallproc *allproc)
    : t_gspp(mark, gset),
      t_gpppflt(mark, gset),
      _fix_mode(FIX_MODE::NO),
      _isFirstFix(true),
      _amb_state(false),
      _site_base(mark_base),
      _gModel_base(nullptr),
      _allproc(allproc),
      _gquality_control(gset, nullptr)
{
    _vel = t_gtriple(0, 0, 0);
    _Qx_vel.ReSize(4);
    _Qx_vel = 0.0;
    _dclkStoModel = new t_whitenoise(dynamic_cast<t_gsetflt *>(_set)->noise_dclk());
    _velStoModel = new t_whitenoise(dynamic_cast<t_gsetflt *>(_set)->noise_vel());
    _fix_mode = dynamic_cast<t_gsetamb *>(_set)->fix_mode();
    _upd_mode = dynamic_cast<t_gsetamb *>(_set)->upd_mode();
    _max_res_norm = dynamic_cast<t_gsetproc *>(_set)->max_res_norm();
    _minsat = dynamic_cast<t_gsetproc *>(_set)->minsat();
    _isBase = false;
    if (!_site_base.empty())
        _isBase = true;
    if (_isBase)
        _rtkinit();
    if (!_pos_kin)
    {
        xyz_standard = dynamic_cast<t_gsetrec *>(gset)->get_crd_xyz(_site);
    }

    _band_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::GPS);
    _band_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::GAL);
    _band_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::GLO);
    _band_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::BDS);
    _band_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::QZS);
    _freq_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::GPS);
    _freq_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::GAL);
    _freq_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::GLO);
    _freq_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::BDS);
    _freq_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::QZS);

    _gobs = dynamic_cast<t_gallobs *>((*_allproc)[t_gdata::ALLOBS]);
    _gnav = dynamic_cast<t_gallprec *>((*_allproc)[t_gdata::GRP_EPHEM]);
    _gupd = dynamic_cast<t_gupd *>((*_allproc)[t_gdata::UPD]);

    this->setOTL(dynamic_cast<t_gallotl *>((*_allproc)[t_gdata::ALLOTL]));
    this->setOBJ(dynamic_cast<t_gallobj *>((*_allproc)[t_gdata::ALLOBJ]));
    this->setDCB(dynamic_cast<t_gallbias *>((*_allproc)[t_gdata::ALLBIAS]));
    this->setFCB(dynamic_cast<t_gallbias *>((*_allproc)[t_gdata::ALLBIAS]));

    _frequency = dynamic_cast<t_gsetproc *>(gset)->frequency();
    _slip_model = dynamic_cast<t_gsetproc *>(gset)->slip_model();
    _gpre = make_shared<t_gpreproc>(_gobs, gset);
    _gpre->spdlog(_spdlog);
    _gpre->setNav(_gnav);

    if (_ambfix)
    {
        delete _ambfix;
        _ambfix = nullptr;
    }
    if (_fix_mode != FIX_MODE::NO)
    {
        _ambfix = new t_gambiguity(_site, _set);
    }

    shared_ptr<t_gbiasmodel> precise_bias(new t_gprecisebiasGPP(_allproc, gset));

    if (!_isBase)
    {
        switch (_observ)
        {
        case OBSCOMBIN::RAW_ALL:
        case OBSCOMBIN::RAW_MIX:
            _base_model = shared_ptr<t_gbasemodel>(new t_gcombALL(gset, precise_bias, _allproc));
            break;
        case OBSCOMBIN::IONO_FREE:
            _base_model = shared_ptr<t_gbasemodel>(new t_gcombIF(gset, precise_bias, _allproc));
            break;
        default:
            _base_model = shared_ptr<t_gbasemodel>(new t_gcombALL(gset, precise_bias, _allproc));
            break;
        }
    }
    else
    {
        _base_model = shared_ptr<t_gbasemodel>(new t_gcombDD(gset, precise_bias, _allproc));
        dynamic_cast<t_gcombDD *>(&(*_base_model))->set_observ(_observ);
        dynamic_cast<t_gcombDD *>(&(*_base_model))->set_site(_site, _site_base);
    }

    _wl_Upd_time = t_gtime(WL_IDENTIFY);
    _ewl_Upd_time = t_gtime(EWL_IDENTIFY);
    _receiverType = dynamic_cast<t_gsetproc *>(_set)->get_receiverType();
}
great::t_gpvtflt::t_gpvtflt(string mark, string mark_base, t_gsetbase *gset, t_spdlog spdlog, t_gallproc *allproc)
    : t_gspp(mark, gset, spdlog),
      t_gpppflt(mark, gset, spdlog),
      _fix_mode(FIX_MODE::NO),
      _isFirstFix(true),
      _amb_state(false),
      _site_base(mark_base),
      _gModel_base(nullptr),
      _allproc(allproc),
      _gquality_control(gset, nullptr)
{
    _gquality_control.spdlog(spdlog);
    _vel = t_gtriple(0, 0, 0);
    _Qx_vel.ReSize(4);
    _Qx_vel = 0.0;
    _dclkStoModel = new t_whitenoise(dynamic_cast<t_gsetflt *>(_set)->noise_dclk());
    _velStoModel = new t_whitenoise(dynamic_cast<t_gsetflt *>(_set)->noise_vel());
    _fix_mode = dynamic_cast<t_gsetamb *>(_set)->fix_mode();
    _upd_mode = dynamic_cast<t_gsetamb *>(_set)->upd_mode();
    _max_res_norm = dynamic_cast<t_gsetproc *>(_set)->max_res_norm();
    _minsat = dynamic_cast<t_gsetproc *>(_set)->minsat();
    _isBase = false;
    if (!_site_base.empty())
        _isBase = true;
    if (_isBase)
        _rtkinit();
    if (!_pos_kin)
    {
        xyz_standard = dynamic_cast<t_gsetrec *>(gset)->get_crd_xyz(_site);
    }
    _band_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::GPS);
    _band_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::GAL);
    _band_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::GLO);
    _band_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::BDS);
    _band_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(gset)->band_index(gnut::QZS);
    _freq_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::GPS);
    _freq_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::GAL);
    _freq_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::GLO);
    _freq_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::BDS);
    _freq_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(gset)->freq_index(gnut::QZS);
    _gobs = dynamic_cast<t_gallobs *>((*_allproc)[t_gdata::ALLOBS]);
    _gnav = dynamic_cast<t_gallprec *>((*_allproc)[t_gdata::GRP_EPHEM]);
    _gupd = dynamic_cast<t_gupd *>((*_allproc)[t_gdata::UPD]);
    this->setOTL(dynamic_cast<t_gallotl *>((*_allproc)[t_gdata::ALLOTL]));
    this->setOBJ(dynamic_cast<t_gallobj *>((*_allproc)[t_gdata::ALLOBJ]));
    this->setDCB(dynamic_cast<t_gallbias *>((*_allproc)[t_gdata::ALLBIAS]));
    this->setFCB(dynamic_cast<t_gallbias *>((*_allproc)[t_gdata::ALLBIAS]));
    _frequency = dynamic_cast<t_gsetproc *>(gset)->frequency();
    _slip_model = dynamic_cast<t_gsetproc *>(gset)->slip_model();
    _gpre = make_shared<t_gpreproc>(_gobs, gset);
    _gpre->spdlog(_spdlog);
    _gpre->setNav(_gnav);
    if (_ambfix)
    {
        delete _ambfix;
        _ambfix = nullptr;
    }
    if (_fix_mode != FIX_MODE::NO)
    {
        _ambfix = new t_gambiguity(_site, _set);
    }
    shared_ptr<t_gbiasmodel> precise_bias(new t_gprecisebiasGPP(_allproc, _spdlog, gset));
    if (!_isBase)
    {
        switch (_observ)
        {
        case OBSCOMBIN::RAW_ALL:
        case OBSCOMBIN::RAW_MIX:
            _base_model = shared_ptr<t_gbasemodel>(new t_gcombALL(gset, _spdlog, precise_bias, _allproc));
            break;
        case OBSCOMBIN::IONO_FREE:
            _base_model = shared_ptr<t_gbasemodel>(new t_gcombIF(gset, _spdlog, precise_bias, _allproc));
            break;
        default:
            _base_model = shared_ptr<t_gbasemodel>(new t_gcombALL(gset, _spdlog, precise_bias, _allproc));
            break;
        }
    }
    else
    {
        _base_model = shared_ptr<t_gbasemodel>(new t_gcombDD(gset, _spdlog, precise_bias, _allproc));
        dynamic_cast<t_gcombDD *>(&(*_base_model))->set_observ(_observ);
        dynamic_cast<t_gcombDD *>(&(*_base_model))->set_site(_site, _site_base);
    }

    _wl_Upd_time = t_gtime(WL_IDENTIFY);
    _ewl_Upd_time = t_gtime(EWL_IDENTIFY);
    _receiverType = dynamic_cast<t_gsetproc *>(_set)->get_receiverType();
}

great::t_gpvtflt::~t_gpvtflt()
{

    if (_ambfix)
    {
        delete _ambfix;
        _ambfix = nullptr;
    }
    if (_gModel_base)
    {
        delete _gModel_base;
        _gModel_base = nullptr;
    }
}

int great::t_gpvtflt::_addObsD(t_gsatdata &satdata, unsigned int &iobs, t_gallpar &param, t_gtriple &XYZ, Matrix &A, ColumnVector &l, DiagonalMatrix &P)
{

    t_gsys gsys(satdata.gsys());
    GSYS gs = gsys.gsys();
    GOBSBAND b1, b2, b3, b4, b5;
    b1 = b2 = b3 = b4 = b5 = BAND;

    if (_auto_band)
    { // automatic dual band selection -> for Anubis purpose
        set<GOBSBAND> bands = satdata.band_avail();
        auto itBAND = bands.begin();
        if (bands.size() < 2)
            return -1;
        b1 = *itBAND;
        itBAND++;
        b2 = *itBAND;
    }
    else
    { // select fix defined band according the table
        b1 = t_gsys::band_priority(gs, FREQ_1);
        b2 = t_gsys::band_priority(gs, FREQ_2);
        b3 = t_gsys::band_priority(gs, FREQ_3);
        b4 = t_gsys::band_priority(gs, FREQ_4);
        b5 = t_gsys::band_priority(gs, FREQ_5);
    }

    t_gobs gobs1, gobs2, gobs3, gobs4, gobs5;

    _getgobs(satdata.sat(), TYPE_D, b1, gobs1);
    _getgobs(satdata.sat(), TYPE_D, b2, gobs2);
    _getgobs(satdata.sat(), TYPE_D, b3, gobs3);
    _getgobs(satdata.sat(), TYPE_D, b4, gobs4);
    _getgobs(satdata.sat(), TYPE_D, b5, gobs5);

    double D1, D2, D3, D4, D5;
    D1 = D2 = D3 = D4 = D5 = 0.0;
    vector<int> index_freq(5, 0);
    D1 = satdata.obs_D(gobs1);
    D2 = satdata.obs_D(gobs2);
    if (b3 != BAND)
        D3 = satdata.obs_D(gobs3);
    if (b4 != BAND)
        D4 = satdata.obs_D(gobs4);
    if (b5 != BAND)
        D5 = satdata.obs_D(gobs5);

    double weight_coef = _weightObs(satdata, gobs1);
    int obsnum = 0;
    if (!double_eq(D1, 0))
    {
        obsnum++;
        index_freq[0] = 1;
    }
    if (!double_eq(D2, 0))
    {
        obsnum++;
        index_freq[1] = 1;
    }
    if (!double_eq(D3, 0))
    {
        obsnum++;
        index_freq[2] = 1;
    }
    if (!double_eq(D4, 0))
    {
        obsnum++;
        index_freq[3] = 1;
    }
    if (!double_eq(D5, 0))
    {
        obsnum++;
        index_freq[4] = 1;
    }

    if (obsnum == 0)
        return -1;

    double sigDoppler = 0.0;
    switch (gs)
    {
    case GPS:
        sigDoppler = _sigDopplerGPS;
        break;
    case GLO:
        sigDoppler = _sigDopplerGLO;
        break;
    case GAL:
        sigDoppler = _sigDopplerGAL;
        break;
    case BDS:
        sigDoppler = _sigDopplerBDS;
        break;
    case QZS:
        sigDoppler = _sigDopplerQZS;
        break;
    default:
        sigDoppler = 0.0;
    }

    //Create reduced measurements (prefit residuals)
    double modObsD = 0.0;
    for (int i = 0; i < obsnum; i++)
    {
        double Di = 0.0;
        if (i == 0)
        {
            modObsD = _gModel->cmpObsD(_epoch, param, satdata, gobs1);
            Di = D1;
        }
        if (i == 1)
        {
            modObsD = _gModel->cmpObsD(_epoch, param, satdata, gobs2);
            Di = D2;
        }
        if (i == 2)
        {
            modObsD = _gModel->cmpObsD(_epoch, param, satdata, gobs3);
            Di = D3;
        }
        if (i == 3)
        {
            modObsD = _gModel->cmpObsD(_epoch, param, satdata, gobs4);
            Di = D4;
        }
        if (i == 4)
        {
            modObsD = _gModel->cmpObsD(_epoch, param, satdata, gobs5);
            Di = D5;
        }
        if (double_eq(modObsD, 0.0) || double_eq(Di, 0.0))
            continue;
        l(iobs + i) = -Di - modObsD;
    }

    // Create weight matrix
    for (int i = 0; i < obsnum; i++)
    {
        P(iobs + i, iobs + i) = weight_coef * (1 / (sigDoppler * sigDoppler));
    }

    // Create first design matrix
    double A_conf;
    int i = param.getParam(_site, par_type::VEL_X, "");
    int j = param.getParam(_site, par_type::VEL_Y, "");
    int k = param.getParam(_site, par_type::VEL_Z, "");
    t_gtriple groundVel(param[i].value(), param[j].value(), param[k].value());

    for (int i = 0; i < obsnum; i++)
    {
        for (unsigned int ipar = 1; ipar <= param.parNumber(); ipar++)
        {
            if (i == 0)
                A_conf = param[ipar - 1].partial_doppler(satdata, XYZ, groundVel);
            if (i == 1)
                A_conf = param[ipar - 1].partial_doppler(satdata, XYZ, groundVel);
            if (i == 2)
                A_conf = param[ipar - 1].partial_doppler(satdata, XYZ, groundVel);
            if (i == 3)
                A_conf = param[ipar - 1].partial_doppler(satdata, XYZ, groundVel);
            if (i == 4)
                A_conf = param[ipar - 1].partial_doppler(satdata, XYZ, groundVel);
            if (double_eq(A_conf, 0.0) || double_eq(l(iobs + i), 0))
                continue;
            A(iobs + i, ipar) = A_conf;
        }
    }

    iobs = iobs + obsnum;

    return 1;
}

int great::t_gpvtflt::_rtkinit()
{
    _param.delAllParam();
    _Qx.Release();
    int ipar = 0;

    // Add coordinates parameters
    if (_crd_est != CONSTRPAR::FIX)
    {
        _param.addParam(t_gpar(_site, par_type::CRD_X, ++ipar, ""));
        _param.addParam(t_gpar(_site, par_type::CRD_Y, ++ipar, ""));
        _param.addParam(t_gpar(_site, par_type::CRD_Z, ++ipar, ""));
    }

    // Add tropospheric wet delay parameter
    if (_tropo_est)
    {
        //rover
        t_gpar trp_rover(_site, par_type::TRP, ++ipar, "");
        trp_rover.setMF(_ztd_mf);
        _param.addParam(trp_rover);
        //base
        t_gpar trp_base(_site_base, par_type::TRP, ++ipar, "");
        trp_base.setMF(_ztd_mf);
        _param.addParam(trp_base);
    }

    // Filling init parameter covariance matrix
    _Qx.ReSize(_param.parNumber());
    _Qx = 0.0;
    double crdInit = _sig_init_crd;
    double ztdInit = _sig_init_ztd;
    for (unsigned int i = 1; i <= _param.parNumber(); i++)
    {
        if (_param[i - 1].parType == par_type::CRD_X)
            _Qx(i, i) = crdInit * crdInit;
        else if (_param[i - 1].parType == par_type::CRD_Y)
            _Qx(i, i) = crdInit * crdInit;
        else if (_param[i - 1].parType == par_type::CRD_Z)
            _Qx(i, i) = crdInit * crdInit;
        else if (_param[i - 1].parType == par_type::TRP)
            _Qx(i, i) = ztdInit * ztdInit;
    }

    return 1;
}

int great::t_gpvtflt::_setCrd()
{
    //save base coordinate to grec
    shared_ptr<gnut::t_gobj> grec_base = _gallobj->obj(_site_base);
    if (grec_base == nullptr)
        return -1;

    _gModel_base = new t_gpppmodel(_site_base, _spdlog, _set);
    _gModel_base->reset_observ(_observ);
    _gModel_base->setOBJ(_gallobj);

    //get base coordinate
    BASEPOS basepos = dynamic_cast<t_gsetproc *>(_set)->basepos();
    t_gtime beg = dynamic_cast<t_gsetgen *>(_set)->beg();
    t_gtime end = dynamic_cast<t_gsetgen *>(_set)->end();

    if (basepos == BASEPOS::SPP)
    { 
        t_gtime time = _gobs->beg_obs(_site_base);
        shared_ptr<t_gsppflt> sppflt = make_shared<t_gsppflt>(_site_base, _set, _spdlog);
        sppflt->minsat(5);
        sppflt->setDAT(_gobs, _gnav);
        sppflt->setOBJ(_gallobj);
        sppflt->setDCB(_gallbias);
        sppflt->processBatch(time, time + 60);
        t_gtriple crd = sppflt->getCrd(time + 60);
        grec_base->crd(crd, t_gtriple(1, 1, 1), FIRST_TIME, LAST_TIME, true);
    }
    else if (basepos == BASEPOS::CFILE)
    {
        // set the value [read from xml]
        auto crd = dynamic_cast<t_gsetrec *>(_set)->get_crd_xyz(_site_base);
        grec_base->crd(crd, t_gtriple(0.01, 0.01, 0.01), beg, end, true);
    }

    //rover
    if (_crd_est == CONSTRPAR::FIX && !_pos_kin)
    {
        // set the value [read from xml/snx/rnxo]
        auto crd = dynamic_cast<t_gsetrec *>(_set)->get_crd_xyz(_site);
        _grec->crd(crd, t_gtriple(0.01, 0.01, 0.01), beg, end, true);
    }

    return 1;
}

bool great::t_gpvtflt::_valid_residual(bool phase_process, string sat_name, enum FREQ_SEQ &f, map<string, map<FREQ_SEQ, pair<int, int>>> &index_l)
{
    try
    {
        /* if no phase observable, psudorange is also unusable */
        if (phase_process)
        {
            index_l.at(sat_name).at(_observ == OBSCOMBIN::IONO_FREE ? FREQ_1 : f).second;
        }
        else
        {
            index_l.at(sat_name).at(_observ == OBSCOMBIN::IONO_FREE ? FREQ_1 : f).second;
            index_l.at(sat_name).at(_observ == OBSCOMBIN::IONO_FREE ? FREQ_1 : f).first;
        }
    }
    catch (exception e)
    {
        return false;
    }
    return true;
}

int great::t_gpvtflt::_combineDD(Matrix &A, SymmetricMatrix &P, ColumnVector &l)
{
    try
    {
        map<string, map<FREQ_SEQ, pair<int, int>>> index_l;
        for (int i = 0; i < _obs_index.size(); i++)
        {
            string sat = _obs_index[i].first;
            FREQ_SEQ f = _obs_index[i].second.first;
            GOBSTYPE obstype = _obs_index[i].second.second;
            if (index_l.find(sat) == index_l.end())
            {
                index_l.insert(make_pair(sat, map<FREQ_SEQ, pair<int, int>>()));
            }
            if (index_l[sat].find(f) == index_l[sat].end())
            {
                index_l[sat].insert(make_pair(f, make_pair(-1, -1)));
            }
            if (obstype == TYPE_C)
                index_l[sat][f].first = i + 1;
            else if (obstype == GOBSTYPE::TYPE_L)
                index_l[sat][f].second = i + 1;
        }

        _obs_index.clear();
        int iobs = 1;

        set<string> sysall = dynamic_cast<t_gsetgen *>(_set)->sys();

        int nrows = A.Nrows();
        Matrix DD;
        DD.ReSize(nrows, nrows);
        DD = 0.0;
        _sat_ref.clear();
        bool isSetRefSat = dynamic_cast<t_gsetamb *>(_set)->isSetRefSat();

        bool isPhaseProcess = true;
        for (auto sys_iter = sysall.begin(); sys_iter != sysall.end(); sys_iter++)
        {
            enum GSYS sys = t_gsys::str2gsys(*sys_iter);
            vector<GOBSBAND> band = dynamic_cast<t_gsetgnss*>(_set)->band(sys);
            int nf = 5;
            if (band.size())
                nf = band.size();
            if (_observ == OBSCOMBIN::IONO_FREE)
                nf = 1;
            FREQ_SEQ f;
            string sat_ref;
            for (FREQ_SEQ freq = FREQ_1; freq <= 2 * nf; freq = (FREQ_SEQ)(freq + 1))
            {
                if (freq <= nf)
                { //phase equations
                    isPhaseProcess = true;
                    f = freq;
                }
                else
                { //code equations
                    isPhaseProcess = false;
                    f = (FREQ_SEQ)(freq - nf);
                }
                if (f > _frequency)
                    continue;
                string sat;
                t_gsatdata obs_sat_ref;
                enum GSYS gs;
                if (!isSetRefSat || (_observ == OBSCOMBIN::RAW_MIX && !isPhaseProcess))
                    sat_ref.clear();
                if (sat_ref.empty())
                {
                    for (auto it = _data.begin(); it != _data.end(); it++)
                    {
                        string sat = it->sat();

                        gs = it->gsys();
                        if (gs == QZS)
                            gs = GPS;
                        if (gs != sys)
                            continue;
                        if ((gs == BDS) && t_gsys::bds_geo(sat))
                            continue;
                        if (!_reset_amb && !_reset_par && it->islip())
                            continue;

                        if (isSetRefSat && (_observ == gnut::OBSCOMBIN::RAW_ALL || _observ == OBSCOMBIN::RAW_MIX))
                        {
                            FREQ_SEQ f1 = FREQ_1;
                            FREQ_SEQ f2 = FREQ_2;
                            FREQ_SEQ f3 = FREQ_3;
                            FREQ_SEQ f4 = FREQ_4;
                            FREQ_SEQ f5 = FREQ_5;
                            if (_valid_residual(isPhaseProcess, sat, f1, index_l) == false || (_frequency >= 2 && nf >= 2 && _valid_residual(isPhaseProcess, sat, f2, index_l) == false) || (_frequency >= 3 && nf >= 3 && _valid_residual(isPhaseProcess, sat, f3, index_l) == false) || (_frequency >= 4 && nf >= 4 && _valid_residual(isPhaseProcess, sat, f4, index_l) == false) || (_frequency >= 5 && nf >= 5 && _valid_residual(isPhaseProcess, sat, f5, index_l) == false))
                                continue;
                        }
                        else if (_valid_residual(isPhaseProcess, sat, f, index_l) == false)
                            continue;

                        if (sat_ref.empty())
                        {
                            sat_ref = sat;
                            obs_sat_ref = *it;
                            continue;
                        }

                        if (it->ele_deg() >= obs_sat_ref.ele_deg())
                        {
                            sat_ref = sat;
                            obs_sat_ref = *it;
                        }
                    }
                }
                if (sat_ref.empty())
                    continue;
                if (_ipSatRep[sys] != "" && sat_ref != _ipSatRep[sys])
                {
                    cerr << "refsat bug" << endl;
                    continue;
                }
                if (freq == FREQ_1)
                    _sat_ref.insert(sat_ref);

                int index_ref;
                if (isPhaseProcess)
                    index_ref = index_l[sat_ref][f].second;
                else
                    index_ref = index_l[sat_ref][f].first;

                for (auto it = _data.begin(); it != _data.end(); it++)
                {
                    sat = it->sat();
                    if (sat == sat_ref)
                        continue;

                    gs = it->gsys();
                    if (gs == QZS)
                        gs = GPS;
                    if (gs != sys)
                        continue;

                    if (_valid_residual(isPhaseProcess, sat, f, index_l) == false)
                        continue;
                    int index_sat;
                    if (isPhaseProcess)
                        index_sat = index_l[sat][f].second;
                    else
                        index_sat = index_l[sat][f].first;

                    DD(iobs, index_ref) = -1;
                    DD(iobs, index_sat) = 1;
                    GOBSTYPE obstype = TYPE_C;
                    if (isPhaseProcess)
                        obstype = GOBSTYPE::TYPE_L;
                    _obs_index.push_back(make_pair(it->sat(), make_pair(f, obstype)));
                    iobs++;
                } //end sat
            }     //end f
        }         //end sys
        // delete zero rows/cols
        iobs--;
        if (iobs <= 0)
        {
            SPDLOG_LOGGER_INFO(_spdlog, _site + " and "+_site_base+" have no common satellite in one system!");
            _delPar(par_type::AMB_IF);
            _delPar(par_type::AMB_L1);
            _delPar(par_type::AMB_L2);
            _delPar(par_type::AMB_L3);
            _delPar(par_type::AMB_L4);
            _delPar(par_type::AMB_L5);
            _delPar(par_type::SION);
            _delPar(par_type::VION);
            return -1;
        }

        DD = DD.Rows(1, iobs);

        A = DD * A;
        l = DD * l;
        P << (DD * P.i() * DD.t()).i();

        return 1;
    }
    catch (...)
    {
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "t_gpvtflt_newmodelmodel", "generate double-differented equation failed!");
        return -1;
    }
}

int great::t_gpvtflt::_postRes(const Matrix &A, const SymmetricMatrix &P, const ColumnVector &l,const ColumnVector &dx)
{
    if (_sat_ref.empty())
        return -1;
    if (!_amb_state)
    {
        _post_A = A;
        _post_P = P;
        _post_l = l;
        return -1;
    }
    string site = _site;
    t_gallpar par_temp = _param;
    double amb_correct=0, amb_correct_2=0,amb_correct_dd=0;
    int idx = 0, idx2 = 0;
    map<string, map<FREQ_SEQ, pair<int, int>>> index_l;
    for (int i = 0; i < _obs_index.size(); i++)
    {
        string sat = _obs_index[i].first;
        FREQ_SEQ f = _obs_index[i].second.first;
        GOBSTYPE obstype = _obs_index[i].second.second;
        if (index_l.find(sat) == index_l.end())
        {
            index_l.insert(make_pair(sat, map<FREQ_SEQ, pair<int, int>>()));
        }
        if (index_l[sat].find(f) == index_l[sat].end())
        {
            index_l[sat].insert(make_pair(f, make_pair(-1, -1)));
        }
        if (obstype == TYPE_C)
            index_l[sat][f].first = i + 1;
        else if (obstype == GOBSTYPE::TYPE_L)
            index_l[sat][f].second = i + 1;
    }

    int nobs = P.Nrows();
    int ncols = A.Ncols();
    _post_A.ReSize(nobs, A.Ncols());
    _post_A = A;
    _post_P = P;
    _post_l.ReSize(nobs);
    _post_l = l;
    DiagonalMatrix delP;
    delP.ReSize(nobs);
    delP = 0;

    t_DD_amb DD = _ambfix->getDD();
    t_DD_amb DD_sav = _ambfix->getDD_sav();
    int iobs = 0;
    vector<int> ind;
    for (auto it = 1; it <= nobs; it++)
        ind.push_back(it);

    for (auto itref = _sat_ref.begin(); itref != _sat_ref.end(); itref++)
    {
        string satref = *itref;
        for (auto itdd = DD.begin(); itdd != DD.end(); itdd++)
        {
            if (itdd->isWlFixed && itdd->isNlFixed)
            {
                string sat1 = get<0>(itdd->ddSats[0]);
                string sat2 = get<0>(itdd->ddSats[1]);

                string crt_sat = sat1;
                if (crt_sat.substr(0, 1) != satref.substr(0, 1))
                    continue;
                if (sat1 != satref && sat2 != satref)
                    continue;
                if (sat1 == satref && sat2 != satref)
                {
                    crt_sat = sat2;
                    itdd->rlc *= -1.0;
                    itdd->rwl *= -1.0;
                    itdd->inl *= -1.0;
                    itdd->iwl *= -1.0;
                }
                FREQ_SEQ freq = FREQ_X;
                double dif = 0.0;
                double dif2 = 0.0;
                double factor_L2 = 0.0;
                bool f2_use = true;
                if (_observ == OBSCOMBIN::IONO_FREE)
                {
                    t_gobsgnss gnss(_spdlog, satref);
                    auto gs = gnss.gsys();
                    double lambda_1 = gnss.wavelength(_band_index[gs][FREQ_1]);
                    double lambda_2 = gnss.wavelength(_band_index[gs][FREQ_2]);

                    double wl = round(itdd->rwl);
                    double mw_coef = lambda_2 * SQR(lambda_1) / (SQR(lambda_2) - SQR(lambda_1));
                    dif = itdd->rlc - (itdd->inl * itdd->factor + wl * mw_coef);
                    freq = FREQ_1;
                }
                else if (_observ == OBSCOMBIN::RAW_ALL||_observ==OBSCOMBIN::RAW_MIX)
                {
                    if (itdd->ambtype == "AMB_L1")
                        freq = FREQ_1;
                    else if (itdd->ambtype == "AMB_L2")
                        freq = FREQ_2;
                    else if (itdd->ambtype == "AMB_L3")
                        freq = FREQ_3;
                    else if (itdd->ambtype == "AMB_L4")
                        freq = FREQ_4;
                    else if (itdd->ambtype == "AMB_L5")
                        freq = FREQ_5;
                    else
                        continue;

                    for (auto itdd_sav = DD_sav.begin(); itdd_sav != DD_sav.end(); itdd_sav++)
                    {
                        if (itdd_sav->ambtype == "AMB_L2" && get<0>(itdd_sav->ddSats[0]) == get<0>(itdd->ddSats[0])&& get<0>(itdd_sav->ddSats[1])== get<0>(itdd->ddSats[1]))
                        {
                            factor_L2 = itdd_sav->factor;
                            f2_use = true;
                        }
                        else
                            f2_use = false;
                        if (f2_use)
                        {
                            idx = par_temp.getParam(_site, par_type::AMB_L2, crt_sat);
                            idx2 = par_temp.getParam(_site, par_type::AMB_L2, satref);
                            amb_correct = dx(idx);
                            amb_correct_2 = dx(idx2);
                            amb_correct_dd = amb_correct - amb_correct_2;
                            dif2 = itdd_sav->rlc - amb_correct_dd - (-itdd->iwl + itdd->inl) * factor_L2;
                            break;
                        }
                    }
                    idx = par_temp.getParam(_site, par_type::AMB_L1, crt_sat);
                    idx2 = par_temp.getParam(_site, par_type::AMB_L1, satref);
                    amb_correct =dx(idx);
                    amb_correct_2 =dx(idx2);
                    amb_correct_dd = amb_correct - amb_correct_2;
                    dif = itdd->rlc-amb_correct_dd - itdd->inl * itdd->factor;
                }
                if (freq != FREQ_X)
                {
                    if (index_l.find(crt_sat) == index_l.end())
                    {
                        continue;
                    }
                    auto index = index_l[crt_sat][freq].second;
                    _post_l(index) += dif;
                     vector<int>::iterator it = find(ind.begin(), ind.end(), index);
                    if (it != ind.end())ind.erase(it);

                    if (f2_use)
                    {
                        auto index2 = index_l[crt_sat][FREQ_2].second;
                        if (index2>0)
                        {
                            _post_l(index2) += dif2;
                            vector<int>::iterator it2 = find(ind.begin(), ind.end(), index2);
                            if (it2 != ind.end())ind.erase(it2);
                        }
                    }
                }
            }
        }  //end dd
    }      //end satref

    vector<int> ind1 = ind, ind2 = ind;

    int pobs = nobs - ind.size();
    Matrix_rem(_post_P, ind);
    Matrix_remR(_post_A, ind1);
    Matrix_remR(_post_l, ind2);
    _post_A.SubMatrix(1, pobs, 4, ncols) = 0.0;

    return iobs;
}

int great::t_gpvtflt::_prepareData()
{

    int flag = 1;
    if (_isBase)
    {
        if (_preprocess(_site_base, _data_base) < 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, _site +  ": Preparing data failed!");
            flag = -1;
        }
        _vBanc_base = _vBanc;
    }
    if (_preprocess(_site, _data) < 0)
    {
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, _site + ": Preparing data failed!");
        flag = -1;
    }
    if (_cntrep == 0)
    {
        t_gtriple xyz = t_gtriple(_vBanc.Rows(1, 3)); 
        _gquality_control.processOneEpoch(_epoch, _site, xyz, _data);
        if (_isBase)
        {
            xyz = t_gtriple(_vBanc_base.Rows(1, 3));
            _gquality_control.processOneEpoch(_epoch, _site_base, xyz, _data_base);
        }
    }

    if (_isBase)
    {
        if ((_nSat = _selcomsat(_data_base, _data)) < _minsat)
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "no common satellite");
            flag = -1;
        }
    }
    if (flag == -1)
    {
        _nSat = 0;
    }

    if (_data.size() < _minsat)
    {
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, _site + _epoch.str_ymdhms(" epoch ") + int2str(_data.size()) + " skipped (data.size < minsat)");
        return -1;
    }

    return flag;
}

int great::t_gpvtflt::_preprocess(const string &ssite, vector<t_gsatdata> &sdata)
{

    Matrix BB;

    BB.ReSize(sdata.size(), 4);
    BB = 0.0;
    int iobs = 0;
    vector<t_gsatdata>::iterator iter = sdata.begin();

    _nSat = sdata.size(); 
    set<string> sat_rm = dynamic_cast<t_gsetgen *>(_set)->sat_rm();
    while (iter != sdata.end())
    {
        // except sat from config file
        string satname = iter->sat();
        if (sat_rm.find(satname) != sat_rm.end())
        {
            iter = sdata.erase(iter);
            continue;
        }
        
        //except sat without pcv
        if (!_isBase)
        {
            shared_ptr<t_gobj> sat_obj = this->_gallobj->obj(satname);
            shared_ptr<t_gpcv> sat_pcv = sat_obj->pcv(_epoch);
            if (!sat_pcv)
            {
                iter = sdata.erase(iter);
                continue;
            }
        }

        //check each satellite obs and crd
        if (!_check_sat(ssite, &*iter, BB, iobs))
        {
            iter = sdata.erase(iter);
            continue;
        }
        else
        {
            ++iter;
        }
    } //end sdata

    if (sdata.size() < _minsat)
    {
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, _epoch.str_ymdhms(ssite + " epoch ") + " skipped (Bancroft not calculated: " + int2str(sdata.size()) + " < _minsat: " + int2str(_minsat) + ")");
        return -1;
    }

    BB = BB.Rows(1, iobs); // delete zero rows

    if (BB.Nrows() < static_cast<int>(_minsat))
    {
        if (SPDLOG_LEVEL_TRACE == _spdlog->level())
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, _epoch.str_ymdhms(_site + " epoch ") + " skipped (Bancroft not calculated: BB.Nrows < _minsat)");
        return -1;
    }

    if (!_cmp_rec_crd(ssite, BB))
    {
        return -1;
    }

    //Compute sat elevation and rho
    iter = sdata.begin();
    while (iter != sdata.end())
    {
        if (!_cmp_sat_info(ssite, &*iter))
        {
            iter = sdata.erase(iter); 
            if (sdata.size() < _minsat)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, ssite + _epoch.str_ymdhms(" epoch ") + int2str(sdata.size()) + " skipped (data.size < _rtk_set->minsat)");
                return -1;
            }
        }
        else
        {
            ++iter;
        }

    } //end sdata

    return 0;
}

void great::t_gpvtflt::_predict(const t_gtime& runEpoch)
{

    // Predict coordinates, clock and troposphere
    // state vector is the same, covariance matrix is predicted with noise
    double bl = 0.0;

    _cntrep++;

    if (!_isBase)
        _syncSys();

    // add/remove ionosphere delay
    //Actually,we only estimate sion/sdsion by using white noise here.   
    if (_iono_est)
        _syncIono();
    if (_isBase && 1) 
    {
        if (_iono_est)
        {
            auto crd = _gallobj->obj(_site_base)->crd_arp(_epoch);
            t_gtriple tmpcrd(_vBanc);
            t_gtriple tmpell, tmpdxyz, tmpneu;
            xyz2ell(crd, tmpell, false);
            tmpdxyz = tmpcrd - crd;
            xyz2neu(tmpell, tmpdxyz, tmpneu);
            bl = tmpneu.norm();

            if (bl < 10000 && _pos_kin)
            {
                _delPar(par_type::VION);
                _delPar(par_type::SION);
            }
        }
    }

    // add/remove inter-frequency biases
    if (!_isBase && _frequency >= 3)
        _syncIFB();

    _Noise.ReSize(_Qx.Nrows());
    _Noise = 0;

    // Predict ambiguity
    // Add or remove ambiguity parameter and appropriate rows/columns covar. matrix
    if (_phase)
    {
        _syncAmb();
    }
  
    _predictCrd();
    _predictClk();
    _predictBias();
    _predictIono(bl, runEpoch);
    _predictTropo();
    _predictAmb();
}

void great::t_gpvtflt::_delPar(const par_type par)
{
    // Remove params and appropriate rows/columns covar. matrix
    for (unsigned int i = 0; i <= _param.parNumber() - 1; i++)
    {
        if (_param[i].parType == par)
        {
            Matrix_remRC(_Qx, _param[i].index, _param[i].index);
            _param.delParam(i);
            _param.reIndex();
            i--;
        }
    }

    return;
}

int great::t_gpvtflt::_combineMW(t_gsatdata &satdata)
{

    string sat = satdata.sat();
    double obs_intv = _sampling;
    t_gtime crt_time = _epoch;
    if (crt_time == _beg_time)
    {
        _MW[crt_time - obs_intv][sat][1] = 0.0;
        _MW[crt_time - obs_intv][sat][2] = 0.0;
        _MW[crt_time - obs_intv][sat][3] = 0.0;
        _MW[crt_time - obs_intv][sat][4] = 0.0;
        _MW[crt_time - obs_intv][sat][5] = 0.0; // ele
    }

    GOBSBAND b1, b2;
    if (_auto_band)
    { // automatic dual band selection -> for Anubis purpose
        set<GOBSBAND> bands = satdata.band_avail();
        auto itBAND = bands.begin();
        b1 = *itBAND;
        itBAND++;
        b2 = *itBAND;
    }
    else
    { // select fix defined band according the table
        vector<GOBSBAND> band = dynamic_cast<t_gsetgnss *>(_set)->band(satdata.gsys());
        if (band.size())
        {
            b1 = band[0];
            b2 = band[1];
        }
        else
        {
            b1 = t_gsys::band_priority(satdata.gsys(), FREQ_1);
            b2 = t_gsys::band_priority(satdata.gsys(), FREQ_2);
        }
    }
    t_gobs gobsP1 = t_gobs(satdata.select_range(b1));
    t_gobs gobsP2 = t_gobs(satdata.select_range(b2));
    t_gobs gobsL1 = t_gobs(satdata.select_phase(b1));
    t_gobs gobsL2 = t_gobs(satdata.select_phase(b2));
    double mw_obs = satdata.MW_cycle(gobsL1, gobsL2, gobsP1, gobsP2);
    bool islip = (satdata.getlli(gobsL1.gobs()) >= 1 || satdata.getlli(gobsL2.gobs()) >= 1);

    if (_isBase)
    {
        t_gsatdata satdata_base;
        for (int i = 0; i < _data_base.size(); i++)
        {
            if (_data_base[i].sat() == satdata.sat())
                satdata_base = _data_base[i];
        }
        if (satdata_base.site() == "")
            mw_obs = 0.0;
        else
        {
            gobsP1 = t_gobs(satdata_base.select_range(b1));
            gobsP2 = t_gobs(satdata_base.select_range(b2));
            gobsL1 = t_gobs(satdata_base.select_phase(b1));
            gobsL2 = t_gobs(satdata_base.select_phase(b2));
            double mw_obs_base = satdata_base.MW_cycle(gobsL1, gobsL2, gobsP1, gobsP2);

            if (double_eq(mw_obs, 0.0) || double_eq(mw_obs_base, 0.0))
                mw_obs = 0.0;
            else
                mw_obs -= mw_obs_base;

            if (!islip)
                islip = (satdata_base.getlli(gobsL1.gobs()) >= 1 || satdata_base.getlli(gobsL2.gobs()) >= 1);
        }
    }

    if (!double_eq(mw_obs, 0.0))
    {
        // calculate smooth MW
        t_gtime pre_time = crt_time - obs_intv;
        if (_MW.find(pre_time) != _MW.end() && _MW[pre_time].find(sat) != _MW[pre_time].end() && !double_eq(_MW[pre_time][sat][4], 0.0) && !islip)
        {
            _MW[crt_time][sat][1] = _MW[pre_time][sat][1] + 1;
            _MW[crt_time][sat][4] = mw_obs;
            if (!double_eq(_MW[crt_time][sat][1], 1.0) && _MW[crt_time][sat][1] > 1.0)
            {
                _MW[crt_time][sat][3] = _MW[pre_time][sat][3] * (_MW[crt_time][sat][1] - 2) /
                                            (_MW[crt_time][sat][1] - 1) +
                                        pow(mw_obs - _MW[pre_time][sat][2], 2) / _MW[crt_time][sat][1];
            }
            else
            {
                _MW[crt_time][sat][3] = _MW[pre_time][sat][3];
            }
            _MW[crt_time][sat][2] = _MW[pre_time][sat][2] + (mw_obs - _MW[pre_time][sat][2]) / _MW[crt_time][sat][1];
            _MW[crt_time][sat][5] = satdata.ele_deg();
        }
        // exclude mw begining ,it is different from last all
        else
        {
            _MW[crt_time][sat][1] = 1;
            _MW[crt_time][sat][4] = mw_obs;
            _MW[crt_time][sat][3] = 0.0;
            _MW[crt_time][sat][2] = mw_obs / _MW[crt_time][sat][1];
            _MW[crt_time][sat][5] = satdata.ele_deg();
        }
    }
    // exclude zero wl
    else
    {
        _MW[crt_time][sat][1] = 0.0;
        _MW[crt_time][sat][2] = 0.0;
        _MW[crt_time][sat][3] = 0.0;
        _MW[crt_time][sat][4] = 0.0;
        _MW[crt_time][sat][5] = 0.0;
        return -1;
    }

    return 1;
}

int great::t_gpvtflt::_processEpochVel()
{
    double threshold = 0.0;

    Matrix A;
    DiagonalMatrix P;
    ColumnVector l, dx;
    vector<t_gsatdata>::iterator it;
    t_gallpar param_vel;

    int ipar = 0;
    param_vel.addParam(t_gpar(_site, par_type::VEL_X, ++ipar, ""));
    param_vel.addParam(t_gpar(_site, par_type::VEL_Y, ++ipar, ""));
    param_vel.addParam(t_gpar(_site, par_type::VEL_Z, ++ipar, ""));
    param_vel.addParam(t_gpar(_site, par_type::CLK_RAT, ++ipar, ""));

    if (!_initialized || _Qx_vel(1, 1) == 0)
    {
        for (int i = 1; i <= 3; i++)
        {
            _Qx_vel(i, i) = _sig_init_vel * _sig_init_vel;
        }
        _Qx_vel(4, 4) = _dclkStoModel->getQ() * _dclkStoModel->getQ();
    }
    else
    {
        for (int i = 1; i <= param_vel.parNumber(); i++)
        {
            if (param_vel[i - 1].parType == par_type::VEL_X)
            {
                param_vel[i - 1].value(_vel[0]);
                _Qx_vel(i, i) += _velStoModel->getQ() * _velStoModel->getQ();
            }
            else if (param_vel[i - 1].parType == par_type::VEL_Y)
            {
                param_vel[i - 1].value(_vel[1]);
                _Qx_vel(i, i) += _velStoModel->getQ() * _velStoModel->getQ();
            }
            else if (param_vel[i - 1].parType == par_type::VEL_Z)
            {
                param_vel[i - 1].value(_vel[2]);
                _Qx_vel(i, i) += _velStoModel->getQ() * _velStoModel->getQ();
            }
            else if (param_vel[i - 1].parType == par_type::CLK_RAT)
            {
                param_vel[i - 1].value(0);
                _Qx_vel(i, i) = _dclkStoModel->getQ() * _dclkStoModel->getQ();
            }
        }
    }

    do
    {
        // define a number of measurements
        unsigned int nObs = _data.size();
        if (_doppler)
        {
            nObs *= 5;
        }

        unsigned int nPar = param_vel.parNumber();
        A.ReSize(nObs, nPar);
        A = 0.0;
        l.ReSize(nObs);
        l = 0.0;
        P.ReSize(nObs);
        P = 0.0;
        dx.ReSize(nPar);
        dx = 0.0;
        unsigned int iobs = 1;

        t_gtriple groundXYZ;
        _param.getCrdParam(_site, groundXYZ);

        // Create matrices and vectors for estimation
        // ----------------------------------------------------------------------------------
        // loop over all measurement in current epoch

        for (it = _data.begin(); it != _data.end();)
        {
            if (_addObsD(*it, iobs, param_vel, groundXYZ, A, l, P) > 0)
            {
            }
            else
            {
                it = _data.erase(it);
                continue;
            } // just return next iterator

            ++it; // increase iterator if not erased
        }
        // delete zero rows/cols
        iobs--;
        A = A.Rows(1, iobs);
        P = P.SymSubMatrix(1, iobs);
        l = l.Rows(1, iobs);

        _filter->update(A, P, l, dx, _Qx_vel);

        int freedom = A.Nrows() - A.Ncols();
        if (freedom < 1)
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "t_gpvtflt", "No redundant observations!");
            freedom = 1;
        }

        int index = param_vel.getParam(_site, par_type::VEL_X, "");

        for (unsigned int iPar = 0; iPar < param_vel.parNumber(); iPar++)
        {
            param_vel[iPar].value(param_vel[iPar].value() + dx(param_vel[iPar].index));
        }
        threshold = sqrt(dx(index + 1) * dx(index + 1) + dx(index + 2) * dx(index + 2) + dx(index + 3) * dx(index + 3));
    } while (threshold > 1e-2);

    if (param_vel.getVelParam(_site, _vel) < 0)
        return -1;

    return 1;
}

int great::t_gpvtflt::_processEpoch(const t_gtime &runEpoch)
{
    if (_grec == nullptr)
    {
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "No receiver settings available!!!");
        return -1;
    }

    _epoch = runEpoch;
    _amb_state = false;

    if (!_crd_xml_valid())
        _sig_init_crd = 100.0;

    Matrix A;
    SymmetricMatrix P;
    ColumnVector l, dx;
    ColumnVector v_norm;
    SymmetricMatrix Qsav, QsavBP;
    t_gallpar XsavBP;
    double vtpv;
    int nobs_total, npar_number;
    string outlier = "";
    // number of iterations caused by outliers
    _cntrep = 0; 
    _crt_SNR.clear();

    do
    {
        _remove_sat(outlier);

        if (_prepareData() < 0)
        {
            if (_initialized)
            {
                _predict(runEpoch); 
            }
            return -1;
        }

        QsavBP = _Qx;
        XsavBP = _param;
        _predict(runEpoch);
        _initialized = true;

        if (_data.size() < _minsat)
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "Not enough visible satellites!");
            _restore(QsavBP, XsavBP);
            return -1;
        }

        // define a number of measurements
        unsigned int nObs = _data.size();
        unsigned int mult = 1;
        if (_observ == OBSCOMBIN::RAW_ALL )
        {
            mult = 2;
            nObs *= 5;
        } 
        if (_observ == OBSCOMBIN::RAW_MIX)
        {
            mult = 1;
            nObs *= 5;
        }
        if (_phase)
        {
            mult *= 2;
            nObs *= 2;
        } 

        unsigned int nPar = _param.parNumber();
        unsigned int iobs = 1;

        _frqNum.clear();
        _obs_index.clear();
        if (_isBase)
        {
            dynamic_cast<t_gcombDD *>(&(*_base_model))->set_base_data(&_data_base);
            dynamic_cast<t_gcombDD *>(&(*_base_model))->set_rec_info(_gallobj->obj(_site_base)->crd_arp(_epoch), _vBanc(4), _vBanc_base(4));
        }
        // use combmodel
        t_gfltEquationMatrix equ;

        iobs = _cmp_equ(equ);

        equ.chageNewMat(A, P, l, nPar);
        dx.ReSize(nPar);
        dx = 0.0;
        
        // generate obs_index
        _obs_index.clear();
        _generateObsIndex(equ);

        if (iobs < _minsat * mult)
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "Not enough processable observations!");
            _restore(QsavBP, XsavBP);
            return -1;
        }

        if (_isBase)
        {
            if (_combineDD(A, P, l) < 0)
                return -1;
        }

        Qsav = _Qx;

        try
        {
            _filter->update(A, P, l, dx, _Qx);
        }
        catch (...)
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "t_gpvtflt", " filter update failed!");
            _Qx = Qsav;
            return -1;
        }

        // increasing variance after update in case of introducing new ambiguity
        if (_cntrep == 1 && !_reset_amb && !_reset_par && !_pos_kin)
        {
            for (size_t iPar = 0; iPar < _param.parNumber(); iPar++)
            {
                if (_param[iPar].str_type().find("AMB") != string::npos)
                {
                    string sat = _param[iPar].prn;
                    if (_newAMB.find(sat) != _newAMB.end())
                    {
                        if (_newAMB[sat] == 1)
                            _Qx(iPar + 1, iPar + 1) += 10;
                        if (_newAMB[sat] == 2 && _Qx(iPar + 1, iPar + 1) > 0.01)
                            _Qx(iPar + 1, iPar + 1) += 1;
                    }
                }
            }
            auto amb_prn = _param.amb_prns();
            for (const auto &sat : amb_prn)
            {
                if (_newAMB.find(sat) != _newAMB.end())
                    _newAMB[sat]++;
            }
        }
        _posterioriTest(A, P, l, dx, _Qx, v_norm, vtpv);
        nobs_total = A.Nrows();
        npar_number = A.ncols();
        _realnobs = l.size();

    } while (_outlierDetect(v_norm, Qsav, outlier) != 0);
    _nSat_excl = _nSat - _data.size(); // number of excluded satellites due to different reasons

    try
    {
        _n_ALL_flt++;
        Cholesky(_Qx);
    }
    catch (NPDException)
    {
        _n_NPD_flt++;
    }

    if (_data.size() < _minsat)
    {
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, _site + _epoch.str_ymdhms(" epoch ") + " skipped: " + int2str(_data.size()) + " < _minsat)");
        _restore(QsavBP, XsavBP);
        return -1;
    }

    if (_doppler)
    {
        int irc = _processEpochVel();
        if (irc < 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, "t_gpvtflt", "estimate velocity by doppler failed!");
            _vel = t_gtriple(0, 0, 0);
            _Qx_vel = 0;
        }
    }

    _filter->add_data(_param, dx, _Qx, _sig_unit, Qsav);
    _filter->add_data(A, P, l);
    _filter->add_data(vtpv, nobs_total, npar_number);

    t_gallpar param_after = _param;

	_amb_resolution();
	if (_amb_state) _postRes(A, P, l,dx);

    for (unsigned int iPar = 0; iPar < _param.parNumber(); iPar++)
    {
        _param[iPar].value(_param[iPar].value() + dx(_param[iPar].index));
    }

    return _amb_state ? 1 : 0;
}

int great::t_gpvtflt::_amb_resolution()
{
    ColumnVector dx_tmp = _filter->dx();
    SymmetricMatrix Qx_tmp = _filter->Qx();
    _param_fixed = _filter->param();
    _amb_state = false;

    if (_fix_mode != FIX_MODE::NO )
    {
        if (_gupd && _gupd->wl_epo_mode())
        {
            _ewl_Upd_time = _epoch;
            _wl_Upd_time = _epoch;
        }

        if (_isFirstFix)
        {
            // set glofrq number
            _glofrq_num = _gobs->glo_freq_num();
            if (_glofrq_num.empty() && _gnav)
            {
                _glofrq_num = _gnav->glo_freq_num();
            }
            _ambfix->setLOG(_spdlog);
            _ambfix->setWaveLength(_glofrq_num);
            _ambfix->setUPD(_gupd);
            _isFirstFix = false;
        }
        _ambfix->setObsType(_observ);
        _ambfix->setActiveAmb(_filter->npar_number());
        if (_observ == OBSCOMBIN::IONO_FREE)
            _ambfix->setMW(_MW[_epoch]);
        else
        {
            _ambfix->setELE(_crt_ele);
            _ambfix->setSNR(_crt_SNR);
        }

        // getting the reference satellite
        bool isSetRefSat = dynamic_cast<t_gsetamb *>(_set)->isSetRefSat();
        if (isSetRefSat && !_isBase)
        { //ppp mode
            bool ref_valid = _getSatRef();
            if (!ref_valid)
                return 0;
        }
        if (!isSetRefSat)
            _sat_ref.clear();
        _ambfix->setSatRef(_sat_ref);

        if ((_observ == gnut::OBSCOMBIN::RAW_ALL || _observ == gnut::OBSCOMBIN::RAW_MIX) && _frequency >= 3)
        {
            _ambfix->processBatch(_epoch, _filter, "EWL");
        }
        if ((_observ == gnut::OBSCOMBIN::RAW_ALL || _observ == gnut::OBSCOMBIN::RAW_MIX) && _frequency >= 4)
        {
            _ambfix->processBatch(_epoch, _filter, "EWL24");
        }
        if ((_observ == gnut::OBSCOMBIN::RAW_ALL || _observ == gnut::OBSCOMBIN::RAW_MIX) && _frequency >= 5)
        {
            _ambfix->processBatch(_epoch, _filter, "EWL25");
        }
        if (_observ == OBSCOMBIN::RAW_ALL || (_observ == OBSCOMBIN::RAW_MIX  && _frequency >= 2))
        {
            _ambfix->processBatch(_epoch, _filter, "WL");
        }


        int nlfix_valid = _ambfix->processBatch(_epoch, _filter, "NL");
        if (nlfix_valid < 0)
        {
            _amb_state = false;
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, _site + _epoch.str_ymdhms(" epoch ") + ": fix ambiguity failed !");
        }
        else
            _amb_state = _ambfix->amb_fixed();
    }

    // output the fixed result
    ostringstream os;
    if (_amb_state) //fixed
    {
        _param_fixed = _ambfix->getFinalParams();
        _prtOut(_epoch, _param_fixed, _filter->Qx(), _data, os, line, true);
        _prt_port(_epoch, _param_fixed, _filter->Qx(), _data);
        cout << _epoch.str_ymdhms() << endl;
	}
    else
    {
        for (unsigned int iPar = 0; iPar < _param_fixed.parNumber(); iPar++)
        {
            _param_fixed[iPar].value(_param_fixed[iPar].value() + dx_tmp(_param_fixed[iPar].index));
        }
        _prtOut(_epoch, _param_fixed, Qx_tmp, _data, os, line, true);
        _prt_port(_epoch, _param_fixed, Qx_tmp, _data);
    }

    // Print flt results
    if (_flt)
    {
        _flt->write(os.str().c_str(), os.str().size());
        _flt->flush();
    }

    return 1;
}


bool t_gpvtflt::_getSatRef()
{
    _sat_ref.clear();
    one_epoch_upd wl_upd, nl_upd, ewl_upd;

    if (_gupd)
    {
        wl_upd = _gupd->get_epo_upd(UPDTYPE::WL, _wl_Upd_time);
        ewl_upd = _gupd->get_epo_upd(UPDTYPE::EWL, _ewl_Upd_time);
        if (_upd_mode == UPD_MODE::UPD)
        {
            nl_upd = _gupd->get_epo_upd(UPDTYPE::NL, _epoch);
            if (wl_upd.size() == 0 || nl_upd.size() == 0)
            {
                return false;
            }
        }
    }
    t_gallpar params_ALL = _filter->param();
    set<string> sysall = dynamic_cast<t_gsetgen *>(_set)->sys();
    for (auto sys_iter = sysall.begin(); sys_iter != sysall.end(); sys_iter++)
    {
        GSYS sys = t_gsys::str2gsys(*sys_iter);
        double max_mw = 0.0;
        string ref;

        for (int i = 0; i < params_ALL.parNumber(); i++)
        {
            if (params_ALL[i].str_type().find("AMB") == string::npos)
                continue;

            if (t_gsys::sat2gsys(params_ALL[i].prn) != sys)
                continue;

            if (sys == GSYS::BDS && stoi(params_ALL[i].prn.substr(1, 3)) <= 5)
                continue; // exclude BDS C01~C05

            if (_observ == gnut::OBSCOMBIN::RAW_MIX && _receiverType != RECEIVERTYPE::And)
            {
                if (_sat_freqs[params_ALL[i].prn] == "1")
                    continue;
            }

            if (_upd_mode == UPD_MODE::UPD)
            {
                if (_gupd && _frequency >= 3 && (ewl_upd.find(params_ALL[i].prn) == ewl_upd.end() || ewl_upd[params_ALL[i].prn]->npoint <= 2))
                    continue;
                if (_gupd && !(wl_upd.find(params_ALL[i].prn) != wl_upd.end() && nl_upd.find(params_ALL[i].prn) != nl_upd.end()))
                    continue;
                if (_gupd && !(wl_upd[params_ALL[i].prn]->npoint > 2))
                    continue;
            }

            if (_crt_ele[params_ALL[i].prn] < 15 || (_epoch - params_ALL[i].beg <= 0))
                continue;

            double temp = (_epoch - params_ALL[i].beg + 1) * _crt_ele[params_ALL[i].prn];

            if (temp > max_mw)
            {
                max_mw = temp;
                ref = params_ALL[i].prn;
            }
        }

        if (!ref.empty())
        {
            _sat_ref.insert(ref);
        }
    }
    if (_sat_ref.empty())
        return false;
    else
        return true;
}

int great::t_gpvtflt::_outlierDetect(const ColumnVector &v, const SymmetricMatrix &Qsav, string &sat)
{

    int nobs = v.Nrows();
    double max = 0.0;
    int idx = 0;
    for (int i = 1; i <= nobs; i++)
    {
        if (fabs(v(i)) > max && fabs(v(i)) > _max_res_norm)
        {
            max = fabs(v(i));
            idx = i;
        }
    }
    if (idx > 0 && idx <= _obs_index.size())
    {
        _Qx = Qsav;
        sat = _obs_index[idx - 1].first;
        string obsType = gobstype2str(_obs_index[idx - 1].second.second);
        ostringstream os;
        os << _site << " outlier (" << obsType << _obs_index[idx - 1].second.first << ") " << sat
           << " v: " << fixed << setw(16) << right << setprecision(3) << max;
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, _epoch.str_ymdhms(" epoch ") + os.str());
    }
    else
    {
        sat = "";
        idx = 0;
    }
    return idx;
}

string great::t_gpvtflt::_gen_kml_description(const t_gtime &epoch, const t_gposdata::data_pos &posdata)
{
    char s[1000];
    string grade = _quality_grade(posdata);
    sprintf(s, "<B>Epoch:</B><BR><BR>\
        <TABLE border = \"1\"  width=\"100%%\"  Align = \"center\">\
        <TR ALIGN = RIGHT>\
        <TR ALIGN = RIGHT><TD ALIGN = LEFT>Time : </TD><TD>%s</TD><TD>%s</TD><TR>\
        <TR ALIGN = RIGHT><TD ALIGN = LEFT>Week : </TD><TD> %s </TD><TD>Sow : </TD><TD>%.4f</TD><TR>\
        <TR ALIGN = RIGHT><TD ALIGN = LEFT>Pos(m): </TD><TD>%.4f</TD><TD>%.4f</TD><TD>%.4f</TD><TR>\
        <TR ALIGN = RIGHT><TD ALIGN = LEFT>Quality : </TD><TD>Q%s </TD><TD>Amb : </TD><TD>%s</TD><TR>\
        <TR ALIGN = RIGHT><TD ALIGN = LEFT>nSats : </TD><TD>%d </TD><TD>PDOP : </TD><TD>%.2f </TD><TR>\
        <TR ALIGN = RIGHT><TD ALIGN = LEFT>Sig(m) : </TD><TD>%.3f</TD><TD>%.3f</TD><TD>%.3f</TD><TR>\
        <TR ALIGN = RIGHT><TD ALIGN = LEFT>Vel(m/s) : </TD><TD>%.3f</TD><TD>%.3f</TD><TD>%.3f</TD><TR>\
        </TABLE>",
        epoch.str_ymd().c_str(), epoch.str_hms().c_str(),
        epoch.str_gwk().c_str(), posdata.t,
        posdata.pos[0], posdata.pos[1], posdata.pos[2],
        grade.c_str(), posdata.amb_state ? "FIXED" : "FLOAT",
        _param.amb_prns().size(), posdata.PDOP,
        sqrt(posdata.Rpos[0]), sqrt(posdata.Rpos[1]), sqrt(posdata.Rpos[2]),
        posdata.vn[0], posdata.vn[1], posdata.vn[2]
    );

    return string(s);
}

string great::t_gpvtflt::_quality_grade(const t_gposdata::data_pos & pos)
{
    if (sqrt(pos.Rpos[0]) < 0.1 &&
        sqrt(pos.Rpos[1]) < 0.1&&
        sqrt(pos.Rpos[2]) < 0.1)
    {
        if (pos.amb_state)
            return "1";
        else
            return "2"; // for float PPP
    }
    else if (
        sqrt(pos.Rpos[0]) < 0.2 &&
        sqrt(pos.Rpos[1]) < 0.2&&
        sqrt(pos.Rpos[2]) < 0.2)
    {
        if (pos.amb_state)
            return "2";
        else
            return "3";
    }
    else if (
        sqrt(pos.Rpos[0]) < 0.5 &&
        sqrt(pos.Rpos[1]) < 0.5 &&
        sqrt(pos.Rpos[2]) < 0.5)
        if (pos.amb_state)
            return "3";
        else
            return "4";
    else if (
        sqrt(pos.Rpos[0]) < 1 &&
        sqrt(pos.Rpos[1]) < 1 &&
        sqrt(pos.Rpos[2]) < 1)
        return "5";
    else
        return "6";
}

int great::t_gpvtflt::processBatch(const t_gtime &beg_r, const t_gtime &end_r, bool prtOut)
{
    _gmutex.lock();

    if (_grec == nullptr)
    {
        ostringstream os;
        os << "ERROR: No object found (" << _site << "). Processing terminated!!! " << beg_r.str_ymdhms() << " -> " << end_r.str_ymdhms() << endl;
        if (_spdlog)
            SPDLOG_LOGGER_ERROR(_spdlog, os.str());
        _gmutex.unlock();
        return -1;
    }

    int sign = 1;
    double subint = 0.1;

    if (!_beg_end)
        sign = -1;

    InitProc(beg_r, end_r, &subint);

    t_gtime now(_beg_time);

    if (_spdlog)
        SPDLOG_LOGGER_INFO(_spdlog, _site + ": Start GNSS Processing filtering: " + now.str_ymdhms() + " " + _end_time.str_ymdhms());
    bool time_loop = true;

    while (time_loop)
    {
        if (_beg_end && (now < _end_time || now == _end_time))
            time_loop = true;
        else if (_beg_end && now > _end_time)
        {
            time_loop = false;
            break;
        }

        if (!_beg_end && (now > _end_time || now == _end_time))
            time_loop = true;
        else if (!_beg_end && now < _end_time)
        {
            time_loop = false;
            break;
        }

        // synchronization
        if (now != _end_time)
        {
            if (!time_sync(now, _sampling, _scale, _spdlog))
            {                                       
                now.add_dsec(sign * subint / 100); // add_dsec used for synchronization!

                continue;
            }
            if (_sampling > 1)
                now.reset_dsec();
        }   

        _slip_detect(now);

        int irc_epo = ProcessOneEpoch(now);
        if (irc_epo < 0)
        {
            if (_sampling > 1)
                now.add_secs(int(sign * _sampling)); // =<1Hz data
            else
                now.add_dsec(sign * _sampling); //  >1Hz data
            continue;
        }
        else
            _success = true;

        if (SPDLOG_LEVEL_TRACE == _spdlog->level())
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, _site + now.str_ymdhms(" processing epoch: "));

        double percent = now.diff(_beg_time) / _end_time.diff(_beg_time) * 100.0;
        if (double_eq(now.sow() % 1, 0.0))
            std::cerr << "\r" << _site << "   " << now.str_ymdhms() << setw(5) << " Q = " << (_amb_state ? 1 : 2) << fixed << setprecision(1) << setw(6) << percent << "%";
        if (_sampling > 1)
            now.add_secs(int(sign * _sampling)); // =<1Hz data
        else
            now.add_dsec(sign * _sampling); //  >1Hz data

    }

    _running = false;

    if (beg_r != end_r)
    { 
        double npd_perc = 0;
        npd_perc = (double(_n_NPD_flt) / double(_n_ALL_flt)) * 100;
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, _site + ": Forward filter -     Not positive definite covariance matrices: " + dbl2str(npd_perc, 0) + " %   (" + int2str(_n_NPD_flt) + ", " + int2str(_n_ALL_flt) + ")");
    }

    _gmutex.unlock();

    return 1;
}

bool great::t_gpvtflt::InitProc(const t_gtime &begT, const t_gtime &endT, double *subint)
{
    if (_beg_end)
    {
        _beg_time = begT;
        _end_time = endT;
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "t_gpvtflt", "Filtering in begin -> end direction!");
    }
    else
    {
        _beg_time = endT;
        _end_time = begT;
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "t_gpvtflt", "Filtering in end -> begin direction!");
    }

    if (_isBase)
    {
        if (_setCrd() < 0)
            return -1;
    }

    _gquality_control.setNav(_gnav);
    _dop.set_data(_gnav, _gobs, _site);
    _running = true;

    if (!_isBase)
    {
        for (int i = 0; i < _param.parNumber(); i++)
            if (_param[i].site == "")
                _param.setSite(_site);
    }

    if (subint)
    {
        if (_scale > 0)
            *subint = 1.0 / _scale;
        if (_sampling > 1)
            *subint = pow(10, floor(log10(_sampling)));
    }

    _prtOutHeader();

    return true;
}

int great::t_gpvtflt::ProcessOneEpoch(const t_gtime &now, vector<t_gsatdata> *data_rover, vector<t_gsatdata> *data_base)
{

    if (_getData(now, data_rover, false) == 0)
    {
        if (SPDLOG_LEVEL_TRACE == _spdlog->level())
        {
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, _site + now.str_ymdhms(" no observation found at epoch: "));
        }
        return -1;
    }

    // apply dcb
    if (_gallbias)
    {
        for (auto &itdata : _data)
        {
            itdata.apply_bias(_gallbias);
        }
    }

    vector<t_gsatdata>::iterator it = _data.begin();
    string double_freq = "";
    string single_freq = "";

    _sat_freqs.clear();
    while (it != _data.end())
    {
        GOBSBAND b1 = _band_index[it->gsys()][FREQ_1];
        GOBSBAND b2 = _band_index[it->gsys()][FREQ_2];

        auto obsL1 = it->select_phase(b1);
        auto obsL2 = it->select_phase(b2);
        auto obsP1 = it->select_range(b1);
        auto obsP2 = it->select_range(b2);
        auto snrL1 = it->getobs(pl2snr(obsP1)) > it->getobs(pl2snr(obsL1)) ? it->getobs(pl2snr(obsP1)) : it->getobs(pl2snr(obsL1));
        auto snrL2 = it->getobs(pl2snr(obsP2)) > it->getobs(pl2snr(obsL2)) ? it->getobs(pl2snr(obsP2)) : it->getobs(pl2snr(obsL2));
        auto ele = it->ele_deg();

        if ((obsL1 == GOBS::X && obsL2 != GOBS::X) || (obsL1 != GOBS::X && obsL2 == GOBS::X))
        {
            single_freq += "  " + it->sat();
            _sat_freqs[it->sat()] = "1";
        }

        if (obsL1 != GOBS::X && obsL2 != GOBS::X)
        {
            double_freq += "  " + it->sat();
            _sat_freqs[it->sat()] = "2";
        }
        ++it;
    }

    if (_isBase)
    {
        if (_getData(now, data_base, true) == 0)
        {
            if (SPDLOG_LEVEL_TRACE == _spdlog->level())
            {
                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, string("gpvtflt:  ") +  _site_base + now.str_ymdhms(" no observation found at epoch: "));
            }
            return -1;
        }
        // apply dcb
        if (_gallbias)
        {
            for (auto &itdata_base : _data_base)
            {
                itdata_base.apply_bias(_gallbias);
            }
        }
    }

    t_gtime obsEpo = _data.begin()->epoch();

    _timeUpdate(obsEpo);

    if (_reset_par > 0)
    {
        if (now.sod() % _reset_par == 0)
        {
            _reset_param();
        }
    }

    // save apriory coordinates
    if (_crd_est != CONSTRPAR::FIX)
        _saveApr(obsEpo, _param, _Qx);
    BASEPOS basepos = dynamic_cast<t_gsetproc*>(_set)->basepos();

    int irc_epo = t_gpvtflt::_processEpoch(obsEpo);

    if (irc_epo < 0)
    {
        _success = false;
        _removeApr(obsEpo);
        if (SPDLOG_LEVEL_TRACE == _spdlog->level())
            if (_spdlog)
                SPDLOG_LOGGER_INFO(_spdlog, _site + now.str_ymdhms(" epoch ") + " was not calculated");
    }
    else
        _success = true;

    return irc_epo;
}

void great::t_gpvtflt::Add_UPD(t_gupd *gupd)
{
    _gupd = gupd;
}

int great::t_gpvtflt::_selcomsat(vector<t_gsatdata> &data_base, vector<t_gsatdata> &data_rover)
{
    int nSat = 0;
    vector<t_gsatdata> bdata, rdata;
    //first to recycle on base-site observations
    for (auto iter_base = data_base.begin(); iter_base != data_base.end(); iter_base++)
    {
        string comsat = iter_base->sat();
        //second to recycle on rover-site obserations
        for (auto iter_rover = data_rover.begin(); iter_rover != data_rover.end(); iter_rover++)
        {
            if (iter_rover->sat() == comsat)
            {
                bdata.push_back(*iter_base);
                rdata.push_back(*iter_rover);
                nSat++;
                break;
            }
        }
    }
    if (nSat)
    {
        data_base = bdata;
        data_rover = rdata;
    }
    return nSat;
}

void great::t_gpvtflt::_udsdAmb()
{

    for (map<string, int>::iterator it = _newAMB.begin(); it != _newAMB.end();)
    {
        if (it->second > 2)
            _newAMB.erase(it++);
        else
            ++it;
    }
    if (_nSat == 0)
    {
        for (unsigned int i = 0; i < _param.parNumber(); i++)
        {
            if (_param[i].parType == par_type::AMB_IF ||
                _param[i].parType == par_type::AMB_L1 ||
                _param[i].parType == par_type::AMB_L2 ||
                _param[i].parType == par_type::AMB_L3 ||
                _param[i].parType == par_type::AMB_L4 ||
                _param[i].parType == par_type::AMB_L5)
            {

                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, "AMB will be removed! For Sat PRN " + _param[i].prn + " Epoch: " + _epoch.str_ymdhms());

                _amb_obs.erase(make_pair(_param[i].prn, _param[i].parType));

                _newAMB.erase(_param[i].prn);

                Matrix_remRC(_Qx, _param[i].index, _param[i].index);
                _param.delParam(i);
                _param.reIndex();
                i--;
            }
        }
        return;
    }
    // Add ambiguity parameter and appropriate rows/columns covar. matrix
    set<string> mapPRN;
    for (int i = 0; i < _data.size(); i++)
    {
        auto rsatdata = _data[i];
        auto bsatdata = _data_base[i];

        GSYS gs = rsatdata.gsys();
        string sat = rsatdata.sat();
        mapPRN.insert(rsatdata.sat());

        vector<GOBSBAND> band = dynamic_cast<t_gsetgnss *>(_set)->band(gs);

        tuple<GOBS, GOBS, GOBS, GOBS> amb_obs_identifier = make_tuple(X, X, X, X);

        if (_observ == OBSCOMBIN::IONO_FREE)
        {
            GOBSBAND b1, b2;
            if (_auto_band)
            { // automatic dual band selection -> for Anubis purpose
                set<GOBSBAND> bands = rsatdata.band_avail();
                auto itBAND = bands.begin();
                if (bands.size() < 2)
                    continue;
                b1 = *itBAND;
                itBAND++;
                b2 = *itBAND;
            }
            else
            { // select fix defined band according the table
                if (!band.empty())
                {
                    b1 = band[0];
                    b2 = band[1];
                }
                else
                {
                    b1 = t_gsys::band_priority(gs, FREQ_1);
                    b2 = t_gsys::band_priority(gs, FREQ_2);
                }
            }
            bool update_amb = false;
            double amb[2] = {0};
            
            for (int j = 0; j < 2; j++)
            {
                t_gsatdata tmpsatdata = rsatdata;
                if (j == 1)
                    tmpsatdata = bsatdata;
                t_gobs gobs1(tmpsatdata.select_phase(b1));
                t_gobs gobs1_P(tmpsatdata.select_range(b1));
                t_gobs gobs2(tmpsatdata.select_phase(b2));
                t_gobs gobs2_P(tmpsatdata.select_range(b2));
                if (j == 0)
                {
                    get<0>(amb_obs_identifier) = gobs1.gobs();
                    get<1>(amb_obs_identifier) = gobs2.gobs();
                }
                else if (j == 1)
                {
                    get<2>(amb_obs_identifier) = gobs1.gobs();
                    get<3>(amb_obs_identifier) = gobs2.gobs();
                }

                if (gobs1.gobs() == X || gobs2.gobs() == X || gobs1_P.gobs() == X || gobs2_P.gobs() == X)
                {
                    update_amb = false;
                    break;
                }
                amb[j] = tmpsatdata.L3(gobs1, gobs2);
                if (double_eq(amb[j], 0.0))
                {
                    update_amb = false;
                    break;
                }
                amb[j] -= tmpsatdata.P3(gobs1_P, gobs2_P);
                if (_param.getParam(_site, par_type::AMB_IF, sat) < 0)
                {
                    update_amb = true;
                }
                if (tmpsatdata.getlli(gobs1.gobs()) >= 1 || tmpsatdata.getlli(gobs2.gobs()) >= 1)
                {
                    update_amb = true;
                    rsatdata.addslip(true);
                }
            }
            if (update_amb == false)
            {
                if (_amb_obs.find(make_pair(sat, par_type::AMB_IF)) == _amb_obs.end())
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_INFO(_spdlog, "amb_obs not correct!" + sat + " " + _epoch.str_hms());
                }
                else if (_amb_obs[make_pair(sat, par_type::AMB_IF)] != amb_obs_identifier)
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_INFO(_spdlog, "Warning: amb_obs switched silently!" + sat + " " + _epoch.str_hms());
                    _amb_obs[make_pair(sat, par_type::AMB_IF)] = amb_obs_identifier;
                }
                continue;
            }
            double sdamb = amb[0] - amb[1];
            int idx = _param.getParam(_site, par_type::AMB_IF, sat);
            if (idx < 0)
            {
                t_gpar newPar(_site, par_type::AMB_IF, _param.parNumber() + 1, sat);
                newPar.value(sdamb); // first ambiguity value
                _param.addParam(newPar);
                _newAMB[sat] = 1;
                Matrix_addRC(_Qx, _param.parNumber(), _param.parNumber());
                _Qx(_param.parNumber(), _param.parNumber()) = _sigAmbig * _sigAmbig;
            }
            else
            {
                _param[idx].value(sdamb);
                _Qx(idx + 1, idx + 1) = _sigAmbig * _sigAmbig;
                _newAMB[sat] = 1;
            }

            _amb_obs[make_pair(sat, par_type::AMB_IF)] = amb_obs_identifier;

        }
        else if (_observ == OBSCOMBIN::RAW_ALL || _observ == OBSCOMBIN::RAW_MIX)
        {
            int newAmb = 0;
            int nf = 5;
            if (_auto_band)
            { // automatic dual band selection -> for Anubis purpose
                GOBSBAND b1, b2;
                set<GOBSBAND> bands = rsatdata.band_avail();
                auto itBAND = bands.begin();
                if (bands.size() < 2)
                    continue;
                b1 = *itBAND;
                itBAND++;
                b2 = *itBAND;
                band.clear();
                band.push_back(b1);
                band.push_back(b2);
            }
            if (band.size())
                nf = band.size();
            for (FREQ_SEQ f = FREQ_1; f <= nf; f = (FREQ_SEQ)(f + 1))
            {
                GOBSBAND b;
                par_type amb_type;
                switch (f)
                {
                case gnut::FREQ_1:
                    amb_type = par_type::AMB_L1;
                    break;
                case gnut::FREQ_2:
                    amb_type = par_type::AMB_L2;
                    break;
                case gnut::FREQ_3:
                    amb_type = par_type::AMB_L3;
                    break;
                case gnut::FREQ_4:
                    amb_type = par_type::AMB_L4;
                    break;
                case gnut::FREQ_5:
                    amb_type = par_type::AMB_L5;
                    break;
                default:
                    break;
                }
                if (f > _frequency)
                    continue;
                if (band.size() >= f)
                { // automatic dual band selection -> for Anubis purpose
                    b = band[f - 1];
                }
                else
                    b = t_gsys::band_priority(gs, f);

                bool update_amb = false;
                bool skip = false;
                double amb[2] = {0};
                amb[0] = amb[1] = 0.0;
                for (int j = 0; j < 2; j++)
                {
                    t_gsatdata tmpsatdata = rsatdata;
                    if (j == 1)
                        tmpsatdata = bsatdata;
                    t_gobs gobs(tmpsatdata.select_phase(b));
                    if (gobs.gobs() == X)
                    {
                        update_amb = false;
                        skip = true;
                        break;
                    }
                    amb[j] = tmpsatdata.obs_L(gobs);
                    if (double_eq(amb[j], 0.0))
                    {
                        update_amb = false;
                        skip = true;
                        break;
                    }
                    t_gobs gobsP(tmpsatdata.select_range(b));
                    if (gobsP.gobs() == X)
                    {
                        update_amb = false;
                        skip = true;
                        break;
                    }
                    amb[j] -= tmpsatdata.obs_C(gobsP);

                    if (j == 0)
                    {
                        get<0>(amb_obs_identifier) = gobs.gobs();
                    }
                    else if (j == 1)
                    {
                        get<2>(amb_obs_identifier) = gobs.gobs();
                    }
                    // add L1 amb - everytime when RAW observations
                    if (_param.getParam(_site, amb_type, sat) < 0)
                    {
                        update_amb = true;
                    }
                    if (tmpsatdata.getlli(gobs.gobs()) >= 1)
                    {
                        update_amb = true;
                        _data[i].addslip(true); // save slip information in rover's data
                    }
                } //end base+rove
                if (skip)
                    continue;
                if (update_amb == false)
                {
                    if (_amb_obs.find(make_pair(sat, amb_type)) == _amb_obs.end())
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_INFO(_spdlog, "amb_obs not correct!" + sat + " " + _epoch.str_hms());
                    }
                    else if (_amb_obs[make_pair(sat, amb_type)] != amb_obs_identifier)
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_INFO(_spdlog, "Warning: amb_obs switched silently!" + sat + " " + _epoch.str_hms());
                        _amb_obs[make_pair(sat, amb_type)] = amb_obs_identifier;
                    }
                    continue;
                }
                double sdamb = amb[0] - amb[1];
                int idx = _param.getParam(_site, amb_type, sat);
                if (idx < 0)
                {
                    t_gpar newPar(_site, amb_type, _param.parNumber() + 1, sat);
                    newPar.value(sdamb);
                    _param.addParam(newPar);
                    Matrix_addRC(_Qx, _param.parNumber(), _param.parNumber());
                    _Qx(_param.parNumber(), _param.parNumber()) = _sigAmbig * _sigAmbig;
                    newAmb = 1;
                }
                else
                {
                    _param[idx].value(sdamb);
                    _Qx(idx + 1, idx + 1) = _sigAmbig * _sigAmbig;
                    newAmb = 1;
                }

                _amb_obs[make_pair(sat, amb_type)] = amb_obs_identifier;

            } 
            if (newAmb)
                _newAMB[sat] = 1;
        } //end uc

    } //end data

    // Remove ambiguity parameter and appropriate rows/columns covar. matrix

    for (unsigned int i = 0; i < _param.parNumber(); i++)
    {
        if (_param[i].parType == par_type::AMB_IF ||
            _param[i].parType == par_type::AMB_L1 ||
            _param[i].parType == par_type::AMB_L2 ||
            _param[i].parType == par_type::AMB_L3 ||
            _param[i].parType == par_type::AMB_L4 ||
            _param[i].parType == par_type::AMB_L5)
        {

            set<string>::iterator prnITER = mapPRN.find(_param[i].prn);
            if (prnITER == mapPRN.end())
            {

                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, "AMB will be removed! For Sat PRN " + _param[i].prn + " Epoch: " + _epoch.str_ymdhms());

                _amb_obs.erase(make_pair(_param[i].prn, _param[i].parType));

                _newAMB.erase(_param[i].prn);

                Matrix_remRC(_Qx, _param[i].index, _param[i].index);
                _param.delParam(i);
                _param.reIndex();
                i--;
            }
        }
    }

    return;
}

void great::t_gpvtflt::_udAmb()
{
    for (map<string, int>::iterator it = _newAMB.begin(); it != _newAMB.end();)
    {
        if (it->second > 2)
            _newAMB.erase(it++);
        else
            ++it;
    }

    // Add ambiguity parameter and appropriate rows/columns covar. matrix
    set<string> mapPRN;

    vector<t_gsatdata>::iterator it;
    for (it = _data.begin(); it != _data.end(); ++it)
    { // loop over all satellites
        mapPRN.insert(it->sat());

        tuple<GOBS, GOBS, GOBS, GOBS> amb_obs_identifier = make_tuple(X, X, X, X);

        GSYS gs = it->gsys();

        GOBSBAND b1 = _band_index[gs][FREQ_1];
        GOBSBAND b2 = _band_index[gs][FREQ_2];
        GOBSBAND b3 = _band_index[gs][FREQ_3];
        GOBSBAND b4 = _band_index[gs][FREQ_4];
        GOBSBAND b5 = _band_index[gs][FREQ_5];

        t_gobs gobs1(it->select_phase(b1));
        t_gobs gobs2(it->select_phase(b2));
        t_gobs gobs3(it->select_phase(b3));
        t_gobs gobs4(it->select_phase(b4));
        t_gobs gobs5(it->select_phase(b5));
        t_gobs gobs1_P(it->select_range(b1));
        t_gobs gobs2_P(it->select_range(b2));
        t_gobs gobs3_P(it->select_range(b3));
        t_gobs gobs4_P(it->select_range(b4));
        t_gobs gobs5_P(it->select_range(b5));

        double LIF, L1, L2, L3, L4, L5;
        double PIF, P1, P2, P3, P4, P5;
        LIF = L1 = L2 = L3 = L4 = L5 = 0.0;
        PIF = P1 = P2 = P3 = P4 = P5 = 0.0;

        if (_observ == gnut::OBSCOMBIN::IONO_FREE)
        {
            LIF = it->L3(gobs1, gobs2);
            PIF = it->P3(gobs1_P, gobs2_P);
        }
        L1 = it->obs_L(gobs1);
        P1 = it->obs_C(gobs1_P);
        L2 = it->obs_L(gobs2);
        P2 = it->obs_C(gobs2_P);
        if (!double_eq(L2, 0.0))
        L3 = it->obs_L(gobs3);
        P3 = it->obs_C(gobs3_P);
        L4 = it->obs_L(gobs4);
        P4 = it->obs_C(gobs4_P);
        L5 = it->obs_L(gobs5);
        P5 = it->obs_C(gobs5_P);
        if (_observ == OBSCOMBIN::IONO_FREE)
        {
            int idx = _param.getParam(_site, par_type::AMB_IF, it->sat());
            if (idx < 0)
            {
                if (double_eq(LIF, 0.0) || double_eq(PIF, 0.0))
                    continue;

                t_gpar newPar(it->site(), par_type::AMB_IF, _param.parNumber() + 1, it->sat());

                newPar.value(LIF - PIF);           // first ambiguity value
                newPar.setTime(_epoch, LAST_TIME); // beg -> end
                _param.addParam(newPar);
                _newAMB[it->sat()] = 1;

                Matrix_addRC(_Qx, _param.parNumber(), _param.parNumber());
                _Qx(_param.parNumber(), _param.parNumber()) = _sigAmbig * _sigAmbig;
                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, "AMB_IF was added! For Sat PRN " + it->sat() + " Epoch: " + _epoch.str_ymdhms());
            }
            else if (it->getlli(gobs1.gobs()) >= 1 || it->getlli(gobs2.gobs()) >= 1)
            {
                if (double_eq(LIF, 0.0) || double_eq(PIF, 0.0))
                    continue;
                it->addslip(true);
                _param[idx].value(LIF - PIF);
                _param[idx].setTime(_epoch, LAST_TIME); // beg -> end  (because of cycle slip)
                _Qx(idx + 1, idx + 1) = _sigAmbig * _sigAmbig;
                _newAMB[it->sat()] = 1;
            }

            // check amb_obs
            amb_obs_identifier = make_tuple(gobs1.gobs(), gobs2.gobs(), X, X);
            if (idx < 0 || it->getlli(gobs1.gobs()) >= 1 || it->getlli(gobs2.gobs()) >= 1)
                _amb_obs[make_pair(it->sat(), par_type::AMB_IF)] = amb_obs_identifier;
            else if (_amb_obs.find(make_pair(it->sat(), par_type::AMB_IF)) == _amb_obs.end())
            {
                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, "amb_obs not correct!" + it->sat() + " " + _epoch.str_hms());
            }
            else if (_amb_obs[make_pair(it->sat(), par_type::AMB_IF)] != amb_obs_identifier)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, "Warning: amb_obs switched silently!" + it->sat() + " " + _epoch.str_hms());
                _amb_obs[make_pair(it->sat(), par_type::AMB_IF)] = amb_obs_identifier;
            }
        }
        else if (_observ == OBSCOMBIN::RAW_ALL || _observ == OBSCOMBIN::RAW_MIX)
        {
            int newAmb = 0;
            for (const auto &ib : _band_index[gs])
            {
                if (ib.first > _frequency)
                    continue;
                t_gobs gobsi;
                double Li;
                double Pi;
                par_type amb_type;

                if (ib.first == FREQ_1)
                {
                    amb_type = par_type::AMB_L1;
                    Li = L1;
                    Pi = P1;
                    gobsi = gobs1;
                }
                else if (ib.first == FREQ_2)
                {
                    amb_type = par_type::AMB_L2;
                    Li = L2;
                    Pi = P2;
                    gobsi = gobs2;
                }
                else if (ib.first == FREQ_3)
                {
                    amb_type = par_type::AMB_L3;
                    Li = L3;
                    Pi = P3;
                    gobsi = gobs3;
                }
                else if (ib.first == FREQ_4)
                {
                    amb_type = par_type::AMB_L4;
                    Li = L4;
                    Pi = P4;
                    gobsi = gobs4;
                }
                else if (ib.first == FREQ_5)
                {
                    amb_type = par_type::AMB_L5;
                    Li = L5;
                    Pi = P5;
                    gobsi = gobs5;
                }
                else
                {
                    throw std::logic_error("Unknown band!!");
                };

                if (double_eq(Li, 0.0) || double_eq(Pi, 0.0))
                    continue;
                int idx = -1;
                // add L1 amb - everytime when RAW observations
                idx = _param.getParam(_site, amb_type, it->sat());
                amb_obs_identifier = make_tuple(gobsi.gobs(), X, X, X);
                // check amb_obs
                if (idx < 0 || it->getlli(gobsi.gobs()) >= 1)
                    _amb_obs[make_pair(it->sat(), amb_type)] = amb_obs_identifier;
                else if (_amb_obs[make_pair(it->sat(), amb_type)] != amb_obs_identifier)
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_INFO(_spdlog, "Warning: amb_obs switched silently!" + it->sat() + " " + _epoch.str_hms());
                    _amb_obs[make_pair(it->sat(), amb_type)] = amb_obs_identifier;
                    it->addlli(gobsi.gobs(), 1);
                }
                if (idx < 0)
                {
                    t_gpar newPar(it->site(), amb_type, _param.parNumber() + 1, it->sat());
                    newPar.value(Li - Pi);
                    newPar.setTime(_epoch, LAST_TIME); // beg -> end
                    _param.addParam(newPar);

                    Matrix_addRC(_Qx, _param.parNumber(), _param.parNumber());
                    _Qx(_param.parNumber(), _param.parNumber()) = _sigAmbig * _sigAmbig;
                    if (_spdlog)
                        SPDLOG_LOGGER_INFO(_spdlog, "RAW AMB_L1 was added! For Sat PRN " + it->sat() + " Epoch: " + _epoch.str_ymdhms());
                    newAmb = 1;
                }
                else if (it->getlli(gobsi.gobs()) >= 1) // check cycle slip
                {
                    it->addslip(true);
                    _param[idx].value(Li - Pi);
                    _param[idx].setTime(_epoch, LAST_TIME); // beg -> end  (because of cycle slip)
                    _Qx(idx + 1, idx + 1) = _sigAmbig * _sigAmbig;
                    _newAMB[it->sat()] = 1;
                    if (_param[idx].amb_ini == true)
                        std::cout << "reset AMB  " << _epoch.str_hms() << endl;
                }
            }

            if (newAmb)
                _newAMB[it->sat()] = 1;
        }

    } // end loop over all observations

    // Remove ambiguity parameter and appropriate rows/columns covar. matrix

    for (unsigned int i = 0; i < _param.parNumber(); i++)
    {
        if (_param[i].parType == par_type::AMB_IF ||
            _param[i].parType == par_type::AMB_L1 ||
            _param[i].parType == par_type::AMB_L2 ||
            _param[i].parType == par_type::AMB_L3 ||
            _param[i].parType == par_type::AMB_L4 ||
            _param[i].parType == par_type::AMB_L5)
        {

            if (_param[i].site != _site)
                continue;
            set<string>::iterator prnITER = mapPRN.find(_param[i].prn);
            if (prnITER == mapPRN.end())
            {

                if (_spdlog)
                    SPDLOG_LOGGER_INFO(_spdlog, "AMB will be removed! For Sat PRN " + _param[i].prn + " Epoch: " + _epoch.str_ymdhms());

                _amb_obs.erase(make_pair(_param[i].prn, _param[i].parType));

                _newAMB.erase(_param[i].prn);

                Matrix_remRC(_Qx, _param[i].index, _param[i].index);
                _param.delParam(i);
                _param.reIndex();
                i--;
            }
        }
    }

    return;
}



void great::t_gpvtflt::Add_rho_azel(const string &site_name, t_gtriple &xyz_s, const t_gtriple &xyz_r, t_gsatdata &obs_sat)
{
    t_gtriple xyz_rho = xyz_s - xyz_r;
    t_gtriple ell_r, neu_s;
    gnut::xyz2ell(xyz_r, ell_r, false);

    xyz2neu(ell_r, xyz_rho, neu_s);

    // Earth rotation correction
    t_gtriple xRec;
    double rho0 = sqrt(pow(xyz_r[0] - xyz_s[0], 2) + pow(xyz_r[1] - xyz_s[1], 2) + pow(xyz_r[2] - xyz_s[2], 2));
    double dPhi = OMEGA * rho0 / CLIGHT;
    xRec[0] = xyz_r[0] * cos(dPhi) - xyz_r[1] * sin(dPhi);
    xRec[1] = xyz_r[1] * cos(dPhi) + xyz_r[0] * sin(dPhi);
    xRec[2] = xyz_r[2];

    // Apply tides
    if (_isBase && site_name != _site)
    {
        string tmp = _site;
        _site = site_name;
        _apply_tides(_epoch, xRec);
        _site = tmp;
    }
    else
        _apply_tides(_epoch, xRec);

    double tmp = (xyz_s.crd_cvect() - xRec.crd_cvect()).norm_Frobenius();

    obs_sat.addrho(tmp);
    double NE2 = neu_s[0] * neu_s[0] + neu_s[1] * neu_s[1];
    double ele = acos(sqrt(NE2) / tmp);
    if (neu_s[2] < 0.0)
    {
        ele *= -1.0;
    }

    if (sqrt(NE2) / tmp > 1.0)
        obs_sat.addele(0.0);
    else
        obs_sat.addele(ele);

    double azi = atan2(neu_s[1], neu_s[0]);
    if (azi < 0)
        azi += 2 * G_PI;
    obs_sat.addazi_rec(azi);

    return;
}

void great::t_gpvtflt::_prtOut(t_gtime &epoch, t_gallpar &X, const SymmetricMatrix &Q, vector<t_gsatdata> &data, ostringstream &os, xml_node &node, bool saveProd)
{

    // get CRD params
    t_gtriple xyz, ell;
    X.getCrdParam(_site, xyz);
    if (_crd_est == CONSTRPAR::FIX)
    {
        shared_ptr<t_gobj> grec = _gallobj->obj(_site);
        xyz = grec->crd_arp(epoch);
    }
    xyz2ell(xyz, ell, false);

    // CRD using eccentricities
    t_gtriple xyz_ecc = xyz - _grec->eccxyz(epoch); // MARKER + ECC = ARP

    // get CRD rms  (XYZ)
    double Xrms = 0.0, Yrms = 0.0, Zrms = 0.0,
           Vxrms = 0.0, Vyrms = 0.0, Vzrms = 0.0;
    double cov_xy = 0.0, cov_xz = 0.0, cov_yz = 0.0;
    int icrdx = _param.getParam(_site, par_type::CRD_X, "");
    int icrdy = _param.getParam(_site, par_type::CRD_Y, "");
    int icrdz = _param.getParam(_site, par_type::CRD_Z, "");
    int icrdclk = _param.getParam(_site, par_type::CLK, "");
    double clk = 0.0;
    if (icrdclk >= 0)
    {
        clk = X[icrdclk].value();
    }
    if (icrdx >= 0 && icrdy >= 0 && icrdz >= 0)
    {
        if (Q(icrdx + 1, icrdx + 1) < 0)
            Xrms = -1;
        else
            Xrms = sqrt(Q(icrdx + 1, icrdx + 1));
        if (Q(icrdy + 1, icrdy + 1) < 0)
            Yrms = -1;
        else
            Yrms = sqrt(Q(icrdy + 1, icrdy + 1));
        if (Q(icrdz + 1, icrdz + 1) < 0)
            Zrms = -1;
        else
            Zrms = sqrt(Q(icrdz + 1, icrdz + 1));
        cov_xy = Q(icrdy + 1, icrdx + 1);
        cov_xz = Q(icrdz + 1, icrdx + 1);
        cov_yz = Q(icrdz + 1, icrdy + 1);
    }

    t_gtriple crd_rms(Xrms, Yrms, Zrms);
    t_gtriple vRec(0, 0, 0);

    if (_doppler)
    {
        vRec = _vel;
        if (_Qx_vel(1, 1) < 0)
            Vxrms = -1;
        else
            Vxrms = sqrt(_Qx_vel(1, 1));
        if (_Qx_vel(2, 2) < 0)
            Vyrms = -1;
        else
            Vyrms = sqrt(_Qx_vel(2, 2));
        if (_Qx_vel(3, 3) < 0)
            Vzrms = -1;
        else
            Vzrms = sqrt(_Qx_vel(3, 3));
    }
    else
    {
        vRec = t_gtriple(0.0, 0.0, 0.0);
    }

    set<string> sat_list;
    for (auto it = _data.begin(); it != _data.end(); it++)
        sat_list.insert(it->sat());
    _dop.set_sats(sat_list);
    double pdop = -1, hdop = -1;
    if (_dop.calculate(epoch, xyz) >= 0)
    {
        pdop = _dop.pdop();
        hdop = _dop.hdop();
    }

    set<string> ambs = X.amb_prns();
    int nsat = ambs.size();

    // get amb status
    string amb = "Float";
    if (_amb_state)
        amb = "Fixed";

    if (_allprod != 0 && saveProd)
    {
        shared_ptr<t_gprod> prdcrd = _allprod->get(_site, t_gdata::POS, epoch);
        if (prdcrd)
        {
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->xyz(xyz_ecc);
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->xyz_rms(crd_rms);
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->cov(COV_XY, cov_xy);
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->cov(COV_XZ, cov_xz);
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->cov(COV_YZ, cov_yz);
        }
        else
        {
            prdcrd = make_shared<t_gprodcrd>(_spdlog, epoch);
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->xyz(xyz_ecc);
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->xyz_rms(crd_rms);
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->cov(COV_XY, cov_xy);
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->cov(COV_XZ, cov_xz);
            dynamic_pointer_cast<t_gprodcrd>(prdcrd)->cov(COV_YZ, cov_yz);
            _allprod->add(prdcrd, _site);
        }

        t_gtriple Ell, XYZ;
        if (_param.getCrdParam(_site, XYZ) > 0)
        {
        }
        else if (_valid_crd_xml)
        {
            XYZ = _grec->crd_arp(_epoch);
        }
        xyz2ell(XYZ, Ell, false);
        int itrp = _param.getParam(_site, par_type::TRP, "");
        if (itrp >= 0)
            _param[itrp].apriori(_gModel->tropoModel()->getZHD(Ell, _epoch));
    }
    t_gtriple blh; 
    xyz2ell(xyz_ecc, blh, true);
    double crt = epoch.sow() + epoch.dsec();
    Eigen::Vector3d Qpos(Xrms*Xrms, Yrms*Yrms, Zrms*Zrms), Qvel(Vxrms*Vxrms, Vyrms*Vyrms, Vzrms*Vzrms);
    Eigen::Vector3d position(xyz_ecc[0], xyz_ecc[1], xyz_ecc[2]), velocity(vRec[0], vRec[1], vRec[2]);
    t_gposdata::data_pos posdata = t_gposdata::data_pos{ crt, position, velocity, Qpos, Qvel, pdop, nsat, _amb_state };
    bool ins = dynamic_cast<t_gsetinp *>(_set)->input_size("imu") > 0 ? true : false;
    // write kml
    if (_kml)
    {
        ostringstream out;
        t_gtriple ell1(ell);
        if (ell1[1] > G_PI)
            ell1[1] = ell1[1] - 2 * G_PI;
        out << fixed << setprecision(11) << " " << setw(0) << ell1[1] * R2D << ',' << ell1[0] * R2D;
        string val = out.str();

        xml_node root = _doc;
        node = this->_default_node(root, _root.c_str());
        xml_node document = node.child("Document");
        xml_node last_child = document.last_child();
        xml_node placemark = document.insert_child_after("Placemark", last_child);
        string q = "#P" + _quality_grade(posdata);
        this->_default_node(placemark, "styleUrl", q.c_str());
        this->_default_node(placemark, "time", int2str(_epoch.sow()).c_str());
        xml_node point = this->_default_node(placemark, "Point");
        this->_default_node(point, "coordinates", val.c_str()); // for point
        xml_node description = placemark.append_child("description");
        description.append_child(pugi::node_cdata).set_value(_gen_kml_description(epoch, posdata).c_str());
        xml_node TimeStamp = placemark.append_child("TimeStamp");
        string time = trim(epoch.str_ymd()) + "T" + trim(epoch.str_hms()) + "Z";
        this->_default_node(TimeStamp, "when", time.c_str());
        xml_node Placemark = document.child("Placemark");
        xml_node LineString = Placemark.child("LineString");
        this->_default_node(LineString, "coordinates", val.c_str(), false); 
    }

    string str_dsec = dbl2str(epoch.dsec());

    double bl = 0;
    t_gtriple tmpell, tmpdxyz, tmpneu;
    if (_isBase)
    {
        auto crd_base = _gallobj->obj(_site_base)->crd_arp(epoch);
        xyz2ell(crd_base, tmpell, false);
        tmpdxyz = xyz - crd_base;
        xyz2neu(tmpell, tmpdxyz, tmpneu);
        bl = tmpneu.norm();
    }


    os << fixed << setprecision(4) << " "
       << " " << epoch.sow() + epoch.dsec();
    if (_crd_est != CONSTRPAR::FIX)
    {
        os << fixed << setprecision(4)
           << " " << setw(15) << xyz_ecc[0] // [m]
           << " " << setw(15) << xyz_ecc[1] // [m]
           << " " << setw(15) << xyz_ecc[2] // [m]
           << " " << setw(10) << vRec[0]    // [m/s]
           << " " << setw(10) << vRec[1]    // [m/s]
           << " " << setw(10) << vRec[2]    // [m/s]
           << fixed << setprecision(4)
           << " " << setw(9) << Xrms  // [m]
           << " " << setw(9) << Yrms  // [m]
           << " " << setw(9) << Zrms  // [m]
           << " " << setw(9) << Vxrms // [m/s]
           << " " << setw(9) << Vyrms // [m/s]
           << " " << setw(9) << Vzrms // [m/s]
            ;
    }
    os << fixed << setprecision(0)
       << " " << setw(5) << nsat // nsat
       << fixed << setprecision(1)
       << fixed << setprecision(2)
       << " " << setw(5) << pdop // pdop
       << fixed << setprecision(2)
       << " " << setw(8) << _sig_unit // m0
       << " " << setw(8) << amb;
    if (_fix_mode != FIX_MODE::NO)
        os << setprecision(2) << " " << fixed << setw(10) << _ambfix->get_ratio();
    if (_isBase)
    {
        os << " " << fixed << setprecision(3) << setw(10) << bl; 
    }
    os << " " << setw(8) << _quality_grade(posdata);
    if (_observ == OBSCOMBIN::RAW_MIX)
    {
        map<string, int> satsnum = _param.freq_sats_num(2);
        os << setw(8) << satsnum["Single"] << setw(8) << satsnum["Double"];
    }
    os << endl;

    return;
}

void great::t_gpvtflt::_prt_port(t_gtime &epoch, t_gallpar &X, const SymmetricMatrix &Q, vector<t_gsatdata> &data)
{

    ostringstream os;
    // get CRD params
    t_gtriple xyz, ell;
    X.getCrdParam(_site, xyz);
    if (_crd_est == CONSTRPAR::FIX)
    {
        shared_ptr<t_gobj> grec = _gallobj->obj(_site);
        xyz = grec->crd_arp(epoch);
    }
    xyz2ell(xyz, ell, false);

    // CRD using eccentricities
    t_gtriple xyz_ecc = xyz - _grec->eccxyz(epoch); // MARKER + ECC = ARP

    double Xrms = 0.0, Yrms = 0.0, Zrms = 0.0;
    int icrdx = _param.getParam(_site, par_type::CRD_X, "");
    int icrdy = _param.getParam(_site, par_type::CRD_Y, "");
    int icrdz = _param.getParam(_site, par_type::CRD_Z, "");
    if (icrdx >= 0 && icrdy >= 0 && icrdz >= 0)
    {
        if (Q(icrdx + 1, icrdx + 1) < 0)
            Xrms = -1;
        else
            Xrms = sqrt(Q(icrdx + 1, icrdx + 1));
        if (Q(icrdy + 1, icrdy + 1) < 0)
            Yrms = -1;
        else
            Yrms = sqrt(Q(icrdy + 1, icrdy + 1));
        if (Q(icrdz + 1, icrdz + 1) < 0)
            Zrms = -1;
        else
            Zrms = sqrt(Q(icrdz + 1, icrdz + 1));
    }

    t_gtriple crd_rms(Xrms, Yrms, Zrms);
    t_gtriple vRec(0, 0, 0);
    if (_doppler)
    {
        vRec = _vel;
    }
    else
    {
        vRec = t_gtriple(0.0, 0.0, 0.0);
    }

    set<string> sat_list;
    for (auto it = _data.begin(); it != _data.end(); it++)
        sat_list.insert(it->sat());
    _dop.set_sats(sat_list);
    double pdop = -1;
    if (_dop.calculate(epoch, xyz) >= 0)
        pdop = _dop.pdop();

    set<string> ambs = X.amb_prns();
    int nsat = ambs.size();

    // get amb status
    string amb = "Float";
    if (_amb_state)
        amb = "Fixed";

    t_gtriple blh;
    xyz2ell(xyz_ecc, blh, true);

    string str_dsec = dbl2str(epoch.dsec());

    if (_isBase)
    {
        auto crd_base = _gallobj->obj(_site_base)->crd_arp(epoch);
        t_gtriple tmpell, tmpdxyz, tmpneu;
        xyz2ell(crd_base, tmpell, false);
        tmpdxyz = xyz - crd_base;
        xyz2neu(tmpell, tmpdxyz, tmpneu);
    }

    os << fixed << setprecision(4) << " "
       << " " << epoch.sow() + epoch.dsec()
       << fixed << setprecision(4)
       << " " << setw(15) << xyz_ecc[0] // [m]
       << " " << setw(15) << xyz_ecc[1] // [m]
       << " " << setw(15) << xyz_ecc[2] // [m]
       << " " << setw(10) << vRec[0]    // [m/s]
       << " " << setw(10) << vRec[1]    // [m/s]
       << " " << setw(10) << vRec[2]    // [m/s]
       << fixed << setprecision(0)
       << " " << setw(5) << nsat // nsat
       << fixed << setprecision(2)
       << " " << setw(5) << pdop // pdop
       << fixed << setprecision(2)
       << " " << setw(8) << amb
       << setw(10) << (_amb_state ? _ambfix->get_ratio() : 0.0)
       << endl;
}

void great::t_gpvtflt::_prtOutHeader()
{
    ostringstream os;

    if (_isBase)
    {
        auto beg = dynamic_cast<t_gsetgen *>(_set)->beg();
        auto crd_base = _gallobj->obj(_site_base)->crd(beg);
        os << "# base -" << _site_base << " Pos(XYZ): " << fixed << setprecision(4) << crd_base[0] << setw(15) << crd_base[1] << setw(15) << crd_base[2] << endl;
        if (_crd_est == CONSTRPAR::FIX)
        {
            auto crd = _gallobj->obj(_site)->crd(beg);
            os << "# rover-" << _site << " Pos(XYZ): " << fixed << setprecision(4) << crd[0] << setw(15) << crd[1] << setw(15) << crd[2] << endl;
        }
    }

    // first line
    os << "#" << setw(15) << "Seconds of Week";
    if (_crd_est != CONSTRPAR::FIX)
    {
        os << " " << setw(12) << "X-ECEF " << // [m]
            " " << setw(15) << "Y-ECEF" <<      // [m]
            " " << setw(15) << "Z-ECEF" <<      // [m]
            " " << setw(10) << "Vx-ECEF" <<      // [m/s]
            " " << setw(10) << "Vy-ECEF" <<      // [m/s]
            " " << setw(10) << "Vz-ECEF" <<      // [m/s]
            " " << setw(9) << "X-RMS"
           << " " << setw(9) << "Y-RMS"
           << " " << setw(9) << "Z-RMS"
           << " " << setw(9) << "Vx-RMS"
           << " " << setw(9) << "Vy-RMS"
           << " " << setw(9) << "Vz-RMS";
    }
    os << " " << setw(5) << "NSat"
       << " " << setw(5) << "PDOP"
       << " " << setw(8) << "sigma0"
       << " " << setw(10) << "AmbStatus";
    if (_fix_mode != FIX_MODE::NO)
        os << " " << setw(10) << "Ratio"; 
    if (_isBase)
        os << " " << setw(10) << "BL"; 
    os << " " << setw(8) << "Quality"; 
    os << endl;

    // second line
    os << "#" << setw(15) << "(s)";
    if (_crd_est != CONSTRPAR::FIX)
    {
        os << " " << setw(12) << "(m)" << // [m]
            " " << setw(15) << "(m)" <<      // [m]
            " " << setw(15) << "(m)" <<      // [m]
            " " << setw(10) << "(m/s)" << // [m/s]
            " " << setw(10) << "(m/s)" << // [m/s]
            " " << setw(10) << "(m/s)" << // [m/s]
            " " << setw(9) << "(m)" <<      // [m]
            " " << setw(9) << "(m)" <<      // [m]
            " " << setw(9) << "(m)" <<      // [m]
            " " << setw(9) << "(m/s)" <<  // [m/s]
            " " << setw(9) << "(m/s)" <<  // [m/s]
            " " << setw(9) << "(m/s)"      // [m/s]
            ;
    }
    os << " " << setw(5) << "(#)"
       << " " << setw(5) << "(#)"
       << " " << setw(8) << "(m)"
       << " " << setw(10) << " ";
    if (_fix_mode != FIX_MODE::NO)
        os << setw(10) << " "; 
    if (_isBase)
        os << " " << setw(10) << "(m)"; 
    os << setw(8) << " ";
    os << endl;

    // Print flt results
    if (_flt)
    {
        _flt->write(os.str().c_str(), os.str().size());
        _flt->flush();
    }
}

void great::t_gpvtflt::_generateObsIndex(t_gfltEquationMatrix &equ)
{
    auto band_index = dynamic_cast<t_gcombmodel *>(&(*_base_model))->get_band_index();
    auto freq_index = dynamic_cast<t_gcombmodel *>(&(*_base_model))->get_freq_index();
    for (int i = 0; i < equ.num_equ(); i++)
    {
        char sys_char = equ.get_satname(i)[0];
        GSYS sys;
        bool sys_valid = true;
        switch (sys_char)
        {
        case 'G':
            sys = GSYS::GPS;
            break;
        case 'R':
            sys = GSYS::GLO;
            break;
        case 'E':
            sys = GSYS::GAL;
            break;
        case 'C':
            sys = GSYS::BDS;
            break;
        case 'J':
            sys = GSYS::QZS;
            break;
        default:
            sys_valid = false;
            break;
        }
        if (!sys_valid)
            throw exception();
        GOBSBAND b = equ.get_obscombtype(i).getBand_1();
        FREQ_SEQ f = freq_index[sys][b];
        GOBSTYPE obstype = equ.get_obscombtype(i).is_code() ? TYPE_C : GOBSTYPE::TYPE_L;
        _obs_index.push_back(make_pair(equ.get_satname(i), make_pair(f, obstype)));
    }
}

bool great::t_gpvtflt::_slip_detect(const t_gtime& now)
{
    _gpre->ProcessBatch(_site, now, now, _sampling, false);
    if (_isBase)
    {
        _gpre->ProcessBatch(_site_base, now, now, _sampling, false);
    }
    return true;
}

int great::t_gpvtflt::_getData(const t_gtime& now, vector <t_gsatdata>* data, bool isBase)
{
    // clean/collect/filter epoch data
    if (data != NULL)
    {
        if (!isBase)
        {
            _data.erase(_data.begin(), _data.end());
            _data = *data;
        }
        else
        {
            _data_base.erase(_data_base.begin(), _data_base.end());
            _data_base = *data;
        }

    }
    else
    {
        if (!isBase)
        {
            _data.erase(_data.begin(), _data.end());
            _data = _gobs->obs(_site, now);
        }
        else
        {
            _data_base.erase(_data_base.begin(), _data_base.end());
            _data_base = _gobs->obs(_site_base, now);
        }
        
    }

    return (isBase ? static_cast<int>(_data_base.size()) : static_cast<int>(_data.size()));
}

bool great::t_gpvtflt::_crd_xml_valid()
{
    t_gtriple crdapr = _grec->crd_arp(_epoch);

    if (double_eq(crdapr[0], 0.0) && double_eq(crdapr[1], 0.0) &&
        double_eq(crdapr[2], 0.0))
    {
        _valid_crd_xml = false;
    }
    else
    {
        _valid_crd_xml = true;
    }

    return _valid_crd_xml;
}

void great::t_gpvtflt::_remove_sat(const string& satid)
{
    vector<t_gsatdata>::iterator it;
    // erase sat satellite because of outliers
    it = _data.begin();
    if (!satid.empty())
    { 
        while (it != _data.end())
        {
            if (it->sat() == satid)
            {
                it = _data.erase(it);
                continue;
            }
            ++it;
        }
    }

    return;
}

bool great::t_gpvtflt::_check_sat(const string& ssite, t_gsatdata* const iter, Matrix& BB, int& iobs)
{
    GSYS gs = iter->gsys();

    //GOBSBAND b1, b2;
    GOBSBAND b1 = _band_index[gs][FREQ_1];
    GOBSBAND b2 = _band_index[gs][FREQ_2];

    iter->spdlog(_spdlog);

    // check data availability
    auto l1 = iter->select_phase(b1);
    auto l2 = iter->select_phase(b2);

    auto p1 = iter->select_range(b1);
    auto p2 = iter->select_range(b2);

    // check data availability
    if (p1 == X || l1 == X)
    {
        return false;
    }

    string strGOBS = gobs2str(p1);
    strGOBS.replace(0, 1, "S");
    double obs_snr1 = iter->getobs(str2gobs(strGOBS));
    double obs_snr2 = iter->getobs(pha2snr(l1));
    _crt_SNR[iter->sat()][FREQ_1] = double_eq(obs_snr2, 0.0) ? obs_snr1 : obs_snr2;

    strGOBS = gobs2str(p2);
    strGOBS.replace(0, 1, "S");
    obs_snr1 = iter->getobs(str2gobs(strGOBS));
    obs_snr2 = iter->getobs(pha2snr(l2));
    _crt_SNR[iter->sat()][FREQ_2] = double_eq(obs_snr2, 0.0) ? obs_snr1 : obs_snr2;

    double P3, L3;
    if (_observ == OBSCOMBIN::RAW_MIX)
    {
        if (p2 == X || l2 == X)
        {
            P3 = iter->obs_C(p1);
            L3 = iter->obs_L(l1);
        }
        else
        {
            P3 = iter->P3(p1, p2);
            L3 = iter->L3(l1, l2);
        }
    }
    else
    {
        if (p2 == X || l2 == X)
        {
            return false;
        }
        P3 = iter->P3(p1, p2);
        L3 = iter->L3(l1, l2);
    }

    if (double_eq(L3, 0.0) && _phase)
    {
        ostringstream str;
        str << "prepareData: erasing data due to no phase double bands observation, "
            << "epo: " << _epoch.str_hms() << ", "
            << "prn: " << iter->sat() << " (" << iter->channel() << ")";
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, str.str());
        return false;
    }

    if (double_eq(P3, 0.0))
    {
        ostringstream str;
        str << "prepareData: erasing data due to no code double bands observation, "
            << "epo: " << _epoch.str_hms() << ", "
            << "prn: " << iter->sat();
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, str.str());
        return false;
    }

    if (_satPos(_epoch, *iter) < 0)
    {
       ostringstream str;
       str << "prepareData: erasing data since _satPos failed, "
           << "epo: " << _epoch.str_hms() << ", "
           << "prn: " << iter->sat();
       if (_spdlog)
           SPDLOG_LOGGER_INFO(_spdlog, str.str());
        return false;
    }
    else
    {
        iobs++;
        BB(iobs, 1) = iter->satcrd().crd(0);
        BB(iobs, 2) = iter->satcrd().crd(1);
        BB(iobs, 3) = iter->satcrd().crd(2);
        BB(iobs, 4) = P3 + iter->clk();
    }
    return true;
}

bool great::t_gpvtflt::_cmp_rec_crd(const string& ssite, Matrix& BB)
{
    _vBanc = 0.0;

    if (BB.Nrows() >= 4)
        gbancroft(BB, _vBanc);
    shared_ptr<t_gobj> grec = _gallobj->obj(ssite);
    if (grec == nullptr)
        return false;

    if (!_initialized && (_vBanc.Rows(1, 3) - grec->crd_arp(_epoch).crd_cvect()).NormFrobenius() > 1000)
    {
        if (ssite == _site)
            _valid_crd_xml = false;
        else
        { 
            _vBanc(1) = grec->crd_arp(_epoch)[0];
            _vBanc(2) = grec->crd_arp(_epoch)[1];
            _vBanc(3) = grec->crd_arp(_epoch)[2];
        }
    }

    if (_valid_crd_xml && !_initialized)
    {
        _vBanc(1) = grec->crd_arp(_epoch)[0];
        _vBanc(2) = grec->crd_arp(_epoch)[1];
        _vBanc(3) = grec->crd_arp(_epoch)[2];
    }

    else if (ssite == _site_base && dynamic_cast<t_gsetproc*>(_set)->basepos() == BASEPOS::CFILE)
    {
        _vBanc(1) = grec->crd_arp(_epoch)[0];
        _vBanc(2) = grec->crd_arp(_epoch)[1];
        _vBanc(3) = grec->crd_arp(_epoch)[2];
    }
    t_gtriple test_xyz(_vBanc);
    t_gtriple test_ell;
    xyz2ell(test_xyz, test_ell, true);
    double radius = test_xyz.norm();
    if (radius < B_WGS - 500)
    {
        string warning = "WARNING: Unexpected site (" + ssite + ") coordinates from Bancroft. Orbits/clocks or code observations should be checked.";
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, warning + _epoch.str_ymdhms(" Epoch "));
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, warning + _epoch.str_ymdhms(" Epoch "));
        if (!_phase)
        {
            return false;            
        }
        return false;
    }

    return true;
}

bool great::t_gpvtflt::_cmp_sat_info(const string& ssite, t_gsatdata* const iter)
{
    t_gtriple xyz_r, xyz_s;
    xyz_s = iter->satcrd();

    shared_ptr<t_gobj> grec = _gallobj->obj(ssite);

    if (_isBase && ssite == _site_base)
    { 
        xyz_r = grec->crd_arp(_epoch);
        if (xyz_r.norm() == 0)
        {
            cerr << endl;
            cerr << xyz_r[0] << " " << xyz_r[1] << " " << xyz_r[2] << endl;
        }
        _vBanc(1) = xyz_r[0];
        _vBanc(2) = xyz_r[1];
        _vBanc(3) = xyz_r[2];
    }
    else
    {
        if (_crd_est == CONSTRPAR::FIX && _valid_crd_xml)
        {
            xyz_r = grec->crd_arp(_epoch);
            _vBanc(1) = xyz_r[0];
            _vBanc(2) = xyz_r[1];
            _vBanc(3) = xyz_r[2];
        }
        else if (_crd_est == CONSTRPAR::EST || _crd_est == CONSTRPAR::KIN)
        {
            if (!_initialized)
            {
                xyz_r.set(_vBanc.Rows(1, 3));
            }
            else
            {
                if (_pos_kin || _crd_est == CONSTRPAR::KIN)
                {
                    xyz_r.set(_vBanc.Rows(1, 3));
                }
                else
                    _param.getCrdParam(ssite, xyz_r);
            }
            if (_pos_kin)
            {
                t_gtriple crd_r = xyz_r - grec->eccxyz(_epoch);
                grec->crd(crd_r, t_gtriple(1, 1, 1), _epoch, LAST_TIME, true);
            }
        }
    }
    Add_rho_azel(ssite, xyz_s, xyz_r, *iter); 

    // check elevation cut-off
    if (iter->ele_deg() < _minElev)
    {
        ostringstream os;
        os << "Erasing " << iter->sat() << " data due to low elevation angle (ele = " << fixed << setprecision(1) << iter->ele_deg()
            << ") " << iter->epoch().str_ymdhms();
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, os.str());
        return false;
    }

    _crt_ele[iter->sat()] = iter->ele_deg();

    // check satellite eclipsing
    if (iter->ecl())
    {
        ostringstream os;
        os << "Erasing " << iter->sat() << " data due to satellite eclipsing (beta = " << fixed << setprecision(1) << iter->beta()
            << " ,orbit angle = " << iter->orb_angle() << ") " << iter->epoch().str_ymdhms();
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, string("gpvtflt:  ") +  os.str());
        return false;
    }
    else
        return true;
}

void great::t_gpvtflt::_syncAmb()
{
    if (_isBase)
        _udsdAmb();
    else
    {
        _udAmb();
    }
    return;
}

void great::t_gpvtflt::_predictCrd()
{
    double crdInit = _sig_init_crd;

    int i = 0;

    i = _param.getParam(_site, par_type::CRD_X, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(_vBanc(1));
            _Qx(i + 1, i + 1) = crdInit * crdInit;
        }
        else
        {
            if (_pos_kin)
                _param[i].value(_vBanc(1));
            if (_cntrep == 1 && _success)
                _Qx(i + 1, i + 1) += _crdStoModel->getQ() * _crdStoModel->getQ();
        }
    }

    i = _param.getParam(_site, par_type::CRD_Y, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(_vBanc(2));
            _Qx(i + 1, i + 1) = crdInit * crdInit;
        }
        else
        {
            if (_pos_kin)
                _param[i].value(_vBanc(2));
            if (_cntrep == 1 && _success)
                _Qx(i + 1, i + 1) += _crdStoModel->getQ() * _crdStoModel->getQ();
        }
    }

    i = _param.getParam(_site, par_type::CRD_Z, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(_vBanc(3));
            _Qx(i + 1, i + 1) = crdInit * crdInit;
        }
        else
        {
            if (_pos_kin)
                _param[i].value(_vBanc(3));
            if (_cntrep == 1 && _success)
                _Qx(i + 1, i + 1) += _crdStoModel->getQ() * _crdStoModel->getQ();
        }
    }

    return;
}

void great::t_gpvtflt::_predictClk()
{
    int i = 0;

    i = _param.getParam(_site, par_type::CLK, "");
    if (i >= 0)
    {
        _param[i].value(_vBanc(4));
        for (unsigned int jj = 1; jj <= _param.parNumber(); jj++)
            _Qx(i + 1, jj) = 0.0;
        _Qx(i + 1, i + 1) = _clkStoModel->getQ() * _clkStoModel->getQ();
    }
    return;
}

void great::t_gpvtflt::_predictBias()
{
    int i = 0;

    i = _param.getParam(_site, par_type::GLO_ISB, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = _sig_init_glo * _sig_init_glo;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _gloStoModel->getQ();
        }
    }

    i = _param.getParam(_site, par_type::GLO_IFCB, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = _sig_init_glo * _sig_init_glo;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _gloStoModel->getQ();
        }
    }

    i = _param.getParam(_site, par_type::GAL_ISB, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = _sig_init_gal * _sig_init_gal;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _galStoModel->getQ();
        }
    }

    i = _param.getParam(_site, par_type::BDS_ISB, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = _sig_init_bds * _sig_init_bds;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _bdsStoModel->getQ();
        }
    }

    i = _param.getParam(_site, par_type::QZS_ISB, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = _sig_init_qzs * _sig_init_qzs;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _qzsStoModel->getQ();
        }
    }

    i = _param.getParam(_site, par_type::IFB_GPS, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = 3000 * 3000;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _gpsStoModel->getQ();
        }
    }
    i = _param.getParam(_site, par_type::IFB_GAL, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = 3000 * 3000;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _galStoModel->getQ();
        }
    }
    i = _param.getParam(_site, par_type::IFB_GAL_2, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = 3000 * 3000;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _galStoModel->getQ();
        }
    }
    i = _param.getParam(_site, par_type::IFB_GAL_3, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = 3000 * 3000;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _galStoModel->getQ();
        }
    }
    i = _param.getParam(_site, par_type::IFB_BDS, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = 3000 * 3000;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _bdsStoModel->getQ();
        }
    }
    i = _param.getParam(_site, par_type::IFB_BDS_2, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = 3000 * 3000;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _bdsStoModel->getQ();
        }
    }
    i = _param.getParam(_site, par_type::IFB_BDS_3, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _param[i].value(0.0);
            _Qx(i + 1, i + 1) = 3000 * 3000;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _bdsStoModel->getQ();
        }
    }
    i = _param.getParam(_site, par_type::IFB_QZS, "");
    if (i >= 0)
    {
        if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
        {
            _Qx(i + 1, i + 1) = 3000 * 3000;
        }
        else
        {
            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _qzsStoModel->getQ();
        }
    }
    return;
}

void great::t_gpvtflt::_predictIono(const double& bl, const t_gtime& runEpoch)
{
    int i = 0;

    // predict ionosphere delay
    if (_iono_est)
    {
        vector<t_gsatdata>::iterator it;
        for (it = _data.begin(); it != _data.end(); ++it)
        {
            i = _param.getParam(_site, par_type::VION, it->sat());
            if (i >= 0)
            {
                if (_gion)
                { // Introduce new apriory value from IONEX if available
                    t_gtriple site_xyz(0.0, 0.0, 0.0);
                    t_gtriple site_ell(0.0, 0.0, 0.0);
                    t_gtriple ipp_ell(0.0, 0.0, 0.0);

                    _param.getCrdParam(_site, site_xyz);
                    if (site_xyz.zero())
                        site_xyz.set(_vBanc);
                    xyz2ell(site_xyz, site_ell, false);
                    ell2ipp(*it, site_ell, ipp_ell);

                    // use iono-free combination instead ! //
                    double ionomodel = 1.0; 

                    _param[i].apriori(ionomodel);
                }

                if (_cntrep == 1 &&
                    !double_eq(_Qx(i + 1, i + 1), _sig_init_vion * _sig_init_vion))
                {
                    _Qx(i + 1, i + 1) += _ionStoModel->getQ(); // *_ionStoModel->getQ();
                }
            }

            i = _param.getParam(_site, par_type::SION, it->sat());
            if (i >= 0)
            {
                double var = _sig_init_vion * _sig_init_vion;
                if (_isBase && double_eq(_Qx(i + 1, i + 1), var))
                {
                    var *= SQR(bl / (1e4));
                    _Qx(i + 1, i + 1) = var;
                    _param[i].value(1e-6);
                }
                _param[i].value(0.0);
                _Qx(i + 1, i + 1) = var;
                if (_cntrep == 1 && !double_eq(_Qx(i + 1, i + 1), var))
                {

                    if (_isBase)
                    {
                        _Qx(i + 1, i + 1) += SQR(bl / (1e4) * cos(it->ele())) * _ionStoModel->getQ();
                    }
                    else
                    {
                        _Qx(i + 1, i + 1) += _ionStoModel->getQ();
                    }
                }
            }
        }
    }

    return;
}

void great::t_gpvtflt::_predictTropo()
{
    int i = 0;
    i = _param.getParam(_site, par_type::GRD_N, "");
    if (i >= 0)
    {
        if (_cntrep == 1 && _initialized)
            _Qx(i + 1, i + 1) += _grdStoModel->getQ();
        if (_smooth)
            _Noise(i + 1, i + 1) = _grdStoModel->getQ();
    }

    i = _param.getParam(_site, par_type::GRD_E, "");
    if (i >= 0)
    {
        if (_cntrep == 1 && _initialized)
            _Qx(i + 1, i + 1) += _grdStoModel->getQ();
        if (_smooth)
            _Noise(i + 1, i + 1) = _grdStoModel->getQ();
    }

    if (_tropo_est)
    {
        double ztdInit = _sig_init_ztd;
        for (int j = 0; j < 1 + (_isBase ? 1 : 0); j++)
        {
            auto tmpsite = _site;
            auto tmpgrec = _grec;
            auto tmpgmodel = _gModel;

            if (j)
            {
                tmpsite = _site_base;
                tmpgrec = _gallobj->obj(_site_base);
                tmpgmodel = _gModel_base;
            }
            i = _param.getParam(tmpsite, par_type::TRP, "");
            if (i >= 0)
            {
                t_gtriple Ell, XYZ;

                if (_param.getCrdParam(tmpsite, XYZ) > 0)
                {
                }
                else
                {
                    XYZ = tmpgrec->crd_arp(_epoch);
                }
                xyz2ell(XYZ, Ell, false);
                if (tmpgmodel->tropoModel() != 0)
                {
                    _param[i].apriori(_sig_init_ztd);
                }

                if (!_initialized || _Qx(i + 1, i + 1) == 0.0)
                {
                    if (_valid_ztd_xml)
                    {
                        _param[i].value(_aprox_ztd_xml - tmpgmodel->tropoModel()->getZHD(Ell, _epoch));
                    }
                    else
                    {
                        if (tmpgmodel->tropoModel() != 0)
                        {
                            _param[i].value(tmpgmodel->tropoModel()->getZWD(Ell, _epoch));
                        }
                    }
                    _Qx(i + 1, i + 1) = ztdInit * ztdInit;
                }
                else
                {
                    if (_cntrep == 1)
                        _Qx(i + 1, i + 1) += _trpStoModel->getQ();
                }
            }
        }
    }
    return;
}

void great::t_gpvtflt::_predictAmb()
{
    // ambiguity randomwalk
    for (unsigned int i = 0; i < _param.parNumber(); i++)
    {
        if (_param[i].parType == par_type::AMB_IF ||
            _param[i].parType == par_type::AMB_L1 ||
            _param[i].parType == par_type::AMB_L2 ||
            _param[i].parType == par_type::AMB_L3 ||
            _param[i].parType == par_type::AMB_L4 ||
            _param[i].parType == par_type::AMB_L5)
        {

            if (_cntrep == 1)
                _Qx(i + 1, i + 1) += _ambStoModel->getQ();
            if (_smooth)
                _Noise(i + 1, i + 1) = _ambStoModel->getQ();
            _param[i].stime = _param[i].end = _epoch;
        }
    }
    return;
}

unsigned int great::t_gpvtflt::_cmp_equ(t_gfltEquationMatrix &equ)
{
    vector<t_gsatdata>::iterator it = _data.begin();
    for (it = _data.begin(); it != _data.end();)
    {
        if (!_base_model->cmb_equ(_epoch, _param, *it, equ))
        {
            it = _data.erase(it);
            continue;
        }
        if (_observ == OBSCOMBIN::IONO_FREE)
            _combineMW(*it);
        ++it;
    }

    return equ.num_equ();
}

void great::t_gpvtflt::_posterioriTest(const Matrix& A, const SymmetricMatrix& P, const ColumnVector& l,
    const ColumnVector& dx, const SymmetricMatrix& Q, ColumnVector& v_norm, double& vtpv)
{;
    ColumnVector v_orig, v_test;
    // post-fit residuals
    v_orig = l - A * dx;

    // normalized post-fit residuals
    Matrix Qv = A * _Qx * A.t() + P.i();
    v_norm.ReSize(v_orig.Nrows());
    for (int i = 1; i <= v_norm.Nrows(); i++)
    {
        v_norm(i) = sqrt(1 / Qv(i, i)) * v_orig(i);
    }

    int freedom = A.Nrows() - A.Ncols();
    if (freedom < 1)
    {
        if (_spdlog)
            SPDLOG_LOGGER_INFO(_spdlog, "No redundant observations!");
        freedom = 1;
    }

    ColumnVector vtPv = v_orig.t() * P * v_orig;
    _sig_unit = vtPv(1) / freedom;
    vtpv = vtPv(1);
    return;
}

