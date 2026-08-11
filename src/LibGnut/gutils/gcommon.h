/**
*
* @verbatim
    History
    2011-02-14  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gcommon.h
* @brief       Common utility helpers.
* @author      JD
* @version     1.0.0
* @date        2011-02-14
*
*/

#ifndef GCOMMON_H
#define GCOMMON_H

#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include "gexport/ExportLibGnut.h"

#ifdef _WIN32
#include <direct.h>
#include <Shlwapi.h>
#ifndef GET_CURRENT_PATH
#define GET_CURRENT_PATH _getcwd
#endif
#pragma warning(disable : 4503) // suppress Visual Studio WARNINGS 4503 about truncated decorated names

#else
#include <unistd.h>
#ifndef GET_CURRENT_PATH
#define GET_CURRENT_PATH getcwd
#endif
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif


namespace gnut
{

    static const char lf = 0x0a; ///< = "\n" .. line feed
    static const char cr = 0x0d; ///< = "\r" .. carriage return

#if defined __linux__ || defined __APPLE__
    static const std::string crlf = std::string("") + lf; ///< "\n"
#endif

#if defined _WIN32 || defined _WIN64
    static const std::string crlf = std::string("") + lf; // "\r\n"
#endif

    /** @brief cut crlf. */
    inline std::string cut_crlf(std::string s)
    {
        if (!s.empty() && s[s.length() - 1] == lf)
        {
            s.erase(s.length() - 1);
        }
        if (!s.empty() && s[s.length() - 1] == cr)
        {
            s.erase(s.length() - 1); // glfeng
        }
        return s;
    }

#ifdef DEBUG
#ifndef ID_FUNC
#define ID_FUNC printf(" .. %s\n", __func__)
#endif
#else
#ifndef ID_FUNC
#define ID_FUNC
#endif
#endif

#ifndef SQR
#define SQR(x) ((x) * (x))
#endif
#ifndef SQRT
#define SQRT(x) ((x) <= 0.0 ? 0.0 : sqrt(x))
#endif

    /**
     * @brief tail of a string
     * @param[in] str        original string
     * @param[in] length        the need length
     * @return result string
     */
    inline std::string tail(const std::string& str, const std::size_t length)
    {
        if (length >= str.size())
        {
            return str;
        }
        else
        {
            return str.substr(str.size() - length);
        }
    }
} // namespace gnut

#endif // # GCOMMON_H
