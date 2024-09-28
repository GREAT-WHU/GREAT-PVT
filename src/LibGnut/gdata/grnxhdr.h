/**
*
* @verbatim
    History
    2011-01-10  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       grnxhdr.h
* @brief      Purpose: implements receiver class
*.
* @author     JD
* @version    1.0.0
* @date       2011-01-10
*
*/
#ifndef T_RNXHDR
#define T_RNXHDR

#include <string>
#include <vector>

#include "gutils/gtime.h"
#include "gutils/gtriple.h"
#include "gdata/gobsgnss.h"

namespace gnut
{

    class LibGnut_LIBRARY_EXPORT t_rnxhdr
    {
    public:
        /** @brief default constructor. */
        t_rnxhdr();

        /** @brief default destructor. */
        ~t_rnxhdr();

        typedef vector<pair<GOBS, double>> t_vobstypes;
        typedef map<string, t_vobstypes> t_obstypes;

        /**
         * @brief get path
         * 
         * @return string 
         */
        string path() const { return _path; } 

        /**
         * @brief set path
         * 
         * @param s 
         */
        void path(const string &s) { _path = s; } 

        /**
         * @brief get rinex system
         * 
         * @return char 
         */
        char rnxsys() { return _rnxsys; } 

        /**
         * @brief set rinex system
         * 
         * @param s 
         */
        void rnxsys(const char &s) { _rnxsys = s; } 

        /**
         * @brief get rinex version
         * 
         * @return const string& 
         */
        const string &rnxver() const { return _rnxver; } 

        /**
         * @brief set rinex version
         * 
         * @param s 
         */
        void rnxver(const string &s) { _rnxver = s; } 

        /**
         * @brief get program
         * 
         * @return string 
         */
        string program() const { return _program; } 

        /**
         * @brief set program
         * 
         * @param pgm 
         */
        void program(const string &pgm) { _program = pgm; }

        /**
         * @brief get runby
         * 
         * @return string 
         */
        string runby() const { return _runby; } 

        /**
         * @brief set runby
         * 
         * @param rnb 
         */
        void runby(const string &rnb) { _runby = rnb; } 

        /**
         * @brief get gtime
         * 
         * @return t_gtime 
         */
        t_gtime gtime() const { return _gtime; } 

        /**
         * @brief set gtime
         * 
         * @param t 
         */
        void gtime(const t_gtime &t) { _gtime = t; } 

        /**
         * @brief get comment
         * 
         * @return vector<string> 
         */
        vector<string> comment() { return _comment; } 

        /**
         * @brief set comment
         * 
         * @param cmt 
         */
        void comment(const vector<string> &cmt) { _comment = cmt; } 

        /**
         * @brief get markname
         * 
         * @return string 
         */
        string markname() const { return _markname; } 

        /**
         * @brief set markname
         * 
         * @param mrk 
         */
        void markname(const string &mrk) { _markname = mrk; } 

        /**
         * @brief set mark number
         * 
         * @return string 
         */
        string marknumb() const { return _marknumb; } 

        /**
         * @brief get mark number
         * 
         * @param mnb 
         */
        void marknumb(const string &mnb) { _marknumb = mnb; } 

        /**
         * @brief get mark type
         * 
         * @return string 
         */
        string marktype() const { return _marktype; } 

        /**
         * @brief set mark type
         * 
         * @param mtp 
         */
        void marktype(const string &mtp) { _marktype = mtp; } 

        /**
         * @brief get observer
         * 
         * @return string 
         */
        string observer() const { return _observer; } 

        /**
         * @brief set observer
         * 
         * @param obsrv 
         */
        void observer(const string &obsrv) { _observer = obsrv; } 

        /**
         * @brief get agency
         * 
         * @return string 
         */
        string agency() const { return _agency; }

        /**
         * @brief set agency
         * 
         * @param agn 
         */
        void agency(const string &agn) { _agency = agn; } 

        /**
         * @brief get recnumb
         * 
         * @return string 
         */
        string recnumb() const { return _recnumb; } 

        /**
         * @brief set recnumb
         * 
         * @param rnb 
         */
        void recnumb(const string &rnb) { _recnumb = rnb; } 

        /**
         * @brief get rectype
         * 
         * @return string 
         */
        string rectype() const { return _rectype; }

        /**
         * @brief set rectype
         * 
         * @param rtp 
         */
        void rectype(const string &rtp) { _rectype = rtp; } 

        /**
         * @brief set recvers
         * 
         * @return string 
         */
        string recvers() const { return _recvers; } 

        /**
         * @brief get recvers
         * 
         * @param rvs 
         */
        void recvers(const string &rvs) { _recvers = rvs; }

        /**
         * @brief get antnumb
         * 
         * @return string 
         */
        string antnumb() const { return _antnumb; } 

        /**
         * @brief set antnumb
         * 
         * @param ant 
         */
        void antnumb(const string &ant) { _antnumb = ant; } 

        /**
         * @brief get anttype
         * 
         * @return string 
         */
        string anttype() const { return _anttype; } 

        /**
         * @brief set anttype
         * 
         * @param ant 
         */
        void anttype(const string &ant) { _anttype = ant; } 

