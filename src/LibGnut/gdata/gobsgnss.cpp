/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "gdata/gobsgnss.h"
#include "gutils/gconst.h"
#include "gutils/gtypeconv.h"
#include "gall/gallbias.h"

using namespace std;

namespace gnut
{

    t_gobsgnss::t_gobsgnss()
        : t_gdata(),
          _apr_ele(-1),
          _channel(DEF_CHANNEL),
          _rtcm_end(2),
          _health(true),
          _dcb_correct_mark(false),
          _bds_code_bias_mark(false),
          _range_smooth_mark(false)
    {
        id_type(t_gdata::OBSGNSS);
        id_group(t_gdata::GRP_OBSERV);
    }

    t_gobsgnss::t_gobsgnss(t_spdlog spdlog)
        : t_gdata(spdlog),
          _apr_ele(-1),
          _channel(DEF_CHANNEL),
          _rtcm_end(2),
          _health(true),
          _dcb_correct_mark(false),
          _bds_code_bias_mark(false),
          _range_smooth_mark(false)
    {
        id_type(t_gdata::OBSGNSS);
        id_group(t_gdata::GRP_OBSERV);
    }
    t_gobsgnss::t_gobsgnss(t_spdlog spdlog, const string &sat)
        : t_gdata(spdlog),
          _apr_ele(-1),
          _channel(DEF_CHANNEL),
          _rtcm_end(2),
          _health(true),
          _dcb_correct_mark(false),
          _bds_code_bias_mark(false),
          _range_smooth_mark(false),
          _satid(sat),
          _gsys(t_gsys::char2gsys(sat[0]))
    {
        id_type(t_gdata::OBSGNSS);
        id_group(t_gdata::GRP_OBSERV);
    }

    t_gobsgnss::t_gobsgnss(t_spdlog spdlog,
                           const string &site,
                           const string &sat,
                           const t_gtime &t)
        : t_gdata(spdlog),
          _staid(site),
          _satid(sat),
          _gsys(t_gsys::char2gsys(_satid[0])),
          _epoch(t),
          _apr_ele(-1),
          _channel(DEF_CHANNEL),
          _rtcm_end(2),
          _health(true),
          _dcb_correct_mark(false),
          _bds_code_bias_mark(false),
          _range_smooth_mark(false)
    {
        id_type(t_gdata::OBSGNSS);
        id_group(t_gdata::GRP_OBSERV);
    }

    t_gobsgnss::~t_gobsgnss()
    {
    }

