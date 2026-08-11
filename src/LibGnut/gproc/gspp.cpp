
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include "gproc/gspp.h"
#include "gmodels/gsppmodel.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace spdlog;
namespace gnut
{
    t_gspp::t_gspp(std::string mark, t_gsetbase* set) :
        _grec(nullobj),
        _gallobj(0),
        _gallbias(0),
        _weight(OBSWEIGHT::SINEL),
        _observ(OBSCOMBIN::IONO_FREE),
        _valid_crd_xml(false),
        _valid_ztd_xml(false),
        _site(mark),
        _set(set),
        _gobs(0),
        _gnav(0),
        _gmet(0),
        _gion(0),
        _allprod(0),
        _phase(false),
        _doppler(false),
        _gnss(GSYS::GNS),
        _initialized(false),
        _use_ecl(false)
    {
        if (nullptr == set)
        {
            GREAT_CRITICAL("your set pointer is nullptr !");
            throw std::logic_error("");
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

    t_gspp::t_gspp(std::string mark, t_gsetbase* set, std::string mode) :
        _grec(nullobj),
        _gallobj(0),
        _gallbias(0),
        _weight(OBSWEIGHT::SINEL),
        _observ(OBSCOMBIN::IONO_FREE),
        _valid_crd_xml(false),
        _valid_ztd_xml(false),
        _site(mark),
        _set(set),
        _gobs(0),
        _gnav(0),
        _gmet(0),
        _gion(0),
        _allprod(0),
        _phase(false),
        _doppler(false),
        _gnss(GSYS::GNS),
        _initialized(false),
        _use_ecl(false)
    {
        // set the setting pointer
        if (nullptr == set)
        {
            GREAT_CRITICAL("your set pointer is nullptr !");
            throw std::logic_error("");
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

    t_gspp::~t_gspp()
    {
        if (_gModel)
        {
            delete _gModel;
        }
    }

    void t_gspp::setDAT(t_gallobs* gobs, t_gallnav* gnav)
    {
        _gobs = gobs;
        _gnav = gnav;
    }

    void t_gspp::setOUT(t_gallprod* products)
    {
        this->_setOut();
        _allprod = products;
    }

    void t_gspp::setOBJ(t_gallobj* gallobj)
    {
        _gallobj = gallobj;

        if (_gallobj)
        {
            _grec = _gallobj->obj(_site);
            dynamic_cast<t_gsppmodel*>(_gModel)->setrec(_grec);
        }
    }

    void t_gspp::setDCB(t_gallbias* bias)
    {
        _gallbias = bias;
        _gModel->setBIAS(bias);
    }

    void t_gspp::setFCB(t_gallbias* bias)
    {
        _gallfcb = bias;
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
        _tropo_est = dynamic_cast<t_gsetproc*>(_set)->tropo();
        _iono_est = dynamic_cast<t_gsetproc*>(_set)->iono();
        _tropo_grad = dynamic_cast<t_gsetproc*>(_set)->tropo_grad();
        _tropo_slant = dynamic_cast<t_gsetproc*>(_set)->tropo_slant();
        _ztd_mf = dynamic_cast<t_gsetproc*>(_set)->tropo_mf();
        _grd_mf = dynamic_cast<t_gsetproc*>(_set)->grad_mf();
        _crd_est = dynamic_cast<t_gsetproc*>(_set)->crd_est();
        _sampling = dynamic_cast<t_gsetgen*>(_set)->sampling();
        _scale = dynamic_cast<t_gsetgen*>(_set)->sampling_scalefc();
        _minElev = dynamic_cast<t_gsetproc*>(_set)->minimum_elev();
        _sig_init_crd = dynamic_cast<t_gsetproc*>(_set)->sig_init_crd();
        _sig_init_vel = dynamic_cast<t_gsetproc*>(_set)->sig_init_vel();
        _sig_init_ztd = dynamic_cast<t_gsetproc*>(_set)->sig_init_ztd();
        _sig_init_vion = dynamic_cast<t_gsetproc*>(_set)->sig_init_vion();
        _sig_init_grd = dynamic_cast<t_gsetproc*>(_set)->sig_init_grd();
        _sig_init_glo = dynamic_cast<t_gsetproc*>(_set)->sig_init_glo();
        _sig_init_gal = dynamic_cast<t_gsetproc*>(_set)->sig_init_gal();
        _sig_init_bds = dynamic_cast<t_gsetproc*>(_set)->sig_init_bds();
        _sig_init_qzs = dynamic_cast<t_gsetproc*>(_set)->sig_init_qzs();

        _sigCodeGPS = dynamic_cast<t_gsetgnss*>(_set)->sigma_C(GSYS::GPS);
        _sigCodeGLO = dynamic_cast<t_gsetgnss*>(_set)->sigma_C(GSYS::GLO);
        _sigCodeGAL = dynamic_cast<t_gsetgnss*>(_set)->sigma_C(GSYS::GAL);
        _sigCodeBDS = dynamic_cast<t_gsetgnss*>(_set)->sigma_C(GSYS::BDS);
        _sigCodeQZS = dynamic_cast<t_gsetgnss*>(_set)->sigma_C(GSYS::QZS);
        _sigPhaseGPS = dynamic_cast<t_gsetgnss*>(_set)->sigma_L(GSYS::GPS);
        _sigPhaseGLO = dynamic_cast<t_gsetgnss*>(_set)->sigma_L(GSYS::GLO);
        _sigPhaseGAL = dynamic_cast<t_gsetgnss*>(_set)->sigma_L(GSYS::GAL);
        _sigPhaseBDS = dynamic_cast<t_gsetgnss*>(_set)->sigma_L(GSYS::BDS);
        _sigPhaseQZS = dynamic_cast<t_gsetgnss*>(_set)->sigma_L(GSYS::QZS);
        _sigDopplerGPS = dynamic_cast<t_gsetgnss*>(_set)->sigma_D(GSYS::GPS);
        _sigDopplerGLO = dynamic_cast<t_gsetgnss*>(_set)->sigma_D(GSYS::GLO);
        _sigDopplerGAL = dynamic_cast<t_gsetgnss*>(_set)->sigma_D(GSYS::GAL);
        _sigDopplerBDS = dynamic_cast<t_gsetgnss*>(_set)->sigma_D(GSYS::BDS);
        _sigDopplerQZS = dynamic_cast<t_gsetgnss*>(_set)->sigma_D(GSYS::QZS);

        _pos_kin = dynamic_cast<t_gsetproc*>(_set)->pos_kin();
        _weight = dynamic_cast<t_gsetproc*>(_set)->weighting();
        _observ = dynamic_cast<t_gsetproc*>(_set)->obs_combin();

        if (_sampling == 0)
        {
            _sampling = dynamic_cast<t_gsetgen*>(_set)->sampling_default();
        }

        return 1;
    }

    void t_gspp::_setOut()
    {
        std::string tmp(dynamic_cast<t_gsetout*>(_set)->outputs("res"));
        if (!tmp.empty())
        {
            substitute(tmp, "$(rec)", _site, false);
        }
    }
} // namespace gnut
