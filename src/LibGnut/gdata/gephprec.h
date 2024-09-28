
/**
*
* @verbatim
    History
    2011-01-10  JD: created
  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gephprec.h
* @brief      derive eph to implement precise ephemerides class
*.
* @author     JD
* @version    1.0.0
* @date       2011-01-10
*
*/

#ifndef GEPHPREC_H
#define GEPHPREC_H

#include "geph.h"
#include "gmodels/gpoly.h"

#define UNDEFVAL_CLK 999999999.999
#define UNDEFVAL_POS 0.000

#define MAXDIFF_CLK 300.0
#define MAXDIFF_EPH 900.0

using namespace std;

namespace gnut
{

    /** @brief class for precise ephemerides data. */
    class LibGnut_LIBRARY_EXPORT t_gephprec : public t_geph
    {

    public:
        /** @brief default constructor. */
        explicit t_gephprec();

        /**
         * @brief Construct a new t gephprec object
         * 
         * @param spdlog 
         */
        explicit t_gephprec(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gephprec();

        /**
        * @brief get the position value
        * @param[in]  t            GPST.
        * @param[in]  xyz        position value.
        * @param[in]  var        position variation, default value is NULL.
        * @param[in]  vel        vel, default value is NULL.
        * @param[in]  chk_health    the heath of satellite, default value is true.
        * @return      int
        */
        int pos(const t_gtime &t, double xyz[3], double var[3] = NULL, double vel[3] = NULL, bool chk_health = true); // [m]

        /**
        * @brief get the GPS time of transmission,
        * @param[in]  t            GPST.
        * @param[in]  clk        clock offset value.
        * @param[in]  var        clock variation, default value is NULL.
        * @param[in]  dclk        clock difference, default value is NULL.
        * @param[in]  chk_health    the heath of satellite, default value is true.
        * @return      int
        */
        int clk(const t_gtime &t, double *clk, double *var = NULL, double *dclk = NULL, bool chk_health = true); // [s]

        /**
        * @brief get the GPS time of transmission,
        * @param[in]  t            GPST.
        * @param[in]  clk        clock offset value.
        * @param[in]  var        clock variation, default value is NULL.
        * @param[in]  dclk        clock difference, default value is NULL.
        * @return      int
        */
        int clk_int(const t_gtime &t, double *clk, double *var = NULL, double *dclk = NULL); // [s]

        /**
        * @brief set the data of xyzc,
        * @param[in]  t            GPST.
        * @param[in]  x         _xcrd value.
        * @param[in]  y         _ycrd value.
        * @param[in]  z         _zcrd value.
        * @param[in]  c         _clkc value.
        * @return      int
        */
        int add(string sat, vector<t_gtime> t,
                const vector<double> &x,
                const vector<double> &y,
                const vector<double> &z,
                const vector<double> &c);

        /** @brief get chk. */
        int chk() const { return 1; }

        /** @brief get str. */
        string str() const { return ""; }

        /** @brief print str. */
        int print()
        {
            cout << str();
            return 0;
        }

        /** @brief set degree of polynomials. */
        void degree(int d)
        {
            _clear();
            _degree = d;
        }

        /** @brief get degree of polynomials. */
        unsigned int degree() const { return _degree; }

        /** @brief get number of needed data. */
        unsigned int ndata() const { return _degree + 1; }

        /** @brief get validity span. */
        unsigned int interval() const { return (size_t)_poly_x.span(); }

        /** @brief check validity (incl.data span). */
        bool valid(const t_gtime &t) const;

    protected:
        /** @brief clear. */
        void _clear();

        /** @brief clk valid? */
        bool _valid_clk() const;

        /** @brief crd valid? */
        bool _valid_crd() const;

    private:
        unsigned int _degree; ///< polynomial degree
        t_gpoly _poly_x;      ///< X polynomials
        t_gpoly _poly_y;      ///< Y polynomials
        t_gpoly _poly_z;      ///< Z polynomials
        t_gpoly _poly_c;      ///< C polynomials

        // _epoch is reference epoch
        vector<double> _dt;   ///< time difference to _epoch
        vector<double> _xcrd; ///< x-coordinate
        vector<double> _ycrd; ///< y-coordinate
        vector<double> _zcrd; ///< z-coordinate
        vector<double> _clkc; ///< clock correction
    };

}

#endif
