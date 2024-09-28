/**
 * @file         gnavde.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        The base class for save JPL DE file information.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GNAVDE_H
#define GNAVDE_H

#include "gexport/ExportLibGREAT.h"
#include "gdata/gdata.h"
#include "gutils/gtime.h"
#include "gutils/gtriple.h"
#include "newmat/newmat.h"

using namespace gnut;

namespace great
{
    /** @brief PLANET considered */
    enum PLANET
    {
        PLANET_MERC = 0,   ///< Mercury.
        PLANET_VENU = 1,   ///< Venus.
        PLANET_EART = 2,   ///< Earth.
        PLANET_MARS = 3,   ///< Mars.
        PLANET_JUPI = 4,   ///< Jupiter.
        PLANET_SATU = 5,   ///< Saturn.
        PLANET_URAN = 6,   ///< Uranus.
        PLANET_NEPT = 7,   ///< Neptune.
        PLANET_PLUT = 8,   ///< Pluto.
        PLANET_MOON = 9,   ///< Moon.
        PLANET_SUN = 10,   ///< Sun.
        SOLAR_SYSTEM = 11, ///< Solar_system.
        EARTH_MOON = 12,   ///< Earth-Moon system.
        PLANET_UNDEF = 13  ///< undefined planet.
    };

    /**
    *@brief       Class for save one planet information from DE file
    *
    * The class contains GM, radius, position of coefficient in DE data of one planet
    */
    class t_gplanet
    {
    public:
        /** @brief default constructor. */
        t_gplanet();

        /** @brief default destructor. */
        virtual ~t_gplanet(){};

        double gm = 0.0;  ///< GM
        double rad = 0.0; ///< radius;
        int ipt = 0;      ///< position of coeff
        int ncf = 0;      ///< # of coeeficients for component
        int na = 0;       ///< # of sets of coeficients in full array
    };

    /**
    *@brief       Class for save DE data mainly include Chebychev coefficient
    */
    class LibGREAT_LIBRARY_EXPORT t_gnavde : public t_gdata
    {
    public:
        /** @brief default constructor. */
        t_gnavde();

        t_gnavde(t_spdlog spdlog);
        /** @brief default destructor. */
        virtual ~t_gnavde();

        /**
        * @brief add JPL header data.
        * @param[in]   SS            begin time,end time and interval of the JPL data
        * @param[in]   au            value of astronomical unit
        * @param[in]   emrat        Earth-Moon mass ratio
        * @param[in]   ipt            position of coeff and ncf and na data
        * @param[in]   constname    constants' name
        * @param[in]   constval        constants' value
        */
        void add_head(double SS[3], const double &au, const double &emrat, int ipt[3][13], const vector<string> &constname, const vector<double> &constval);

        /**
        * @brief add JPL body data.
        * @param[in]   index        chebyshev coefficient's index
        * @param[in]   coeff        value of chebyshev coefficient
        */
        void add_data(const int &index, const vector<double> &coeff); 

        /**
        * @brief get position of planet in crs(J2000), earth is center body.
        * @param[in]   tm             dynamic time(mjd)
        * @param[in]   planet_name   planet's name
        * @param[out]  pos             position of planet(t_gtriple)
        */
        void get_pos(const double &tm, const string &plane_tname, t_gtriple &pos);

        /**
        * @brief get position of planet in crs(J2000), earth is center body.
        * @param[in]   tm             dynamic time(mjd)
        * @param[in]   planet_name   planet's name
        * @param[out]  pos             position of planet(columnvector)
        */
        void get_pos(const double &tm, const string &planet_name, ColumnVector &pos);

        /**
        * @brief string type to PLANET
        * @param[in]   planet    planet name expressed by string
        * @return    planet name expressed by PLANET(enum)
        */
        int str2planet(const string &planet) const;

    protected:
        double _start_mjd = 0.0;
        t_gtime _start;                       ///< start time of JPL data(MJD)
        t_gtime _end;                         ///< end time of data
        double _days;                         ///< interval time for Chebychev coefficient
        double _au;                           ///< Astronomical unit
        double _emrat;                        ///< Earth-Moon mass ratio.
        map<int, vector<double>> _chebycoeff; ///< int--index of time  Chebychev coefficient
        map<int, t_gplanet> _allplanets;      ///< int--planet

        double empty_return = 0.0;

    private:
        /**
        * @brief calculate planet postion and velocity.
        * @param[in]   et         dynamic time(jd)
        * @param[in]   planet     planet(enum)
        * @param[in]   center     center body
        * @param[out]  rrd         postion and velocity of planet
        */
        void _pleph(const double &et, const int &planet, const int &center, double *rrd);

        /**
        * @brief calculate planet postion and velocity.
        * @param[in]   et         dynamic time(jd)
        * @param[in]   list         list of the interpolation symbol of the planet
        * @param[out]  pv         postion and velocity of all planet
        * @param[out]  pnut         postion and velocity of the target planet
        */
        int _state(double et[], int list[], double pv[][13], double pnut[]);

        /**
        * @brief interpolate to get position and velocity.
        * @param[in]   coeff     coefficient to interpolate from JPL
        * @param[in]   ipt         corresponding position of coefficient of the planet
        * @param[in]   dt         interpolate time
        * @param[in]   ncf         # of coeeficients for component
        * @param[in]   ncm
        * @param[in]   na         # of sets of coeficients in full array
        * @param[in]   ifl         interpolation symbol(1--interp pos only;2--pos and vel)
        * @param[out]  pv         postion and velocity of the target planet
        */
        void _interp(const vector<double> &coeff, const int &ipt, const int &ncf, const int &ncm, const int &na, const int &ifl, double dt[], double *pv);

        /**
        * @brief split the integer part and the decimal par.
        * @param[in]   tt         needed to be split
        * @param[in]   fr         split result
        */
        void _split(const double &tt, double *dFR);
    };
}
#endif // !GPLANETEPH_H
