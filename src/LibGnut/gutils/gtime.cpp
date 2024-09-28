
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <algorithm>

#if defined _WIN32 || defined _WIN64
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "gutils/gtime.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{

    t_gtime::t_gtime(const t_tsys &ts)
        : _mjd(0), _sod(0), _dsec(0.0), _tsys(UTC)
    {
        from_time(time(NULL), 0.0, true); // input UTC time-system
        _tsys = ts;                       // now switch to required TS

        _reset_conv();
    }

    t_gtime::t_gtime(const time_t &tt, const double &ds, const t_tsys &ts)
        : _mjd(0), _sod(0), _dsec(0.0), _tsys(UTC)
    {
        from_time(tt, ds, true); // input UTC time-system
        _tsys = ts;              // now switch to required TS

        _reset_conv(); 
    }

    t_gtime::t_gtime(const int &yr, const int &mn,
                     const int &dd, const int &hr,
                     const int &mi, const int &sc,
                     const double &ds, const t_tsys &ts)
        : _mjd(0), _sod(0), _dsec(0.0), _tsys(ts)
    {
        from_ymdhms(yr, mn, dd, hr, mi, sc + ds, true);
    }

    t_gtime::t_gtime(const int &gw, const int &dw, const int &sd,
                     const double &ds, const t_tsys &ts)
        : _mjd(0), _sod(0), _dsec(0.0), _tsys(ts)
    {
        from_gwd(gw, dw, sd, ds, true);
    }

    t_gtime::t_gtime(const int &gw, const double &sow, const t_tsys &ts)
        : _mjd(0), _sod(0), _dsec(0.0), _tsys(ts)
    {
        from_gws(gw, sow, true);
    }

    t_gtime::t_gtime(const int &mjd, const int &sd,
                     const double &ds, const t_tsys &ts)
        : _mjd(0), _sod(0), _dsec(0.0), _tsys(ts)
    {
        from_mjd(mjd, sd, ds, true);
    }

    t_gtime::~t_gtime()
    {
    }

    string t_gtime::tsys2str(const t_tsys &ts)
    {
        switch (ts)
        {
        case USER:
            return "USER";
        case TAI:
            return "TAI";
        case UTC:
            return "UTC";
        case LOC:
            return "LOC";
        case GPS:
            return "GPS";
        case GLO:
            return "GLO";
        case GAL:
            return "GAL";
        case BDS:
            return "BDS";
        case TT:
            return "TT";

        default:
        {
            cout << "*** warning: unknown time system!\n";
            cout.flush();
        }
        }
        return "TAI";
    }

    t_gtime::t_tsys t_gtime::str2tsys(const string &tmp)
    {
        string s(tmp);
        if (s.size() == 0)
        {
            cout << "*** warning: not defined time system\n";
            cout.flush();
            return TAI;
        }

        transform(s.begin(), s.end(), s.begin(), ::toupper);

        if (s == "USER")
            return USER;
        else if (s == "TAI")
            return TAI;
        else if (s == "UTC")
            return UTC;
        else if (s == "LOC")
            return LOC;
        else if (s == "GPS")
            return GPS;
        else if (s == "GLO")
            return GLO;
        else if (s == "GAL")
            return GAL;
        else if (s == "BDS")
            return BDS;
        else if (s == "TT")
            return TT;
        else
        {
            cout << "*** warning: not defined correct time system [" << s[0] << "]\n";
            cout.flush();
        }

        return TAI;
    }

    t_gtime t_gtime::current_time(const t_tsys &ts)
    {
        t_gtime tmp(UTC);
        tmp.from_time(time(NULL), 0.0, true);
        tmp.tsys(ts);
        return tmp;
    }

    int t_gtime::from_time(const time_t &tt, const double &ds, const bool &conv)
    {
        struct tm *tm = gmtime(&tt);
        from_ymd(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                 tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec,
                 ds, conv);

        _norm_dsec();
        _norm_sod();

        _reset_conv();
        return 0;
    }

    int t_gtime::from_gws(const int &gw, const double &sow, const bool &conv)
    {
        int dw = (int)floor(sow / 86400);
        _mjd = 44244 + 7 * gw + dw;
        _sod = (int)floor(sow - dw * 86400.0);
        _dsec = sow - dw * 86400.0 - _sod;

        if (conv)
            _to_tai(); // CONVERT
        _norm_dsec();
        _norm_sod();

        _reset_conv();
        return 0;
    }

    int t_gtime::from_gwd(const int &gw, const int &dw, const int &sd,
                          const double &ds, const bool &conv)
    {
        _mjd = 44244 + 7 * gw + dw;
        _sod = sd;
        _dsec = ds;

        if (conv)
            _to_tai(); // CONVERT
        _norm_dsec();
        _norm_sod();

        _reset_conv();
        return 0;
    }

    int t_gtime::from_ymd(const int &yr, const int &mn, const int &dd,
                          const int &sd, const double &ds, const bool &conv)
    {
        int year(yr);
        _norm_year(year);
        _mjd = _ymd_mjd(year, mn, dd);
        _sod = sd;
        _dsec = ds;

        if (conv)
            _to_tai(); // CONVERT
        _norm_dsec();
        _norm_sod();

        _reset_conv();
        return 0;
    }

    int t_gtime::from_ymdhms(const int &yr, const int &mn,
                             const int &dd, const int &h,
                             const int &m, const double &s,
                             const bool &conv)
    {
        int year(yr);
        _norm_year(year);
        _mjd = _ymd_mjd(year, mn, dd);

        _sod = int((h * 3600.0 + m * 60.0) + floor(s));
        _dsec = s - 1.0 * (floor(s));

        if (conv)
            _to_tai(); // CONVERT
        _norm_dsec();
        _norm_sod();

        _reset_conv();
        return 0;
    }

    int t_gtime::from_mjd(const int &mjd, const int &sd,
                          const double &ds, const bool &conv)
    {
        _mjd = mjd;
        _sod = sd;
        _dsec = ds;

        if (conv)
            _to_tai(); // CONVERT
        _norm_dsec();
        _norm_sod();

        _reset_conv();
        return 0;
    }

    int t_gtime::from_dmjd(const double &dmjd, const bool &conv)
    {
        _mjd = (int)dmjd;
        _sod = (int)((dmjd - _mjd) * 86400.0);
        _dsec = (dmjd - _mjd) * 86400.0 - _sod;

        if (conv)
            _to_tai(); // CONVERT
        _norm_dsec();
        _norm_sod();

        _reset_conv();
        return 0;
    }

    int t_gtime::from_str(const string &ifmt, const string &idat, const bool &conv)
    {
        int cYMD = _ymd;
        int cHMS = _hms;
        int y, b, d, h, m, s;
        y = b = d = h = m = s = 0;
        size_t idx;
        size_t fmtadd = 0;
        size_t datadd = 0;
        size_t fmtpos = 0;
        size_t datpos = 0;

        // search and process keys from left to right
        while ((fmtpos < ifmt.length()) && (idx = ifmt.find('%', fmtpos)) != string::npos)
        {

            fmtpos++; // a priori encrease

            for (int i = 0; i < MAX_DT; ++i)
            {
                string tmp = TD[i];

                // too short ifmt format string, skip!
                if (ifmt.length() - idx - 1 < tmp.length())
                    continue;

                // dat string not identified, skipped !
                if (ifmt.substr(idx + 1, tmp.length()).compare(tmp) != 0)
                    continue;

                fmtpos = idx + tmp.length() + 1; // end of format reading
                datpos = idx - fmtadd + datadd;  // sum of all idat characters
                fmtadd += tmp.length() + 1;      // sum of all ifmt characters

                if (!tmp.compare("Y"))
                {
                    y = str2int(idat.substr(datpos, 4));
                    datadd += 4;
                    cYMD += t_tymd(_year);
                }
                else if (!tmp.compare("y"))
                {
                    y = str2int(idat.substr(datpos, 2));
                    datadd += 2;
                    cYMD += t_tymd(_year);
                }
                else if (!tmp.compare("b"))
                {
                    b = mon(idat.substr(datpos, 3));
                    datadd += 3;
                    cYMD += t_tymd(_mon);
                }
                else if (!tmp.compare("m"))
                {
                    b = str2int(idat.substr(datpos, 2));
                    datadd += 2;
                    cYMD += t_tymd(_mon);
                }
                else if (!tmp.compare("d"))
                {
                    d = str2int(idat.substr(datpos, 2));
                    datadd += 2;
                    cYMD += t_tymd(_day);
                }
                else if (!tmp.compare("j"))
                {
                    b = 1;
                    cYMD += t_tymd(_mon);
                    d = str2int(idat.substr(datpos, 3));
                    datadd += 3;
                    cYMD += t_tymd(_day);
                }
                else if (!tmp.compare("H"))
                {
                    h = str2int(idat.substr(datpos, 2));
                    datadd += 2;
                    cHMS += t_thms(_hour);
                }
                else if (!tmp.compare("M"))
                {
                    m = str2int(idat.substr(datpos, 2));
                    datadd += 2;
                    cHMS += t_thms(_min);
                }
                else if (!tmp.compare("S"))
                {
                    s = str2int(idat.substr(datpos, 2));
                    datadd += 2;
                    cHMS += t_thms(_sec);
                }
                else if (!tmp.compare("W"))
                {
                    y = 1980;
                    cYMD += t_tymd(_year);
                    b = 1;
                    cYMD += t_tymd(_mon);
                    d += 6 + str2int(idat.substr(datpos, 4)) * 7;
                    datadd += 4;
                }
                else if (!tmp.compare("w"))
                {
                    d += str2int(idat.substr(datpos, 1));
                    datadd += 1;
                    cYMD += t_tymd(_day);
                }
                else if (!tmp.compare("v"))
                {
                    s += str2int(idat.substr(datpos, 6));
                    datadd += 6;
                    cYMD += t_tymd(_day);
                    cHMS += t_thms(_hour) + t_thms(_min) + t_thms(_sec);
                }
                else if (!tmp.compare("s"))
                {
                    s = str2int(idat.substr(datpos, 5));
                    datadd += 5;
                    cHMS += t_thms(_hour) + t_thms(_min) + t_thms(_sec);
                }
                else if (!tmp.compare("I"))
                {
                    y = 2000;
                    cYMD += t_tymd(_year);
                    b = 1;
                    cYMD += t_tymd(_mon);
                    d = 1 - 51544 + (str2int(idat.substr(datpos, 11)));
                    cYMD += t_tymd(_day);
                    s = 86400 * (str2int(idat.substr(datpos, 11)) - (str2int(idat.substr(datpos, 11))));
                    datadd += 11;
                    cHMS += t_thms(_hour) + t_thms(_min) + t_thms(_sec);
                }
                else if (!tmp.compare("J"))
                {
                    y = 2000;
                    cYMD += t_tymd(_year);
                    b = 1;
                    cYMD += t_tymd(_mon);
                    d = 1 - 51544 + str2int(idat.substr(datpos, 5));
                    datadd += 5;
                    cYMD += t_tymd(_day);
                }
                else if (!tmp.compare("T"))
                {
                    cerr << "input time-system conversion not yet supported here!\n";
                }
                else
                {
                    cerr << " warning : t_gtime - unknown date/time identifier [" << tmp << "]\n";
                }
            }
        }

        if (cYMD == (_year | _mon | _day) && cHMS == (_hour | _min | _sec))
        {

            from_ymd(y, b, d, h * 3600 + m * 60 + s, 0.0, conv);
            return 0;
        }

        if (cYMD == (_year | _mon | _day))
        {
           from_ymd(y, b, d, 0, 0.0, conv);
            return 0;
        }

        _reset_conv();
        return 1;
    }

    int t_gtime::reset_dsec()
    {
        _norm_dsec();
        _dsec = 0.0;

        _reset_conv();
        return 0;
    }

    int t_gtime::reset_sod()
    {
        _norm_sod();
        _tai_to();
        _sod = 0;
        _to_tai();
        reset_dsec();

        _reset_conv();
        return 0;
    }

    int t_gtime::tzdiff() const
    {
        switch (_tsys)
        {
        case USER:
            return 0;
        case TAI:
            return 0;
        case UTC:
            return 0;
        case GPS:
            return 0;
        case GLO:
            return 3 * 3600;
        case GAL:
            return 0;
        case BDS:
            return 0;
        case LOC:
        {
            time_t local, utc;
            local = time(NULL);
            utc = mktime(gmtime(&local));
            return int(local - utc);
        }
        default:
            cerr << " warning : t_tsys not recognized !\n";
        }
        return 0;
    }

    int t_gtime::dstime() const
    {
        switch (_tsys)
        {
        case USER:
            return 0;
        case TAI:
            return 0;
        case UTC:
            return 0;
        case GPS:
            return 0;
        case GLO:
            return 0;
        case BDS:
            return 0;
        case GAL:
            return 0;
        case LOC:
        {
            struct tm tm;
            ymd(tm.tm_year, tm.tm_mon, tm.tm_mday, false); // TAI !
            hms(tm.tm_hour, tm.tm_min, tm.tm_sec, false);  // TAI !

            tm.tm_year -= 1900;
            tm.tm_mon -= 1;
            tm.tm_sec -= leapsec() + tzdiff(); // convert tai to LOC !
            tm.tm_isdst = -1;

            mktime(&tm);
            return int(3600.0 * tm.tm_isdst); // [sec]
        }
        default:
            cerr << " warning : t_tsys not recognized !\n";
        }
        return 0;
    }

    int t_gtime::leapsec() const
    {
        int leap = 0;
        for (int i = sizeof(leapseconds) / sizeof(*leapseconds) - 1; i >= 0; --i)
        {
            double mjd_time = _mjd + _sod / 86400.0 - leapseconds[i]._leap / 86400.0;
            double mjd_leap = static_cast<double>(_ymd_mjd(leapseconds[i]._year,
                                                           leapseconds[i]._mon,
                                                           leapseconds[i]._day));
            if (mjd_leap <= mjd_time)
            {
                leap = leapseconds[i]._leap;
                break;
            }
        }
        return leap;
    }

    int t_gtime::leapyear(const int &y) const
    {
        if (y % 4 == 0.0 && y % 100 != 0.0)
            return 1; //     leap year !
        if (y % 400 == 0.0)
            return 1; //     leap year !

        return 0; // not leap year
    }

    int t_gtime::mjd(const bool &conv) const
    {
        if (!conv)
        {
            return _mjd;
        }
        else
        {
            return _mjd_conv;
        }

    }

    double t_gtime::dmjd(const bool &conv) const
    {
        double ret;
        ret = dsec(conv) / 86400.0 + (double)sod(conv) / 86400.0 + (double)mjd(conv);
        return ret;
    }

    double t_gtime::dmjd_vlbi(const bool &conv) const
    {
        int m, y, b = 0;
        bool ind = false;
        if (mon() <= 2)
        {
            m = mon() + 12;
            y = year() - 1;
        }
        else
        {
            m = mon();
            y = year();
        }
        if (y <= 1582 && m <= 10 && day() <= 4)
        {
            b = -2;
            ind = true;
        }
        if (ind == false)
        {
            b = int(y / 400) - int(y / 100);
        }
        double jd = int(365.25 * y) - 2400000.5;
        double tmjd = jd + int(30.6001 * (m + 1)) + b + 1720996.5 + day() + hour() / 24.0 + mins() / 1440.0 + secs() / 86400.0 + dsec() / 86400.0;
        return tmjd;
    }

    int t_gtime::sod(const bool &conv) const
    {
        if (!conv)
        {
            return _sod;
        }
        else
        {
            return _sod_conv;
        }
    }

    double t_gtime::dsec(const bool &conv) const
    {
        if (!conv)
        {
            return _dsec;
        }
        else
        {
            return _dsec_conv;
        }
    }

    time_t t_gtime::tim() const
    {
        int y, b, d, h, m, s;
        ymd(y, b, d, true); 
        hms(h, m, s, true); 

        struct tm t;
        t.tm_year = y - 1900;
        t.tm_mon = b - 1;
        t.tm_mday = d;
        t.tm_hour = h;
        t.tm_min = m;
        t.tm_sec = s;
        t.tm_isdst = -1;

        return mktime(&t);
    }

    int t_gtime::doy(const bool &conv) const
    {
        int y, m, d;
        ymd(y, m, d, conv);

        int doy = d;
        for (int i = 0; i < m && i < 13; ++i)
            doy += monthdays[i];
        if (m > 2)
            doy += leapyear(y); // count for leap year
        return doy;
    }

    int t_gtime::bwk() const
    {
        return static_cast<int>((_mjd_conv - 44244.0) / 7.0) - CONV_BWK2GWK; // return true TS
    }

    int t_gtime::gwk() const
    {
        return static_cast<int>((mjd(true) - 44244.0) / 7.0); 
    }

    int t_gtime::dow() const
    {
        return static_cast<int>(mjd(true) - 44244.0 - gwk() * 7); 
    }

    int t_gtime::sow() const
    {
        return static_cast<int>(dow() * 86400.0 + sod()); 
    }

    int t_gtime::year() const
    {
        int y, m, d;
        ymd(y, m, d, false);

        return static_cast<int>(y);
    }

    int t_gtime::yr() const
    {
        int y, m, d;
        ymd(y, m, d, false);
        int yr = this->yr(y);
        return yr;
    }

    int t_gtime::mon() const
    {
        int y, m, d;
        ymd(y, m, d, true);

        return static_cast<int>(m);
    }

    int t_gtime::day() const
    {
        int y, m, d;
        ymd(y, m, d, true);

        return static_cast<int>(d);
    }

    int t_gtime::hour() const
    {
        return static_cast<int>((sod(true) % 86400) / 3600.0);
    }

    int t_gtime::mins() const
    {
        return static_cast<int>((sod(true) % 3600) / 60.0);
    }

    int t_gtime::secs() const
    {
        return static_cast<int>(sod(true) % 60);
    }

    void t_gtime::hms(int &h, int &m, int &s, bool conv) const
    {
        h = static_cast<int>((sod(conv) % 86400) / 3600.0);
        m = static_cast<int>((sod(conv) % 3600) / 60.0);
        s = static_cast<int>((sod(conv) % 60));
    }

    void t_gtime::ymd(int &y, int &m, int &d, bool conv) const
    {
        int jj, mm, dd;
        long ih, ih1, ih2;
        double t1, t2, t3, t4;
        double mjd = 0.0;

        mjd = this->mjd(conv) + (this->sod(conv)) / 86400.0;

        t1 = 1.0 + mjd - fmod(mjd, 1.0) + 2400000.0;
        t4 = fmod(mjd, 1.0);
        ih = int((t1 - 1867216.25) / 36524.25);
        t2 = t1 + 1 + ih - ih / 4;
        t3 = t2 - 1720995.0;
        ih1 = int((t3 - 122.1) / 365.25);
        t1 = ih1 * 365.25 - fmod(ih1 * 365.25, 1.0);
        ih2 = int((t3 - t1) / 30.6001);
        dd = int(t3 - t1 - int(ih2 * 30.6001) + t4);
        mm = ih2 - 1;
        if (ih2 > 13)
            mm = ih2 - 13;
        jj = ih1;
        if (mm <= 2)
            jj = jj + 1;
        y = jj;
        m = mm;
        d = dd;
    }

    string t_gtime::str(const string &ofmt, const bool &conv) const
    {
        t_gtime gt(*this);

        char cstr[12] = "char";
        string str = ofmt; // copy of requested format
        size_t idx = 0;
        int y, b, d, h, m, s;
        gt.ymd(y, b, d, conv);
        gt.hms(h, m, s, conv);
        int y2 = gt.yr(y);

        while ((idx = str.find('%')) != string::npos && idx + 1 <= str.length())
        {

            bool replace = false;
            for (int i = 0; i < MAX_DT; ++i)
            {
                string tmp = TD[i];

                if (str.substr(idx + 1, tmp.length()).compare(tmp) != 0)
                    continue;

                if (!tmp.compare("Y"))
                {
                    sprintf(cstr, "%04i", y);
                }
                else if (!tmp.compare("y"))
                {
                    sprintf(cstr, "%02i", y2);
                }
                else if (!tmp.compare("b"))
                {
                    sprintf(cstr, "%3.3s", gt.mon(b).c_str());
                }
                else if (!tmp.compare("m"))
                {
                    sprintf(cstr, "%02i", b);
                }
                else if (!tmp.compare("d"))
                {
                    sprintf(cstr, "%02i", d);
                }
                else if (!tmp.compare("j"))
                {
                    sprintf(cstr, "%03i", gt.doy());
                }
                else if (!tmp.compare("H"))
                {
                    sprintf(cstr, "%02i", h);
                }
                else if (!tmp.compare("M"))
                {
                    sprintf(cstr, "%02i", m);
                }
                else if (!tmp.compare("S"))
                {
                    sprintf(cstr, "%02i", s);
                }
                else if (!tmp.compare("W"))
                {
                    sprintf(cstr, "%04i", gt.gwk());
                }
                else if (!tmp.compare("w"))
                {
                    sprintf(cstr, "%01i", gt.dow());
                }
                else if (!tmp.compare("v"))
                {
                    sprintf(cstr, "%6i", gt.sow());
                }
                else if (!tmp.compare("s"))
                {
                    sprintf(cstr, "%5i", gt.sod(conv));
                }
                else if (!tmp.compare("J"))
                {
                    sprintf(cstr, "%5i", gt.mjd(conv));
                }
                else if (!tmp.compare("I"))
                {
                    sprintf(cstr, "%11.5f", gt.mjd(conv) + gt.sod(conv) / 86400.0 + gt.dsec(conv) / 86400.0);
                }
                else if (!tmp.compare("T"))
                {
                    sprintf(cstr, "%3s", sys().c_str());
                    // add for output clk files C L K O P
                }
                else if (!tmp.compare("C"))
                {
                    sprintf(cstr, "%2i", b);
                }
                else if (!tmp.compare("L"))
                {
                    sprintf(cstr, "%2i", d);
                }
                else if (!tmp.compare("K"))
                {
                    sprintf(cstr, "%2i", h);
                }
                else if (!tmp.compare("O"))
                {
                    sprintf(cstr, "%2i", m);
                }
                else if (!tmp.compare("P"))
                {
                    sprintf(cstr, "%9.6f", static_cast<double>(s));
                }
                else if (!tmp.compare("p"))
                {
                    sprintf(cstr, "%10.7f", static_cast<double>(s) + gt.dsec(conv));
                }
                else
                {
                    cerr << " warning : t_gtime - unknown date/time identifier [" << tmp << "]\n";
                }

                str.replace(idx, 2, cstr);
                replace = true;
            }
            if (!replace)
                str.replace(idx, 1, "");
        }
        return str;
    }

    string t_gtime::str_ymd(const string &str, const bool &conv) const
    {
        char cstr[12];
        int y = 0, b = 0, d = 0;
        this->ymd(y, b, d, conv);
        sprintf(cstr, "%04i-%02i-%02i", y, b, d);
        return str + " " + string(cstr);
    }

    string t_gtime::str_hms(const string &str, const bool &conv) const
    {
        char cstr[12];
        int h = 0, m = 0, s = 0;
        this->hms(h, m, s, conv);
        sprintf(cstr, "%02i:%02i:%02i", h, m, s);
        return str + " " + string(cstr);
    }

    string t_gtime::str_ydoysod(const string &str, const bool &conv) const
    {
        char cstr[15];
        sprintf(cstr, "%04i:%03i:%05i", year(), doy(conv), sod());
        return cstr;
    }

    string t_gtime::str_ymdhms(const string &str,
                               const bool &ts,
                               const bool &conv) const
    {
        char cstr[25];
        int y = 0, b = 0, d = 0, h = 0, m = 0, s = 0;
        this->ymd(y, b, d, conv);
        this->hms(h, m, s, conv);
        if (ts)
            sprintf(cstr, "%04i-%02i-%02i %02i:%02i:%02i[%3s]", y, b, d, h, m, s, sys().c_str());
        else
            sprintf(cstr, "%04i-%02i-%02i %02i:%02i:%02i", y, b, d, h, m, s);
        return str + " " + string(cstr);
    }

    string t_gtime::str_mjdsod(const string &str, const bool &ts, const bool &conv) const
    {
        char cstr[25];

        int mjd = this->mjd(ts);
        double sod = (this->dmjd(ts) - mjd) * 86400.0;
        int isod = 0;
        if (fabs(sod - (int)(sod)) < 0.5e0)
        {
            isod = round(sod);
            sprintf(cstr, "%05i %10.4f", mjd, static_cast<double>(isod));
        }
        else
        {
            sprintf(cstr, "%05i %10.4f", mjd, sod);
        }

        return str + " " + string(cstr);
    }

    string t_gtime::str_yyyydoy(const bool &conv) const
    {
        int year, doy;
        string tmp;
        year = this->year();
        doy = this->doy(conv);
        string cdoy = int2str(doy);
        if (doy >= 10 && doy < 100)
            cdoy = "0" + cdoy;
        if (doy < 10)
            cdoy = "00" + cdoy;
        tmp = int2str(year) + cdoy;
        return tmp;
    }

    string t_gtime::str_yyyy(const bool &conv) const
    {
        return int2str(this->year());
    };

    string t_gtime::str_doy(const bool &conv) const
    {
        string tmp = int2str(this->doy());
        if (tmp.size() == 1)
            return "00" + tmp;
        else if (tmp.size() == 2)
            return "0" + tmp;
        else
            return tmp;
    };

    string t_gtime::str_gwkd(const bool &conv) const
    {
        return int2str(this->gwk()) + int2str(this->dow());
    }

    string t_gtime::str_gwk(const bool &conv) const
    {
        return int2str(this->gwk());
    }

    string t_gtime::str_yr(const bool &conv) const
    {
        string tmp = int2str(this->yr());
        if (tmp.size() == 1)
            return "0" + tmp;
        else
            return tmp;
    }

    string t_gtime::str_mon(const bool &conv) const
    {
        string tmp = int2str(this->mon());
        if (tmp.size() == 1)
            return "0" + tmp;
        else
            return tmp;
    }
    string t_gtime::str_day(const bool &conv) const
    {
        string tmp = int2str(this->day());
        if (tmp.size() == 1)
            return "0" + tmp;
        else
            return tmp;
    }

    string t_gtime::str_hour(const bool & conv) const
    {
        string tmp = int2str(this->hour());
        if (tmp.size() == 1)
            return "0" + tmp;
        else
            return tmp;
    }

    string t_gtime::str_min(const bool & conv) const
    {
        string tmp = int2str(this->mins());
        if (tmp.size() == 1)
            return "0" + tmp;
        else
            return tmp;
    }

    int t_gtime::yr(const int &y) const
    {
        if (y <= 2079 && y >= 2000)
            return y - 2000;
        if (y < 2000 && y >= 1980)
            return y - 1900;

        return -1;
    }

    string t_gtime::mon(const int &m) const
    {
        if (m < 1 || m > 12)
            return "XXX";
        return MON[m];
    }

    int t_gtime::mon(const string &str) const
    {
        for (int i = 1; i <= 12; ++i)
        {
            if (str.compare(MON[i]) == 0)
                return i;
        }
        return 0;
    }

    string t_gtime::sys() const
    {
        string ts(t_tstr[_tsys]);
        return ts;
    }

    double t_gtime::operator-(const t_gtime &t) const
    {
        return (this->diff(t));
    }

    bool t_gtime::operator<(const double &t) const
    {
        return (this->sow() + this->dsec() < t);
    }

    t_gtime t_gtime::operator-(const double &sec) const
    {
        t_gtime tmp(*this);
        tmp.add_secs(-static_cast<int>(sec));
        tmp.add_dsec(-static_cast<double>(sec - static_cast<int>(sec))); // already normed
        return tmp;
    }

    t_gtime t_gtime::operator+(const double &sec) const
    {
        t_gtime tmp(*this);
        int second = floor(sec);
        double dsecond = sec - second;
        tmp.add_secs(second);
        tmp.add_dsec(dsecond);
        return tmp;
    }

    bool t_gtime::operator<(const t_gtime &t) const
    {
        return ((_mjd_conv < t._mjd_conv) || (_mjd_conv == t._mjd_conv && _sod_conv < t._sod_conv) || (_mjd_conv == t._mjd_conv && _sod_conv == t._sod_conv && _dsec_conv < t._dsec_conv));
    }

    bool t_gtime::operator<=(const t_gtime &t) const
    {
        return (*this < t || *this == t);
    }

    bool t_gtime::operator>=(const t_gtime &t) const
    {
        return (*this > t || *this == t);
    }

    bool t_gtime::operator>(const t_gtime &t) const
    {
        return ((_mjd_conv > t._mjd_conv) || (_mjd_conv == t._mjd_conv && _sod_conv > t._sod_conv) || (_mjd_conv == t._mjd_conv && _sod_conv == t._sod_conv && _dsec_conv > t._dsec_conv));
    }

    bool t_gtime::operator==(const t_gtime &t) const
    {
        return (_mjd_conv == t._mjd_conv && _sod_conv == t._sod_conv && _dsec_conv == t._dsec_conv);
    }

    bool t_gtime::operator!=(const t_gtime &t) const
    {
        return (_mjd_conv != t._mjd_conv || _sod_conv != t._sod_conv || _dsec_conv != t._dsec_conv);
    }

    t_gtime t_gtime::operator=(const t_gtime &t)
    {
        _mjd = t.mjd(false);
        _sod = t.sod(false);
        _dsec = t.dsec(false);
        tsys(t.tsys());

        _reset_conv();
        return *this;
    }

    void t_gtime::del_secs(const int &sec)
    {
        _sod -= sec;
        _norm_sod();

        _reset_conv();
    }

    void t_gtime::add_secs(const int &sec)
    {
        _sod += sec;
        _norm_sod();

        _reset_conv();
    }

    void t_gtime::add_dsec(const double &dsec)
    {
        _dsec += dsec;
        _norm_dsec();
        _norm_sod();

        _reset_conv();
    }

    double t_gtime::diff(const t_gtime &t) const
    {
        bool b = false; // compare in TAI
        return ((dsec(b) - t.dsec(b)) + (sod(b) - t.sod(b)) + (mjd(b) * 86400.0 - t.mjd(b) * 86400.0));
    }

    void t_gtime::_to_tai()
    {
        _norm_dsec();
        _norm_sod();
        switch (_tsys)
        {
        case USER:
            break;
            ;
        case TAI:
            break;
            ;
        case UTC:
            _sod += leapsec();
            break;
            ;
        case GPS:
            _sod += TAI_GPS;
            break;
            ;
        case GLO:
            _sod -= tzdiff();
            _sod += leapsec();
            break;
            ;
        case GAL:
            _sod += TAI_GAL;
            break;
            ;
        case BDS:
            _sod += TAI_BDS;
            break;
            ;
        case LOC:
            _sod -= tzdiff();
            _sod -= dstime();
            _sod += leapsec();
            break;
            ;
        case TT:
            _sod -= TAI_TT;
            _dsec -= TAT_TT_DSEC;
            break;
            ;
        default:
            break;
            ;
        }
        _norm_dsec();
        _norm_sod();
    }

    void t_gtime::_reset_conv()
    {
        _mjd_conv = _mjd + floor((_sod + tai_tsys(_tsys) + _dsec + tai_tsys_dsec(_tsys)) / 86400.0); // 86400.0  have to be float!

        double sod_temp = _sod + tai_tsys(_tsys) + _dsec + tai_tsys_dsec(_tsys);
        while (sod_temp >= 86400)
        {
            sod_temp -= 86400;
        }
        while (sod_temp < 0)
        {
            sod_temp += 86400;
        }
        _sod_conv = int(sod_temp);

        _dsec_conv = _dsec + tai_tsys_dsec(_tsys);
        while (_dsec_conv >= 1.0)
        {
            _dsec_conv -= 1.0;
        }
        while (_dsec_conv < 0.0)
        {
            _dsec_conv += 1.0;
        }
    }

    t_gtime t_gtime::_tai_to()
    {
        _sod += tai_tsys(_tsys);
        _dsec += tai_tsys_dsec(_tsys);
        _norm_dsec();
        _norm_sod();

        return *this;
    }

    int t_gtime::tai_tsys(const t_tsys &ts) const
    {
        double sec = 0.0;
        switch (ts)
        {
        case USER:
            break;
        case TAI:
            break;
        case UTC:
            sec -= leapsec();
            break;
            ;
        case GPS:
            sec -= TAI_GPS;
            break;
            ;
        case GLO:
            sec -= leapsec();
            sec += tzdiff();
            break;
            ;
        case GAL:
            sec -= TAI_GAL;
            break;
            ;
        case BDS:
            sec -= TAI_BDS;
            break;
            ;
        case LOC:
            sec -= leapsec();
            sec += dstime();
            sec += tzdiff();
            break;
            ;
        case TT:
            sec += TAI_TT;
            break;
            ;
        default:
            break;
            ;
        }

        return sec;
    }

    double t_gtime::tai_tsys_dsec(const t_tsys &ts) const
    {
        double sec = 0.0;
        switch (ts)
        {
        case TT:
            sec += TAT_TT_DSEC;
            break;
            ;
        default:
            break;
            ;
        }
        return sec;
    }

    void t_gtime::_norm_dsec()
    {
        while (_dsec >= 1.0)
        {
            _dsec -= 1.0;
            _sod += 1;
        }
        while (_dsec < 0.0)
        {
            _dsec += 1.0;
            _sod -= 1;
        }
    }

    void t_gtime::_norm_sod()
    {
        while (_sod >= 86400)
        {
            _sod -= 86400;
            _mjd += 1;
        }
        while (_sod < 0)
        {
            _sod += 86400;
            _mjd -= 1;
        }
    }

    void t_gtime::_norm_year(int &year) const
    {
        if (year < 100.0)
            year += year < 80 ? 2000 : 1900;
    }

    int t_gtime::_ymd_mjd(const int &yr, const int &mn, const int &dd) const
    {
        int year(yr);
        int mon(mn);
        _norm_year(year);

        double mjd = 0.0;
        if (mon <= 2)
        {
            mon = mon + 12;
            year = year - 1;
        }

        mjd = 365.25 * year - fmod(365.25 * year, 1.0) - 679006.0;
        mjd += floor(30.6001 * (mon + 1)) + 2.0 - floor(year / 100) + floor(year / 400) + dd;
        return int(mjd);
    }

    void t_gtime::gmsleep(unsigned int ms) // milisecond
    {

#if defined __linux__
        usleep(ms * 1000);
#elif defined __APPLE__
        usleep(ms * 1000);
#elif defined _WIN32 || defined _WIN64
        Sleep(ms);
#else
        clock_t goal = ms + clock();
        while (goal > clock())
            ;
#endif
    }

    void t_gtime::gusleep(unsigned int us)
    {
#if defined __linux__
        usleep(us);
#elif defined __APPLE__
        usleep(us);
#elif defined _WIN32 || defined _WIN64
        this_thread::sleep_for(chrono::microseconds(us));                                                                                                                                                                           
#endif
    }

    void t_gtime::yeardoy2monthday(int year, int doy, int *month, int *day)
    {
        int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        days_in_month[1] = 28;
        int id;

        if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
        {
            days_in_month[1] = 29;
        }

        id = doy;
        for (*month = 1; *month <= 12; (*month)++) 
        {
            id = id - days_in_month[*month - 1];
            if (id > 0)
                continue;
            else
            {
                *day = id + days_in_month[*month - 1];
                break;
            }
        }

        return;
    }

    bool operator<(double left, const t_gtime &t)
    {
        return (left < t.sow() + t.dsec());
    }

    bool operator<(const t_gtime &t, double left)
    {
        return (t.sow() + t.dsec() < left);
    }

    t_gtime MAX_TIME(const t_gtime &A, const t_gtime &B)
    {
        return A > B ? A : B;
    }

    t_gtime MAX_TIME(const vector<t_gtime> &A)
    {
        t_gtime tmax;
        if (A.empty())
            return tmax;
        tmax = A[0];
        for (const auto &iter : A)
        {
            if (tmax < iter)
                tmax = iter;
        }
        return tmax;
    }

    t_gtime MIN_TIME(const t_gtime &A, const t_gtime &B)
    {
        return A > B ? B : A;
    }

    t_gtime MIN_TIME(const vector<t_gtime> &A)
    {
        t_gtime tmin;
        if (A.empty())
            return tmin;
        tmin = A[0];
        for (const auto &iter : A)
        {
            if (tmin > iter)
                tmin = iter;
        }
        return tmin;
    }

} // namespace
