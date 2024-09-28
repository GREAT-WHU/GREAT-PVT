/**
*
* @verbatim
    History
     -1.0 ZhengHJ  2019-04-02  created
  @endverbatim
* Copyright (c) 2018, Wuhan University. All rights reserved.
*
* @file         gallprocdata.cpp
* @brief        header files of storaging all data 
*
* @author       ZhengHJ, Wuhan University
* @version      1.0.0
* @date         2019-04-16
*
*/

#include "gall/gallproc.h"

namespace great
{
    t_gallproc::t_gallproc() : t_gdata()
    {
    }

    t_gallproc::t_gallproc(t_spdlog spdlog) : t_gdata(spdlog)
    {
    }

    t_gallproc::t_gallproc(const t_gallproc &Other) : t_gdata(Other.spdlog())
    {
        this->_mapData = Other._mapData;
    }

    t_gallproc::~t_gallproc()
    {
    }

    void t_gallproc::operator=(const t_gallproc &Other)
    {
        this->_mapData = Other._mapData;
    }

    void t_gallproc::Add_Data(const string &type, t_gdata *data)
    {
        if (data)
        {
            _mapData[data->id_type()] = data;
        }
        else
        {
            SPDLOG_LOGGER_CRITICAL(_spdlog, "your {} is nullptr", type);
            throw logic_error("t_gdata pointer is nullptr!");
        }
    }

    t_gdata *t_gallproc::operator[](t_gdata::ID_TYPE type)
    {
        if (_mapData.find(type) == _mapData.end())
        {
            return nullptr;
        }
        return _mapData[type];
    }

    t_gdata *t_gallproc::operator[](t_gdata::ID_GROUP group)
    {
        for (auto data_iter = _mapData.begin(); data_iter != _mapData.end(); data_iter++)
        {
            if (data_iter->second->id_group() == group)
            {
                return data_iter->second;
            }
        }
        return nullptr;
    }
}