/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <bitset>

#include "gdata/gnavgal.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{

    t_gnavgal::t_gnavgal()
        : t_gnav(),
          _iode(0),
          _health(0),
          _toe(t_gtime::GAL),
          _toc(t_gtime::GAL),
          _tot(t_gtime::GAL),
          _a(0.0),
          _e(0.0),
          _m(0.0),
          _i(0.0),
          _idot(0.0),
          _omega(0.0),
          _OMG(0.0),
          _OMGDOT(0.0),
          _dn(0.0),
          _crc(0.0),
          _cic(0.0),
          _cuc(0.0),
          _crs(0.0),
          _cis(0.0),
          _cus(0.0),
          _f0(0.0),
          _f1(0.0),
          _f2(0.0),
          _source(0),
          _sisa(0.0)
    {
        _tgd[0] = _tgd[1] = 0.0;
        id_type(t_gdata::EPHGAL);
        id_group(t_gdata::GRP_EPHEM);
    }

    t_gnavgal::t_gnavgal(t_spdlog spdlog)
        : t_gnav(spdlog),
          _iode(0),
          _health(0),
          _toe(t_gtime::GAL),
          _toc(t_gtime::GAL),
          _tot(t_gtime::GAL),
          _a(0.0),
          _e(0.0),
          _m(0.0),
          _i(0.0),
          _idot(0.0),
          _omega(0.0),
          _OMG(0.0),
          _OMGDOT(0.0),
          _dn(0.0),
          _crc(0.0),
          _cic(0.0),
          _cuc(0.0),
          _crs(0.0),
          _cis(0.0),
          _cus(0.0),
          _f0(0.0),
          _f1(0.0),
          _f2(0.0),
          _source(0),
          _sisa(0.0)
    {
        _tgd[0] = _tgd[1] = 0.0;

        id_type(t_gdata::EPHGAL);
        id_group(t_gdata::GRP_EPHEM);
    }

    t_gnavgal::~t_gnavgal()
    {
    }

    string t_gnavgal::line() const
    {
        int w = 20;
        ostringstream tmp;

        tmp << " " << setw(3) << sat()
            << " " << _toc.str("%Y-%m-%d %H:%M:%S")
            << scientific << setprecision(12)
            << setw(w) << _f0
            << setw(w) << _f1
            << setw(w) << _f2
            << setw(w) << _iode / 1.0
            << setw(w) << _health / 1.0
            << setw(w) << SQRT(_a)
            << setw(w) << _e
            << setw(w) << _i
            << setw(w) << _m
            << setw(w) << _idot
            << setw(w) << _omega
            << setw(w) << _OMG
            << setw(w) << _OMGDOT
            << setw(w) << _dn
            << setw(w) << _crc
            << setw(w) << _cic
            << setw(w) << _cuc
            << setw(w) << _crs
            << setw(w) << _cis
            << setw(w) << _cus
            << setw(w) << _tgd[0] // E5a/E1
            << setw(w) << _tgd[1] // E5b/E1
            << endl;

        return tmp.str();
    }

    string t_gnavgal::linefmt() const
    {

        ostringstream tmp;

        string source_bin = _source_str();
        string health_bin = _health_str();

        tmp << " " << setw(3) << sat() << fixed
            << " " << _toc.str("%Y-%m-%d %H:%M:%S")
            << fixed << setprecision(0)
            << setw(8) << _tot - _toc
            << setw(8) << _tot - _toe
            << setw(4) << _iode
            << setw(4) << _health
            << setw(4) << _source
            << " |"
            << setw(9) << setprecision(0) << _a * 1e0 // 1 [m]
            << setw(8) << setprecision(3) << _e * 1e3 // 2
            << setw(8) << setprecision(3) << _i * 1e3 // 3
            << setw(7) << setprecision(3) << _m * 1e0 // 4
            << " |"
            << setw(7) << setprecision(3) << _idot * 1e9   // 5
            << setw(9) << setprecision(5) << _omega * 1e0  // 6
            << setw(9) << setprecision(5) << _OMG * 1e0    // 7
            << setw(9) << setprecision(5) << _OMGDOT * 1e9 // 8
            << setw(7) << setprecision(3) << _dn * 1e9     // 9
            << " |"
            << setw(9) << setprecision(3) << _crc * 1e0 // 10
            << setw(7) << setprecision(3) << _cic * 1e6 // 11
            << setw(7) << setprecision(3) << _cuc * 1e6 // 12
            << setw(9) << setprecision(3) << _crs * 1e0 // 13
            << setw(7) << setprecision(3) << _cis * 1e6 // 14
            << setw(7) << setprecision(3) << _cus * 1e6 // 15
            << " |"
            << setw(9) << setprecision(3) << _tgd[0] * 1e9 // E5a/E1
            << setw(9) << setprecision(3) << _tgd[1] * 1e9 // E5b/E1
            << " |"
            << setw(11) << health_bin
            << setw(11) << source_bin
            << setw(4) << src(true)
            << setw(9) << gnavtype2str(_gnavtype(true));

        return tmp.str();
    }

    int t_gnavgal::chk(set<string> &msg)
    {

        _gmutex.lock();

        string binary = _source_str();

        if (binary.compare(0, 3, "000") == 0 || binary.compare(0, 3, "111") == 0)
        {
            msg.insert("Issue: " + _sat + " nav source not valid [" + binary + "]");
            _validity = false;
        }

        if (!_healthy())
        {
            msg.insert("Mesg: " + _sat + " nav unhealthy satellite " + _toc.str_ymdhms());
        }

        if ((_tot - _toc) > 86400 || (_toc - _tot) > t_gnav::nav_validity(GAL))
        {
            msg.insert("Issue: " + _sat + " nav large difference [tot-toc] " + dbl2str(_tot - _toc) + " s " + _tot.str_ymdhms() + _toc.str_ymdhms(" "));
            _validity = false;
        }

        if ((_tot - _toe) > 86400 || (_toe - _tot) > t_gnav::nav_validity(GAL))
        {
            msg.insert("Issue: " + _sat + " nav large difference [tot-toe] " + dbl2str(_tot - _toe) + " s " + _tot.str_ymdhms() + _toe.str_ymdhms(" "));
            _validity = false;
        }

        if (_toe == FIRST_TIME)
        {
            msg.insert("Issue: " + _sat + " nav invalid [toe] " + _toe.str_ymdhms());
            _validity = false;
        }

        int sod_frac = _toc.sod() % 600;
        if (sod_frac == 0)
        {
        }
        else
        {
            msg.insert("Issue: " + _sat + " nav unexpected [toc] " + _toc.str_ymdhms());
            _validity = false;
        }

        if (_iode < 0 || 1023 < _iode)
        {
            msg.insert("Issue: " + _sat + " nav invalid [iode] " + _toe.str_ymdhms() + " " + int2str(_iode));
            _validity = false;
        }

        if (_a < 20000000.0)
        {
            msg.insert("Issue: " + _sat + " nav invalid [a] " + _toe.str_ymdhms() + " " + dbl2str(_a));
            _validity = false;
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gnavgal::pos(const t_gtime &t, double xyz[], double var[], double vel[], bool chk_health)
    {

        if (sat().empty())
            return -1; // check if valid
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

        _gmutex.lock();

        double Tk = t.diff(_toe); // T - toe difference
        while (Tk > 302400.0)
        {
            Tk -= 604800.0;
        } // check the correct GAL week
        while (Tk < -302400.0)
        {
            Tk += 604800.0;
        } // check the correct GAL week

        double Ek, dEk;
        _ecc_anomaly(Tk, Ek, dEk); // eccentric anomaly and its derivative

        double V = atan2(sqrt(1.0 - pow(_e, 2.0)) * sin(Ek),
                         cos(Ek) - _e); // true anomaly
        double U0 = V + _omega;         // argument of latitude

        double sin2U = sin(2 * U0);
        double cos2U = cos(2 * U0);

        double Rk = _a * (1.0 - _e * cos(Ek)); // radius
        double Uk = U0;                        // argument of latitude
        double Ik = _i + _idot * Tk;           // inclination

        Rk += _crs * sin2U + _crc * cos2U; // corrected radius
        Ik += _cis * sin2U + _cic * cos2U; // corrected inclination
        Uk += _cus * sin2U + _cuc * cos2U; // corrected argument of latitude

        double cosU = cos(Uk);
        double sinU = sin(Uk);
        double cosI = cos(Ik);
        double sinI = sin(Ik);

        double x = Rk * cosU; // position in orbital plane
        double y = Rk * sinU;

        double OMG = _OMG + (_OMGDOT - OMGE_DOT_GAL) * Tk - OMGE_DOT_GAL * _toe.sow(); // corrected long. of asc. node

        double cosOMG = cos(OMG);
        double sinOMG = sin(OMG);

        xyz[0] = x * cosOMG - y * cosI * sinOMG; // earth-fixed coordinates [m]
        xyz[1] = x * sinOMG + y * cosI * cosOMG;
        xyz[2] = y * sinI;

        // error variance
        if (var)
        {
            *var = SQRT(_sisa);
        }

        // velocities
        if (vel)
        {
            double n = _mean_motion();

            double tanv2 = tan(V / 2);
            double dEdM = 1 / (1 - _e * cos(Ek));
            double dotv = sqrt((1.0 + _e) / (1.0 - _e)) / cos(Ek / 2) / cos(Ek / 2) / (1 + tanv2 * tanv2) * dEdM * n;
            double dotu = dotv + (-_cuc * sin2U + _cus * cos2U) * 2 * dotv;
            double dotom = _OMGDOT - OMGE_DOT_GAL;
            double doti = _idot + (-_cic * sin2U + _cis * cos2U) * 2 * dotv;
            double dotr = _a * _e * sin(Ek) * dEdM * n + (-_crc * sin2U + _crs * cos2U) * 2 * dotv;
            double dotx = dotr * cosU - Rk * sinU * dotu;
            double doty = dotr * sinU + Rk * cosU * dotu;

            vel[0] = cosOMG * dotx - cosI * sinOMG * doty - x * sinOMG * dotom - y * cosI * cosOMG * dotom + y * sinI * sinOMG * doti;

            vel[1] = sinOMG * dotx + cosI * cosOMG * doty + x * cosOMG * dotom - y * cosI * sinOMG * dotom - y * sinI * cosOMG * doti;

            vel[2] = sinI * doty + y * cosI * doti;
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gnavgal::clk(const t_gtime &t, double *clk, double *var, double *dclk, bool chk_health)
    {

        if (sat().empty())
            return -1; // not valid !!!
        if (chk_health && _healthy() == false)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "not healthy sat " + sat() + " excluded from clk calculation " + t.str_ymdhms());
            return -1; // HEALTH NOT OK
        }

        _gmutex.lock();

        double Tk = t.diff(_toc); // difference from clk(!) epoch ref. time
        while (Tk > 302400.0)
        {
            Tk -= 604800.0;
        } // check the correct GAL week
        while (Tk < -302400.0)
        {
            Tk += 604800.0;
        } // check the correct GAL week

        double Ek, dEk;
        _ecc_anomaly(Tk, Ek, dEk); // eccentric anomaly

        // NOT CORRECTED FOR 2nd-order relativistic effect - at user side
        _rel = 4.442807309e-10 * sqrt(_a) * _e * sin(Ek);
        *clk = _f0 + Tk * _f1 + Tk * Tk * _f2;

        // check impossible clock correction
        if (clk && *clk > 1.0)
        {
            cerr << "* warning: too large clock - skipped: " << fixed << setprecision(0) << *clk << endl;
            _gmutex.unlock();
            return -1;
        }

        // error variance
        if (var)
        {
            *var = SQRT(_sisa);
        }

        // velocities
        if (dclk)
        {
            *dclk = 0.0;
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gnavgal::data2nav(string sat, const t_gtime &ep, const t_gnavdata &data)
    {
        _gmutex.lock();

        char tmp[12];

        if (sat.substr(0, 1) == "E")
            _sat = sat;
        else
            _sat = "E" + sat;

        _toc = ep; // time of clocks (tot) .. RINEX reference time
        _epoch = ep;

        int data_21 = static_cast<int>(data[21]); // toe week
        int data_11 = static_cast<int>(data[11]); // toe sow
        int data_27 = static_cast<int>(data[27]); // tot sow

        if (data_21 < 0)
        {
            _toe = FIRST_TIME; 
            _gmutex.unlock();
            return -1;
        }

        sprintf(tmp, "%4i %6i", data_21, data_11);
        _toe.from_str("%W %v", tmp); // time of ephemeris    (toe)
        if (data_27 == 0.9999e9 || data_27 == 0)
            _tot = _toe;
        else
        {
            sprintf(tmp, "%4i %6i", data_21, data_27);
            _tot.from_str("%W %v", tmp);
        } // time of transmission (tot)

        if (fabs(_tot - _toe - 604800) < MAX_GAL_TIMEDIFF)
            _tot.add_secs(-604800); // adjust tot to toe gwk
        if (fabs(_tot - _toe + 604800) < MAX_GAL_TIMEDIFF)
            _tot.add_secs(+604800); // adjust tot to toe gwk

        _f0 = data[0];
        _f1 = data[1];
        _f2 = data[2];

        _iode = static_cast<int>(data[3]);
        _health = static_cast<int>(data[24]);

        _a = SQR(data[10]);
        _e = data[8];
        _m = data[6];
        _i = data[15];
        _idot = data[19];
        _OMG = data[13];
        _OMGDOT = data[18];
        _omega = data[17];
        _dn = data[5];

        _crs = data[4];
        _cus = data[9];
        _cis = data[14];
        _crc = data[16];
        _cuc = data[7];
        _cic = data[12];

        _tgd[0] = data[25]; // [s] E5a/E1
        _tgd[1] = data[26]; // [s] E5b/E1
        _source = (int)data[20];
        _sisa = data[23];

        _gmutex.unlock();
        return 0;
    }

    int t_gnavgal::nav2data(t_gnavdata &data)
    {
        _gmutex.lock();

        if (!this->_valid())
            return -1;

        data[0] = _f0;
        data[1] = _f1;
        data[2] = _f2;
        data[3] = _iode;
        data[4] = _crs;
        data[5] = _dn;
        data[6] = _m;
        data[7] = _cuc;
        data[8] = _e;
        data[9] = _cus;
        data[10] = sqrt(_a);
        data[11] = _toe.sow();
        data[12] = _cic;
        data[13] = _OMG;
        data[14] = _cis;
        data[15] = _i;
        data[16] = _crc;
        data[17] = _omega;
        data[18] = _OMGDOT;
        data[19] = _idot;
        data[20] = _source;
        data[21] = _toe.gwk();
        data[22] = 0.0; // spare
        data[23] = _sisa;
        data[24] = _health;
        data[25] = _tgd[0]; // E5a/E1
        data[26] = _tgd[1]; // E5b/E1
        data[27] = _tot.sow();

        while (data[27] < 0)
        {
            data[27] += 604800;
        }
        while (data[27] > +604800)
        {
            data[27] -= 604800;
        }

        _gmutex.unlock();
        return 0;
    }

    bool t_gnavgal::chktot(const t_gtime &t)
    {
        if (t > _tot)
            return true;
        else
            return false;
    }

    t_timdbl t_gnavgal::param(const NAVDATA &n)
    {
        _gmutex.lock();

        t_timdbl tmp;
        switch (n)
        {

        case NAV_A:
            tmp = make_pair(_toc, _a * 1e0);
            break; // meters
        case NAV_E:
            tmp = make_pair(_toc, _e * 1e3);
            break; // -
        case NAV_M:
            tmp = make_pair(_toc, _m * 1e0);
            break; // radians
        case NAV_I:
            tmp = make_pair(_toc, _i * 1e0);
            break; // radians
        case NAV_IDOT:
            tmp = make_pair(_toc, _idot * 1e9);
            break; // radians/sec
        case NAV_OMEGA:
            tmp = make_pair(_toc, _omega * 1e0);
            break; // radians
        case NAV_OMG:
            tmp = make_pair(_toc, _OMG * 1e0);
            break; // radians
        case NAV_OMGDOT:
            tmp = make_pair(_toc, _OMGDOT * 1e9);
            break; // radians/sec
        case NAV_DN:
            tmp = make_pair(_toc, _dn * 1e9);
            break; // radians/sec

        case NAV_CRC:
            tmp = make_pair(_toc, _crc * 1e0);
            break; // meters
        case NAV_CIC:
            tmp = make_pair(_toc, _cic * 1e6);
            break; // radians
        case NAV_CUC:
            tmp = make_pair(_toc, _cuc * 1e6);
            break; // radians
        case NAV_CRS:
            tmp = make_pair(_toc, _crs * 1e0);
            break; // meters
        case NAV_CIS:
            tmp = make_pair(_toc, _cis * 1e6);
            break; // radians

        case NAV_F0:
            tmp = make_pair(_toc, _f0 * 1e6);
            break; // ns
        case NAV_F1:
            tmp = make_pair(_toc, _f1 * 1e6);
            break; // ns/sec
        case NAV_F2:
            tmp = make_pair(_toc, _f2 * 1e6);
            break; // ns/sec^2

        case NAV_IOD:
            tmp = make_pair(_toc, _iode * 1e0);
            break; //
        case NAV_HEALTH:
            tmp = make_pair(_toc, _health * 1e0);
            break; //
        case NAV_TGD0:
            tmp = make_pair(_toc, _tgd[0] * 1e0);
            break; // sec E5a/E1
        case NAV_TGD1:
            tmp = make_pair(_toc, _tgd[1] * 1e0);
            break; // sec E5b/E1

        default:
            break;
        }

        _gmutex.unlock();
        return tmp;
    }

    int t_gnavgal::param(const NAVDATA &n, double val)
    {
        _gmutex.lock();

        switch (n)
        { // SELECTED only, ! use the same MULTIPLICATOR as in param()

        case NAV_IOD:
            _iode = val / 1.e0;
            break;
        case NAV_HEALTH:
            _health = val / 1.e0;
            break;
        case NAV_TGD0:
            _tgd[0] = val / 1.e0;
            break; // sec E5a/E1
        case NAV_TGD1:
            _tgd[1] = val / 1.e0;
            break; // sec E5b/E1

        default:
            break;
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gnavgal::src(bool full) const
    {

        string source_bin = _source_str();

        int src = 0; // bitset<3> tmp(source_bin.substr(0,3));
        if (full)
        {
            bitset<3> tmp(source_bin.substr(0, 3));
            src = (int)tmp.to_ulong();
        }
        else if (source_bin.compare(0, 1, "1") == 0 ||
                 source_bin.compare(2, 1, "1") == 0)
        {
            bitset<3> tmp("101");
            src = (int)tmp.to_ulong();
        }
        else if (source_bin.compare(1, 1, "1") == 0)
        {
            bitset<3> tmp("010");
            src = (int)tmp.to_ulong();
        }

        return src;
    }

    GNAVTYPE t_gnavgal::gnavtype(bool full) const
    {

        _gmutex.lock();

        GNAVTYPE gnavtype = _gnavtype(full);

        _gmutex.unlock();
        return gnavtype;
    }

    GNAVTYPE t_gnavgal::_gnavtype(bool full) const
    {

        GNAVTYPE gnavtype = NAV;
        string binary = _source_str();

        if ((full && binary.compare(0, 1, "1") == 0 && binary.compare(2, 1, "1") == 0) ||  // full + combined
            (!full && (binary.compare(0, 1, "1") == 0 || binary.compare(2, 1, "1") == 0))) // not full, any-source
        {
            gnavtype = INAV;
        }
        else if (full && binary.compare(0, 1, "1") == 0)
        {
            gnavtype = INAV_E01;
        }
        else if (full && binary.compare(2, 1, "1") == 0)
        {
            gnavtype = INAV_E07;
        }
        else if (binary.compare(1, 1, "1") == 0)
        {
            gnavtype = FNAV;
        }

        return gnavtype;
    }

    string t_gnavgal::_source_str() const
    {

        string source_bin = bitset<10>(_source).to_string();
        reverse(source_bin.begin(), source_bin.end());
        return source_bin;
    }

    string t_gnavgal::_health_str() const
    {

        string health_bin = bitset<9>(_health).to_string();
        reverse(health_bin.begin(), health_bin.end());
        return health_bin;
    }

    bool t_gnavgal::_healthy() const
    {

        string source_bin = _source_str();
        string health_bin = _health_str();

        //INAV E01-B + E5b-I
        if (source_bin.compare(0, 1, "1") == 0 &&
            source_bin.compare(2, 1, "1") == 0)
        {
            if (health_bin.compare(1, 2, "00") == 0 && health_bin.compare(0, 1, "0") == 0 &&
                health_bin.compare(7, 2, "00") == 0 && health_bin.compare(6, 1, "0") == 0)
            {
                return true;
            } // HS && DVS
            else
            {
                return false;
            }
        }

        //INAV E01-B;
        if (source_bin.compare(0, 1, "1") == 0)
        {
            if (health_bin.compare(1, 2, "00") == 0 && health_bin.compare(0, 1, "0") == 0)
            {
                return true;
            } // HS && DVS
            else
            {
                return false;
            }
        }

        //FNAV E5a-I;
        if (source_bin.compare(1, 1, "1") == 0)
        {
            if (health_bin.compare(4, 2, "00") == 0 && health_bin.compare(3, 1, "0") == 0)
            {
                return true;
            } // HS && DVS
            else
            {
                return false;
            }
        }

        //INAV E5b-I;
        if (source_bin.compare(2, 1, "1") == 0)
        {
            if (health_bin.compare(7, 2, "00") == 0 && health_bin.compare(6, 1, "0") == 0)
            {
                return true;
            } // HS && DVS
            else
            {
                return false;
            }
        }

        return false;
    }

    double t_gnavgal::_mean_motion()
    {

        double n0 = sqrt(GM_GAL / (pow(_a, 3.0))); // computed mean motion [rad/sec]
        double n = n0 + _dn;                       // corrected mean motion

        return n;
    }

    void t_gnavgal::_ecc_anomaly(double dt, double &Ek, double &dEk)
    {

        double n = _mean_motion(); // eccentric anomaly
        double Mk = _m + n * dt;   // mean anomaly

        Ek = Mk;
        dEk = n;
        for (double E = 0; fabs(Ek - E) * _a > 0.001;)
        {
            E = Ek;
            Ek = Mk + _e * sin(E);        // kepler equation for eccentric anomaly [rad]
            dEk = n + _e * cos(Ek) * dEk; // derivatives w.r.t. time
        }
    }

} // namespace
