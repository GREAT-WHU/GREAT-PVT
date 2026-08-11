
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "gutils/gsys.h"
#include "gutils/gcommon.h"


namespace gnut
{
    t_gsys::t_gsys(const GSYS& sys) :
        _gsys(sys)
    {
    }

    t_gsys::t_gsys(const std::string& sys) :
        _gsys(GSYS::GNS)
    {
        if (sys.size() > 0)
        {
            _gsys = str2gsys(sys);
        }
    }

    t_gsys::t_gsys(const char& c)
    {
        _gsys = char2gsys(c);
    }

    GOBSBAND t_gsys::gfrq2band(const GSYS& gs, const GFRQ& gfreq)
    {
        if (gs == GSYS::GNS || gfreq == gnut::GFRQ::LAST_GFRQ)
        {
            return gnut::GOBSBAND::BAND;
        }
        try
        {
            t_vec_freq sys_freq = GNSS_FREQ_PRIORITY.at(gs);
            unsigned int size = sys_freq.size();
            for (unsigned int i = 0; i < size; ++i)
            {
                if (sys_freq.at(i) == gfreq)
                {
                    return band_priority(gs, FREQ_SEQ(i));
                }
            }
            return gnut::GOBSBAND::BAND;
        }
        catch (...)
        {
            return gnut::GOBSBAND::BAND;
        }
    }

    GFRQ t_gsys::band2gfrq(const GSYS& gs, const GOBSBAND& gband)
    {
        if (gs == GSYS::GNS || gband == gnut::GOBSBAND::BAND)
        {
            return gnut::GFRQ::LAST_GFRQ;
        }
        try
        {
            t_vec_band sys_band = GNSS_BAND_PRIORITY.at(gs);
            unsigned int size = sys_band.size();
            for (unsigned int i = 0; i < size; ++i)
            {
                if (sys_band.at(i) == gband)
                {
                    return freq_priority(gs, FREQ_SEQ(i));
                }
            }
            return gnut::GFRQ::LAST_GFRQ;
        }
        catch (...)
        {
            return gnut::GFRQ::LAST_GFRQ;
        }
    }

    FREQ_SEQ t_gsys::gfrq2freq(const GSYS& gs, const GFRQ& gfreq)
    {
        if (gs == GSYS::GNS || gfreq == gnut::GFRQ::LAST_GFRQ)
        {
            return gnut::FREQ_SEQ::FREQ_X;
        }
        try
        {
            t_vec_freq sys_freq = GNSS_FREQ_PRIORITY.at(gs);
            unsigned int size = sys_freq.size();
            for (unsigned int i = 0; i < size; ++i)
            {
                if (sys_freq.at(i) == gfreq)
                {
                    return FREQ_SEQ(i);
                }
            }
            return gnut::FREQ_SEQ::FREQ_X;
        }
        catch (...)
        {
            return gnut::FREQ_SEQ::FREQ_X;
        }
    }

    GSYS t_gsys::gfrq2gsys(const GFRQ& freq)
    {
        std::string s = t_gfreq::gfreq2str(freq);
        return t_gsys::char2gsys(s[0]);
    }

    FREQ_SEQ t_gsys::str2freq(const std::string& freq)
    {
        if (freq.empty())
        {
            return FREQ_SEQ::FREQ_X;
        }

        char cfreq = freq[0];
        switch (cfreq)
        {
            case '0':
                return FREQ_SEQ::FREQ_X;
            case '1':
                return FREQ_SEQ::FREQ_1;
            case '2':
                return FREQ_SEQ::FREQ_2;
            case '3':
                return FREQ_SEQ::FREQ_3;
            case '4':
                return FREQ_SEQ::FREQ_4;
            case '5':
                return FREQ_SEQ::FREQ_5;
            case '6':
                return FREQ_SEQ::FREQ_6;
            case '7':
                return FREQ_SEQ::FREQ_7;
            default:
                return FREQ_SEQ::FREQ_X;
        }
    }

