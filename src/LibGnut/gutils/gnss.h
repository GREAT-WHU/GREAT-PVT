
/**
* @verbatim
    History
    2012-09-26  JD: created
    2019-11-19  glfeng: add BDS-3 frequency information
                BeiDou C02(IQX) C07(7-IQX) C06(IQX) C05(5-DPX)  C09(7-DPZ)  C08(8-DPX)  C01(1-DPX)
                       B1I      B2I(BDS-2) B3I      B2a(BDS-3)  B2b(BDS-3)  B2(BDS-3)   B1C(BDS-3)

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gfile.h
* @brief       Purpose: definition of GNSS data
*.
* @author      JD
* @version     1.0.0
* @date        2012-09-26
*
*/

#ifndef GNSS_H
#define GNSS_H
#include <map>
#include <set>
#include <vector>
#include <string>

#include "gutils/gtriple.h"
#include "gutils/gcommon.h" 
#include "gexport/ExportLibGnut.h"
using namespace std;

namespace gnut
{
    /** @brief GNSS systems and augmentations. */
    enum GSYS
    { // GXX = -1,
        GPS = 1,
        GAL = 2,
        GLO = 3,
        BDS = 4,
        QZS = 5,
        SBS = 6,
        IRN = 7,
        GNS = 999
    };

    /** @brief GNSS freq Sequence ID. */
    enum FREQ_SEQ
    {
        FREQ_1 = 1,
        FREQ_2 = 2,
        FREQ_3 = 3,
        FREQ_4 = 4,
        FREQ_5 = 5,
        FREQ_6 = 6,
        FREQ_7 = 7,
        FREQ_X = 999
    };

    /** @brief GNSS frequencies. */
    enum GFRQ
    { // FXX = -1,
        G01 = 10,
        G02 = 11,
        G05 = 12, // GPS
        R01 = 20,
        R02 = 21, // GLONASS FDMA
        R01_CDMA = 30,
        R02_CDMA = 31,
        R03_CDMA = 32,
        R05_CDMA = 33, // GLONASS CDMA
        E01 = 50,
        E05 = 51,
        E07 = 52,
        E08 = 53,
        E06 = 54, // Galileo
        // BeiDou C02(IQX) C07(7-IQX) C06(IQX) C05(5-DPX)  C09(7-DPZ)  C08(8-DPX)  C01(1-DPX)
        //        B1I      B2I(BDS-2) B3I      B2a(BDS-3)  B2b(BDS-3)  B2(BDS-3)   B1C(BDS-3)
        C02 = 60,
        C07 = 61,
        C06 = 62,
        C05 = 63,
        C09 = 64,
        C08 = 65,
        C01 = 66,
        J01 = 70,
        J02 = 71,
        J05 = 72,
        J06 = 73, // QZSS
        S01 = 80,
        S05 = 81, // SBAS
        I05 = 90, // I09,                // IRNSS
        LAST_GFRQ = 999
    };

    /** @brief GNSS receiver types. */
    enum RECTYPE
    {
        P1P2, // receiver providing C1, P1, P2
        C1X2, // cross-correlation
        C1P2  // modern receivers providing C1, P2
    };

    /** @brief Broadcast messages types. */
    enum GNAVTYPE
    {
        FNAV,
        INAV,
        INAV_E01,
        INAV_E07,
        CNAV,
        NAV
    };

    /** @brief GNSS type/band/attr definitions. */
    enum GOBSTYPE
    {
        TYPE_C = 1,
        TYPE_L = 2,
        TYPE_D = 3,
        TYPE_S = 4,
        TYPE_P = 101, // only for P-code!
        TYPE_SLR,
        TYPE_KBRRANGE,
        TYPE_LRIRANGE,
        TYPE_KBRRATE,
        TYPE_LRIRATE,
        TYPE = 999 // ""  UNKNOWN
    };
    enum GOBSBAND
    {
        BAND_1 = 1,
        BAND_2 = 2,
        BAND_3 = 3,
        BAND_5 = 5,
        BAND_6 = 6,
        BAND_7 = 7,
        BAND_8 = 8,
        BAND_9 = 9, // Band_9 is set for BDS-3 B2b
        BAND_A = 101,
        BAND_B = 102,
        BAND_C = 103,
        BAND_D = 104,
        BAND_SLR,
        BAND_KBR,
        BAND_LRI,
        BAND = 999 // ""  UNKNOWN
    };
    enum GOBSATTR
    {
        ATTR_A = 1,
        ATTR_B = 2,
        ATTR_C = 3,
        ATTR_D = 4,
        ATTR_I = 5,
        ATTR_L = 6,
        ATTR_M = 7,
        ATTR_N = 8,
        ATTR_P = 9,
        ATTR_Q = 10,
        ATTR_S = 11,
        ATTR_W = 12,
        ATTR_X = 13,
        ATTR_Y = 14,
        ATTR_Z = 15,
        ATTR_NULL = 16, // " " 2CHAR code
        ATTR = 999 // ""  UNKNOWN
    };

