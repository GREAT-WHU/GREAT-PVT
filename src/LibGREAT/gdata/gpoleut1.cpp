/**
 * @file         gpoleut1.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        The class for storaging poleut1 data.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gdata/gpoleut1.h"
#include "math.h"
#include "gutils/gtypeconv.h"
using namespace std;
namespace great
{
    t_gpoleut1::t_gpoleut1()
        : t_gdata()
    {
        id_type(t_gdata::ALLPOLEUT1);
    }
    t_gpoleut1::t_gpoleut1(t_spdlog spdlog)
        : t_gdata(spdlog)
    {
        id_type(t_gdata::ALLPOLEUT1);
    }

    void t_gpoleut1::setBegEndTime(int beg, int end)
    {
        _beg_time = beg;
        _end_end = end;
    }

    void t_gpoleut1::setEopData(t_gtime mjdtime, map<string, double> data, string mode, double intv)
    {
        _poleut1_data[mjdtime] = data;
        _UT1_mode = mode;
        _intv = intv;
    }

    bool t_gpoleut1::isEmpty()
    {
        _gmutex.lock();
        if (_poleut1_data.size() == 0)
        {
            _gmutex.unlock();
            return true;
        }
        else
        {
            _gmutex.unlock();
            return false;
        }
    }

}