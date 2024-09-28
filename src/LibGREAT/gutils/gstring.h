/**
 * @file         gstring.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        for string
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include <string>
#include <vector>
#include <set>
#include "gexport/ExportLibGREAT.h"
using namespace std;

namespace great
{
    /** @brief string format. */
    LibGREAT_LIBRARY_EXPORT std::string format(const char *fmt, ...);
}
