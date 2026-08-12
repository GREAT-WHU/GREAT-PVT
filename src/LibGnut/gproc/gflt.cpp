
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

using namespace std::chrono;

namespace gnut
{
    t_gflt::t_gflt()
    {
    }

    void t_gflt::add_data(const t_gallpar& param,
                          const Eigen::VectorXd& dx,
                          const Eigen::MatrixXd& Qx,
                          const double& sigma0,
                          const Eigen::MatrixXd& Qx0)
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

    void t_gflt::add_data(const Eigen::MatrixXd& A, const Eigen::MatrixXd& P, const Eigen::VectorXd& l)
    {
        try
        {
            _A = A;
            _P = P;
            _l = l;
            _A_virtual.setZero(0, 0);
            _P_virtual.setZero(0, 0);
            _l_virtual.setZero(0);
        }
        catch (...)
        {
        }
    }

    void t_gflt::add_data(const double& vtpv, const int& nobs_total, const int& npar_number)
    {
        _vtpv = vtpv;
        _nobs_total = nobs_total;
        _npar_number = npar_number;
    }

    void t_gflt::add_virtual_obs(const Eigen::MatrixXd& A, const Eigen::MatrixXd& P, const Eigen::VectorXd& l)
    {
        int virtual_obs_num = l.size();
        Eigen::MatrixXd expanded_A = Eigen::MatrixXd::Zero(_nobs_total + virtual_obs_num, _npar_number);
        Eigen::MatrixXd expanded_P = Eigen::MatrixXd::Zero(_nobs_total + virtual_obs_num, _nobs_total + virtual_obs_num);
        Eigen::VectorXd expanded_l = Eigen::VectorXd::Zero(_nobs_total + virtual_obs_num);
        expanded_A.topRows(_nobs_total) = _A;
        expanded_P.topLeftCorner(_nobs_total, _nobs_total) = _P;
        expanded_l.head(_nobs_total) = _l;
        int virtual_total_obs_num = _l_virtual.size();
        Eigen::MatrixXd expanded_A_virtual = Eigen::MatrixXd::Zero(virtual_total_obs_num + virtual_obs_num, _npar_number);
        Eigen::MatrixXd expanded_P_virtual = Eigen::MatrixXd::Zero(virtual_total_obs_num + virtual_obs_num,
                                                                   virtual_total_obs_num + virtual_obs_num);
        Eigen::VectorXd expanded_l_virtual = Eigen::VectorXd::Zero(virtual_total_obs_num + virtual_obs_num);
        if (virtual_total_obs_num > 0)
        {
            expanded_A_virtual.topRows(virtual_total_obs_num) = _A_virtual;
            expanded_P_virtual.topLeftCorner(virtual_total_obs_num, virtual_total_obs_num) = _P_virtual;
            expanded_l_virtual.head(virtual_total_obs_num) = _l_virtual;
        }

        // Float + Fixed
        expanded_A.bottomRows(virtual_obs_num) = A;
        expanded_P.bottomRightCorner(virtual_obs_num, virtual_obs_num) = P;
        expanded_l.tail(virtual_obs_num) = l;
        _A = std::move(expanded_A);
        _P = std::move(expanded_P);
        _l = std::move(expanded_l);
        _nobs_total += virtual_obs_num;

        // Fixed Only
        expanded_A_virtual.bottomRows(virtual_obs_num) = A;
        expanded_P_virtual.bottomRightCorner(virtual_obs_num, virtual_obs_num) = P;
        expanded_l_virtual.tail(virtual_obs_num) = l;
        _A_virtual = std::move(expanded_A_virtual);
        _P_virtual = std::move(expanded_P_virtual);
        _l_virtual = std::move(expanded_l_virtual);
    }

    void t_gflt::change_Qx(int row, int col, double xx)
    {
        _Qx(row - 1, col - 1) = xx;
    }

