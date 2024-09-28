/**
*
* @verbatim
    History
*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     gallobs.h
* @brief    container for all observation
*
* @author   JD
* @version  1.0.0
* @date     2012-05-02
*
*/

#ifndef GALLOBS_H
#define GALLOBS_H

#include <iostream>
#include <string.h>
#include <map>
#include <set>
#include <memory>

#include "gdata/gdata.h"
#include "gdata/gobsgnss.h"
#include "gdata/gsatdata.h"
#include "gutils/gtypeconv.h"
#include "gutils/gconst.h"
#include "gutils/gtime.h"
#include "gutils/gnss.h"
#include "gutils/gsys.h"
#include "gutils/gobs.h"
#include "gset/gsetgen.h"

#define DIFF_SEC_NOMINAL 0.905 ///< [sec] returns observations within +- DIFF_SEC for 1Hz

// normalize for 1Hz !
#define DIFF_SEC(a) (((a) > (0.0) && (a) < (1.0)) ? (DIFF_SEC_NOMINAL * a) : (DIFF_SEC_NOMINAL))

using namespace std;

namespace gnut
{

    typedef shared_ptr<t_gobsgnss> t_spt_gobs;

    /**
    *@brief Class for t_allobs derive from t_gdata
    */
    class LibGnut_LIBRARY_EXPORT t_gallobs : public t_gdata
    {

    public:
        /** @brief default constructor. */
        t_gallobs();

        /** @brief default constructor. */
        t_gallobs(t_spdlog spdlog);
        /** @brief default constructor. */
        t_gallobs(t_spdlog spdlog, t_gsetbase *set);
        /** @brief default destructor. */
        virtual ~t_gallobs();

        /** @brief XDATA type. */
        enum XDATA
        {
            XDATA_BEG,
            XDATA_END,
            XDATA_SMP,
            XDATA_SYS
        }; 

        /**@brief first : XDATA, second : */
        typedef map<XDATA, int> t_map_xdat; 

        struct t_xfilter
        {
            t_map_xdat xdat;
            t_gtime beg, end;
        }; 

        /**@brief first : , second : file filtered data */
        typedef map<string, t_xfilter> t_map_xfil; 
        /**@brief first : , second : all filtered data */
        typedef map<string, t_map_xfil> t_map_xflt;
        /**@brief first : name, second : all data-types/single epoch */
        typedef map<string, t_spt_gobs> t_map_osat;
        /**@brief first : time, second : all data-types/all epochs/single object */
        typedef map<t_gtime, t_map_osat> t_map_oref;
        /**@brief first : time, second : all data-types/all epochs/all objects */
        typedef map<string, t_map_oref> t_map_oobj;
        /**@brief first : , second :GOBSBAND , third :GOBS , fourth: */
        typedef map<string, map<GOBSBAND, map<GOBS, int>>> t_map_frq; 

        /**
        * @brief settings of system&sampling&scalefc.
        *
        * @param[in]  *set        settings
        * @return void
        */
        virtual void gset(t_gsetbase *);

        /**
        * @brief get all stations.
        *
        * @return all stations
        */
        virtual set<string> stations();

        /**
        * @brief if map contains the site.
        *
        * @param[in]  site    sites
        * @return
            if map contains the site        true
            else                            false
        */
        virtual bool isSite(const string &site); 

        /**
        * @brief get all satellites for epoch t and system.
        *
        * @param[in]  site        sites
        * @param[in]  t            time
        * @param[in]  gnss        system type
        * @return
            the list of available satellites.
        */
        virtual set<string> sats(const string &site, const t_gtime &t, GSYS gnss);

        /**
         * @brief get all t_gsatdata for epoch t
         * 
         * @param site 
         * @param t 
         * @return vector<t_gsatdata> 
         */
        virtual vector<t_gsatdata> obs(const string &site, const t_gtime &t); 

        /**
         * @brief get all t_gobsgnss pointers for epoch t
         * 
         * @param site 
         * @param t 
         * @return vector<t_spt_gobs> 
         */
        virtual vector<t_spt_gobs> obs_pt(const string &site, const t_gtime &t); 

        /**
         * @brief  get all t_gepochs for site
         * 
         * @param site 
         * @return vector<t_gtime> 
         */
        virtual vector<t_gtime> epochs(const string &site); 

        /**
         * @brief get first t_gobs epoch for site
         * 
         * @param site 
         * @param smpl 
         * @return t_gtime 
         */
        virtual t_gtime beg_obs(const string &site, double smpl = 0.0); 

        /**
         * @brief get last  t_gobs epoch for site
         * 
         * @param site 
         * @return t_gtime 
         */
        virtual t_gtime end_obs(const string &site); 

        /**
         * @brief add site-specific filtered data/epochs
         * 
         * @param site 
         * @param file 
         * @param xflt 
         */
        void xdata(const string &site, const string &file, const t_xfilter &xflt); 

        /**
         * @brief add single station observation
         * 
         * @param obs 
         * @return int 
         */
        int addobs(t_spt_gobs obs); 

        /**
         * @brief number of epochs for station
         * 
         * @param site 
         * @return unsigned int 
         */
        unsigned int nepochs(const string &site);

        /**
         * @brief find appropriate t_gobsgnss element for site/epoch
         * 
         * @param site 
         * @param t 
         * @return t_map_osat 
         */
        virtual t_map_osat find(const string &site, const t_gtime &t); 

        /**
         * @brief get number of occurance of individual signals
         * 
         * @param site 
         * @return t_map_frq 
         */
        t_map_frq frqobs(const string &site); 
                                              
        /**
         * @brief add site crd
         * 
         * @param site 
         * @param crd 
         */
        void addsitecrd(const string &site, const t_gtriple &crd) { _mapcrds[site] = crd; }

        /**
         * @brief glo freq num
         * 
         * @return map<string, int> 
         */
        const map<string, int> &glo_freq_num() const { return _glofrq; }

        /**
         * @brief add glo freq
         * 
         * @param sat 
         * @param freqNum 
         */
        void add_glo_freq(const string &sat, int freqNum)
        {
            if (_glofrq.find(sat) == _glofrq.end())
                _glofrq[sat] = freqNum;
        }

    protected:
        /**@brief return all sats */
        virtual set<string> _sats(const string &site, const t_gtime &t, GSYS gnss);

        /**@brief return all obs */
        virtual vector<t_gsatdata> _gobs(const string &site, const t_gtime &t);

        /**@brief find epoch from the map */
        int _find_epo(const string &site, const t_gtime &epo, t_gtime &tt); 

    protected:
        t_gsetbase *_set = nullptr;
        unsigned int _nepoch;            ///< maximum number of epochs (0 = keep all)
        t_map_oobj _mapobj;              ///< map over all objects (receivers)
        t_map_xflt _filter;              ///< structure of stations/files filtered data (QC)
        bool _overwrite;                 ///< rewrite/add only mode
        set<string> _sys;                ///< systems settings
        double _smp;                     ///< sampling settings
        double _scl;                     ///< sampling scale-factor
        map<string, t_gtriple> _mapcrds; ///< all sites apr coordinates 
        map<string, int> _glofrq;        ///< map of GLONASS slot/frequency 
        set<string> _map_sites;          ///< map of sites 
    private:
    };

} // namespace

#endif
