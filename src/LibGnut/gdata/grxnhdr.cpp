/* ----------------------------------------------------------------------
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  Purpose: rinexn header
  Version: $Rev:$
  2018-08-03 /JD: created
-*/
#include "grxnhdr.h"

namespace gnut
{

    t_rxnhdr::t_rxnhdr()
    {
    }

    t_rxnhdr::~t_rxnhdr()
    {
    }

    void t_rxnhdr::clear()
    {
        _gtime = t_gtime::current_time(t_gtime::GPS);

        _path = "";
        _rxnsys = 'G';
        _rxnver = 'x';
        _program = "";
        _runby = "";
        _leapsec = 0;

        _comment.clear();
    }

    string tsys_corr2str(TSYS_CORR c)
    {
        switch (c)
        {
        case TS_GAUT:
            return "GAUT";
        case TS_GPUT:
            return "GPUT";
        case TS_SBUT:
            return "SBUT";
        case TS_GLUT:
            return "GLUT";
        case TS_GPGA:
            return "GPGA";
        case TS_GLGP:
            return "GLGP";
        case TS_QZGP:
            return "QZGP";
        case TS_QZUT:
            return "QZUT";
        case TS_BDUT:
            return "BDUT";
        case TS_IRUT:
            return "IRUT";
        case TS_IRGP:
            return "IRGP";
        case TS_NONE:
            return "XXXX";
        }
        return "XXXX";
    }

    string iono_corr2str(IONO_CORR c)
    {
        switch (c)
        {
        case IO_GAL:
            return "GAL ";
        case IO_GPSA:
            return "GPSA";
        case IO_GPSB:
            return "GPSB";
        case IO_QZSA:
            return "QZSA";
        case IO_QZSB:
            return "QZSB";
        case IO_BDSA:
            return "BDSA";
        case IO_BDSB:
            return "BDSB";
        case IO_IRNA:
            return "IRNA";
        case IO_IRNB:
            return "IRNB";
        case IO_NONE:
            return "XXXX";
        }
        return "XXXX";
    }

    TSYS_CORR str2tsys_corr(string s)
    {
        if (s.compare("GAUT") == 0)
            return TS_GAUT;
        else if (s.compare("GPUT") == 0)
            return TS_GPUT;
        else if (s.compare("SBUT") == 0)
            return TS_SBUT;
        else if (s.compare("GLUT") == 0)
            return TS_GLUT;
        else if (s.compare("GPGA") == 0)
            return TS_GPGA;
        else if (s.compare("GLGP") == 0)
            return TS_GLGP;
        else if (s.compare("QZGP") == 0)
            return TS_QZGP;
        else if (s.compare("QZUT") == 0)
            return TS_QZUT;
        else if (s.compare("BDUT") == 0)
            return TS_BDUT;
        else if (s.compare("IRUT") == 0)
            return TS_IRUT;
        else if (s.compare("IRGP") == 0)
            return TS_IRGP;

        return TS_NONE;
    }

    IONO_CORR str2iono_corr(string s)
    {
        if (s.compare("GAL ") == 0)
            return IO_GAL;
        else if (s.compare("GPSA") == 0)
            return IO_GPSA;
        else if (s.compare("GPSB") == 0)
            return IO_GPSB;
        else if (s.compare("QZSA") == 0)
            return IO_QZSA;
        else if (s.compare("QZSB") == 0)
            return IO_QZSB;
        else if (s.compare("BDSA") == 0)
            return IO_BDSA;
        else if (s.compare("BDSB") == 0)
            return IO_BDSB;
        else if (s.compare("IRNA") == 0)
            return IO_IRNA;
        else if (s.compare("IRNB") == 0)
            return IO_IRNB;

        return IO_NONE;
    }

    set<TSYS_CORR> t_rxnhdr::tsys_corr() const
    {
        set<TSYS_CORR> set_ts;
        for (auto it = _ts_corr.begin(); it != _ts_corr.end(); ++it)
            set_ts.insert(it->first);

        return set_ts;
    }

    t_tsys_corr t_rxnhdr::tsys_corr(const TSYS_CORR &c) const
    {
        if (_ts_corr.find(c) != _ts_corr.end())
        {
            return _ts_corr.at(c);
        }
        t_tsys_corr ts;
        return ts;
    }

    void t_rxnhdr::tsys_corr(const TSYS_CORR &c, const t_tsys_corr &ts)
    {
        if (_ts_corr.find(c) == _ts_corr.end())
        {
            _ts_corr[c] = ts;
        }
    }

    set<IONO_CORR> t_rxnhdr::iono_corr() const
    {
        set<IONO_CORR> set_io;
        for (auto it = _io_corr.begin(); it != _io_corr.end(); ++it)
            set_io.insert(it->first);

        return set_io;
    }

    t_iono_corr t_rxnhdr::iono_corr(const IONO_CORR &c) const
    {
        if (_io_corr.find(c) != _io_corr.end())
        {
            return _io_corr.at(c);
        }
        t_iono_corr io;
        return io;
    }

    void t_rxnhdr::iono_corr(const IONO_CORR &c, const t_iono_corr &io)
    {
        if (_io_corr.find(c) == _io_corr.end())
        {
            _io_corr[c] = io;
        }
    }

    ostream &operator<<(ostream &os, const t_rxnhdr &x)
    {
        os << "program: " << x.program() << endl;
        os << "runby: " << x.runby() << endl;
        os << "gtime: " << x.gtime().str_hms() << endl;
        os << "leapsec: " << x.leapsec() << endl;
        return os;
    }

} // namespace
