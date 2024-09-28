
/**
*
* @verbatim
    History
    2017-11-10  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gprodclk.h
* @brief      clk class
*.
* @author     JD
* @version    1.0.0
* @date       2017-11-10
*
*/

#ifndef GPRODCLK_H
#define GPRODCLK_H

#include <iostream>

#include "gprod/gprod.h"
#include "gutils/gtriple.h"

using namespace std;

namespace gnut
{
    /** @brief class for t_gprodclk derive from t_gprod. */
    class LibGnut_LIBRARY_EXPORT t_gprodclk : public t_gprod
    {

    public:
        /** @brief constructor 1. */
        t_gprodclk(const t_gtime &t, shared_ptr<t_gobj> pt = nullobj);

        t_gprodclk(t_spdlog spdlog, const t_gtime &t, shared_ptr<t_gobj> pt = nullobj);
        /** @brief default destructor. */
        virtual ~t_gprodclk();

        /** @brief add clk. */
        void clk(const double &val, const double &rms = 0.0);

        /** @brief get clk. */
        double clk();

    protected:
        double _clk;     ///< clk
        double _clk_rms; ///< clk rms

    private:
    };

} // namespace

#endif
