/**
 * @file         gbasemodel.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        base interface model  
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GBASEMODEL_H
#define GBASEMODEL_H

#include "gexport/ExportLibGREAT.h"
#include <vector>
#include "gutils/gtime.h"
#include "gall/gallpar.h"
#include "gdata/gsatdata.h"

using namespace std;
using namespace gnut;

namespace great
{
    class LibGREAT_LIBRARY_EXPORT t_gbaseEquation
    {
    public:
        /**
         * @brief Construct a new t gbaseEquation object
         */
        t_gbaseEquation();
        /**
         * @brief Destroy the t gbaseEquation object
         */
        virtual ~t_gbaseEquation();
        /**
         * @brief Define equation+
         * @param[in]  Other     another equation
         * @return t_gbaseEquation& the equation + another equation
         */
        t_gbaseEquation &operator+(t_gbaseEquation &Other);

        vector<vector<pair<int, double>>> B; ///< coeff of equations
        vector<double> P;                    ///< weight of equations
        vector<double> l;                    ///< res of equations
    };

    // interface class for all mode
    class LibGREAT_LIBRARY_EXPORT t_gbasemodel
    {
    public:
        /**
         * @brief Construct a new t gbasemodel object
         */
        t_gbasemodel();
        /**
         * @brief Destroy the t gbasemodel object
         */
        ~t_gbasemodel();
        /**
         * @brief base madel calculate BPL
         * @param[in]  epoch     time
         * @param[in]  params    parameter
         * @param[in]  obsdata   observation data
         * @param[in]  result    result equation
         * @return
         *      @retval true can calculate equation
         *      @retval false can not calculate equation
         */
        virtual bool cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gbaseEquation &result) = 0;
    };
}

#endif