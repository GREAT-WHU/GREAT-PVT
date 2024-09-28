/**
 * @file         gifcb.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Storage the ifcb files' data
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GIFCB_H
#define GIFCB_H

#include "gexport/ExportLibGREAT.h"
#include "gdata/gdata.h"
#include "gutils/gtime.h"

using namespace gnut;

namespace great
{
    /**
    *@brief       Class for storaging one satellite ifcb data
    */
    class LibGREAT_LIBRARY_EXPORT t_ifcbrec
    {
    public:
        /** @brief default constructor. */
        t_ifcbrec();

        /** @brief default destructor. */
        ~t_ifcbrec(){};

        string obj;   ///< ifcb objection may be site or satellite
        int npoint;   ///< site number
        double value; ///< ifcb value
        double sigma; ///< std
        bool isRef;   ///< true set as a reference
    };

    /** map container using satellite name as a index for storaging t_ifcbrec ptr , one epoch/all satellite
    *   for wide-lane only one sign epoch "WL_IDENTIFY"   */
    typedef map<string, shared_ptr<t_ifcbrec>> LibGREAT_LIBRARY_EXPORT one_epoch_ifcb;

    /**
    *@brief     Class for storaging all epoch/all satellite ifcb data
    */
    class LibGREAT_LIBRARY_EXPORT t_gifcb : public t_gdata
    {
    public:
        /** @brief default constructor. */
        t_gifcb();

        /** @brief default constructor. */
        t_gifcb(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gifcb();

        /**
        * @brief add ifcb data of one epoch/one satellite.
        * @param[in]  epoch          epoch.
        * @param[in]  prn          satellite name.
        * @param[in]  one_sat_ifcb ifcb data of one epoch/one satellite
        * @return      void
        */
        void add_sat_ifcb(t_gtime epoch, string prn, t_ifcbrec one_sat_ifcb);

        /**
        * @brief get ifcb data of one epoch/all satellite.
        * @param[in] t epoch time
        * @param[in] str satellite name
        * @return the ifcb data in epoch
        */
        one_epoch_ifcb &get_epo_ifcb(const t_gtime &t);

    protected:
        map<t_gtime, one_epoch_ifcb> _ifcb; ///< ifcb map container of all epoch/all satellite
        t_gtime _valid_beg;                 ///< valid begin epoch
        one_epoch_ifcb _null_epoch_ifcb;    ///< ione epoch ifcb value

    private:
    };

}
#endif // !GALLPLANETEPH_H