    /** @brief GNSS observations. */
    enum GOBS
    {

        // psedorange [in meters] (RINEX 3.x)
        C1A = 0,
        C1B = 1,
        C1C = 2,
        C1D = 3,
        C1I = 4,
        C1L = 5,
        C1M = 6,
        C1P = 8,
        C1S = 9,
        C1Q = 10,
        C1W = 11,
        C1X = 12,
        C1Y = 13,
        C1Z = 14,
        C2C = 15,
        C2D = 16,
        C2I = 17,
        C2L = 18,
        C2M = 19,
        C2P = 21,
        C2S = 22,
        C2Q = 23,
        C2W = 24,
        C2X = 25,
        C2Y = 26,
        C3I = 27,
        C3Q = 28,
        C3X = 29,
        C5A = 30,
        C5B = 31,
        C5C = 32,
        C5D = 33,
        C5I = 34,
        C5P = 35,
        C5Q = 36,
        C5X = 37,
        C6A = 38,
        C6B = 39,
        C6C = 40,
        C6I = 41,
        C6L = 42,
        C6S = 43,
        C6Q = 44,
        C6X = 45,
        C6Z = 46,
        C7I = 47,
        C7Q = 48,
        C7X = 50,
        C8D = 51,
        C8I = 52,
        C8P = 53,
        C8Q = 54,
        C8X = 55,
        C9D = 56,
        C9P = 57,
        C9Z = 58, // BDS-3 B2b

        // carrier phase [in whole cycles] (RINEX 3.x)
        L1A = 100,
        L1B = 101,
        L1C = 102,
        L1D = 103,
        L1I = 104,
        L1L = 105,
        L1M = 106,
        L1N = 107,
        L1P = 108,
        L1S = 109,
        L1Q = 110,
        L1W = 111,
        L1X = 112,
        L1Y = 113,
        L1Z = 114,
        L2C = 115,
        L2D = 116,
        L2I = 117,
        L2L = 118,
        L2M = 119,
        L2N = 120,
        L2P = 121,
        L2S = 122,
        L2Q = 123,
        L2W = 124,
        L2X = 125,
        L2Y = 126,
        L3I = 127,
        L3Q = 128,
        L3X = 129,
        L5A = 130,
        L5B = 131,
        L5C = 132,
        L5D = 133,
        L5I = 134,
        L5P = 135,
        L5Q = 136,
        L5X = 137,
        L6A = 138,
        L6B = 139,
        L6C = 140,
        L6I = 141,
        L6L = 142,
        L6S = 143,
        L6Q = 144,
        L6X = 145,
        L6Z = 146,
        L7I = 147,
        L7Q = 148,
        L7X = 150,
        L8D = 151,
        L8I = 152,
        L8P = 153,
        L8Q = 154,
        L8X = 155,
        L9D = 156,
        L9P = 157,
        L9Z = 158, // BDS-3 B2b

        // doppler [cycles/sec] (RINEX 3.x)
        D1A = 200,
        D1B = 201,
        D1C = 202,
        D1D = 203,
        D1I = 204,
        D1L = 205,
        D1M = 206,
        D1N = 207,
        D1P = 208,
        D1S = 209,
        D1Q = 210,
        D1W = 211,
        D1X = 212,
        D1Y = 213,
        D1Z = 214,
        D2C = 215,
        D2D = 216,
        D2I = 217,
        D2L = 218,
        D2M = 219,
        D2N = 220,
        D2P = 221,
        D2S = 222,
        D2Q = 223,
        D2W = 224,
        D2X = 225,
        D2Y = 226,
        D3I = 227,
        D3Q = 228,
        D3X = 229,
        D5A = 230,
        D5B = 231,
        D5C = 232,
        D5D = 233,
        D5I = 234,
        D5P = 235,
        D5Q = 236,
        D5X = 237,
        D6A = 238,
        D6B = 239,
        D6C = 240,
        D6I = 241,
        D6L = 242,
        D6S = 243,
        D6Q = 244,
        D6X = 245,
        D6Z = 246,
        D7I = 247,
        D7Q = 248,
        D7X = 250,
        D8D = 251,
        D8I = 252,
        D8P = 253,
        D8Q = 254,
        D8X = 255,
        D9D = 256,
        D9P = 257,
        D9Z = 258, // BDS-3 B2b

