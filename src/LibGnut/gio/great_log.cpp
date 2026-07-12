#include "gio/great_log.h"

#include "spdlog/async.h"                    //support for async logging.
#include "spdlog/spdlog.h"                   // IWYU pragma: keep
#include "spdlog/sinks/rotating_file_sink.h" // IWYU pragma: keep
#include "spdlog/sinks/basic_file_sink.h"    // IWYU pragma: keep
#include "spdlog/sinks/daily_file_sink.h"    // IWYU pragma: keep
#include "spdlog/sinks/stdout_color_sinks.h" // IWYU pragma: keep

#define DEF_FORMAT_LOG
#ifndef DEF_FORMAT_LOG
#define DEF_FORMAT_LOG "[%Y-%m-%d %H:%M:%S] [%L] [%-4#] <%-40!> : %v"
#else
#define DEF_FORMAT_LOG "[%Y-%m-%d %H:%M:%S] [%L] : %v"
#endif

#include <chrono>
#include <iostream>
#include <stdexcept>

namespace gnut
{
    t_great_log& t_great_log::get_instance()
    {
        static t_great_log myLog;
        return myLog;
    }

    void t_great_log::init_logger(const std::string& path, const std::string& level)
    {
        if (level == "trace")
        {
            init_logger(path, spdlog::level::trace);
        }
        else if (level == "debug")
        {
            init_logger(path, spdlog::level::debug);
        }
        else if (level == "warn")
        {
            init_logger(path, spdlog::level::warn);
        }
        else if (level == "error")
        {
            init_logger(path, spdlog::level::err);
        }
        else
        {
            init_logger(path, spdlog::level::info);
        }
    }

    void t_great_log::init_logger(const std::string& path, const spdlog::level::level_enum& level)
    {
        try
        {
            spdlog::drop("app");
            _gspdlog.reset();
            sinks.clear();

            // creat log
            auto sink_stdo = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sinks.push_back(sink_stdo);

            if (!path.empty())
            {
                auto sink_file = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    path,
                    500 * 1024 * 1024,
                    1000,
                    true); // multi part log files, with every part 500M, max 1000 files
                sinks.push_back(sink_file);
            }

            //_gspdlog = std::make_unique<spdlog::logger>("app", sinks.begin(), sinks.end());
            _gspdlog = std::make_shared<spdlog::logger>("app", sinks.begin(), sinks.end());

            // custom format
            _gspdlog->set_pattern(DEF_FORMAT_LOG); // with timestamp, thread_id, filename and line number

            _gspdlog->set_level(level);
            _gspdlog->flush_on(level);

            spdlog::flush_every(std::chrono::seconds(3));
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            std::cout << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

    void t_great_log::drop_log()
    {
    }

    t_great_log::t_great_log()
    {
    }

    t_great_log::~t_great_log()
    {
    }

    std::shared_ptr<spdlog::logger> t_great_log::get_logger()
    {
        if (!_gspdlog)
        {
            init_logger("", spdlog::level::info);
        }
        return _gspdlog;
    }

    void throw_logical_error(const std::string& message)
    {
        GREAT_CRITICAL(message);
        throw std::logic_error("");
    }

} // namespace gnut