    void t_gflt::change_dx(int n, double xx)
    {
        if (n == 0)
        {
            _dx.setConstant(xx);
        }
        else
        {
            _dx(n - 1) = xx;
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

    Eigen::VectorXd t_gflt::stdx()
    {
        _stdx.setZero(_Qx.rows());
        for (int i = 0; i < _Qx.rows(); ++i)
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

    bool t_gflt::outlierDetect(const Eigen::MatrixXd& A, const Eigen::MatrixXd& Q, const Eigen::VectorXd& dx, const Eigen::VectorXd& l, Eigen::MatrixXd& P)
    {
        bool res = false;
        double k0 = 3, k1 = 7;
        int n = l.rows();
        Eigen::VectorXd v_post = l - A * dx;
        Eigen::MatrixXd Qz = A * Q * A.transpose() + P.inverse();
        Eigen::VectorXd v_norm;
        v_norm.setZero(n);
        int idx = -1;
        double max_res = 0.0;
        double gamma = 1.0;

        for (int i = 0; i < n; ++i)
        {
            v_norm(i) = sqrt(1.0 / Qz(i, i)) * abs(v_post(i));
            if (v_norm(i) > max_res)
            {
                max_res = v_norm(i);
                idx = i;
            }
        }

        if (max_res <= k0)
        {
            gamma = 1.0;
        }
        else if (max_res > k0 && max_res <= k1)
        {
            gamma = max_res / k0 * (k1 - k0) / (k1 - max_res);
        }
        else
        {
            gamma = 1e2;
        }
        if (gamma > 1.0)
        {
            res = true;
        }
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

    void t_kalman::update(const Eigen::MatrixXd& A, const Eigen::DiagonalMatrix<double, Eigen::Dynamic>& Pl, const Eigen::VectorXd& l, Eigen::VectorXd& dx, Eigen::MatrixXd& Qx)
    {
        Eigen::MatrixXd K; // Kalman gain
        Eigen::MatrixXd NN;

        NN = Pl.inverse().toDenseMatrix() + A * Qx * A.transpose();

        K = Qx * A.transpose() * NN.inverse();

        Eigen::MatrixXd I = Eigen::MatrixXd::Identity(Qx.rows(), Qx.cols());
        Eigen::MatrixXd KA = K * A;
        Eigen::MatrixXd I_KA = I - K * A;

        dx = K * l;                                      // update state vector
        Qx = I_KA * Qx * I_KA.transpose() + K * Pl.inverse() * K.transpose(); // update variance-covariance matrix of state
    }

    void t_kalman::update(const Eigen::MatrixXd& A, const Eigen::MatrixXd& Pl, const Eigen::VectorXd& l, Eigen::VectorXd& dx, Eigen::MatrixXd& Qx)
    {
        Eigen::MatrixXd K; // Kalman gain
        Eigen::MatrixXd NN;
        Eigen::MatrixXd Pli = Pl.inverse();

        NN = Pli + A * Qx * A.transpose();
        K = Qx * A.transpose() * NN.inverse();
        Eigen::MatrixXd I = Eigen::MatrixXd::Identity(Qx.rows(), Qx.cols());
        Eigen::MatrixXd KA = K * A;
        Eigen::MatrixXd I_KA = I - K * A;

        dx = K * l;                                   // update state vector
        Qx = I_KA * Qx * I_KA.transpose() + K * Pli * K.transpose(); // update variance-covariance matrix of state
    }

    void t_SRF::update()
    {
        t_SRF::update(_A, _P, _l, _dx, _Qx);
    }

    void t_SRF::update(const Eigen::MatrixXd& A, const Eigen::DiagonalMatrix<double, Eigen::Dynamic>& Pl, const Eigen::VectorXd& l, Eigen::VectorXd& dx, Eigen::MatrixXd& Qx)
    {
        int nObs = A.rows();
        int nPar = A.cols();

        Eigen::LLT<Eigen::MatrixXd> state_llt(Qx);
        if (state_llt.info() != Eigen::Success)
        {
            return;
        }
        Eigen::MatrixXd SS = state_llt.matrixU();

        Eigen::MatrixXd SA = SS * A.transpose();
        Eigen::MatrixXd SRF = Eigen::MatrixXd::Zero(nObs + nPar, nObs + nPar);
        SRF.diagonal().head(nObs) = Pl.diagonal().array().sqrt().inverse().matrix();

        SRF.bottomLeftCorner(nPar, nObs) = SA;
        SRF.bottomRightCorner(nPar, nPar) = SS;

        Eigen::HouseholderQR<Eigen::MatrixXd> qr(SRF);
        Eigen::MatrixXd UU = qr.matrixQR().template triangularView<Eigen::Upper>();

        SS = UU.bottomRightCorner(nPar, nPar);
        Eigen::MatrixXd SH_rt = UU.topLeftCorner(nObs, nObs);
        Eigen::MatrixXd YY = UU.topRightCorner(nObs, nPar);

        Eigen::MatrixXd KT = SH_rt.template triangularView<Eigen::Upper>().solve(YY);
        dx.noalias() = KT.transpose() * l;
        Eigen::MatrixXd covariance = SS.transpose() * SS;
        Qx = covariance.selfadjointView<Eigen::Lower>();
    }

    void t_SRF::update(const Eigen::MatrixXd& A, const Eigen::MatrixXd& Pl, const Eigen::VectorXd& l, Eigen::VectorXd& dx, Eigen::MatrixXd& Qx)
    {
        int nObs = A.rows();
        int nPar = A.cols();

        Eigen::LLT<Eigen::MatrixXd> state_llt(Qx);
        if (state_llt.info() != Eigen::Success)
        {
            return;
        }
        Eigen::MatrixXd SS = state_llt.matrixU();

        Eigen::MatrixXd SA = SS * A.transpose();

        Eigen::MatrixXd SRF = Eigen::MatrixXd::Zero(nObs + nPar, nObs + nPar);

        Eigen::LLT<Eigen::MatrixXd> observation_llt(Pl);
        if (observation_llt.info() != Eigen::Success)
        {
            return;
        }
        Eigen::MatrixXd SPl = observation_llt.matrixL().solve(Eigen::MatrixXd::Identity(nObs, nObs));

        SRF.topLeftCorner(nObs, nObs) = SPl;
        SRF.bottomLeftCorner(nPar, nObs) = SA;
        SRF.bottomRightCorner(nPar, nPar) = SS;

        Eigen::HouseholderQR<Eigen::MatrixXd> qr(SRF);
        Eigen::MatrixXd UU = qr.matrixQR().template triangularView<Eigen::Upper>();

        SS = UU.bottomRightCorner(nPar, nPar);
        Eigen::MatrixXd SH_rt = UU.topLeftCorner(nObs, nObs);
        Eigen::MatrixXd YY = UU.topRightCorner(nObs, nPar);

        Eigen::MatrixXd KT = SH_rt.template triangularView<Eigen::Upper>().solve(YY);
        dx.noalias() = KT.transpose() * l;
        Eigen::MatrixXd covariance = SS.transpose() * SS;
        Qx = covariance.selfadjointView<Eigen::Lower>();
    }

    void t_SRIF::update(const Eigen::MatrixXd& A, const Eigen::DiagonalMatrix<double, Eigen::Dynamic>& Pl, const Eigen::VectorXd& l, Eigen::VectorXd& dx, Eigen::MatrixXd& Qx)
    {
        int nObs = A.rows();
        int nPar = A.cols();
        Eigen::LLT<Eigen::MatrixXd> covariance_llt(Qx);
        if (covariance_llt.info() != Eigen::Success)
        {
            return;
        }
        Eigen::MatrixXd INF = covariance_llt.solve(Eigen::MatrixXd::Identity(nPar, nPar));
        Eigen::LLT<Eigen::MatrixXd> information_llt(INF);
        if (information_llt.info() != Eigen::Success)
        {
            return;
        }
        Eigen::MatrixXd R = information_llt.matrixU();

        Eigen::VectorXd z = Eigen::VectorXd::Zero(nPar);

        Eigen::MatrixXd SRIF = Eigen::MatrixXd::Zero(nObs + nPar, nPar + 1);

        SRIF.topLeftCorner(nPar, nPar) = R;
        SRIF.bottomLeftCorner(nObs, nPar) = A;
        SRIF.col(nPar).head(nPar) = z;
        SRIF.col(nPar).tail(nObs) = l;

        Eigen::HouseholderQR<Eigen::MatrixXd> qr(SRIF);
        Eigen::MatrixXd MU = qr.matrixQR().topRows(nPar + 1).template triangularView<Eigen::Upper>();

        R = MU.topLeftCorner(nPar, nPar);
        z = MU.col(nPar).head(nPar);

        INF = R * R.transpose();

        dx = R.template triangularView<Eigen::Upper>().solve(z);
        Eigen::LLT<Eigen::MatrixXd> final_information_llt(INF);
        if (final_information_llt.info() != Eigen::Success)
        {
            return;
        }
        Qx = final_information_llt.solve(Eigen::MatrixXd::Identity(nPar, nPar));
    }

} // namespace gnut