        // signal strength [DBHZ] (RINEX 3.x)
        S1A = 300,
        S1B = 301,
        S1C = 302,
        S1D = 303,
        S1I = 304,
        S1L = 305,
        S1M = 306,
        S1N = 307,
        S1P = 308,
        S1S = 309,
        S1Q = 310,
        S1W = 311,
        S1X = 312,
        S1Y = 313,
        S1Z = 314,
        S2C = 315,
        S2D = 316,
        S2I = 317,
        S2L = 318,
        S2M = 319,
        S2N = 320,
        S2P = 321,
        S2S = 322,
        S2Q = 323,
        S2W = 324,
        S2X = 325,
        S2Y = 326,
        S3I = 327,
        S3Q = 328,
        S3X = 329,
        S5A = 330,
        S5B = 331,
        S5C = 332,
        S5D = 333,
        S5I = 334,
        S5P = 335,
        S5Q = 336,
        S5X = 337,
        S6A = 338,
        S6B = 339,
        S6C = 340,
        S6I = 341,
        S6L = 342,
        S6S = 343,
        S6Q = 344,
        S6X = 345,
        S6Z = 346,
        S7I = 347,
        S7Q = 348,
        S7X = 350,
        S8D = 351,
        S8I = 352,
        S8P = 353,
        S8Q = 354,
        S8X = 355,
        S9D = 356,
        S9P = 357,
        S9Z = 358, // BDS-3 B2b

        // special cases: v2.x or unknown tracking modes
        P1 = 1000,
        P2 = 1001,
        P5 = 1002,
        C1 = 1003,
        C2 = 1004,
        C5 = 1005,
        C6 = 1006,
        C7 = 1007,
        C8 = 1008,
        CA = 1009,
        CB = 1010,
        CC = 1011,
        CD = 1012,

        L1 = 1100,
        L2,
        L5,
        L6,
        L7,
        L8,
        LA,
        LB,
        LC,
        LD,

        D1 = 1200,
        D2,
        D5,
        D6,
        D7,
        D8,
        DA,
        DB,
        DC,
        DD,

        S1 = 1300,
        S2,
        S5,
        S6,
        S7,
        S8,
        SA,
        SB,
        SC,
        SD,

        X = 9999 // LAST_GOBS
    };

    /** @brief GOBS LC. */
    enum GOBS_LC
    {
        LC_L1 = 1,
        LC_L2 = 2,
        LC_L3 = 3,
        LC_L4 = 4,
        LC_L5 = 5,
        LC_IF = 6,
        LC_MW = 7,
        LC_NL = 8,
        LC_WL = 9,
        LC_GF = 10,
        LC_UNDEF = 999
    };

    /** @brief TYPEDEF. */
    typedef vector<GOBSATTR> t_vec_attr;          ///< attr vector
    typedef vector<GOBSBAND> t_vec_band;          ///< band vector
    typedef vector<GFRQ> t_vec_freq;              ///< frequency vector
    typedef map<GOBSTYPE, t_vec_attr> t_map_attr; ///< attr map
    typedef map<GOBSBAND, t_map_attr> t_map_type; ///< type map
    typedef map<GSYS, set<string>> t_map_sats;    ///< sats map
    typedef map<GSYS, set<string>> t_map_gnav;    ///< nav map
    typedef map<GSYS, t_map_type> t_map_gnss;     ///< gnss map
    typedef map<GSYS, t_vec_band> t_map_band;     ///< band map
    typedef map<GSYS, t_vec_freq> t_map_freq;     ///< frequency mao

    typedef map<GOBSBAND, t_gtriple> t_map_pcos; ///< triple: ATX  NORTH / EAST / UP
    typedef map<GSYS, t_map_pcos> t_map_offs;

    /** @brief GLOBAL FUNCTIONS. */
    LibGnut_LIBRARY_EXPORT GOBSATTR str2gobsattr(string s); ///< get GOBSATTR enum from gobs string
    LibGnut_LIBRARY_EXPORT GOBSBAND str2gobsband(string s); ///< get GOBSBAND enum from gobs string
    LibGnut_LIBRARY_EXPORT GOBSTYPE str2gobstype(string s); ///< get GOBSTYPE enum from gobs string
    LibGnut_LIBRARY_EXPORT GNAVTYPE str2gnavtype(string s); ///< get GNAVTYPE enum from gobs string
    LibGnut_LIBRARY_EXPORT FREQ_SEQ str2sysfreq(string s);  ///< get FREQ_SEQ enum from string 
    LibGnut_LIBRARY_EXPORT FREQ_SEQ str2gnssfreq(string s); ///< get FREQ_SEQ enum from gobs string

    LibGnut_LIBRARY_EXPORT GOBSATTR char2gobsattr(char c); ///< get GOBSATTR enum from char
    LibGnut_LIBRARY_EXPORT GOBSBAND char2gobsband(char c); ///< get GOBSBAND enum from char
    LibGnut_LIBRARY_EXPORT GOBSBAND int2gobsband(int c);   ///< get GOBSBAND enum from char
    LibGnut_LIBRARY_EXPORT GOBSTYPE char2gobstype(char c); ///< get GOBSTYPE enum from char
    LibGnut_LIBRARY_EXPORT FREQ_SEQ char2gnssfreq(char c); ///< get FREQ_SEQ enum from char
    LibGnut_LIBRARY_EXPORT GOBS_LC int2gobsfreq(int c);    ///< get GOBS_LC enum from char