    std::string t_gsys::freq2str(const FREQ_SEQ& freq)
    {
        switch (freq)
        {
            case FREQ_SEQ::FREQ_X:
                return std::string("0");
            case FREQ_SEQ::FREQ_1:
                return std::string("1");
            case FREQ_SEQ::FREQ_2:
                return std::string("2");
            case FREQ_SEQ::FREQ_3:
                return std::string("3");
            case FREQ_SEQ::FREQ_4:
                return std::string("4");
            case FREQ_SEQ::FREQ_5:
                return std::string("5");
            case FREQ_SEQ::FREQ_6:
                return std::string("6");
            case FREQ_SEQ::FREQ_7:
                return std::string("7");

            default:
                return std::string("0");
        }
    }

    FREQ_SEQ t_gsys::band2freq(const GSYS& gs, const GOBSBAND& gband)
    {
        if (gs == GSYS::GNS || gband == gnut::GOBSBAND::BAND)
        {
            return gnut::FREQ_SEQ::FREQ_X;
        }
        try
        {
            t_vec_band sys_band = GNSS_BAND_PRIORITY.at(gs);
            unsigned int size = sys_band.size();
            for (unsigned int i = 0; i < size; ++i)
            {
                if (sys_band.at(i) == gband)
                {
                    return FREQ_SEQ(i);
                }
            }
            return gnut::FREQ_SEQ::FREQ_X;
        }
        catch (...)
        {
            return gnut::FREQ_SEQ::FREQ_X;
        }
    }

    GOBSBAND t_gsys::band_priority(const GSYS& gs,
                                   const FREQ_SEQ& iseq) // FREQ_SEQ priority band
    {
        try
        {
            const std::size_t index = static_cast<std::size_t>(iseq);
            if (gs == GSYS::GNS || index >= GNSS_BAND_PRIORITY.at(gs).size())
            {
                return gnut::GOBSBAND::BAND;
            }
            return GNSS_BAND_PRIORITY.at(gs).at(index);
        }
        catch (...)
        {
            return gnut::GOBSBAND::BAND;
        }
    }

    GOBS t_gsys::gobs_priority(const GSYS& gs, const GOBS& gobs)
    {
        if (gs == GSYS::GPS)
        {
            switch (gobs)
            {
                case gnut::GOBS::C1C:
                case gnut::GOBS::C1P:
                case gnut::GOBS::C1W:
                {
                    return gnut::GOBS::C1W;
                }
                case gnut::GOBS::C2C:
                case gnut::GOBS::C2L:
                case gnut::GOBS::C2X:
                case gnut::GOBS::C2P:
                case gnut::GOBS::C2W:
                {
                    return gnut::GOBS::C2W;
                }
                case gnut::GOBS::C5Q:
                case gnut::GOBS::C5X:
                {
                    return gnut::GOBS::C5X;
                }
                default:
                    return GOBS::X;
            }
        }
        else if (gs == GSYS::GLO)
        {
            switch (gobs)
            {
                case gnut::GOBS::C1C:
                case gnut::GOBS::C1P:
                    return gnut::GOBS::C1P;
                case gnut::GOBS::C2C:
                case gnut::GOBS::C2P:
                    return gnut::GOBS::C2P;
                default:
                    return GOBS::X;
            }
        }
        else
        {
            return gobs;
        }
    }

