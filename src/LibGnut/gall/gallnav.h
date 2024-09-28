/**
*
* @verbatim
History
-1.0 JD  2011-02-14  creat the file.
-1.1 JD  2018-08-13  update the file
*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     gallnav.h
* @brief    container for all navigation systems
*
*
* @author   PV
* @version  1.1.0
* @date     2018-08-13
*
*/

#ifndef GALLNAV_H
#define GALLNAV_H

#include "gexport/ExportLibGnut.h"
#include "gdata/gdata.h"
#include "gdata/geph.h"
#include "gdata/gnav.h"
#include "gdata/grxnhdr.h"
#include "gutils/gconst.h"
#include "gutils/gtime.h"
#include "gdata/gnavglo.h" 
#include "gdata/gnavgps.h" 
#include "gdata/gnavbds.h"  
#include "gdata/gnavgal.h"  
#include "gdata/gnavqzs.h"   
#include "gdata/gnavsbs.h"   

#define MAX_GPS_PRN 32
#define MAX_GLO_PRN 24
#define MAX_GAL_PRN 30
#define NAV_BUF 1024

using namespace std;

namespace gnut
{
    /**
     *@brief Class for navigation system setting derive from t_gdata
     */
    class LibGnut_LIBRARY_EXPORT t_gallnav : public t_gdata
    {

    public:
        /** @brief constructor. */
        explicit t_gallnav();
        explicit t_gallnav(t_spdlog spdlog);

        /** @brief destructor. */
        virtual ~t_gallnav();

        typedef multimap<t_gtime, shared_ptr<t_geph>> t_map_ref; ///<  all data for a single satellite
        typedef map<string, t_map_ref> t_map_sat;                ///<  all data for all satellites

        /**
        * @brief get satellite health.
        *
        * @param[in]  sat    satellite
        * @param[in]  t        time
        * @return    status
        */
        virtual bool health(const string &sat, const t_gtime &t);

        /**
         * @brief 
         * 
         * @param sat 
         * @param t 
         * @param xyz 
         * @param var 
         * @param vel 
         * @param chk_mask 
         * @return int 
         */
        virtual int nav(const string &sat,
                        const t_gtime &t,
                        double xyz[3] = NULL,
                        double var[3] = NULL,
                        double vel[3] = NULL,
                        const bool &chk_mask = true); ///< [m]

        /**
         * @brief 
         * 
         * @param sat 
         * @param t 
         * @param xyz 
         * @param var 
         * @param vel 
         * @param chk_mask 
         * @return int 
         */
        virtual int pos(const string &sat,
                        const t_gtime &t,
                        double xyz[3] = NULL,
                        double var[3] = NULL,
                        double vel[3] = NULL,
                        const bool &chk_mask = true); ///< [m]

        /**
         * @brief 
         * 
         * @param sat 
         * @param iod 
         * @param t 
         * @param xyz 
         * @param var 
         * @param vel 
         * @param chk_mask 
         * @return int 
         */
        virtual int pos(const string &sat,
                        const int &iod,
                        const t_gtime &t,
                        double xyz[3] = NULL,
                        double var[3] = NULL,
                        double vel[3] = NULL,
                        const bool &chk_mask = true); ///< [m] add by glfeng

        /**
        * @brief return clock corrections.
        *
        * @param[in]  sat        satellite
        * @param[in]  t            time
        * @param[in]  clk        clock offset
        * @param[in]  var        
        * @param[in]  dclk        difference of clock offset
        * @param[in]  chk_mask    
        * @return    irc
        */
        virtual int clk(const string &sat,
                        const t_gtime &t,
                        double *clk = NULL,
                        double *var = NULL,
                        double *dclk = NULL,
                        const bool &chk_mask = true);

        /**
        * @brief clean function.
        *
        * @param[in]  beg        begin time
        * @param[in]  end        end time
        * @return    void
        */
        virtual void clean_outer(const t_gtime &beg = FIRST_TIME,
                                 const t_gtime &end = LAST_TIME);

        /**
        * @brief get all satellites.
        *
        * @return    all satellites
        */
        virtual set<string> satellites() const;

        /**
        * @brief add single navigation message.
        *
        * @param[in]  nav    navigation system
        * @return    0
        */
        virtual int add(shared_ptr<t_gnav> nav);

        /**
        * @brief get number of epochs.
        *
        * @param[in]  prn    satellite prn
        * @return    number of epochs
        */
        virtual unsigned int nepochs(const string &prn);

        /**
        * @brief position/clock reference point.
        *
        * @return    com
        */
        const bool &com() const { return _com; }

        /**
        * @brief get number of satellites.
        *
        * @param[in]  gs        navigation system
        * @return    number of satellites
        */
        virtual int nsat(const GSYS &gs) const;

        /**
        * @brief find appropriate t_geph element (interface only).
        *
        * @param[in]  sat        satellite
        * @param[in]  t            time
        * @param[in]  chk_mask    
        * @return    tmp
        */
        shared_ptr<t_geph> find(const string &sat, const t_gtime &t, const bool &chk_mask = true);

        /**
        * @brief return frequency number of GLONASS.
        *
        * @return    _glo_freq_num
        */
        const map<string, int> &glo_freq_num() const { return _gloFreqNum; }

        /**
        * @brief get ionosphere correction.
        *
        * @param[in]  c        ionosphere correction
        * @return    io
        */
        t_iono_corr get_iono_corr(const IONO_CORR &c) const;

        /**
        * @brief add ionosphere correction.
        *
        * @param[in]  c        ionosphere correction
        * @param[in]  io        ionosphere correction
        * @return    void
        */
        void add_iono_corr(const IONO_CORR &c, const t_iono_corr &io);

    protected:
        /**
        * @brief find appropriate t_geph element.
        *
        * @param[in]  sat        satellite
        * @param[in]  t            time
        * @param[in]  chk_mask
        * @return    null
        */
        virtual shared_ptr<t_geph> _find(const string &sat, const t_gtime &t, const bool &chk_mask = true);

        /**
        * @brief find appropriate t_geph element.
        *
        * @param[in]  sat        satellite
        * @param[in]  iod        issue of data 
        * @param[in]  t            time
        * @param[in]  chk_mask
        * @return    
        */
        virtual shared_ptr<t_geph> _find(const string &sat, const int &iod, const t_gtime &t, const bool &chk_mask = true);

    protected:
        bool _com = false;       ///< position/clock reference point (com = true; apc = false);
        bool _multimap = false;  ///< use multimap for redundant records
        bool _overwrite = false; ///< overwrite mode (for derived classes with MAP)
        bool _chkHealth = false; ///< check satellite health (navigation)
        bool _chkNavig = false;  ///< check navigation messages (internal)
        bool _chkTot = false;
        int _nepoch = 0;                          ///< maximum number of epochs (0 = keep all)
        t_map_sat _mapsat;                        ///< map over all satellites (positions,..?)
        shared_ptr<t_geph> _null;                 ///< null pointer
        t_map_iono _brdc_iono_cor;                ///< ionosphere correction in BRDC
        map<string, int> _gloFreqNum;             ///< frequency number of GLONASS
    };

} // namespace

#endif
