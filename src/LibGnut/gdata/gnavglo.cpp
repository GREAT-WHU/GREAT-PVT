/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <math.h>

#include "gdata/gnavglo.h"
#include "gmodels/gephplan.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{
    t_gnavglo::t_gnavglo()
        : t_gnav(),
          _iodc(0),
          _toc(t_gtime::UTC)
    {
        id_type(t_gdata::EPHGLO);
        id_group(t_gdata::GRP_EPHEM);
        _maxEphAge = MAX_GLO_TIMEDIFF; // a bit above 900; //[s]
        _min_step = 10;
    }

    t_gnavglo::t_gnavglo(t_spdlog spdlog)
        : t_gnav(spdlog),
          _iodc(0),
          _toc(t_gtime::UTC)
    {

        id_type(t_gdata::EPHGLO);
        id_group(t_gdata::GRP_EPHEM);

        _maxEphAge = MAX_GLO_TIMEDIFF; // a bit above 900; //[s]
        _min_step = 10;
    }

    t_gnavglo::~t_gnavglo()
    {
    }

    int t_gnavglo::chk(set<string> &msg)
    {
        _gmutex.lock();

        _min_step = 225; // to speed up

        if (!_healthy())
        {
            msg.insert("Mesg: " + _sat + " unhealthy satellite " + _toc.str_ymdhms());
        }

        double clkB[1] = {0.0};
        double clkF[1] = {0.0};
        if (_clk(_toc - _maxEphAge, clkB) < 0 ||
            _clk(_toc + _maxEphAge, clkF) < 0 ||
            fabs(*clkF - *clkB) * CLK_GLO_FACTOR > MAX_GLO_CLKDIF)
        {
            msg.insert("Issue: " + _sat + " nav not correct [clk] " + _toc.str_ymdhms() + dbl2str(fabs(*clkF - *clkB) * CLK_GLO_FACTOR));
            _validity = false;
        }
        *clkB *= CLK_GLO_FACTOR;
        *clkF *= CLK_GLO_FACTOR;

        double xyzB[3] = {0.0, 0.0, 0.0};
        double xyzF[3] = {0.0, 0.0, 0.0};
        if (_pos(_toc - _maxEphAge, xyzB) < 0 ||
            _pos(_toc + _maxEphAge, xyzF) < 0)
        {
            msg.insert("Issue: " + _sat + " nav not correct [pos] " + _toc.str_ymdhms() + dbl2str(xyzF[0] - xyzB[0]));
            _validity = false;
        }

        double radB2 = xyzB[0] * xyzB[0] + xyzB[1] * xyzB[1] + xyzB[2] * xyzB[2];
        double radF2 = xyzF[0] * xyzF[0] + xyzF[1] * xyzF[1] + xyzF[2] * xyzF[2];
        if (radB2 < 0 || radF2 < 0)
        {
            msg.insert("Issue: " + _sat + " nav not correct [rad2] " + _toc.str_ymdhms());
            _validity = false;
        }

        double radB = sqrt(radB2) * RAD_GLO_FACTOR;
        double radF = sqrt(radF2) * RAD_GLO_FACTOR;
        if (radB < MIN_GLO_RADIUS || radB > MAX_GLO_RADIUS ||
            radF < MIN_GLO_RADIUS || radF > MAX_GLO_RADIUS)
        {
            msg.insert("Issue: " + _sat + " nav not correct [rad] " + _toc.str_ymdhms() + dbl2str(radF) + dbl2str(radB));
            _validity = false;
        }

        if (fabs(radF - radB) > MAX_GLO_RADDIF)
        {
            msg.insert("Issue: " + _sat + " nav not correct [rad-diff] " + _toc.str_ymdhms() + dbl2str(fabs(radF - radB)));
            _validity = false;
        }

        int sod_frac = _toc.sod() % 3600;
        if (sod_frac == 900 ||
            sod_frac == 2700)
        {
        }
        else
        {
            msg.insert("Issue: " + _sat + " nav unexpected [toc] " + _toc.str_ymdhms());
            _validity = false;
        }

        if (_freq_num > 240)
        { // 255 = -1, 254 = -2, 253 = -3 atd
            _freq_num -= 256;
            msg.insert("Warn: " + _sat + " corrected [channel] " + int2str(_freq_num + 256) + " -> " + int2str(_freq_num));
        }

        _gmutex.unlock();
        return 0;
    }

    /* --- */
    int t_gnavglo::channel() const
    {

        if (_freq_num < -7 || _freq_num > 13)
        {
            return 256;
        }

        return _freq_num;
    }

    int t_gnavglo::pos(const t_gtime &t, double xyz[], double var[], double vel[], bool chk_health)
    {

        _gmutex.lock();

        _min_step = 10;

        int i = _pos(t, xyz, var, vel, chk_health);

        _gmutex.unlock();
        return i;
    }

    int t_gnavglo::nav(const t_gtime &t, double xyz[], double var[], double vel[], bool chk_health)
    {
        _gmutex.lock();

        _min_step = 225;

        int i = _pos(t, xyz, var, vel, chk_health);

        _gmutex.unlock();
        return i;
    }

    int t_gnavglo::clk(const t_gtime &t, double *clk, double *var, double *dclk, bool chk_health)
    {
        _gmutex.lock();

        int i = _clk(t, clk, var, dclk, chk_health);

        _gmutex.unlock();
        return i;
    }

    int t_gnavglo::_pos(const t_gtime &t, double xyz[], double var[], double vel[], bool chk_health)
    {

        if (sat().empty())
            return -1; // not valid !!!
        if (chk_health && _healthy() == false)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "not healthy sat " + sat() + " excluded from pos calculation " + t.str_ymdhms());
            return -1; // HEALTH NOT OK
        }

        xyz[0] = xyz[1] = xyz[2] = 0.0;
        if (var)
            var[0] = var[1] = var[2] = 0.0;
        if (vel)
            vel[0] = vel[1] = vel[2] = 0.0;

        double Tk = t.diff(_toc); // T - toc difference

        if (fabs(Tk) > _maxEphAge)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "CRD " + _sat + ": The user time and GLONASS ephemerides epoch differ too much. TOC: " + _toc.str_ymdhms() + " T: " + t.str_ymdhms());
            return -1;
        }

        // init state vector (crd, vel)
        ColumnVector yy(6);

        if (double_eq(_x, 0.0) || double_eq(_y, 0.0) || double_eq(_z, 0.0))
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Zero ephemerides:" + t.str_ymdhms(sat() + " "));
            return -1;
        }

        yy << _x << _y << _z << _x_d << _y_d << _z_d;
        // acceleration
        t_gtriple acc(_x_dd, _y_dd, _z_dd);

        int nsteps = (int)fabs(floor(Tk / _min_step + 0.5)); // step number for RungeKutta
        double step = fabs(Tk) / nsteps;                     // step length for RungeKutta, length is cca 10s
                                                             //  cout << Tk << " " << nsteps << " " << step << endl;
        if (Tk < 0)
            step = -step;
        ColumnVector yy_integr(6);
        yy_integr = _RungeKutta(step, nsteps, yy, acc);

        xyz[0] = yy_integr(1);
        xyz[1] = yy_integr(2);
        xyz[2] = yy_integr(3);

        // PZ_90.11 to ITRF_2008 transformation
        ColumnVector T(3);
        T << -0.003 << -0.001 << 0.000;
        Matrix R(3, 3);
        R = 0;
        double mas = (1 / (36e5)) * D2R;
        R << 1 << 0.002 * mas << 0.042 * mas
          << -0.002 * mas << 1 << 0.019 * mas
          << -0.042 * mas << -0.019 * mas << 1;
        ColumnVector xyzWGS;
        xyzWGS = T + R * yy_integr.Rows(1, 3);

        // position at time t
        xyz[0] = xyzWGS(1);
        xyz[1] = xyzWGS(2);
        xyz[2] = xyzWGS(3);

        // velocity at positon t
        if (vel)
        {
            vel[0] = yy_integr(4);
            vel[1] = yy_integr(5);
            vel[2] = yy_integr(6);
        }

        return 0;
    }

    int t_gnavglo::_clk(const t_gtime &t, double *clk, double *var, double *dclk, bool chk_health)
    {

        if (sat().empty())
            return -1; // not valid !!!
        if (chk_health && _healthy() == false)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "not healthy sat " + sat() + " excluded from clk calculation " + t.str_ymdhms());
            return -1; // HEALTH NOT OK
        }
        double Tk = t.diff(_toc); // T - toc difference

        if (fabs(Tk) > _maxEphAge)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "CLK " + _sat + ": The user time and GLONASS ephemerides epoch differs too much. TOC: " + _toc.str_ymdhms() + " T: " + t.str_ymdhms());
            return -1;
        }

        for (int i = 0; i < 2; i++)
        {
            Tk -= -_tau + _gamma * Tk;
        }

        *clk = -_tau + _gamma * Tk;

        return 0;
    }

    int t_gnavglo::data2nav(string sat, const t_gtime &ep, const t_gnavdata &data)
    {
        _gmutex.lock();

        if (sat.find("R") != string::npos)
        {
            _sat = sat;
        }
        else
        {
            ostringstream tmp;
            tmp << setw(1) << 'R' << setfill('0') << setw(2) << sat;
            _sat = tmp.str();
        }
        _epoch = ep;
        _toc = ep;
        _iodc = _iod();
        _tau = -data[0]; // in RINEX is stored -tauN
        _gamma = data[1];
        _tki = data[2];
        if (_tki < 0)
            _tki += 86400;

        _x = data[3] * 1.e3;
        _x_d = data[4] * 1.e3;
        _x_dd = data[5] * 1.e3;

        _health = data[6];

        _y = data[7] * 1.e3;
        _y_d = data[8] * 1.e3;
        _y_dd = data[9] * 1.e3;

        _freq_num = (int)data[10];

        _z = data[11] * 1.e3;
        _z_d = data[12] * 1.e3;
        _z_dd = data[13] * 1.e3;

        _E = data[14];

        _gmutex.unlock();
        return 0;
    }

    int t_gnavglo::nav2data(t_gnavdata &data)
    {
        _gmutex.lock();

        if (!this->_valid())
            return -1;

        data[0] = -_tau; // in RINEX is stored -tauN
        data[1] = _gamma;
        data[2] = _tki; //  if (_tki < 0) _tki += 86400;

        data[3] = _x / 1.e3;
        data[4] = _x_d / 1.e3;
        data[5] = _x_dd / 1.e3;

        data[6] = _health;

        data[7] = _y / 1.e3;
        data[8] = _y_d / 1.e3;
        data[9] = _y_dd / 1.e3;

        data[10] = _freq_num;

        data[11] = _z / 1.e3;
        data[12] = _z_d / 1.e3;
        data[13] = _z_dd / 1.e3;

        data[14] = _E;

        _gmutex.unlock();
        return 0;
    }

    t_timdbl t_gnavglo::param(const NAVDATA &n)
    {
        _gmutex.lock();

        t_timdbl tmp;
        switch (n)
        {
        case NAV_X:
            tmp = make_pair(_toc, _x * 1e0);
            break; // meters
        case NAV_XD:
            tmp = make_pair(_toc, _x_d * 1e0);
            break; // meters
        case NAV_XDD:
            tmp = make_pair(_toc, _x_dd * 1e0);
            break; // meters
        case NAV_Y:
            tmp = make_pair(_toc, _y * 1e0);
            break; // meters
        case NAV_YD:
            tmp = make_pair(_toc, _y_d * 1e0);
            break; // meters
        case NAV_YDD:
            tmp = make_pair(_toc, _y_dd * 1e0);
            break; // meters
        case NAV_Z:
            tmp = make_pair(_toc, _z * 1e0);
            break; // meters
        case NAV_ZD:
            tmp = make_pair(_toc, _z_d * 1e0);
            break; // meters
        case NAV_ZDD:
            tmp = make_pair(_toc, _z_dd * 1e0);
            break; // meters

        case NAV_IOD:
            tmp = make_pair(_toc, _iodc * 1e0);
            break; //
        case NAV_HEALTH:
            tmp = make_pair(_toc, _health * 1e0);
            break; //

        default:
            break;
        }

        _gmutex.unlock();
        return tmp;
    }

    int t_gnavglo::param(const NAVDATA &n, double val)
    {
        _gmutex.lock();

        switch (n)
        { // SELECTED only, ! use the same MULTIPLICATOR as in param()

        case NAV_IOD:
            _iodc = val / 1.e0;
            break;
        case NAV_HEALTH:
            _health = val / 1.e0;
            break;

        default:
            break;
        }

        _gmutex.unlock();
        return 0;
    }

    string t_gnavglo::line() const
    {

        int w = 20;
        ostringstream tmp;

        tmp << " " << setw(3) << sat()
            << " " << _toc.str("%Y-%m-%d %H:%M:%S")
            << scientific << setprecision(12)
            << setw(w) << _tau
            << setw(w) << _gamma
            << setw(w) << _tki
            << setw(w) << _x
            << setw(w) << _x_d
            << setw(w) << _x_dd
            << setw(w) << _health
            << setw(w) << _y
            << setw(w) << _y_d
            << setw(w) << _y_dd
            << setw(w) << _freq_num
            << setw(w) << _z
            << setw(w) << _z_d
            << setw(w) << _z_dd
            << setw(w) << _E;

        return tmp.str();
    }

    string t_gnavglo::linefmt() const
    {

        ostringstream tmp;

        tmp << " " << setw(3) << sat() << fixed
            << " " << _toc.str("%Y-%m-%d %H:%M:%S")
            //     << " " << _toe.str("%Y-%m-%d %H:%M:%S")
            //     << " " << _tot.str("%Y-%m-%d %H:%M:%S")
            << fixed << setprecision(0)

            << setw(8) << _E
            << setw(8) << _freq_num
            << setw(4) << _iodc
            << setw(4) << _health
            << " |"
            << setw(12) << setprecision(3) << _tau * 1e9  //   [sec]
            << setw(8) << setprecision(3) << _gamma * 1e9 //   [sec]
            << setw(8) << setprecision(0) << _tki * 1e0   //
            << " |"
            << setw(14) << setprecision(3) << _x * 1e0   // 1 [km]
            << setw(11) << setprecision(3) << _x_d * 1e0 // 2 [km/s]
            << setw(9) << setprecision(3) << _x_dd * 1e6 // 3 [km/s^2]
            << " |"
            << setw(14) << setprecision(3) << _y * 1e0   // 1 [km]
            << setw(11) << setprecision(3) << _y_d * 1e0 // 2 [km/s]
            << setw(9) << setprecision(3) << _y_dd * 1e6 // 3 [km/s^2]
            << " |"
            << setw(14) << setprecision(3) << _z * 1e0   // 1 [km]
            << setw(11) << setprecision(3) << _z_d * 1e0 // 2 [km/s]
            << setw(9) << setprecision(3) << _z_dd * 1e6 // 3 [km/s^2]
            ;

        return tmp.str();
    }

    bool t_gnavglo::_healthy() const
    {
        if (_health == 0)
            return true;
        return false;
    }

    ColumnVector t_gnavglo::_deriv(const ColumnVector &xx, const t_gtriple &acc)
    {

        t_gtriple crd(xx.rows(1, 3));
        t_gtriple vel(xx.rows(4, 6));

        double r = crd.crd_cvect().norm_Frobenius();

        double k1 = -GM_PZ90 / (r * r * r);
        double k2 = (3.0 / 2.0) * C20_PZ90 * (GM_PZ90 * Aell_PZ90 * Aell_PZ90) / pow(r, 5);

        ColumnVector xxdot(6);

        xxdot(1) = vel[0];
        xxdot(2) = vel[1];
        xxdot(3) = vel[2];
        xxdot(4) = k1 * crd[0] + k2 * (1.0 - 5.0 * crd[2] * crd[2] / (r * r)) * crd[0] + OMEGA * OMEGA * crd[0] + 2 * OMEGA * vel[1] + acc[0];
        xxdot(5) = k1 * crd[1] + k2 * (1.0 - 5.0 * crd[2] * crd[2] / (r * r)) * crd[1] + OMEGA * OMEGA * crd[1] - 2 * OMEGA * vel[0] + acc[1];
        xxdot(6) = k1 * crd[2] + k2 * (3.0 - 5.0 * crd[2] * crd[2] / (r * r)) * crd[2] + acc[2];

        return xxdot;
    }

    ColumnVector t_gnavglo::_RungeKutta(double step, int nsteps, const ColumnVector &yy, const t_gtriple &acc)
    {

        ColumnVector yyn = yy;

        for (int i = 1; i <= nsteps; i++)
        {
            ColumnVector k1 = step * _deriv(yyn, acc);
            ColumnVector k2 = step * _deriv(yyn + k1 / 2.0, acc);
            ColumnVector k3 = step * _deriv(yyn + k2 / 2.0, acc);
            ColumnVector k4 = step * _deriv(yyn + k3, acc);

            yyn += k1 / 6.0 + k2 / 3.0 + k3 / 3.0 + k4 / 6.0;
        }

        return yyn;
    }

    int t_gnavglo::_iod() const
    {

        t_gtime gloTime = _toc + 3 * 3600.0; // toc in GLO (if toc in UTC +3*3600.0;)

        int iod = int(gloTime.sod() / 900);

        // doy is added for making iod unique over days
        int doy = _toc.doy();
        iod += doy;

        return iod;
    }

} // namespace
