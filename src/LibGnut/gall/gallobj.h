/**
*
* @verbatim
    History

*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     gallobj.h
* @brief    container for all objects
*
* @author   JD
* @version  1.0.0
* @date     2012-12-04
*
*/

#ifndef GALLOBJ_H
#define GALLOBJ_H

#include <vector>
#include <memory>

#include "gdata/grec.h"
#include "gdata/gtrn.h"
#include "gutils/gsysconv.h"
#include "gutils/gtypeconv.h"
#include "gall/gallotl.h"

using namespace std;

namespace gnut
{

    /**
     *@brief Class for t_allobj derive from t_gdata
     */
    class LibGnut_LIBRARY_EXPORT t_gallobj : public t_gdata
    {
    public:
        /** @brief default constructor. */
        explicit t_gallobj();

        /** @brief constructor pcv_olt. */
        explicit t_gallobj(t_gallpcv *pcv, t_gallotl *otl);

        /**
         * @brief Construct a new t gallobj object
         * 
         * @param spdlog 
         */
        explicit t_gallobj(t_spdlog spdlog);

        /**
         * @brief Construct a new t gallobj object
         * 
         * @param spdlog 
         * @param pcv 
         * @param otl 
         */
        explicit t_gallobj(t_spdlog spdlog, t_gallpcv *pcv, t_gallotl *otl);

        /** @brief default destructor. */
        virtual ~t_gallobj();

        typedef map<string, shared_ptr<t_gobj>> t_map_obj;   ///< first : , second :allocated data of a single object

        /**
         * @brief set/get single obj element
         * 
         * @param obj 
         * @return int 
         */
        int add(shared_ptr<t_gobj> obj); 

        /**
         * @brief 
         * 
         * @param s 
         * @return shared_ptr<t_gobj> 
         */
        shared_ptr<t_gobj> obj(const string &s);

        /**
        * @brief synchronize PCVs.
        *
        * @return    void
        */
        void sync_pcvs();

        /**
        * @brief read satellite information.
        *
        * @param[in]  epo    epoch
        * @return    void
        */
        void read_satinfo(t_gtime &epo);

        /**
        * @brief get all object elements.
        *
        * @param[in]  id
        * @return    all object
        */
        virtual map<string, shared_ptr<t_gobj>> objects(const t_gdata::ID_TYPE &id = NONE);

    private:
        t_map_obj _mapobj; ///< map of all objects
        t_gallpcv *_gpcv;  ///< map of all PCV
        t_gallotl *_gotl;  ///< map of all otl

        /** @brief alocate all t_gtrn objects for all GNSS. */
        void _aloctrn();  
    };

} // namespace

#endif
