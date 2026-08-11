
/**
* @verbatim
    History
    2012-09-26  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gsys.h
* @brief       Purpose: definition of GNSS constellations and observation frequencies
*.
* @author     JD
* @version    1.0.0
* @date       2012-09-26
*
*/

#ifndef GSYS_H
#define GSYS_H

#include <set>
#include <map>
#include <vector>
#include <string>
#ifdef BMUTEX
#include <boost/std::thread/std::mutex.hpp>
#endif

#include "gutils/gnss.h"
#include "gutils/gobs.h"
#include "gutils/gconst.h"
#include "gutils/gmutex.h"
#include "gexport/ExportLibGnut.h"


namespace gnut
{

#ifndef SBS_OFFSET
#define SBS_OFFSET (100) ///< offset for SBAS satellites (RINEX3)
#endif
#ifndef QZS_OFFSET
#define QZS_OFFSET (192) ///< offset for QZSS satellites (RINEX3)
#endif

///< GNSS base frequencies [Hz]
#ifndef GPS_FRQ
#define GPS_FRQ 10230000.0      ///< GPS     base frequency
#endif
#ifndef GLO_FRQ
#define GLO_FRQ 178000000.0     ///< GLONASS base frequency (FDMA)
#endif
#ifndef GLO_FRQ_CDMA
#define GLO_FRQ_CDMA 10230000.0 ///< GLONASS base frequency (CDMA)
#endif
#ifndef GAL_FRQ
#define GAL_FRQ 10230000.0      ///< GALILEO base frequency
#endif
#ifndef BDS_FRQ
#define BDS_FRQ 10230000.0      ///< BEIDOU  base frequency
#endif
#ifndef QZS_FRQ
#define QZS_FRQ 10230000.0      ///< QZSS    base frequency
#endif
#ifndef SBS_FRQ
#define SBS_FRQ 10230000.0      ///< SBAS    base frequency
#endif
#ifndef IRN_FRQ
#define IRN_FRQ 10230000.0      ///< IRNSS   base frequency
#endif

///< GNSS frequency multiplier factors (dimensionless)
#ifndef G01_MLT
#define G01_MLT 154.0
#endif
#ifndef G02_MLT
#define G02_MLT 120.0
#endif
#ifndef G05_MLT
#define G05_MLT 115.0
#endif
#ifndef R01_MLT
#define R01_MLT 9.0        ///< FDMA                                    1602.0MHz
#endif
#ifndef R02_MLT
#define R02_MLT 7.0        ///< FDMA                                    1246.0MHz
#endif
#ifndef R01_MLT_CDMA
#define R01_MLT_CDMA 9.0   ///< CDMA                  156.5 * GPS_FRQ = 1600.995 MHz
#endif
#ifndef R02_MLT_CDMA
#define R02_MLT_CDMA 7.0   ///< CDMA                  122.0 * GPS_FRQ = 1248.06  MHz
#endif
#ifndef R03_MLT_CDMA
#define R03_MLT_CDMA 117.5 ///< CDMA                  117.5 * GPS_FRQ = 1202.25  MHz L3OC
#endif
#ifndef R05_MLT_CDMA
#define R05_MLT_CDMA 115.5 ///< CDMA                  115.0 * GPS_FRQ = 1176.45  MHz L5OCM
#endif
#ifndef R01_STEP
#define R01_STEP 562500.0
#endif
#ifndef R02_STEP
#define R02_STEP 437500.0
#endif
#ifndef E01_MLT
#define E01_MLT 154.0
#endif
#ifndef E05_MLT
#define E05_MLT 115.0
#endif
#ifndef E06_MLT
#define E06_MLT 125.0
#endif
#ifndef E07_MLT
#define E07_MLT 118.0
#endif
#ifndef E08_MLT
#define E08_MLT 116.5
#endif
#ifndef C02_MLT
#define C02_MLT 152.6 ///< B1I BeiDou-2
#endif
#ifndef C06_MLT
#define C06_MLT 124.0 ///< B3I BeiDou-2/3
#endif
#ifndef C07_MLT
#define C07_MLT 118.0 ///< B2I BeiDou-2
#endif
#ifndef C01_MLT
#define C01_MLT 154.0 ///< B1C BeiDou-3
#endif
#ifndef C08_MLT
#define C08_MLT 116.5 ///< B2  BeiDou-3
#endif
#ifndef C05_MLT
#define C05_MLT 115.0 ///< B2a BeiDou-3
#endif
#ifndef C09_MLT
#define C09_MLT 118.0 ///< B2b BeiDou-3
#endif

#ifndef J01_MLT
#define J01_MLT 154.0
#endif
#ifndef J02_MLT
#define J02_MLT 120.0
#endif
#ifndef J05_MLT
#define J05_MLT 115.0
#endif
#ifndef J06_MLT
#define J06_MLT 125.0
#endif
#ifndef S01_MLT
#define S01_MLT 154.0
#endif
#ifndef S05_MLT
#define S05_MLT 115.0
#endif
#ifndef I05_MLT
#define I05_MLT 115.0
#endif

///< GNSS derived frequencies [Hz]
#ifndef G01_F
#define G01_F G01_MLT* GPS_FRQ ///<  L1    - GPS NAVSTAR
#endif
#ifndef G02_F
#define G02_F G02_MLT* GPS_FRQ ///<  L2    - GPS NAVSTAR
#endif
#ifndef G05_F
#define G05_F G05_MLT* GPS_FRQ ///<  L5    - GPS NAVSTAR
#endif

#ifndef R01_F
#define R01_F(a) R01_MLT* GLO_FRQ + R01_STEP*(a) ///<  G1    - GLONASS (FDMA) base frequency  L1 = 1602 MHz + (n x 0.5625) MHz
#endif
#ifndef R02_F
#define R02_F(a) R02_MLT* GLO_FRQ + R02_STEP*(a) ///<  G2    - GLONASS (FDMA) base frequency  L2 = 1246 MHz + (n x 0.4375) MHz
#endif
#ifndef R01_F_CDMA
#define R01_F_CDMA R03_MLT_CDMA* GLO_FRQ_CDMA    ///<  G1    - GLONASS (CDMA)
#endif
#ifndef R02_F_CDMA
#define R02_F_CDMA R05_MLT_CDMA* GLO_FRQ_CDMA    ///<  G2    - GLONASS (CDMA)
#endif
#ifndef R03_F_CDMA
#define R03_F_CDMA R03_MLT_CDMA* GLO_FRQ_CDMA    ///<  G3    - GLONASS (CDMA)
#endif
#ifndef R05_F_CDMA
#define R05_F_CDMA R05_MLT_CDMA* GLO_FRQ_CDMA    ///<  G5    - GLONASS (CDMA)
#endif

#ifndef E01_F
#define E01_F E01_MLT* GAL_FRQ ///<  E1    - Galileo
#endif
#ifndef E05_F
#define E05_F E05_MLT* GAL_FRQ ///<  E5a   - Galileo
#endif
#ifndef E06_F
#define E06_F E06_MLT* GAL_FRQ ///<  E6    - Galileo
#endif
#ifndef E07_F
#define E07_F E07_MLT* GAL_FRQ ///<  E5b   - Galileo
#endif
#ifndef E08_F
#define E08_F E08_MLT* GAL_FRQ ///<  E5a+b - Galileo
#endif

#ifndef C02_F
#define C02_F C02_MLT* BDS_FRQ ///< B1I BeiDou-2
#endif
#ifndef C06_F
#define C06_F C06_MLT* BDS_FRQ ///< B3I BeiDou-2/3
#endif
#ifndef C07_F
#define C07_F C07_MLT* BDS_FRQ ///< B2I BeiDou-2
#endif
#ifndef C01_F
#define C01_F C01_MLT* BDS_FRQ ///< glfeng B1C BeiDou-3
#endif
#ifndef C08_F
#define C08_F C08_MLT* BDS_FRQ ///< glfeng B2  BeiDou-3
#endif
#ifndef C05_F
#define C05_F C05_MLT* BDS_FRQ ///< glfeng B2a BeiDou-3
#endif
#ifndef C09_F
#define C09_F C09_MLT* BDS_FRQ ///< glfeng B2b BeiDou-3
#endif

#ifndef J01_F
#define J01_F J01_MLT* QZS_FRQ ///<  L1    - QZSS
#endif
#ifndef J02_F
#define J02_F J02_MLT* QZS_FRQ ///<  L2    - QZSS
#endif
#ifndef J05_F
#define J05_F J05_MLT* QZS_FRQ ///<  L5    - QZSS
#endif
#ifndef J06_F
#define J06_F J06_MLT* QZS_FRQ ///<  LEX   - QZSS
#endif

#ifndef S01_F
#define S01_F S01_MLT* SBS_FRQ ///<  L1    - SBAS
#endif
#ifndef S05_F
#define S05_F S05_MLT* SBS_FRQ ///<  L5    - SBAS
#endif

#ifndef I05_F
#define I05_F I05_MLT* IRN_FRQ ///<  I5    - IRNSS
#endif

///< OBSOLETE INFORMATIONS -- NOT YET !?
#ifndef GPSLAMB1
#define GPSLAMB1 CLIGHT / (G01_F) ///< GPS L1 wave length [m]
#endif
#ifndef GPSLAMB2
#define GPSLAMB2 CLIGHT / (G02_F) ///< GPS L2 wave length [m]
#endif
#ifndef GPSLAMB3
#define GPSLAMB3                                                                                                                           \
    (((G01_F * G01_F) / (G01_F * G01_F - G02_F * G02_F)) * GPSLAMB1 -                                                                      \
     ((G02_F * G02_F) / (G01_F * G01_F - G02_F * G02_F)) * GPSLAMB2) ///< GPS ionospher-free wave lenght [m]
#endif
#ifndef LAMBWL
#define LAMBWL CLIGHT / (G01_F - G02_F)                              ///< GPS wide-lane wave length [m]
#endif

