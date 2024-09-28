
/**
*
* @verbatim
    History
    2017-08-04  JD: created
    2018-09-14  JD: updated

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gnote.h
* @brief      Purpose: implements motes (messages,warning,errors)
*.
* @author     JD
* @version    1.0.0
* @date       2017-08-04
*
*/

#ifndef GNOTE_H
#define GNOTE_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>
#include <stdio.h>
#include <fstream>

#include "gutils/gmutex.h"

#define BUF_SIZE 1024

using namespace std;

namespace gnut
{

    /** @brief the type of NOTE. */
    enum t_note
    {
        GERROR,
        GWARNING,
        GMESSAGE
    };

    /** @brief class for t_gnote. */
    class LibGnut_LIBRARY_EXPORT t_gnote
    {

    public:
        /** @brief constructor 1. */
        t_gnote(t_note n, string f, string s);

        /** @brief default destructor. */
        virtual ~t_gnote();

        /** @brief util function. */
        string str() const { return _str() + _text; }
        string text() const { return _text; }
        string func() const { return _func; }
        t_note status() const { return _stat; }

        /** @brief override operator. */
        bool operator<(const t_gnote &n) const;
        bool operator==(const t_gnote &n) const;
        friend ostream &operator<<(ostream &os, const t_gnote &n);

    protected:
        virtual string _str() const;

        string _func; ///< note function
        string _text; ///< note text
        t_note _stat; ///< note status

    private:
    };

    // container for gallnotes, should not be derived from gdata as others
    // ----------

    /** @brief class for t_gallnote. */
    class t_gallnote
    {

    public:
        /** @brief default constructor. */
        t_gallnote();

        /** @brief default destructor. */
        virtual ~t_gallnote();

        /** @brief set/get notes (messages/warning/errors). */
        void mesg(t_note note, string func, string text);

        /** @brief get note (messages/warning/errors). */
        vector<t_gnote> mesg();

        void clear();

    protected:
        mutable t_gmutex _gmutex;
        vector<t_gnote> _gnotes; ///< cummulate notes message/warning/error
    };

} // namespace

#endif
