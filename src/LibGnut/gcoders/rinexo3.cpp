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
#include <memory>
#include <algorithm>

#include "gcoders/rinexo3.h"
#include "gdata/gobsgnss.h"

using namespace std;

namespace gnut
{

    t_rinexo3::t_rinexo3(t_gsetbase *s, string version, int sz)
        : t_rinexo2(s, version, sz)
    {
    }
    t_rinexo3::t_rinexo3(t_gtime beg, t_gtime end, t_gsetbase *s, string version, int sz)
        : t_rinexo2(beg, end, s, version, sz)
    {
    }

    int t_rinexo3::_decode_head()
    {

        // -------- "SYS / # / OBS TYPES" --------
        if (_line.substr(60, 19).find("SYS / # / OBS TYPES") != string::npos)
        {

            // read all lines at once
            string sys(1, _line[0]);

            if (sys == " ")
            {
                mesg(GERROR, "SYS / # / OBS TYPES not recognized SYS!");
                _irc++;
            }
            else
            {

                int num = str2int(_line.substr(3, 3));
                int nlines = (int)ceil(num / 13.0); // up to 13 values/line
                int addsize = 0;
                string tmp;
                for (int nl = 0; nl < nlines; ++nl)
                {
                    int irc = t_gcoder::_getline(tmp, _tmpsize + addsize);
                    if (irc >= 0)
                    {
                        addsize += irc;
                    }
                    else
                    {
                        _complete = false;
                        break;
                    } // incomplete RECORD
                }

                if (!_complete)
                    return -1;

                for (int ii = 0; ii < num; ++ii)
                {                                                     // all linked lines were read
                    string go = _line.substr(7 + 4 * (ii % (13)), 3); // check existing vobstypes
                    _fix_band(sys, go);                               // fix band
                    GOBS obs = str2gobs(go);

                    t_rnxhdr::t_vobstypes::iterator it;
                    for (it = _mapobs[sys].begin(); it != _mapobs[sys].end(); ++it)
                    { // SUBSTITUTE
                        if (it->first == obs)
                        {
                            it->second = 1;
                            break;
                        } // scale fator still 1 (changed later)
                    }
                    _mapobs[sys].push_back(make_pair(obs, 1)); 

                    if ((ii + 1) % 13 == 0 && ii + 1 < num)
                        _tmpsize += t_gcoder::_getline(_line, _tmpsize); // get newline and check!

                } // loop over # observations
            }

            // -------- "SYS / SCALE FACTOR" --------
        }
        else if (_line.substr(60, 18).find("SYS / SCALE FACTOR") != string::npos)
        {

            string sys(1, _line[0]); // read all linked lines at once!

            if (sys == " ")
            {
                mesg(GERROR, "SYS / SCALE FACTOR not recognized SYS!");
                _irc++;
            }
            else
            {
                double fact = str2dbl(_line.substr(2, 4));
                string nobs = _line.substr(8, 2);

                int num = (nobs == "  ") ? 0 : str2int(nobs); // blank = 0 (all observations)
                int nlines = (int)ceil(num / 12.0);           // up to 10 values/line
                int addsize = 0;
                string tmp;
                for (int nl = 0; nl < nlines; ++nl)
                {
                    int cnt = t_gcoder::_getline(tmp, _tmpsize + addsize);
                    if (cnt >= 0)
                    {
                        addsize += cnt;
                    }
                    else
                    {
                        _complete = false;
                        break;
                    } // incomplete RECORD
                }

                if (!_complete)
                    return -1;

                if (num == 0)
                { // SYSTEM-specific (mapkey => SYS, e.g. 'G','R', ..)
                    t_rnxhdr::t_vobstypes::iterator it;
                    for (it = _mapobs[sys].begin(); it != _mapobs[sys].end(); ++it)
                        it->second = fact;
                }
                else
                { // satellite-specific (mapkey => SAT, e.g. 'G10', 'G20', ..)

                    for (int ii = 0; ii < num; ++ii)
                    { // all linked lines were read
                        string go = _line.substr(11 + 4 * (ii % (12)), 3);
                        _fix_band(sys, go);
                        GOBS obs = str2gobs(go); // fix band

                        t_rnxhdr::t_vobstypes::iterator it;
                        for (it = _mapobs[sys].begin(); it != _mapobs[sys].end(); ++it)
                            if (it->first == obs)
                            {
                                it->second = fact;
                                break;
                            }

                        if (it == _mapobs[sys].end())
                            _mapobs[sys].push_back(make_pair(obs, fact));

                        if ((ii + 1) % 12 == 0 && ii + 1 < num)
                            _tmpsize += t_gcoder::_getline(_line, _tmpsize); // get newline and check!

                    } // loop over # observations
                }
            }

            // -------- "SYS / PHASE SHIFT" --------
        }
        else if (_line.substr(60, 17).find("SYS / PHASE SHIFT") != string::npos)
        {

            string sys(1, _line[0]); // read all linked lines at once!
            string obs = trim(_line.substr(2, 3));
            GOBS gobs = X;

            if (sys == " ")
            {
                mesg(GERROR, "SYS / PHASE SHIFT not recognized SYS!");
                _irc++;
            }
            else if (obs.empty())
            {
                mesg(GERROR, "SYS / PHASE SHIFT not recognized OBS:" + obs);
                _irc++;
            }
            else if ((gobs = str2gobs(obs)) == X)
            {
                mesg(GERROR, "SYS / PHASE SHIFT not recognized OBS:" + obs);
                _irc++;
            }
            else
            {
                double qcyc = str2dbl(_line.substr(6, 8));
                string nsat = _line.substr(16, 2);
                int num = (nsat == "  ") ? 0 : str2int(nsat); // blank = 0 (all satellites)
                int nlines = (int)ceil(num / 10.0);           // up to 10 values/line
                int addsize = 0;
                string tmp;
                for (int nl = 0; nl < nlines; ++nl)
                {
                    int cnt = t_gcoder::_getline(tmp, _tmpsize + addsize);
                    if (cnt >= 0)
                    {
                        addsize += cnt;
                    }
                    else
                    {
                        _complete = false;
                        break;
                    } // incomplete RECORD
                }

                if (!_complete)
                    return -1;

                if (num == 0)
                { // SYSTEM-specific (mapkey => SYS, e.g. 'G','R', ..)
                    t_rnxhdr::t_vobstypes::iterator it;
                    for (it = _mapcyc[sys].begin(); it != _mapcyc[sys].end(); ++it)
                    {
                        if (it->first == gobs)
                        {
                            it->second = qcyc;
                            break;
                        }
                    }

                    if (it == _mapcyc[sys].end())
                        _mapcyc[sys].push_back(make_pair(gobs, qcyc));
                }
                else
                { // satellite-specific (mapkey => SAT, e.g. 'G10', 'G20', ..)

                    for (int ii = 0; ii < num; ++ii)
                    { // all linked lines were read
                        string sat = _line.substr(19 + 4 * (ii % (10)), 3);
                        t_rnxhdr::t_vobstypes::iterator it;

                        for (it = _mapcyc[sat].begin(); it != _mapcyc[sat].end(); ++it)
                        {
                            if (it->first == gobs)
                            {
                                it->second = qcyc;
                                break;
                            }
                        }

                        if (it == _mapcyc[sat].end())
                            _mapcyc[sat].push_back(make_pair(gobs, qcyc));

                        if ((ii + 1) % 10 == 0 && ii + 1 < num)
                            _tmpsize += t_gcoder::_getline(_line, _tmpsize); // get newline and check!
                    }
                }
            }

            // -------- "GLONASS SLOT / FRQ # --------
        }
        else if (_line.substr(60, 20).find("GLONASS SLOT / FRQ #") != string::npos)
        {

            int nsat = str2int(_line.substr(0, 3)); // read all linked lines at once!
            GOBS gobs = X;                          // X: ALL OBS!

            if (nsat == 0)
            {
                mesg(GERROR, "GLONASS SLOT / FRQ zero obs codes!");
                _irc++;
            }
            else
            {
                int nlines = (int)ceil(nsat / 8.0); // up to 8 values/line
                int addsize = 0;
                string tmp;
                for (int nl = 0; nl < nlines; ++nl)
                {
                    int cnt = t_gcoder::_getline(tmp, _tmpsize + addsize);
                    if (cnt >= 0)
                    {
                        addsize += cnt;
                    }
                    else
                    {
                        _complete = false;
                        break;
                    } // incomplete RECORD
                }

                if (!_complete)
                    return -1;

                for (int ii = 0; ii < nsat; ++ii)
                { // all linked lines were read
                    string sat = _line.substr(4 + 7 * (ii % (8)), 3);
                    double frq = str2dbl(_line.substr(7 + 7 * (ii % (8)), 3));

                    t_rnxhdr::t_vobstypes::iterator it;
                    for (it = _glofrq[sat].begin(); it != _glofrq[sat].end(); ++it)
                        if (it->first == gobs)
                        {
                            it->second = frq;
                            break;
                        }

                    if (it == _glofrq[sat].end())
                        _glofrq[sat].push_back(make_pair(gobs, frq));

                    if ((ii + 1) % 8 == 0 && ii + 1 < nsat)
                        _tmpsize += t_gcoder::_getline(_line, _tmpsize); // get newline and check
                }

                map<string, t_gdata *>::iterator itDAT = _data.begin();
                while (itDAT != _data.end())
                {
                    if (itDAT->second->id_type() == t_gdata::ALLOBS)
                    {
                        for (auto itfrq : _glofrq)
                        {
                            for (auto itObs : itfrq.second)
                            {
                                if (itObs.first != GOBS::X)
                                    continue;
                                ((t_gallobs *)itDAT->second)->add_glo_freq(itfrq.first, itObs.second);
                            }
                        }
                    }
                    itDAT++;
                }
                // ------------------------------------------------------------------
            }

            // -------- "GLONASS COD/PHS/BIS --------
        }
        else if (_line.substr(60, 19).find("GLONASS COD/PHS/BIS") != string::npos)
        {

            int nobs = 4;
            for (int ii = 0; ii < nobs; ++ii)
            { // single line (4 fields or less)

                string obs = trim(_line.substr(1 + 13 * (ii % (4)), 3));
                if (obs == "")
                    break;
                string val = trim(_line.substr(5 + 13 * (ii % (4)), 8));
                GOBS gobs = str2gobs(obs);
                double bias = str2dbl(val);
                if (gobs == X)
                {
                    mesg(GERROR, "GLONASS COD/PHS/BIS not recognized OBS:" + obs);
                    _irc++;
                }
                else
                {
                    t_rnxhdr::t_vobstypes::iterator it;
                    for (it = _globia.begin(); it != _globia.end(); ++it)
                    {
                        if (it->first == gobs)
                        {
                            it->second = bias;
                            break;
                        }
                    }

                    if (it == _globia.end())
                        _globia.push_back(make_pair(gobs, bias));
                }
            }

            // -------- "RCV CLOCK OFFS APPL" --------
        }
        else if (_line.substr(60, 19).find("RCV CLOCK OFFS APPL") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "RCV CLOCK OFFS APPL: " + string("not implemented"));

            // --------"# OF SATELLITES" --------
        }
        else if (_line.substr(60, 15).find("# OF SATELLITES") != string::npos)
        {

            _rnxhdr.numsats(str2int(_line.substr(0, 6)));

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "# OF SATELLITES: " + int2str(_rnxhdr.numsats()));

