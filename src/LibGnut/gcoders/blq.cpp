/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include "gcoders/blq.h"
#include "gutils/gtriple.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{

    t_blq::t_blq(t_gsetbase *s, string version, int sz)
        : t_gcoder(s, version, sz)
    {
    }


    int t_blq::decode_head(char *buff, int sz, vector<string> &errmsg)
    {

        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };

        string line;
        int consume = 0;
        int tmpsize = 0;
        while ((tmpsize = t_gcoder::_getline(line)) >= 0)
        {

            consume += tmpsize;

            if (line.find("END HEADER") != string::npos)
            {

                t_gcoder::_consume(tmpsize);
                _mutex.unlock();
                return -1;
            }

            t_gcoder::_consume(tmpsize);
        } // end while

        _mutex.unlock();
        return consume;
    }

    int t_blq::decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {

        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };


        string line;
        int consume = 0;
        int tmpsize = 0;
        int sitsize = 0;
        bool complete = false;

        while ((tmpsize = t_gcoder::_getline(line, sitsize)) >= 0)
        {
            sitsize += tmpsize;
            consume += tmpsize;
            complete = false;

            if (line.substr(0, 2).compare("$$") == 0)
            {
                unsigned int pos = 0;
                if ((line.find("lon/lat:")) != string::npos)
                { // read lat, lon
                    pos = line.find("lon/lat:");
                    line.erase(0, pos + 8);
                    istringstream istr(line);
                    istr >> _lon >> _lat;
                    if (_lon > 180)
                    {
                        _lon -= 360.0;
                    }
                    continue;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                istringstream istr(line);
                double a, b, c, d, e, f, g, h, i, j, k;
                istr >> a >> b >> c >> d >> e >> f >> g >> h >> i >> j >> k;
                if (!istr.fail())
                {
                    int ii = 1;
                    _blqdata.ReSize(6, 11);
                    _blqdata.Row(ii) << a << b << c << d << e << f << g << h << i << j << k;
                    ii++;
                    while (ii <= 6)
                    {
                        if ((tmpsize = t_gcoder::_getline(line, sitsize)) < 0)
                        {
                            break;
                        }
                        istringstream istr(line);
                        istr >> a >> b >> c >> d >> e >> f >> g >> h >> i >> j >> k;
                        _blqdata.Row(ii) << a << b << c << d << e << f << g << h << i << j << k;
                        ii++;
                        sitsize += tmpsize;
                        consume += tmpsize;
                        if (ii == 6)
                            complete = true;
                    }
                }
                else
                {
                    istringstream istr(line);
                    istr >> _site;
                    _site = _site.substr(0, 4); //trim(_site);
                }
            }

            if (complete)
            {
                t_gotl otl(_spdlog);
                map<string, t_gdata *>::iterator it = _data.begin();
                if (it != _data.end())
                    otl.setdata(_site, _lon, _lat, _blqdata);

                while (it != _data.end())
                {
                    if (it->second->id_type() == t_gdata::ALLOTL)
                    {
                        ((t_gallotl *)it->second)->add(otl);
                    }
                    it++;
                }
                t_gcoder::_consume(sitsize);
                sitsize = 0;
            }
        }

        _mutex.unlock();
        return consume;
    }

} // namespace
