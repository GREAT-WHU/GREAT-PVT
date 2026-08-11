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

        /** @brief default destructor. */
        virtual ~t_gallpcv();

        typedef std::map<t_gtime, std::shared_ptr<t_gpcv>> t_map_epo; ///< map of 1-ant/1-sn/N-epochs
        typedef std::map<std::string, t_map_epo> t_map_num;           ///< map of 1-ant/N-sn/N-epochs
        typedef std::map<std::string, t_map_num> t_map_pcv;           ///< map of N-ant/N-sn/N-epochs
        typedef std::map<GSAT, t_map_epo> t_map_sat_pcv;         ///< satellite PCV by PRN

        /**
         *@brief add single antenn pattern (PCV)
         */
        int addpcv(std::shared_ptr<t_gpcv> pcv);

        /**
         *@brief get single antenn pattern (PCV)
         */
        std::shared_ptr<t_gpcv> gpcv(const std::string& ant, const std::string& num, const t_gtime& t);
        std::shared_ptr<t_gpcv> gpcv(GSAT sat, const t_gtime& t);

    protected:
        /**
         *@brief find appropriate t_gpcv element
         */
        virtual std::shared_ptr<t_gpcv> _find(const std::string& ant, const std::string& ser, const t_gtime& t);
        std::shared_ptr<t_gpcv> _find(GSAT sat, const t_gtime& t);

    private:
        t_map_pcv _mappcv; ///< complete PCV-map
        t_map_sat_pcv _mapsatpcv; ///< satellite PCV-map
        bool _overwrite;   ///< rewrite/add only mode

        std::shared_ptr<t_gpcv> _pcvnull; ///< pcvnull
    };

} // namespace gnut

#endif