    GOBS t_gsys::gobs_priority(const GSYS& gs, const GOBSBAND& band)
    {
        if (gs == GSYS::GPS)
        {
            switch (band)
            {
                case gnut::GOBSBAND::BAND_1:
                {
                    return gnut::GOBS::C1W;
                }
                case gnut::GOBSBAND::BAND_2:
                {
                    return gnut::GOBS::C2W;
                }
                case gnut::GOBSBAND::BAND_5:
                {
                    return gnut::GOBS::C5X;
                }
                default:
                    return GOBS::X;
            }
        }
        else if (gs == GSYS::GLO)
        {
            switch (band)
            {
                case gnut::GOBSBAND::BAND_1:
                    return gnut::GOBS::C1P;
                case gnut::GOBSBAND::BAND_2:
                    return gnut::GOBS::C2P;
                default:
                    return GOBS::X;
            }
        }
        else if (gs == GSYS::GAL)
        {
            switch (band)
            {
                case gnut::GOBSBAND::BAND_1:
                    return gnut::GOBS::C1X;
                case gnut::GOBSBAND::BAND_5:
                    return gnut::GOBS::C5X;
                case gnut::GOBSBAND::BAND_7:
                    return gnut::GOBS::C7X;
                default:
                    return GOBS::X;
            }
        }
        else
        {
            return GOBS::X;
        }
    }

    GOBS t_gsys::gobs_defaults(const GSYS& gs, const GOBS& gobs1, const GOBSBAND& band)
    {
        if (gs == GSYS::GPS)
        {
            switch (band)
            {
                case gnut::GOBSBAND::BAND_1:
                {
                    return gnut::GOBS::C1W;
                }
                case gnut::GOBSBAND::BAND_2:
                {
                    return gnut::GOBS::C2W;
                }
                case gnut::GOBSBAND::BAND_5:
                {
                    return gnut::GOBS::C5X;
                }
                default:
                    return GOBS::X;
            }
        }
        else if (gs == GSYS::GLO)
        {
            switch (band)
            {
                case gnut::GOBSBAND::BAND_1:
                    return gnut::GOBS::C1P;
                case gnut::GOBSBAND::BAND_2:
                    return gnut::GOBS::C2P;
                default:
                    return GOBS::X;
            }
        }
        else if (gs == GSYS::GAL)
        {
            std::string strGOBS = gobs2str(gobs1);
            switch (band)
            {
                case gnut::GOBSBAND::BAND_1:
                {
                    if (strGOBS[strGOBS.size() - 1] == 'X')
                    {
                        return gnut::GOBS::C1X;
                    }
                    else
                    {
                        return gnut::GOBS::C1C;
                    }
                }
                case gnut::GOBSBAND::BAND_5:
                {
                    if (strGOBS[strGOBS.size() - 1] == 'X')
                    {
                        return gnut::GOBS::C5X;
                    }
                    else
                    {
                        return gnut::GOBS::C5Q;
                    }
                }
                case gnut::GOBSBAND::BAND_7:
                {
                    if (strGOBS[strGOBS.size() - 1] == 'X')
                    {
                        return gnut::GOBS::C7X;
                    }
                    else
                    {
                        return gnut::GOBS::C7Q;
                    }
                }
                default:
                    return GOBS::X;
            }
        }
        else
        {
            return GOBS::X;
        }
    }

    GFRQ t_gsys::freq_priority(const GSYS& gs,
                               const FREQ_SEQ& iseq) // FREQ_SEQ priority frequency
    {
        try
        {
            const std::size_t index = static_cast<std::size_t>(iseq);
            if (gs == GSYS::GNS || index >= GNSS_FREQ_PRIORITY.at(gs).size())
            {
                return gnut::GFRQ::LAST_GFRQ;
            }

            return GNSS_FREQ_PRIORITY.at(gs).at(index);
        }
        catch (...)
        {
            return gnut::GFRQ::LAST_GFRQ;
        }
    }

    GOBSATTR t_gsys::attr_priority(const GSYS& gs,
                                   const GOBSBAND& gb,
                                   const GOBSTYPE& gt,
                                   const unsigned int& iseq) // iseq priority sequence
    {
        t_map_gnss m = GNSS_DATA_PRIORITY();

        if (gs == GSYS::GNS || gt == gnut::GOBSTYPE::TYPE || gb == gnut::GOBSBAND::BAND || iseq > m[gs][gb][gt].size())
        {
            return gnut::GOBSATTR::ATTR;
        }

        return m[gs][gb][gt][iseq];
    }

