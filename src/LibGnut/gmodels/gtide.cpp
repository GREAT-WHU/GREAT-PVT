
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include "gutils/gsysconv.h"
#include "gmodels/gtide.h"


namespace gnut
{

    t_gtide::t_gtide() :
        _gotl(nullptr)
    {
    }

    t_gtide::~t_gtide()
    {
    }

    t_gtriple t_gtide::tide_searth(const t_gtime& epoch, t_gtriple& crd)
    {
        t_gtriple dxyz(0.0, 0.0, 0.0);
        return dxyz;
    }

    t_gtriple t_gtide::tide_pole()
    {
        t_gtriple dxyz(0.0, 0.0, 0.0);
        return dxyz;
    }

    t_gtriple t_gtide::load_ocean(const t_gtime& epoch, const std::string& site, const t_gtriple& xRec)
    {
        t_gtriple dxyz(0.0, 0.0, 0.0);
        return dxyz;
    }

    t_gtriple t_gtide::load_atmosph()
    {
        t_gtriple dxyz(0.0, 0.0, 0.0);
        return dxyz;
    }

    void t_gtide::setOTL(t_gallotl* gallotl)
    {
        _gotl = gallotl;
        return;
    }

} // namespace gnut
