/**
*
* @verbatim
    History
    2011-01-10  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gflt.h
* @brief      Purpose: implements Kalman filter class
*.
* @author     PV
* @version    1.0.0
* @date       2011-01-10
*
*/

#ifndef FLT_H
#define FLT_H

#include "newmat/newmat.h"
#include "newmat/newmatap.h"
#include "gall/gallpar.h"
#include "gexport/ExportLibGnut.h"

using namespace std;

namespace gnut
{
    /** @brief Base class for all filtering technique. */
    class LibGnut_LIBRARY_EXPORT t_gflt
    {
    public:
        /** @brief default constructor. */
        t_gflt();

        /** @brief default destructor. */
        virtual ~t_gflt();

        /** @brief update parameter. */
        virtual void update() {}

        /**
        * @brief update parametere.
        *
        * @param[in]  A        A matrix in flt
        * @param[in]  P        P matrix in flt
        * @param[in]  l           l matrix in flt
        * @param[in]  dx       dx matrix in flt
        * @param[in]  Q              Q matrix in flt
        * @return void
        */
        virtual void update(const Matrix &A, const DiagonalMatrix &P, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Q){};

        /**
        * @brief update parametere.
        *
        * @param[in]  A        A matrix in flt
        * @param[in]  P        P matrix in flt
        * @param[in]  l           l matrix in flt
        * @param[in]  dx       dx matrix in flt
        * @param[in]  Q              Q matrix in flt
        * @return void
        */
        virtual void update(const Matrix &A, const SymmetricMatrix &P, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Q){};

        /**
        * @brief add data.
        *
        * @param[in]  param        Pars in flt
        * @param[in]  dx              dx matrix in flt
        * @param[in]  Qx           Qx matrix in flt
        * @param[in]  sigma0       sigma0 in flt
        * @param[in]  Qx0              Qx0 matrix in flt
        * @return void
        */
        virtual void add_data(const t_gallpar &param, const ColumnVector &dx, const SymmetricMatrix &Qx, const double &sigma0, const SymmetricMatrix &Qx0);

        /**
        * @brief add data.
        *
        * @param[in]  A        A matrix in flt
        * @param[in]  P           P matrix in flt
        * @param[in]  l              l matrix in flt
        * @return void
        */
        virtual void add_data(const Matrix &A, const SymmetricMatrix &P, const ColumnVector &l);

        /**
        * @brief add data.
        *
        * @param[in]  vtpv                 v*p*v in flt
        * @param[in]  nobs_total           number of obs in flt
        * @param[in]  npar_number              number of pars in flt
        * @return void
        */
        virtual void add_data(const double &vtpv, const int &nobs_total, const int &npar_number);

        /**
        * @brief add virtual obs to flt.
        *
        * @param[in]  A        A matrix in flt
        * @param[in]  P           P matrix in flt
        * @param[in]  l              l matrix in flt
        * @return void
        */
        virtual void add_virtual_obs(const Matrix &A, const SymmetricMatrix &P, const ColumnVector &l);

        /**
        * @brief detect outlier.
        *
        * @param[in]  A        A matrix in flt
        * @param[in]  Q        Q matrix in flt
        * @param[in]  dx       dx matrix in flt
        * @param[in]  l        l matrix in flt
        * @param[in]  P           P matrix in flt
        * @return void
        */
        virtual bool outlierDetect(const Matrix &A, const SymmetricMatrix &Q, const ColumnVector &dx, const ColumnVector &l, SymmetricMatrix &P);

        /** @brief reset Qx. */
        virtual void resetQ();

        /** @brief set Qx Matrix specified location 
        * @param[in]  row        row of Qx matrix in flt
        * @param[in]  col        col of Qx matrix in flt
        * @param[in]  xx         value for (row,col) of Qx matrix in flt
        * 
        */
        void change_Qx(int row, int col, double xx);

        /** @brief set dx  specified location 
        * @param[in]  n          n of dx matrix in flt
        * @param[in]  xx         value for (n) of dx matrix in flt
        * 
        */
        void change_dx(int n, double xx);

        /** @brief set sigma0 by  specified sigma 
        * @param[in]  n          n of dx matrix in flt
        * @param[in]  xx         value for (n) of sigma0 matrix in flt
        * 
        */
        void change_sigma0(double sigma);

