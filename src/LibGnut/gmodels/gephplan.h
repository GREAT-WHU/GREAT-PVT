
/**
*
* @verbatim
    History
    2013-03-26  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gephplan.h
* @brief      Purpose: planetary ephemerises
*.
* @author     PV
* @version    1.0.0
* @date       2013-03-26
*
*/

#ifndef GEPHPLAN_H
#define GEPHPLAN_H

#include "gexport/ExportLibGnut.h"
#include <vector>

#include "gutils/gconst.h"
#include "gutils/gtime.h"
#include "gutils/gtriple.h"
#include "gmodels/geop80.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_gephplan. */
    class LibGnut_LIBRARY_EXPORT t_gephplan
    {

    public:
        /** @brief default constructor. */
        t_gephplan();

        /** @brief default destructor. */
        virtual ~t_gephplan();

        /** @brief Sun position. */
        t_gtriple sunPos(double mjd, bool itrf = true);

        /** @brief Moon position. */
        t_gtriple moonPos(double mjd);

        /** @brief Greenwich Mean Sidereal Time. */
        double gmst(double mjd);

    protected:
        /** @brief Frac part of double. */
        double _frac(double x);

        t_geop80 _eop; ///< eop

    private:
        map<double, t_gtriple> _record_itrf_sunpos;
    };
}

#endif