    std::string t_gsys::gsys2str(const GSYS& sys)
    {
        switch (sys)
        {
            case GSYS::GPS:
                return "GPS";
            case GSYS::GLO:
                return "GLO";
            case GSYS::GAL:
                return "GAL";
            case GSYS::BDS:
                return "BDS";
            case GSYS::SBS:
                return "SBS";
            case GSYS::QZS:
                return "QZS";
            case GSYS::IRN:
                return "IRN";
            case GSYS::GNS:
                return "GNS";

            default:
            {
                std::cout << "*** warning: unknown GNSS system!\n";
                std::cout.flush();
            }
        }

        return "GNS";
    }

    char t_gsys::gsys2char(const GSYS& sys)
    {
        switch (sys)
        {
            case GSYS::GPS:
                return 'G';
            case GSYS::GLO:
                return 'R';
            case GSYS::GAL:
                return 'E';
            case GSYS::BDS:
                return 'C';
            case GSYS::SBS:
                return 'S';
            case GSYS::QZS:
                return 'J';
            case GSYS::IRN:
                return 'I';
            case GSYS::GNS:
                return 'X';

            default:
            {
                std::cout << "*** warning: unknown GNSS system \n";
                std::cout.flush();
            }
        }

        return 'X';
    }

    std::string t_gsys::str2strfirst(const std::string& tmp)
    {
        std::string s(tmp);
        if (s.size() == 0)
        {
            std::cout << "*** warning: not defined GNSS system code [NULL]\n";
            std::cout.flush();
            return "X";
        }

        transform(s.begin(), s.end(), s.begin(), ::toupper);

        if (s == "G" || s == "GPS" || s == "NAVSTAR")
        {
            return "G";
        }
        else if (s == "R" || s == "GLO" || s == "GLONASS")
        {
            return "R";
        }
        else if (s == "E" || s == "GAL" || s == "GALILEO")
        {
            return "E";
        }
        else if (s == "C" || s == "COMP" || s == "COMPASS")
        {
            return "C";
        }
        else if (s == "C" || s == "BDS" || s == "BEIDOU")
        {
            return "C";
        }
        else if (s == "S" || s == "SBS" || s == "EGNOS")
        {
            return "S";
        }
        else if (s == "S" || s == "SBAS")
        {
            return "S";
        }
        else if (s == "J" || s == "QZS" || s == "JAXA")
        {
            return "J";
        }
        else if (s == "J" || s == "QZSS")
        {
            return "J";
        }
        else if (s == "I" || s == "IRN" || s == "IRNSS")
        {
            return "I";
        }
        else
        {
            std::cout << "*** warning: not defined GNSS system code [" << s[0] << "]\n";
            std::cout.flush();
        }

        return "X";
    }

