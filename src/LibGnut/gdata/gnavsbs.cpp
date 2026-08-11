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

#include "gdata/gnavsbs.h"
#include "gmodels/gephplan.h"
#include "gutils/gtypeconv.h"


namespace gnut
{

    t_gnavsbs::t_gnavsbs() :
        t_gnav(),
        _toc(t_gtime::t_tsys::GPS)
    {
        id_type(t_gdata::ID_TYPE::EPHSBS);
        id_group(t_gdata::ID_GROUP::GRP_EPHEM);
    }

    t_gnavsbs::~t_gnavsbs()
    {
    }

    int t_gnavsbs::chk(std::set<std::string>& msg)
    {
        _gmutex.lock();

        if (!_healthy())
        {
            msg.insert("Issue: " + _sat + " nav unhealthy satellite " + _toc.str_ymdhms());
            _validity = false;
        }

        if (_tki >= 9.9999e8)
        {
            msg.insert("Issue: " + _sat + " nav too large value [tki] " + dbl2str(_tki));
            _validity = false;
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gnavsbs::pos(const t_gtime& t, double xyz[], double var[], double vel[], bool chk_health)
    {
        if (sat() == GSAT::NONE)
        {
            return -1; // not valid !!!
        }
        if (chk_health && _healthy() == false)
        {
            GREAT_DEBUG("not healthy sat " + sat() + " excluded from pos calculation " + t.str_ymdhms());
            return -1; // HEALTH NOT OK
        }

        xyz[0] = xyz[1] = xyz[2] = 0.0;
        if (var)
        {
            var[0] = var[1] = var[2] = 0.0;
        }
        if (vel)
        {
            vel[0] = vel[1] = vel[2] = 0.0;
        }

        _gmutex.lock();

        double Tk = t.diff(_toc); // T - toe difference

        if (fabs(Tk) > MAX_SBS_TIMEDIFF)
        {
            GREAT_DEBUG("CRD " + _sat + ": The user time and Glonass ephemerides epoch differ too much. TOE: " + _toc.str_ymdhms() +
                        " T: " + t.str_ymdhms());
            _gmutex.unlock();
            return -1;
        }

        if (double_eq(_x, 0.0) || double_eq(_y, 0.0) || double_eq(_z, 0.0))
        {
            _gmutex.unlock();
            return -1;
        }

        xyz[0] = _x + _x_d * Tk + _x_dd * Tk * Tk / 2.0;
        xyz[1] = _y + _y_d * Tk + _y_dd * Tk * Tk / 2.0;
        xyz[2] = _z + _z_d * Tk + _z_dd * Tk * Tk / 2.0;

        // velocity at positon t
        if (vel)
        {
            vel[0] = _x_dd;
            vel[1] = _y_dd;
            vel[2] = _z_dd;
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gnavsbs::clk(const t_gtime& t, double* clk, double* var, double* dclk, bool chk_health)
    {
        if (sat() == GSAT::NONE)
        {
            return -1; // not valid !!!
        }

        if (chk_health && _healthy() == false)
        {
            GREAT_DEBUG("not healthy sat " + sat() + " excluded from clk calculation " + t.str_ymdhms());
            _gmutex.unlock();
            return -1; // HEALTH NOT OK
        }

        _gmutex.lock();

        double Tk = t.diff(_toc); // T - toe difference

        if (fabs(Tk) > MAX_SBS_TIMEDIFF)
        {
            GREAT_DEBUG("CLK " + _sat + ": The user time and SBAS ephemerides epoch differs too much. TOE: " + _toc.str_ymdhms() +
                        " T: " + t.str_ymdhms());
            _gmutex.unlock();
            return -1;
        }

        for (int i = 0; i < 2; i++)
        {
            Tk -= _f0 + _f1 * Tk;
        }

        *clk = _f0 + _f1 * Tk;

        _gmutex.unlock();
        return 0;
    }

    int t_gnavsbs::data2nav(std::string sat, const t_gtime& ep, const t_gnavdata& data)
    {
        _gmutex.lock();

        _sat = str2gsat(sat, GSYS::SBS);

        _epoch = ep;
        _toc = ep;
        _f0 = data[0];
        _f1 = data[1];
        _tki = data[2];
        if (_tki < 0)
        {
            _tki += 86400;
        }

        _x = data[3] * 1.e3;
        _x_d = data[4] * 1.e3;
        _x_dd = data[5] * 1.e3;

        _health = data[6];

        _y = data[7] * 1.e3;
        _y_d = data[8] * 1.e3;
        _y_dd = data[9] * 1.e3;

        _C_rms = data[10];

        _z = data[11] * 1.e3;
        _z_d = data[12] * 1.e3;
        _z_dd = data[13] * 1.e3;

        _iod = data[14];

        _gmutex.unlock();
        return 0;
    }

    int t_gnavsbs::nav2data(t_gnavdata& data)
    {
        _gmutex.lock();

        data[0] = _f0;
        data[1] = _f1;
        data[2] = _tki; //  if (_tki < 0) _tki += 86400;

        data[3] = _x / 1.e3;
        data[4] = _x_d / 1.e3;
        data[5] = _x_dd / 1.e3;

        data[6] = _health;

        data[7] = _y / 1.e3;
        data[8] = _y_d / 1.e3;
        data[9] = _y_dd / 1.e3;

        data[10] = _C_rms;

        data[11] = _z / 1.e3;
        data[12] = _z_d / 1.e3;
        data[13] = _z_dd / 1.e3;

        data[14] = _iod;

        _gmutex.unlock();
        return 0;
    }

    t_timdbl t_gnavsbs::param(const NAVDATA& n)
    {
        _gmutex.lock();

        t_timdbl tmp;
        switch (n)
        {
            case NAVDATA::NAV_X:
                tmp = std::make_pair(_toc, _x * 1e0);
                break; // meters
            case NAVDATA::NAV_XD:
                tmp = std::make_pair(_toc, _x_d * 1e0);
                break; // meters
            case NAVDATA::NAV_XDD:
                tmp = std::make_pair(_toc, _x_dd * 1e0);
                break; // meters
            case NAVDATA::NAV_Y:
                tmp = std::make_pair(_toc, _y * 1e0);
                break; // meters
            case NAVDATA::NAV_YD:
                tmp = std::make_pair(_toc, _y_d * 1e0);
                break; // meters
            case NAVDATA::NAV_YDD:
                tmp = std::make_pair(_toc, _y_dd * 1e0);
                break; // meters
            case NAVDATA::NAV_Z:
                tmp = std::make_pair(_toc, _z * 1e0);
                break; // meters
            case NAVDATA::NAV_ZD:
                tmp = std::make_pair(_toc, _z_d * 1e0);
                break; // meters
            case NAVDATA::NAV_ZDD:
                tmp = std::make_pair(_toc, _z_dd * 1e0);
                break; // meters

            case NAVDATA::NAV_IOD:
                tmp = std::make_pair(_toc, _iod * 1e0);
                break; //
            case NAVDATA::NAV_HEALTH:
                tmp = std::make_pair(_toc, _health * 1e0);
                break; //

            default:
                break;
        }

        _gmutex.unlock();
        return tmp;
    }

    int t_gnavsbs::param(const NAVDATA& n, double val)
    {
        _gmutex.lock();

        switch (n)
        { // SELECTED only, ! use the same MULTIPLICATOR as in param()

            case NAVDATA::NAV_IOD:
                _iod = val / 1.e0;
                break;
            case NAVDATA::NAV_HEALTH:
                _health = val / 1.e0;
                break;

            default:
                break;
        }

        _gmutex.unlock();
        return 0;
    }

    std::string t_gnavsbs::line() const
    {
        int w = 20;
        std::ostringstream tmp;

        tmp << " " << std::setw(3) << sat() << " " << _toc.str("%Y-%m-%d %H:%M:%S") << std::scientific << std::setprecision(12) << std::setw(w) << _f0
            << std::setw(w) << _f1 << std::setw(w) << _tki << std::setw(w) << _x << std::setw(w) << _x_d << std::setw(w) << _x_dd << std::setw(w) << _health << std::setw(w)
            << _y << std::setw(w) << _y_d << std::setw(w) << _y_dd << std::setw(w) << _C_rms << std::setw(w) << _z << std::setw(w) << _z_d << std::setw(w) << _z_dd
            << std::setw(w) << _iod;

        return tmp.str();
    }

    std::string t_gnavsbs::linefmt() const
    {
        std::ostringstream tmp;

        tmp << " " << std::setw(3) << sat() << std::fixed << " " << _toc.str("%Y-%m-%d %H:%M:%S") << std::fixed << std::setprecision(0)

            << std::setw(8) << 0 << std::setw(8) << _C_rms << std::setw(4) << _iod << std::setw(4) << _health << " |" << std::setw(12) << std::setprecision(3)
            << _f0 * 1e9                                       //   [sec]
            << std::setw(8) << std::setprecision(3) << _f1 * 1e9         //   [sec]
            << std::setw(8) << std::setprecision(0) << _tki * 1e0        //
            << " |" << std::setw(14) << std::setprecision(3) << _x * 1e0 // 1 [km]
            << std::setw(11) << std::setprecision(3) << _x_d * 1e0       // 2 [km/s]
            << std::setw(9) << std::setprecision(3) << _x_dd * 1e6       // 3 [km/s^2]
            << " |" << std::setw(14) << std::setprecision(3) << _y * 1e0 // 1 [km]
            << std::setw(11) << std::setprecision(3) << _y_d * 1e0       // 2 [km/s]
            << std::setw(9) << std::setprecision(3) << _y_dd * 1e6       // 3 [km/s^2]
            << " |" << std::setw(14) << std::setprecision(3) << _z * 1e0 // 1 [km]
            << std::setw(11) << std::setprecision(3) << _z_d * 1e0       // 2 [km/s]
            << std::setw(9) << std::setprecision(3) << _z_dd * 1e6       // 3 [km/s^2]
            ;

        return tmp.str();
    }

    bool t_gnavsbs::_healthy() const
    {
        if (_health == 0)
        {
            return true;
        }
        return false;
    }

} // namespace gnut
