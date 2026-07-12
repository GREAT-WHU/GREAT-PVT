/**
 * @file tool_log.h
 * @brief
 * @author jdhuang (jiande@whu.edu.cn)
 * @version 1.0.1
 * @date 2024-07-31
 *
 * @copyright Copyright (c) 2024  jdhuang
 *
 */

#ifndef TOOL_LOG_H
#define TOOL_LOG_H

#include <string>
#include <memory>
#include <vector>

#include "spdlog/spdlog.h"
#include "gexport/ExportLibGnut.h"

namespace gnut
{
    /**
     * @brief typedef for spldlog
     * @author jdhuang (jiande@whu.edu.cn)
     * @date 2024-08-01
     */
    typedef std::shared_ptr<spdlog::logger> t_spdlog;

    /**
     * @brief logger class for PP project
     * @author jdhuang (jiande@whu.edu.cn)
     * @date 2024-08-01
     */
    class LibGnut_LIBRARY_EXPORT t_great_log
    {
    public:
        /**
         * @brief Get the instance object
         * @return t_tool_log&
         * @author jdhuang (jiande@whu.edu.cn)
         * @date 2024-08-01
         */
        static t_great_log& get_instance();

        /**
         * @brief init logger for program
         * @param  path
         * @param  level
         * @author jdhuang (jiande@whu.edu.cn)
         * @date 2024-08-01
         */
        void init_logger(const std::string& path, const std::string& level);
        void init_logger(const std::string& path, const spdlog::level::level_enum& level);

        /**
         * @brief release logger
         * @author jdhuang (jiande@whu.edu.cn)
         * @date 2024-08-01
         */
        void drop_log();

        /**
         * @brief Destroy the t tool log object
         * @author jdhuang (jiande@whu.edu.cn)
         * @date 2024-08-01
         */
        virtual ~t_great_log();

        /**
         * @brief
         *
         * @return std::shared_ptr<logger>
         */
        std::shared_ptr<spdlog::logger> get_logger();

    private:
        /**
         * @brief Construct the t_great_log object
         *
         */
        t_great_log();

        /**
         * @brief Construct a new t tool log object
         * @param  other
         * @author jdhuang (jiande@whu.edu.cn)
         * @date 2024-08-01
         */
        t_great_log(const t_great_log& other) = delete;

        /**
         * @brief remove the operator for "="
         * @param  other
         * @return t_tool_log&
         * @author jdhuang (jiande@whu.edu.cn)
         * @date 2024-08-01
         */
        t_great_log& operator=(const t_great_log& other) = delete;

        t_spdlog _gspdlog = nullptr; ///< spdlog ptr
        std::vector<spdlog::sink_ptr> sinks;
    };

    LibGnut_LIBRARY_EXPORT void throw_logical_error(const std::string& message);

/**
 * @brief logger function
 * @author jdhuang (jiande@whu.edu.cn)
 * @date 2024-08-01
 */
#ifndef PLOG_FUNCTION
#define PLOG_FUNCTION

#define SPACE_ROW "========================================================="
#define GREAT_TRACE(...) SPDLOG_LOGGER_CALL(t_great_log::get_instance().get_logger().get(), spdlog::level::trace, __VA_ARGS__)
#define GREAT_DEBUG(...) SPDLOG_LOGGER_CALL(t_great_log::get_instance().get_logger().get(), spdlog::level::debug, __VA_ARGS__)
#define GREAT_INFO(...) SPDLOG_LOGGER_CALL(t_great_log::get_instance().get_logger().get(), spdlog::level::info, __VA_ARGS__)
#define GREAT_WARN(...) SPDLOG_LOGGER_CALL(t_great_log::get_instance().get_logger().get(), spdlog::level::warn, __VA_ARGS__)
#define GREAT_ERROR(...) SPDLOG_LOGGER_CALL(t_great_log::get_instance().get_logger().get(), spdlog::level::err, __VA_ARGS__)
#define GREAT_CRITICAL(...) SPDLOG_LOGGER_CALL(t_great_log::get_instance().get_logger().get(), spdlog::level::critical, __VA_ARGS__)

    // no line
    // #define PLOG_TRACE(...)       t_tool_log::Instance().getLogger().get()->trace(__VA_ARGS__)
    // #define PLOG_DEBUG(...)       t_tool_log::Instance().getLogger().get()->debug(__VA_ARGS__)
    // #define PLOG_INFO(...)        t_tool_log::Instance().getLogger().get()->info(__VA_ARGS__)
    // #define PLOG_WARN(...)        t_tool_log::Instance().getLogger().get()->warn(__VA_ARGS__)
    // #define PLOG_ERROR(...)       t_tool_log::Instance().getLogger().get()->error(__VA_ARGS__)
    // #define PLOG_CRITICAL(...)    t_tool_log::Instance().getLogger().get()->critical(__VA_ARGS__)

#endif
} // namespace gnut

#endif
