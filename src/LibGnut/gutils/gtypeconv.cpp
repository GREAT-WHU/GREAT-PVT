
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <algorithm>
#include <sstream>
#include <cmath>
#include <limits>
#include <stdlib.h>

#include "gutils/gtypeconv.h"
#include "gutils/gconst.h"


namespace gnut
{

    bool double_eq(const double& a, const double& b)
    {
        if (fabs(a - b) < std::numeric_limits<double>::epsilon())
        {
            return true; // must be editeed with machine epsilon (FOLLOWING FLOAT BELLOW!)
        }
        else
        {
            return false;
        }
    }

    bool float_eq(const float& a, const float& b)
    {
        if (fabs(a - b) < std::numeric_limits<float>::epsilon())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    double dround(double d)
    {
        return floor(d + 0.5);
    }

    std::string dbl2str(const double& d, int prec)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(prec) << " " << std::setw(0) << d;
        return out.str();
    }

#ifndef STR2DBL
    double str2dbl(const std::string& s)
    {
        return strtod(s.c_str(), NULL);

        // 2x slower alternative using stringstream !

        //  double i;
        //  istringstream istr(s);
        //  istr >> i;
        //  return i;
    }
#else
    // http://tinodidriksen.com/2011/05/28/cpp-convert-string-to-double-speed/
    // http://pastebin.com/dHP1pgQ4
    // bool naive(T & r, const char *p)
    double str2dbl(const std::string& s)
    {
        return str2dbl(s.c_str());
    }
    // string to double conversion (avoiding blanks)      //
    // http://tinodidriksen.com/2011/05/28/cpp-convert-string-to-double-speed/
    // ----------
    double str2dbl(const char* p)
    {

#ifndef white_space
#define white_space(c) ((c) == ' ' || (c) == '\t')
#endif
#ifndef valid_digit
#define valid_digit(c) ((c) >= '0' && (c) <= '9')
#endif
#ifndef null_double
#define null_double 0.0;
#endif

        // Skip leading white space, if any.
        while (white_space(*p))
        {
            p += 1;
        }

        double r = 0.0;
        int c = 0; // counter to check how many numbers we got!

        // Get the sign!
        bool neg = false;
        if (*p == '-')
        {
            neg = true;
            ++p;
        }
        else if (*p == '+')
        {
            neg = false;
            ++p;
        }

        // Get the digits before decimal point
        while (valid_digit(*p))
        {
            r = (r * 10.0) + (*p - '0');
            ++p;
            ++c;
        }

        // Get the digits after decimal point
        if (*p == '.')
        {
            double f = 0.0;
            double scale = 1.0;
            ++p;
            while (valid_digit(*p))
            { // while ( *p >= '0' && *p <= '9' )
                f = (f * 10.0) + (*p - '0');
                ++p;
                scale *= 10.0;
                ++c;
            }
            r += f / scale;
        }

        // FIRST CHECK:
        if (c == 0)
        {
            return null_double;
        } // we got no decimal places! this cannot be any number!

        // Get the digits after the "e"/"E" (exponent)
        if (*p == 'e' || *p == 'E' || *p == 'd' || *p == 'D')
        {
            int e = 0;

            bool negE = false;
            ++p;
            if (*p == '-')
            {
                negE = true;
                ++p;
            }
            else if (*p == '+')
            {
                negE = false;
                ++p;
            }

            // Get exponent
            c = 0;
            while (valid_digit(*p))
            {
                e = (e * 10) + (*p - '0');
                ++p;
                ++c;
            }

            if (!neg && e > std::numeric_limits<double>::max_exponent10)
            {
                e = std::numeric_limits<double>::max_exponent10;
            }
            else if (e < std::numeric_limits<double>::min_exponent10)
            {
                e = std::numeric_limits<double>::max_exponent10;
            }

            // SECOND CHECK:
            if (c == 0)
            {
                return null_double; // we got no  exponent! this was not intended!!
            }

            double scaleE = 1.0;

            // Calculate scaling factor.
            while (e >= 50)
            {
                scaleE *= 1E50;
                e -= 50;
            }
            // while (e >=  8) { scaleE *= 1E8;  e -=  8; }
            while (e > 0)
            {
                scaleE *= 10.0;
                e -= 1;
            }

            if (negE)
            {
                r /= scaleE;
            }
            else
            {
                r *= scaleE;
            }
        }

        // POST CHECK:
        // skip post whitespaces
        while (white_space(*p))
        {
            ++p;
        }

        if (*p != '\0')
        {
            return null_double;
        } // if next character is not the terminating character

        // Apply sign to number
        if (neg)
        {
            r = -r;
        }

        return r;
    }
#endif

    double strSci2dbl(const std::string& s)
    {
        double i = 0.0;
        std::string str(s);
        substitute(str, "d", "E");
        substitute(str, "D", "E");
        std::istringstream istr(str);
        istr >> i;
        return i;
    }

    std::string int2str(const int& i)
    {
        std::ostringstream out;
        out << i;
        return out.str();
    }

    std::string int2str(const int& i, const int& width)
    {
        std::ostringstream out;
        out << std::setw(width) << std::setfill('0') << i;
        return out.str();
    }

    int str2int(const std::string& s)
    {
        int i = 0;
        std::istringstream istr(s);
        istr >> i;
        return i;
    }

    std::string bl2str(const bool& s)
    {
        if (s)
        {
            return std::string("TRUE");
        }
        else
        {
            return std::string("FALSE");
        }
    }

    std::string rtrim(const std::string& s)
    {
        std::string str;
        std::size_t endpos = s.find_last_not_of(" \t");
        if (std::string::npos != endpos)
        {
            str = s.substr(0, endpos + 1);
        }
        return str;
    }

    std::string ltrim(const std::string& s)
    {
        std::string str;
        std::size_t startpos = s.find_first_not_of(" \t");
        if (std::string::npos != startpos)
        {
            str = s.substr(startpos);
        }
        return str;
    }

    std::string trim(const std::string& s)
    {
        return ltrim(rtrim(s));
    }

    std::size_t substitute(std::string& line, const std::string& a, const std::string& b, bool caseSensitive)
    {
        std::size_t n = 0;

        if (caseSensitive)
        {
            std::string tmp(line);
            while ((n = line.find(a)) != std::string::npos)
            {
                tmp = line.substr(0, n) + b + line.substr(n + a.length());
                line = tmp;
            }
        }
        else
        {
            std::string lineLC(line);
            std::string findLC(a);
            transform(lineLC.begin(), lineLC.end(), lineLC.begin(), ::tolower);
            transform(findLC.begin(), findLC.end(), findLC.begin(), ::tolower);

            while ((n = lineLC.find(findLC)) != std::string::npos)
            {
                lineLC = line.substr(0, n) + b + line.substr(n + a.length());
                line = lineLC;
            }
        }

        return n + b.length();
    }

    double frac(double x)
    {
        return x - floor(x);
    }

} // namespace gnut
