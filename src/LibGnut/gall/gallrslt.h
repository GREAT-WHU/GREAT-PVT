/**
*
* @verbatim
History
*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
            (c) 2011-2017 Geodetic Observatory Pecny, http://www.pecny.cz (gnss@pecny.cz)
      Research Institute of Geodesy, Topography and Cartography
      Ondrejov 244, 251 65, Czech Republic
*
* @file     gallrslt.h
* @brief    container for all observation
*
* @author   PV
* @version  1.0.0
* @date     2017-11-05
*
*/

#ifndef GALLRSLT_H
#define GALLRSLT_H

#include <vector>
#include <set>
#include <iostream>

#include "gutils/gtime.h"

namespace gnut
{
    /**
    *@brief Class for t_gallrslt
    */
    class LibGnut_LIBRARY_EXPORT t_gallrslt
    {
    public:
        /** @brief default constructor. */
        t_gallrslt();

        /** @brief struct result. */
        struct result
        {
            string type; ///< type
            string prn;  ///< prn
            t_gtime beg; ///< begin time
            t_gtime end; ///< end time
            int index;   ///< index
            double adj;  ///< adj
            double rms;  ///< rms
            double val;  ///< val

            /** @brief override operator <. */
            bool operator<(const t_gallrslt::result &) const;
        };

        vector<t_gallrslt::result> v_rslt; ///< result vector
        set<t_gallrslt::result> set_crd;   ///< crd set
        set<t_gallrslt::result> set_trp;   ///< trop set
        set<t_gallrslt::result> set_clk;   ///< clk set
        set<t_gallrslt::result> set_amb;   ///< amb set

        /** @brief append result structure. */
        void append(const t_gallrslt::result &rslt);

        /**
         * @brief 
         * 
         * @param type 
         * @param beg 
         * @param end 
         * @param idx 
         * @param prn 
         * @param adj 
         * @param rms 
         * @param val 
         */
        void append(const string &type, const t_gtime &beg, const t_gtime &end, int idx,
                    const string &prn, double adj, double rms, double val);
    };

} // namespace

#endif // GALLRSLT_H
