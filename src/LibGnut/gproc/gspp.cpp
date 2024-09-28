
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include "gproc/gspp.h"
#include "gmodels/gsppmodel.h"
#include "spdlog/spdlog.h"
#include "gio/grtlog.h"

using namespace spdlog;
namespace gnut
{
    t_gspp::t_gspp(string mark, t_gsetbase *set)
        : _grec(nullobj),
          _gallobj(0),
          _gallbias(0),
          _weight(OBSWEIGHT::SINEL),
          _observ(OBSCOMBIN::IONO_FREE),
          _valid_crd_xml(false),
          _valid_ztd_xml(false),
          _site(mark),
          _set(set),
          _res(0),
          _gobs(0),
          _gnav(0),
          _gmet(0),
          _gion(0),
          _allprod(0),
          _phase(false),
          _doppler(false),
          _gnss(GNS),
          _initialized(false),
          _use_ecl(false)
    {
        if (nullptr == set)
        {
            spdlog::critical("your set pointer is nullptr !");
            throw logic_error("");
        }
        else
        {
            _set = set;
        }

        _crd_begStat = _crd_endStat = FIRST_TIME;
        _ztd_begStat = _ztd_endStat = FIRST_TIME;

        _valid_ztd_xml = false;

        _get_settings();

        _gModel = new t_gsppmodel(_site, _set);
    }

