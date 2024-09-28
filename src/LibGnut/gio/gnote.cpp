/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include "gio/gnote.h"
#include "gutils/gcommon.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{

    t_gnote::t_gnote(t_note n, string f, string s)
    {

        _stat = n;
        _func = f;
        _text = s;
    }

    t_gnote::~t_gnote()
    {
    }

    ostream &operator<<(ostream &os, const t_gnote &n)
    {
        os << n.str();
        return os;
    }

    bool t_gnote::operator==(const t_gnote &n) const
    {
        return (n.status() == _stat &&
                n.func() == _func &&
                n.text() == _text);
    }

    bool t_gnote::operator<(const t_gnote &n) const
    {
        return (n.status() < _stat &&
                n.func() < _func &&
                n.text() < _text);
    }

    string t_gnote::_str() const
    {
        string note;
        switch (_stat)
        {
        case GMESSAGE:
            note = "Message - ";
            break;
        case GWARNING:
            note = "Warning - ";
            break;
        case GERROR:
            note = "Error - ";
            break;
        }

        return note;
    }

    t_gallnote::t_gallnote()
    {
    }

    t_gallnote::~t_gallnote()
    {

        this->clear();
    }

    void t_gallnote::clear()
    {

        _gmutex.lock();
        _gnotes.clear();
        _gmutex.unlock();
    }

    void t_gallnote::mesg(t_note note, string func, string text)
    {

        _gmutex.lock();

        t_gnote gnote(note, func, text);

        // eliminate repeating messages
        bool exist = false;
        for (auto it = _gnotes.begin(); it != _gnotes.end(); ++it)
        {
            if (*it == gnote)
            {
                exist = true;
            }
        }
        if (!exist)
            _gnotes.push_back(gnote);

        _gmutex.unlock();
        return;
    }

    vector<t_gnote> t_gallnote::mesg()
    {

        return _gnotes;
    }

} // namespace