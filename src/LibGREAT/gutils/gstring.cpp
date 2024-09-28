/**
 * @file         gstring.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        for string
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gutils/gstring.h"

#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <math.h>
#include <algorithm>

namespace great
{
    std::string format(const char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int len = vsnprintf(nullptr, 0, fmt, ap);
        va_end(ap);
        std::string buf(len + 1, '\0');
        va_start(ap, fmt);
        vsnprintf(&buf[0], buf.size(), fmt, ap);
        va_end(ap);
        buf.pop_back();
        return buf;
    }
}