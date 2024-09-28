/**
*
* @verbatim
History
*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     gallprod.h
* @brief    Purpose: container for all products
*
* @author   PV
* @version  1.0.0
* @date     2014-05-14
*
*/

#ifndef GALLPROD_H
#define GALLPROD_H

#include <map>
#include <set>
#include <string>
#include <memory>
#include <string>

#include "gprod/gprod.h"
#include "gutils/gtime.h"

using namespace std;

namespace gnut
{

    /**
    *@brief Class for t_gallprod derive from t_gdata
    */
    class LibGnut_LIBRARY_EXPORT t_gallprod : public t_gdata
    {

    public:
        /** @brief default constructor. */
        t_gallprod();
        /**
         * @brief Construct a new t gallprod object
         * 
         * @param spdlog 
         */
        t_gallprod(t_spdlog spdlog);
        /** @brief default destructor. */
        virtual ~t_gallprod();

        typedef map<t_gtime, shared_ptr<t_gprod>> t_map_epo; ///< time + t_gprod
        typedef map<ID_TYPE, t_map_epo> t_map_id;            ///< ID_TYPE + t_map_id
        typedef map<string, t_map_id> t_map_prd;             ///< string + t_map_prd

        /** @brief add product. */
        int add(shared_ptr<t_gprod> prod, string site = "");

        /** @brief get product. */
        shared_ptr<t_gprod> get(const string &site, ID_TYPE type, const t_gtime &t);

        /** @brief remove appropriate element t_gprod*. */
        void rem(const string &site, ID_TYPE type, const t_gtime &t);

        /** @brief get the list of sites*. */
        set<string> prod_sites();

        /** @brief get the list of product types. */
        set<t_gtime> prod_epochs(const string &site, ID_TYPE type);

        /** @brief clean. */
        void clear();

        /** @brief clean data out of the interval. */
        void clean_outer(const t_gtime &beg = FIRST_TIME, const t_gtime &end = LAST_TIME);

    protected:
        /** @brief find appropriate element t_gprod*. */
        shared_ptr<t_gprod> _find(const string &site, ID_TYPE type, const t_gtime &t); // find element

        t_map_prd _map_prod; ///< t_map_prd
    };

} // namespace

#endif
