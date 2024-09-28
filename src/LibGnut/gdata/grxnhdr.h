/**
*
* @verbatim
    History
    2018-08-03  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        grxnhdr.h
* @brief    Purpose: rinexn header
*.
* @author   JD
* @version    1.0.0
* @date        2018-08-03
*
*/

#ifndef T_RXNHDR
#define T_RXNHDR

#include <string>
#include <vector>
#include <map>
#include <set>

#include "gutils/gtime.h"
#include "gutils/gtriple.h"

namespace gnut
{

    /** @brief system correct. */
    enum TSYS_CORR
    {
        TS_NONE,
        TS_GAUT,
        TS_GPUT,
        TS_SBUT,
        TS_GLUT,
        TS_GPGA,
        TS_GLGP,
        TS_QZGP,
        TS_QZUT,
        TS_BDUT,
        TS_IRUT,
        TS_IRGP
    };

    /** @brief iono correct. */
    enum IONO_CORR
    {
        IO_NONE,
        IO_GAL,
        IO_GPSA,
        IO_GPSB,
        IO_QZSA,
        IO_QZSB,
        IO_BDSA,
        IO_BDSB,
        IO_IRNA,
        IO_IRNB
    };

    /** @brief struct system correct. */
    struct t_tsys_corr
    {
        double a0 = 0.0, a1 = 0.0;
        int T = 0, W = 0;
    };

    /** @brief struct iono correct. */
    struct t_iono_corr
    {
        double x0 = 0.0, x1 = 0.0, x2 = 0.0, x3 = 0.0;
        int T = 0, sat = 0;
    };

    /** @brief map system correct. */
    typedef map<TSYS_CORR, t_tsys_corr> t_map_tsys;

    /** @brief map iono correct. */
    typedef map<IONO_CORR, t_iono_corr> t_map_iono;

    /** @brief convert sys_corr to string. */
    string tsys_corr2str(TSYS_CORR c);

    /** @brief convert iono_corr to string. */
    string iono_corr2str(IONO_CORR c);

    /** @brief convert string to sys_corr. */
    TSYS_CORR str2tsys_corr(string s);

    /** @brief convert string to iono_corr. */
    IONO_CORR str2iono_corr(string s);

    /** @brief class for rinex header information. */
    class t_rxnhdr
    {
    public:
        /** @brief default constructor. */
        t_rxnhdr();

        /** @brief default destructor. */
        ~t_rxnhdr();

        /**
         * @brief get path
         * 
         * @return const string& 
         */
        const string &path() const { return _path; } ///< get path

        /**
         * @brief set path
         * 
         * @param s 
         */
        void path(const string &s) { _path = s; } ///< set path

        /**
         * @brief get rinex system
         * 
         * @return const char& 
         */
        const char &rxnsys() const { return _rxnsys; } ///< get rinex system

        /**
         * @brief set rinex system
         * 
         * @param s 
         */
        void rxnsys(const char &s) { _rxnsys = s; } ///< set rinex system

        /**
         * @brief get rinex receiver
         * 
         * @return const string& 
         */
        const string &rxnver() const { return _rxnver; } ///< get rinex receiver

        /**
         * @brief set rinex receiver
         * 
         * @param s 
         */
        void rxnver(const string &s) { _rxnver = s; } ///< set rinex receiver

        /**
         * @brief get program
         * 
         * @return const string& 
         */
        const string &program() const { return _program; } ///< get program

        /**
         * @brief set program
         * 
         * @param pgm 
         */
        void program(const string &pgm) { _program = pgm; } ///< set program

        /**
         * @brief get runby
         * 
         * @return const string& 
         */
        const string &runby() const { return _runby; } ///< get runby

        /**
         * @brief set program
         * 
         * @param rnb 
         */
        void runby(const string &rnb) { _runby = rnb; } ///< set program

        /**
         * @brief get gtime
         * 
         * @return const t_gtime& 
         */
        const t_gtime &gtime() const { return _gtime; } ///< get gtime

        /**
         * @brief set gtime
         * 
         * @param t 
         */
        void gtime(const t_gtime &t) { _gtime = t; } ///< set gtime

        /**
         * @brief get comment
         * 
         * @return const vector<string>& 
         */
        const vector<string> &comment() const { return _comment; } ///< get comment

        /**
         * @brief set comment
         * 
         * @param cmt 
         */
        void comment(const vector<string> &cmt) { _comment = cmt; } ///< set comment

        /**
         * @brief get leap second
         * 
         * @return const int& 
         */
        const int &leapsec() const { return _leapsec; } ///< get leap second

        /**
         * @brief set leap second
         * 
         * @param ls 
         */
        void leapsec(const int &ls) { _leapsec = ls; } ///< set leap second

        /**
         * @brief system correct
         * 
         * @return set<TSYS_CORR> 
         */
        set<TSYS_CORR> tsys_corr() const; ///< system correct

        /**
         * @brief iono correct
         * 
         * @return set<IONO_CORR> 
         */
        set<IONO_CORR> iono_corr() const; ///< iono correct

        /**
         * @brief get sys_corr
         * 
         * @param c 
         * @return t_tsys_corr 
         */
        t_tsys_corr tsys_corr(const TSYS_CORR &c) const; ///< get sys_corr

        /**
         * @brief set sys_corr
         * 
         * @param c 
         * @param ts 
         */
        void tsys_corr(const TSYS_CORR &c, const t_tsys_corr &ts); ///< set sys_corr

        /**
         * @brief get iono_corr
         * 
         * @param c 
         * @return t_iono_corr 
         */
        t_iono_corr iono_corr(const IONO_CORR &c) const; ///< get iono_corr

        /**
         * @brief set iono_corr
         * 
         * @param c 
         * @param io 
         */
        void iono_corr(const IONO_CORR &c, const t_iono_corr &io); ///< set iono_corr

        /**
         * @brief clear
         * 
         */
        void clear();

        /**
         * @brief override operator
         * 
         * @param os 
         * @param x 
         * @return ostream& 
         */
        friend ostream &operator<<(ostream &os, const t_rxnhdr &x);

    private:
        char _rxnsys;            ///< G=GPS, R=GLO, E=GAL, S=SBAS, M=Mix
        string _path;            ///< rinex file path
        string _rxnver;          ///< rinex version
        string _program;         ///< name of program creating RINEX file
        string _runby;           ///< name of agency  creating RINEX file
        t_gtime _gtime;          ///< name of date and file of RINEX creation
        vector<string> _comment; ///< vector of comments
        int _leapsec;            ///< leapseconds since 6-Jan-1980
        t_map_tsys _ts_corr;     ///< RINEX header tsys corrections
        t_map_iono _io_corr;     ///< RINEX header iono corrections
    };

} // namespace

#endif
