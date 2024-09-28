/**
*
* @verbatim
History
-1.0  2021-08-01  jdhuang       create the file

@endverbatim
* Copyright (c) 2020, Wuhan University. All rights reserved.
*
* @file         grtlog.cpp
* @brief        the class for great app log
*
* @author       GREAT Wuhan University
* @version      1.0.0
* @date         2021-08-01
*
*/
#include <algorithm>

#include "gexport/ExportLibGnut.h"
#include "gutils/gfileconv.h"
#include "gio/grtlog.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define DEF_FORMAT_LOG "[%Y-%m-%d %H:%M:%S] [%L] [%-4#] <%-40!> : %v"
using namespace spdlog;

namespace gnut
{
    t_grtlog::t_grtlog()
    {
    }
    t_grtlog::t_grtlog(const string &log_type, const level::level_enum &log_level, const string &log_name)
    {
        _type = log_type;
        transform(_type.begin(), _type.end(), _type.begin(), ::toupper);
        _name = log_name;
        _level = log_level;
        _pattern = string(DEF_FORMAT_LOG);

        // Creat and set the log file : clk.log
        spdlog::set_level(_level);
        spdlog::set_pattern(_pattern);
        spdlog::flush_on(_level);

        string str_log_name = _name;

        if (_type.find("CONSOLE") != string::npos)
        {
            _spdlog = spdlog::stdout_color_mt(_name);
        }
        else if (_type.find("ROTATING") != string::npos)
        {
            _spdlog = spdlog::rotating_logger_mt(_name, str_log_name, 1024 * 1024, 10);
        }
        else if (_type.find("BASIC") != string::npos)
        {
            if (ACCESS(str_log_name.c_str(), 0) != -1)
            {
                int re = (remove(str_log_name.c_str()));
                spdlog::info(re);
            }
            _spdlog = spdlog::basic_logger_mt(_name, str_log_name);
        }
        else if (_type.find("DAILY") != string::npos)
        {
            if (ACCESS(str_log_name.c_str(), 0) != -1)
            {
                int re = (remove(str_log_name.c_str()));
                spdlog::info(re);
            }
            _spdlog = spdlog::daily_logger_mt(_name, str_log_name, 0, 0);
        }
        else
        {
            _spdlog = spdlog::stdout_color_mt(_name);
        }

        _spdlog->set_level(_level);
        _spdlog->flush_on(_level);
        _spdlog->set_pattern(_pattern);
    }

    t_grtlog::~t_grtlog()
    {
        spdlog::drop(_name);
    }

    std::shared_ptr<logger> t_grtlog::spdlog()
    {
        return _spdlog;
    }

    void t_grtlog::set_log(const string &log_type, const level::level_enum &log_level, const string &log_name)
    {
        _type = log_type;
        transform(_type.begin(), _type.end(), _type.begin(), ::toupper);
        _name = log_name;
        _level = log_level;
        _pattern = string(DEF_FORMAT_LOG);

        // Creat and set the log file : clk.log
        spdlog::set_level(_level);
        spdlog::set_pattern(_pattern);
        spdlog::flush_on(_level);

        if (_type.find("CONSOLE") != string::npos)
        {
            _spdlog = spdlog::stdout_color_mt(_name);
        }
        else if (_type.find("ROTATING") != string::npos)
        {
            _spdlog = spdlog::rotating_logger_mt(_name, _name + ".spd_log", 1024 * 1024, 10);
        }
        else if (_type.find("BASIC") != string::npos)
        {
            _spdlog = spdlog::basic_logger_mt(_name, _name + ".spd_log");
        }
        else if (_type.find("DAILY") != string::npos)
        {
            _spdlog = spdlog::daily_logger_mt(_name, _name + ".spd_log", 0, 0);
        }
        else
        {
            _spdlog = spdlog::stdout_color_mt(_name);
        }

        _spdlog->set_level(_level);
        _spdlog->flush_on(_level);
        _spdlog->set_pattern(_pattern);
    }

    void throw_logical_error(t_spdlog spdlog, const string &message)
    {
        if (nullptr != spdlog)
        {
            SPDLOG_LOGGER_CRITICAL(spdlog, message);
            throw logic_error("");
        }
        else
        {
            spdlog::critical(message);
            throw logic_error("");
        }
    }

}
