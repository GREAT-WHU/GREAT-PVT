/**
*
* @verbatim
    History
    2011-01-10  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file      gnav.h
* @brief     implements ephemerides and navigation classes
*            overwritting mode
*            return first/last
*            limit for number of data inclusions per satellites
*.
* @author    JD
* @version   1.0.0
* @date      2011-01-10
*
*/
#ifndef GNAV_H
#define GNAV_H

#include "gdata/geph.h"
#include "gutils/gsys.h"
#include "gutils/gtime.h"

#define MAX_RINEXN_REC 29 ///< maximum number of RINEXN records for any system !!

#define MAX_NAV_TIMEDIFF 3600 * 2 ///< NAV GNS valitity interval [s]
#define MAX_GPS_TIMEDIFF 3600 * 2 ///< NAV GPS validity interval [s]
#define MAX_GLO_TIMEDIFF 60 * 17  ///< NAV GLO validity interval [s]
#define MAX_GAL_TIMEDIFF 3600 * 3 ///< NAV GAL validity interval [s]
#define MAX_BDS_TIMEDIFF 3600     ///< NAV BDS validity interval [s]
#define MAX_SBS_TIMEDIFF 360      ///< NAV SBS validity interval [s]
#define MAX_IRN_TIMEDIFF 3600 * 2 ///< NAV IRN validity interval [s]
#define MAX_QZS_TIMEDIFF 3600     ///< NAV QZS validity interval [s]

using namespace std;

namespace gnut
{

    /** @brief navdata. */
    typedef double t_gnavdata[MAX_RINEXN_REC];

    /** @brief Class for navigation data storing. */
    class LibGnut_LIBRARY_EXPORT t_gnav : public t_geph
    {

    public:
        /** @brief default constructor. */
        explicit t_gnav();
        explicit t_gnav(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gnav();

        /** @brief get GNSS NAV validity (half-interval) [s]. */
        static int nav_validity(GSYS gs);

        /** @brief convert data to nav. */
        virtual int data2nav(string sat, const t_gtime &ep, const t_gnavdata &data) { return -1; }

        /** @brief convert nav to data. */
        virtual int nav2data(t_gnavdata &data) { return -1; }

        /** @brief get iod. */
        virtual int iod() const { return -1; }

        /** @brief get rec. */
        virtual int rec() const { return MAX_RINEXN_REC; }

        /** @brief get health. */
        virtual bool healthy() const override;

        /** @brief convert health to string. */
        virtual string health_str() const override { return _health_str(); }

        /** @brief get chk. */
        virtual int chk(set<string> &msg) { return 1; }

        /** @brief get the number of frequency. */
        virtual int freq_num() const { return 255; }

    protected:
        /** @brief get the health state. */
        virtual bool _healthy() const { return true; }

        /** @brief convert health to string. */
        virtual string _health_str() const { return ""; }

    private:
    };

} // namespace

#endif