            // --------"PRN / # OF OBS" --------
        }
        else if (_line.substr(60, 20).find("PRN / # OF OBS") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "PRN / # OF OBS: " + string("not implemented"));

            // -------- "SIGNAL STRENGTH UNIT" --------
        }
        else if (_line.substr(60, 20).find("SIGNAL STRENGTH UNIT") != string::npos)
        {
            _rnxhdr.strength(trim(_line.substr(0, 20)));

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "SIGNAL STRENGTH UNIT: " + _rnxhdr.strength());

            // -------- "SYS / DCBS APPL" --------
        }
        else if (_line.substr(60, 15).find("SYS / DCBS APPL") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "SYS / DCBS APPL: " + string("not implemented"));

            // -------- "SYS / PCVS APPL" --------
        }
        else if (_line.substr(60, 15).find("SYS / PCVS APPL") != string::npos)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "SYS / PCVS APPL: " + string("not implemented"));
        }
        else if (t_rinexo2::_decode_head())
        {
        };

        return 1;
    }

    int t_rinexo3::_decode_data()
    {

        int irc = t_rinexo3::_read_epoch();

        // not ready to read
        if (irc < 0)
        {
            _complete = false;
            return -1;
        }

        // skip special lines
        if (irc > 0)
        {
            for (int i = 0; i < _nsat; ++i)
            {
                int sz = t_gcoder::_getline(_line, _tmpsize);
                if (sz <= 0)
                {
                    _complete = false;
                    return -1;
                }
                if (_line.substr(0, 1) == ">" && irc == 200)
                {
                    break;
                }
                _tmpsize += sz;
                //      cout << "_line skip: " << _line;
            }

            t_gcoder::_consume(_tmpsize);
            _consume += _tmpsize;
            _tmpsize = 0;
            return 0;
        }

        // DATA READING
        // ------------
        // loop over satellits records (x-lines)
        for (int i = 0; i < _nsat; i++)
        {

            string sat = "";
            string key = "";

            // loop over all observation types for system/satellite key in _mapobs
            if (t_rinexo3::_read_obstypes(sat, key) < 0)
            {
                _complete = 0;
                return -1;
            }
        }

        if (_complete)
        {

            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, _epoch.str_ymdhms("reading finished at "));

            if (beg_epoch == t_gtime(0, 0) && end_epoch == t_gtime(0, 0))
            {
                _count += this->_fill_data();
            }
            else
            {
                if ((_epoch >= beg_epoch) && (_epoch <= end_epoch))
                {
                    _count += this->_fill_data();
                }
            }
            t_gcoder::_consume(_tmpsize);
            _consume += _tmpsize;
            _tmpsize = 0;
        }

        return 0;
    }

    int t_rinexo3::_check_head()
    {

        t_rinexo2::_check_head();

        if (_rnxhdr.rnxsys() == ' ')
        {
            mesg(GERROR, "RINEX SYS not available!");
            _irc++;
        } // mandatory
        if (_rnxhdr.mapcyc().size() == 0)
        {
            mesg(GERROR, "SYS PHASE SHIFT not available!");
            _irc++;
        } // mandatory
        if (_rnxhdr.glofrq().size() == 0)
        {
            mesg(GERROR, "GLO SLOT/FREQ not available!");
            _irc++;
        } // mandatory

        if (_version >= "3.02" &&
            _rnxhdr.globia().size() == 0)
        {
            mesg(GERROR, "GLO BIASES not available!");
            _irc++;
        } // mandatory

        return _irc;
    }

    int t_rinexo3::_read_epoch()
    {

        _nsat = 0;
        _flag = 'X';

        // not enough data to recognize standard epoch or special event
        if (_line.length() < 35)
            return _stop_read();
        _flag = _line[31]; //  cout << "_flag = [" << _flag << "]\n";
        _nsat = str2int(_line.substr(32, 3));

        if (_line.substr(0, 1) != ">")
            return 200;

        if (_line.substr(1, 6) == "      ")
        {
            switch (_flag)
            {
            case '2':
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Warning: start moving antenna identified, but not yet implemented!");
                return 1;
            case '3':
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Warning: new site occupation identified, but not yet implemented!");
                return 1;

            case '4':
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Warning: new header information identified, but not fully implemented!");
                return 1;
            case '5':
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Warning: external event identified, but not yet implemented!");
                return 1;
            }
        }
        string dummy;
        int yr, mn, dd, hr, mi;
        double sc;
        istringstream is(_line);
        is.clear();

        is >> dummy >> yr >> mn >> dd >> hr >> mi >> sc >> _flag >> _nsat;

        // check success
        if (is.fail())
        {
            return _stop_read();
        }

        _epoch.from_ymdhms(yr, mn, dd, hr, mi, sc);
        epoch = _epoch;
        // filter out data
        if (_flag != '0' &&
            _flag != '1')
        {
            return 1;
        }
        if (_epoch > _end)
        {
            if (_epoch > _epo_end)
            {
                _epo_end = _epoch;
            }
            _xend++;
            return 2;
        }
        if (_epoch < _beg)
        {
            if (_epoch < _epo_beg)
            {
                _epo_beg = _epoch;
            }
            _xbeg++;
            return 2;
        }

        if (!_filter_epoch(_epoch))
        {
            _xsmp++;
            return 2;
        }

        return 0;
    }

    int t_rinexo3::_read_obstypes(const string &sat, const string &sys)
    {

        int ii = 0;
        int addsize = 0;
        unsigned int idx = 0;

        // general sys- or sat- specific group of observations
        string key = sat;
        t_rnxhdr::t_obstypes::const_iterator itMAP = _mapobs.find(sat);
        if (itMAP == _mapobs.end())
        {
            key = sys;
        }

        string tmpsat("");

        // read new line for individual satellite (complete observations)
        if ((addsize = t_gcoder::_getline(_line, _tmpsize)) >= 3)
        {
            _tmpsize += addsize;
            tmpsat = t_gsys::eval_sat(_line.substr(1, 2), t_gsys::char2gsys(_line[0]));
        }
        else
            return _stop_read();

        // filter GNSS and SAT
        bool filter_sat = _filter_gnss(tmpsat);
        if (!filter_sat)
        {
            _xsys++;
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "skip " + tmpsat);
        }

        t_spt_gobs obs = make_shared<t_gobsgnss>(_spdlog, _site, tmpsat, _epoch);

        // loop over sys-defined observation types
        t_rnxhdr::t_vobstypes::const_iterator it = _mapobs[tmpsat.substr(0, 1)].begin();
        unsigned int len;
        while (_complete && (it != _mapobs[tmpsat.substr(0, 1)].end()))
        {
            idx = 3 + 16 * ii;
            len = _line.length();

            // getline succeed only if 'EOL' found
            if (len <= 0 && len < (idx + 14))
                return _stop_read();

            // check completness (excluding last SNR+LLI - sometimes missing)
            if (len < (idx + 14))
            {
                if (filter_sat)
                    _null_log(tmpsat, gobs2str(it->first));
            }
            else
            {
                if (filter_sat)
                    _read_obs(idx, it, obs);
            }

            ii++;
            it++;

        } // while (over observation types)

        if (filter_sat)
            _vobs.push_back(obs);

        return 1;
    }

    int t_rinexo3::_fix_band(string sys, string &go)
    {

        // EXCEPTION FOR BEIDOU (B1 --> B2)
        if (t_gsys::str2gsys(sys) == BDS)
        {
            if (go[1] == '1' && _version <= "3.03")
            {
                go[1] = '2';

                if (_spdlog)
                    SPDLOG_LOGGER_WARN(_spdlog, "Warning: Both C1x and C2x coding should be accepted and treated as C2x in RINEX 3.02/3.03");
            }
            if (go[1] == '3')
            {
                go[1] = '6'; // for B3I (change C3I -> C6I or so on !!!)

                if (_spdlog)
                    SPDLOG_LOGGER_WARN(_spdlog, "Warning: BDS band changed: C3x -> C6x ");
            }
            if (go[1] == '7' && _version >= "3.04")
            {
                if (go[2] == 'D' || go[2] == 'P' || go[2] == 'Z')
                {
                    go[1] = '9'; // Distinguish BDS-3 B2b and BDS-2 B2b(B2I)
                }

                if (_spdlog)
                    SPDLOG_LOGGER_WARN(_spdlog, "Warning: Changing BDS-3 C7D/C7P/C7Z to C9D/C9P/C9Z in RINEX 3.04");
            }
        }

        return 1;
    }

} // namespace
