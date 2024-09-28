
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

  This file is part of the G-Nut C++ library.
 
-*/

#include <iostream>
#include <iomanip>

#include "gutils/gobs.h"

using namespace std;

namespace gnut
{
    bool t_gattr::valid() const
    {
        return (_gattr != ATTR);
    }

    void t_gattr::attr(const GOBSATTR &a)
    {
        _gattr = a;
    }

    GOBSATTR t_gattr::attr() const
    {
        return _gattr;
    }

    bool t_gattr::operator==(const t_gattr &g) const
    {
        return (_gattr == g.attr());
    }

    bool t_gband::valid() const
    {
        return (t_gattr::valid() && _gband != BAND);
    }

    void t_gband::band(const GOBSBAND &b)
    {
        _gband = b;
    }

    GOBSBAND t_gband::band() const
    {
        return _gband;
    }

    void t_gband::gattr(const t_gattr &g)
    {
        _gattr = g.attr();
        _gband = BAND;
    }

    t_gattr t_gband::gattr() const
    {
        t_gattr g(_gattr);
        return g;
    }

    bool t_gband::operator==(const t_gband &g) const
    {
        return (_gband == g.band() &&
                _gattr == g.attr());
    }

    bool t_gobs::valid() const
    {
        return (t_gband::valid() && _gtype != TYPE);
    }

    void t_gobs::type(const GOBSTYPE &t)
    {
        _gtype = t;
    }

    GOBSTYPE t_gobs::type() const
    {
        return _gtype;
    }

    void t_gobs::gband(const t_gband &g)
    {
        _gattr = g.attr();
        _gband = g.band();
        _gtype = TYPE;
    }

    t_gband t_gobs::gband() const
    {
        t_gband g(_gband, _gattr);
        return g;
    }

    bool t_gobs::operator==(const t_gobs &g) const
    {
        return (_gtype == g.type() &&
                _gband == g.band() &&
                _gattr == g.attr());
    }

    int t_gobs::gobs(const GOBS &g)
    {
        string s = gobs2str(g);
        _gtype = str2gobstype(s);
        _gband = str2gobsband(s);
        _gattr = str2gobsattr(s);

        return 1;
    }

    int t_gobs::gobs(const string &s)
    {
        _gtype = str2gobstype(s);
        _gband = str2gobsband(s);
        _gattr = str2gobsattr(s);

        return 1;
    }

    GOBS t_gobs::gobs() const
    {
        string s = gobstype2str(_gtype) +
                   gobsband2str(_gband) +
                   gobsattr2str(_gattr);
        return str2gobs(s);
    }

    GOBS t_gobs::gobs2CH(GSYS gs) const
    {
        GOBS g = X;

        if (_gattr == ATTR_NULL)
        { // already 2char signal
            g = this->gobs();
        }
        else
        {
            if (_gtype == TYPE_C)
            {
                if (_gattr != ATTR_C && (gs == GPS || gs == GLO))
                    g = tba2gobs(TYPE_P, _gband, ATTR_NULL);
                else
                    g = tba2gobs(_gtype, _gband, ATTR_NULL);
            }
            if (_gtype == TYPE_L)
            {
                g = tba2gobs(_gtype, _gband, ATTR_NULL);
            }
        }

        return g;
    }

    GOBS t_gobs::gobs3CH() const
    {
        GOBS g = X;

        if (_gattr != ATTR_NULL && _gattr != ATTR)
        { // already 3char signal
            g = this->gobs();
        }
        else
        {
            if (_gtype == TYPE_C)
            {
                g = tba2gobs(_gtype, _gband, ATTR_C);
            }
            else if (_gtype == TYPE_P)
            {
                g = tba2gobs(TYPE_C, _gband, ATTR_W);
            }
        }

        return g;
    }

    void t_gobs::gobs2to3(GSYS sys)
    {
        if (_gattr != ATTR_NULL)
        {
            return;
        }
        GOBS obs2 = this->gobs();
        switch (sys)
        {
        case gnut::GPS:
            if (obs2 == L1 || obs2 == L2 || obs2 == C1 || obs2 == C2 || obs2 == D1 || obs2 == D2 || obs2 == S1 || obs2 == S2)
            {
                _gattr = ATTR_C;
            }
            else if (obs2 == L5 || obs2 == C5 || obs2 == D5 || obs2 == S5)
            {
                _gattr = ATTR_Q;
            }
            else if (obs2 == P1 || obs2 == P2 || obs2 == P5)
            {
                _gtype = TYPE_C;
                _gattr = ATTR_W;
            }
            break;
        case gnut::GAL: 
            if (obs2 == L1 || obs2 == L6 || obs2 == C1 || obs2 == C6 || obs2 == S1 || obs2 == S6 || obs2 == D1 || obs2 == D6)
            {
                _gattr = ATTR_X;
            }
            else if (obs2 == L5 || obs2 == L7 || obs2 == L8 || obs2 == C5 || obs2 == C7 || obs2 == C8 || obs2 == D5 || obs2 == D7 || obs2 == D8 || obs2 == S5 || obs2 == S7 || obs2 == S8)
            {
                _gattr = ATTR_I;
            }
            else if (obs2 == P5)
            {
                _gtype = TYPE_C;
                _gattr = ATTR_Q;
            }
            break;
        case gnut::GLO: 
            if (obs2 == L1 || obs2 == L2 || obs2 == C1 || obs2 == C2 || obs2 == D1 || obs2 == D2 || obs2 == S1 || obs2 == S2)
            {
                _gattr = ATTR_C;
            }
            else if (obs2 == P1 || obs2 == P2)
            {
                _gtype = TYPE_C;
                _gattr = ATTR_P;
            }
            break;
        case gnut::BDS: 
            if (obs2 == L1 || obs2 == L7 || obs2 == C1 || obs2 == C7 || obs2 == S1 || obs2 == S7 || obs2 == D1 || obs2 == D7)
            {
                _gattr = ATTR_I;
            }
            else if (obs2 == L2 || obs2 == C2 || obs2 == D2 || obs2 == S2)
            {
                _gband = BAND_7;
                _gattr = ATTR_I;
            }
            else if (obs2 == P1)
            {
                _gtype = TYPE_C;
                _gattr = ATTR_Q;
            }
            else if (obs2 == P2)
            {
                _gtype = TYPE_C;
                _gband = BAND_7;
                _gattr = ATTR_Q;
            }
            else if (obs2 == C5 || obs2 == L5)
            {
                _gband = BAND_3;
                _gattr = ATTR_I;
            }
            break;
        case gnut::QZS: 
            if (obs2 == L1 || obs2 == L5 || obs2 == C1 || obs2 == C5)
            {
                _gattr = ATTR_C;
            }
            else if (obs2 == D1 || obs2 == D5 || obs2 == S1 || obs2 == S5)
            {
                _gattr = ATTR_C;
            }
            break;
        case gnut::SBS:
            break;
        case gnut::IRN:
            break;
        case gnut::GNS:
            break;
        default:
            break;
        }
        return;
    }

    bool t_gobs::is_code() const
    {
        return (_gtype == TYPE_C || _gtype == TYPE_P);
    }

    bool t_gobs::is_phase() const
    {
        return (_gtype == TYPE_L);
    }
    bool t_gobs::is_doppler() const
    {
        return (_gtype == TYPE_D);
    }
} // namespace
