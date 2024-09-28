/**
*
* @verbatim
    History
    2012-06-06  JD: created
    2013-06-18  JD: joined gpppfilter & glsq
  
  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com).
*           (c) 2011-2017 Geodetic Observatory Pecny, http://www.pecny.cz (gnss@pecny.cz)
      Research Institute of Geodesy, Topography and Cartography
      Ondrejov 244, 251 65, Czech Republic
* @file       gppp.h
* @brief      Purpose: implements PPP client abstract class
*.
* @author     JD
* @version    1.0.0
* @date       2012-06-06
*
*/

#ifndef GPPP_H
#define GPPP_H

#ifdef BMUTEX
#include <boost/thread/mutex.hpp>
#endif

#include "gproc/gspp.h"
#include "gdata/gsatdata.h"
#include "gall/gallotl.h"
#include "gall/gallrslt.h"
#include "gmodels/gtropo.h"
#include "gmodels/gpar.h"
#include "gmodels/gpar.h"
#include "gmodels/gtropo.h"
#include "gmodels/gtide.h"
#include "gmodels/gtide96.h"
#include "gmodels/gtide2010.h"
#include "gmodels/gephplan.h"

#ifdef _WIN32
#pragma warning(disable : 4250) // suppress Visual Studio WARNINGS about inheritance via dominance
#endif

namespace gnut
{
    /** @brief class for t_gppp derive from t_gspp. */
    class LibGnut_LIBRARY_EXPORT t_gppp : public virtual t_gspp
    {
    public:
        /** @brief constructor 1. */
        t_gppp(string mark, t_gsetbase *set);

        t_gppp(string mark, t_gsetbase *set, t_spdlog spdlog);
        /** @brief default destructor. */
        virtual ~t_gppp();

        /** @brief set OTL. 
        * @param[in] gotl ocean tide data
        */
        virtual void setOTL(t_gallotl *gotl);

        /** @brief set OBJ. 
        * @param[in] gobj object data
        */
        virtual void setOBJ(t_gallobj *gobj);

        /** @brief get isRunning. */
        virtual bool isRunning();

    protected:
        /** @brief Get settings from XML file and set local variables. */
        virtual int _get_settings();

        /** @brief Print results. */
        virtual void _prt_results(t_giof *giof, const t_gallrslt &rslt);

        t_gallotl *_gotl; ///< Ocean loading date.
        t_gtide *_tides;  ///< Tides.

        bool _running;    ///< Running status.
        double _sigAmbig; ///< Ambiguity sigma (constraints).

        t_gmutex _gmutex; ///< gmutex

    private:
    };

} // namespace

#endif //GPPP_H
