/**
* @verbatim
    History
    2011-01-10  JD: created
    2018-07-13  JD: updated (fixed current_time, revised FIRST/LAST time, speeded etc.)

* @endverbatim

 Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com).
* Internal representation is in TAI time-system (TS)
* If input is in other TS, it needs to be set via constructor
* and all inputs are then converted from TS to TAI.
* All outputs are implicitely in TAI thus use of
* gps(), utc(), local() duplicating function is required.
* Input TS can be changed only via convert(TS) function.
*
* @file        gtime.h
* @brief       Purpose: implements gtime class (day and precise time)
*.
* @author      JD
* @version     1.0.0
* @date        2011-01-10
*
*/

#ifndef GTIME_H
#define GTIME_H

#include "gexport/ExportLibGnut.h"
#include <time.h>
#include <string>
#include <vector>


namespace gnut
{

#if defined GTIME_TAI
#ifndef DEFAULT_TIME
#define DEFAULT_TIME t_gtime::t_tsys::TAI // default gtime TAI
#endif
#elif defined GTIME_UTC
#ifndef DEFAULT_TIME
#define DEFAULT_TIME t_gtime::t_tsys::UTC // default gtime UTC
#endif
#elif defined GTIME_USER
#ifndef DEFAULT_TIME
#define DEFAULT_TIME t_gtime::t_tsys::USER // default gtime USER
#endif
#elif defined GTIME_LOC
#ifndef DEFAULT_TIME
#define DEFAULT_TIME t_gtime::t_tsys::LOC // default gtime LOCAL
#endif
#elif defined GTIME_GPS
#ifndef DEFAULT_TIME
#define DEFAULT_TIME t_gtime::t_tsys::GPS // default gtime GPS
#endif
#else
#ifndef DEFAULT_TIME
#define DEFAULT_TIME t_gtime::t_tsys::GPS // default gtime GPS
#endif
#endif

#ifndef MAX_DT
#define MAX_DT 21 + 1 /// number of time string identifiers /16 -> 21/
#endif
#ifndef MAXLEAP
#define MAXLEAP 50    ///< maximum number of leap second in table
#endif
#ifndef TAI_GPS
#define TAI_GPS 19    ///< seconds of which GPS is ahead of TAI at 6.1.1980
#endif
#ifndef TAI_BDS
#define TAI_BDS 33    ///< seconds of which BDS is ahead of TAI at 1.1.2006
#endif
#ifndef TAI_GAL
#define TAI_GAL 19    ///< coincide with GPS time
#endif
#ifndef TAI_TT
#define TAI_TT 32     ///< TAI TT
#endif
#ifndef TAT_TT_DSEC
#define TAT_TT_DSEC 0.184
#endif
#ifndef CONV_BWK2GWK
#define CONV_BWK2GWK 1356 ///< convert BDS to GPS week
#endif

