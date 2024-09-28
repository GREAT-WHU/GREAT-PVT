/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <cstring>
#include <iostream>

#include "gio/gio.h"
#include "gutils/gcommon.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{

    t_gio::t_gio()
        : _fd(-1),
          _size(BUF_SIZE),
          _path(""),
          _giof(),
          _count(0),
          _verb(0),
          _stop(0),
          _opened(0),
          _running(0)
    {
        _coder = 0;
    }

    t_gio::t_gio(t_spdlog spdlog)
        : _fd(-1),
          _size(BUF_SIZE),
          _path(""),
          _giof(),
          _count(0),
          _verb(0),
          _stop(0),
          _opened(0),
          _running(0)
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

        _coder = 0;
    }

    t_gio::~t_gio()
    {
    }

    int t_gio::path(string str)
    {

        if (str == "")
            return -1;

        _path = str;
        return 1;
    }

    ostream &operator<<(ostream &os, const t_gio &n)
    {
        os << n.path();
        return os;
    }

    bool t_gio::operator==(const t_gio &n) const
    {
        return (n.path() == _path);
    }

    void t_gio::spdlog(t_spdlog spdlog)
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

    bool t_gio::operator<(const t_gio &n) const
    {
        return (n.path() < _path);
    }

    void t_gio::run_read()
    {

        _gmutex.lock();

        // be sure always clean decoder
        if (_coder)
        {
            _coder->clear();
        }
        // For error message
        vector<string> errmsg;

        char *loc_buff = new char[_size];

        if (init_read() < 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_WARN(_spdlog, " warning - initialization failed");
            t_gtime::gmsleep(3000); // [ms]
                                    //  sleep(3);               // [s] linux only
        }
        int nbytes = 0;
        _stop = 0;
        _running = 1;

        while (((nbytes = _gio_read(loc_buff, _size)) > 0) && _stop != 1)
        {
            // archive the stream
            _locf_write(loc_buff, nbytes);
            // volatile int decoded = 0;
            if (_coder && nbytes > 0)
            {
                _coder->decode_data(loc_buff, nbytes, _count, errmsg);
                
                if (_coder->end_epoch > t_gtime(0, 0))
                {
                    if (_coder->epoch > _coder->end_epoch)
                    {
                        break;
                    }
                }
            }
            else
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "0 data decoded");
            }

        }

        _stop_common();
        delete[] loc_buff;
        _gmutex.unlock();
        return;
    }

    int t_gio::_locf_read(char *buff, int size)
    {

        if (_giof.mask() == "")
            return -1;

        return _giof.read(buff, size);
    }

    int t_gio::_locf_write(const char *buff, int size)
    {

        if (_giof.mask() == "")
            return -1;

        int irc = _giof.write(buff, size);

        return irc;
    }

    int t_gio::_init_common()
    {

        return 1;
    }

    int t_gio::_stop_common()
    {

        _opened = 0; // for TCP/NTRIP now, need to be checked for gfile!
        _running = 0;
        return _running;
    }

} // namespace