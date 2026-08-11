
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

  This file is part of the G-Nut C++ library.

-*/

#include <iostream>
#include <iomanip>

#include "gutils/gobs.h"


namespace gnut
{
    bool t_gattr::valid() const
    {
        return (_gattr != gnut::GOBSATTR::ATTR);
    }

    void t_gattr::attr(const GOBSATTR& a)
    {
        _gattr = a;
    }

    GOBSATTR t_gattr::attr() const
    {
        return _gattr;
    }

    bool t_gattr::operator==(const t_gattr& g) const
    {
        return (_gattr == g.attr());
    }

    bool t_gband::valid() const
    {
        return (t_gattr::valid() && _gband != gnut::GOBSBAND::BAND);
    }

    void t_gband::band(const GOBSBAND& b)
    {
        _gband = b;
    }

    GOBSBAND t_gband::band() const
    {
        return _gband;
    }

    void t_gband::gattr(const t_gattr& g)
    {
        _gattr = g.attr();
        _gband = gnut::GOBSBAND::BAND;
    }

    t_gattr t_gband::gattr() const
    {
        t_gattr g(_gattr);
        return g;
    }

    bool t_gband::operator==(const t_gband& g) const
    {
        return (_gband == g.band() && _gattr == g.attr());
    }

    bool t_gobs::valid() const
    {
        return (t_gband::valid() && _gtype != gnut::GOBSTYPE::TYPE);
    }

    void t_gobs::type(const GOBSTYPE& t)
    {
        _gtype = t;
    }

    GOBSTYPE t_gobs::type() const
    {
        return _gtype;
    }

    void t_gobs::gband(const t_gband& g)
    {
        _gattr = g.attr();
        _gband = g.band();
        _gtype = gnut::GOBSTYPE::TYPE;
    }

    t_gband t_gobs::gband() const
    {
        t_gband g(_gband, _gattr);
        return g;
    }

    bool t_gobs::operator==(const t_gobs& g) const
    {
        return (_gtype == g.type() && _gband == g.band() && _gattr == g.attr());
    }

    int t_gobs::gobs(const GOBS& g)
    {
        std::string s = gobs2str(g);
        _gtype = str2gobstype(s);
        _gband = str2gobsband(s);
        _gattr = str2gobsattr(s);

        return 1;
    }

    int t_gobs::gobs(const std::string& s)
    {
        _gtype = str2gobstype(s);
        _gband = str2gobsband(s);
        _gattr = str2gobsattr(s);

        return 1;
    }

    GOBS t_gobs::gobs() const
    {
        std::string s = gobstype2str(_gtype) + gobsband2str(_gband) + gobsattr2str(_gattr);
        return str2gobs(s);
    }

    GOBS t_gobs::gobs2CH(GSYS gs) const
    {
        GOBS g = gnut::GOBS::X;

        if (_gattr == gnut::GOBSATTR::ATTR_NULL)
        { // already 2char signal
            g = this->gobs();
        }
        else
        {
            if (_gtype == gnut::GOBSTYPE::TYPE_C)
            {
                if (_gattr != gnut::GOBSATTR::ATTR_C && (gs == GSYS::GPS || gs == GSYS::GLO))
                {
                    g = tba2gobs(gnut::GOBSTYPE::TYPE_P, _gband, gnut::GOBSATTR::ATTR_NULL);
                }
                else
                {
                    g = tba2gobs(_gtype, _gband, gnut::GOBSATTR::ATTR_NULL);
                }
            }
            if (_gtype == gnut::GOBSTYPE::TYPE_L)
            {
                g = tba2gobs(_gtype, _gband, gnut::GOBSATTR::ATTR_NULL);
            }
        }

        return g;
    }

    GOBS t_gobs::gobs3CH() const
    {
        GOBS g = gnut::GOBS::X;

        if (_gattr != gnut::GOBSATTR::ATTR_NULL && _gattr != gnut::GOBSATTR::ATTR)
        { // already 3char signal
            g = this->gobs();
        }
        else
        {
            if (_gtype == gnut::GOBSTYPE::TYPE_C)
            {
                g = tba2gobs(_gtype, _gband, gnut::GOBSATTR::ATTR_C);
            }
            else if (_gtype == gnut::GOBSTYPE::TYPE_P)
            {
                g = tba2gobs(gnut::GOBSTYPE::TYPE_C, _gband, gnut::GOBSATTR::ATTR_W);
            }
        }

        return g;
    }

