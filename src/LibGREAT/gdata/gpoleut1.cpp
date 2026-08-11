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
namespace great
{
    t_gpoleut1::t_gpoleut1() :
        t_gdata()
    {
        id_type(t_gdata::ID_TYPE::ALLPOLEUT1);
    }
    void t_gpoleut1::setBegEndTime(int beg, int end)
    {
        _beg_time = beg;
        _end_end = end;
    }

    void t_gpoleut1::setEopData(t_gtime mjdtime, std::map<std::string, double> data, std::string mode, double intv)
    {
        _poleut1_data[mjdtime] = data;
        _UT1_mode = mode;
        _intv = intv;
    }

    void t_gpoleut1::getEopData(t_gtime mjdtime, std::map<std::string, double>& data)
    {
        std::lock_guard<t_gmutex> lock(_gmutex);
        auto right = _poleut1_data.upper_bound(mjdtime);
        if (right == _poleut1_data.begin())
        {
            throw std::runtime_error("interpolation poleut1 wrong");
        }

        auto left = right;
        --left;
        if (left->first == mjdtime || right == _poleut1_data.end())
        {
            if (left->first != mjdtime)
            {
                throw std::runtime_error("interpolation poleut1 wrong");
            }
            data = left->second;
            return;
        }

        const double span = right->first - left->first;
        if (_intv > 0.0 && span > _intv * 86400.0 + 1.0)
        {
            throw std::runtime_error("poleut1 data gap exceeds declared interval");
        }

        const double alpha = (mjdtime - left->first) / span;
        data.clear();
        for (const auto& value : left->second)
        {
            data[value.first] = value.second + alpha * (right->second.at(value.first) - value.second);
        }
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

} // namespace great
