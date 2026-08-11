/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <iostream>

#include "gio/giof.h"
#include "gutils/gcommon.h"
#include "gutils/gtypeconv.h"
#include "gutils/gfileconv.h"


namespace gnut
{

    t_giof::t_giof(std::string mask) :
        _irc(0),
        _mask(mask),
        _name(""),
        _omode(std::ios::trunc),
        _repl(false),
        _toff(0),
        _loop(false),
        _tsys(t_gtime::t_tsys::UTC)
    {
        substitute(mask, GFILE_PREFIX, "");

        this->exceptions(std::fstream::failbit | std::fstream::badbit);
        if (_mask.find('%') != std::string::npos)
        {
            _repl = true;
        }
        _name = _replace();
    }

    t_giof::~t_giof()
    {

        if (this->is_open())
        {
            try
            {
                this->close();
            }
            catch (std::exception e)
            {
                std::cerr << "Exception closing file " << _name << ": " << e.what() << std::endl;
            }
        }
    }

    int t_giof::mask(std::string mask)
    {

        _irc = 0;
        substitute(mask, GFILE_PREFIX, "");
#if defined _WIN32 || defined _WIN32
        substitute(mask, "/", PATH_SEPARATOR);
#else
        substitute(mask, "\\", PATH_SEPARATOR);
#endif
        this->clear();
        if (this->is_open())
        {
            try
            {
                this->close();
            }
            catch (std::exception e)
            {
                std::cerr << "Exception closing file " << _name << ": " << e.what() << std::endl;
                _irc++;
                return -1;
            }
        }
        if (mask.find('%') != std::string::npos)
        {
            _repl = true;
        }
        _mask = mask;
        _name = _replace();
        return 1;
    }

    int t_giof::write(const char* buff, int size)
    {
        if (_mask == "" || size < 1)
        {
            return -1;
        }


        std::string name = _replace();

        this->clear();
        if (!this->is_open() || name != _name)
        {
            if (this->is_open())
            {
                try
                {
                    this->close();
                }
                catch (std::exception e)
                {
                    std::cerr << "Exception closing file " << _name << ": " << e.what() << std::endl;
                    _irc++;
                    return -1;
                }
            }

            try
            {
                _name = name;
                make_path(_name);
                this->open(_name.c_str(), std::ios::binary | std::ios::out | _omode);
            }
            catch (std::exception e)
            {
                std::cerr << "Exception opening file " << _name << ": " << e.what() << std::endl;
                _irc++;
                return -1;
            }
        }

        try
        {
            std::fstream::write(buff, size);
            std::fstream::flush();
        }
        catch (std::exception e)
        {
            std::cerr << "Exception writting to file " << _name << ": " << e.what() << std::endl;
            _irc++;
            return -1;
        }

        return size;
    }

    int t_giof::read(char* buff, int size)
    {
        if (_mask == "")
        {
            return -1;
        }

        _name = _replace();

        int nbytes;
        for (int i = 0; i < 2; i++)
        {
            this->clear();
            if (!this->is_open())
            {
                try
                {
                    make_path(_name);
                    this->open(_name.c_str(), std::ios::binary | std::ios::in);
                }
                catch (std::exception e)
                {
                    std::cerr << "Exception opening file " << _name << ": " << e.what() << std::endl;
                    _irc++;
                    return -1;
                }
            }

            nbytes = size;
            try
            {
                std::fstream::read(buff, size);
            }
            catch (std::exception e)
            {
                if (this->eof())
                {
                    nbytes = (int)this->gcount();

                    if (_loop)
                    {
                        this->close();
                    }

                    if (nbytes < 1)
                    {
                        // there was no data left before eof, let's try opening the file again
                        continue;
                    }
                }
                else
                {
                    std::cerr << "Exception reading from file " << _name << ": " << e.what() << std::endl;
                    _irc++;
                    return -1;
                }
            }
            break; // nbytes is more than 0, we can break
        }
        return nbytes;
    }

    void t_giof::append(const bool& b)
    {
        if (b)
        {
            _omode = std::ios::app;
        }
        else
        {
            _omode = std::ios::trunc;
        }
    }

    void t_giof::tsys(t_gtime::t_tsys ts)
    {
        _tsys = ts;
        return;
    }

    t_gtime::t_tsys t_giof::tsys()
    {
        t_gtime::t_tsys tsys = _tsys;
        return tsys;
    }

    std::string t_giof::_replace()
    {
        if (!_repl)
        {
            return _mask;
        }

        t_gtime file_tm = t_gtime::current_time(_tsys);
        file_tm.add_secs(_toff * 60.0);
        return file_tm.str(_mask);
    }

} // namespace gnut
