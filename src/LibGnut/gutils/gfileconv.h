
/**
*
* @verbatim
    History
    2016-06-26  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gfileconv.h
* @brief       Purpose: file conversion utilities
* @author      JD
* @version     1.0.0
* @date        2016-06-26
*
*/

#ifndef GFILECONV_H
#define GFILECONV_H

#include <string>
#include "gexport/ExportLibGnut.h"

#if defined _WIN32 || defined _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

#define GFILE_PREFIX "file://"

#define MAX_PATH_LEN 256

#ifdef WIN32 //|| _WIN64
#include <io.h>
#define ACCESS(fileName, accessMode) _access(fileName, accessMode)
#define MKDIR(path) _mkdir(path)
#else
#define ACCESS(fileName, accessMode) access(fileName, accessMode)
#define MKDIR(path) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

using namespace std;

namespace gnut
{
    LibGnut_LIBRARY_EXPORT string base_name(const string &path); ///< extract file base name
    LibGnut_LIBRARY_EXPORT string dir_name(const string &path);  ///< extract file dir  name
    LibGnut_LIBRARY_EXPORT bool dir_exists(const string &path);  ///< check existance of path
    LibGnut_LIBRARY_EXPORT int make_path(const string &path);    ///< create path recursively
    LibGnut_LIBRARY_EXPORT int make_dir(const string &path);     ///< create single directory
} // namespace

#endif