        /** @brief set vtpv by  specified value 
        * @param[in]  xx         value for vtpv  in flt
        * 
        */
        void change_vtpv(double xx);

        /** @brief get stdx */
        ColumnVector stdx();

        /** @brief get dx */
        ColumnVector dx() { return _dx; }

        /** @brief get Qx */
        SymmetricMatrix Qx() { return _Qx; }

        /** @brief get param */
        t_gallpar param() { return _param; }

        /** @brief get sigma0 */
        double sigma0() { return _sigma0; }

        /** @brief get vtpv */
        double vtpv() { return _vtpv; }

        /** @brief get nobs total */
        int nobs_total() { return _nobs_total; }

        /** @brief get npar number */
        int npar_number() { return _npar_number; }

        /** @brief set/get amb 
        * @param[in]  state         amb state  in flt
        */
        void amb(bool state) { _amb = state; }

        /** @brief set/get amb */
        bool amb() { return _amb; }

    protected:
        ColumnVector _dx;    ///< dx
        ColumnVector _stdx;  ///< stdx
        SymmetricMatrix _Qx; ///< Qx
        //LX added for WL constraint
        SymmetricMatrix _Qx_tmp;    ///< tmp Qx matrix
        SymmetricMatrix _Qx0;       ///< Qx0 matrix
        Matrix _A;                  ///< A matrix
        Matrix _A_virtual;          ///< A matrix for virtual using
        SymmetricMatrix _P;         ///< P matrix
        SymmetricMatrix _P_virtual; ///< P matrix for virtual using
        ColumnVector _l;            ///< l matrix
        ColumnVector _l_virtual;    ///< l matrix for virtual using
        t_gallpar _param;           ///< all pars in flt
        double _sigma0;             ///< sigma0
        double _vtpv;               ///< value of v*p*v
        int _nobs_total;            ///< number of obs
        int _npar_number;           ///< number of par
        bool _amb = false;          ///< fix amb
    };

    /** @brief class for Classical formule for Kalman filter. */
    class LibGnut_LIBRARY_EXPORT t_kalman : public t_gflt
    {

    public:
        /** @brief default constructor. */
        t_kalman();

        /** @brief default destructor. */
        ~t_kalman();

        /** @brief update parameter. */
        virtual void update();
        void update(const Matrix &A, const DiagonalMatrix &P, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Q);
        void update(const Matrix &A, const SymmetricMatrix &P, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Q);
    };

    /** @brief class for Square root covariance filter derive from t_gflt. */
    class LibGnut_LIBRARY_EXPORT t_SRF : public t_gflt
    {
    public:
        /** @brief default constructor. */
        t_SRF();

        /** @brief default destructor. */
        ~t_SRF();

        /** @brief update parameter. */
        virtual void update();

        /**
        * @brief update parametere.
        *
        * @param[in]  A        A matrix in flt
        * @param[in]  P        P matrix in flt
        * @param[in]  l           l matrix in flt
        * @param[in]  dx       dx matrix in flt
        * @param[in]  Q              Q matrix in flt
        * @return void
        */
        void update(const Matrix &A, const DiagonalMatrix &P, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Q);

        /**
        * @brief update parametere.
        *
        * @param[in]  A        A matrix in flt
        * @param[in]  P        P matrix in flt
        * @param[in]  l           l matrix in flt
        * @param[in]  dx       dx matrix in flt
        * @param[in]  Q              Q matrix in flt
        * @return void
        */
        void update(const Matrix &A, const SymmetricMatrix &P, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Q);
    };

    /** @brief class for Square root information filter derive from t_gflt. */
    class LibGnut_LIBRARY_EXPORT t_SRIF : public t_gflt
    {
    public:
        /** @brief default constructor. */
        t_SRIF();

        /** @brief default destructor. */
        ~t_SRIF();

        /**
        * @brief update parametere.
        *
        * @param[in]  A        A matrix in flt
        * @param[in]  P        P matrix in flt
        * @param[in]  l           l matrix in flt
        * @param[in]  dx       dx matrix in flt
        * @param[in]  Q              Q matrix in flt
        * @return void
        */
        void update(const Matrix &A, const DiagonalMatrix &P, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Q);
    };

} // namespace

#endif
