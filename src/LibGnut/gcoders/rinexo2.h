/**
*
* @verbatim
    History
    2011-11-04  JD: created
    2012-09-24  JD: RINEX 2.11 implemented
    2012-09-27  JD: support of multi-GNSS (phase not converted to [m])
    2013-03-08  JD: filtering via general settings
    2014-05-03  PV: gobj getting information from rinexo header
    2014-11-19  JD: shared pointers
    2014-12-21  JD: advanced request filtering
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     rinexo2.h
* @brief    Purpose: Obs RINEX encoder/decoder
*    Todo:  header information completed
            encoders implementation
* @author   JD
* @version  1.0.0
* @date     2011-11-04
*
*/

#ifndef RINEXO2_H
#define RINEXO2_H

#include <string>
#include <vector>

#include "gall/gallobs.h"
#include "gall/gallobj.h"
#include "gutils/gtime.h"
#include "gutils/gtriple.h"
#include "gutils/gsys.h"
#include "gutils/gobs.h"
#include "gutils/gtypeconv.h"
#include "gcoders/gcoder.h"
#include "gdata/grnxhdr.h"

using namespace std;

namespace gnut
{
    /**
    *@brief Class for t_rinexo2 derive from t_gcoder
    */
    class LibGnut_LIBRARY_EXPORT t_rinexo2 : public t_gcoder
    {
    public:
        /** @brief constructor set + version + sz. */
        t_rinexo2(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief constructor beg + end + set + version + sz. */
        t_rinexo2(t_gtime beg, t_gtime end, t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief default destructor. */
        virtual ~t_rinexo2();

        /** @brief clear. */
        virtual void clear();

        /** @brief decode head. */
        virtual int decode_head(char *buff, int sz, vector<string> &errmsg) = 0;

        /**
         * @brief decode data
         * 
         * @param buff 
         * @param sz 
         * @param cnt 
         * @param errmsg 
         * @return int 
         */
        virtual int decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg) = 0;

        /** @brief get epoch. */
        t_gtime get_epoch() { return _epoch; };

    protected:
        /** @brief decode head. */
        virtual int _decode_head();

        /** @brief decode data. */
        virtual int _decode_data();

        /** @brief read epoch & number of satellites, return flag. */
        virtual int _read_epoch();

        /** @brief read sat/sys-specific key to _mapobs (G,E,R,.. M, satellite). */
        virtual int _read_syskey(string &sat, string &key);

        /** @brief read satellite list (vector). */
        virtual int _read_satvec(vector<string> &satellites);

        /** @brief read single satellite observation types. */
        virtual int _read_obstypes(const string &sat, const string &sys);

        /** @brief fill single observation element. */
        virtual int _read_obs(const unsigned int &idx,
                              const t_rnxhdr::t_vobstypes::const_iterator &it,
                              t_spt_gobs obs);

        /** @brief fill header information. */
        virtual int _fill_head();

        /** @brief fill observation data structure. */
        virtual int _fill_data();

        /** @brief fill header information. */
        virtual int _check_head();

        /** @brief check band (extended RINEX 2.11). */
        virtual int _fix_band(string sys, string &go);

        /** @brief write log for null observations (sat, obstype). */
        virtual int _null_log(const string &sat, const string &obstype);

        /** @brief common stop reading used in local subroutines. */
        virtual int _stop_read();

        string _csys;             ///< A1 (G=GPS, R=GLO, E=GAL, S=SBAS, M=Mix), but may be also G11, ..
        t_gtime _epoch;           ///< working epoch
        string _line;             ///< working line read from
        string _site;             ///< working site
        int _tmpsize;             ///< working amount of bytes processed
        int _consume;             ///< working total amount of bytes (return)
        bool _complete;           ///< working flag for completed epoch decoding
        char _flag;               ///< working special event flag
        vector<t_spt_gobs> _vobs; ///< working GNSS observation vector
        int _nsat;                ///< working # of satellites
        int _count;               ///< working # of epochs

        t_gtime _epo_beg; ///< begin time
        t_gtime _epo_end; ///< end time

        int _xbeg; ///< # epochs filtered before BEG
        int _xend; ///< # epochs filtered after  END
        int _xsmp; ///< # epochs filtered by sampling
        int _xsys; ///< # obs data filtered by systems

        t_rnxhdr _rnxhdr;             ///< Rinex header instance
        vector<string> _comment;      ///< Rinex comments
        vector<char> _pcosat;         ///< GNSS (G/R/E/S/...) for antenna phase center vs. ARP
        vector<string> _pcosys;       ///< XYZ or NEU system for pco
        vector<t_gtriple> _pcoecc;    ///< PCO-ARP values in system above (XYZ, NEU) [m]
        t_rnxhdr::t_obstypes _mapobs; ///< GNSS/sat (G/R/E/S/...) observation group
    };

} // namespace

#endif
