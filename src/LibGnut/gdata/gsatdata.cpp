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

#include "gdata/gsatdata.h"
#include "gmodels/gephplan.h"
#include "gutils/gtypeconv.h"
#include "gutils/gsysconv.h"
#include "gutils/gmatrixconv.h"
#include "gdata/gnavbds.h"
#include "gdata/gnavgps.h"
#include "gdata/gnavgal.h"
#include "gdata/gnavglo.h"
#include "gdata/gnavqzs.h"
#include "gdata/gnavsbs.h"
#include "gdata/gnavirn.h"
using namespace std;

namespace gnut
{
    t_gsatdata::t_gsatdata() : t_gobsgnss()
    {
        id_type(t_gdata::SATDATA);
        id_group(t_gdata::GRP_OBSERV);
    }
    t_gsatdata::t_gsatdata(t_spdlog spdlog) : t_gobsgnss(spdlog)
    {
        id_type(t_gdata::SATDATA);
        id_group(t_gdata::GRP_OBSERV);
    }
    t_gsatdata::t_gsatdata(t_spdlog spdlog, const string &site, const string &sat, const t_gtime &t)
        : t_gobsgnss(spdlog, site, sat, t),
          _satcrd(0.0, 0.0, 0.0),
          _satpco(0.0, 0.0, 0.0),
          _dloudx(0.0, 0.0, 0.0),
          _drate(0.0),
          _clk(0.0),
          _dclk(0.0),
          _ele(0.0),
          _azi_rec(0.0),
          _rho(0.0),
          _eclipse(false),
          _mfH(0.0),
          _mfW(0.0),
          _mfG(0.0),
          _wind(0.0),
          _low_prec(false),
          _slipf(false),
          _beta_val(999),
          _orb_angle_val(999),
          _yaw(999)

    {
        id_type(t_gdata::SATDATA);
        id_group(t_gdata::GRP_OBSERV);
    }

    t_gsatdata::t_gsatdata(const t_gobsgnss &obs)
        : t_gobsgnss(obs), // t_gobsgnss(obs.site() ,obs.sat(), obs.epoch() ),
          _satcrd(0.0, 0.0, 0.0),
          _satpco(0.0, 0.0, 0.0),
          _clk(0.0),
          _ele(0.0),
          _azi_rec(0.0),
          _rho(0.0),
          _eclipse(false),
          _mfH(0.0),
          _mfW(0.0),
          _mfG(0.0),
          _wind(0.0),
          _low_prec(false),
          _slipf(false),
          _beta_val(999),
          _orb_angle_val(999),
          _drate(0.0),
          _yaw(999)
    {
        _spdlog = (obs.spdlog());
        id_type(t_gdata::SATDATA);
        id_group(t_gdata::GRP_OBSERV);
    }

    t_gsatdata::~t_gsatdata()
    {
    }

    void t_gsatdata::addpco(const t_gtriple &pco)
    {
        _gmutex.lock();
        _satpco = pco;
        _gmutex.unlock();
        return;
    }

    void t_gsatdata::addcrd(const t_gtriple &crd)
    {
        _gmutex.lock();
        _satcrd = crd;
        _gmutex.unlock();
        return;
    }

    void t_gsatdata::addcrdcrs(const t_gtriple &crd)
    {
        _satcrdcrs = crd;
        return;
    }

    void t_gsatdata::addvel(const t_gtriple &vel)
    {
        _gmutex.lock();
        _satvel = vel;
        _gmutex.unlock();
        return;
    }

    void t_gsatdata::addvel_crs(const t_gtriple &vel)
    {
        _gmutex.lock();
        _satvel_crs = vel;
        _gmutex.unlock();
        return;
    }

    int t_gsatdata::addprd(t_gallnav *gnav, const bool &corrTOT, const bool &msk_health)
    {
        _low_prec = false;

        int irc = this->_addprd(gnav, corrTOT, msk_health);

        return irc;
    }

    int t_gsatdata::addprd_nav(t_gallnav *gnav, const bool &corrTOT, const bool &msk_health)
    {

        _gmutex.lock();

        _low_prec = true;

        int irc = this->_addprd(gnav, corrTOT, msk_health);

        _gmutex.unlock();
        return irc;
    }

