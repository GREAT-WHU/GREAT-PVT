/**
 * @file         gifcb.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Storage the ifcb files' data
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gdata/gifcb.h"

using namespace std;
namespace great
{
    t_ifcbrec::t_ifcbrec()
    {
        obj = " ";
        npoint = 0;
        value = 0.0;
        sigma = 1E4;
        isRef = false;
    }

    t_gifcb::t_gifcb() : t_gdata()
    {

        id_type(t_gdata::IFCB);
    }

    t_gifcb::t_gifcb(t_spdlog spdlog) : t_gdata(spdlog)
    {

        id_type(t_gdata::IFCB);
    }

    t_gifcb::~t_gifcb()
    {
        _ifcb.clear();
    }

    void t_gifcb::add_sat_ifcb(t_gtime epoch, string prn, t_ifcbrec one_sat_ifcb)
    {
        _ifcb[epoch][prn] = make_shared<t_ifcbrec>(one_sat_ifcb);
    }

    one_epoch_ifcb &t_gifcb::get_epo_ifcb(const t_gtime &t)
    {
        if (_ifcb.find(t) != _ifcb.end())
            return _ifcb[t];
        else
        {
            auto latter = _ifcb.lower_bound(t);
            auto former = latter;
            if (latter == _ifcb.end())
            {
                if (latter != _ifcb.begin())
                {
                    latter--;
                    if (t.diff(latter->first) < 30)
                        return _ifcb[latter->first];
                    else
                        return _null_epoch_ifcb;
                }
                else
                    return _null_epoch_ifcb;
            }
            else
            {
                if (former != _ifcb.begin())
                    former--;
                double diff1 = latter->first.diff(t);
                double diff2 = t.diff(former->first);
                if (diff1 >= 30 && diff2 >= 30)
                    return _null_epoch_ifcb;
                else
                {
                    return (diff1 < diff2 ? _ifcb[former->first] : _ifcb[latter->first]);
                }
            }
        }
    }

} //namespace