    t_gspp::t_gspp(string mark, t_gsetbase *set, t_spdlog spdlog, string mode)
        : _grec(nullobj),
          _gallobj(0),
          _gallbias(0),
          _weight(OBSWEIGHT::SINEL),
          _observ(OBSCOMBIN::IONO_FREE),
          _valid_crd_xml(false),
          _valid_ztd_xml(false),
          _site(mark),
          _set(set),
          _res(0),
          _gobs(0),
          _gnav(0),
          _gmet(0),
          _gion(0),
          _allprod(0),
          _phase(false),
          _doppler(false),
          _gnss(GNS),
          _initialized(false),
          _use_ecl(false)
    {
        // set the setting pointer
        _setLog(mode);       // set log  (before using it later on!)
        if (nullptr == set)
        {
            spdlog::critical("your set pointer is nullptr !");
            throw logic_error("");
        }
        else
        {
            _set = set;
        }
        _crd_begStat = _crd_endStat = FIRST_TIME;
        _ztd_begStat = _ztd_endStat = FIRST_TIME;
        _valid_ztd_xml = false;
        _get_settings();
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _gspdlog = spdlog;
        }
        _gModel = new t_gsppmodel(_gspdlog, _site, _set);
    }

    t_gspp::~t_gspp()
    {
        if (_gModel)
            delete _gModel;
    }

    void t_gspp::setDAT(t_gallobs *gobs, t_gallnav *gnav)
    {
        _gobs = gobs;
        _gnav = gnav;

    }

    void t_gspp::setOUT(t_gallprod *products)
    {
        this->_setOut();
        _allprod = products;
    }

    void t_gspp::setOBJ(t_gallobj *gallobj)
    {
        _gallobj = gallobj;

        if (_gallobj)
        {
            _grec = _gallobj->obj(_site);
            dynamic_cast<t_gsppmodel *>(_gModel)->setrec(_grec);
        }
    }

    void t_gspp::setDCB(t_gallbias *bias)
    {
        _gallbias = bias;
        _gModel->setBIAS(bias);
    }

    void t_gspp::setFCB(t_gallbias *bias)
    {
        _gallfcb = bias;
    }

    void t_gspp::spdlog(t_spdlog spdlog)
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
    }

    void t_gspp::tropo(bool tropo)
    {
        this->_tropo_est = tropo;
    }

    void t_gspp::tropo_slant(bool slant)
    {
        this->_tropo_slant = slant;
    }

    void t_gspp::phase(bool phase)
    {
        this->_phase = phase;
    }

    void t_gspp::setgnss(GSYS sys)
    {
        this->_success = true;
        this->_gnss = sys;
    }

    int t_gspp::_get_settings()
    {
        _tropo_est = dynamic_cast<t_gsetproc *>(_set)->tropo();
        _iono_est = dynamic_cast<t_gsetproc *>(_set)->iono();
        _tropo_grad = dynamic_cast<t_gsetproc *>(_set)->tropo_grad();
        _tropo_slant = dynamic_cast<t_gsetproc *>(_set)->tropo_slant();
        _ztd_mf = dynamic_cast<t_gsetproc *>(_set)->tropo_mf();
        _grd_mf = dynamic_cast<t_gsetproc *>(_set)->grad_mf();
        _crd_est = dynamic_cast<t_gsetproc *>(_set)->crd_est();
        _sampling = dynamic_cast<t_gsetgen *>(_set)->sampling();
        _scale = dynamic_cast<t_gsetgen *>(_set)->sampling_scalefc();
        _minElev = dynamic_cast<t_gsetproc *>(_set)->minimum_elev();
        _sig_init_crd = dynamic_cast<t_gsetproc *>(_set)->sig_init_crd();
        _sig_init_vel = dynamic_cast<t_gsetproc *>(_set)->sig_init_vel();
        _sig_init_ztd = dynamic_cast<t_gsetproc *>(_set)->sig_init_ztd();
        _sig_init_vion = dynamic_cast<t_gsetproc *>(_set)->sig_init_vion();
        _sig_init_grd = dynamic_cast<t_gsetproc *>(_set)->sig_init_grd();
        _sig_init_glo = dynamic_cast<t_gsetproc *>(_set)->sig_init_glo();
        _sig_init_gal = dynamic_cast<t_gsetproc *>(_set)->sig_init_gal();
        _sig_init_bds = dynamic_cast<t_gsetproc *>(_set)->sig_init_bds();
        _sig_init_qzs = dynamic_cast<t_gsetproc *>(_set)->sig_init_qzs();

        _sigCodeGPS = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(GPS);
        _sigCodeGLO = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(GLO);
        _sigCodeGAL = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(GAL);
        _sigCodeBDS = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(BDS);
        _sigCodeQZS = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(QZS);
        _sigPhaseGPS = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(GPS);
        _sigPhaseGLO = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(GLO);
        _sigPhaseGAL = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(GAL);
        _sigPhaseBDS = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(BDS);
        _sigPhaseQZS = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(QZS);
        _sigDopplerGPS = dynamic_cast<t_gsetgnss *>(_set)->sigma_D(GPS);
        _sigDopplerGLO = dynamic_cast<t_gsetgnss *>(_set)->sigma_D(GLO);
        _sigDopplerGAL = dynamic_cast<t_gsetgnss *>(_set)->sigma_D(GAL);
        _sigDopplerBDS = dynamic_cast<t_gsetgnss *>(_set)->sigma_D(BDS);
        _sigDopplerQZS = dynamic_cast<t_gsetgnss *>(_set)->sigma_D(QZS);

        _pos_kin = dynamic_cast<t_gsetproc *>(_set)->pos_kin();
        _weight = dynamic_cast<t_gsetproc *>(_set)->weighting();
        _observ = dynamic_cast<t_gsetproc *>(_set)->obs_combin();

        if (_sampling == 0)
            _sampling = dynamic_cast<t_gsetgen *>(_set)->sampling_default();

        return 1;
    }

    void t_gspp::_setOut()
    {
        string tmp(dynamic_cast<t_gsetout *>(_set)->outputs("res"));
        if (!tmp.empty())
        {
            substitute(tmp, "$(rec)", _site, false);
            _greslog.set_log("BASIC", spdlog::level::info, tmp);
            _res = _greslog.spdlog();
        }
    }

    void t_gspp::_setLog(string mode)
    {
        string tmp(dynamic_cast<t_gsetout *>(_set)->outputs("ppp"));
        if (!tmp.empty())
        {
            tmp = tmp.substr(7, tmp.size() - 7);
            substitute(tmp, "$(rec)", _site, false);

            t_gtime beg = dynamic_cast<t_gsetgen *>(_set)->beg();
            if (beg == FIRST_TIME)
            {
                beg = t_gtime::current_time(t_gtime::GPS);
            } // real-time model
            
            substitute(tmp, "$(rec)", _site, false);
            substitute(tmp, "$(doy)", int2str(beg.doy()), false);
            substitute(tmp, "$(date)", int2str(beg.year()) + int2str(beg.doy()), false); // add for date
            if (mode != "")
                tmp = tmp + "-" + mode;

            auto log_type = dynamic_cast<t_gsetout*>(_set)->log_type();
            auto log_level = dynamic_cast<t_gsetout*>(_set)->log_level();
            auto log_name = dynamic_cast<t_gsetout*>(_set)->log_name();
            _grtlog.set_log(log_type, log_level, log_name);
            _spdlog = _grtlog.spdlog();
        }
    }

} // namespace