    GSYS t_gsys::str2gsys(const std::string& tmp)
    {
        std::string s(tmp);
        if (s.size() == 0)
        {
            std::cout << "*** warning: not defined GNSS system code [NULL]\n";
            std::cout.flush();
            return GSYS::GNS;
        }

        transform(s.begin(), s.end(), s.begin(), ::toupper);

        if (s == "G" || s == "GPS" || s == "NAVSTAR")
        {
            return GSYS::GPS;
        }
        else if (s.substr(0, 1) == "2" || s.substr(0, 1) == "3" || s.substr(0, 1) == "4" || s.substr(0, 1) == "5")
        {
            return GSYS::GPS;
        }
        else if (s == "R" || s == "GLO" || s == "GLONASS")
        {
            return GSYS::GLO;
        }
        else if (s == "E" || s == "GAL" || s == "GALILEO")
        {
            return GSYS::GAL;
        }
        else if (s == "C" || s == "COMP" || s == "COMPASS")
        {
            return GSYS::BDS;
        }
        else if (s == "C" || s == "BDS" || s == "BEIDOU")
        {
            return GSYS::BDS;
        }
        else if (s == "S" || s == "SBS" || s == "EGNOS")
        {
            return GSYS::SBS;
        }
        else if (s == "S" || s == "SBAS")
        {
            return GSYS::SBS;
        }
        else if (s == "J" || s == "QZS" || s == "JAXA")
        {
            return GSYS::QZS;
        }
        else if (s == "J" || s == "QZSS")
        {
            return GSYS::QZS;
        }
        else if (s == "I" || s == "IRN" || s == "IRNSS")
        {
            return GSYS::IRN;
        }
        else if (s == "X" || s == "GNS" || s == "GNSS")
        {
            return GSYS::GNS;
        }
        else if (s == "M")
        {
            return GSYS::GNS;
        }
        else
        {
            std::cout << "*** warning: not defined GNSS system code [" << s[0] << "]\n";
            std::cout.flush();
        }

        return GSYS::GNS;
    }

    GSYS t_gsys::sat2gsys(const std::string& s)
    {
        std::string tmp = s.substr(0, 1);
        if (s.empty())
        {
            return GSYS::GNS;
        }
        else
        {
            return str2gsys(tmp);
        }
    }

    char t_gsys::str2char(const std::string& s)
    {
        if (s.size() > 0)
        {
            return gsys2char(str2gsys(s));
        }
        return 'X';
    }

    GSYS t_gsys::char2gsys(char c)
    {
        if (c == 'G')
        {
            return GSYS::GPS;
        }
        else if (c == 'R')
        {
            return GSYS::GLO;
        }
        else if (c == 'E')
        {
            return GSYS::GAL;
        }
        else if (c == 'C')
        {
            return GSYS::BDS;
        }
        else if (c == 'S')
        {
            return GSYS::SBS;
        }
        else if (c == 'J')
        {
            return GSYS::QZS;
        }
        else if (c == 'I')
        {
            return GSYS::IRN;
        }
        else if (c == 'M')
        {
            return GSYS::GNS;
        }
        else if (c == 'X')
        {
            return GSYS::GNS;
        }
        else
        {
            std::cout << "*** warning: not defined GNSS system char [" << c << "]\n";
            std::cout.flush();
        }

        return GSYS::GNS;
    }

    std::string t_gsys::char2str(char c)
    {
        return gsys2str(char2gsys(c));
    }

    std::string t_gsys::eval_sat(const std::string& sat, const GSYS& sys)
    {
        std::istringstream is(sat); // .substr(0,3) NE!
        GSYS gnss = sys;
        int svn = 0;
        char chr = 'G'; // if empty, use GPS
        std::size_t l = is.str().length();

        if (l == 0)
        {
            return "X00";
        }

        if (l < 3 || sat[0] == ' ')
        {
            int ret = sscanf(sat.c_str(), "%d", &svn);
        }
        else
        {
            int ret = sscanf(sat.c_str(), "%c%d", &chr, &svn);
        }

        if (is.fail())
        {
            return "X00";
        }
        if (chr != 'G')
        {
            gnss = char2gsys(chr);
        }

        switch (gnss)
        {
            case GSYS::GPS:
                chr = 'G';
                break;
            case GSYS::GLO:
                chr = 'R';
                break;
            case GSYS::GAL:
                chr = 'E';
                break;
            case GSYS::BDS:
                chr = 'C';
                break;
            case GSYS::SBS:
                chr = 'S';
                break;
            case GSYS::QZS:
                chr = 'J';
                break;
            case GSYS::IRN:
                chr = 'I';
                break;
            case GSYS::GNS:
                chr = 'X';
                break;
            default:
                chr = 'X';
                break;
        }

        if (svn > QZS_OFFSET)
        {
            svn -= QZS_OFFSET;
        }
        if (svn > SBS_OFFSET)
        {
            svn -= SBS_OFFSET;
        }

        char tmp[4];
        sprintf(tmp, "%c%02d", chr, svn);

        std::string tmp_svn(tmp);
        return tmp_svn;
    }

