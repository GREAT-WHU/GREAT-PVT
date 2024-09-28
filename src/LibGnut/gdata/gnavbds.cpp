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
#include <stdexcept>

#include "gdata/gnavbds.h"
#include "gutils/gtypeconv.h"

#define CORRECT_BDS 14

using namespace std;

namespace gnut
{

    t_gnavbds::t_gnavbds()
        : t_gnav(),
          _iode(0),
          _iodc(0),
          _health(0),
          _toe(t_gtime::BDS),
          _toc(t_gtime::BDS),
          _tot(t_gtime::BDS),
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
          _acc(0.0)
    {
        _tgd[0] = _tgd[1] = 0.0;
        id_type(t_gdata::EPHBDS);
        id_group(t_gdata::GRP_EPHEM);
    }

    t_gnavbds::t_gnavbds(t_spdlog spdlog)
        : t_gnav(spdlog),
          _iode(0),
          _iodc(0),
          _health(0),
          _toe(t_gtime::BDS),
          _toc(t_gtime::BDS),
          _tot(t_gtime::BDS),
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
          _acc(0.0)
    {
        _tgd[0] = _tgd[1] = 0.0;

        id_type(t_gdata::EPHBDS);
        id_group(t_gdata::GRP_EPHEM);
    }

    t_gnavbds::~t_gnavbds()
    {
    }

    unsigned long _CRC24(long size, const unsigned char *buf)
    {
        unsigned long crc = 0;
        int ii;
        while (size--)
        {
            crc ^= (*buf++) << (16);
            for (ii = 0; ii < 8; ii++)
            {
                crc <<= 1;
                if (crc & 0x1000000)
                {
                    crc ^= 0x01864cfb;
                }
            }
        }
        return crc;
    }

    const int t_gnavbds::_getIODC() const
    {
        int size = 0;
        int numbits = 0;
        long long bitbuffer = 0;

        unsigned char buffer[80];
        if(nullptr == buffer)
        {
            throw std::logic_error("can not get IODC");
        }

        return ((int)_toe.sow() / 720) % 240;
    }

    string t_gnavbds::line() const
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
            << setw(w) << _iodc / 1.0
            << setw(w) << _acc
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
            << setw(w) << _tgd[0]
            << setw(w) << _tgd[1]
            << endl;

