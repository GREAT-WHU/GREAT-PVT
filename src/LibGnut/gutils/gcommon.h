/**
*
* @verbatim
    History
    2011-02-14  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gcommon.h
* @brief       gzstream, C++ iostream classes wrapping the zlib compression library.
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
#define GET_CURRENT_PATH _getcwd
#pragma warning(disable : 4503) // suppress Visual Studio WARNINGS 4503 about truncated decorated names

#else
#include <unistd.h>
#define GET_CURRENT_PATH getcwd
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode)&S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode)&S_IFMT) == S_IFREG)
#endif

using namespace std;

namespace gnut
{

    static const char lf = 0x0a; ///< = "\n" .. line feed
    static const char cr = 0x0d; ///< = "\r" .. carriage return

#if defined __linux__ || defined __APPLE__
    static const string crlf = string("") + lf; ///< "\n"
#endif

#if defined _WIN32 || defined _WIN64
    static const string crlf = string("") + lf; // "\r\n"
#endif

    /** @brief cut crlf. */
    inline string cut_crlf(string s)
    {
        if (!s.empty() && s[s.length() - 1] == lf)
            s.erase(s.length() - 1);
        if (!s.empty() && s[s.length() - 1] == cr)
            s.erase(s.length() - 1); // glfeng
        return s;
    }

#ifdef DEBUG
#define ID_FUNC printf(" .. %s\n", __func__)
#else
#define ID_FUNC
#endif

#define SQR(x) ((x) * (x))
#define SQRT(x) ((x) <= 0.0 ? 0.0 : sqrt(x))

    /**
    * @brief tail of a string
    * @param[in] str        original string
    * @param[in] length        the need length
    * @return result string
    */
    inline string tail(const string &str, const size_t length)
    {
        if (length >= str.size())
            return str;
        else
            return str.substr(str.size() - length);
    }
} // namespace

#endif // # GCOMMON_H
