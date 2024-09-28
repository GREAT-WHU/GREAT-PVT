
/**
*
* @verbatim
    History
    2013-03-27  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       geop.h
* @brief      Purpose: earth orientation parameters, base class
*             Neither precession nor nutation apllied -> return identity matrix
*.
* @author     PV
* @version    1.0.0
* @date       2013-03-27
*
*/

#ifndef GEOP_H
#define GEOP_H

#include "gexport/ExportLibGnut.h"
#include "gdata/gdata.h"
#include "gutils/gconst.h"
#include "gutils/gtime.h"
#include "gutils/gtriple.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_geop. */
    class LibGnut_LIBRARY_EXPORT t_geop
    {

    public:
        /** @brief default constructor. */
        t_geop();

        /** @brief default destructor. */
        virtual ~t_geop();

        /** @brief Nutation Matrix. */
        Matrix nutMatrix(double mjd);

        /** @brief Precession Matrix. */
        Matrix precMatrix(double mjd);

        /** @brief Normalize angle into interval 0 - 2pi. */
        double _normangle(double x);
    };

} // namespace

#endif
