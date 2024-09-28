/**
 * @file         gupd.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Storage the upd files' data include wide-lane, narrow-lane and ex-wide-lane
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GUPD_H
#define GUPD_H


#include "gdata/gdata.h"
#include "gutils/gtime.h"
#include "gset/gsetamb.h"

using namespace gnut;

#ifndef EWL_IDENTIFY
#define EWL_IDENTIFY 666666   ///< the sign epoch for extra-wide-lane upd
#endif

#ifndef EWL24_IDENTIFY
#define EWL24_IDENTIFY 777777 ///< the sign epoch for extra-wide-lane(24) upd
#endif

#ifndef EWL25_IDENTIFY
#define EWL25_IDENTIFY 888888 ///< the sign epoch for extra-wide-lane(25) upd
#endif

#ifndef WL_IDENTIFY
#define WL_IDENTIFY 999999    ///< the sign epoch for wide-lane upd
#endif

namespace great
{

    /**
    *@brief       Class for storaging one satellite upd data
    */
    class LibGREAT_LIBRARY_EXPORT t_updrec
    {
    public:
        /** @brief default constructor. */
        t_updrec();

        /** @brief default destructor. */
        ~t_updrec(){};

        string obj;   ///< upd objection may be site or satellite
        int npoint;   ///< site number
        double ratio; ///< number of good observation divide number of all observation based on rec.
        double value; ///< upd value
        double sigma; ///< std
        bool isRef;   ///< true set as a reference
    };

    /** 
    * map container using satellite name as a index for storaging t_updrec ptr , one epoch/all satellite
    * for wide-lane only one sign epoch "WL_IDENTIFY"   
    */
    typedef map<string, shared_ptr<t_updrec>> LibGREAT_LIBRARY_EXPORT one_epoch_upd;

    /**
    *@brief     Class for storaging all epoch/all satellite upd data
    */
    class LibGREAT_LIBRARY_EXPORT t_gupd : public t_gdata
    {
    public:
        /** @brief default constructor. */
        t_gupd();

        t_gupd(t_spdlog spdlog);
        /** @brief default destructor. */
        virtual ~t_gupd();

        /**
        * @brief add upd data of one epoch/one satellite.
        * @param[in]  upd_type    upd type
        * @param[in]  epoch          epoch.
        * @param[in]  prn          satellite name.
        * @param[in]  one_sat_upd upd data of one epoch/one satellite
        * @return      void
        */
        void add_sat_upd(UPDTYPE upd_type, t_gtime epoch, string prn, t_updrec one_sat_upd);

        /**
        * @brief get upd data of one epoch/all satellite.
        * @param[in] upd_type    upd type
        * @param[in] t           epoch time
        * @param[in] str         satellite name
        * @return the upd data in epoch
        */
        one_epoch_upd &get_epo_upd(const UPDTYPE &upd_type, const t_gtime &t); 

        /**
        * @brief reinitialize upd data of one epoch/one satellite.
        * @param[in] upd_type    upd type
        * @param[in] t           epoch time
        * @param[in] str         satellite name
        * @return      void
        */
        void re_init_upd(const UPDTYPE &upd_type, const t_gtime &t, string str); 

        /**
        * @ set/get wl upd mode (Epoch by epoch/ Single day solution)
        */
        void wl_epo_mode(bool is) { _wl_epo_mode = is; };
        bool wl_epo_mode() { return _wl_epo_mode; };

    protected:
        map<UPDTYPE, map<t_gtime, one_epoch_upd>> _upd; ///< upd map container of all epoch/all satellite(different type)
        UPDTYPE _est_upd_type;                          ///< upd mode (for estimation)
        map<UPDTYPE, t_gtime> _valid_beg;               ///< valid begin epoch (for encoder)
        one_epoch_upd _null_epoch_upd;
        bool _wait_stream; ///< Real-time upd interrupt to maintain float solution results
        bool _wl_epo_mode; ///< WL epoch by epoch or not?
        t_gtime _tend;     ///< The end Time of real-time upd.
        double _upd_intv;  ///< The interval of real-time upd.

    private:
        t_gtime _ewl_flag;   ///< ewl flag
        t_gtime _ewl24_flag; ///< ewl24 flag
        t_gtime _ewl25_flag; ///< ewl25 flag
        t_gtime _wl_flag;    ///< wl flag
    };

}
#endif // !GALLPLANETEPH_H