    void t_gsatdata::addclk(const double &clk)
    {
        _gmutex.lock();
        _clk = clk;
        _gmutex.unlock();
    }

    void t_gsatdata::addreldelay(const double &rel)
    {
        _gmutex.lock();
        _reldelay = rel;
        _gmutex.unlock();
    }

    void t_gsatdata::addSCF2CRS(const Matrix &scf2crs, const Matrix &scf2trs)
    {
        _scf2crs = scf2crs;
        _scf2trs = scf2trs;
    }

    void t_gsatdata::adddrate(const double &drate)
    {
        _drate = drate;
    }

    void t_gsatdata::addrecTime(const t_gtime &recTime)
    {
        _TR = recTime;
    }
    void t_gsatdata::addsatTime(const t_gtime &satTime)
    {
        _TS = satTime;
    }

    void t_gsatdata::addele(const double &ele)
    {
        _gmutex.lock();
        _ele = ele;
        _gmutex.unlock();
    }

    void t_gsatdata::addele_leo(const double &ele)
    {
        _gmutex.lock();
        _ele_leo = ele;
        _gmutex.unlock();
    }

    void t_gsatdata::addazi_rec(const double &azi)
    {
        _gmutex.lock();
        _azi_rec = azi;
        _gmutex.unlock();
    }

    void t_gsatdata::addzen_rec(const double &zen)
    {
        _gmutex.lock();
        _zen_rec = zen;
        _gmutex.unlock();
    }

    void t_gsatdata::addazi_sat(const double &azi_sat)
    {
        _gmutex.lock();
        _azi_sat = azi_sat;
        _gmutex.unlock();
    }

    void t_gsatdata::addzen_sat(const double &zen_sat)
    {
        _gmutex.lock();
        _zen_sat = zen_sat;
        _gmutex.unlock();
    }

    void t_gsatdata::addnadir(const double &nadir)
    {
        _gmutex.lock();
        _nadir = nadir;
        _gmutex.unlock();
    }

    void t_gsatdata::addrho(const double &rho)
    {
        _gmutex.lock();
        _rho = rho;
        _gmutex.unlock();
    }

    void t_gsatdata::addmfH(const double &mfH)
    {
        _gmutex.lock();
        _mfH = mfH;
        _gmutex.unlock();
    }

    void t_gsatdata::addmfW(const double &mfW)
    {
        _gmutex.lock();
        _mfW = mfW;
        _gmutex.unlock();
    }

    void t_gsatdata::addmfG(const double &mfG)
    {
        _gmutex.lock();
        _mfG = mfG;
        _gmutex.unlock();
    }

    const t_gtriple &t_gsatdata::satcrd() const
    {
        return _satcrd;
    }

    const t_gtriple &t_gsatdata::satvel() const
    {
        return _satvel;
    }

    const double &t_gsatdata::clk() const
    {
        return _clk;
    }

    const double &t_gsatdata::dclk() const
    {
        return _dclk;
    }

    const double &t_gsatdata::drate() const
    {
        return _drate;
    }

    const Matrix &t_gsatdata::orbfunct() const
    {
        return _orbfunct;
    }

    int t_gsatdata::satindex()
    {
        return _satindex;
    }

    const t_gtriple &t_gsatdata::reccrd() const
    {
        return _reccrd;
    }
    const t_gtriple &t_gsatdata::sat2reccrs() const
    {
        return _sat2reccrs;
    }

    const Matrix &t_gsatdata::rotmat() const
    {
        return _rotmat;
    }

    const Matrix &t_gsatdata::drdxpole() const
    {
        return _drdxpole;
    }

    const Matrix &t_gsatdata::drdypole() const
    {
        return _drdypole;
    }

    const Matrix &t_gsatdata::drdut1() const
    {
        return _drdut1;
    }

    const t_gtime &t_gsatdata::recTime() const
    {
        return _TR;
    }

    const double &t_gsatdata::ele() const
    {
        return _ele;
    }

    const double &t_gsatdata::ele_leo() const
    {
        return _ele_leo;
    }

    double t_gsatdata::ele_deg() const
    {
        _gmutex.lock();
        double tmp = _ele * 180.0 / G_PI;
        _gmutex.unlock();
        return tmp;
    }