    /** @brief month string. */
    const static std::string MON[13] = {"   ", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    /** @brief days in a month. */
    static const int monthdays[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /** @brief leap second struct. */
    struct leapsec_table
    {
        int _year; ///< year
        int _mon;  ///< month
        int _day;  ///< day
        int _leap; ///< leap second
    };

    /** @brief leap second structs. */
    const static struct leapsec_table leapseconds[] = {
        {1971, 12, 31, 11}, {1972, 12, 31, 12}, {1973, 12, 31, 13}, {1974, 12, 31, 14}, {1975, 12, 31, 15}, {1976, 12, 31, 16},
        {1977, 12, 31, 17}, {1978, 12, 31, 18}, {1979, 12, 31, 19}, {1981, 06, 30, 20}, {1982, 06, 30, 21}, {1983, 06, 30, 22},
        {1985, 06, 30, 23}, {1987, 12, 31, 24}, {1989, 12, 31, 25}, {1990, 12, 31, 26}, {1992, 06, 30, 27}, {1993, 06, 30, 28},
        {1994, 06, 30, 29}, {1995, 12, 31, 30}, {1997, 06, 30, 31}, {1998, 12, 31, 32}, {2005, 12, 31, 33}, {2008, 12, 31, 34},
        {2011, 06, 30, 35}, {2015, 06, 30, 36}, {2017, 01, 01, 37}};

    /** @brief supported io keys of fixed size ! */
    const static std::string TD[MAX_DT] = {
        "Y", /// [4] year            (1900..2099)
        "y", /// [2] 2-dig year      (80..99,00..79)
        "m", /// [2] month           (1..12+)
        "b", /// [3] month           (Jan..Dec)
        "d", /// [2] day of month    (1..31+)
        "j", /// [3] day of year     (0..365+)
        "H", /// [2] hours           (0..23+)
        "M", /// [2] minutes         (0..59+)
        "S", /// [2] seconds         (0..59+)
        "W", /// [4] GPS week        (0000..XXXX)
        "w", /// [1] day of week     (0..6)
        "v", /// [6] seconds of week (0..604800+)
        "s", /// [5] seconds of day  (0..86400+)
        "J", /// [5]  modified julian date = integer only ! (e.g. 55725)
        "I", /// [11] modified julian date                  (e.g. 55725.81712)
        "T", /// [3] time-system string
        "C", /// [2] month for lsqclk, when single , no zero  ( 1, 2,...,10,11,12)
        "L", /// [2] day for lsqclk, when single , no zero  ( 1, 2,...,10,...,29,..)
        "K", /// [2] hour for lsqclk, when single , no zero  ( 1, 2,...,10,...,24)
        "O", /// [2] minutes for lsqclk, when single , no zero  ( 1, 2,...,10,...,60)
        "P", /// [9] seconds for lsqclk, when single , no zero for ten digit.( 1.000000, ...,59.000000)
        "p", /// [12] seconds  double (30.0000000 )
    };

    /** @brief conversion to GAL + GLO are not implemented */
    const static char t_tstr[8][4] = {"USR", "TAI", "UTC", "LOC", "GPS", "GAL", "GLO", "BDS"};

    /** @brief class for t_gtime */
    class LibGnut_LIBRARY_EXPORT t_gtime
    {
    public:
        /** @brief time types */
        enum class t_tsys
        {
            USER, ///< user time
            TAI,  ///< TAI
            UTC,  ///< UTC
            LOC,  ///< local time
            GPS,  ///< GPST
            GAL,  ///< Galileo time
            GLO,  ///< Glonass time
            BDS,  ///< BDS time
            TT    ///< TT
        };

        /** @brief time types to string */
        static std::string tsys2str(const t_tsys& ts);

        /** @brief string to t_tsys*/
        static t_tsys str2tsys(const std::string& s);

        /** @brief initiate with current time !*/
        explicit t_gtime(const t_tsys& ts = DEFAULT_TIME);

        /**
         * @brief constructor 1.
         * @param[in]  head        ambflag file head data.
         * @return
         */
        explicit t_gtime(const time_t& tt, const double& dsec = 0, const t_tsys& ts = DEFAULT_TIME);

        /**
         * @brief constructor 2.
         * @param[in]  head        ambflag file head data.
         * @return
         */
        t_gtime(const int& yr,
                const int& mn,
                const int& dd,
                const int& hr,
                const int& mi,
                const int& sc,
                const double& ds = 0.0,
                const t_tsys& ts = DEFAULT_TIME);

        /**
         * @brief constructor 3.
         * @param[in]  head        ambflag file head data.
         * @return
         */
        t_gtime(const int& gw, const int& dw, const int& sd, const double& ds = 0.0, const t_tsys& ts = DEFAULT_TIME);

        /**
         * @brief constructor 4.
         * @param[in]  head        ambflag file head data.
         * @return
         */
        t_gtime(const int& gw, const double& sow, const t_tsys& ts = DEFAULT_TIME);

        /**
         * @brief constructor 5.
         * @param[in]  head        ambflag file head data.
         * @return
         */
        t_gtime(const int& mjd, const int& sd, const double& ds = 0.0, const t_tsys& ts = DEFAULT_TIME);

        /** @brief default destructor. */
        ~t_gtime();

        // returning only static functions
        /** @brief get current time. */
        static t_gtime current_time(const t_tsys& ts = DEFAULT_TIME);

        // set functions
        /**
         * @brief set from time_t,dsec.
         * @param[in]    conv    true: different sys; false: the same sys.
         */
        int from_time(const time_t& tt, const double& ds = 0.0, const bool& conv = true);

        /** @brief set from GPSwk,sow,dsec. */
        int from_gws(const int& gw, const double& sow, const bool& conv = true);

        /** @brief set from GPSwk+dw,sod,dsec. */
        int from_gwd(const int& gw, const int& dw, const int& sd, const double& ds = 0.0, const bool& conv = true);

        /** @brief set from yr+mn+dd,sod,dsec. */
        int from_ymd(const int& yr, const int& mn, const int& dd, const int& sd, const double& ds = 0.0, const bool& conv = true);

        /** @brief set from yr+mn+dd + H+M+S. */
        int from_ymdhms(const int& yr, const int& mn, const int& dd, const int& h, const int& m, const double& s, const bool& conv = true);

        /** @brief set from mjd,sod,dsec. */
        int from_mjd(const int& mjd, const int& sd, const double& ds = 0.0, const bool& conv = true);

        /** @brief set from mjd. */
        int from_dmjd(const double& dmjd, const bool& conv = true);

        /** @brief set from defined string. */
        int from_str(const std::string& ifmt, const std::string& dat, const bool& conv = true);

        /** @brief reset dsec only (in TAI)!. */
        int reset_dsec();

        /** @brief reset sod + dsec only (in TAI)!. */
        int reset_sod();

        /** @brief set new io time-system. */
        int tsys(const t_tsys& ts)
        {
            _tsys = ts;
            _reset_conv();
            return 0;
        }

        // get functions (default includes conversion to _tsys, if conv=false return TAI)
        int mjd(const bool& conv = true) const;          ///< get MJD
        double dmjd(const bool& conv = true) const;      ///< get MJD
        double dmjd_vlbi(const bool& conv = true) const; ///< get MJD for VLBI
        int sod(const bool& conv = true) const;          ///< get seconds of day
        double dsec(const bool& conv = true) const;      ///< get fractional sec

        // get functions (always includes conversion to _tsys, for TAI use only mjd, sod and dsec)
        time_t tim() const;                     ///< get time_t
        int gwk() const;                        ///< get GPS week
        int bwk() const;                        ///< get BDS week
        int dow() const;                        ///< get integer day of GPS week
        int sow() const;                        ///< get integer Sec of Week
        int year() const;                       ///< get integer 4-char year
        int yr() const;                         ///< get integer 2-char year (1980-2079 only)
        int doy(const bool& conv = true) const; ///< get integer day of year
        int day() const;                        ///< get integer day of month
        int mon() const;                        ///< get integer day
        int hour() const;                       ///< get integer hour
        int mins() const;                       ///< get integer minute
        int secs() const;                       ///< get integer seconds
        std::string sys() const;                     ///< get time system identifier

        void hms(int& h, int& m, int& s,
                 bool conv = true) const; ///< get hour, minute, seconds
        void ymd(int& y, int& m, int& d,
                 bool conv = true) const; ///< get year, month, day
        std::string str(const std::string& ofmt = "%Y-%m-%d %H:%M:%S",
                   const bool& conv = true) const; ///< get any string
        std::string str_ymd(const std::string& str = "",
                       const bool& conv = true) const; ///< get fixed date string
        std::string str_hms(const std::string& str = "",
                       const bool& conv = true) const; ///< get fixed time string
        std::string str_ydoysod(const std::string& str = "",
                           const bool& conv = true) const; ///< get year-doy-sod
        std::string str_ymdhms(const std::string& str = "", const bool& ts = true,
                          const bool& conv = true) const; ///< get fixed date and time string
        std::string str_mjdsod(const std::string& str = "", const bool& ts = true,
                          const bool& conv = true) const; ///< get mjd-sod

        std::string str_yyyydoy(const bool& conv = true) const; ///< get yyyydoy add by glfeng
        std::string str_yyyy(const bool& conv = true) const;    ///< get year-string
        std::string str_doy(const bool& conv = true) const;     ///< get doy-string
        std::string str_gwkd(const bool& conv = true) const;    ///< get gpsweek and day -string
        std::string str_gwk(const bool& conv = true) const;     ///< get gpsweek -string
        std::string str_yr(const bool& conv = true) const;      ///< get 2-char year
        std::string str_mon(const bool& conv = true) const;     ///< get month -string
        std::string str_day(const bool& conv = true) const;     ///< get day -string
        std::string str_hour(const bool& conv = true) const;    ///< get hour -string
        std::string str_min(const bool& conv = true) const;     ///< get min -string
        // general conversion functions
        int yr(const int& y) const;       ///< get integer 2-char year (1980-2079 only)
        std::string mon(const int& m) const;   ///< get month string
        int mon(const std::string& str) const; ///< get month number

        int tzdiff() const;                           ///< get UTC-LOC difference [sec]
        int dstime() const;                           ///< get day-saving time [sec]
        int leapsec() const;                          ///< get TAI-UTC leap seconds [sec]
        int leapyear(const int& y) const;             ///< get YEAR with leapsec [0,1]
        int tai_tsys(const t_tsys& ts) const;         ///< get TAI-tsys difference [sec]
        double tai_tsys_dsec(const t_tsys& ts) const; ///< get TAI-tsys difference [dsec]
        t_tsys tsys() const
        {
            return _tsys;
        } ///< get required inp/out time-system

        void del_secs(const int& sec);       ///< date/time with deleted X-seconds,lvhb added in 20210413
        void add_secs(const int& sec);       ///< date/time with added X-seconds
        void add_dsec(const double& dsec);   ///< date/time with added X-dseconds
        double diff(const t_gtime& t) const; ///< time difference (this - t) [s]

        /** @brief override operator. */
        bool operator<(const t_gtime& t) const;
        bool operator<=(const t_gtime& t) const;
        bool operator>=(const t_gtime& t) const;
        bool operator>(const t_gtime& t) const;
        bool operator==(const t_gtime& t) const;
        bool operator!=(const t_gtime& t) const;
        double operator-(const t_gtime& t) const; // [sec]
        bool operator<(const double& t) const;
        t_gtime operator-(const double& sec) const; // gtime
        t_gtime operator+(const double& sec) const; // gtime
        t_gtime operator=(const t_gtime& t);
        friend bool operator<(double left, const t_gtime& t);
        friend bool operator<(const t_gtime& t, double left);

        /** @brief multiplatform msleep function [ms]. */
        static void gmsleep(unsigned int ms);

        /** @brief multiplatform usleep function [us]. */
        static void gusleep(unsigned int us);

        /** @brief convert yeardoy to month_day. */
        void yeardoy2monthday(int year, int doy, int* month, int* day);

    protected:
    private:
        int _mjd_conv;      ///< mjd conv
        int _sod_conv;      ///< sod conv
        double _dsec_conv;  ///< dsec conv
        void _reset_conv(); ///< reset conv

        int _mjd;          ///< integer MJD [TAI]
        int _sod;          ///< seconds of day [TAI]
        double _dsec;      ///< fractional seconds
        t_tsys _tsys;      ///< time system
        t_gtime _tai_to(); ///< conversion from TAI to TS with changing gtime !
        void _to_tai();    ///< conversion from TS to TAI with changing gtime !

        void _norm_dsec();                ///< normalize dsec (range in 0-1 sec)
        void _norm_sod();                 ///< normalize sod  (range in 0-86399 sec)
        void _norm_year(int& year) const; ///< normalize year (range 1980 - 2079)
        int _ymd_mjd(const int& yr, const int& mn, const int& dd) const;

        /** @brief ymd type. */
        enum class t_tymd
        {
            _ymd = 0,
            _year = 1,
            _mon = 2,
            _day = 4
        };

        /** @brief hms type. */
        enum class t_thms
        {
            _hms = 0,
            _hour = 1,
            _min = 2,
            _sec = 4
        };
    };

    const static t_gtime FIRST_TIME(44239, 0, 0.0,
                                    t_gtime::t_tsys::TAI); ///< first valid time  "1980-01-01 00:00:00"
    const static t_gtime LAST_TIME(80763, 86399, 0.0,
                                   t_gtime::t_tsys::TAI); ///< last  valid time  "2079-12-31 23:59:59"

    /** @brief max/min time. */
    LibGnut_LIBRARY_EXPORT t_gtime MAX_TIME(const t_gtime& A, const t_gtime& B);
    LibGnut_LIBRARY_EXPORT t_gtime MAX_TIME(const std::vector<t_gtime>& A);
    LibGnut_LIBRARY_EXPORT t_gtime MIN_TIME(const t_gtime& A, const t_gtime& B);
    LibGnut_LIBRARY_EXPORT t_gtime MIN_TIME(const std::vector<t_gtime>& A);

} // namespace gnut

#endif
