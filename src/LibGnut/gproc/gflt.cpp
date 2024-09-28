
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

  This file is part of the G-Nut C++ library.

-*/

#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <chrono>

#include "gproc/gflt.h"
#include "gutils/gmatrixconv.h"

using namespace std;
using namespace std::chrono;

namespace gnut
{
    t_gflt::t_gflt()
    {
    }

    void t_gflt::add_data(const t_gallpar &param, const ColumnVector &dx, const SymmetricMatrix &Qx, const double &sigma0, const SymmetricMatrix &Qx0)
    {
        try
        {
            _param = param;
            _dx = dx;
            _Qx = Qx;
            _sigma0 = sigma0;
            _Qx0 = Qx0;
        }
        catch (...)
        {
        }
    }

    void t_gflt::add_data(const Matrix &A, const SymmetricMatrix &P, const ColumnVector &l)
    {
        try
        {
            _A = A;
            _P = P;
            _l = l;
            _A_virtual.ReSize(0, 0);
            _P_virtual.ReSize(0);
            _l_virtual.ReSize(0);
        }
        catch (...)
        {
        }
    }

    void t_gflt::add_data(const double &vtpv, const int &nobs_total, const int &npar_number)
    {
        _vtpv = vtpv;
        _nobs_total = nobs_total;
        _npar_number = npar_number;
    }

    void t_gflt::add_virtual_obs(const Matrix &A, const SymmetricMatrix &P, const ColumnVector &l)
    {
        int virtual_obs_num = l.size();
        _A.resize_keep(_nobs_total + virtual_obs_num, _npar_number);
        _P.resize_keep(_nobs_total + virtual_obs_num);
        _l.resize_keep(_nobs_total + virtual_obs_num);

        int virtual_total_obs_num = _l_virtual.size();
        _A_virtual.resize_keep(virtual_total_obs_num + virtual_obs_num, _npar_number);
        _P_virtual.resize_keep(virtual_total_obs_num + virtual_obs_num);
        _l_virtual.resize_keep(virtual_total_obs_num + virtual_obs_num);

        // Float + Fixed
        _A.SubMatrix(_nobs_total + 1, _nobs_total + virtual_obs_num, 1, _npar_number) = A;
        _P.SubMatrix(_nobs_total + 1, _nobs_total + virtual_obs_num, _nobs_total + 1, _nobs_total + virtual_obs_num) = P;
        _l.SubMatrix(_nobs_total + 1, _nobs_total + virtual_obs_num, 1, 1) = l;
        _nobs_total += virtual_obs_num;

        // Fixed Only
        _A_virtual.SubMatrix(virtual_total_obs_num + 1, virtual_total_obs_num + virtual_obs_num, 1, _npar_number) = A;
        _P_virtual.SubMatrix(virtual_total_obs_num + 1, virtual_total_obs_num + virtual_obs_num, virtual_total_obs_num + 1, virtual_total_obs_num + virtual_obs_num) = P;
        _l_virtual.SubMatrix(virtual_total_obs_num + 1, virtual_total_obs_num + virtual_obs_num, 1, 1) = l;
    }

    void t_gflt::change_Qx(int row, int col, double xx)
    {
        _Qx(row, col) = xx;
    }

    void t_gflt::change_dx(int n, double xx)
    {
        if (n == 0)
        {
            _dx = xx;
        }
        else
        {
            _dx(n) = xx;
        }
    }

    void t_gflt::change_sigma0(double sigma)
    {
        _sigma0 = sigma;
    }

    void t_gflt::change_vtpv(double xx)
    {
        _vtpv = xx;
    }

    ColumnVector t_gflt::stdx()
    {
        _stdx.ReSize(_Qx.Nrows());
        _stdx = 0.0;
        for (int i = 1; i <= _Qx.Nrows(); i++)
        {
            _stdx(i) = sqrt(_Qx(i, i)) * _sigma0;
        }
        return _stdx;
    }

    t_kalman::t_kalman()
    {
    }

    t_SRF::t_SRF()
    {
    }

    t_SRIF::t_SRIF()
    {
    }

    t_gflt::~t_gflt()
    {
    }

    t_kalman::~t_kalman()
    {
    }

    t_SRF::~t_SRF()
    {
    }

    t_SRIF::~t_SRIF()
    {
    }

    bool t_gflt::outlierDetect(const Matrix &A, const SymmetricMatrix &Q, const ColumnVector &dx, const ColumnVector &l, SymmetricMatrix &P)
    {
        bool res = false;
        double k0 = 3, k1 = 7;
        int n = l.Nrows();
        ColumnVector v_post = l - A * dx;
        Matrix Qz = A * Q * A.t() + P.i();
        ColumnVector v_norm;
        v_norm.ReSize(n);
        v_norm = 0;
        int idx = -1;
        double max_res = 0.0;
        double gamma = 1.0;

        for (int i = 1; i <= n; i++)
        {
            v_norm(i) = sqrt(1.0 / Qz(i, i)) * abs(v_post(i));
            if (v_norm(i) > max_res)
            {
                max_res = v_norm(i);
                idx = i;
            }
        }

        if (max_res <= k0)
            gamma = 1.0;
        else if (max_res > k0 && max_res <= k1)
        {
            gamma = max_res / k0 * (k1 - k0) / (k1 - max_res);
        }
        else 
        {
            gamma = 1e2;
        }
        if (gamma > 1.0)
            res = true;
        P(idx, idx) = P(idx, idx) / gamma;
        return res;
    }

    void t_gflt::resetQ()
    {
        _Qx = _Qx0;
    }

