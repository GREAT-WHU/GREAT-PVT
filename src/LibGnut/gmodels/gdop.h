
/**
*
* @verbatim
    History
    2014-01-21  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gdop.h
* @brief      Purpose: dilution of precesion - gdop, pdop, hdop, vdop, tdop
*.
* @author     PV
* @version    1.0.0
* @date       2014-01-21
*
*/

#ifndef GDOP_H
#define GDOP_H

#include <iostream>
#include <set>

#include "newmat/newmat.h"
#include "gall/gallnav.h"
#include "gall/gallobs.h"
#include "gutils/gtriple.h"


namespace gnut
{

    /** @brief class for t_gdop. */
    class LibGnut_LIBRARY_EXPORT t_gdop
    {
    public:
        /** @brief default constructor. */
        explicit t_gdop();

        /** @brief constructor 1. */
        explicit t_gdop(t_gallnav* gnav, t_gallobs* gobs, std::string site);

        /** @brief constructor 2. */
        explicit t_gdop(t_gallnav* gnav, std::set<GSAT> sats);

        /** @brief default destructor. */
        ~t_gdop();

        /** @brief Calculate dop - _Qx. */
        int calculate(const t_gtime& epoch, t_gtriple& rec, GSYS gnss = GSYS::GNS);

        /** @brief Position dilution of precision. */
        double pdop();

        /** @brief Geom dilution of precision. */
        double gdop();

        /** @brief Time dilution of precision. */
        double tdop();

        /** @brief Horizontal dilution of precision. */
        double hdop();

        /** @brief Vertical dilution of precision. */
        double vdop();

        /** @brief set nav, obs, site. */
        void set_data(t_gallnav* gnav, t_gallobs* gobs, std::string site);

        /** @brief set log. */
        /** @brief set satellite list for calculation. */
        void set_sats(std::set<GSAT>& sats);

    private:
        std::string _site;        ///< site name
        t_gallnav* _gnav;    ///< ephemerides
        t_gallobs* _gobs;    ///< observations
        std::set<GSAT> _sats;     ///< set of visible satellites
        t_gtriple _rec;      ///< receiver position
        SymmetricMatrix _Qx; ///< variance-covariance matrix
    };
} // namespace gnut

#endif