        /**
         * @brief get apr position
         * 
         * @return t_gtriple 
         */
        t_gtriple aprxyz() const { return _aprxyz; }

        /**
         * @brief set apr position
         * 
         * @param apr 
         */
        void aprxyz(const t_gtriple &apr) { _aprxyz = apr; } 

        /**
         * @brief get ant position
         * 
         * @return t_gtriple 
         */
        t_gtriple antxyz() const { return _antxyz; } 

        /**
         * @brief set ant position
         * 
         * @param ecc 
         */
        void antxyz(const t_gtriple &ecc) { _antxyz = ecc; } 

        /**
         * @brief get ant neu
         * 
         * @return t_gtriple 
         */
        t_gtriple antneu() const { return _antneu; }         
        void antneu(const t_gtriple &ecc) { _antneu = ecc; } 

        /**
         * @brief get strength
         * 
         * @return string 
         */
        string strength() const { return _strength; } 

        /**
         * @brief set strength
         * 
         * @param s 
         */
        void strength(const string &s) { _strength = s; } 

        /**
         * @brief get interval
         * 
         * @return double 
         */
        double interval() const { return _interval; } 

        /**
         * @brief set interval
         * 
         * @param i 
         */
        void interval(const double &i) { _interval = i; } 

        /**
         * @brief get first epoch
         * 
         * @return t_gtime 
         */
        t_gtime first() const { return _first; } 

        /**
         * @brief set first epoch
         * 
         * @param frst 
         */
        void first(const t_gtime &frst) { _first = frst; } 

        /**
         * @brief get last epoch
         * 
         * @return t_gtime 
         */
        t_gtime last() const { return _last; } 

        /**
         * @brief set last epoch
         * 
         * @param lst 
         */
        void last(const t_gtime &lst) { _last = lst; } 

        /**
         * @brief get leap second
         * 
         * @return int 
         */
        int leapsec() const { return _leapsec; } 

        /**
         * @brief set leap second
         * 
         * @param ls 
         */
        void leapsec(int ls) { _leapsec = ls; } 

        /**
         * @brief get number of satellite
         * 
         * @return int 
         */
        int numsats() const { return _numsats; } 

        /**
         * @brief set number of satellite
         * 
         * @param nums 
         */
        void numsats(const int &nums) { _numsats = nums; } 

        /**
         * @brief get mapobs
         * 
         * @return t_obstypes 
         */
        t_obstypes mapobs() const { return _mapobs; } 

        /**
         * @brief set mapobs
         * 
         * @param types 
         */
        void mapobs(const t_obstypes &types) { _mapobs = types; }

        /**
         * @brief get mapcyc
         * 
         * @return t_obstypes 
         */
        t_obstypes mapcyc() const { return _mapcyc; } 

        /**
         * @brief set mapcyc
         * 
         * @param types 
         */
        void mapcyc(const t_obstypes &types) { _mapcyc = types; } 

        /**
         * @brief get glonass frequency
         * 
         * @return t_obstypes 
         */
        t_obstypes glofrq() const { return _glofrq; } 

        /**
         * @brief set glonass frequency
         * 
         * @param types 
         */
        void glofrq(const t_obstypes &types) { _glofrq = types; }

        /**
         * @brief get glonass bias
         * 
         * @return t_vobstypes 
         */
        t_vobstypes globia() const { return _globia; }

        /**
         * @brief set glonass bias
         * 
         * @param types 
         */
        void globia(const t_vobstypes &types) { _globia = types; } 

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
        friend ostream &operator<<(ostream &os, const t_rnxhdr &x); 

    private:
        char _rnxsys;            ///< G=GPS, R=GLO, E=GAL, S=SBAS, M=Mix
        string _path;            ///< rinex file path
        string _rnxver;          ///< rinex version
        string _program;         ///< name of program creating RINEX file
        string _runby;           ///< name of agency  creating RINEX file
        t_gtime _gtime;          ///< name of date and file of RINEX creation
        vector<string> _comment; ///< vector of comments
        string _markname;        ///< marker name
        string _marknumb;        ///< marker number
        string _marktype;        ///< marker type
        string _observer;        ///< name of observer
        string _agency;          ///< name of agency
        string _recnumb;         ///< receiver number
        string _rectype;         ///< receiver type
        string _recvers;         ///< receiver version
        string _antnumb;         ///< antenna number
        string _anttype;         ///< antenna type
        t_gtriple _aprxyz;       ///< approximate xyz position [m]
        t_gtriple _antxyz;       ///< antenna xyx eccentricities [m]
        t_gtriple _antneu;       ///< antenna north/east/up eccentricities [m]
        string _strength;        ///< signal strength [DBHZ/...]
        double _interval;        ///< interval [sec]
        t_gtime _first;          ///< time of first observation
        t_gtime _last;           ///< time of last observation
        int _leapsec;            ///< leapseconds since 6-Jan-1980
        int _numsats;            ///< number of satellites
        t_obstypes _mapobs;      ///< map of GOBS and scaling factors
        t_obstypes _mapcyc;      ///< map of GOBS phase quater-cycle shifts
        t_obstypes _glofrq;      ///< map of GLONASS slot/frequency
        t_vobstypes _globia;     ///< vec of GLONASS obs code-phase biases
    };

} // namespace

#endif
