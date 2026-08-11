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
        _gtime = t_gtime::current_time(t_gtime::t_tsys::GPS);

        _path = "";
        _rxnsys = 'G';
        _rxnver = 'x';
        _program = "";
        _runby = "";
        _leapsec = 0;

        _comment.clear();
    }

    std::string tsys_corr2str(TSYS_CORR c)
    {
        switch (c)
        {
            case TSYS_CORR::TS_GAUT:
                return "GAUT";
            case TSYS_CORR::TS_GPUT:
                return "GPUT";
            case TSYS_CORR::TS_SBUT:
                return "SBUT";
            case TSYS_CORR::TS_GLUT:
                return "GLUT";
            case TSYS_CORR::TS_GPGA:
                return "GPGA";
            case TSYS_CORR::TS_GLGP:
                return "GLGP";
            case TSYS_CORR::TS_QZGP:
                return "QZGP";
            case TSYS_CORR::TS_QZUT:
                return "QZUT";
            case TSYS_CORR::TS_BDUT:
                return "BDUT";
            case TSYS_CORR::TS_IRUT:
                return "IRUT";
            case TSYS_CORR::TS_IRGP:
                return "IRGP";
            case TSYS_CORR::TS_NONE:
                return "XXXX";
        }
        return "XXXX";
    }

    std::string iono_corr2str(IONO_CORR c)
    {
        switch (c)
        {
            case IONO_CORR::IO_GAL:
                return "GAL ";
            case IONO_CORR::IO_GPSA:
                return "GPSA";
            case IONO_CORR::IO_GPSB:
                return "GPSB";
            case IONO_CORR::IO_QZSA:
                return "QZSA";
            case IONO_CORR::IO_QZSB:
                return "QZSB";
            case IONO_CORR::IO_BDSA:
                return "BDSA";
            case IONO_CORR::IO_BDSB:
                return "BDSB";
            case IONO_CORR::IO_IRNA:
                return "IRNA";
            case IONO_CORR::IO_IRNB:
                return "IRNB";
            case IONO_CORR::IO_NONE:
                return "XXXX";
        }
        return "XXXX";
    }

    TSYS_CORR str2tsys_corr(std::string s)
    {
        if (s.compare("GAUT") == 0)
        {
            return TSYS_CORR::TS_GAUT;
        }
        else if (s.compare("GPUT") == 0)
        {
            return TSYS_CORR::TS_GPUT;
        }
        else if (s.compare("SBUT") == 0)
        {
            return TSYS_CORR::TS_SBUT;
        }
        else if (s.compare("GLUT") == 0)
        {
            return TSYS_CORR::TS_GLUT;
        }
        else if (s.compare("GPGA") == 0)
        {
            return TSYS_CORR::TS_GPGA;
        }
        else if (s.compare("GLGP") == 0)
        {
            return TSYS_CORR::TS_GLGP;
        }
        else if (s.compare("QZGP") == 0)
        {
            return TSYS_CORR::TS_QZGP;
        }
        else if (s.compare("QZUT") == 0)
        {
            return TSYS_CORR::TS_QZUT;
        }
        else if (s.compare("BDUT") == 0)
        {
            return TSYS_CORR::TS_BDUT;
        }
        else if (s.compare("IRUT") == 0)
        {
            return TSYS_CORR::TS_IRUT;
        }
        else if (s.compare("IRGP") == 0)
        {
            return TSYS_CORR::TS_IRGP;
        }

        return TSYS_CORR::TS_NONE;
    }

    IONO_CORR str2iono_corr(std::string s)
    {
        if (s.compare("GAL ") == 0)
        {
            return IONO_CORR::IO_GAL;
        }
        else if (s.compare("GPSA") == 0)
        {
            return IONO_CORR::IO_GPSA;
        }
        else if (s.compare("GPSB") == 0)
        {
            return IONO_CORR::IO_GPSB;
        }
        else if (s.compare("QZSA") == 0)
        {
            return IONO_CORR::IO_QZSA;
        }
        else if (s.compare("QZSB") == 0)
        {
            return IONO_CORR::IO_QZSB;
        }
        else if (s.compare("BDSA") == 0)
        {
            return IONO_CORR::IO_BDSA;
        }
        else if (s.compare("BDSB") == 0)
        {
            return IONO_CORR::IO_BDSB;
        }
        else if (s.compare("IRNA") == 0)
        {
            return IONO_CORR::IO_IRNA;
        }
        else if (s.compare("IRNB") == 0)
        {
            return IONO_CORR::IO_IRNB;
        }

        return IONO_CORR::IO_NONE;
    }

    std::set<TSYS_CORR> t_rxnhdr::tsys_corr() const
    {
        std::set<TSYS_CORR> set_ts;
        for (auto it = _ts_corr.begin(); it != _ts_corr.end(); ++it)
        {
            set_ts.insert(it->first);
        }

        return set_ts;
    }

    t_tsys_corr t_rxnhdr::tsys_corr(const TSYS_CORR& c) const
    {
        if (_ts_corr.find(c) != _ts_corr.end())
        {
            return _ts_corr.at(c);
        }
        t_tsys_corr ts;
        return ts;
    }

    void t_rxnhdr::tsys_corr(const TSYS_CORR& c, const t_tsys_corr& ts)
    {
        if (_ts_corr.find(c) == _ts_corr.end())
        {
            _ts_corr[c] = ts;
        }
    }

    std::set<IONO_CORR> t_rxnhdr::iono_corr() const
    {
        std::set<IONO_CORR> set_io;
        for (auto it = _io_corr.begin(); it != _io_corr.end(); ++it)
        {
            set_io.insert(it->first);
        }

        return set_io;
    }

    t_iono_corr t_rxnhdr::iono_corr(const IONO_CORR& c) const
    {
        if (_io_corr.find(c) != _io_corr.end())
        {
            return _io_corr.at(c);
        }
        t_iono_corr io;
        return io;
    }

    void t_rxnhdr::iono_corr(const IONO_CORR& c, const t_iono_corr& io)
    {
        if (_io_corr.find(c) == _io_corr.end())
        {
            _io_corr[c] = io;
        }
    }

    std::ostream& operator<<(std::ostream& os, const t_rxnhdr& x)
    {
        os << "program: " << x.program() << std::endl;
        os << "runby: " << x.runby() << std::endl;
        os << "gtime: " << x.gtime().str_hms() << std::endl;
        os << "leapsec: " << x.leapsec() << std::endl;
        return os;
    }

} // namespace gnut