    void t_kalman::update()
    {
        t_kalman::update(_A, _P, _l, _dx, _Qx);
    }

    void t_kalman::update(const Matrix &A, const DiagonalMatrix &Pl, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Qx)
    {
        Matrix K; // Kalman gain
        Matrix NN;

        NN = Pl.i() + A * Qx * A.t();

        K = Qx * A.t() * NN.i();

        IdentityMatrix I = IdentityMatrix(Qx.nrows());
        Matrix KA = K * A;
        Matrix I_KA = I - K * A;

        dx = K * l;                                      // update state vector
        Qx << I_KA * Qx * I_KA.t() + K * Pl.i() * K.t(); // update variance-covariance matrix of state
    }

    void t_kalman::update(const Matrix &A, const SymmetricMatrix &Pl, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Qx)
    {
        Matrix K; // Kalman gain
        Matrix NN;
        SymmetricMatrix Pli = Pl.i();

        NN = Pli + A * Qx * A.t();
        K = Qx * A.t() * NN.i();
        IdentityMatrix I = IdentityMatrix(Qx.nrows());
        Matrix KA = K * A;
        Matrix I_KA = I - K * A;

        dx = K * l;                                   // update state vector
        Qx << I_KA * Qx * I_KA.t() + K * Pli * K.t(); // update variance-covariance matrix of state
    }

    void t_SRF::update()
    {
        t_SRF::update(_A, _P, _l, _dx, _Qx);
    }

    void t_SRF::update(const Matrix &A, const DiagonalMatrix &Pl, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Qx)
    {
        int nObs = A.Nrows();
        int nPar = A.Ncols();

        UpperTriangularMatrix SS;

        try
        {
            SS = Cholesky(Qx).t();
        }
        catch (NPDException)
        {
            return;
        }

        Matrix SA = SS * A.t();
        Matrix SRF(nObs + nPar, nObs + nPar);
        SRF = 0;
        for (int ii = 1; ii <= nObs; ++ii)
        {
            SRF(ii, ii) = 1.0 / sqrt(Pl(ii, ii));
        }

        SRF.SubMatrix(nObs + 1, nObs + nPar, 1, nObs) = SA;
        SRF.SymSubMatrix(nObs + 1, nObs + nPar) = SS;

        UpperTriangularMatrix UU;
        QRZ(SRF, UU);

        SS = UU.SymSubMatrix(nObs + 1, nObs + nPar);
        UpperTriangularMatrix SH_rt = UU.SymSubMatrix(1, nObs);
        Matrix YY = UU.SubMatrix(1, nObs, nObs + 1, nObs + nPar);

        UpperTriangularMatrix SHi = SH_rt.i();

        Matrix KT = SHi * YY;
        SymmetricMatrix Hi;
        Hi << SHi * SHi.t();

        dx = KT.t() * l;
        Qx << (SS.t() * SS);
    }

    void t_SRF::update(const Matrix &A, const SymmetricMatrix &Pl, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Qx)
    {

        int nObs = A.Nrows();
        int nPar = A.Ncols();

        UpperTriangularMatrix SS;

        try
        {
            SS = Cholesky(Qx).t();
        }
        catch (NPDException)
        {
            return;
        }

        Matrix SA = SS * A.t();

        Matrix SRF(nObs + nPar, nObs + nPar);
        SRF = 0;

        SymmetricMatrix Pli = Pl.i();
        Matrix SPl = Cholesky(Pli).t();

        SRF.SubMatrix(1, nObs, 1, nObs) = SPl;

        SRF.SubMatrix(nObs + 1, nObs + nPar, 1, nObs) = SA;
        SRF.SymSubMatrix(nObs + 1, nObs + nPar) = SS;

        UpperTriangularMatrix UU;
        QRZ(SRF, UU);

        SS = UU.SymSubMatrix(nObs + 1, nObs + nPar);
        UpperTriangularMatrix SH_rt = UU.SymSubMatrix(1, nObs);
        Matrix YY = UU.SubMatrix(1, nObs, nObs + 1, nObs + nPar);

        UpperTriangularMatrix SHi = SH_rt.i();

        Matrix KT = SHi * YY;
        SymmetricMatrix Hi;
        Hi << SHi * SHi.t();

        dx = KT.t() * l;
        Qx << (SS.t() * SS);

    }

    void t_SRIF::update(const Matrix &A, const DiagonalMatrix &Pl, const ColumnVector &l, ColumnVector &dx, SymmetricMatrix &Qx)
    {
        int nObs = A.Nrows();
        int nPar = A.Ncols();
        SymmetricMatrix INF(nPar);
        INF = 0;
        INF = Qx.i();
        UpperTriangularMatrix R = Cholesky(INF).t();

        ColumnVector z(nPar);
        z = 0;

        Matrix SRIF(nObs + nPar, nPar + 1);
        SRIF = 0;

        SRIF.SymSubMatrix(1, nPar) = R;
        SRIF.SubMatrix(nPar + 1, nPar + nObs, 1, nPar) = A;
        SRIF.SubMatrix(1, nPar, nPar + 1, nPar + 1) = z;
        SRIF.SubMatrix(nPar + 1, nPar + nObs, nPar + 1, nPar + 1) = l;

        UpperTriangularMatrix MU;
        QRZ(SRIF, MU);

        R = 0;
        z = 0;
        R = MU.SymSubMatrix(1, nPar);
        z = MU.SubMatrix(1, nPar, nPar + 1, nPar + 1);

        INF << R * R.t();

        dx = R.i() * z;
        Qx = INF.i();
    }

} // namespace
