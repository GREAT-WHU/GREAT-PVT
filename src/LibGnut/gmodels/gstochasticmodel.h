
/**
*
* @verbatim
    History
    2012-05-02 /PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gstochasticmodel.h
* @brief      Purpose: stochastic models
*.
* @author     PV
* @version    1.0.0
* @date       2012-05-02
*
*/

#ifndef STOCHASTIC_H
#define STOCHASTIC_H

#include "gutils/gtime.h"
#include "newmat/newmat.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_stochastic. */
    class LibGnut_LIBRARY_EXPORT t_stochastic
    {
    public:
        /** @brief default constructor. */
        t_stochastic();

        /** @brief default destructor. */
        virtual ~t_stochastic(){};

        /** @brief get Q. */
        virtual double getQ()
        {
            return 0.0;
        }

    protected:
    private:
    };

    /** @brief class for t_randomwalk derive from t_stochastic. */
    class LibGnut_LIBRARY_EXPORT t_randomwalk : public t_stochastic
    {
    public:
        /** @brief default constructor. */
        t_randomwalk();

        /** @brief default destructor. */
        virtual ~t_randomwalk(){};

        /** @brief get Q. */
        virtual double getQ();

        /** @brief set Tprev. */
        void setTprev(const t_gtime &);

        /** @brief set Tcurr. */
        void setTcurr(const t_gtime &);

        /** @brief update time. */
        void updateTime(const t_gtime &);

        /** @brief set Q. */
        void setq(double q);

        /** @brief get dt. */
        double get_dt();

    protected:
    private:
        t_gtime _Tprev; ///< time of prev
        t_gtime _Tcurr; ///< time of current
        double _dSig;   ///< dsigma
    };

    /** @brief class for t_whitenoise derive from t_stochastic. */
    class LibGnut_LIBRARY_EXPORT t_whitenoise : public t_stochastic
    {
    public:
        /** @brief constructor 1. */
        t_whitenoise(double);

        /** @brief default destructor. */
        virtual ~t_whitenoise(){};

        /** @brief get Q. */
        virtual double getQ();

        /** @brief set Var */
        void setVar(double);

    private:
        double _var; ///< var
    };

    /** @brief class for t_statemode. */
    class LibGnut_LIBRARY_EXPORT t_statemode
    {
    public:
        /** @brief default constructor. */
        t_statemode();

        /** @brief constructor 1. */
        t_statemode(int order, double dt, double noise);

        /** @brief default destructor. */
        virtual ~t_statemode();

        int order;         ///< order
        Matrix M;          ///< M
        SymmetricMatrix P; ///< P

    private:
        static const double _coeff[6]; ///coff
    };

} // namespace

#endif // STOCHASTIC_H
