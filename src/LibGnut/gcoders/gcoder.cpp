/**
*
* @verbatim
History
    -1.0 Jiande Huang  2019-10-25  creat the file.
@endverbatim
* Copyright (c) 2018, Wuhan University. All rights reserved.
*
* @file       gallcoder.cpp
* @brief      main decode function for all files
*
*
* @author     Jiande Huang, Wuhan University
* @version    1.0.0
* @date       2019-10-25
*
*/

#include <vector>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <algorithm>

#include "gcoders/gcoder.h"
#include "gutils/gtypeconv.h"
#include "gutils/gfileconv.h"
#include "gutils/gtimesync.h"
#include "gset/gsetrec.h"

using namespace std;

namespace gnut
{

    t_gcoder::t_gcoder()
    {
    }

    t_gcoder::t_gcoder(t_gsetbase *s, string version, int sz, string id)
        : _class(id),
          _initialized(false),
          _gnss(true),
          _initver(version),
          _close_with_warning(true)
    {
        _init();

        _buffsz = sz;
        _spdlog = nullptr;
        _set = nullptr;
        _irc = 0;
        _hdr = false;

        _ss_position = 0;

        _out_smp = 0;
        _out_len = 0;
        _out_epo = LAST_TIME;

        _buffer = (char *)malloc((_buffsz + 1) * sizeof(char)); // due to realocate function!

        if (s)
            _gset(s); // READ SETTINGS IN CONSTRUCT ONLY!

    }
    t_gcoder::t_gcoder(t_gtime beg, t_gtime end, t_gsetbase *s, string version, int sz, string id)
        : _class(id),
          _initialized(false),
          _gnss(true),
          _initver(version),
          _close_with_warning(true),
          beg_epoch(beg),
          end_epoch(end)
    {
        _init();

        _buffsz = sz;
        _spdlog = nullptr;
        _set = nullptr;
        _irc = 0;
        _hdr = false;

        _ss_position = 0;

        _out_smp = 0;
        _out_len = 0;
        _out_epo = LAST_TIME;
        _buffer = (char *)malloc((_buffsz + 1) * sizeof(char)); // due to realocate function!

        if (s)
            _gset(s); 

    }

    t_gcoder::t_gcoder(t_gsetbase *s, int sz)
        : _initialized(false),
          _gnss(false),
          _close_with_warning(true)
    {
        _init();

        _buffsz = sz;
        _spdlog = nullptr;
        _set = nullptr;
        _irc = 0;
        _hdr = false;

        _ss_position = 0;

        _out_smp = 0;
        _out_len = 0;
        _out_epo = LAST_TIME;

        _buffer = (char *)malloc((_buffsz + 1) * sizeof(char)); // due to realocate function!

        if (s)
            _gset(s); // READ SETTINGS IN CONSTRUCT ONLY!
    }

    t_gcoder::~t_gcoder()
    {
        // use free instead of delete due to realocate function!
        if (_buffer)
        {
            free(_buffer);
            _buffer = NULL; 
        }

        if (_close_with_warning)
        {
            std::sort(_notes.begin(), _notes.end());
            for (auto it = _notes.begin(); it != _notes.end(); ++it)
            {
                if (_spdlog)
                    SPDLOG_LOGGER_WARN(_spdlog, it->str() + " .. " + base_name(_fname));
            }
        }
    }

