/**
*
* @verbatim
    History
    2014-04-18  PV: created
    2018-09-28  JD: revised

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gbancroft.h
* @brief      Purpose: statistical function (1D)
*.
* @author     PV
* @version    1.0.0
* @date       2014-04-18
*
*/

#ifndef GBANCROFT_H
#define GBANCROFT_H

#include "newmat/newmatap.h"

#include "gall/gallobs.h"
#include "gall/gallnav.h"
#include "gutils/gtriple.h"
#include "gutils/gmutex.h"
#include "gset/gsetbase.h"

namespace gnut
{

    LibGnut_LIBRARY_EXPORT int gbancroft(const Matrix &BBpass, ColumnVector &pos);

    LibGnut_LIBRARY_EXPORT inline double lorentz(const ColumnVector &aa, const ColumnVector &bb);

} // namespace

#endif
