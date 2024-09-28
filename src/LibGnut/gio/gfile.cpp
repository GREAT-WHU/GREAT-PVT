/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <cstring>
#include <sstream>

#include "gio/gfile.h"
#include "gutils/gcommon.h"
#include "gutils/gfileconv.h"

using namespace std;

namespace gnut
{
    t_gfile::t_gfile(t_spdlog spdlog)
        : t_gio(spdlog),
          _irc(0),
          _gzip(false)
    {

        _file = 0;
        _size = FILEBUF_SIZE;
    }

    t_gfile::~t_gfile()
    {

        reset();
    }

    int t_gfile::path(string str)
    {

        if (str == "")
            return -1;

        size_t idx1 = 0, idx2 = 0;
        string prefix = GFILE_PREFIX;

        // check if file
        idx2 = str.find(prefix);
        if (idx2 == string::npos)
        {
            str = prefix + str;
            idx2 = 0;
        }

        // file path
        idx1 = idx2 + prefix.length();
        idx2 = str.length();
        if (idx2 != string::npos && idx2 > idx1)
        {

            string name = str.substr(idx1, idx2 - idx1);
            _set_gzip(name);

            if (_file == 0)
                _file = new t_giof(name.c_str());
            else
                return -1;

            if (_file)
                _file->mask(name);
            else
            {
                cerr << "cannot assign file to path!\n";
                _irc++;
                return -1;
            }

            ostringstream ltmp;
            ltmp << "File: " << int(idx1) << ":" << int(idx2) << " = " << name;
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, ltmp.str());
        }
        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_WARN(_spdlog, "warning: path does not contain file://dir/name  [check file://]");
            return -1;
        }

        t_gio::path(str);

        return 1;
    }

    string t_gfile::path()
    {
        return GFILE_PREFIX + mask();
    }

    string t_gfile::name()
    {

        return mask();
    }

    bool t_gfile::eof()
    {
        if (_file)
            return _file->eof();

        return true;
    }

    string t_gfile::mask()
    {
        if (_file)
            return _file->mask();

        return "";
    }

    int t_gfile::init_read()
    {

        if (!_coder)
        {
            return 0;
        }

        char *loc_buff = new char[FILEHDR_SIZE];

        int nbytes = 0;
        vector<string> errmsg;
        while ((nbytes = _gio_read(loc_buff, FILEHDR_SIZE)) > 0 && _stop != 1)
        {
            if (_coder->decode_head(loc_buff, nbytes, errmsg) < 0)
                break;
        }

        if (nbytes < 0)
        { 
            ++_irc;
            if (_coder)
                _coder->mesg(GERROR, "Incomplete header identified.");
        }
        delete[] loc_buff;
        return 1;
    }

    void t_gfile::reset()
    {

        if (_file)
        {
            delete _file;
            _file = 0;
        }
    }

    int t_gfile::_gio_read(char *buff, int size)
    {

        if (mask() == "")
            return -1;
        int nbytes = this->_read(buff, size);

        if (nbytes == 0 && this->eof())
            return -1;
        return nbytes;

    }

    int t_gfile::_stop_common()
    {

        return t_gio::_stop_common();
    }

    void t_gfile::_set_gzip(string name)
    {

        if ( 
            name.compare(name.length() - 3, name.length(), ".gz") == 0)

            _gzip = true;
        else
            _gzip = false;
    }

    int t_gfile::_read(char *b, int s)
    {
        if (_file)
            return _file->read(b, s);

        return -1;
    }

} // namespace