    double t_gsatdata::ele_leo_deg() const
    {
        _gmutex.lock();
        double tmp = _ele_leo * 180.0 / G_PI;
        _gmutex.unlock();
        return tmp;
    }

    const double &t_gsatdata::azi() const
    {
        return _azi_rec;
    }

    const double &t_gsatdata::azi_sat() const
    {

        return _azi_sat;
    }

    const double &t_gsatdata::rho() const
    {
        return _rho;
    }

    bool t_gsatdata::valid()
    {
        _gmutex.lock();
        bool tmp = this->_valid();
        _gmutex.unlock();
        return tmp;
    }

    bool t_gsatdata::is_carrier_range(const GOBSBAND& band) const
    {
        if (_is_carrier_range.find(band) == _is_carrier_range.end())
            return false;
        else
            return _is_carrier_range.at(band);
    }

    void t_gsatdata::addslip(const bool &flag)
    {
        _gmutex.lock();
        _slipf = flag;
        _gmutex.unlock();
    }

    const bool &t_gsatdata::islip() const
    {
        return _slipf;
    }

    double t_gsatdata::beta()
    {
        _gmutex.lock();
        double tmp = _b();
        _gmutex.unlock();
        return tmp;
    }

    double t_gsatdata::orb_angle()
    {
        _gmutex.lock();
        double tmp = _orb_angle();
        _gmutex.unlock();
        return tmp;
    }

    void t_gsatdata::clear()
    {
        _gmutex.lock();
        this->_clear();
        _gmutex.unlock();
        return;
    }

