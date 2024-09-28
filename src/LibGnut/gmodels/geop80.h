
/**
*
* @verbatim
    History
    2013-03-27  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
  Reference:
      Explanatory Supplement to the Astronomical Almanac,
      P. Kenneth Seidelmann (ed), University Science Books (1992),
      Section 3.222 (p111).

      Derived from SOFA software (www.iausofa.org)
*.
* @file       geop80.h
* @brief      Purpose: earth orientation parameters (IAU Conventions 1980)
*.
* @author     PV
* @version    1.0.0
* @date       2013-03-27
*
*/

#ifndef GEOP80_H
#define GEOP80_H

#include "gdata/gdata.h"
#include "gutils/gconst.h"
#include "gutils/gtime.h"
#include "gutils/gtriple.h"
#include "gmodels/geop.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_geop80 based on t_geop. */
    class LibGnut_LIBRARY_EXPORT t_geop80 : public t_geop
    {

    public:
        /** @brief default constructor. */
        t_geop80();

        /** @brief default destructor. */
        virtual ~t_geop80();

        /** @brief Nutation Matrix. */
        Matrix nutMatrix(double mjd);

        /** @brief Precession Matrix. */
        Matrix precMatrix(double mjd_1);

    protected:
        /** @brief Frac part of double. */
        double _frac(double x);
    };
}

#endif