    std::string t_gsys::eval_sat(const int& svn, const GSYS& sys)
    {
        char chr;
        switch (sys)
        {
            case GSYS::GPS:
                chr = 'G';
                break;
            case GSYS::GLO:
                chr = 'R';
                break;
            case GSYS::GAL:
                chr = 'E';
                break;
            case GSYS::BDS:
                chr = 'C';
                break;
            case GSYS::SBS:
                chr = 'S';
                break;
            case GSYS::QZS:
                chr = 'J';
                break;
            case GSYS::IRN:
                chr = 'I';
                break;
            case GSYS::GNS:
                chr = 'X';
                break;
            default:
                chr = 'X';
                break;
        }

        int num = svn;
        if (svn > QZS_OFFSET)
        {
            num -= QZS_OFFSET;
        }
        if (svn > SBS_OFFSET)
        {
            num -= SBS_OFFSET;
        }

        std::ostringstream os;
        os << std::setw(1) << chr << std::setfill('0') << std::setw(2) << num;

        return os.str();
    }

    void t_gsys::from_string(const std::string& sys)
    {
        _gsys = str2gsys(sys);
    }

    void t_gsys::from_gsys(const GSYS& sys)
    {
        _gsys = sys;
    }

    bool t_gsys::operator==(const std::string& sys) const
    {
        if (_gsys == str2gsys(sys))
        {
            return true;
        }
        return false;
    }

    bool t_gsys::operator==(const GSYS& sys) const
    {
        return false;
    }

    GFRQ t_gfreq::str2gfreq(const std::string& freq_const)
    {
        std::string freq(freq_const);
        transform(freq.begin(), freq.end(), freq.begin(), ::toupper);

        if (freq == "G01")
        {
            return GFRQ::G01;
        }
        else if (freq == "G02")
        {
            return GFRQ::G02;
        }
        else if (freq == "G05")
        {
            return GFRQ::G05;
        }
        else if (freq == "R01")
        {
            return GFRQ::R01;
        }
        else if (freq == "R02")
        {
            return GFRQ::R02;
        }
        else if (freq == "R03")
        {
            return gnut::GFRQ::R03_CDMA;
        }
        else if (freq == "R05")
        {
            return gnut::GFRQ::R05_CDMA;
        }
        else if (freq == "E01")
        {
            return GFRQ::E01;
        }
        else if (freq == "E05")
        {
            return GFRQ::E05;
        }
        else if (freq == "E07")
        {
            return GFRQ::E07;
        }
        else if (freq == "E08")
        {
            return GFRQ::E08;
        }
        else if (freq == "E06")
        {
            return GFRQ::E06;
        }
        else if (freq == "C02")
        {
            return GFRQ::C02;
        }
        else if (freq == "C07")
        {
            return GFRQ::C07;
        }
        else if (freq == "C06")
        {
            return GFRQ::C06;
        }
        else if (freq == "C01")
        {
            return GFRQ::C01;
        }
        else if (freq == "C08")
        {
            return GFRQ::C08;
        }
        else if (freq == "C05")
        {
            return GFRQ::C05;
        }
        else if (freq == "C09")
        {
            return GFRQ::C09;
        }
        else if (freq == "J01")
        {
            return GFRQ::J01;
        }
        else if (freq == "J02")
        {
            return GFRQ::J02;
        }
        else if (freq == "J05")
        {
            return GFRQ::J05;
        }
        else if (freq == "J06")
        {
            return GFRQ::J06;
        }
        else if (freq == "S01")
        {
            return gnut::GFRQ::S01;
        }
        else if (freq == "S05")
        {
            return gnut::GFRQ::S05;
        }
        else if (freq == "I05")
        {
            return GFRQ::I05;
        }
        else if (freq == "LAST_GFRQ")
        {
            return gnut::GFRQ::LAST_GFRQ;
        }

        return gnut::GFRQ::LAST_GFRQ;
    }