    void t_gobsgnss::addobs(const GOBS &obs, const double &d)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();
        _gobs[obs] = d;
        _gmutex.unlock();
        return;
    }

    void t_gobsgnss::addlli(const GOBS &obs, const int &i)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();
        if (i != 0)
            _glli[obs] = i;
        _gmutex.unlock();
        return;
    }

    void t_gobsgnss::addslip(const GOBS &obs, const int &i)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();
        _gslip[obs] = i;
        _gmutex.unlock();
        return;
    }

    void t_gobsgnss::addoutliers(const GOBS &obs, const int &i)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();
        if (i != 0)
            _goutlier[obs] = i;
        _gmutex.unlock();
        return;
    }

    void t_gobsgnss::setrangestate(const string &name, const bool &b)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();
        if (name == "bds_code_bias")
            _bds_code_bias_mark = b;
        else if (name == "smooth_range")
            _range_smooth_mark = b;
        _gmutex.unlock();
        return;
    }

    bool t_gobsgnss::getrangestate(const string &name)
    {
        _gmutex.lock();
        bool tmp = false;
        if (name == "bds_code_bias")
            tmp = _bds_code_bias_mark;
        else if (name == "smooth_range")
            tmp = _range_smooth_mark;
        _gmutex.unlock();
        return tmp;
    }

    void t_gobsgnss::setobsLevelFlag(const GOBS &gobs, const int &flag)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();
        _gLevel[gobs] = flag;
        _gmutex.unlock();
        return;
    }

    vector<GOBS> t_gobsgnss::obs() const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        vector<GOBS> tmp;
        map<GOBS, double>::const_iterator it = _gobs.begin();
        while (it != _gobs.end())
        {
            tmp.push_back(it->first);
            it++;
        }

        _gmutex.unlock();
        return tmp;
    }

    set<GOBS> t_gobsgnss::obs_phase(const int &band) const
    {
        _gmutex.lock();

        set<GOBS> tmp;
        map<GOBS, double>::const_iterator it = _gobs.begin();
        while (it != _gobs.end())
        {
            GOBS gobs = it->first;
            int b = gobs2band(gobs);
            if (band == b && gobs_phase(gobs))
                tmp.insert(gobs);
            it++;
        }

        _gmutex.unlock();
        return tmp;
    }

    double t_gobsgnss::getobs(const GOBS &obs) const
    {
        _gmutex.lock();

        double tmp = NULL_GOBS;
        if (_gobs.find(obs) != _gobs.end())
            tmp = _gobs.at(obs);

        _gmutex.unlock();
        return tmp;
    }

    void t_gobsgnss::resetobs(const GOBS &obs, const double &v)
    {
        _gmutex.lock();

        if (_gobs.find(obs) != _gobs.end())
            _gobs[obs] = v;

        _gmutex.unlock();
    }

    void t_gobsgnss::eraseobs(const GOBS &obs)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gobs.find(obs) != _gobs.end())
            _gobs.erase(obs);
        if (_glli.find(obs) != _glli.end())
            _glli.erase(obs);
        if (_goutlier.find(obs) != _goutlier.end())
            _goutlier.erase(obs);
        if (_gslip.find(obs) != _gslip.end())
            _gslip.erase(obs);
        if (_gLevel.find(obs) != _gLevel.end())
            _gLevel.erase(obs);

        _gmutex.unlock();
    }

    void t_gobsgnss::eraseband(const GOBSBAND &b)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        //_gmutex.lock();

        vector<GOBS> obs_vec = this->obs();
        for (auto it : obs_vec) // delete
        {
            if (str2gobsband(gobs2str(it)) == b)
                this->eraseobs(it);
        }

        //_gmutex.unlock();
    }

    bool t_gobsgnss::apply_bias(t_gallbias *allbias)
    {
         if (nullptr == allbias)
         {
            if (_spdlog)
                SPDLOG_LOGGER_WARN(_spdlog, "{} : NO DCB FILE", this->site());
            return true;
         }

        if (allbias->get_used_ac() == "SGG_A")
            return apply_code_phase_bias(allbias); 
        else
            return apply_dcb(allbias);
    }

    bool t_gobsgnss::apply_code_phase_bias(t_gallbias *allbias)
    {
        double bias1 = 0.0;
        double bias2 = 0.0;
        double c1 = 0.0;
        double c2 = 0.0;
        double biasif = 0.0;
        GOBSBAND b1, b2;
        if (this->_dcb_correct_mark)
        {
            return true;
        }

        string gsat = this->sat();
        t_gtime gepo = this->epoch();
        GSYS gsys = this->gsys();

        for (GOBS obs_type : this->obs())
        {

            t_gobs gobs_type(obs_type);
            gobs_type.gobs2to3(gsys);

            if (t_gobs(obs_type).is_code())
            {
            }
            else if (t_gobs(obs_type).is_phase())
            {
                if (obs_type == L1W)
                {
                    bias1 = allbias->get(gepo, gsat, gobs_type.gobs(), gobs_type.gobs(), "WHU_PHASE");
                    b1 = t_gobs(obs_type).band();
                }
                else if (obs_type == L2W)
                {
                    bias2 = allbias->get(gepo, gsat, gobs_type.gobs(), gobs_type.gobs(), "WHU_PHASE");
                    b2 = t_gobs(obs_type).band();
                }
            }
            else
            {
                continue;
            }

            // apply dcb to obsdata
            double obs_value = this->getobs(gobs_type.gobs());
            if (obs_type == L2W)
            {
                coef_ionofree(b1, c1, b2, c2);
                biasif = c1 * bias1 + c2 * bias2;
                std::cout << gsat << " " << t_gobs(obs_type).gobs() << " " << biasif << endl;
            }
            this->resetobs(gobs_type.gobs(), obs_value);
        }
        this->_dcb_correct_mark = true;
        return true;
    }

    bool t_gobsgnss::apply_dcb(t_gallbias* allbias)
    {
        if (allbias == NULL)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "{} : NO DCB FILE", this->site());
            return true;
        }

        //  skip already correct dcb obsdata
        if (this->_dcb_correct_mark)
        {
            return true;
        }

        string gsat = this->sat();
        t_gtime gepo = this->epoch();
        GSYS gsys = this->gsys();

        // two band is bound to precise clock
        auto band1 = GNSS_BAND_PRIORITY.at(gsys)[1];
        auto band2 = GNSS_BAND_PRIORITY.at(gsys)[2];

        if (gsys == GSYS::BDS)
        {
            band2 = GNSS_BAND_PRIORITY.at(gsys)[3];
        }

        // caculate interdcb band1-band2
        double alfa12 = 0.0;
        // double alfa13 = 0.0;
        double beta12 = 0.0;
        // double beta13 = 0.0;
        this->coef_ionofree(band1, alfa12, band2, beta12);
        double inter_dcb_12 = this->interFreqDcb(*allbias, band1, band2);
        double inter_dcb_1x = 0.0;
        double inter_dcb_2x = 0.0;
        for (GOBS obs_type : this->obs())
        {

            // skip not code obs
            if (!t_gobs(obs_type).is_code())
            {
                continue;
            }
            double obs_value = this->getobs(obs_type);

            if (double_eq(obs_value, 0.0))
                continue;

            // apply intra dcb correct [GPS]
            t_gobs gobs_type(obs_type);
            gobs_type.gobs2to3(gsys);

            double dcb_value = 0.0;
            if (gsys == GSYS::GPS)
            {
                switch (gobs_type.gobs())
                {
                case C1C:
                    dcb_value = allbias->get(gepo, gsat, C1W, C1C);
                    break;
                case C1P:
                    dcb_value = 0.0;
                case C1Y:
                    dcb_value = 0.0;
                case C1W:
                    dcb_value = 0.0;
                    break;
                case C2C:
                    dcb_value = allbias->get(gepo, gsat, C2W, C2C);
                    break;
                case C2L:
                    dcb_value = allbias->get(gepo, gsat, C2W, C2L);
                    break;
                case C2X:
                    dcb_value = allbias->get(gepo, gsat, C2W, C2X);
                    break;
                case C2P:
                    dcb_value = 0.0;
                case C2Y:
                    dcb_value = 0.0;
                case C2W:
                    dcb_value = 0.0;
                    break;
                case C5Q:
                    dcb_value = allbias->get(gepo, gsat, C5X, C5Q);
                    break;
                case C5X:
                    dcb_value = 0.0;
                    break;
                default:
                    dcb_value = 0.0;
                    break;
                }
            }
            else if (gsys == GSYS::GLO)
            {
                switch (gobs_type.gobs())
                {
                case C1C:
                    dcb_value = allbias->get(gepo, gsat, C1P, C1C);
                    break;
                case C1P:
                    dcb_value = 0.0;
                    break;
                case C2C:
                    dcb_value = allbias->get(gepo, gsat, C2P, C2C);
                    break;
                case C2P:
                    dcb_value = 0.0;
                    break;
                default:
                    dcb_value = 0.0;
                    break;
                }
            }

            // apply inter dcb correct
            if (gobs_type.band() == band1)
            {
                dcb_value += -beta12 * inter_dcb_12;
            }
            else if (gobs_type.band() == band2)
            {
                dcb_value += +alfa12 * inter_dcb_12;
            }
            else
            {
                GOBSBAND band_x = gobs_type.band();
                inter_dcb_1x = this->interFreqDcb(*allbias, band1, band_x);
                inter_dcb_2x = this->interFreqDcb(*allbias, band2, band_x);
                dcb_value += alfa12 * inter_dcb_1x + beta12 * inter_dcb_2x;
            }

            // apply dcb to obsdata

            obs_value += dcb_value;
            this->resetobs(obs_type, obs_value);
        }
        this->_dcb_correct_mark = true;
        return true;
    }

    double t_gobsgnss::interFreqDcb(t_gallbias &allbias, const GOBSBAND &band1, const GOBSBAND &band2) const
    {

        auto gsys = this->gsys();
        auto gsat = this->sat();
        auto grec = this->site();
        auto gepo = this->epoch();

        t_gobs g1 = t_gobs(this->select_range(band1, true));
        t_gobs g2 = t_gobs(this->select_range(band2, true));

        g1.gobs2to3(this->gsys());
        g2.gobs2to3(this->gsys());

        double dcb_value = 0.0;

        GOBS obs1 = t_gsys::gobs_priority(gsys, g1.gobs());
        GOBS obs2 = t_gsys::gobs_priority(gsys, g2.gobs());

         auto default_band1 = GNSS_BAND_PRIORITY.at(gsys)[1];
         auto default_band2 = GNSS_BAND_PRIORITY.at(gsys)[2];
         auto default_band3 = GNSS_BAND_PRIORITY.at(gsys)[3];

        if (obs1 == GOBS::X && obs2 == GOBS::X)
        {
            return 0.0;
        }

        if (obs1 == GOBS::X)
        {
            obs1 = t_gsys::gobs_defaults(gsys, obs2, band1);
        }

        if (obs2 == GOBS::X)
        {
            obs2 = t_gsys::gobs_defaults(gsys, obs1, band2);
        }

        dcb_value = allbias.get(gepo, gsat, obs1, obs2);

        if (double_eq(dcb_value, 0.0))
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "interFreqDcb {}", gobs2str(obs1) + " " + gobs2str(obs2) + " " + " " + gsat + " " + " " + grec + " " + gepo.str_mjdsod("wrong type of GNSS sys, only support GPS and GAL."));

            return 0.0;
        }

        return dcb_value;
    }

    double t_gobsgnss::getobs(const string &obs) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        double tmp = NULL_GOBS;
        if (_gobs.find(str2gobs(obs)) != _gobs.end())
        {
            tmp = _gobs.at(str2gobs(obs));
        }

        _gmutex.unlock();
        return tmp;
    }

    int t_gobsgnss::getlli(const GOBS &obs) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        int tmp = 0;
        if (_glli.find(obs) != _glli.end())
            tmp = _glli.at(obs);

        _gmutex.unlock();
        return tmp;
    }

    int t_gobsgnss::getslip(const GOBS &obs) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        int tmp = 0;
        if (_gslip.find(obs) != _gslip.end())
            tmp = _gslip.at(obs);

        _gmutex.unlock();
        return tmp;
    }

    int t_gobsgnss::getoutliers(const GOBS &obs) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        int tmp = 0;
        if (_goutlier.find(obs) != _goutlier.end())
            tmp = _goutlier.at(obs);

        _gmutex.unlock();
        return tmp;
    }

    int t_gobsgnss::getobsLevelFlag(const GOBS &obs)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        int tmp = -1;
        if (_gLevel.find(obs) != _gLevel.end())
            tmp = _gLevel[obs];

        _gmutex.unlock();
        return tmp;
    }

    bool t_gobsgnss::obsWithCorr()
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        bool tmp = true;
        for (auto it : _gLevel)
        {
            if (it.second == 0)
            {
                tmp = false;
                break;
            }
        }
        _gmutex.unlock();
        return tmp;
    }

    int t_gobsgnss::getlli(const string &obs) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        int tmp = 0;
        if (_glli.find(str2gobs(obs)) != _glli.end())
            tmp = _glli.at(str2gobs(obs));

        _gmutex.unlock();
        return tmp;
    }

    void t_gobsgnss::addele(const double &d)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();
        _apr_ele = d;
        _gmutex.unlock();
        return;
    }

    const double &t_gobsgnss::getele() const
    {
        return _apr_ele;
    }

    void t_gobsgnss::channel(const int &ch)
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();
        _channel = ch;
        _gmutex.unlock();
        return;
    }

    const int &t_gobsgnss::channel() const
    {
        return _channel;
    }

    size_t t_gobsgnss::size() const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        size_t tmp = _gobs.size();

        _gmutex.unlock();
        return tmp;
    }

    double t_gobsgnss::frequency(const int &b) const
    {
        GOBSBAND gb = GOBSBAND(b);
        return this->frequency(gb);
    }

    double t_gobsgnss::frequency(const GOBSBAND &b) const
    {

        switch (_gsys)
        {

        case GPS:
            switch (b)
            {
            case BAND_1:
                return G01_F;
            case BAND_2:
                return G02_F;
            case BAND_5:
                return G05_F;
            default:
                return 0.0;
            }

        case GLO:
            switch (b)
            {
            case BAND_1:
                return R01_F(_channel);
            case BAND_2:
                return R02_F(_channel);
            case BAND_3:
                return R03_F_CDMA;
            case BAND_5:
                return R05_F_CDMA;
            default:
                return 0.0;
            }

        case GAL:
            switch (b)
            {
            case BAND_1:
                return E01_F;
            case BAND_5:
                return E05_F;
            case BAND_6:
                return E06_F;
            case BAND_7:
                return E07_F;
            case BAND_8:
                return E08_F;
            default:
                return 0.0;
            }

        case BDS:
            switch (b)
            {
            case BAND_2:
                return C02_F;
            case BAND_6:
                return C06_F;
            case BAND_7:
                return C07_F;
            case BAND_5:
                return C05_F; 
            case BAND_8:
                return C08_F;
            case BAND_9:
                return C09_F;
            case BAND_1:
                return C01_F;

            default:
                return 0.0;
            }

        case QZS:
            switch (b)
            {
            case BAND_1:
                return J01_F;
            case BAND_2:
                return J02_F;
            case BAND_5:
                return J05_F;
            case BAND_6:
                return J06_F;
            default:
                return 0.0;
            }

        case IRN:
            switch (b)
            {
            case BAND_5:
                return I05_F;
            default:
                return 0.0;
            }

        case SBS:
            switch (b)
            {
            case BAND_1:
                return S01_F;
            case BAND_5:
                return S05_F;
            default:
                return 0.0;
            }

        case GNS:
            return 0.0;
        default:
            return 0.0;
        }

        return 0.0;
    }

    double t_gobsgnss::wavelength(const GOBSBAND &b) const
    {
        double frq = this->frequency(b);
        if (frq != 0.0)
            return CLIGHT / frq;

        return 0.0;
    }

    double t_gobsgnss::wavelength_L3(const GOBSBAND &b1, const GOBSBAND &b2) const
    {
        double c1, c2;
        _coef_ionofree(b1, c1, b2, c2);

        double f1 = this->frequency(b1);
        double f2 = this->frequency(b2);

        if (!double_eq(f1, 0.0) && !double_eq(f2, 0.0))
        {
            double lamb_L3 = c1 * CLIGHT / f1 + c2 * CLIGHT / f2;
            return lamb_L3;
        }
        return 0.0;
    }

    double t_gobsgnss::wavelength_WL(const GOBSBAND &b1, const GOBSBAND &b2) const
    {
        double f1 = this->frequency(b1);
        double f2 = this->frequency(b2);

        if (!double_eq(f1, 0.0) && !double_eq(f2, 0.0))
        {
            return CLIGHT / (f1 - f2);
        }
        return 0.0;
    }

    double t_gobsgnss::wavelength_NL(const GOBSBAND &b1, const GOBSBAND &b2) const
    {
        double f1 = this->frequency(b1);
        double f2 = this->frequency(b2);

        if (!double_eq(f1, 0.0) && !double_eq(f2, 0.0))
        {
            double lamb_WL = CLIGHT / (f1 + f2);
            return lamb_WL;
        }
        return 0.0;
    }

    GSYS t_gobsgnss::gsys() const
    {
        return _gsys;
    }

    double t_gobsgnss::obs_C(const t_gobs &go) const
    {
        double tmp = this->_obs_range(go);
        return tmp;
    }

    double t_gobsgnss::obs_C(const GOBS &gobs) const
    {
        t_gobs obs(gobs);
        double tmp = this->_obs_range(obs);
        return tmp;
    }

    double t_gobsgnss::obs_C(const t_gband &gb) const
    {
        double tmp = this->_obs_range(gb);
        return tmp;
    }

    double t_gobsgnss::_obs_range(const t_gobs &go) const
    {
        // Glonass has 255 channel number
        if (!_valid_obs())
            return NULL_GOBS;

        // AUTO SELECTION
        if (go.attr() == ATTR)
        {
            GOBS gobs = _id_range(go.band());
            if (gobs == X)
                return NULL_GOBS;
            else
                return _gobs.at(gobs);
            return NULL_GOBS;
        }

        // FIXED SELECTION
        // here avoid a PROBLEM WITH the legacy TYPE_P (only when NULL_ATTR)
        // -----------------------------
        map<GOBS, double>::const_iterator it = _gobs.end();

        if (go.attr() == ATTR_NULL &&
            (go.type() == TYPE ||
             go.type() == TYPE_P))
        {
            t_gobs gobs(TYPE_P, go.band(), go.attr());
            it = _gobs.find(gobs.gobs());
        }

        if (it == _gobs.end() &&
            go.type() != TYPE_P)
        {
            t_gobs gobs(TYPE_C, go.band(), go.attr());
            it = _gobs.find(gobs.gobs());
        }

        if (it == _gobs.end())
            return NULL_GOBS;

        return it->second;
    }

    double t_gobsgnss::_obs_range(const t_gband &gb) const
    {
        t_gobs gobs(TYPE, gb.band(), gb.attr());

#ifdef DEBUG
        cout << _epoch.str_hms() << " sat: " << sat() << " band: " << gobsband2str(gb.band()) << " attr: " << gobsattr2str(gb.attr()) << endl;
#endif

        return _obs_range(gobs);
    }

    double t_gobsgnss::obs_L(const t_gobs &go) const
    {
        double tmp = this->_obs_phase(go.gband());
        return tmp;
    }

    double t_gobsgnss::obs_L(const GOBS &gobs) const
    {
        t_gobs obs(gobs);
        double tmp = this->_obs_phase(obs.gband());
        return tmp;
    }

    double t_gobsgnss::obs_D(const t_gobs &gobs) const
    {
        double tmp = this->_obs_doppler(gobs.gband());
        return tmp;
    }

    double t_gobsgnss::obs_D(const GOBS &gobs) const
    {
        t_gobs obs(gobs);
        double tmp = this->_obs_doppler(obs.gband());
        return tmp;
    }

    double t_gobsgnss::obs_S(const t_gobs &gobs) const
    {
        double tmp = this->_obs_snr(gobs.gband());
        return tmp;
    }

    double t_gobsgnss::obs_L(const t_gband &gb) const
    {
        double tmp = this->_obs_phase(gb);
        return tmp;
    }

    double t_gobsgnss::obs_D(const t_gband &gb) const
    {
        double tmp = this->_obs_doppler(gb);
        return tmp;
    }

    double t_gobsgnss::obs_S(const t_gband &gb) const
    {
        double tmp = this->_obs_snr(gb);
        return tmp;
    }

    double t_gobsgnss::_obs_phase(const t_gband &gb) const
    {
        // Glonass has 255 channel number
        if (!_valid_obs())
            return NULL_GOBS;

        // AUTO SELECTION
        if (gb.attr() == ATTR)
        {
            GOBS gobs = _id_phase(gb.band());
            if (gobs == X)
                return NULL_GOBS;
            else
                return _gobs.at(gobs) * this->wavelength(gb.band()); // transfer from whole cycles to meters!;
        }

        // FIXED SELECTION
        t_gobs go(TYPE_L, gb.band(), gb.attr());

        map<GOBS, double>::const_iterator it;
        it = _gobs.find(go.gobs());

        if (it == _gobs.end())
            return NULL_GOBS;
        return it->second * this->wavelength(gb.band()); // transfer from whole cycles to meters!
    }

    double t_gobsgnss::_obs_doppler(const t_gband &gb) const
    {
        // Glonass has 255 channel number
        if (!_valid_obs())
            return NULL_GOBS;

        // AUTO SELECTION
        if (gb.attr() == ATTR)
        {
            GOBS gobs = _id_doppler(gb.band());
            if (gobs == X)
                return NULL_GOBS;
            else
                return _gobs.at(gobs) * this->wavelength(gb.band());
            return NULL_GOBS;
        }

        t_gobs go(TYPE_D, gb.band(), gb.attr());

        map<GOBS, double>::const_iterator it = _gobs.end();

        it = _gobs.find(go.gobs());
        if (it == _gobs.end())
            return NULL_GOBS;

        return it->second * this->wavelength(gb.band());

        return 0.0;
    }

    double t_gobsgnss::_obs_snr(const t_gband &gb) const
    {
        // Glonass has 255 channel number
        if (!_valid_obs())
            return NULL_GOBS;

        // AUTO SELECTION
        if (gb.attr() == ATTR)
        {
            GOBS gobs = _id_snr(gb.band());
            if (gobs == X)
                return NULL_GOBS;
            else
                return _gobs.at(gobs);
            return NULL_GOBS;
        }

        t_gobs go(TYPE_S, gb.band(), gb.attr());

        map<GOBS, double>::const_iterator it = _gobs.end();

        it = _gobs.find(go.gobs());
        if (it == _gobs.end())
            return NULL_GOBS;

        return it->second;

        return 0.0;
    }

    int t_gobsgnss::mod_L(const double &dL, const GOBS &gobs, const int i)
    {
        map<GOBS, double>::iterator it;

        if (gobs == X)
        {
            for (it = _gobs.begin(); it != _gobs.end(); ++it)
            {
                // phase only !!!!
                string gobs_str = gobs2str(it->first);
                if (gobs_str.compare(0, 1, "L") != 0)
                    continue;

                // TEMPORARY CONVERSION !
                GOBSBAND gb1 = GOBSBAND(gobs2band(it->first));

                if (i == 1)
                    it->second += dL / this->wavelength(gb1); // transfer back to cycles
                else if (i == 0)
                    it->second += dL;
                else
                    cout << "Observations not modified!" << endl;
            }

            // modify requested only
        }
        else
        {
            it = _gobs.find(gobs);
            if (it == _gobs.end())
                return -1;

            // TEMPORARY CONVERSION !
            GOBSBAND gb1 = GOBSBAND(gobs2band(gobs));

            if (i == 1)
                it->second += dL / this->wavelength(gb1); // transfer back to cycles
            else if (i == 0)
                it->second += dL;
            else
                cout << "Observations not modified!" << endl;
        }
        return 0; // not found
    }

    int t_gobsgnss::mod_L(const double &dL, const GOBSBAND &band, const int i)
    {
        for (map<GOBS, double>::iterator it = _gobs.begin(); it != _gobs.end(); ++it)
        {
            // phase only !!!!
            string gobs_str = gobs2str(it->first);
            if (gobs_str.compare(0, 1, "L") != 0)
                continue;

            // TEMPORARY CONVERSION !
            GOBSBAND gb1 = GOBSBAND(gobs2band(it->first));
            if (gb1 != band)
                continue;

            if (i == 1)
                it->second += dL / this->wavelength(gb1); // transfer back to cycles
            else if (i == 0)
                it->second += dL;
            else if (i == 2)
                it->second = 0; // set to zero
            else
                cout << "Observations not modified!" << endl;
        }
        return 0;
    }

    double t_gobsgnss::frequency_lc(const int &band1, const double &coef1,
                                    const int &band2, const double &coef2,
                                    const int &band3, const double &coef3) const
    {
        double f1 = 0.0;
        double f2 = 0.0;
        double f3 = 0.0;

        // TEMPORARY CONVERSION !
        GOBSBAND gb1 = GOBSBAND(band1);
        GOBSBAND gb2 = GOBSBAND(band2);
        GOBSBAND gb3 = GOBSBAND(band3);

        if (coef1 != 0.0)
        {
            f1 = this->frequency(gb1);
            if (double_eq(f1, NULL_GOBS))
                return NULL_GOBS;
        }
        if (coef2 != 0.0)
        {
            f2 = this->frequency(gb2);
            if (double_eq(f2, NULL_GOBS))
                return NULL_GOBS;
        }
        if (coef3 != 0.0)
        {
            f3 = this->frequency(gb3);
            if (double_eq(f3, NULL_GOBS))
                return NULL_GOBS;
        }

        double freq = (coef1 * f1 + coef2 * f2 + coef3 * f3);

        return freq;
    }

    double t_gobsgnss::isf_lc(const int &band1, const double &coef1,
                              const int &band2, const double &coef2) const
    {

        double f1 = 1.0;
        double f2 = 1.0;

        // TEMPORARY CONVERSION !
        GOBSBAND gb1 = GOBSBAND(band1);
        GOBSBAND gb2 = GOBSBAND(band2);

        f1 = this->frequency(gb1);
        if (double_eq(f1, NULL_GOBS))
            return NULL_GOBS;
        f2 = this->frequency(gb2);
        if (double_eq(f2, NULL_GOBS))
            return NULL_GOBS;

        double denom = coef1 * f1 + coef2 * f2;
        double num = f1 * f1 * (coef1 / f1 + coef2 / f2);
        double isf = num / denom;

        return isf;
    }

    double t_gobsgnss::pnf_lc(const int &band1, const double &coef1,
                              const int &band2, const double &coef2,
                              const int &band3, const double &coef3) const
    {
        double f1 = 1.0;
        double f2 = 1.0;
        double f3 = 1.0;

        // TEMPORARY CONVERSION !
        GOBSBAND gb1 = GOBSBAND(band1);
        GOBSBAND gb2 = GOBSBAND(band2);
        GOBSBAND gb3 = GOBSBAND(band3);

        if (coef1 != 0.0)
        {
            f1 = this->frequency(gb1);
            if (double_eq(f1, NULL_GOBS))
                return NULL_GOBS;
        }
        if (coef2 != 0.0)
        {
            f2 = this->frequency(gb2);
            if (double_eq(f2, NULL_GOBS))
                return NULL_GOBS;
        }
        if (coef3 != 0.0)
        {
            f3 = this->frequency(gb3);
            if (double_eq(f3, NULL_GOBS))
                return NULL_GOBS;
        }

        double denom = pow(coef1 * f1 + coef2 * f2 + coef3 * f3, 2);
        double num = f1 * f1 * coef1 * coef1 + f2 * f2 * coef2 * coef2 + f3 * f3 * coef3 * coef3;
        double pnf = num / denom;

        return sqrt(pnf);
    }

    double t_gobsgnss::_lcf_range(const t_gobs *gobs1, const int &coef1,
                                  const t_gobs *gobs2, const int &coef2,
                                  const t_gobs *gobs3, const int &coef3) const
    {
        double C1 = NULL_GOBS, f1 = 0.0;
        double C2 = NULL_GOBS, f2 = 0.0;
        double C3 = NULL_GOBS, f3 = 0.0;

        if (gobs1 && coef1 != 0.0)
        {
            f1 = frequency(gobs1->band());
            C1 = obs_C(*gobs1);
            if (double_eq(C1, NULL_GOBS))
                return NULL_GOBS;
        }
        if (gobs2 && coef2 != 0.0)
        {
            f2 = frequency(gobs2->band());
            C2 = obs_C(*gobs2);
            if (double_eq(C2, NULL_GOBS))
                return NULL_GOBS;
        }
        if (gobs3 && coef3 != 0.0)
        {
            f3 = frequency(gobs3->band());
            C3 = obs_C(*gobs3);
            if (double_eq(C3, NULL_GOBS))
                return NULL_GOBS;
        }

        double lc = NULL_GOBS;

        if (fabs(C1 - C2) > 100)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Inconsistencies of the code observations " + _epoch.str_ymdhms());
            return lc;
        }

        string sat = this->sat();

        if (gobs3)
            lc = (coef1 * f1 * C1 + coef2 * f2 * C2 + coef3 * f3 * C3) / (coef1 * f1 + coef2 * f2 + coef3 * f3);
        else
            lc = (coef1 * f1 * C1 + coef2 * f2 * C2) / (coef1 * f1 + coef2 * f2);

        return lc;
    }

    double t_gobsgnss::_lcf_phase(const t_gobs *gobs1, const int &coef1,
                                  const t_gobs *gobs2, const int &coef2,
                                  const t_gobs *gobs3, const int &coef3) const
    {
        double L1 = NULL_GOBS, f1 = 0;
        double L2 = NULL_GOBS, f2 = 0;
        double L3 = NULL_GOBS, f3 = 0;

        if (gobs1 && coef1 != 0.0)
        {
            f1 = frequency(gobs1->band());
            L1 = obs_L(*gobs1);
            if (double_eq(L1, NULL_GOBS))
                return NULL_GOBS;
        }
        if (gobs2 && coef2 != 0.0)
        {
            f2 = frequency(gobs2->band());
            L2 = obs_L(*gobs2);
            if (double_eq(L2, NULL_GOBS))
                return NULL_GOBS;
        }
        if (gobs3 && coef3 != 0.0)
        {
            f3 = frequency(gobs3->band());
            L3 = obs_L(*gobs3);
            if (double_eq(L3, NULL_GOBS))
                return NULL_GOBS;
        }

        double lc = NULL_GOBS;

        if (gobs3)
            lc = (coef1 * f1 * L1 + coef2 * f2 * L2 + coef3 * f3 * L3) / (coef1 * f1 + coef2 * f2 + coef3 * f3);
        else
            lc = (coef1 * f1 * L1 + coef2 * f2 * L2) / (coef1 * f1 + coef2 * f2);

        return lc;
    }

    int t_gobsgnss::_coef_multpath(const GOBSBAND &bC, double &cC,
                                   const GOBSBAND &b1, double &c1,
                                   const GOBSBAND &b2, double &c2) const
    {
        if (b1 == b2)
        {
            c1 = c2 = 0.0;
            return -1;
        } // phase should be different!

        double pfacC = pow(this->frequency(bC), 2);
        double pfac1 = pow(this->frequency(b1), 2);
        double pfac2 = pow(this->frequency(b2), 2);

        double denominator = (pfac1 - pfac2) * pfacC;

        cC = 1.0;
        c1 = -(pfac1 * (pfacC + pfac2)) / denominator;
        c2 = +(pfac2 * (pfacC + pfac1)) / denominator;

        return 0;
    }

    double t_gobsgnss::P3(const GOBSBAND &b1, const GOBSBAND &b2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        double coef1, coef2;

        t_gobs g1(TYPE, b1, ATTR);
        t_gobs g2(TYPE, b2, ATTR);

        _coef_ionofree(g1.band(), coef1, g2.band(), coef2);

        double lc = _lc_range(&g1, coef1, &g2, coef2); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::P3(const t_gobs &g1, const t_gobs &g2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        double coef1, coef2;

        _coef_ionofree(g1.band(), coef1, g2.band(), coef2);

        double lc = _lc_range(&g1, coef1, &g2, coef2); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::P3(const GOBS &g1, const GOBS &g2) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        t_gobs tmp1(g1);
        t_gobs tmp2(g2);

        double coef1, coef2;

        _coef_ionofree(tmp1.band(), coef1, tmp2.band(), coef2);

        double lc = _lc_range(&tmp1, coef1, &tmp2, coef2); // still dual-frequency only
        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::P4(const GOBSBAND &b1, const GOBSBAND &b2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        double coef1, coef2;

        t_gobs g1(TYPE, b1, ATTR);
        t_gobs g2(TYPE, b2, ATTR);

        _coef_geomfree(g1.band(), coef1, g2.band(), coef2);

        double lc = _lc_range(&g1, coef1, &g2, coef2); // still dual-frequency only
        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::P4(const t_gobs &g1, const t_gobs &g2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        double coef1, coef2;

        _coef_geomfree(g1.band(), coef1, g2.band(), coef2);

        double lc = _lc_range(&g1, coef1, &g2, coef2); // still dual-frequency only
        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::L3(const GOBSBAND &b1, const GOBSBAND &b2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double coef1, coef2;

        t_gobs g1(TYPE_L, b1, ATTR);
        t_gobs g2(TYPE_L, b2, ATTR);

        _coef_ionofree(b1, coef1, b2, coef2);

        double lc = _lc_phase(&g1, coef1, &g2, coef2); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::L3(const t_gobs &g1, const t_gobs &g2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double coef1, coef2;

        _coef_ionofree(g1.band(), coef1, g2.band(), coef2);

        double lc = _lc_phase(&g1, coef1, &g2, coef2); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::L3(const GOBS &g1, const GOBS &g2) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        t_gobs tmp1(g1);
        t_gobs tmp2(g2);

        double coef1, coef2;

        _coef_ionofree(tmp1.band(), coef1, tmp2.band(), coef2);

        double lc = _lc_phase(&tmp1, coef1, &tmp2, coef2); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::L3_cycle(const t_gobs &g1, const t_gobs &g2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double wlength1 = wavelength(g1.band());
        double wlength2 = wavelength(g2.band());
        double fact = wlength1 / wlength2;
        double fact2 = fact * fact;

        double L1 = NULL_GOBS;
        double L2 = NULL_GOBS;

        L1 = obs_L(g1) / wlength1;
        if (double_eq(L1, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L2 = obs_L(g2) / wlength2;
        if (double_eq(L2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double lc = L1 / (1.0 - fact2) - L2 / (1.0 / fact - fact);

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::L4(const GOBSBAND &b1, const GOBSBAND &b2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        t_gobs g1(TYPE_L, b1, ATTR);
        t_gobs g2(TYPE_L, b2, ATTR);

        double coef1, coef2;

        _coef_geomfree(b1, coef1, b2, coef2);

        double lc = _lc_phase(&g1, coef1, &g2, coef2); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::L4(const t_gobs &g1, const t_gobs &g2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double coef1, coef2;

        _coef_geomfree(g1.band(), coef1, g2.band(), coef2);

        double lc = _lc_phase(&g1, coef1, &g2, coef2); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::L4_cycle(const t_gobs &g1, const t_gobs &g2) const // const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double wlength1 = wavelength(g1.band());
        double wlength2 = wavelength(g2.band());
        double fact = wlength1 / wlength2;

        double L1 = NULL_GOBS;
        double L2 = NULL_GOBS;

        L1 = obs_L(g1) / wlength1;
        if (double_eq(L1, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L2 = obs_L(g2) / wlength2;
        if (double_eq(L2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double lg = (fact * L1 - L2) / (1 - fact);
        _gmutex.unlock();
        return lg;
    }

    double t_gobsgnss::MW(const t_gobs &g1, const t_gobs &g2, bool phacod_consistent) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (gsys() == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double wideL = NULL_GOBS;
        double narrP = NULL_GOBS;

        t_gobs L1(TYPE_L, g1.band(), g1.attr()); // should always be as input 1st argument
        t_gobs L2(TYPE_L, g2.band(), g2.attr()); // should always be as input 2nd argument

        t_gobs C1(TYPE, g1.band(), ATTR); // TYPE universal to handle P+C code
        t_gobs C2(TYPE, g2.band(), ATTR); // TYPE universal to handle P+C code

        if (phacod_consistent)
        { // overwrite attribute
            C1.gattr(g1.gattr());
            C2.gattr(g2.gattr());
        }

        wideL = _lcf_phase(&L1, 1, &L2, -1);
        narrP = _lcf_range(&C1, 1, &C2, 1);

        if (double_eq(narrP, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        if (double_eq(wideL, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double lc = narrP - wideL;

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::MW_cycle(const t_gobs &gL1, const t_gobs &gL2,
                                const t_gobs &gC1, const t_gobs &gC2) const
    {
        _gmutex.lock();

        if (gsys() == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double wlength1 = wavelength(gL1.band());
        double wlength2 = wavelength(gL2.band());
        double fact = wlength1 / wlength2;

        double C1 = NULL_GOBS;
        double C2 = NULL_GOBS;
        double L1 = NULL_GOBS;
        double L2 = NULL_GOBS;

        C1 = obs_C(gC1);
        if (double_eq(C1, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        C2 = obs_C(gC2);
        if (double_eq(C2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L1 = obs_L(gL1) / wlength1;
        if (double_eq(L1, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L2 = obs_L(gL2) / wlength2;
        if (double_eq(L2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double mw = L1 - L2 - (C1 / wlength1 + C2 / wlength2) * (1.0 - fact) / (1.0 + fact);

        _gmutex.unlock();
        return mw;
    }
    double t_gobsgnss::EWL_cycle(const t_gobs &gL1, const t_gobs &gL2, const t_gobs &gL3,
                                 const t_gobs &gC1, const t_gobs &gC2) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (gsys() == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double wlength1 = wavelength(gL1.band());
        double wlength2 = wavelength(gL2.band());
        double wlength3 = wavelength(gL3.band());
        if (double_eq(wlength1, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        if (double_eq(wlength2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        if (double_eq(wlength3, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        double alpha = (wlength2 * wlength3 - wlength1 * wlength1) / (wlength2 * wlength2 - wlength1 * wlength1);

        double C1 = NULL_GOBS;
        double C2 = NULL_GOBS;
        double L2 = NULL_GOBS;
        double L3 = NULL_GOBS;

        C1 = obs_C(gC1);
        if (double_eq(C1, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        C2 = obs_C(gC2);
        if (double_eq(C2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L2 = obs_L(gL2) / wlength2;
        if (double_eq(L2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L3 = obs_L(gL3) / wlength3;
        if (double_eq(L3, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double ewl = L2 - L3 - (C1 * (1 - alpha) + C2 * alpha) * (wlength3 - wlength2) / (wlength2 * wlength3);

        _gmutex.unlock();
        return ewl;
    }
    double t_gobsgnss::LW_meter(const t_gobs &gL1, const t_gobs &gL2) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (gsys() == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double wlength1 = wavelength(gL1.band());
        double wlength2 = wavelength(gL2.band());

        double L1 = NULL_GOBS;
        double L2 = NULL_GOBS;

        L1 = obs_L(gL1) / wlength1;
        if (double_eq(L1, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L2 = obs_L(gL2) / wlength2;
        if (double_eq(L2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double lw = (L1 - L2) / (1.0 / wlength1 - 1.0 / wlength2);

        _gmutex.unlock();
        return lw;
    }
    double t_gobsgnss::LE_meter(const t_gobs &gL2, const t_gobs &gL3) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (gsys() == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double wlength2 = wavelength(gL2.band());
        double wlength3 = wavelength(gL3.band());
        if (double_eq(wlength2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        if (double_eq(wlength3, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double L2 = NULL_GOBS;
        double L3 = NULL_GOBS;

        L2 = obs_L(gL2) / wlength2;
        if (double_eq(L2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L3 = obs_L(gL3) / wlength3;
        if (double_eq(L3, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double le = (L2 - L3) / (1.0 / wlength2 - 1.0 / wlength3);

        _gmutex.unlock();
        return le;
    }
    double t_gobsgnss::LWL_factor13(const t_gobs &gL1, const t_gobs &gL3) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (gsys() == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double wlength1 = wavelength(gL1.band());
        double wlength3 = wavelength(gL3.band());
        if (double_eq(wlength1, NULL_GOBS) || double_eq(wlength3, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double lwl_factor13 = wlength1 / wlength3;

        _gmutex.unlock();
        return lwl_factor13;
    }

    double t_gobsgnss::LNL(const t_gobs &g1, const t_gobs &g2) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double lc = _lcf_phase(&g1, 2, &g2, -1); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::PWL(const t_gobs &g1, const t_gobs &g2) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double lc = _lcf_range(&g1, 1, &g2, 1); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::LWL(const t_gobs &g1, const t_gobs &g2) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (_gsys == GLO && double_eq(_channel, DEF_CHANNEL))
        {
            _gmutex.unlock();
            return 0.0;
        }

        double lc = _lcf_phase(&g1, 1, &g2, -1); // still dual-frequency only

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::MP(const t_gobs &code, const t_gobs &L1, const t_gobs &L2) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        if (!code.is_code())
            return NULL_GOBS;
        if (!L1.is_phase())
            return NULL_GOBS;
        if (!L2.is_phase())
            return NULL_GOBS;

        double coef1, coef2, coefC;
        double C = _obs_range(code);

        if (double_eq(C, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        _coef_multpath(code.band(), coefC, L1.band(), coef1, L2.band(), coef2);

        double L = _lc_phase(&L1, coef1, &L2, coef2);
        if (double_eq(L, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        string sat = this->sat();

        double lc = coefC * C + L;

        _gmutex.unlock();
        return lc;
    }

    double t_gobsgnss::GFIF_meter(const t_gobs &gL1, const t_gobs &gL2, const t_gobs &gL3) const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        double frequency1 = frequency(gL1.band());
        double frequency2 = frequency(gL2.band());
        double frequency3 = frequency(gL3.band());
        if (double_eq(frequency1, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        if (double_eq(frequency2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        if (double_eq(frequency3, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double coef_1 = frequency1 * frequency1 / (frequency1 * frequency1 - frequency2 * frequency2);
        double coef_2 = -frequency2 * frequency2 / (frequency1 * frequency1 - frequency2 * frequency2);
        double coef_3 = frequency1 * frequency1 / (frequency1 * frequency1 - frequency3 * frequency3);
        double coef_4 = -frequency3 * frequency3 / (frequency1 * frequency1 - frequency3 * frequency3);

        double L1 = NULL_GOBS;
        double L2 = NULL_GOBS;
        double L3 = NULL_GOBS;

        L1 = obs_L(gL1);
        if (double_eq(L1, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L2 = obs_L(gL2);
        if (double_eq(L2, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }
        L3 = obs_L(gL3);
        if (double_eq(L3, NULL_GOBS))
        {
            _gmutex.unlock();
            return NULL_GOBS;
        }

        double GFIF_obs = (coef_1 * L1 + coef_2 * L2) - (coef_3 * L1 + coef_4 * L3);

        _gmutex.unlock();
        return GFIF_obs;
    }

    bool t_gobsgnss::valid() const
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        bool tmp = this->_valid();

        _gmutex.unlock();
        return tmp;
    }

    bool t_gobsgnss::obs_empty() const
    {
        return _gobs.empty();
    }

    void t_gobsgnss::clear()
    {
#ifdef BMUTEX
        boost::mutex::scoped_lock lock(_mutex);
#endif
        _gmutex.lock();

        this->_clear();

        _gmutex.unlock();
        return;
    }

    void t_gobsgnss::_clear()
    {
        _staid.clear();
        _epoch = FIRST_TIME;
    }

    bool t_gobsgnss::_valid() const
    {
        if (_staid.empty() ||
            _staid == "" ||
            _epoch == FIRST_TIME)
            return false;

        return true;
    }

    void t_gobsgnss::nbands(pair<int, int> &nb)
    {
        for (int b = 1; b <= 8; b++)
        {
            if (_cod_id(b) != X)
                nb.first++;
            if (_pha_id(b) != X)
                nb.second++;
        }
    }

    set<GOBSBAND> t_gobsgnss::band_avail(bool phase) const
    {
        _gmutex.lock();

        set<GOBSBAND> s_f;

        if (phase)
            s_f = _band_avail();
        else
            s_f = _band_avail_code();

        _gmutex.unlock();
        return s_f;
    }

    set<GFRQ> t_gobsgnss::freq_avail() const
    {
        _gmutex.lock();
        set<GFRQ> s_f = _freq_avail();

        _gmutex.unlock();
        return s_f;
    }

    bool t_gobsgnss::contain_freq(const FREQ_SEQ &freq) const
    {
        _gmutex.lock();

        bool res = false;

        set<GFRQ> frqs = _freq_avail();
        for (auto it = frqs.begin(); it != frqs.end(); it++)
        {
            FREQ_SEQ fs = t_gsys::gfrq2freq(_gsys, *it);
            if (fs == freq)
                res = true;
        }

        _gmutex.unlock();
        return res;
    }

    set<GFRQ> t_gobsgnss::_freq_avail() const
    {
        set<GFRQ> s_f;
        for (map<GOBS, double>::const_iterator it = _gobs.begin(); it != _gobs.end(); it++)
        {
            t_gobs gobs;
            gobs.gobs(it->first);
            if (double_eq(it->second, 0.0) || !gobs.is_phase())
                continue;
            else
            {
                s_f.insert(t_gsys::band2gfrq(_gsys, gobs.band()));
            }
        }
        return s_f;
    }

    set<GOBSBAND> t_gobsgnss::_band_avail() const
    {
        set<GOBSBAND> s_f;
        for (map<GOBS, double>::const_iterator it = _gobs.begin(); it != _gobs.end(); it++)
        {
            t_gobs gobs;
            gobs.gobs(it->first);

            if (double_eq(it->second, 0.0) || !gobs.is_phase())
                continue;
            else
                s_f.insert(gobs.band());
        }
        return s_f;
    }

    set<GOBSBAND> t_gobsgnss::_band_avail_code() const
    {
        set<GOBSBAND> s_f;
        for (map<GOBS, double>::const_iterator it = _gobs.begin(); it != _gobs.end(); it++)
        {
            t_gobs gobs;
            gobs.gobs(it->first);

            if (double_eq(it->second, 0.0) || gobs.is_phase())
                continue;
            else
                s_f.insert(gobs.band());
        }
        return s_f;
    }

    bool t_gobsgnss::_valid_obs() const
    {
        if (_gsys == GLO && _channel == 255)
            return false;
        else
            return true;
    }

    double t_gobsgnss::_lc_range(const t_gobs *g1, const double &coef1,
                                 const t_gobs *g2, const double &coef2,
                                 const t_gobs *g3, const double &coef3) const
    {
        double C1 = NULL_GOBS;
        double C2 = NULL_GOBS;
        double C3 = NULL_GOBS;

        if (g1 && coef1 != 0.0)
        {
            C1 = obs_C(*g1);
            if (double_eq(C1, NULL_GOBS))
                return NULL_GOBS;
        }
        if (g2 && coef2 != 0.0)
        {
            C2 = obs_C(*g2);
            if (double_eq(C2, NULL_GOBS))
                return NULL_GOBS;
        }
        if (g3 && coef3 != 0.0)
        {
            C3 = obs_C(*g3);
            if (double_eq(C3, NULL_GOBS))
                return NULL_GOBS;
        }

        if (fabs(C1 - C2) > 100)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Inconsistencies of the code observations " + _epoch.str_ymdhms());
            return 0.0;
        }

        string sat = this->sat();

        double lc = (coef1 * C1 + coef2 * C2 + coef3 * C3);

        return lc;
    }

    double t_gobsgnss::_lc_phase(const t_gobs *g1, const double &coef1,
                                 const t_gobs *g2, const double &coef2,
                                 const t_gobs *g3, const double &coef3) const
    {
        double L1 = NULL_GOBS;
        double L2 = NULL_GOBS;
        double L3 = NULL_GOBS;

        if (g1 && coef1 != 0.0)
        {
            L1 = obs_L(*g1);
            if (double_eq(L1, NULL_GOBS))
                return NULL_GOBS;
        }
        if (g2 && coef2 != 0.0)
        {
            L2 = obs_L(*g2);
            if (double_eq(L2, NULL_GOBS))
                return NULL_GOBS;
        }
        if (g3 && coef3 != 0.0)
        {
            L3 = obs_L(*g3);
            if (double_eq(L3, NULL_GOBS))
                return NULL_GOBS;
        }

        double lc = (coef1 * L1 + coef2 * L2 + coef3 * L3);

        if (double_eq(L1, 0.0) || double_eq(L2, 0.0))
            return 0.0;

        return lc;
    }

    GOBS t_gobsgnss::_id_range(const GOBSBAND &b) const
    {
        map<GOBS, double>::const_iterator it = _gobs.begin();

        while (it != _gobs.end())
        {
            GOBS gobs = it->first;
            if (gobs_code(gobs))
            {
                GOBSBAND tmp = str2gobsband(gobs2str(gobs));
                if (tmp == b)
                    return gobs;
            }
            it++;
        }
        return X;
    }

    GOBS t_gobsgnss::_id_phase(const GOBSBAND &b) const
    {
        map<GOBS, double>::const_iterator it = _gobs.begin();

        while (it != _gobs.end())
        {
            GOBS gobs = it->first;
            if (gobs_phase(gobs))
            {
                GOBSBAND tmp = str2gobsband(gobs2str(gobs));
                if (tmp == b)
                    return gobs;
            }
            it++;
        }
        return X;
    }

    GOBS t_gobsgnss::_id_doppler(const GOBSBAND &b) const
    {
        map<GOBS, double>::const_iterator it = _gobs.begin();

        while (it != _gobs.end())
        {
            GOBS gobs = it->first;
            if (gobs_doppler(gobs))
            {
                GOBSBAND tmp = str2gobsband(gobs2str(gobs));
                if (tmp == b)
                    return gobs;
            }
            it++;
        }
        return X;
    }

    GOBS t_gobsgnss::_id_snr(const GOBSBAND &b) const
    {
        map<GOBS, double>::const_iterator it = _gobs.begin();

        while (it != _gobs.end())
        {
            GOBS gobs = it->first;
            if (gobs_snr(gobs))
            {
                GOBSBAND tmp = str2gobsband(gobs2str(gobs));
                if (tmp == b)
                    return gobs;
            }
            it++;
        }
        return X;
    }

    GOBS t_gobsgnss::select_range(const GOBSBAND &band, const bool &isRawAll) const
    {

        int max = -1;
        map<GOBS, double>::const_iterator it = _gobs.begin();
        map<GOBS, double>::const_iterator it_find = _gobs.begin();
        while (it != _gobs.end())
        {
            GOBS gobs = it->first;

            if (gobs_code(gobs))
            {
                GOBSBAND tmp = str2gobsband(gobs2str(gobs));
                if (tmp == band)
                {
                    t_gobs gobs_temp(gobs);
                    gobs_temp.gobs2to3(_gsys);
                    int loc = -1;

                    char attr_tmp = (gobs2str(gobs_temp.gobs()))[2];
                    try
                    {
                        if (!isRawAll)
                        {
                            loc = range_order_attr_cmb.at(_gsys).at(band).find(attr_tmp);
                        }
                        else
                        {
                            loc = range_order_attr_raw.at(_gsys).at(band).find(attr_tmp);
                        }
                    }
                    catch (...)
                    {
                        loc = -1;
                    }

                    if (loc > max)
                    {
                        it_find = it;
                        max = loc;
                    }
                }
            }
            it++;
        }

        if (max == -1)
        {
            return X;
        }
        else
        {
            return (it_find->first);
        }
    }

    GOBS t_gobsgnss::select_phase(const GOBSBAND &band, const bool &isRawAll) const
    {
        int max = -1;
        map<GOBS, double>::const_iterator it = _gobs.begin();
        map<GOBS, double>::const_iterator it_find = _gobs.begin();
        while (it != _gobs.end())
        {
            GOBS gobs = it->first;
            if (gobs_phase(gobs))
            {
                GOBSBAND tmp = str2gobsband(gobs2str(gobs));
                if (tmp == band)
                {
                    t_gobs gobs_temp(gobs);
                    gobs_temp.gobs2to3(_gsys);
                    int loc = -1;

                    char attr_tmp = (gobs2str(gobs_temp.gobs()))[2];
                    try
                    {
                        if (!isRawAll)
                        {
                            loc = phase_order_attr_cmb.at(_gsys).at(band).find(attr_tmp);
                        }
                        else
                        {
                            loc = phase_order_attr_raw.at(_gsys).at(band).find(attr_tmp);
                        }
                    }
                    catch (...)
                    {
                        loc = -1;
                    }

                    if (loc > max)
                    {
                        it_find = it;
                        max = loc;
                    }
                }
            }
            it++;
        }

        if (max == -1)
        {
            return X;
        }
        else
        {
            return (it_find->first);
        }
    }

    GOBS t_gobsgnss::_cod_id(const int &band) const // , const GOBS& obs)
    {
        size_t sz = sizeof(code_choise[band]) / sizeof(GOBS);
        map<GOBS, double>::const_iterator it; 

        GOBS tmp = X; // obs;

        // choices
        for (size_t i = 0; i < sz; ++i)
        {
            tmp = code_choise[band][i];

            it = _gobs.find(tmp);
            if (tmp == X || it == _gobs.end())
                continue;

            if (!double_eq(it->second, NULL_GOBS))
                return tmp;
        }

        return X;
    }

    GOBS t_gobsgnss::_pha_id(const int &band) const // , const GOBS& obs)
    {
        size_t sz = sizeof(phase_choise[band]) / sizeof(GOBS);
        map<GOBS, double>::const_iterator it; 

        GOBS tmp = X; // obs;

        // choices
        for (size_t i = 0; i < sz; ++i)
        {
            tmp = phase_choise[band][i];

            it = _gobs.find(tmp);
            if (tmp == X || it == _gobs.end())
                continue;

            if (!double_eq(it->second, NULL_GOBS))
                return tmp;
        }

        return X;
    }

    int t_gobsgnss::_coef_ionofree(const GOBSBAND &b1, double &c1,
                                   const GOBSBAND &b2, double &c2) const
    {
        if (b1 == b2)
        {
            c1 = c2 = 0.0;
            return -1;
        }

        double fac1 = this->frequency(b1);
        double fac2 = this->frequency(b2);

        c1 = fac1 * fac1 / (fac1 * fac1 - fac2 * fac2);
        c2 = -fac2 * fac2 / (fac1 * fac1 - fac2 * fac2);

        return 0;
    }

    int t_gobsgnss::coef_ionofree(const GOBSBAND &b1, double &c1,
                                  const GOBSBAND &b2, double &c2) const
    {
        return _coef_ionofree(b1, c1, b2, c2);
    }

    int t_gobsgnss::coef_ionofree_phi(const GOBSBAND &b1, double &c1, const GOBSBAND &b2, double &c2) const
    {
        if (b1 == b2)
        {
            c1 = c2 = 0.0;
            return -1;
        }

        double fac1 = this->frequency(b1);
        double fac2 = this->frequency(b2);

        c1 = fac1 * fac1 / (fac1 * fac1 - fac2 * fac2);
        c2 = -fac1 * fac2 / (fac1 * fac1 - fac2 * fac2);

        return 0;
    }

    int t_gobsgnss::_coef_geomfree(const GOBSBAND &b1, double &c1,
                                   const GOBSBAND &b2, double &c2) const
    {
        if (b1 == b2)
        {
            c1 = c2 = 0.0;
            return -1;
        }
        c1 = 1.0;
        c2 = -1.0;

        return 0;
    }

    int t_gobsgnss::coef_geomfree(const GOBSBAND &b1, double &c1,
                                  const GOBSBAND &b2, double &c2) const
    {
        return _coef_geomfree(b1, c1, b2, c2);
    }

    int t_gobsgnss::_coef_narrlane(const GOBSBAND &b1, double &c1,
                                   const GOBSBAND &b2, double &c2) const
    {
        if (b1 == b2)
        {
            c1 = c2 = 0.0;
            return -1;
        }

        double fac1 = this->frequency(b1);
        double fac2 = this->frequency(b2);

        c1 = fac1 / (fac1 + fac2);
        c2 = fac2 / (fac1 + fac2);

        return 0;
    }

    int t_gobsgnss::coef_narrlane(const GOBSBAND &b1, double &c1,
                                  const GOBSBAND &b2, double &c2) const
    {
        return _coef_narrlane(b1, c1, b2, c2);
    }

    int t_gobsgnss::_coef_widelane(const GOBSBAND &b1, double &c1,
                                   const GOBSBAND &b2, double &c2) const
    {
        if (b1 == b2)
        {
            c1 = c2 = 0.0;
            return -1;
        }

        double fac1 = this->frequency(b1);
        double fac2 = this->frequency(b2);

        c1 = fac1 / (fac1 - fac2);
        c2 = -fac2 / (fac1 - fac2);

        return 0;
    }

    int t_gobsgnss::coef_widelane(const GOBSBAND &b1, double &c1,
                                  const GOBSBAND &b2, double &c2) const
    {
        return _coef_widelane(b1, c1, b2, c2);
    }

    bool t_obscmb::operator<(const t_obscmb &t) const
    {
        return (this->first.type() < t.first.type() &&
                this->first.band() < t.first.band() &&
                this->first.attr() < t.first.attr());
    }

} // namespace
