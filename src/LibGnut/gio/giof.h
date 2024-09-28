/**
*
* @verbatim
    History
    2011-10-15  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file      giof.h
* @brief
*.  Purpose: class derived from fstream
*           - file name/mask added
*           - support file hunting according to date/time
*           - automated open/close handling
* @author   JD
* @version  1.0.0
* @date     2011-01-15
*
*/

#ifndef GIOF_H
#define GIOF_H

#include <fstream>
#include <string>

#ifdef BMUTEX
#include <boost/thread/mutex.hpp>
#endif

#include "gutils/gtime.h"
#include "gutils/gmutex.h"
#include "gexport/ExportLibGnut.h"

using namespace std;

namespace gnut
{
    /** @brief class for fstream. */
    class LibGnut_LIBRARY_EXPORT t_giof : public fstream
    {

    public:
        /** @brief constructor 1. */
        explicit t_giof(string mask = "");

        /** @brief default destructor. */
        virtual ~t_giof();

        /** @brief set/get file mask. */
        int mask(string mask);
        string mask() const { return _mask; }

        /** @brief get last filename. */
        string name() const { return _name; }

        /** @brief get irc status. */
        int irc() const { return _irc; };

        /** @brief set/get loop read. */
        void loop(bool l) { _loop = l; }
        bool loop() const { return _loop; }

        /** @brief set/get time offset [min] for the file name. */
        void toff(int i) { _toff = i; }
        int toff() const { return _toff; }

        /** @brief writting. */
        virtual int write(const char *buff, int size);

        /** @brief reading. */
        int read(char *buff, int size);

        /** @brief append mode [false/true]. */
        void append(const bool &b = true);

        /** @brief set/get time system for replacement. */
        void tsys(t_gtime::t_tsys);
        t_gtime::t_tsys tsys();

    protected:
        string _replace(); ///< replace mask to name

        int _irc;              ///  irc status OK=0, Warning>0, Error<0
        string _mask;          ///< original name mask
        string _name;          ///< actual (evaluated) name
        ios::openmode _omode;  ///< output open mode
        bool _repl;            ///< replace if time-specific
        int _toff;             ///< if replace, time offset [min] for the file name
        bool _loop;            ///< loop read
        t_gtime::t_tsys _tsys; ///< time system for replacement
        t_gmutex _gmutex;

    private:
    };

} // namespace

#endif
