/**
*
* @verbatim
    History
*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     gallobs.h
* @brief    Purpose: container along sites for BLQ data
*
* @author   PV
* @version  1.0.0
* @date     2013-03-29
*
*/

#ifndef GALLOTL_H
#define GALLOTL_H
#include <string>
#include <map>
#include "newmat/newmat.h"
#include "gdata/gdata.h"
#include "gutils/gtime.h"
#include "gmodels/gotl.h"

using namespace std;

namespace gnut
{
    /**
     *@brief Class for t_gallotl derive from t_gdata
     */
    class LibGnut_LIBRARY_EXPORT t_gallotl : public t_gdata
    {
    public:
        /** @brief default constructor. */
        t_gallotl();

        /**
         * @brief Construct a new t gallotl object
         * 
         * @param spdlog 
         */
        t_gallotl(t_spdlog spdlog);

        /** @brief default destructor. */
        ~t_gallotl();

        /** @brief get data. */
        int data(Matrix &data, const string &site);

        /**
         * @brief 
         * 
         * @param data 
         * @param lon 
         * @param lat 
         * @return int 
         */
        int data(Matrix &data, double lon, double lat);

        /**
         * @brief 
         * 
         * @param otl 
         */
        void add(t_gotl &otl);

    private:
        map<string, t_gotl> _mapotl;
    };

} // namespace

#endif
