/**
*
* @verbatim
    History
*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     gallpcv.h
* @brief    Purpose: pcv data
*
* @author   JD
* @version  1.0.0
* @date     2011-02-14
*
*/

#ifndef GALLPCV_H
#define GALLPCV_H

#include <iostream>
#include <string.h>
#include "gdata/gdata.h"
#include "gutils/gsys.h"
#include "gutils/gconst.h"
#include "gutils/gtime.h"
#include "gmodels/gpcv.h"

using namespace std;

namespace gnut
{

    /**
    *@brief Class for t_gallpcv derive from t_gdata
    */
    class LibGnut_LIBRARY_EXPORT t_gallpcv : public t_gdata
    {

    public:
        /** @brief default constructor. */
        t_gallpcv();

        /**
         * @brief Construct a new t gallpcv object
         * 
         * @param spdlog 
         */
        t_gallpcv(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gallpcv();

        typedef map<t_gtime, shared_ptr<t_gpcv>> t_map_epo; ///< map of 1-ant/1-sn/N-epochs
        typedef map<string, t_map_epo> t_map_num;           ///< map of 1-ant/N-sn/N-epochs
        typedef map<string, t_map_num> t_map_pcv;           ///< map of N-ant/N-sn/N-epochs

        /**
        *@brief add single antenn pattern (PCV)
        */
        int addpcv(shared_ptr<t_gpcv> pcv);

        /**
        *@brief get single antenn pattern (PCV)
        */
        shared_ptr<t_gpcv> gpcv(const string &ant, const string &num,
                                const t_gtime &t);

    protected:
        /**
        *@brief find appropriate t_gpcv element
        */
        virtual shared_ptr<t_gpcv>
        _find(const string &ant, const string &ser, const t_gtime &t);

    private:
        t_map_pcv _mappcv; ///< complete PCV-map
        bool _overwrite;   ///< rewrite/add only mode

        shared_ptr<t_gpcv> _pcvnull; ///< pcvnull
    };

} // namespace

#endif
