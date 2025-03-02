/**
 * @file         gtideIERS.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        precise model for computing correction
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GTIDEIERS_H
#define GTIDEIERS_H

#include "gexport/ExportLibGREAT.h"
#include "gmodels/gtide.h"
#include "gdata/gnavde.h"
#include "gall/gallopl.h"

namespace great
{
    /** @brief The class for gtide of IERS model.*/
    class LibGREAT_LIBRARY_EXPORT t_gtideIERS : public t_gtide
    {
    public:
        /** @brief constructor.
        *
        *param[in] l                set spdlog control
        */
        t_gtideIERS();

        /** @brief constructor.
        *
        *param[in] l                set spdlog control
        */
        t_gtideIERS(t_spdlog spdlog);

        /** @brief destructor.
        *
        *param[in] l                set spdlog control
        *param[in] otl                TODO
        */
        t_gtideIERS(t_gallotl *otl);

        /** @brief destructor.
        *
        *param[in] l                set spdlog control
        *param[in] otl                TODO
        */
        t_gtideIERS(t_spdlog spdlog, t_gallotl *otl);

        /** @brief destructor.*/
        virtual ~t_gtideIERS(){};

        /** @brief solid earth tides(Mark sure the pos in J2000).*/
        t_gtriple tide_solid(const t_gtime &epo, t_gtriple &xyz, Matrix &rot_trs2crs, t_gnavde *nav_planet);
        t_gtriple tide_solid(const t_gtime &epo, t_gtriple &xyz, Matrix &rot_trs2crs, ColumnVector &sun_pos, ColumnVector &moon_pos);

        /** @brief solid earth tide corrections to be computed in the frequency domain from the diurnal band*/
        /** Reference: IERS Conventions 2010 v1.0, Eq (7.12); STEP2DIU.F from IERS 2010 Conventions software collection */
        t_gtriple tide_solid_frequency_diurnal(const t_gtime &epo, t_gtriple &xyz);

        /** @brief solid earth tide corrections to be computed in the frequency domain from the long-period band*/
        /** Reference: IERS Conventions 2010 v1.0, Eq (7.13); STEP2LON.F from IERS 2010 Conventions software collection */
        t_gtriple tide_solid_frequency_longperiod(const t_gtime &epo, t_gtriple &xyz);

        /** @brief the out-of-phase corrections induced by mantle anelasticity in the diurnal band.*/
        /** Reference: IERS Conventions 2010 v1.0, Eq (7.10) */
        /** Remeber: SIN(PHIj)=Zj/Rj; SIN(LAMBDAj)=Yj/COS(PHIj)/Rj; COS(LAMBDAj)=Xj/COS(PHIj)/Rj */
        ColumnVector _st1idiu(t_gtriple &xyz, ColumnVector &sun, ColumnVector &moon, double &FAC2SUN, double &FAC2MON);

        /** @brief the out-of-phase corrections induced by mantle anelasticity in the semi-diurnal band.*/
        /** Reference: IERS Conventions 2010 v1.0, Eq (7.11) */
        /** Remeber: SIN(PHIj)=Zj/Rj; SIN(LAMBDAj)=Yj/COS(PHIj)/Rj; COS(LAMBDAj)=Xj/COS(PHIj)/Rj */
        ColumnVector _st1isem(t_gtriple &xyz, ColumnVector &sun, ColumnVector &moon, double &FAC2SUN, double &FAC2MON);

        /** @brief the corrections induced by the latitude dependence given by L^1 in Mathews et al. 1991.*/
        /** Reference: IERS Conventions 2010 v1.0, Eq (7.7-7.9).*/
        ColumnVector _st1l1(t_gtriple &xyz, ColumnVector &sun, ColumnVector &moon, double &FAC2SUN, double &FAC2MON);

        /** @brief atmospheric tide loading.*/
        /** References: [1] IERS Conventions 2010, Sect. 7.1.3           */
        /**             [2] https://geophy.uni.lu/displacementgrids/     */
        /**             [3] /gamit/model/etide.f                         */
        t_gtriple atmospheric_loading(const t_gtime &epoch, const t_gtriple &xyz);


        /** @brief set opl grid.*/
        void set_opl_grid(t_gallopl *opl_grid);

        /** @brief load oceanpole.*/
        t_gtriple load_oceanpole(const t_gtime &epo, const t_gtriple &xRec, const double xp, const double yp);

        /** @brief set mean pole model.*/
        void set_mean_pole_model(modeofmeanpole mean_pole_model);

        /** @brief pole tides.*/
        t_gtriple tide_pole();

        /** @brief pole tides.*/
        t_gtriple tide_pole_pod(const t_gtime &epo, double xpole, double ypole, t_gtriple &xyz);

        /** @brief ocean tide loading.*/
        t_gtriple load_ocean(const t_gtime &epoch, const string &site, const t_gtriple &xRec) override;

        /** @brief atmospheric tide loading.*/
        t_gtriple load_atmosph() override;

        /** @brief get frequency of tide.*/
        t_gtriple tide_freq(const string &site, const t_gtriple &xRec, double gast);

        /** @brief get mean pole.*/
        void getMeanPole(double mjd, double &xpm, double &ypm);

    protected:
        double _EARTH_R = 6378.1366; // in km
        double _MASS_RATIO_MOON = 0.0123000371;
        double _MASS_RATIO_SUN = 332946.0482;
        // Solid tide corrections due to the frequency dependence of Love and Shida numbers in the diurnal band
        // References: IERS 2010 p107 Table 7.3a and STEP2DIU.F function from IERS 2010 software collection
        const double _solid_diurnal_tide[31][9] = {
            {-3.0, 0.0, 2.0, 0.0, 0.0, -0.01, 0.0, 0.0, 0.0},
            {-3.0, 2.0, 0.0, 0.0, 0.0, -0.01, 0.0, 0.0, 0.0},
            {-2.0, 0.0, 1.0, -1.0, 0.0, -0.02, 0.0, 0.0, 0.0},
            {-2.0, 0.0, 1.0, 0.0, 0.0, -0.08, 0.0, -0.01, 0.01},
            {-2.0, 2.0, -1.0, 0.0, 0.0, -0.02, 0.0, 0.0, 0.0},
            {-1.0, 0.0, 0.0, -1.0, 0.0, -0.10, 0.0, 0.0, 0.0},
            {-1.0, 0.0, 0.0, 0.0, 0.0, -0.51, 0.0, -0.02, 0.03},
            {-1.0, 2.0, 0.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0},
            {0.0, -2.0, 1.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0},
            {0.0, 0.0, -1.0, 0.0, 0.0, 0.02, 0.0, 0.0, 0.0},
            {0.0, 0.0, 1.0, 0.0, 0.0, 0.06, 0.0, 0.0, 0.0},
            {0.0, 0.0, 1.0, 1.0, 0.0, 0.01, 0.0, 0.0, 0.0},
            {0.0, 2.0, -1.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0},
            {1.0, -3.0, 0.0, 0.0, 1.0, -0.06, 0.0, 0.0, 0.0},
            {1.0, -2.0, 0.0, -1.0, 0.0, 0.01, 0.0, 0.0, 0.0},
            {1.0, -2.0, 0.0, 0.0, 0.0, -1.23, -0.07, 0.06, 0.01},
            {1.0, -1.0, 0.0, 0.0, -1.0, 0.02, 0.0, 0.0, 0.0},
            {1.0, -1.0, 0.0, 0.0, 1.0, 0.04, 0.0, 0.0, 0.0},
            {1.0, 0.0, 0.0, -1.0, 0.0, -0.22, 0.01, 0.01, 0.0},
            {1.0, 0.0, 0.0, 0.0, 0.0, 12.00, -0.80, -0.67, -0.03},
            {1.0, 0.0, 0.0, 1.0, 0.0, 1.73, -0.12, -0.10, 0.0},
            {1.0, 0.0, 0.0, 2.0, 0.0, -0.04, 0.0, 0.0, 0.0},
            {1.0, 1.0, 0.0, 0.0, -1.0, -0.50, -0.01, 0.03, 0.0},
            {1.0, 1.0, 0.0, 0.0, 1.0, 0.01, 0.0, 0.0, 0.0},
            {0.0, 1.0, 0.0, 1.0, -1.0, -0.01, 0.0, 0.0, 0.0},
            {1.0, 2.0, -2.0, 0.0, 0.0, -0.01, 0.0, 0.0, 0.0},
            {1.0, 2.0, 0.0, 0.0, 0.0, -0.11, 0.01, 0.01, 0.0},
            {2.0, -2.0, 1.0, 0.0, 0.0, -0.01, 0.0, 0.0, 0.0},
            {2.0, 0.0, -1.0, 0.0, 0.0, -0.02, 0.0, 0.0, 0.0},
            {3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            {3.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0}};

        // Solid tide corrections due to the frequency dependence of Love and Shida numbers in the long period band
        // References: IERS 2010 p108 Table 7.3b and STEP2LON.F function from IERS 2010 software collection
        const double _solid_long_tide[5][9] = {
            {0, 0, 0, 1, 0, 0.47, 0.23, 0.16, 0.07},
            {0, 2, 0, 0, 0, -0.20, -0.12, -0.11, -0.05},
            {1, 0, -1, 0, 0, -0.11, -0.08, -0.09, -0.04},
            {2, 0, 0, 0, 0, -0.13, -0.11, -0.15, -0.07},
            {2, 0, 0, 1, 0, -0.05, -0.05, -0.06, -0.03}};

        // atmospheric loading frequencies S1 S2
        double _atm_freq[2] = {7.27220521664304e-05, 0.000145444104332861};
        t_gallopl *_opl = nullptr;
        // mean pole model
        modeofmeanpole _mean_pole_model;
    };
} // namespace

#endif // !GTIDEPOD_H