    /** @brief class for t_gsys. */
    class LibGnut_LIBRARY_EXPORT t_gsys
    {
    public:
        /** @brief constructor 1. */
        explicit t_gsys(const GSYS& sys);

        /** @brief constructor 2. */
        explicit t_gsys(const std::string& sys);

        /** @brief constructor 3. */
        explicit t_gsys(const char& c);

        /** @brief default destructor. */
        virtual ~t_gsys() {};

        /**@brief get freq priority  */
        static GFRQ freq_priority(const GSYS& gs, const FREQ_SEQ& iseq);

        /**@brief get obs priority  */
        static GOBS gobs_priority(const GSYS& gs, const GOBS& gobs);
        static GOBS gobs_priority(const GSYS& gs, const GOBSBAND& band);

        /**@brief get obs defaults  */
        static GOBS gobs_defaults(const GSYS& gs, const GOBS& gobs1, const GOBSBAND& band);

        /**@brief get band priority  */
        static GOBSBAND band_priority(const GSYS& gs, const FREQ_SEQ& iseq);

        /**@brief get attr priority  */
        static GOBSATTR attr_priority(const GSYS& gs, const GOBSBAND& gb, const GOBSTYPE& gt, const unsigned int& iseq);

        /**@brief convert band (RINEX 3) to freq enum  */
        static GFRQ band2gfrq(const GSYS& gs, const GOBSBAND& band);

