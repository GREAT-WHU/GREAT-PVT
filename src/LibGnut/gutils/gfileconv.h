
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
#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR "\\"
#endif
#else
#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR "/"
#endif
#endif

#ifndef GFILE_PREFIX
#define GFILE_PREFIX "file://"
#endif

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 256
#endif

#ifdef WIN32 //|| _WIN64
#include <io.h>
#ifndef ACCESS
#define ACCESS(fileName, accessMode) _access(fileName, accessMode)
#endif
#ifndef MKDIR
#define MKDIR(path) _mkdir(path)
#endif
#else
#ifndef ACCESS
#define ACCESS(fileName, accessMode) access(fileName, accessMode)
#endif
#ifndef MKDIR
#define MKDIR(path) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif
#endif


namespace gnut
{
    LibGnut_LIBRARY_EXPORT std::string base_name(const std::string& path); ///< extract file base name
    LibGnut_LIBRARY_EXPORT std::string dir_name(const std::string& path);  ///< extract file dir  name
    LibGnut_LIBRARY_EXPORT bool dir_exists(const std::string& path);  ///< check existance of path
    LibGnut_LIBRARY_EXPORT int make_path(const std::string& path);    ///< create path recursively
    LibGnut_LIBRARY_EXPORT int make_dir(const std::string& path);     ///< create single directory
} // namespace gnut

#endif
