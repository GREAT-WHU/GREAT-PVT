
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

  This file is part of the G-Nut C++ library.
 
-*/

#include <iostream>

#include "gio/grtlog.h"
#include "gutils/gcommon.h"
#include "gutils/gtimesync.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{
    bool time_sync(const t_gtime &epo, double smp, double scl, t_spdlog spdlog)
    {
        if (smp > 0)
        {
            if (scl > 0 && smp <= 1)
            {
                int smpl = (int)round(scl * smp);
                int iepo = (int)(round(epo.sod() * scl + epo.dsec() * scl)); // synced to .0 day i.e >=1Hz suggests .0 day epochs at least!

                if (smpl == 0)
                    return false; // to be save if mixed high/low-rate sampling occurs
                int resi = iepo % smpl;
                if (resi != 0)
                    return false;
            }

            else if (int(round(epo.sod() + epo.dsec())) % int(smp) != 0)
            {
                return false;
            }
        }
        return true;
    }

    bool time_sync(double dsec, double smp, double scl, t_spdlog spdlog)
    {
        if (dsec < 0)
        {
            return false;
        }
        if (smp > 0)
        {
            if (scl > 0 && smp <= 1)
            {
                int smpl = (int)round(scl * smp);
                int iepo = (int)(round(dsec)); // synced to .0 day i.e >=1Hz suggests .0 day epochs at least!

                if (smpl == 0)
                    return false; // to be save if mixed high/low-rate sampling occurs
                int resi = iepo % smpl;
                if (resi != 0)
                    return false;
            }

            else if (int(round(dsec)) % int(smp) != 0)
            {
                return false;
            }
        }
        return true;
    }

} // namespace
