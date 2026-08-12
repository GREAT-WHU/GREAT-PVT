
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <cctype>

#include "gutils/gnss.h"
#include "gutils/gtypeconv.h"

// using namespace std;

namespace gnut
{
    namespace
    {
        int gsat_block(GSYS sys)
        {
            switch (sys)
            {
                case GSYS::BDS: return 1; // C
                case GSYS::GAL: return 2; // E
                case GSYS::GPS: return 3; // G
                case GSYS::IRN: return 4; // I
                case GSYS::QZS: return 5; // J
                case GSYS::GLO: return 6; // R
                case GSYS::SBS: return 7; // S
                default: return 0;
            }
        }

        GSYS gsat_system(int block)
        {
            switch (block)
            {
                case 1: return GSYS::BDS; // C
                case 2: return GSYS::GAL; // E
                case 3: return GSYS::GPS; // G
                case 4: return GSYS::IRN; // I
                case 5: return GSYS::QZS; // J
                case 6: return GSYS::GLO; // R
                case 7: return GSYS::SBS; // S
                default: return GSYS::GNS;
            }
        }
    }

    GSAT str2gsat(const std::string& sat, GSYS sys)
    {
        if (sat.empty())
        {
            return GSAT::NONE;
        }

        const char* first = sat.c_str();
        while (isspace(static_cast<unsigned char>(*first)))
        {
            ++first;
        }

        if (*first == '\0')
        {
            return GSAT::NONE;
        }

        if (isalpha(static_cast<unsigned char>(*first)))
        {
            switch (toupper(static_cast<unsigned char>(*first++)))
            {
                case 'G':
                    sys = GSYS::GPS;
                    break;
                case 'E':
                    sys = GSYS::GAL;
                    break;
                case 'R':
                    sys = GSYS::GLO;
                    break;
                case 'C':
                    sys = GSYS::BDS;
                    break;
                case 'J':
                    sys = GSYS::QZS;
                    break;
                case 'S':
                    sys = GSYS::SBS;
                    break;
                case 'I':
                    sys = GSYS::IRN;
                    break;
                default:
                    return GSAT::NONE;
            }
        }

        char* end = nullptr;
        long prn = strtol(first, &end, 10);
        while (end && isspace(static_cast<unsigned char>(*end)))
        {
            ++end;
        }
        if (end == first || (end && *end != '\0'))
        {
            return GSAT::NONE;
        }

        if (sys == GSYS::QZS && prn > 192) // RINEX QZSS offset
        {
            prn -= 192;
        }
        if (sys == GSYS::SBS && prn > 100) // RINEX SBAS offset
        {
            prn -= 100;
        }
        int block = gsat_block(sys);
        if (prn < 1 || prn > 99 || block == 0)
        {
            return GSAT::NONE;
        }

        GSAT result = static_cast<GSAT>(block * 100 + prn);
        return valid_gsat(result) ? result : GSAT::NONE;
    }

    bool valid_gsat(GSAT sat)
    {
        int value = static_cast<unsigned short>(sat);
        int prn = value % 100;
        switch (gsat_system(value / 100))
        {
            case GSYS::GPS:
                return prn >= 1 && prn <= 32;
            case GSYS::GAL:
                return prn >= 1 && prn <= 36;
            case GSYS::GLO:
                return prn >= 1 && prn <= 24;
            case GSYS::BDS:
                return prn >= 1 && prn <= 63;
            case GSYS::QZS:
                return prn >= 1 && prn <= 10;
            case GSYS::SBS:
                return (prn >= 20 && prn <= 58) || prn == 83;
            case GSYS::IRN:
                return prn >= 1 && prn <= 14;
            default:
                return false;
        }
    }

    GSYS gsat2gsys(GSAT sat)
    {
        int block = static_cast<unsigned short>(sat) / 100;
        return valid_gsat(sat) ? gsat_system(block) : GSYS::GNS;
    }

    int gsat2prn(GSAT sat)
    {
        return gsat2gsys(sat) == GSYS::GNS ? 0 : static_cast<unsigned short>(sat) % 100;
    }

    std::string gsat2str(GSAT sat)
    {
        char system = 'X';
        switch (gsat2gsys(sat))
        {
            case GSYS::GPS:
                system = 'G';
                break;
            case GSYS::GAL:
                system = 'E';
                break;
            case GSYS::GLO:
                system = 'R';
                break;
            case GSYS::BDS:
                system = 'C';
                break;
            case GSYS::QZS:
                system = 'J';
                break;
            case GSYS::SBS:
                system = 'S';
                break;
            case GSYS::IRN:
                system = 'I';
                break;
            default:
                return "X00";
        }

        std::ostringstream out;
        out << system << std::setfill('0') << std::setw(2) << gsat2prn(sat);
        return out.str();
    }

    std::ostream& operator<<(std::ostream& os, GSAT sat)
    {
        return os << gsat2str(sat);
    }

    std::string operator+(const std::string& text, GSAT sat)
    {
        return text + gsat2str(sat);
    }

    std::string operator+(GSAT sat, const std::string& text)
    {
        return gsat2str(sat) + text;
    }

    t_map_sats GNSS_SATS()
    {
        t_map_sats m;
        const std::pair<GSYS, int> ranges[] =
            {{GSYS::GPS, 32}, {GSYS::GLO, 24}, {GSYS::GAL, 36}, {GSYS::BDS, 46}, {GSYS::QZS, 7}, {GSYS::IRN, 14}};
        for (const auto& range : ranges)
        {
            for (int prn = 1; prn <= range.second; ++prn)
            {
                m[range.first].insert(str2gsat(std::to_string(prn), range.first));
            }
        }
        m[GSYS::BDS].insert(str2gsat("C59"));
        m[GSYS::BDS].insert(str2gsat("C60"));

        const int sbas_prns[] = {20, 24, 25, 26, 27, 28, 29, 33, 35, 36, 37, 38, 40, 83};
        for (int prn : sbas_prns)
        {
            m[GSYS::SBS].insert(str2gsat(std::to_string(prn), GSYS::SBS));
        }

        return m;
    }

    t_map_gnav GNSS_GNAV()
    {
        t_map_gnav m;
        // (string) necessary for VS2013
        m[GSYS::GPS] = {(std::string) "NAV", (std::string) "CNAV"};
        m[GSYS::GLO] = {(std::string) "NAV"};
        m[GSYS::GAL] = {(std::string) "NAV",
                        (std::string) "INAV_E01",
                        (std::string) "INAV_E01",
                        (std::string) "INAV",
                        (std::string) "FNAV"};
        m[GSYS::BDS] = {(std::string) "NAV"};
        m[GSYS::SBS] = {(std::string) "NAV"};
        m[GSYS::QZS] = {(std::string) "NAV"};

        return m;
    }

    t_map_band GNSS_BAND_SORTED()
    {
        t_map_band m;
        // order is important!
        m[GSYS::GPS] = {gnut::GOBSBAND::BAND, gnut::GOBSBAND::BAND_1, gnut::GOBSBAND::BAND_2, gnut::GOBSBAND::BAND_5};
        m[GSYS::GLO] = {gnut::GOBSBAND::BAND,
                        gnut::GOBSBAND::BAND_1,
                        gnut::GOBSBAND::BAND_2,
                        gnut::GOBSBAND::BAND_3,
                        gnut::GOBSBAND::BAND_5};
        m[GSYS::GAL] = {gnut::GOBSBAND::BAND,
                        gnut::GOBSBAND::BAND_1,
                        gnut::GOBSBAND::BAND_5,
                        gnut::GOBSBAND::BAND_7,
                        gnut::GOBSBAND::BAND_8,
                        gnut::GOBSBAND::BAND_6};
        // BDS bands: B1I, B2I(BDS-2), B3I, B2a(BDS-3), B2b(BDS-3), B2(BDS-3), B1C(BDS-3)
        m[GSYS::BDS] = {gnut::GOBSBAND::BAND,
                        gnut::GOBSBAND::BAND_2,
                        gnut::GOBSBAND::BAND_7,
                        gnut::GOBSBAND::BAND_6,
                        gnut::GOBSBAND::BAND_5,
                        gnut::GOBSBAND::BAND_9,
                        gnut::GOBSBAND::BAND_8,
                        gnut::GOBSBAND::BAND_1};
        m[GSYS::QZS] = {gnut::GOBSBAND::BAND,
                        gnut::GOBSBAND::BAND_1,
                        gnut::GOBSBAND::BAND_2,
                        gnut::GOBSBAND::BAND_5,
                        gnut::GOBSBAND::BAND_6};
        m[GSYS::SBS] = {gnut::GOBSBAND::BAND, gnut::GOBSBAND::BAND_1, gnut::GOBSBAND::BAND_2, gnut::GOBSBAND::BAND_5};
        m[GSYS::GNS] = {};

        return m;
    }

