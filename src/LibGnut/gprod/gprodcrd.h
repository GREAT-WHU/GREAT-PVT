/**
*
* @verbatim
    History
    2011-03-25  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gprodcrd.h
* @brief      crd class
*.
* @author     JD
* @version    1.0.0
* @date       2011-03-25
*
*/

#ifndef GPRODCRD_H
#define GPRODCRD_H

#include <iostream>

#include "gprod/gprod.h"
#include "gutils/gtriple.h"

using namespace std;

namespace gnut
{
    /** @brief cov type. */
    enum COV_TYPE
    {
        COV_XY,
        COV_XZ,
        COV_YZ
    };

    /** @brief class for t_gprodcrd derive from t_gprod. */
    class LibGnut_LIBRARY_EXPORT t_gprodcrd : public t_gprod
    {

    public:
        /** @brief constructor 1. */
        t_gprodcrd(const t_gtime &t, shared_ptr<t_gobj> pt = nullobj);

        t_gprodcrd(t_spdlog spdlog, const t_gtime &t, shared_ptr<t_gobj> pt = nullobj);
        /** @brief default destructor. */
        virtual ~t_gprodcrd();

        /** @brief set/get xyz. */
        void xyz(const t_gtriple &xyz);
        t_gtriple xyz() const;

        /** @brief set/get xyz rms. */
        void xyz_rms(const t_gtriple &xyz_rms);
        t_gtriple xyz_rms() const;

        /** @brief get xyz var. */
        t_gtriple xyz_var() const;

        /** @brief set/get apr. */
        void apr(const t_gtriple &apr);
        t_gtriple apr() const;

        /** @brief set/get apr rms. */
        void apr_rms(const t_gtriple &apr_rms);
        t_gtriple apr_rms() const;

        /** @brief get apr var. */
        t_gtriple apr_var() const;

        /** @brief add cov. */
        void cov(COV_TYPE type, double &cov);

        /** @brief get xyz cov. */
        double cov(COV_TYPE type) const;

    protected:
        t_gtriple _xyz;     ///< xyz
        t_gtriple _xyz_rms; ///< xyz rms
        t_gtriple _apr;     ///< apr
        t_gtriple _apr_rms; ///< apr rms

        double _xy_cov; ///< xy cov
        double _xz_cov; ///< xz cov
        double _yz_cov; ///< yz cov

    private:
    };

} // namespace

#endif
