
/**
*
* @verbatim
    History
    2012-09-20  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gpppmodel.h
* @brief      Purpose: various PPP models
*.
* @author     PV
* @version    1.0.0
* @date       2012-09-20
*
*/

#ifndef GPPPMODEL_H
#define GPPPMODEL_H

#include <string>
#include <map>
#include <cmath>

#include "gmodels/gsppmodel.h"
#include "gutils/gtypeconv.h"
#include "gmodels/gephplan.h"
#include "gall/gallobj.h"

using namespace std;

namespace gnut
{
    /** @brief class for t_gpppmodel derive from t_gsppmodel. */
    class LibGnut_LIBRARY_EXPORT t_gpppmodel : public t_gsppmodel
    {
    public:
        /** @brief constructor 1. */
        t_gpppmodel(string site, t_spdlog spdlog, t_gsetbase *settings);

        /** @brief default constructor. */
        t_gpppmodel();

        /** @brief default destructor. */
        virtual ~t_gpppmodel();

        /** @brief set object. */
        virtual void setOBJ(t_gallobj *obj);

    protected:

        t_gallobj *_gallobj;             ///< all obj
        GRDMPFUNC _grad_mf;              ///< grad mf
        ATTITUDES _attitudes;            ///< attitudes

    };

} // namespace

#endif //  GPPPMODEL_H
