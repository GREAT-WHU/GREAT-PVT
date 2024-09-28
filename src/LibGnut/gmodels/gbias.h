/**
*
* @verbatim
    History
    2012-11-05  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gbias.h
* @brief      Purpose: implements GNSS code biases
*.
* @author     JD
* @version    1.0.0
* @date       2012-11-05
*
*/

#ifndef GBIAS_H
#define GBIAS_H

#include "gexport/ExportLibGnut.h"
#include <vector>

#include "newmat/newmat.h"
#include "gdata/gdata.h"
#include "gdata/gobsgnss.h"
#include "gutils/gconst.h"
#include "gutils/gtime.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_gbias. */
    class LibGnut_LIBRARY_EXPORT t_gbias : public t_gdata
    {

    public:
        /** @brief default constructor. */
        explicit t_gbias();

        explicit t_gbias(t_spdlog spdlog);
        /** @brief default destructor. */
        virtual ~t_gbias();

        /** @brief add single differential bias in meters. */
        void set(const t_gtime &beg, const t_gtime &end, double d, GOBS obs1, GOBS obs2 = X);
        void set(double d, GOBS obs1, GOBS obs2 = X);

        /** @brief get signgle differential bias. */
        double bias(bool meter = true);

        GOBS gobs() const { return _gobs; }
        double val() const { return _val; }
        GOBS ref() const { return _ref; }

        /** @brief set/get valid from. */
        void beg(const t_gtime &t) { _beg = t; }
        const t_gtime &beg() const { return _beg; }

        /** @brief set/get valid until. */
        void end(const t_gtime &t) { _end = t; }
        const t_gtime &end() const { return _end; }

        /** @brief valid. */
        bool valid(const t_gtime &epo);

    private:
        t_gtime _beg; ///< valid from
        t_gtime _end; ///< valid until
        GOBS _gobs;  ///< observation
        GOBS _ref;   ///< reference
        double _val; ///< code biases are stored in meters
    };

} // namespace

#endif
