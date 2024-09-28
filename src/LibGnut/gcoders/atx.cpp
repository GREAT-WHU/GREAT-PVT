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

#include "gall/gallpcv.h"
#include "gcoders/atx.h"
#include "gutils/gsys.h"
#include "gutils/gtime.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{

    t_atx::t_atx(t_gsetbase *s, string version, int sz)
        : t_gcoder(s, version, sz)
    {
    }

    int t_atx::decode_head(char *buff, int sz, vector<string> &errmsg)
    {

        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        }

        string tmp;
        int consume = 0;
        int tmpsize = 0;
        while ((tmpsize = t_gcoder::_getline(tmp)) >= 0)
        {

            consume += tmpsize;
            if (tmp.find("ANTEX VERSION", 60) != string::npos)
            { // first line

                _version = tmp.substr(0, 8);
                if (_spdlog && substitute(_version, " ", "") > 0)
                {
                    ostringstream ltmp;
                    ltmp << "version = " << _version;

                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, ltmp.str());
                }
            }
            else if (tmp.find("PCV TYPE / REFANT", 60) != string::npos)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "reading PCV TYPE / REFANT");
            }
            else if (tmp.find("COMMENT", 60) != string::npos)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "reading COMMENT");
            }
            else if (tmp.find("END OF HEADER", 60) != string::npos)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "reading END OF HEADER ");
                t_gcoder::_consume(tmpsize);
                _mutex.unlock();
                return -1;
            }
            t_gcoder::_consume(tmpsize);
        }

        _mutex.unlock();
        return consume;
    }

    int t_atx::decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg)
    {

        _mutex.lock();

        if (t_gcoder::_add2buffer(buff, sz) == 0)
        {
            _mutex.unlock();
            return 0;
        };

        int tmpsize = 0;   // individual reading counter
        int consume = 0;   // total read counter
        int recsize = 0;   // single record counter
        int rms = false;   // indicator for rms
        bool freq = false; // indicator for frequency
        string line;
        istringstream istr;
        string freqstr;
        // loop over current buffer (all records)
        // always from beginning of the buffer
        while ((tmpsize = t_gcoder::_getline(line, recsize)) >= 0)
        {
            recsize = tmpsize; //initialize

            // START OF ANTENNA
            // identify first antenna record
            if (line.find("START OF ANTENNA", 60) != string::npos)
            {

                if ((tmpsize = t_gcoder::_getline(line, recsize)) <= 0)
                {
                    _mutex.unlock();
                    return consume;
                }

                GFRQ freq1(LAST_GFRQ); // freq1 = XXX;
                GFRQ freq2(LAST_GFRQ); // freq2 = XXX;

                t_gtriple neu;
                t_gpcv::t_map_Z mapZ;
                t_gpcv::t_map_A mapA;
                shared_ptr<t_gpcv> pcv = make_shared<t_gpcv>(_spdlog);
                pcv->beg(FIRST_TIME);
                pcv->end(LAST_TIME);
                t_gtime beg(t_gtime::GPS), end(t_gtime::GPS);

                // loop over a single record (keeping unfinished record in buffer)
                // begin from START OF ANTENNA
                while ((tmpsize = t_gcoder::_getline(line, recsize)) >= 0)
                {
                    recsize += tmpsize; // increase counter

                    // END OF ANTENNA
                    // end of antenna record --> process it
                    if (line.find("END OF ANTENNA", 60) != string::npos)
                    {

                        // fill pcv
                        map<string, t_gdata *>::iterator it = _data.begin();
                        while (it != _data.end())
                        {
                            if (it->second->id_type() == t_gdata::ALLPCV)
                            {
                                dynamic_cast<t_gallpcv *>(it->second)->addpcv(pcv);
                            }

                            it++;
                        }
                        consume += t_gcoder::_consume(recsize);
                        recsize = 0;
                        cnt++;

                        break; // exit a single antenna record loop

                        // TYPE / SERIAL NO
                    }
                    else if (line.find("TYPE / SERIAL NO", 60) != string::npos)
                    {

                        pcv->anten(trim(line.substr(0, 20)));
                        pcv->ident(trim(line.substr(20, 20)));
                        pcv->svcod(trim(line.substr(40, 20)));

                        // METH / BY / # / DATE
                    }
                    else if (line.find("METH / BY / # / DATE", 60) != string::npos)
                    {
                        pcv->method(trim(line.substr(0, 20)));
                        pcv->source(trim(line.substr(20, 20)));

                        // DAZI
                    }
                    else if (line.find("DAZI", 60) != string::npos)
                    {
                        pcv->dazi(str2dbl(line.substr(2, 6)));
                        // ZEN1 / ZEN2 / DZEN
                    }
                    else if (line.find("ZEN1 / ZEN2 / DZEN", 60) != string::npos)
                    {
                        pcv->zen1(str2dbl(line.substr(2, 6)));
                        pcv->zen2(str2dbl(line.substr(8, 6)));
                        pcv->dzen(str2dbl(line.substr(14, 6)));

                        // # OF FREQUENCIES
                    }
                    else if (line.find("# OF FREQUENCIES", 60) != string::npos)
                    {

                        // VALID FROM
                    }
                    else if (line.find("VALID FROM", 60) != string::npos)
                    {

                        int yr, mn, dd, hr, mi;
                        double sc;
                        istr.clear();
                        istr.str(line);
                        istr >> yr >> mn >> dd >> hr >> mi >> sc;
                        if (istr.fail())
                        {
                            _mutex.unlock();
                            return consume;
                        }

                        beg.from_ymd(yr, mn, dd, (hr * 3600 + mi * 60), sc);
                        pcv->beg(beg);

                        // VALID UNTIL
                    }
                    else if (line.find("VALID UNTIL", 60) != string::npos)
                    {
                        int yr, mn, dd, hr, mi;
                        double sc;

                        istr.clear();
                        istr.str(line);
                        istr >> yr >> mn >> dd >> hr >> mi >> sc;
                        if (istr.fail())
                        {
                            _mutex.unlock();
                            return consume;
                        }

                        end.from_ymd(yr, mn, dd, (hr * 3600 + mi * 60), sc);
                        pcv->end(end);

                        // SINEX CODE
                    }
                    else if (line.find("SINEX CODE", 60) != string::npos)
                    {
                        pcv->snxcod(line.substr(0, 10));

                        // COMMENT
                    }
                    else if (line.find("COMMENT", 60) != string::npos)
                    {

                    }
                    else if (line.find("START OF FREQUENCY", 60) != string::npos)
                    {
                        freq = true;
                        freqstr = line.substr(3, 3);
                        freq1 = t_gfreq::str2gfreq(freqstr);

                        // NORTH / EAST / UP
                    }
                    else if (freq && line.find("NORTH / EAST / UP", 60) != string::npos)
                    {
                        istr.clear();
                        istr.str(line);
                        istr >> neu[0] >> neu[1] >> neu[2];
                        if (istr.fail())
                        {
                            _mutex.unlock();
                            return consume;
                        }

                        // END OF FREQUENCY
                        // end of frequency record -> break loop
                    }
                    else if (line.find("END OF FREQUENCY", 60) != string::npos)
                    {
                        freq = false;
                        freqstr = line.substr(3, 3);
                        freq2 = t_gfreq::str2gfreq(freqstr);
                        if (freq1 == freq2)
                        {
                            if (freq1 == LAST_GFRQ)
                            {
                                if (_spdlog)
                                    SPDLOG_LOGGER_ERROR(_spdlog, "Not defined frequency code " + line.substr(3, 3));
                            }
                            else
                            {
                                pcv->pco(freq1, neu);

                                pcv->pcvzen(freq1, mapZ);
                                pcv->pcvazi(freq1, mapA);
                            }
                        }
                        else
                        {
                            cout << "WARNING: INCOMPLETE FREQUENCY END " << freq1 << " x " << freq2 << " skipped" << endl;
                            consume += t_gcoder::_consume(recsize);
                            _mutex.unlock();
                            return consume;
                        }

                        // START OF FREQ RMS
                    }
                    else if (line.find("START OF FREQ RMS", 60) != string::npos)
                    {
                        freq = true;
                        rms = true;

                        // NORTH / EAST / UP
                    }
                    else if (freq && rms && line.find("NORTH / EAST / UP", 60) != string::npos)
                    {
                        // not yet implemented !
                        //
                        // END OF FREQ RMS
                    }
                    else if (freq && rms && line.find("END OF FREQ RMS", 60) != string::npos)
                    {
                        freq = false;
                        rms = false;
                        // not yet implemented !

                        // NOAZI
                    }
                    else if (freq && line.find("NOAZI", 3) != string::npos)
                    {
                        istr.clear();
                        istr.str(line);
                        string dummy;
                        istr >> dummy;
                        for (double i = pcv->zen1(); i <= pcv->zen2(); i += pcv->dzen())
                            istr >> mapZ[i];

                        if (istr.fail())
                        {
                            _mutex.unlock();
                            return consume;
                        }

                        // AZIMUTH-DEPENDENT
                    }
                    else if (freq)
                    {
                        istr.clear();
                        istr.str(line);
                        double azi;
                        istr >> azi;
                        t_gpcv::t_map_Z mapZ_A;
                        for (double i = pcv->zen1(); i <= pcv->zen2(); i += pcv->dzen())
                        {
                            istr >> mapZ_A[i];
                        }
                        mapA[azi] = mapZ_A;

                        if (istr.fail())
                        {
                            _mutex.unlock();
                            return consume;
                        }
                    }
                    else
                    {
                        cout << endl
                             << "atx: record not recognized: " << line << endl
                             << endl;
                    }
                } // LOOP INSIDE INDIVIDIUAL ANTENNA
            }     // END OF ANTENNA START
        }         // LOOP OVER ALL ANTENNAS

        _mutex.unlock();
        return consume;
    }


} // namespace
