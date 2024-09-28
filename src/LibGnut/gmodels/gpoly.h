
/**
*
* @verbatim
    History
    2011-04-20  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gpoly.h
* @brief      Purpose: implements polynomial approximation
*             directly interpolate
*             get/fit/evaluate polynomials
*.
* @author     JD
* @version    1.0.0
* @date       2011-04-20
*
*/

#ifndef GPOLY_H
#define GPOLY_H

#include "newmat/newmat.h"

#include <vector>
#include <iostream>

using namespace std;

namespace gnut
{

    /** @brief class for t_gpoly. */
    class t_gpoly
    {
    public:
        /** @brief default constructor. */
        explicit t_gpoly();
        /** @brief default destructor. */
        virtual ~t_gpoly();

        /** @brief reset polynom. */
        void reset();

        /** @brief get polynomial degree. */
        int degree() const { return _ncoeff - 1; }

        /** @brief get # of coefficients. */
        int ncoeff() const { return _ncoeff; }

        /** @brief get accuracy. */
        double rms() const { return _rms; }

        /** @brief get validity. */
        bool valid() const { return _valid; }

        /** @brief get x-span. */
        double span() const { return _span; }

        /** @brief get x-reference value. */
        double xref() const { return _xref; }

        /** @brief get polynomials. */
        vector<double> polynomials() const { return _coef; }

        int interpolate(const vector<double> &X, const vector<double> &Y, const double &x, double &y, double &dy);
        int polynomials(const vector<double> &X, const vector<double> &Y);
        void evaluate(double x, int I, double &y);

        /**
        * @brief fit polynom
        * @param[in]  X            xdata time-difference.
        * @param[in]  Y            ydata.
        * @param[in]  N            degree of polynom.
        * @param[in]  tunit        X-time units [sec].
        * @param[in]  t            reference time.
        * @return      int
        */
        int fitpolynom(const vector<double> &X,
                       const vector<double> &Y,
                       int N, double tunit,
                       const t_gtime &t);

    private:
        bool _valid;          ///< are coefficients valid 
        int _ncoeff;          ///< polynomial order (n) for n+1 points
        double _rms;          ///< RMS of fitted coefficients  [meters]
        double _xref;         ///< x reference value is always 0.0 !!!!!!
        double _span;         ///< x span
        vector<double> _coef; ///< coefficients
    };

    double LibGnut_LIBRARY_EXPORT lagrange_interpolate(const vector<double> &X, const vector<double> &Y, double x, const bool &lvel = false);

} // namespace

#endif
