
/**
*
* @verbatim
    History
    2013-06-05  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       ginterp.h
* @brief      Purpose: implementation various interpolation tecnique
*.
* @author     PV
* @version    1.0.0
* @date       2013-06-05
*
*/

#ifndef GINTERP_H
#define GINTERP_H

#include <vector>
#include <map>

#include "newmat/newmat.h"
#include "gutils/gtime.h"
#include "gutils/gpair.h"
#include "gdata/gdata.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_ginterp based on t_gdata. */
    class LibGnut_LIBRARY_EXPORT t_ginterp : public t_gdata
    {
    public:
        /** @brief default constructor. */
        t_ginterp();

        t_ginterp(t_spdlog spdlog);
        /** @brief default destructor. */
        virtual ~t_ginterp();

        /** @brief 1D type. */
        enum INTERP_1D
        {
            LINEAR,
            SPLINE
        };

        /** @brief 2D type. */
        enum INTERP_2D
        {
            BILINEAR,
            IDW,
            TPS
        };

        /** @brief 3D type. */
        enum INTERP_3D
        {
            VER2HOR,
            HOR2VER
        };

        /** @brief HT type. */
        enum INTERP_HT
        {
            INTERPOLATE,
            SCALE
        };

        /** @brief convert str to interp 1d/2d/3d/ht. */
        INTERP_1D str_to_interp_1d(const string &str);
        INTERP_2D str_to_interp_2d(const string &str);
        INTERP_3D str_to_interp_3d(const string &str);
        INTERP_HT str_to_interp_ht(const string &str);

        /** @brief convert interp to str 1d/2d/3d/ht. */
        string interp_1d_to_str(const INTERP_1D &typ);
        string interp_2d_to_str(const INTERP_2D &typ);
        string interp_3d_to_str(const INTERP_3D &typ);
        string interp_ht_to_str(const INTERP_HT &typ);

        /** @brief Linear Interpolation (double). */
        int linear(map<double, double> &data, double val, double &fval);

        /** @brief Spline Interpolation (double). */
        int spline(map<double, double> &data, double val, double &fval);

        /** @brief Linear Interpolation (time). */
        int linear(const map<t_gtime, double> &data, const t_gtime &epo, double &fval);

        /** 
        *@brief bilinear interpolation
        * 
        * 0 .. 11 (bottom-left)         21 *---------* 22
        * 1 .. 12 (bottom-right)           |         |
        * 2 .. 21 (top-left)               |         |
        * 3 .. 22 (top-right)           11 *---------* 12
        */
        int bilinear(const map<t_gpair, double> &data, const t_gpair &req_pos, double &fval);

    protected:
        INTERP_1D _interp_1d; ///< 1d Interpolation
        INTERP_2D _interp_2d; ///< 2d Interpolation
        INTERP_3D _interp_3d; ///< 3d Interpolation
        INTERP_HT _interp_ht; ///< ht Interpolation
    };

}

#endif