    int t_gcoder::_gset(t_gsetbase *s)
    {

        if (!s)
            return -1;

        _set = s;
        _beg = dynamic_cast<t_gsetgen *>(_set)->beg();
        _end = dynamic_cast<t_gsetgen *>(_set)->end();

        _hdr = dynamic_cast<t_gsetgen *>(_set)->thin();

        if (_gnss)
            _sys = dynamic_cast<t_gsetgen *>(_set)->sys();

        _rec = dynamic_cast<t_gsetgen *>(_set)->rec_all(); 
        if (_rec.empty())
            _rec = dynamic_cast<t_gsetgen *>(_set)->recs();
        _int = dynamic_cast<t_gsetgen *>(_set)->sampling();
        _scl = dynamic_cast<t_gsetgen *>(_set)->sampling_scalefc(); 


        for (auto itGNSS = _sys.begin(); itGNSS != _sys.end(); ++itGNSS)
        {
            string gs = *itGNSS;
            GSYS gsys = t_gsys::str2gsys(gs);

            _sat[gsys] = dynamic_cast<t_gsetgnss *>(_set)->sat(gsys, false); // empty if not set (to speed up!)
            _obs[gsys] = dynamic_cast<t_gsetgnss *>(_set)->obs(gsys, false); // empty if not set (to speed up!)
            _nav[gsys] = dynamic_cast<t_gsetgnss *>(_set)->nav(gsys, false); // empty if not set (to speed up!)

            set<string> sgobs = dynamic_cast<t_gsetgnss *>(_set)->gobs(gsys);
            for (auto it = sgobs.begin(); it != sgobs.end(); it++)
                _obs[gsys].insert(*it);
        }
        return 0;
    }

    void t_gcoder::spdlog(t_spdlog spdlog)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }
    }

    void t_gcoder::path(const string &s)
    {
        string tmp(s);
        substitute(tmp, GFILE_PREFIX, "");
        _fname = tmp;
    }

    void t_gcoder::mesg(const t_note &n, const string &s)
    {
        t_gnote m(n, "gcoder", s);

        bool exist = false;
        vector<t_gnote>::const_iterator it;
        for (it = _notes.begin(); it != _notes.end(); ++it)
        {
            if (*it == m)
            {
                exist = true;
            }
        }

        if (!exist)
            _notes.push_back(m);
    }

    const vector<t_gnote> &t_gcoder::mesg() const
    {
        return _notes;
    }

    void t_gcoder::_init()
    {

        _version = _initver;
        _endpos = 0;
    }

    void t_gcoder::clear()
    {
        if (_buffer)
        {
            free(_buffer);
            _buffer = NULL;
        }

        _init();

        _buffer = (char *)malloc((_buffsz + 1) * sizeof(char)); // due to realocate function!
    }

    int t_gcoder::_getline(string &str, int from_pos)
    {

        return _decode_buffer.getline(str, from_pos);
    }

    int t_gcoder::_getbuffer(const char *&buff)
    {

        return _decode_buffer.getbuffer(buff);
    }

    int t_gcoder::_add2buffer(char *buff, int sz)
    {

        return _decode_buffer.add(buff, sz);
    }

    int t_gcoder::_consume(const int &bytes_to_eat)
    {

        return _decode_buffer.consume(bytes_to_eat);
    }

    int t_gcoder::add_data(const string &data_id, t_gdata *data)
    {

        if (data_id.empty() || data == 0)
            return -1;

        map<string, t_gdata *>::iterator it = _data.find(data_id);
        if (it != _data.end())
        {
            if (_spdlog)
                SPDLOG_LOGGER_WARN(_spdlog, "warning: structure " + data_id + " already exists !");
            return -1;
        }

        _data[data_id] = data;

        _add_data(data_id, data);

        return 1;
    }

    bool t_gcoder::_filter_epoch(const t_gtime &epo)
    {

        if (time_sync(epo, _int, _scl, _spdlog))
        {
            return true;
        }
        else if (time_sync(epo.diff(_beg), _int, _scl, _spdlog))
        { 
            return true;
        }
        return false;
    }

    bool t_gcoder::_filter_gnss(const string &prn)
    {

        string gs = t_gsys::char2str(prn[0]); // DONT USE SYS FROM ARG WHICH MAY BE EMPTY
        GSYS gsys = t_gsys::str2gsys(gs);     // DONT USE SYS FROM ARG WHICH MAY BE EMPTY
        if ((_sys.size() == 0 || _sys.find(gs) != _sys.end()) &&
            (_sat[gsys].size() == 0 || _sat[gsys].find(prn) != _sat[gsys].end()))
        {
            return true;
        }
        return false;
    }

} 
