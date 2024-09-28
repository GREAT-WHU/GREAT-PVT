
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
 
-*/

#include <cmath>

#include "gprod/gprodclk.h"

using namespace std;

namespace gnut
{

    t_gprodclk::t_gprodclk(const t_gtime &t, shared_ptr<t_gobj> pt)
        : t_gprod(t, pt),
          _clk(0.0),
          _clk_rms(0.0)
    {
        id_type(CLK);
        id_group(GRP_PRODUCT);
    }

    t_gprodclk::t_gprodclk(t_spdlog spdlog, const t_gtime &t, shared_ptr<t_gobj> pt)
        : t_gprod(spdlog, t, pt),
          _clk(0.0),
          _clk_rms(0.0)
    {

        id_type(CLK);
        id_group(GRP_PRODUCT);
    }

    t_gprodclk::~t_gprodclk()
    {
    }

    void t_gprodclk::clk(const double &val, const double &rms)
    {
        _gmutex.lock();

        _clk = val;
        _clk_rms = rms;

        _gmutex.unlock();
        return;
    }

    double t_gprodclk::clk()
    {
        return _clk;
    }

} // namespace