
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

using namespace std;

namespace gnut
{

    bool double_eq(const double &a, const double &b)
    {
        if (fabs(a - b) < numeric_limits<double>::epsilon())
            return true; // must be editeed with machine epsilon (FOLLOWING FLOAT BELLOW!)
        else
            return false;
    }

    bool float_eq(const float &a, const float &b)
    {
        if (fabs(a - b) < numeric_limits<float>::epsilon())
            return true;
        else
            return false;
    }

    double dround(double d)
    {
        return floor(d + 0.5);
    }

    string dbl2str(const double &d, int prec)
    {
        ostringstream out;
        out << fixed << setprecision(prec) << " " << setw(0) << d;
        return out.str();
    }

#ifndef STR2DBL
    double str2dbl(const string &s)
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
    double str2dbl(const string &s)
    {
        return str2dbl(s.c_str());
    }
    // string to double conversion (avoiding blanks)      // http://tinodidriksen.com/2011/05/28/cpp-convert-string-to-double-speed/
    // ----------
    double str2dbl(const char *p)
    {

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')
#define null_double 0.0;

        //Skip leading white space, if any.
        while (white_space(*p))
            p += 1;

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
                return null_double; // we got no  exponent! this was not intended!!

            double scaleE = 1.0;

            // Calculate scaling factor.
            while (e >= 50)
            {
                scaleE *= 1E50;
                e -= 50;
            }
            //while (e >=  8) { scaleE *= 1E8;  e -=  8; }
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
            ++p;

        if (*p != '\0')
        {
            return null_double;
        } // if next character is not the terminating character

        // Apply sign to number
        if (neg)
            r = -r;

        return r;
    }
#endif

    double strSci2dbl(const string &s)
    {
        double i = 0.0;
        string str(s);
        substitute(str, "d", "E");
        substitute(str, "D", "E");
        istringstream istr(str);
        istr >> i;
        return i;
    }

    string int2str(const int &i)
    {
        ostringstream out;
        out << i;
        return out.str();
    }

    string int2str(const int &i, const int &width)
    {
        ostringstream out;
        out << setw(width) << setfill('0') << i;
        return out.str();
    }

    int str2int(const string &s)
    {
        int i = 0;
        istringstream istr(s);
        istr >> i;
        return i;
    }

    string bl2str(const bool &s)
    {
        if (s)
            return string("TRUE");
        else
            return string("FALSE");
    }

    string rtrim(const string &s)
    {
        string str;
        size_t endpos = s.find_last_not_of(" \t");
        if (string::npos != endpos)
            str = s.substr(0, endpos + 1);
        return str;
    }

    string ltrim(const string &s)
    {
        string str;
        size_t startpos = s.find_first_not_of(" \t");
        if (string::npos != startpos)
            str = s.substr(startpos);
        return str;
    }

    string trim(const string &s)
    {
        return ltrim(rtrim(s));
    }

    size_t substitute(string &line, const string &a, const string &b, bool caseSensitive)
    {
        size_t n = 0;

        if (caseSensitive)
        {
            string tmp(line);
            while ((n = line.find(a)) != string::npos)
            {
                tmp = line.substr(0, n) + b + line.substr(n + a.length());
                line = tmp;
            }
        }
        else
        {
            string lineLC(line);
            string findLC(a);
            transform(lineLC.begin(), lineLC.end(), lineLC.begin(), ::tolower);
            transform(findLC.begin(), findLC.end(), findLC.begin(), ::tolower);

            while ((n = lineLC.find(findLC)) != string::npos)
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

} // namespace