    int t_gsatdata::_addprd(t_gallnav *gnav, const bool &corrTOT, const bool &msk_health)
    {

        string satname(_satid);

        GSYS gs = this->gsys();

        GOBSBAND b1, b2;
        b1 = b2 = BAND;

        // automatic selection of two bands for IF LC
        set<GOBSBAND> bands = _band_avail_code();
        auto itBAND = bands.begin();
        if (corrTOT)
        {
            if (bands.size() < 1)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "At least two bands are necessary for TOT correction in sat pos/clk calculation!");
                return -1;
            }
            else if (bands.size() < 2)
            {
                b1 = *itBAND;
            }
            else
            {
                b1 = *itBAND;
                itBAND++;
                b2 = *itBAND;
            }
        }
        else
        {
            b1 = *itBAND;
        }

        double P3 = 0.0;
        if (b1 != BAND && b2 != BAND)
            P3 = this->P3(b1, b2);
        if (double_eq(P3, 0.0))
            P3 = this->obs_C(t_gband(b1, GOBSATTR::ATTR));

        //test for observations availability
        if (gnav == 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, " satellite " + satname + _epoch.str_ymdhms("  t_gallnav pointer is not available "));
            return -1;
        }

        //test for observations availability
        if (double_eq(P3, 0.0) && corrTOT)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, " satellite " + satname + _epoch.str_ymdhms(" P3 = 0!"));
            return -1;
        }

        double xyz[3] = {0.0, 0.0, 0.0};
        double vel[3] = {0.0, 0.0, 0.0};
        double var[3] = {0.0, 0.0, 0.0};
        double clk = 0.0;
        double dclk = 0.0;
        double clkrms = 0.0;

        if (satname.substr(0, 1) != "G" &&
            satname.substr(0, 1) != "R" &&
            satname.substr(0, 1) != "E" &&
            satname.substr(0, 1) != "J" &&
            satname.substr(0, 1) != "C")
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, " satelite " + satname + _epoch.str_ymdhms(" Undefined satellite system! "));
            return -1;
        }

        t_gtime epoT(t_gtime::GPS);
        double satclk = 0.0;
        double satclk2 = 1.0;
        int cnt = 0;

        if (corrTOT)
        {
            while (fabs(satclk - satclk2) > 1.e-3 / CLIGHT)
            {
                satclk2 = satclk;
                epoT = _epoch - P3 / CLIGHT - satclk;

                int irc = gnav->clk(satname, epoT, &clk, &clkrms, &dclk, msk_health);

                if (irc < 0 || cnt++ > 25)
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, " satelite " + satname + _epoch.str_ymdhms(" clocks not calculated (irc|iter) for epoch: "));
                    return -1;
                }
                satclk = clk;
            }
        }
        else
        {
            epoT = _epoch;
            int irc = gnav->clk(satname, epoT, &satclk, &clkrms, &dclk, msk_health);
            if (irc < 0)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, " satelite " + satname + _epoch.str_ymdhms(" clocks not calculated for epoch "));
                return -1;
            }
        }

        int irc = 0;
        irc = gnav->pos(satname, epoT, xyz, var, vel, msk_health);

        if (irc < 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, " satelite " + satname + _epoch.str_ymdhms(" coordinates not calculated for epoch "));
            return -1;
        }

        t_gtriple txyz(xyz);
        t_gtriple tvel(vel);

        // relativistic correction
        // WARNING: GLONASS clk already include the correction if broadcast eph are used !!!!!
        if (gs != GLO ||
            (gs == GLO && gnav->id_type() == t_gdata::ALLPREC))
        {
            double rel = 2.0 * (txyz[0] * vel[0] + txyz[1] * vel[1] + txyz[2] * vel[2]) / CLIGHT / CLIGHT; //default
            shared_ptr<t_geph> eph = gnav->find(satname, epoT);

            if (rel == 0.0)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_ERROR(_spdlog, " satelite " + satname + _epoch.str_ymdhms(" relativity correction not calculated for epoch "));
                return -1;
            }

            satclk -= rel;
            _TS = epoT;
            _reldelay = rel * CLIGHT;
        }

        // filling gsatdata

        _satcrd = txyz;
        _satvel = tvel;
        _clk = satclk * CLIGHT;
        _dclk = dclk * CLIGHT;

        return 1;
    }

    void t_gsatdata::_clear()
    {

        t_gobsgnss::_clear();
        _ele = 0.0;
        _azi_rec = 0.0;
        _rho = 0.0;
        _clk = 0.0;
    }

    bool t_gsatdata::_valid() const
    {

        if (_rho == 0.0 || // single validity identification for gsatdata!
            t_gobsgnss::_valid())
            return false;

        return true;
    }

    void t_gsatdata::setecl(const bool &ecl)
    {
        _eclipse = ecl;
    }

    const bool &t_gsatdata::ecl() const
    {

        return _eclipse;
    }

    void t_gsatdata::addecl(map<string, t_gtime> &lastEcl)
    {

        if (fabs(_b()) < EPS0_GPS && fabs(_orb_angle()) < EPS0_GPS)
        {
            _eclipse = true;
            lastEcl[_satid] = _epoch;
            return;
        }
        else
        {
            auto itLast = lastEcl.find(_satid);
            if (itLast != lastEcl.end())
            {
                double tdiff = _epoch.diff(itLast->second);
                if (abs(tdiff) <= POST_SHADOW)
                    _eclipse = true;
                else
                    _eclipse = false;
            }
        }
    }

    void t_gsatdata::addres(const RESIDTYPE &restype, const GOBSTYPE &type, const double &res)
    {
        _gmutex.lock();

        if (restype == RESIDTYPE::RES_ORIG)
        {
            if (type == TYPE_C)
                _code_res_orig.push_back(res);
            if (type == TYPE_L)
                _phase_res_orig.push_back(res);
        }

        if (restype == RESIDTYPE::RES_NORM)
        {
            if (type == TYPE_C)
                _code_res_norm.push_back(res);
            if (type == TYPE_L)
                _phase_res_norm.push_back(res);
        }

        _gmutex.unlock();
    }

    vector<double> t_gsatdata::residuals(const RESIDTYPE &restype, const GOBSTYPE &type)
    {
        _gmutex.lock();

        vector<double> res;

        if (restype == RESIDTYPE::RES_ORIG)
        {
            if (type == TYPE_C)
                res = _code_res_orig;
            else if (type == TYPE_L)
                res = _phase_res_orig;
        }

        if (restype == RESIDTYPE::RES_NORM)
        {
            if (type == TYPE_C)
                res = _code_res_norm;
            else if (type == TYPE_L)
                res = _phase_res_norm;
        }

        _gmutex.unlock();
        return res;
    }

    void t_gsatdata::clear_res(const RESIDTYPE &restype)
    {
        if (restype == RESIDTYPE::RES_ORIG)
        {
            _code_res_orig.clear();
            _phase_res_orig.clear();
        }

        if (restype == RESIDTYPE::RES_NORM)
        {
            _code_res_norm.clear();
            _phase_res_norm.clear();
        }
    }

    void t_gsatdata::addwind(const double &wind)
    {
        _gmutex.lock();

        _wind = wind;

        _gmutex.unlock();
    }

    const double &t_gsatdata::wind() const
    {
        return _wind;
    }

    double t_gsatdata::_b()
    {

        // test if already calculated
        if (!double_eq(_beta_val, 999))
            return _beta_val;

        if (_satcrd.zero())
            return 999;

        double beta = 0.0;
        double dt = 300;

        double dmjd = _epoch.dmjd();
        t_gephplan eph;
        ColumnVector Sun = eph.sunPos(dmjd, false).crd_cvect(); //ICRF
        double gmt = eph.gmst(dmjd);
        double gmt_dt = eph.gmst(dmjd + dt / 86400.0);

        ColumnVector Satcrd = _satcrd.crd_cvect();
        ColumnVector Satvel = _satvel.crd_cvect();
        ColumnVector Satcrd_dt = Satcrd + Satvel * dt; // 300s for extrapolation

        // prec. and nut. matrix should not change significantly in dt
        t_geop80 eop;
        Matrix prec = eop.precMatrix(dmjd);
        Matrix nut = eop.nutMatrix(dmjd);

        // ITRF -> ICRF
        Satcrd = prec.i() * nut.i() * rotZ(-gmt) * Satcrd;
        Satcrd_dt = prec.i() * nut.i() * rotZ(-gmt_dt) * Satcrd_dt;

        ColumnVector n = crossproduct(Satcrd, Satcrd_dt);

        n /= n.NormFrobenius();

        ColumnVector nSun = Sun / Sun.NormFrobenius();

        double cosa = DotProduct(nSun, n);

        beta = G_PI / 2.0 - acos(cosa);

        _beta_val = beta;

        return beta;
    }

    double t_gsatdata::_orb_angle()
    {

        // test if already calculated
        if (!double_eq(_orb_angle_val, 999))
            return _orb_angle_val;

        if (_satcrd.zero())
            return 999;

        double mi = 0.0;
        double dt = 30;

        double dmjd = _epoch.dmjd();
        t_gephplan eph;
        ColumnVector Sun = eph.sunPos(dmjd, false).crd_cvect(); //ICRF
        double gmt = eph.gmst(dmjd);
        double gmt_dt = eph.gmst(dmjd + dt / 86400.0);

        ColumnVector Satcrd = _satcrd.crd_cvect();
        ColumnVector Satvel = _satvel.crd_cvect();
        ColumnVector Satcrd_dt = Satcrd + Satvel * dt; // 30s for extrapolation

        // prec. and nut. matrix should not change significantly in dt
        t_geop80 eop;
        Matrix prec = eop.precMatrix(dmjd);
        Matrix nut = eop.nutMatrix(dmjd);

        // ITRF -> ICRF
        Satcrd = prec.i() * nut.i() * rotZ(-gmt) * Satcrd;
        Satcrd_dt = prec.i() * nut.i() * rotZ(-gmt_dt) * Satcrd_dt;

        ColumnVector n = crossproduct(Satcrd, Satcrd_dt);

        ColumnVector es = Satcrd / Satcrd.NormFrobenius();

        ColumnVector eSun = Sun / Sun.NormFrobenius();

        ColumnVector p = crossproduct(Sun, n);
        p /= p.NormFrobenius();

        double E = acos(DotProduct(es, p));
        double SunSat = acos(DotProduct(es, eSun));

        if (SunSat > G_PI / 2)
        {
            if (E <= G_PI / 2)
                mi = G_PI / 2 - E;
            else
                mi = G_PI / 2 - E;
        }
        else
        {
            if (E <= G_PI / 2)
                mi = G_PI / 2 + E;
            else
                mi = E - G_PI - G_PI / 2;
        }

        _orb_angle_val = mi;

        return mi;
    }

} // namespace
