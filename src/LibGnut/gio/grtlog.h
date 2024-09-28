/**
*
* @verbatim
History
-1.0  2021-08-01  jdhuang       create the file

@endverbatim
* Copyright (c) 2020, Wuhan University. All rights reserved.
*
* @file         grtlog.h
* @brief        the class for great app log
*
* @author       GREAT Wuhan University
* @version      1.0.0
* @date         2021-08-01
*
*/

#ifndef GRTLOG_H
#define GRTLOG_H


#include "gexport/ExportLibGnut.h"

#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif

#include <string>
#include "spdlog/spdlog.h"

using namespace spdlog;
namespace gnut
{
    typedef std::shared_ptr<spdlog::logger> t_spdlog;

    class LibGnut_LIBRARY_EXPORT t_grtlog
    {
    public:
        /**
         * @brief Construct a new t grtlog object
         */
        t_grtlog();

        /**
         * @brief Construct a new t grtlog object
         * 
         * @param log_type 
         * @param log_level 
         * @param log_name 
         */
        t_grtlog(const std::string &log_type, const level::level_enum &log_level, const std::string &log_name);

        /**
         * @brief Destroy the t grtlog object
         * 
         */
        virtual ~t_grtlog();

        /**
         * @brief 
         * 
         * @return std::shared_ptr<logger> 
         */
        std::shared_ptr<logger> spdlog();

        /**
         * @brief Set the log object
         * 
         * @param log_type 
         * @param log_level 
         * @param log_name 
         */
        void set_log(const std::string &log_type, const level::level_enum &log_level, const std::string &log_name);

        string name() { return _name; }

    private:
        std::string _name;
        std::string _type;
        std::string _pattern;
        level::level_enum _level;
        t_spdlog _spdlog;
    };

    /**
     * @brief 
     * 
     * @param spdlog 
     * @param message 
     * @return LibGnut_LIBRARY_EXPORT 
     */
    LibGnut_LIBRARY_EXPORT void throw_logical_error(t_spdlog spdlog, const std::string &message);

}
#endif
