
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
 
-*/

#include <iostream>
#include <iomanip>

#include "gutils/gtypeconv.h"
#include "gmodels/gstochasticmodel.h"

namespace gnut
{
    t_stochastic::t_stochastic()
    {
    }

    t_randomwalk::t_randomwalk() : t_stochastic()
    {
        _dSig = 9999.9;

        t_gtime Tfirst(1980, 01, 01, 00, 00, 00, t_gtime::GPS);
        setTprev(Tfirst);
        setTcurr(Tfirst);
    }

    void t_randomwalk::setq(double q)
    {
        this->_dSig = q;
    }

    void t_randomwalk::setTprev(const t_gtime &Tprev)
    {
        this->_Tprev = Tprev;
    }

    void t_randomwalk::setTcurr(const t_gtime &Tcurr)
    {
        this->_Tcurr = Tcurr;
    }

    double t_randomwalk::getQ()
    {
        double Q;
        double q = (_dSig * 1e-3 * _dSig * 1e-3) / 3600;
        //   double q = (_dSig*1.e-3)/3600;

        Q = q * std::abs(_Tcurr - _Tprev);

        return Q;
    }

    double t_randomwalk::get_dt()
    {
        return (_Tcurr - _Tprev);
    }

    void t_randomwalk::updateTime(const t_gtime &Tnew)
    {
        setTprev(_Tcurr);
        setTcurr(Tnew);
    }

    t_whitenoise::t_whitenoise(double var) : t_stochastic()
    {
        this->_var = var;
    }

    void t_whitenoise::setVar(double var)
    {
        this->_var = var;
    }

    double t_whitenoise::getQ()
    {
        return this->_var;
    }

    const double t_statemode::_coeff[6] = {1.0, -sqrt(3), 2 * sqrt(3), sqrt(5), -6 * sqrt(5), 12 * sqrt(5)};

    t_statemode::t_statemode() : order(0)
    {
    }

    t_statemode::t_statemode(int order, double dt, double noise) : order(order)
    {
        // set the Matrix M P
        M = Matrix(order, order);

        // set Matrix M
        for (int Row = order; Row >= 1; Row--)
        {
            M(Row, Row) = 1.0;
            for (int Col = Row + 1; Col <= order; Col++)
            {
                M(Row, Col) = M(Row, Col - 1) * dt / ((double)(Col - Row));
            }
        }

        // set Matrix P
        Matrix P_temp(order, order);
        int count = 0;
        for (int Row = order; Row >= 1; Row--)
        {
            for (int Col = order; Col >= Row; Col--)
            {
                P_temp(Row, Col) = _coeff[count] * pow(dt, Col - order);
                count++;
            }
        }

        P_temp = P_temp / (noise * sqrt(dt));

        P << P_temp.t() * P_temp;
    }

    t_statemode::~t_statemode()
    {
    }

} // namespace