    LibGnut_LIBRARY_EXPORT string gobsattr2str(GOBSATTR e); ///< get string enum from GOBSATTR
    LibGnut_LIBRARY_EXPORT string gobsband2str(GOBSBAND e); ///< get string enum from GOBSBAND
    LibGnut_LIBRARY_EXPORT string gobstype2str(GOBSTYPE e); ///< get string enum from GOBSTYPE
    LibGnut_LIBRARY_EXPORT string gnavtype2str(GNAVTYPE e); ///< get string enum from GNAVTYPE

    LibGnut_LIBRARY_EXPORT string gobs2str(GOBS);                             ///< get string from GOBS enum
    LibGnut_LIBRARY_EXPORT GOBS str2gobs(string s);                           ///< get GOBS enum from string
    LibGnut_LIBRARY_EXPORT GOBS tba2gobs(GOBSTYPE t, GOBSBAND b, GOBSATTR a); ///< get GOBS from type, band, and attribute
    LibGnut_LIBRARY_EXPORT string gfreqseq2str(FREQ_SEQ f);                   ///< convert FREQ_SEQ to string

    LibGnut_LIBRARY_EXPORT int gobs2band(GOBS o); ///< get band from GOBS enum

    LibGnut_LIBRARY_EXPORT GOBS pha2snr(GOBS o);        ///  get GOBS enum (pha->snr)
    LibGnut_LIBRARY_EXPORT GOBS pl2snr(GOBS o);         ///  get GOBS enum (pha or code->snr) add wh
    LibGnut_LIBRARY_EXPORT bool gobs_code(GOBS o);      ///< get true for code obs
    LibGnut_LIBRARY_EXPORT bool gobs_phase(GOBS o);     ///< get true for phase obs
    LibGnut_LIBRARY_EXPORT bool gobs_doppler(GOBS o);   ///< get true for doppler obs
    LibGnut_LIBRARY_EXPORT bool gobs_snr(GOBS o);       ///< get true for snr obs

    LibGnut_LIBRARY_EXPORT t_map_sats GNSS_SATS();      ///< static map of default GNSS satellites
    LibGnut_LIBRARY_EXPORT t_map_gnav GNSS_GNAV();      ///< static map of default GNSS navigation types

    LibGnut_LIBRARY_EXPORT t_map_gnss GNSS_DATA_PRIORITY(); ///< static map of default GNSS data types/bands/attrs priorities
    LibGnut_LIBRARY_EXPORT t_map_band GNSS_BAND_SORTED();                             ///< static map of sorted GNSS band w.r.t. wavelength

    LibGnut_LIBRARY_EXPORT vector<GOBSBAND> sort_band(GSYS gs, set<GOBSBAND> &bands); // sort set of bands w.r.t. wavelength

    LibGnut_LIBRARY_EXPORT t_map_offs GNSS_PCO_OFFSETS(); ///< static map of default GNSS PCO offsets
    LibGnut_LIBRARY_EXPORT set<GSYS> GNSS_SUPPORTED(); ///< supported GNSS

    /** @brief GNSS frequency. */
    const t_map_freq GNSS_FREQ_PRIORITY = 
    {
        {GPS, {LAST_GFRQ, G01, G02, G05}},
        {GLO, {LAST_GFRQ, R01, R02, R03_CDMA, R05_CDMA}},
        {GAL, {LAST_GFRQ, E01, E05, E07, E08, E06}},
        {BDS, {LAST_GFRQ, C02, C07, C06, C05, C09, C08, C01}},
        {QZS, {LAST_GFRQ, J01, J02, J05, J06}},
        {SBS, {LAST_GFRQ, S01, S05}},
        {GNS, {}},
    }; // static map of default GNSS freq priorities

    /** @brief GNSS band. */
    const t_map_band GNSS_BAND_PRIORITY = 
    {
        {GPS, {BAND, BAND_1, BAND_2, BAND_5}},
        {GLO, {BAND, BAND_1, BAND_2, BAND_3, BAND_5}},
        {GAL, {BAND, BAND_1, BAND_5, BAND_7, BAND_8, BAND_6}},
        {BDS, {BAND, BAND_2, BAND_7, BAND_6, BAND_5, BAND_9, BAND_8, BAND_1}},
        {QZS, {BAND, BAND_1, BAND_2, BAND_5, BAND_6}},
        {SBS, {BAND, BAND_1, BAND_5}},
        {GNS, {}},
    }; // static map of default GNSS band priorities

} // namespace

#endif // GOBS_H
