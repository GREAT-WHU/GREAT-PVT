
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

using namespace std;

namespace gnut
{

    t_gtide::t_gtide() : _gotl(nullptr)
    {
    }

    t_gtide::t_gtide(t_spdlog spdlog) : _gotl(nullptr)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }
    }

    t_gtide::~t_gtide() {}

    t_gtriple t_gtide::tide_searth(const t_gtime &epoch, t_gtriple &crd)
    {
        _mutex.lock();
        t_gtriple dxyz(0.0, 0.0, 0.0);
        _mutex.unlock();
        return dxyz;
    }

    t_gtriple t_gtide::tide_pole()
    {
        _mutex.lock();
        t_gtriple dxyz(0.0, 0.0, 0.0);
        _mutex.unlock();
        return dxyz;
    }

    t_gtriple t_gtide::load_ocean(const t_gtime &epoch, const string &site, const t_gtriple &xRec)
    {
        _mutex.lock();
        t_gtriple dxyz(0.0, 0.0, 0.0);
        _mutex.unlock();
        return dxyz;
    }

    t_gtriple t_gtide::load_atmosph()
    {
        _mutex.lock();
        t_gtriple dxyz(0.0, 0.0, 0.0);
        _mutex.unlock();
        return dxyz;
    }

    void t_gtide::spdlog(t_spdlog spdlog)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }
    }

    void t_gtide::setOTL(t_gallotl *gallotl)
    {
        _mutex.lock();
        _gotl = gallotl;
        _mutex.unlock();
        return;
    }

} // namespace
