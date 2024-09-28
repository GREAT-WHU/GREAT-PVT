
/**
*
* @verbatim
    History
    2012-11-05  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* References:
    [1] D.D.McCarthy, IERS Technical Note 21, IERS Conventions 1996, July 1996
*.
* @file       gtide96.h
* @brief      Purpose: implements tides
*.
* @author     JD
* @version    1.0.0
* @date       2012-11-05
*
*/

#ifndef GTIDE96_H
#define GTIDE96_H

#include <vector>

#ifdef BMUTEX
#include <boost/thread/mutex.hpp>
#endif

#include "gutils/gconst.h"
#include "gutils/gtime.h"
#include "gutils/gtriple.h"
#include "gmodels/gephplan.h"
#include "gmodels/gtide.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_gtide96 derive from t_gtide. */
    class LibGnut_LIBRARY_EXPORT t_gtide96 : public t_gtide
    {

    public:
        /** @brief constructor 1. */
        t_gtide96(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gtide96();

        /** @brief solid earth tides. */
        virtual t_gtriple tide_searth(const t_gtime &epo, t_gtriple &xyz);

        /** @brief pole tides. */
        virtual t_gtriple tide_pole();

        /** @brief ocean tide loading. */
        virtual t_gtriple load_ocean(const t_gtime &epoch, const string &site, const t_gtriple &xRec);

        /** @brief atmospheric tide loading. */
        virtual t_gtriple load_atmosph();

    protected:
    };

} // namespace

#endif