    std::string t_gfreq::gfreq2str(const GFRQ& freq)
    {
        switch (freq)
        {
            case GFRQ::G01:
                return "G01";
            case GFRQ::G02:
                return "G02";
            case GFRQ::G05:
                return "G05";
            case GFRQ::R01:
                return "R01";
            case GFRQ::R02:
                return "R02";
            case gnut::GFRQ::R01_CDMA:
                return "R01";
            case gnut::GFRQ::R02_CDMA:
                return "R02";
            case gnut::GFRQ::R03_CDMA:
                return "R03";
            case gnut::GFRQ::R05_CDMA:
                return "R05";
            case GFRQ::E01:
                return "E01";
            case GFRQ::E05:
                return "E05";
            case GFRQ::E07:
                return "E07";
            case GFRQ::E08:
                return "E08";
            case GFRQ::E06:
                return "E06";
            case GFRQ::C02:
                return "C02";
            case GFRQ::C06:
                return "C06";
            case GFRQ::C07:
                return "C07";
            case GFRQ::C01:
                return "C01";
            case GFRQ::C08:
                return "C08";
            case GFRQ::C05:
                return "C05";
            case GFRQ::C09:
                return "C09";
            case GFRQ::J01:
                return "J01";
            case GFRQ::J02:
                return "J02";
            case GFRQ::J05:
                return "J05";
            case GFRQ::J06:
                return "J06";
            case gnut::GFRQ::S01:
                return "S01";
            case gnut::GFRQ::S05:
                return "S05";
            case GFRQ::I05:
                return "I05";
            case gnut::GFRQ::LAST_GFRQ:
                return "LAST_GFRQ";

            default:
            {
                std::cout << "*** warning: unknown frequency code \n";
                std::cout.flush();
            }
        }

        return "LAST_GFRQ";
    }

    bool t_gsys::bds_geo(GSAT sat)
    {
        return sat == GSAT::C01 || sat == GSAT::C02 || sat == GSAT::C03 || sat == GSAT::C04 || sat == GSAT::C05 ||
               sat == GSAT::C59 || sat == GSAT::C60;
    }

    bool t_gsys::bds_geo(const std::string& sat)
    {
        return bds_geo(str2gsat(sat, GSYS::BDS));
    }

    bool t_gsys::bds_cast(GSAT sat)
    {
        switch (sat)
        {
            case GSAT::C06: case GSAT::C13: case GSAT::C14: case GSAT::C16:
            case GSAT::C19: case GSAT::C20: case GSAT::C21: case GSAT::C22:
            case GSAT::C23: case GSAT::C24: case GSAT::C32: case GSAT::C33:
            case GSAT::C36: case GSAT::C37: case GSAT::C38: case GSAT::C39:
            case GSAT::C40: case GSAT::C41: case GSAT::C42: case GSAT::C45:
            case GSAT::C46: return true;
            default: return false;
        }
    }

    bool t_gsys::bds_cast(const std::string& sat)
    {
        return bds_cast(str2gsat(sat, GSYS::BDS));
    }

    bool t_gsys::bds_secm(GSAT sat)
    {
        switch (sat)
        {
            case GSAT::C25: case GSAT::C26: case GSAT::C27: case GSAT::C28: case GSAT::C29:
            case GSAT::C30: case GSAT::C34: case GSAT::C35: case GSAT::C43: case GSAT::C44: return true;
            default: return false;
        }
    }

    bool t_gsys::bds_secm(const std::string& sat)
    {
        return bds_secm(str2gsat(sat, GSYS::BDS));
    }
} // namespace gnut