        /**@brief convert band (RINEX 3) to freq sequence  */
        static FREQ_SEQ band2freq(const GSYS& gs, const GOBSBAND& band);

        /**@brief convert freq enum to band (RINEX 3)  */
        static GOBSBAND gfrq2band(const GSYS& gs, const GFRQ& freq);

        /**@brief convert freq enum to frequency sequence  */
        static FREQ_SEQ gfrq2freq(const GSYS& gs, const GFRQ& freq);

        /**@brief convert freq enum to gsys  */
        static GSYS gfrq2gsys(const GFRQ& freq);

        /**@brief convert str to frequency  */
        static FREQ_SEQ str2freq(const std::string& freq);

        /**@brief convert frequency to str  */
        static std::string freq2str(const FREQ_SEQ& freq);

        /**@brief convert GSYS enum to GSYS string  */
        static std::string gsys2str(const GSYS& sys);

        /**@brief convert GSYS enum to GSYS char  */
        static char gsys2char(const GSYS& sys);

        /**@brief convert GSYS string to GSYS enum  */
        static GSYS str2gsys(const std::string& s);

        /**@brief convert sat string to GSYS enum  */
        static GSYS sat2gsys(const std::string& s);

        /**@brief convert GSYS string to GSYS char  */
        static char str2char(const std::string& s);

        /**@brief convert GSYS string to first character of GSYS string  */
        static std::string str2strfirst(const std::string& s);

        /**@brief convert GSYS char to GSYS string  */
        static std::string char2str(char c);

        /**@brief convert GSYS char to GSYS enum  */
        static GSYS char2gsys(char c);

        /**@brief convert full sat string  */
        static std::string eval_sat(const std::string& sat, const GSYS& sys = GSYS::GPS);

        /**@brief convert full sat string  */
        static std::string eval_sat(const int& svn, const GSYS& sys = GSYS::GPS);

        /**@brief get BDS GEO satellite  */
        static bool bds_geo(GSAT sat);
        static bool bds_geo(const std::string& sat);

        /**@brief get BDS GEO satellite  */
        static bool bds_cast(GSAT sat);
        static bool bds_cast(const std::string& sat);

        /**@brief get BDS GEO satellite  */
        static bool bds_secm(GSAT sat);
        static bool bds_secm(const std::string& sat);

        /**@brief set GSYS from string  */
        void from_string(const std::string& sys);

        /**@brief set GSYS from enum  */
        void from_gsys(const GSYS& sys);

        /**@brief get GSYS enum  */
        GSYS gsys() const
        {
            return _gsys;
        }

        /**@brief get GSYS string  */
        std::string str() const
        {
            return gsys2str(_gsys);
        }

        /**@brief overloaded equivalence operator  */
        bool operator==(const std::string& sys) const;

        /**@brief overloaded equivalence operator  */
        bool operator==(const GSYS& sys) const;

    protected:
    private:
        GSYS _gsys;       ///< system
        t_gmutex _gmutex; ///< mutex
    };

    ///<** @brief class for t_gfreq. */
    class t_gfreq
    {
    public:
        /** @brief constructor 1. */
        t_gfreq()
        {
            _gfreq = GFRQ::LAST_GFRQ;
        };

        /** @brief default destructor. */
        ~t_gfreq() {};

        /** @brief get GFRQ enum from string. */
        static GFRQ str2gfreq(const std::string& freq);

        /** @brief get string from GFRQ enum. */
        static std::string gfreq2str(const GFRQ& freq);

    private:
        GFRQ _gfreq;
    };

} // namespace gnut

#endif ///< GFREQ_H