    void t_gobs::gobs2to3(GSYS sys)
    {
        if (_gattr != gnut::GOBSATTR::ATTR_NULL)
        {
            return;
        }
        GOBS obs2 = this->gobs();
        switch (sys)
        {
            case gnut::GSYS::GPS:
                if (obs2 == gnut::GOBS::L1 || obs2 == gnut::GOBS::L2 || obs2 == gnut::GOBS::C1 || obs2 == gnut::GOBS::C2 || obs2 == gnut::GOBS::D1 || obs2 == gnut::GOBS::D2 || obs2 == gnut::GOBS::S1 || obs2 == gnut::GOBS::S2)
                {
                    _gattr = gnut::GOBSATTR::ATTR_C;
                }
                else if (obs2 == gnut::GOBS::L5 || obs2 == gnut::GOBS::C5 || obs2 == gnut::GOBS::D5 || obs2 == gnut::GOBS::S5)
                {
                    _gattr = gnut::GOBSATTR::ATTR_Q;
                }
                else if (obs2 == gnut::GOBS::P1 || obs2 == gnut::GOBS::P2 || obs2 == gnut::GOBS::P5)
                {
                    _gtype = gnut::GOBSTYPE::TYPE_C;
                    _gattr = gnut::GOBSATTR::ATTR_W;
                }
                break;
            case gnut::GSYS::GAL:
                if (obs2 == gnut::GOBS::L1 || obs2 == gnut::GOBS::L6 || obs2 == gnut::GOBS::C1 || obs2 == gnut::GOBS::C6 || obs2 == gnut::GOBS::S1 || obs2 == gnut::GOBS::S6 || obs2 == gnut::GOBS::D1 || obs2 == gnut::GOBS::D6)
                {
                    _gattr = gnut::GOBSATTR::ATTR_X;
                }
                else if (obs2 == gnut::GOBS::L5 || obs2 == gnut::GOBS::L7 || obs2 == gnut::GOBS::L8 || obs2 == gnut::GOBS::C5 || obs2 == gnut::GOBS::C7 || obs2 == gnut::GOBS::C8 || obs2 == gnut::GOBS::D5 || obs2 == gnut::GOBS::D7 ||
                         obs2 == gnut::GOBS::D8 || obs2 == gnut::GOBS::S5 || obs2 == gnut::GOBS::S7 || obs2 == gnut::GOBS::S8)
                {
                    _gattr = gnut::GOBSATTR::ATTR_I;
                }
                else if (obs2 == gnut::GOBS::P5)
                {
                    _gtype = gnut::GOBSTYPE::TYPE_C;
                    _gattr = gnut::GOBSATTR::ATTR_Q;
                }
                break;
            case gnut::GSYS::GLO:
                if (obs2 == gnut::GOBS::L1 || obs2 == gnut::GOBS::L2 || obs2 == gnut::GOBS::C1 || obs2 == gnut::GOBS::C2 || obs2 == gnut::GOBS::D1 || obs2 == gnut::GOBS::D2 || obs2 == gnut::GOBS::S1 || obs2 == gnut::GOBS::S2)
                {
                    _gattr = gnut::GOBSATTR::ATTR_C;
                }
                else if (obs2 == gnut::GOBS::P1 || obs2 == gnut::GOBS::P2)
                {
                    _gtype = gnut::GOBSTYPE::TYPE_C;
                    _gattr = gnut::GOBSATTR::ATTR_P;
                }
                break;
            case gnut::GSYS::BDS:
                if (obs2 == gnut::GOBS::L1 || obs2 == gnut::GOBS::L7 || obs2 == gnut::GOBS::C1 || obs2 == gnut::GOBS::C7 || obs2 == gnut::GOBS::S1 || obs2 == gnut::GOBS::S7 || obs2 == gnut::GOBS::D1 || obs2 == gnut::GOBS::D7)
                {
                    _gattr = gnut::GOBSATTR::ATTR_I;
                }
                else if (obs2 == gnut::GOBS::L2 || obs2 == gnut::GOBS::C2 || obs2 == gnut::GOBS::D2 || obs2 == gnut::GOBS::S2)
                {
                    _gband = gnut::GOBSBAND::BAND_7;
                    _gattr = gnut::GOBSATTR::ATTR_I;
                }
                else if (obs2 == gnut::GOBS::P1)
                {
                    _gtype = gnut::GOBSTYPE::TYPE_C;
                    _gattr = gnut::GOBSATTR::ATTR_Q;
                }
                else if (obs2 == gnut::GOBS::P2)
                {
                    _gtype = gnut::GOBSTYPE::TYPE_C;
                    _gband = gnut::GOBSBAND::BAND_7;
                    _gattr = gnut::GOBSATTR::ATTR_Q;
                }
                else if (obs2 == gnut::GOBS::C5 || obs2 == gnut::GOBS::L5)
                {
                    _gband = gnut::GOBSBAND::BAND_3;
                    _gattr = gnut::GOBSATTR::ATTR_I;
                }
                break;
            case gnut::GSYS::QZS:
                if (obs2 == gnut::GOBS::L1 || obs2 == gnut::GOBS::L5 || obs2 == gnut::GOBS::C1 || obs2 == gnut::GOBS::C5)
                {
                    _gattr = gnut::GOBSATTR::ATTR_C;
                }
                else if (obs2 == gnut::GOBS::D1 || obs2 == gnut::GOBS::D5 || obs2 == gnut::GOBS::S1 || obs2 == gnut::GOBS::S5)
                {
                    _gattr = gnut::GOBSATTR::ATTR_C;
                }
                break;
            case gnut::GSYS::SBS:
                break;
            case gnut::GSYS::IRN:
                break;
            case gnut::GSYS::GNS:
                break;
            default:
                break;
        }
        return;
    }

    bool t_gobs::is_code() const
    {
        return (_gtype == gnut::GOBSTYPE::TYPE_C || _gtype == gnut::GOBSTYPE::TYPE_P);
    }

    bool t_gobs::is_phase() const
    {
        return (_gtype == gnut::GOBSTYPE::TYPE_L);
    }
    bool t_gobs::is_doppler() const
    {
        return (_gtype == gnut::GOBSTYPE::TYPE_D);
    }
} // namespace gnut