        return tmp.str();
    }

    string t_gnavbds::linefmt() const
    {

        ostringstream tmp;

        tmp << " " << setw(3) << sat() << fixed
            << " " << _toc.str("%Y-%m-%d %H:%M:%S")
            << fixed << setprecision(0)
            << setw(8) << _tot - _toc
            << setw(8) << _tot - _toe
            << setw(4) << _iode
            << setw(4) << _health
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
            << setw(9) << setprecision(3) << _tgd[0] * 1e9 // 15
            << setw(9) << setprecision(3) << _tgd[1] * 1e9 // 15
            ;

        return tmp.str();
    }

    int t_gnavbds::chk(set<string> &msg)
    {

        _gmutex.lock();

        if (!_healthy())
        {
            msg.insert("Mesg: " + _sat + " nav unhealthy satellite " + _toc.str_ymdhms());
        }

        if ((_tot - _toc) > 86400 || (_toc - _tot) > t_gnav::nav_validity(BDS))
        {
            msg.insert("Issue: " + _sat + " nav large difference [tot-toc] " + dbl2str(_tot - _toc) + " s " + _tot.str_ymdhms() + _toc.str_ymdhms(" "));
            _validity = false;
        }

        if ((_tot - _toe) > 86400 || (_toe - _tot) > t_gnav::nav_validity(BDS))
        {
            msg.insert("Issue: " + _sat + " nav large difference [tot-toe] " + dbl2str(_tot - _toe) + " s " + _tot.str_ymdhms() + _toe.str_ymdhms(" "));
            _validity = false;
        }

        if (_toe == FIRST_TIME)
        {
            msg.insert("Issue: " + _sat + " nav invalid [toe] " + _toe.str_ymdhms());
            _validity = false;
        }

        int sod_frac = _toc.sod() % 3600;
        if (sod_frac == 0)
        {
        }
        else
        {
            msg.insert("Issue: " + _sat + " nav unexpected [toc] " + _toc.str_ymdhms());
            _validity = false;
        }

        if (_iode < 0 || 255 < _iode)
        {
            msg.insert("Issue: " + _sat + " nav invalid [iode] " + _toe.str_ymdhms());
            _validity = false;
        }

        if (_iodc < 0 || 1023 < _iodc)
        {
            msg.insert("Issue: " + _sat + " nav invalid [iodc] " + _toe.str_ymdhms());
            _validity = false;
        }

        if (_a < 20000000.0)
        {
            msg.insert("Issue: " + _sat + " nav invalid [a] " + _toe.str_ymdhms());
            _validity = false;
        }

        if (_tgd[0] > 1)
        {
            _tgd[0] /= 1e10;
            msg.insert("Warning: " + _sat + " tgd[0] scale corrected " + _toe.str_ymdhms());
        }

        if (_tgd[1] > 1)
        {
            _tgd[1] /= 1e10;
            msg.insert("Warning: " + _sat + " tgd[1] scale corrected " + _toe.str_ymdhms());
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gnavbds::pos(const t_gtime &t, double xyz[], double var[], double vel[], bool chk_health)
    {

        if (sat().empty())
            return -1; // check if valid

        if (chk_health && _healthy() == false)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "not healthy sat {} excluded from pos calculation {}", sat(), t.str_ymdhms());
            return -1;
        }

        xyz[0] = xyz[1] = xyz[2] = 0.0;
        if (var)
            var[0] = var[1] = var[2] = 0.0;
        if (vel)
            vel[0] = vel[1] = vel[2] = 0.0;

        _gmutex.lock();

        double Tk = t.diff(_toe); // + CORRECT_BDS;                         // T - toe difference
        while (Tk > 302400.0)
        {
            Tk -= 604800.0;
        } // check the correct week
        while (Tk < -302400.0)
        {
            Tk += 604800.0;
        } // check the correct week

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

        double OMG = 0.0;
        // MEO/IGS0 satellites
        OMG = _OMG + (_OMGDOT - OMGE_DOT_BDS) * Tk - OMGE_DOT_BDS * _toe.sow(); // - CORRECT_BDS);         // corrected long. of asc. node

        // GEO satellites
        if (t_gsys::bds_geo(sat()))
        {
            OMG = _OMG + _OMGDOT * Tk - OMGE_DOT_BDS * (_toe.sow()); // - CORRECT_BDS);   // corrected long. of asc. node
        }

        double cosOMG = cos(OMG);
        double sinOMG = sin(OMG);

        xyz[0] = x * cosOMG - y * cosI * sinOMG; // earth-fixed coordinates [m]
        xyz[1] = x * sinOMG + y * cosI * cosOMG;
        xyz[2] = y * sinI;

        if (t_gsys::bds_geo(sat()))
        {
            double cosa = cos(-5.0 * D2R);
            double sina = sin(-5.0 * D2R);
            double cosb = cos(OMGE_DOT_BDS * Tk);
            double sinb = sin(OMGE_DOT_BDS * Tk);

            t_gtriple xyzG(xyz);
            xyz[0] = xyzG[0] * cosb + xyzG[1] * sinb * cosa + xyzG[2] * sinb * sina; // earth-fixed coordinates [m]
            xyz[1] = -xyzG[0] * sinb + xyzG[1] * cosb * cosa + xyzG[2] * cosb * sina;
            xyz[2] = -xyzG[1] * sina + xyzG[2] * cosa;
        }

        // error variance
        if (var)
        {
            *var = SQRT(_acc);
        }

        // velocities
        if (vel)
        {
            double n = _mean_motion();

            double tanv2 = tan(V / 2);
            double dEdM = 1 / (1 - _e * cos(Ek));
            double dotv = sqrt((1.0 + _e) / (1.0 - _e)) / cos(Ek / 2) / cos(Ek / 2) / (1 + tanv2 * tanv2) * dEdM * n;
            double dotu = dotv + (-_cuc * sin2U + _cus * cos2U) * 2 * dotv;
            double dotom = _OMGDOT - OMGE_DOT_BDS;
            double doti = _idot + (-_cic * sin2U + _cis * cos2U) * 2 * dotv;
            double dotr = _a * _e * sin(Ek) * dEdM * n + (-_crc * sin2U + _crs * cos2U) * 2 * dotv;
            double dotx = dotr * cosU - Rk * sinU * dotu;
            double doty = dotr * sinU + Rk * cosU * dotu;

            vel[0] = cosOMG * dotx - cosI * sinOMG * doty - x * sinOMG * dotom - y * cosI * cosOMG * dotom + y * sinI * sinOMG * doti;

            vel[1] = sinOMG * dotx + cosI * cosOMG * doty + x * cosOMG * dotom - y * cosI * sinOMG * dotom - y * sinI * cosOMG * doti;

            vel[2] = sinI * doty + y * cosI * doti;
            //lijie correct for GEO vel
            if (t_gsys::bds_geo(sat()))
            {

                dotom = _OMGDOT;
                vel[0] = cosOMG * dotx - cosI * sinOMG * doty - x * sinOMG * dotom - y * cosI * cosOMG * dotom + y * sinI * sinOMG * doti;

                vel[1] = sinOMG * dotx + cosI * cosOMG * doty + x * cosOMG * dotom - y * cosI * sinOMG * dotom - y * sinI * cosOMG * doti;

                vel[2] = sinI * doty + y * cosI * doti;

                double cosa = cos(-5.0 * D2R);
                double sina = sin(-5.0 * D2R);
                double cosb = cos(OMGE_DOT_BDS * Tk);
                double sinb = sin(OMGE_DOT_BDS * Tk);

                t_gtriple velG(vel);
                vel[0] = velG[0] * cosb + velG[1] * sinb * cosa + velG[2] * sinb * sina;
                vel[1] = -velG[0] * sinb + velG[1] * cosb * cosa + velG[2] * cosb * sina;
                vel[2] = -velG[1] * sina + velG[2] * cosa;

                vel[0] = vel[0] + xyz[1] * OMGE_DOT_BDS;
                vel[1] = vel[1] - xyz[0] * OMGE_DOT_BDS;
            }
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gnavbds::clk(const t_gtime &t, double *clk, double *var, double *dclk, bool chk_health)
    {

        if (sat().empty())
            return -1; // not valid !!!

        if (chk_health && _healthy() == false)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "not healthy sat " + sat() + " excluded from clk calculation " + t.str_ymdhms());
            return -1;
        }

        _gmutex.lock();

        double Tk = t.diff(_toc); // difference from clk(!) epoch ref. time
        while (Tk > 302400.0)
        {
            Tk -= 604800.0;
        } // check the correct BDS week
        while (Tk < -302400.0)
        {
            Tk += 604800.0;
        } // check the correct BDS week

        double Ek, dEk;
        _ecc_anomaly(Tk, Ek, dEk); // eccentric anomaly

        // SV time correction (including periodic relativity correction)

        // NOT CORRECTED FOR 2nd-order relativistic effect - at user side
        _rel = 4.442807633e-10 * sqrt(_a) * _e * sin(Ek);
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
            *var = SQRT(_acc);
        }

        // velocities
        if (dclk)
        {
            *dclk = 0.0;
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gnavbds::data2nav(string sat, const t_gtime &ep, const t_gnavdata &data)
    {
        _gmutex.lock();

        char tmp[12];

        if (sat.substr(0, 1) == "C")
            _sat = sat;
        else
            _sat = "C" + sat;

        _toc = ep; // time of clocks (tot) .. RINEX reference time
        _epoch = ep;

        int data_21 = static_cast<int>(data[21]); // toe week (BDS!)
        int data_11 = static_cast<int>(data[11]); // toe sow
        int data_27 = static_cast<int>(data[27]); // tot sow

        if (data_21 < 0)
        {
            _toe = FIRST_TIME; 
            _gmutex.unlock();
            return -1;
        }
        if (data_21 == 0 || ((data_21 != _toc.gwk() - CONV_BWK2GWK) && data_21 != _toc.gwk()))
        { 
            t_gtime curt = t_gtime::current_time(t_gtime::GPS);
            data_21 = curt.gwk();
            t_gtime tt;
            tt.from_gws(data_21, data_11);
            if (fabs(tt - curt) > 86400)
                data_21 -= 1;
            sprintf(tmp, "%4i %6i", data_21, data_11);
        }
        else
        {
            if (data_21 == _toc.gwk() - CONV_BWK2GWK)
            {
                sprintf(tmp, "%4i %6i", data_21 + CONV_BWK2GWK, data_11); // original
            }
            else
            {
                sprintf(tmp, "%4i %6i", data_21, data_11);
            }
        }
        _toe.from_str("%W %v", tmp); // time of ephemeris    (toe)
        if (data_27 == 0.9999e9 || data_27 == 0.0)
            _tot = _toe;

        else
        {
            if (data_21 == _toc.gwk() - CONV_BWK2GWK)
                sprintf(tmp, "%4i %6i", data_21 + CONV_BWK2GWK, data_27);
            else
                sprintf(tmp, "%4i %6i", data_21, data_27);

            _tot.from_str("%W %v", tmp);
        }

        if (fabs(_tot - _toe - 604800) < MAX_BDS_TIMEDIFF)
            _tot.add_secs(-604800); // adjust tot to toe gwk
        if (fabs(_tot - _toe + 604800) < MAX_BDS_TIMEDIFF)
            _tot.add_secs(+604800); // adjust tot to toe gwk

        _f0 = data[0];
        _f1 = data[1];
        _f2 = data[2];

        _iode = static_cast<int>(data[3]);
        _iodc = static_cast<int>(data[28]);
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

        _acc = data[23];
        _tgd[0] = data[25];
        _tgd[1] = data[26];

        _gmutex.unlock();
        return 0;
    }

    int t_gnavbds::nav2data(t_gnavdata &data)
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
        data[11] = _toe.sow(); // -CORRECT_BDS;
        data[12] = _cic;
        data[13] = _OMG;
        data[14] = _cis;
        data[15] = _i;
        data[16] = _crc;
        data[17] = _omega;
        data[18] = _OMGDOT;
        data[19] = _idot;
        data[20] = 0.0;        // spare
        data[21] = _toe.bwk(); // BDS week!
        data[22] = 0.0;        // spare
        data[23] = _acc;       // ura_eph[_acc]; // [m]
        data[24] = _health;
        data[25] = _tgd[0];
        data[26] = _tgd[1];
        data[27] = _tot.sow(); //-CORRECT_BDS;
        data[28] = _iodc;

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

    bool t_gnavbds::chktot(const t_gtime &t)
    {
        if (t > _tot)
            return true;
        else
            return false;
    }

    t_timdbl t_gnavbds::param(const NAVDATA &n)
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
            break; // sec
        case NAV_TGD1:
            tmp = make_pair(_toc, _tgd[1] * 1e0);
            break; // sec

        default:
            break;
        }

        _gmutex.unlock();
        return tmp;
    }

    int t_gnavbds::param(const NAVDATA &n, double val)
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
            break;
        case NAV_TGD1:
            _tgd[1] = val / 1.e0;
            break;

        default:
            break;
        }

        _gmutex.unlock();
        return 0;
    }

    bool t_gnavbds::_healthy() const
    {
        if (_health == 0)
            return true;
        return false;
    }

    double t_gnavbds::_mean_motion()
    {

        double n0 = sqrt(GM_CGCS / (pow(_a, 3.0))); // computed mean motion [rad/sec]
        double n = n0 + _dn;                        // corrected mean motion

        return n;
    }

    void t_gnavbds::_ecc_anomaly(double dt, double &Ek, double &dEk)
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

    int t_gnavbds::iod() const
    {

        _gmutex.lock();
        int iod = _getIODC();
        _gmutex.unlock();
        return iod;
    }
} // namespace