    // sort set of bands w.r.t. wavelength
    std::vector<GOBSBAND> sort_band(GSYS gs, std::set<GOBSBAND>& bands)
    {
        std::vector<GOBSBAND> vec;

        t_map_band bands_sorted = GNSS_BAND_SORTED();

        auto itGSYS = bands_sorted.find(gs);
        if (itGSYS != bands_sorted.end())
        {
            for (auto itBAND = itGSYS->second.begin(); itBAND != itGSYS->second.end(); itBAND++)
            {
                if (bands.find(*itBAND) != bands.end())
                {
                    vec.push_back(*itBAND);
                }
            }
        }

        return vec;
    }

    t_map_offs GNSS_PCO_OFFSETS()
    {
        // axes orientations are in the IGS conventions

        t_map_offs m;
        // triple: ATX  NORTH / EAST / UP
        t_gtriple gpsIIA(279, 0, 2500); //  279.00,   0.00,   2256.50 - 2878.60
        t_gtriple gpsIIR(0, 0, 1000);   //    0,00,   0.00,    681.10 - 1506.40
        t_gtriple gpsIIF(394, 0, 1500); //  394.00,   0.00,   1561.30 - 1600.00
        t_gtriple gpsGEN(200, 0, 1500); // aprox. mean
        m[GSYS::GPS][gnut::GOBSBAND::BAND_1] = {gpsGEN};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_2] = {gpsGEN};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_5] = {gpsGEN};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_A] = {gpsGEN};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_B] = {gpsGEN};

        t_gtriple gloI(0, 0, 2000);      //     0.00, 0.00,    1962.00 - 2222.30
        t_gtriple gloII(-545, 0, 2000);  //  -545.00, 0.00,    2179.40 - 2743.60
        t_gtriple gloGEN(-545, 0, 2000); // approx. mean
        m[GSYS::GLO][gnut::GOBSBAND::BAND_1] = {gloGEN};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_2] = {gloGEN};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_3] = {gloGEN};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_A] = {gloGEN};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_B] = {gloGEN};

        t_gtriple galGEN(-170, 30, 950);
        m[GSYS::GAL][gnut::GOBSBAND::BAND_1] = {galGEN};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_5] = {galGEN};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_6] = {galGEN};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_7] = {galGEN};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_8] = {galGEN};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_A] = {galGEN};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_B] = {galGEN};

        t_gtriple bdsGEN(634, 3, 1075);
        // m[BDS][BAND_1] = { bdsGEN };
        m[GSYS::BDS][gnut::GOBSBAND::BAND_2] = {bdsGEN};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_6] = {bdsGEN};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_7] = {bdsGEN};

        // TODO ... 5 9 8 1 glfeng, do not know default value

        t_gtriple sbsGEN(0, 0, 0); // UNKNOWN!
        m[GSYS::SBS][gnut::GOBSBAND::BAND_1] = {sbsGEN};
        m[GSYS::SBS][gnut::GOBSBAND::BAND_5] = {sbsGEN};

        m[GSYS::QZS][gnut::GOBSBAND::BAND_1] = {t_gtriple(-0.9, 2.9, 3197.9)};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_2] = {t_gtriple(-0.9, 2.9, 2992.9)};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_5] = {t_gtriple(-0.9, 2.9, 3077.9)};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_6] = {t_gtriple(-0.9, 2.9, 3147.9)};

        return m;
    }

    std::set<GSYS> GNSS_SUPPORTED()
    {
        std::set<GSYS> systems;

        systems.insert(GSYS::GPS);
        systems.insert(GSYS::GLO);
        systems.insert(GSYS::GAL);
        systems.insert(GSYS::BDS);
        systems.insert(GSYS::QZS);
        systems.insert(GSYS::IRN);

        return systems;
    }

    t_map_gnss GNSS_DATA_PRIORITY()
    {
        t_map_gnss m;

        m[GSYS::GPS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_W,
                                                                        gnut::GOBSATTR::ATTR_Y,
                                                                        gnut::GOBSATTR::ATTR_M,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_W,
                                                                        gnut::GOBSATTR::ATTR_Y,
                                                                        gnut::GOBSATTR::ATTR_M,
                                                                        gnut::GOBSATTR::ATTR_N,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_W,
                                                                        gnut::GOBSATTR::ATTR_Y,
                                                                        gnut::GOBSATTR::ATTR_M,
                                                                        gnut::GOBSATTR::ATTR_N,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_W,
                                                                        gnut::GOBSATTR::ATTR_Y,
                                                                        gnut::GOBSATTR::ATTR_M,
                                                                        gnut::GOBSATTR::ATTR_N,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_P] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GPS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_W,
                                                                        gnut::GOBSATTR::ATTR_Y,
                                                                        gnut::GOBSATTR::ATTR_M,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_W,
                                                                        gnut::GOBSATTR::ATTR_Y,
                                                                        gnut::GOBSATTR::ATTR_M,
                                                                        gnut::GOBSATTR::ATTR_N,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_W,
                                                                        gnut::GOBSATTR::ATTR_Y,
                                                                        gnut::GOBSATTR::ATTR_M,
                                                                        gnut::GOBSATTR::ATTR_N,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_W,
                                                                        gnut::GOBSATTR::ATTR_Y,
                                                                        gnut::GOBSATTR::ATTR_M,
                                                                        gnut::GOBSATTR::ATTR_N,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_P] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GPS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GPS][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GPS][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GPS][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GLO][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_P] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GLO][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_P] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GLO][gnut::GOBSBAND::BAND_3][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_3][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_3][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_3][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GLO][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GLO][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GLO][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GAL][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_A,
                                                                        gnut::GOBSATTR::ATTR_B,
                                                                        gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_A,
                                                                        gnut::GOBSATTR::ATTR_B,
                                                                        gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_A,
                                                                        gnut::GOBSATTR::ATTR_B,
                                                                        gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_A,
                                                                        gnut::GOBSATTR::ATTR_B,
                                                                        gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GAL][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GAL][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_A,
                                                                        gnut::GOBSATTR::ATTR_B,
                                                                        gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_A,
                                                                        gnut::GOBSATTR::ATTR_B,
                                                                        gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_A,
                                                                        gnut::GOBSATTR::ATTR_B,
                                                                        gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_A,
                                                                        gnut::GOBSATTR::ATTR_B,
                                                                        gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GAL][gnut::GOBSBAND::BAND_7][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_7][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_7][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_7][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GAL][gnut::GOBSBAND::BAND_8][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_8][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_8][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_8][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GAL][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_A][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::GAL][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::GAL][gnut::GOBSBAND::BAND_B][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::BDS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::BDS][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::BDS][gnut::GOBSBAND::BAND_7][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_7][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_7][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_7][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::BDS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        // bds-3 B2b
        m[GSYS::BDS][gnut::GOBSBAND::BAND_9][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_9][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_9][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_9][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_Z,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::BDS][gnut::GOBSBAND::BAND_8][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_8][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_8][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_8][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::BDS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};
        m[GSYS::BDS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_D,
                                                                        gnut::GOBSATTR::ATTR_P,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::SBS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_C};
        m[GSYS::SBS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_C};
        m[GSYS::SBS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_C};
        m[GSYS::SBS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_C};
        m[GSYS::SBS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_P] = {gnut::GOBSATTR::ATTR_NULL};

        m[GSYS::SBS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::SBS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::SBS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::SBS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X};

        m[GSYS::QZS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_1][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_C,
                                                                        gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X,
                                                                        gnut::GOBSATTR::ATTR_Z};

        m[GSYS::QZS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_2][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X};

        m[GSYS::QZS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_5][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_I,
                                                                        gnut::GOBSATTR::ATTR_Q,
                                                                        gnut::GOBSATTR::ATTR_X};

        m[GSYS::QZS][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_C] = {gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_L] = {gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_D] = {gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X};
        m[GSYS::QZS][gnut::GOBSBAND::BAND_6][gnut::GOBSTYPE::TYPE_S] = {gnut::GOBSATTR::ATTR_S,
                                                                        gnut::GOBSATTR::ATTR_L,
                                                                        gnut::GOBSATTR::ATTR_X};

        return m;
    }

    GOBSTYPE char2gobstype(char c)
    {
        if (c == 'C')
        {
            return gnut::GOBSTYPE::TYPE_C;
        }
        else if (c == 'L')
        {
            return gnut::GOBSTYPE::TYPE_L;
        }
        else if (c == 'D')
        {
            return gnut::GOBSTYPE::TYPE_D;
        }
        else if (c == 'S')
        {
            return gnut::GOBSTYPE::TYPE_S;
        }
        else if (c == 'R')
        {
            return gnut::GOBSTYPE::TYPE_SLR;
        }
        else if (c == 'P')
        {
            return gnut::GOBSTYPE::TYPE_P; // P-code only
        }

        return gnut::GOBSTYPE::TYPE;
    }

    LibGnut_LIBRARY_EXPORT FREQ_SEQ char2gnssfreq(char c)
    {
        if (c == '1')
        {
            return FREQ_SEQ::FREQ_1;
        }
        else if (c == '2')
        {
            return FREQ_SEQ::FREQ_2;
        }
        else if (c == '3')
        {
            return FREQ_SEQ::FREQ_3;
        }
        else if (c == '4')
        {
            return FREQ_SEQ::FREQ_4;
        }
        else if (c == '5')
        {
            return FREQ_SEQ::FREQ_5;
        }
        else if (c == '6')
        {
            return FREQ_SEQ::FREQ_6;
        }
        else if (c == '7')
        {
            return FREQ_SEQ::FREQ_7;
        }

        return FREQ_SEQ::FREQ_X;
    }

    GOBS_LC int2gobsfreq(int c)
    {
        if (c == 1)
        {
            return gnut::GOBS_LC::LC_L1;
        }
        else if (c == 2)
        {
            return gnut::GOBS_LC::LC_L2;
        }
        else if (c == 3)
        {
            return gnut::GOBS_LC::LC_L3;
        }
        else if (c == 4)
        {
            return gnut::GOBS_LC::LC_L4;
        }
        else if (c == 5)
        {
            return gnut::GOBS_LC::LC_L5;
        }

        return gnut::GOBS_LC::LC_UNDEF;
    }

    GOBSTYPE str2gobstype(std::string s)
    {
        return char2gobstype(s[0]);
    }

    GOBSBAND char2gobsband(char c)
    {
        if (c == '1')
        {
            return gnut::GOBSBAND::BAND_1;
        }
        else if (c == '2')
        {
            return gnut::GOBSBAND::BAND_2;
        }
        else if (c == '3')
        {
            return gnut::GOBSBAND::BAND_3;
        }
        else if (c == '5')
        {
            return gnut::GOBSBAND::BAND_5;
        }
        else if (c == '6')
        {
            return gnut::GOBSBAND::BAND_6;
        }
        else if (c == '7')
        {
            return gnut::GOBSBAND::BAND_7;
        }
        else if (c == '8')
        {
            return gnut::GOBSBAND::BAND_8;
        }
        else if (c == '9')
        {
            return gnut::GOBSBAND::BAND_9; // BDS-3 B2b
        }
        else if (c == 'A')
        {
            return gnut::GOBSBAND::BAND_A;
        }
        else if (c == 'B')
        {
            return gnut::GOBSBAND::BAND_B;
        }
        else if (c == 'C')
        {
            return gnut::GOBSBAND::BAND_C;
        }
        else if (c == 'D')
        {
            return gnut::GOBSBAND::BAND_D;
        }
        else if (c == 'R')
        {
            return gnut::GOBSBAND::BAND_SLR;
        }

        return gnut::GOBSBAND::BAND;
    }

    GOBSBAND int2gobsband(int c)
    {
        if (c == 1)
        {
            return gnut::GOBSBAND::BAND_1;
        }
        else if (c == 2)
        {
            return gnut::GOBSBAND::BAND_2;
        }
        else if (c == 3)
        {
            return gnut::GOBSBAND::BAND_3;
        }
        else if (c == 5)
        {
            return gnut::GOBSBAND::BAND_5;
        }
        else if (c == 6)
        {
            return gnut::GOBSBAND::BAND_6;
        }
        else if (c == 7)
        {
            return gnut::GOBSBAND::BAND_7;
        }
        else if (c == 8)
        {
            return gnut::GOBSBAND::BAND_8;
        }
        else if (c == 9)
        {
            return gnut::GOBSBAND::BAND_9; // BDS-3 B2b
        }

        return gnut::GOBSBAND::BAND;
    }

    GOBSBAND str2gobsband(std::string s)
    {
        // jdhuang : fix
        if (s.size() == 1)
        {
            return char2gobsband(s[0]);
        }
        else
        {
            return char2gobsband(s[1]);
        }
        // return char2gobsband(s[0]);
    }

    FREQ_SEQ str2sysfreq(std::string s)
    {
        if (s == "1")
        {
            return gnut::FREQ_SEQ::FREQ_1;
        }
        else if (s == "2")
        {
            return gnut::FREQ_SEQ::FREQ_2;
        }
        else if (s == "3")
        {
            return gnut::FREQ_SEQ::FREQ_3;
        }
        else if (s == "4")
        {
            return gnut::FREQ_SEQ::FREQ_4;
        }
        else if (s == "5")
        {
            return gnut::FREQ_SEQ::FREQ_5;
        }
        else if (s == "6")
        {
            return gnut::FREQ_SEQ::FREQ_6;
        }
        else if (s == "7")
        {
            return gnut::FREQ_SEQ::FREQ_7;
        }

        return gnut::FREQ_SEQ::FREQ_X;
    }

    GOBSATTR char2gobsattr(char c)
    {
        if (c == 'A')
        {
            return gnut::GOBSATTR::ATTR_A;
        }
        else if (c == 'B')
        {
            return gnut::GOBSATTR::ATTR_B;
        }
        else if (c == 'C')
        {
            return gnut::GOBSATTR::ATTR_C;
        }
        else if (c == 'D')
        {
            return gnut::GOBSATTR::ATTR_D;
        }
        else if (c == 'I')
        {
            return gnut::GOBSATTR::ATTR_I;
        }
        else if (c == 'L')
        {
            return gnut::GOBSATTR::ATTR_L;
        }
        else if (c == 'M')
        {
            return gnut::GOBSATTR::ATTR_M;
        }
        else if (c == 'N')
        {
            return gnut::GOBSATTR::ATTR_N;
        }
        else if (c == 'P')
        {
            return gnut::GOBSATTR::ATTR_P;
        }
        else if (c == 'Q')
        {
            return gnut::GOBSATTR::ATTR_Q;
        }
        else if (c == 'S')
        {
            return gnut::GOBSATTR::ATTR_S;
        }
        else if (c == 'W')
        {
            return gnut::GOBSATTR::ATTR_W;
        }
        else if (c == 'X')
        {
            return gnut::GOBSATTR::ATTR_X;
        }
        else if (c == 'Y')
        {
            return gnut::GOBSATTR::ATTR_Y;
        }
        else if (c == 'Z')
        {
            return gnut::GOBSATTR::ATTR_Z;
        }
        else if (c == ' ')
        {
            return gnut::GOBSATTR::ATTR_NULL; // 3CH conversion
        }
        else if (c == '\0')
        {
            return gnut::GOBSATTR::ATTR_NULL; // 2CH conversion
        }

        return gnut::GOBSATTR::ATTR; // undefined
    }

    GOBSATTR str2gobsattr(std::string s)
    {
        return char2gobsattr(s[2]); // can still handle 2-char or 3-char string (see char2gobsattr)
    }

    std::string gobstype2str(GOBSTYPE e)
    {
        switch (e)
        {
            case gnut::GOBSTYPE::TYPE_C:
                return "C";
            case gnut::GOBSTYPE::TYPE_L:
                return "L";
            case gnut::GOBSTYPE::TYPE_D:
                return "D";
            case gnut::GOBSTYPE::TYPE_S:
                return "S";
            case gnut::GOBSTYPE::TYPE_P:
                return "P";
            case gnut::GOBSTYPE::TYPE_SLR:
                return "SLR";
            case gnut::GOBSTYPE::TYPE_KBRRANGE:
                return "KRANGE";
            case gnut::GOBSTYPE::TYPE_LRIRANGE:
                return "LRANGE";
            case gnut::GOBSTYPE::TYPE_KBRRATE:
                return "KRATE";
            case gnut::GOBSTYPE::TYPE_LRIRATE:
                return "LRATE";

            case gnut::GOBSTYPE::TYPE:
                return "X";
            default:
                return "X";
        }
        return "X";
    }

    std::string gobsband2str(GOBSBAND e)
    {
        switch (e)
        {
            case gnut::GOBSBAND::BAND_1:
                return "1";
            case gnut::GOBSBAND::BAND_2:
                return "2";
            case gnut::GOBSBAND::BAND_3:
                return "3";
            case gnut::GOBSBAND::BAND_5:
                return "5";
            case gnut::GOBSBAND::BAND_6:
                return "6";
            case gnut::GOBSBAND::BAND_7:
                return "7";
            case gnut::GOBSBAND::BAND_8:
                return "8";
            case gnut::GOBSBAND::BAND_9:
                return "9"; // BDS-3 B2b

            case gnut::GOBSBAND::BAND_A:
                return "A";
            case gnut::GOBSBAND::BAND_B:
                return "B";
            case gnut::GOBSBAND::BAND_C:
                return "C";
            case gnut::GOBSBAND::BAND_D:
                return "D";
            case gnut::GOBSBAND::BAND_SLR:
                return "";
            case gnut::GOBSBAND::BAND_KBR:
                return "";
            case gnut::GOBSBAND::BAND_LRI:
                return "";

            case gnut::GOBSBAND::BAND:
                return "X";
            default:
                return "X";
        }
        return "X";
    }

    std::string gobsattr2str(GOBSATTR e)
    {
        switch (e)
        {
            case gnut::GOBSATTR::ATTR_A:
                return "A";
            case gnut::GOBSATTR::ATTR_B:
                return "B";
            case gnut::GOBSATTR::ATTR_C:
                return "C";
            case gnut::GOBSATTR::ATTR_D:
                return "D";
            case gnut::GOBSATTR::ATTR_I:
                return "I";
            case gnut::GOBSATTR::ATTR_L:
                return "L";
            case gnut::GOBSATTR::ATTR_M:
                return "M";
            case gnut::GOBSATTR::ATTR_N:
                return "N";
            case gnut::GOBSATTR::ATTR_P:
                return "P";
            case gnut::GOBSATTR::ATTR_Q:
                return "Q";
            case gnut::GOBSATTR::ATTR_S:
                return "S";
            case gnut::GOBSATTR::ATTR_W:
                return "W";
            case gnut::GOBSATTR::ATTR_X:
                return "X";
            case gnut::GOBSATTR::ATTR_Y:
                return "Y";
            case gnut::GOBSATTR::ATTR_Z:
                return "Z";

            case gnut::GOBSATTR::ATTR_NULL:
                return " ";

            case gnut::GOBSATTR::ATTR:
                return "";
            default:
                return "";
        }
        return "";
    }

    std::string gobs2str(GOBS o)
    {
        // PROBLEM WITH STATIC FUNCTION
        //  boost::mutex::scoped_lock lock(_mutex);

        switch (o)
        {
            // CODE
            case gnut::GOBS::C1A:
                return "C1A";
            case gnut::GOBS::C1B:
                return "C1B";
            case gnut::GOBS::C1C:
                return "C1C";
            case gnut::GOBS::C1D:
                return "C1D";
            case gnut::GOBS::C1I:
                return "C1I";
            case gnut::GOBS::C1L:
                return "C1L";
            case gnut::GOBS::C1M:
                return "C1M";
            case gnut::GOBS::C1P:
                return "C1P";
            case gnut::GOBS::C1Q:
                return "C1Q";
            case gnut::GOBS::C1S:
                return "C1S";
            case gnut::GOBS::C1W:
                return "C1W";
            case gnut::GOBS::C1X:
                return "C1X";
            case gnut::GOBS::C1Y:
                return "C1Y";
            case gnut::GOBS::C1Z:
                return "C1Z";

            case gnut::GOBS::C2C:
                return "C2C";
            case gnut::GOBS::C2D:
                return "C2D";
            case gnut::GOBS::C2I:
                return "C2I";
            case gnut::GOBS::C2L:
                return "C2L";
            case gnut::GOBS::C2M:
                return "C2M";
            case gnut::GOBS::C2P:
                return "C2P";
            case gnut::GOBS::C2Q:
                return "C2Q";
            case gnut::GOBS::C2S:
                return "C2S";
            case gnut::GOBS::C2W:
                return "C2W";
            case gnut::GOBS::C2X:
                return "C2X";
            case gnut::GOBS::C2Y:
                return "C2Y";

            case gnut::GOBS::C3I:
                return "C3I";
            case gnut::GOBS::C3Q:
                return "C3Q";
            case gnut::GOBS::C3X:
                return "C3X";

            case gnut::GOBS::C5A:
                return "C5A";
            case gnut::GOBS::C5B:
                return "C5B";
            case gnut::GOBS::C5C:
                return "C5C";
            case gnut::GOBS::C5D:
                return "C5D";
            case gnut::GOBS::C5I:
                return "C5I";
            case gnut::GOBS::C5P:
                return "C5P";
            case gnut::GOBS::C5Q:
                return "C5Q";
            case gnut::GOBS::C5X:
                return "C5X";

            case gnut::GOBS::C6A:
                return "C6A";
            case gnut::GOBS::C6B:
                return "C6B";
            case gnut::GOBS::C6C:
                return "C6C";
            case gnut::GOBS::C6I:
                return "C6I";
            case gnut::GOBS::C6L:
                return "C6L";
            case gnut::GOBS::C6S:
                return "C6S";
            case gnut::GOBS::C6Q:
                return "C6Q";
            case gnut::GOBS::C6X:
                return "C6X";
            case gnut::GOBS::C6Z:
                return "C6Z";

            case gnut::GOBS::C7I:
                return "C7I";
            case gnut::GOBS::C7Q:
                return "C7Q";
            case gnut::GOBS::C7X:
                return "C7X";

            case gnut::GOBS::C8D:
                return "C8D";
            case gnut::GOBS::C8I:
                return "C8I";
            case gnut::GOBS::C8P:
                return "C8P";
            case gnut::GOBS::C8Q:
                return "C8Q";
            case gnut::GOBS::C8X:
                return "C8X";

            case gnut::GOBS::C9D:
                return "C9D"; // BDS-3 B2b
            case gnut::GOBS::C9P:
                return "C9P";
            case gnut::GOBS::C9Z:
                return "C9Z";

            // CARRIER-PHASE
            case gnut::GOBS::L1A:
                return "L1A";
            case gnut::GOBS::L1B:
                return "L1B";
            case gnut::GOBS::L1C:
                return "L1C";
            case gnut::GOBS::L1D:
                return "L1D";
            case gnut::GOBS::L1I:
                return "L1I";
            case gnut::GOBS::L1L:
                return "L1L";
            case gnut::GOBS::L1M:
                return "L1M";
            case gnut::GOBS::L1N:
                return "L1N";
            case gnut::GOBS::L1P:
                return "L1P";
            case gnut::GOBS::L1Q:
                return "L1Q";
            case gnut::GOBS::L1S:
                return "L1S";
            case gnut::GOBS::L1W:
                return "L1W";
            case gnut::GOBS::L1X:
                return "L1X";
            case gnut::GOBS::L1Y:
                return "L1Y";
            case gnut::GOBS::L1Z:
                return "L1Z";

            case gnut::GOBS::L2C:
                return "L2C";
            case gnut::GOBS::L2D:
                return "L2D";
            case gnut::GOBS::L2I:
                return "L2I";
            case gnut::GOBS::L2L:
                return "L2L";
            case gnut::GOBS::L2M:
                return "L2M";
            case gnut::GOBS::L2N:
                return "L2N";
            case gnut::GOBS::L2P:
                return "L2P";
            case gnut::GOBS::L2Q:
                return "L2Q";
            case gnut::GOBS::L2S:
                return "L2S";
            case gnut::GOBS::L2W:
                return "L2W";
            case gnut::GOBS::L2X:
                return "L2X";
            case gnut::GOBS::L2Y:
                return "L2Y";

            case gnut::GOBS::L3I:
                return "L3I";
            case gnut::GOBS::L3Q:
                return "L3Q";
            case gnut::GOBS::L3X:
                return "L3X";

            case gnut::GOBS::L5A:
                return "L5A";
            case gnut::GOBS::L5B:
                return "L5B";
            case gnut::GOBS::L5C:
                return "L5C";
            case gnut::GOBS::L5D:
                return "L5D";
            case gnut::GOBS::L5I:
                return "L5I";
            case gnut::GOBS::L5P:
                return "L5P";
            case gnut::GOBS::L5Q:
                return "L5Q";
            case gnut::GOBS::L5X:
                return "L5X";

            case gnut::GOBS::L6A:
                return "L6A";
            case gnut::GOBS::L6B:
                return "L6B";
            case gnut::GOBS::L6C:
                return "L6C";
            case gnut::GOBS::L6L:
                return "L6L";
            case gnut::GOBS::L6S:
                return "L6S";
            case gnut::GOBS::L6I:
                return "L6I";
            case gnut::GOBS::L6Q:
                return "L6Q";
            case gnut::GOBS::L6X:
                return "L6X";
            case gnut::GOBS::L6Z:
                return "L6Z";

            case gnut::GOBS::L7I:
                return "L7I";
            case gnut::GOBS::L7Q:
                return "L7Q";
            case gnut::GOBS::L7X:
                return "L7X";

            case gnut::GOBS::L8D:
                return "L8D";
            case gnut::GOBS::L8I:
                return "L8I";
            case gnut::GOBS::L8P:
                return "L8P";
            case gnut::GOBS::L8Q:
                return "L8Q";
            case gnut::GOBS::L8X:
                return "L8X";

            case gnut::GOBS::L9D:
                return "L9D"; // BDS-3 B2b
            case gnut::GOBS::L9P:
                return "L9P";
            case gnut::GOBS::L9Z:
                return "L9Z";

            // DOPLER
            case gnut::GOBS::D1A:
                return "D1A";
            case gnut::GOBS::D1B:
                return "D1B";
            case gnut::GOBS::D1C:
                return "D1C";
            case gnut::GOBS::D1D:
                return "D1D";
            case gnut::GOBS::D1I:
                return "D1I";
            case gnut::GOBS::D1L:
                return "D1L";
            case gnut::GOBS::D1M:
                return "D1M";
            case gnut::GOBS::D1N:
                return "D1N";
            case gnut::GOBS::D1P:
                return "D1P";
            case gnut::GOBS::D1Q:
                return "D1Q";
            case gnut::GOBS::D1S:
                return "D1S";
            case gnut::GOBS::D1W:
                return "D1W";
            case gnut::GOBS::D1X:
                return "D1X";
            case gnut::GOBS::D1Y:
                return "D1Y";
            case gnut::GOBS::D1Z:
                return "D1Z";

            case gnut::GOBS::D2C:
                return "D2C";
            case gnut::GOBS::D2D:
                return "D2D";
            case gnut::GOBS::D2I:
                return "D2I";
            case gnut::GOBS::D2L:
                return "D2L";
            case gnut::GOBS::D2M:
                return "D2M";
            case gnut::GOBS::D2N:
                return "D2N";
            case gnut::GOBS::D2P:
                return "D2P";
            case gnut::GOBS::D2Q:
                return "D2Q";
            case gnut::GOBS::D2S:
                return "D2S";
            case gnut::GOBS::D2W:
                return "D2W";
            case gnut::GOBS::D2X:
                return "D2X";
            case gnut::GOBS::D2Y:
                return "D2Y";

            case gnut::GOBS::D3I:
                return "D3I";
            case gnut::GOBS::D3Q:
                return "D3Q";
            case gnut::GOBS::D3X:
                return "D3X";

            case gnut::GOBS::D5A:
                return "D5A";
            case gnut::GOBS::D5B:
                return "D5B";
            case gnut::GOBS::D5C:
                return "D5C";
            case gnut::GOBS::D5D:
                return "D5D";
            case gnut::GOBS::D5I:
                return "D5I";
            case gnut::GOBS::D5P:
                return "D5P";
            case gnut::GOBS::D5Q:
                return "D5Q";
            case gnut::GOBS::D5X:
                return "D5X";

            case gnut::GOBS::D6A:
                return "D6A";
            case gnut::GOBS::D6B:
                return "D6B";
            case gnut::GOBS::D6C:
                return "D6C";
            case gnut::GOBS::D6I:
                return "D6I";
            case gnut::GOBS::D6L:
                return "D6L";
            case gnut::GOBS::D6S:
                return "D6S";
            case gnut::GOBS::D6Q:
                return "D6Q";
            case gnut::GOBS::D6X:
                return "D6X";
            case gnut::GOBS::D6Z:
                return "D6Z";

            case gnut::GOBS::D7I:
                return "D7I";
            case gnut::GOBS::D7Q:
                return "D7Q";
            case gnut::GOBS::D7X:
                return "D7X";

            case gnut::GOBS::D8D:
                return "D8D";
            case gnut::GOBS::D8I:
                return "D8I";
            case gnut::GOBS::D8P:
                return "D8P";
            case gnut::GOBS::D8Q:
                return "D8Q";
            case gnut::GOBS::D8X:
                return "D8X";

            case gnut::GOBS::D9D:
                return "D9D"; // BDS-3 B2b
            case gnut::GOBS::D9P:
                return "D9P";
            case gnut::GOBS::D9Z:
                return "D9Z";

            // SNR
            case gnut::GOBS::S1A:
                return "S1A";
            case gnut::GOBS::S1B:
                return "S1B";
            case gnut::GOBS::S1C:
                return "S1C";
            case gnut::GOBS::S1D:
                return "S1D";
            case gnut::GOBS::S1I:
                return "S1I";
            case gnut::GOBS::S1L:
                return "S1L";
            case gnut::GOBS::S1M:
                return "S1M";
            case gnut::GOBS::S1N:
                return "S1N";
            case gnut::GOBS::S1P:
                return "S1P";
            case gnut::GOBS::S1Q:
                return "S1Q";
            case gnut::GOBS::S1S:
                return "S1S";
            case gnut::GOBS::S1W:
                return "S1W";
            case gnut::GOBS::S1X:
                return "S1X";
            case gnut::GOBS::S1Y:
                return "S1Y";
            case gnut::GOBS::S1Z:
                return "S1Z";

            case gnut::GOBS::S2C:
                return "S2C";
            case gnut::GOBS::S2D:
                return "S2D";
            case gnut::GOBS::S2I:
                return "S2I";
            case gnut::GOBS::S2L:
                return "S2L";
            case gnut::GOBS::S2M:
                return "S2M";
            case gnut::GOBS::S2N:
                return "S2N";
            case gnut::GOBS::S2P:
                return "S2P";
            case gnut::GOBS::S2Q:
                return "S2Q";
            case gnut::GOBS::S2S:
                return "S2S";
            case gnut::GOBS::S2W:
                return "S2W";
            case gnut::GOBS::S2X:
                return "S2X";
            case gnut::GOBS::S2Y:
                return "S2Y";

            case gnut::GOBS::S3I:
                return "S3I";
            case gnut::GOBS::S3Q:
                return "S3Q";
            case gnut::GOBS::S3X:
                return "S3X";

            case gnut::GOBS::S5A:
                return "S5A";
            case gnut::GOBS::S5B:
                return "S5B";
            case gnut::GOBS::S5C:
                return "S5C";
            case gnut::GOBS::S5D:
                return "S5D";
            case gnut::GOBS::S5I:
                return "S5I";
            case gnut::GOBS::S5P:
                return "S5P";
            case gnut::GOBS::S5Q:
                return "S5Q";
            case gnut::GOBS::S5X:
                return "S5X";

            case gnut::GOBS::S6A:
                return "S6A";
            case gnut::GOBS::S6B:
                return "S6B";
            case gnut::GOBS::S6C:
                return "S6C";
            case gnut::GOBS::S6I:
                return "S6I";
            case gnut::GOBS::S6L:
                return "S6L";
            case gnut::GOBS::S6S:
                return "S6S";
            case gnut::GOBS::S6Q:
                return "S6Q";
            case gnut::GOBS::S6X:
                return "S6X";
            case gnut::GOBS::S6Z:
                return "S6Z";

            case gnut::GOBS::S7I:
                return "S7I";
            case gnut::GOBS::S7Q:
                return "S7Q";
            case gnut::GOBS::S7X:
                return "S7X";

            case gnut::GOBS::S8D:
                return "S8D";
            case gnut::GOBS::S8I:
                return "S8I";
            case gnut::GOBS::S8P:
                return "S8P";
            case gnut::GOBS::S8Q:
                return "S8Q";
            case gnut::GOBS::S8X:
                return "S8X";

            case gnut::GOBS::S9D:
                return "S9D"; // BDS-3 B2b
            case gnut::GOBS::S9P:
                return "S9P";
            case gnut::GOBS::S9Z:
                return "S9Z";

            // special cases: v2.x or unknown tracking mode
            case gnut::GOBS::C1:
                return "C1 "; // unknown mode GPS P,Y              Galileo E2-L1-E2 I/Q
            case gnut::GOBS::C2:
                return "C2 "; // unknown mode GPS P,Y                                     COMPASS E2 I/Q
            case gnut::GOBS::C5:
                return "C5 "; // unknown mode                      Galileo E5a I/Q
            case gnut::GOBS::C6:
                return "C6 "; // unknown mode                      Galileo E6  I/Q        COMPASS E6 I/Q
            case gnut::GOBS::C7:
                return "C7 "; // unknown mode                      Galielo E5b I/Q        COMPASS E5b I/Q
            case gnut::GOBS::C8:
                return "C8 "; // unknown mode                      Galileo E5  I/Q
            case gnut::GOBS::CA:
                return "CA "; // unknown mode GPS C/A  GLONASS SA
            case gnut::GOBS::CB:
                return "CB "; // unknown mode GPS C1C
            case gnut::GOBS::CC:
                return "CC "; // unknown mode GPS C2C
            case gnut::GOBS::CD:
                return "CD "; // unknown mode          GLONASS SA

            case gnut::GOBS::P1:
                return "P1 "; // unknown mode GPS P,Y  GLONASS HA
            case gnut::GOBS::P2:
                return "P2 "; // unknown mode GPS P,Y  GLONASS HA
            case gnut::GOBS::P5:
                return "P5 "; // unknown mode GPS P,Y

            case gnut::GOBS::L1:
                return "L1 "; // unknown mode GPS P,Y              Galileo E2-L1-E2 I/Q
            case gnut::GOBS::L2:
                return "L2 "; // unknown mode GPS P,Y                                     COMPASS E2 I/Q
            case gnut::GOBS::L5:
                return "L5 "; // unknown mode                      Galileo E5a I/Q
            case gnut::GOBS::L6:
                return "L6 "; // unknown mode                      Galileo E6  I/Q        COMPASS E6 I/Q
            case gnut::GOBS::L7:
                return "L7 "; // unknown mode                      Galielo E5b I/Q        COMPASS E5b I/Q
            case gnut::GOBS::L8:
                return "L8 "; // unknown mode                      Galileo E5  I/Q
            case gnut::GOBS::LA:
                return "LA "; // unknown mode GPS C/A  GLONASS SA
            case gnut::GOBS::LB:
                return "LB "; // unknown mode GPS C1C
            case gnut::GOBS::LC:
                return "LC "; // unknown mode GPS C2C
            case gnut::GOBS::LD:
                return "LD "; // unknown mode          GLONASS SA

            case gnut::GOBS::D1:
                return "D1 "; // unknown mode GPS P,Y              Galileo E2-L1-E2 I/Q
            case gnut::GOBS::D2:
                return "D2 "; // unknown mode GPS P,Y                                     COMPASS E2 I/Q
            case gnut::GOBS::D5:
                return "D5 "; // unknown mode                      Galileo E5a I/Q
            case gnut::GOBS::D6:
                return "D6 "; // unknown mode                      Galileo E6  I/Q        COMPASS E6 I/Q
            case gnut::GOBS::D7:
                return "D7 "; // unknown mode                      Galielo E5b I/Q        COMPASS E5b I/Q
            case gnut::GOBS::D8:
                return "D8 "; // unknown mode                      Galileo E5  I/Q
            case gnut::GOBS::DA:
                return "DA "; // unknown mode GPS C/A  GLONASS SA
            case gnut::GOBS::DB:
                return "DB "; // unknown mode GPS C1C
            case gnut::GOBS::DC:
                return "DC "; // unknown mode GPS C2C
            case gnut::GOBS::DD:
                return "DD "; // unknown mode          GLONASS SA

            case gnut::GOBS::S1:
                return "S1 "; // unknown mode GPS P,Y              Galileo E2-L1-E2 I/Q
            case gnut::GOBS::S2:
                return "S2 "; // unknown mode GPS P,Y                                     COMPASS E2 I/Q
            case gnut::GOBS::S5:
                return "S5 "; // unknown mode                      Galileo E5a I/Q
            case gnut::GOBS::S6:
                return "S6 "; // unknown mode                      Galileo E6  I/Q        COMPASS E6 I/Q
            case gnut::GOBS::S7:
                return "S7 "; // unknown mode                      Galielo E5b I/Q        COMPASS E5b I/Q
            case gnut::GOBS::S8:
                return "S8 "; // unknown mode                      Galileo E5  I/Q
            case gnut::GOBS::SA:
                return "SA "; // unknown mode GPS C/A  GLONASS SA
            case gnut::GOBS::SB:
                return "SB "; // unknown mode GPS C1C
            case gnut::GOBS::SC:
                return "SC "; // unknown mode GPS C2C
            case gnut::GOBS::SD:
                return "SD "; // unknown mode          GLONASS SA

                //    case LAST_GOBS: return "XXX";
            case gnut::GOBS::X:
                return "XXX";

            default:
                std::cerr << "*** Warning - unknown observation code " << static_cast<int>(o) << "\n";
        }

        return "XXX";
    }

    std::string gfreqseq2str(FREQ_SEQ f)
    {
        switch (f)
        {
            case gnut::FREQ_SEQ::FREQ_1:
                return "1";
            case gnut::FREQ_SEQ::FREQ_2:
                return "2";
            case gnut::FREQ_SEQ::FREQ_3:
                return "3";
            case gnut::FREQ_SEQ::FREQ_4:
                return "4";
            case gnut::FREQ_SEQ::FREQ_5:
                return "5";
            case gnut::FREQ_SEQ::FREQ_6:
                return "6";
            case gnut::FREQ_SEQ::FREQ_7:
                return "7";
            case gnut::FREQ_SEQ::FREQ_X:
                return "X";
        }
        return "X";
    }

    GOBS str2gobs(std::string s)
    {
        if (s.length() > 2 && s[2] == ' ')
        {
            s = s.substr(0, 2);
        }

        int len = trim(s).length();
        if (len == 3)
        {
            if (s[0] == 'C')
            { // code
                if (s[1] == '1')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::C1A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::C1B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::C1C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::C1D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::C1I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::C1L;
                    }
                    if (s[2] == 'M')
                    {
                        return gnut::GOBS::C1M;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::C1P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::C1Q;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::C1S;
                    }
                    if (s[2] == 'W')
                    {
                        return gnut::GOBS::C1W;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::C1X;
                    }
                    if (s[2] == 'Y')
                    {
                        return gnut::GOBS::C1Y;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::C1Z;
                    }
                }
                if (s[1] == '2')
                { // band
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::C2C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::C2D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::C2I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::C2L;
                    }
                    if (s[2] == 'M')
                    {
                        return gnut::GOBS::C2M;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::C2P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::C2Q;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::C2S;
                    }
                    if (s[2] == 'W')
                    {
                        return gnut::GOBS::C2W;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::C2X;
                    }
                    if (s[2] == 'Y')
                    {
                        return gnut::GOBS::C2Y;
                    }
                }
                if (s[1] == '3')
                { // band
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::C3I;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::C3Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::C3X;
                    }
                }
                if (s[1] == '5')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::C5A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::C5B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::C5C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::C5D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::C5I;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::C5P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::C5Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::C5X;
                    }
                }
                if (s[1] == '6')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::C6A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::C6B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::C6C;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::C6I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::C6L;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::C6S;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::C6Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::C6X;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::C6Z;
                    }
                }
                if (s[1] == '7')
                { // band
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::C7I;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::C7Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::C7X;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::C9D;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::C9P;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::C9Z;
                    }
                }
                if (s[1] == '8')
                { // band
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::C8D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::C8I;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::C8P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::C8Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::C8X;
                    }
                }
                if (s[1] == '9')
                { // band  for bds3 B2b
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::C9D;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::C9P;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::C9Z;
                    }
                }
            }

            if (s[0] == 'L')
            { // phase
                if (s[1] == '1')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::L1A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::L1B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::L1C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::L1D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::L1I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::L1L;
                    }
                    if (s[2] == 'M')
                    {
                        return gnut::GOBS::L1M;
                    }
                    if (s[2] == 'N')
                    {
                        return gnut::GOBS::L1N;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::L1P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::L1Q;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::L1S;
                    }
                    if (s[2] == 'W')
                    {
                        return gnut::GOBS::L1W;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::L1X;
                    }
                    if (s[2] == 'Y')
                    {
                        return gnut::GOBS::L1Y;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::L1Z;
                    }
                }
                if (s[1] == '2')
                { // band
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::L2C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::L2D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::L2I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::L2L;
                    }
                    if (s[2] == 'M')
                    {
                        return gnut::GOBS::L2M;
                    }
                    if (s[2] == 'N')
                    {
                        return gnut::GOBS::L2N;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::L2P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::L2Q;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::L2S;
                    }
                    if (s[2] == 'W')
                    {
                        return gnut::GOBS::L2W;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::L2X;
                    }
                    if (s[2] == 'Y')
                    {
                        return gnut::GOBS::L2Y;
                    }
                }
                if (s[1] == '3')
                { // band
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::L3I;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::L3Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::L3X;
                    }
                }
                if (s[1] == '5')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::L5A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::L5B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::L5C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::L5D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::L5I;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::L5P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::L5Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::L5X;
                    }
                }
                if (s[1] == '6')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::L6A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::L6B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::L6C;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::L6I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::L6L;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::L6S;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::L6Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::L6X;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::L6Z;
                    }
                }
                if (s[1] == '7')
                { // band
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::L7I;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::L7Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::L7X;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::L9D;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::L9P;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::L9Z;
                    }
                }
                if (s[1] == '8')
                { // band
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::L8D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::L8I;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::L8P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::L8Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::L8X;
                    }
                }
                if (s[1] == '9')
                { // band  for bds3 B2b
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::L9D;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::L9P;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::L9Z;
                    }
                }
            }

            if (s[0] == 'D')
            { // doppler
                if (s[1] == '1')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::D1A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::D1B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::D1C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::D1D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::D1I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::D1L;
                    }
                    if (s[2] == 'M')
                    {
                        return gnut::GOBS::D1M;
                    }
                    if (s[2] == 'N')
                    {
                        return gnut::GOBS::D1N;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::D1P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::D1Q;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::D1S;
                    }
                    if (s[2] == 'W')
                    {
                        return gnut::GOBS::D1W;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::D1X;
                    }
                    if (s[2] == 'Y')
                    {
                        return gnut::GOBS::D1Y;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::D1Z;
                    }
                }
                if (s[1] == '2')
                { // band
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::D2C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::D2D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::D2I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::D2L;
                    }
                    if (s[2] == 'M')
                    {
                        return gnut::GOBS::D2M;
                    }
                    if (s[2] == 'N')
                    {
                        return gnut::GOBS::D2N;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::D2P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::D2Q;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::D2S;
                    }
                    if (s[2] == 'W')
                    {
                        return gnut::GOBS::D2W;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::D2X;
                    }
                    if (s[2] == 'Y')
                    {
                        return gnut::GOBS::D2Y;
                    }
                }
                if (s[1] == '3')
                { // band
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::D3I;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::D3Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::D3X;
                    }
                }
                if (s[1] == '5')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::D5A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::D5B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::D5C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::D5D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::D5I;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::D5P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::D5Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::D5X;
                    }
                }
                if (s[1] == '6')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::D6A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::D6B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::D6C;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::D6I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::D6L;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::D6S;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::D6Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::D6X;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::D6Z;
                    }
                }
                if (s[1] == '7')
                { // band
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::D7I;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::D7Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::D7X;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::D9D;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::D9P;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::D9Z;
                    }
                }
                if (s[1] == '8')
                { // band
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::D8D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::D8I;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::D8P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::D8Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::D8X;
                    }
                }

                if (s[1] == '9')
                { // band  for bds3 B2b
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::D9D;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::D9P;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::D9Z;
                    }
                }
            }

            if (s[0] == 'S')
            { // signal-to-noise ratio
                if (s[1] == '1')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::S1A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::S1B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::S1C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::S1D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::S1I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::S1L;
                    }
                    if (s[2] == 'M')
                    {
                        return gnut::GOBS::S1M;
                    }
                    if (s[2] == 'N')
                    {
                        return gnut::GOBS::S1N;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::S1P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::S1Q;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::S1S;
                    }
                    if (s[2] == 'W')
                    {
                        return gnut::GOBS::S1W;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::S1X;
                    }
                    if (s[2] == 'Y')
                    {
                        return gnut::GOBS::S1Y;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::S1Z;
                    }
                }
                if (s[1] == '2')
                { // band
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::S2C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::S2D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::S2I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::S2L;
                    }
                    if (s[2] == 'M')
                    {
                        return gnut::GOBS::S2M;
                    }
                    if (s[2] == 'N')
                    {
                        return gnut::GOBS::S2N;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::S2P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::S2Q;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::S2S;
                    }
                    if (s[2] == 'W')
                    {
                        return gnut::GOBS::S2W;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::S2X;
                    }
                    if (s[2] == 'Y')
                    {
                        return gnut::GOBS::S2Y;
                    }
                }
                if (s[1] == '3')
                { // band
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::S3I;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::S3Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::S3X;
                    }
                }
                if (s[1] == '5')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::S5A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::S5B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::S5C;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::S5D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::S5I;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::S5P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::S5Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::S5X;
                    }
                }
                if (s[1] == '6')
                { // band
                    if (s[2] == 'A')
                    {
                        return gnut::GOBS::S6A;
                    }
                    if (s[2] == 'B')
                    {
                        return gnut::GOBS::S6B;
                    }
                    if (s[2] == 'C')
                    {
                        return gnut::GOBS::S6C;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::S6I;
                    }
                    if (s[2] == 'L')
                    {
                        return gnut::GOBS::S6L;
                    }
                    if (s[2] == 'S')
                    {
                        return gnut::GOBS::S6S;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::S6Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::S6X;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::S6Z;
                    }
                }
                if (s[1] == '7')
                { // band
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::S7I;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::S7Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::S7X;
                    }
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::S9D;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::S9P;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::S9Z;
                    }
                }
                if (s[1] == '8')
                { // band
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::S8D;
                    }
                    if (s[2] == 'I')
                    {
                        return gnut::GOBS::S8I;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::S8P;
                    }
                    if (s[2] == 'Q')
                    {
                        return gnut::GOBS::S8Q;
                    }
                    if (s[2] == 'X')
                    {
                        return gnut::GOBS::S8X;
                    }
                }
                if (s[1] == '9')
                { // band  for bds3 B2b
                    if (s[2] == 'D')
                    {
                        return gnut::GOBS::S9D;
                    }
                    if (s[2] == 'P')
                    {
                        return gnut::GOBS::S9P;
                    }
                    if (s[2] == 'Z')
                    {
                        return gnut::GOBS::S9Z;
                    }
                }
            }
        }

        if (len == 2)
        {
            if (s[0] == 'C')
            { // code
                if (s[1] == '1')
                {
                    return gnut::GOBS::C1; // unknown mode GPS P,Y              Galileo E2-L1-E2 I/Q
                }
                if (s[1] == '2')
                {
                    return gnut::GOBS::C2; // unknown mode GPS P,Y                                     COMPASS E2 I/Q
                }
                if (s[1] == '5')
                {
                    return gnut::GOBS::C5; // unknown mode                      Galileo E5a I/Q
                }
                if (s[1] == '6')
                {
                    return gnut::GOBS::C6; // unknown mode                      Galileo E6  I/Q        COMPASS E6 I/Q
                }
                if (s[1] == '7')
                {
                    return gnut::GOBS::C7; // unknown mode                      Galielo E5b I/Q        COMPASS E5b I/Q
                }
                if (s[1] == '8')
                {
                    return gnut::GOBS::C8; // unknown mode                      Galileo E5  I/Q
                }
                if (s[1] == 'A')
                {
                    return gnut::GOBS::CA; // unknown mode GPS C/A  GLONASS SA
                }
                if (s[1] == 'B')
                {
                    return gnut::GOBS::CB; // unknown mode GPS C1C
                }
                if (s[1] == 'C')
                {
                    return gnut::GOBS::CC; // unknown mode GPS C2C
                }
                if (s[1] == 'D')
                {
                    return gnut::GOBS::CD; // unknown mode          GLONASS SA
                }
            }
            if (s[0] == 'P')
            { // code
                if (s[1] == '1')
                {
                    return gnut::GOBS::P1; // unknown mode GPS P,Y  GLONASS HA
                }
                if (s[1] == '2')
                {
                    return gnut::GOBS::P2; // unknown mode GPS P,Y  GLONASS HA
                }
                if (s[1] == '5')
                {
                    return gnut::GOBS::P5; // unknown mode GPS P,Y
                }
            }

            if (s[0] == 'L')
            { // phase
                if (s[1] == '1')
                {
                    return gnut::GOBS::L1; // unknown mode GPS P,Y              Galileo E2-L1-E2 I/Q
                }
                if (s[1] == '2')
                {
                    return gnut::GOBS::L2; // unknown mode GPS P,Y                                     COMPASS E2 I/Q
                }
                if (s[1] == '5')
                {
                    return gnut::GOBS::L5; // unknown mode                      Galileo E5a I/Q
                }
                if (s[1] == '6')
                {
                    return gnut::GOBS::L6; // unknown mode                      Galileo E6  I/Q        COMPASS E6 I/Q
                }
                if (s[1] == '7')
                {
                    return gnut::GOBS::L7; // unknown mode                      Galielo E5b I/Q        COMPASS E5b I/Q
                }
                if (s[1] == '8')
                {
                    return gnut::GOBS::L8; // unknown mode                      Galileo E5  I/Q
                }
                if (s[1] == 'A')
                {
                    return gnut::GOBS::LA; // unknown mode GPS C/A  GLONASS SA
                }
                if (s[1] == 'B')
                {
                    return gnut::GOBS::LB; // unknown mode GPS C1C
                }
                if (s[1] == 'C')
                {
                    return gnut::GOBS::LC; // unknown mode GPS C2C
                }
                if (s[1] == 'D')
                {
                    return gnut::GOBS::LD; // unknown mode          GLONASS SA
                }
            }

            if (s[0] == 'D')
            { // doppler
                if (s[1] == '1')
                {
                    return gnut::GOBS::D1; // unknown mode GPS P,Y              Galileo E2-L1-E2 I/Q
                }
                if (s[1] == '2')
                {
                    return gnut::GOBS::D2; // unknown mode GPS P,Y                                     COMPASS E2 I/Q
                }
                if (s[1] == '5')
                {
                    return gnut::GOBS::D5; // unknown mode                      Galileo E5a I/Q
                }
                if (s[1] == '6')
                {
                    return gnut::GOBS::D6; // unknown mode                      Galileo E6  I/Q        COMPASS E6 I/Q
                }
                if (s[1] == '7')
                {
                    return gnut::GOBS::D7; // unknown mode                      Galielo E5b I/Q        COMPASS E5b I/Q
                }
                if (s[1] == '8')
                {
                    return gnut::GOBS::D8; // unknown mode                      Galileo E5  I/Q
                }
                if (s[1] == 'A')
                {
                    return gnut::GOBS::DA; // unknown mode GPS C/A  GLONASS SA
                }
                if (s[1] == 'B')
                {
                    return gnut::GOBS::DB; // unknown mode GPS C1C
                }
                if (s[1] == 'C')
                {
                    return gnut::GOBS::DC; // unknown mode GPS C2C
                }
                if (s[1] == 'D')
                {
                    return gnut::GOBS::DD; // unknown mode          GLONASS SA
                }
            }

            if (s[0] == 'S')
            { // signal-to-noise ratio
                if (s[1] == '1')
                {
                    return gnut::GOBS::S1; // unknown mode GPS P,Y              Galileo E2-L1-E2 I/Q
                }
                if (s[1] == '2')
                {
                    return gnut::GOBS::S2; // unknown mode GPS P,Y                                     COMPASS E2 I/Q
                }
                if (s[1] == '5')
                {
                    return gnut::GOBS::S5; // unknown mode                      Galileo E5a I/Q
                }
                if (s[1] == '6')
                {
                    return gnut::GOBS::S6; // unknown mode                      Galileo E6  I/Q        COMPASS E6 I/Q
                }
                if (s[1] == '7')
                {
                    return gnut::GOBS::S7; // unknown mode                      Galielo E5b I/Q        COMPASS E5b I/Q
                }
                if (s[1] == '8')
                {
                    return gnut::GOBS::S8; // unknown mode                      Galileo E5  I/Q
                }
                if (s[1] == 'A')
                {
                    return gnut::GOBS::SA; // unknown mode GPS C/A  GLONASS SA
                }
                if (s[1] == 'B')
                {
                    return gnut::GOBS::SB; // unknown mode GPS C1C
                }
                if (s[1] == 'C')
                {
                    return gnut::GOBS::SC; // unknown mode GPS C2C
                }
                if (s[1] == 'D')
                {
                    return gnut::GOBS::SD; // unknown mode          GLONASS SA
                }
            }
        }

        return gnut::GOBS::X; // LAST_GOBS;
    }

    GOBS tba2gobs(GOBSTYPE type, GOBSBAND band, GOBSATTR attr)
    {
        std::string s = gobstype2str(type) + gobsband2str(band) + gobsattr2str(attr);
        return str2gobs(s);
    }

    std::string gnavtype2str(GNAVTYPE e)
    {
        switch (e)
        {
            case gnut::GNAVTYPE::NAV:
                return "NAV";
            case gnut::GNAVTYPE::FNAV:
                return "FNAV";
            case gnut::GNAVTYPE::CNAV:
                return "CNAV";
            case gnut::GNAVTYPE::INAV:
                return "INAV"; // combined E01+E07 (or could represent any-source INAV)
            case gnut::GNAVTYPE::INAV_E01:
                return "INAV_E01";
            case gnut::GNAVTYPE::INAV_E07:
                return "INAV_E07";
            default:
                return "NAV";
        }
        return "NAV";
    }

    GNAVTYPE str2gnavtype(std::string s)
    {
        if (s.compare("NAV") == 0)
        {
            return gnut::GNAVTYPE::NAV;
        }
        else if (s.compare("FNAV") == 0)
        {
            return gnut::GNAVTYPE::FNAV;
        }
        else if (s.compare("CNAV") == 0)
        {
            return gnut::GNAVTYPE::CNAV;
        }
        else if (s.compare("INAV") == 0)
        {
            return gnut::GNAVTYPE::INAV; // combined E01+E07, or any-source INAV
        }
        else if (s.compare("INAV_E01") == 0)
        {
            return gnut::GNAVTYPE::INAV_E01;
        }
        else if (s.compare("INAV_E07") == 0)
        {
            return gnut::GNAVTYPE::INAV_E07;
        }

        return gnut::GNAVTYPE::NAV;
    }

    LibGnut_LIBRARY_EXPORT FREQ_SEQ str2gnssfreq(std::string s)
    {
        // jdhuang : fix
        s = trim(s);
        if (s.size() == 1)
        {
            return char2gnssfreq(s[0]);
        }
        else
        {
            return char2gnssfreq(s[1]);
        }
    }

    int gobs2band(GOBS gobs)
    {
        if (gobs == gnut::GOBS::X)
        {
            return 0;
        }

        std::string go = gobs2str(gobs);
        if (go.length() < 2)
        {
            return 0; // DO NOT SUPPORT ANY SHORTER GOBS!!!
        }

        switch (go[1])
        {
            case '1':
                return 1;
            case 'A':
                return 1; // also 1
            case 'B':
                return 1; // also 1

            case '2':
                return 2;
            case 'C':
                return 2; // also 2
            case 'D':
                return 2; // also 2

            case '5':
                return 5;
            case '6':
                return 6;
            case '7':
                return 7;
            case '8':
                return 8;
            case '9':
                return 9; // BDS3 B2b

            default:
                return 0;
        }
        return 0;
    }

    GOBS pha2snr(GOBS o)
    {
        return static_cast<GOBS>(static_cast<int>(o) + 200);
    }

    GOBS pl2snr(GOBS o)
    {
        int value = static_cast<int>(o);
        if (value >= 1000)
        {
            value -= 1000;
        }
        if (value < 100)
        {
            value += 300;
        }
        else if (value < 200)
        {
            value += 200;
        }
        else if (value < 300)
        {
            value += 100;
        }
        return static_cast<GOBS>(value);
    }

    bool gobs_code(GOBS o)
    {
        const int value = static_cast<int>(o) % 1000;
        return value < 100;
    }

    bool gobs_phase(GOBS o)
    {
        const int value = static_cast<int>(o) % 1000;
        return value >= 100 && value < 200;
    }

    bool gobs_doppler(GOBS o)
    {
        const int value = static_cast<int>(o) % 1000;
        return value >= 200 && value < 300;
    }

    bool gobs_snr(GOBS o)
    {
        const int value = static_cast<int>(o) % 1000;
        return value >= 300 && value < 400;
    }

} // namespace gnut
