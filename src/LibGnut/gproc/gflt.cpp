
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
    namespace
    {
        Eigen::MatrixXd lower_cholesky(const Eigen::MatrixXd& matrix)
        {
            Eigen::MatrixXd lower = Eigen::MatrixXd::Zero(matrix.rows(), matrix.cols());
            for (int row = 0; row < matrix.rows(); ++row)
            {
                for (int col = 0; col < row; ++col)
                {
                    double sum = 0.0;
                    for (int k = 0; k < col; ++k)
                    {
                        sum += lower(col, k) * lower(row, k);
                    }
                    lower(row, col) = (matrix(row, col) - sum) / lower(col, col);
                }

                double sum = 0.0;
                for (int k = 0; k < row; ++k)
                {
                    sum += lower(row, k) * lower(row, k);
                }
                const double diagonal = matrix(row, row) - sum;
                if (diagonal <= 0.0)
                {
                    return Eigen::MatrixXd();
                }
                lower(row, row) = std::sqrt(diagonal);
            }
            return lower;
        }

        Eigen::MatrixXd invert_upper(const Eigen::MatrixXd& upper)
        {
            const int size = upper.rows();
            Eigen::MatrixXd inverse = Eigen::MatrixXd::Zero(size, size);
            for (int col = 0; col < size; ++col)
            {
                for (int row = size - 1; row >= 0; --row)
                {
                    double sum = 0.0;
                    for (int k = size - 1; k > row; --k)
                    {
                        sum += upper(row, k) * inverse(k, col);
                    }
                    inverse(row, col) = ((row == col ? 1.0 : 0.0) - sum) / upper(row, row);
                }
            }
            return inverse;
        }

        Eigen::MatrixXd multiply(const Eigen::MatrixXd& left, const Eigen::MatrixXd& right)
        {
            Eigen::MatrixXd result = Eigen::MatrixXd::Zero(left.rows(), right.cols());
            for (int row = 0; row < left.rows(); ++row)
            {
                for (int k = 0; k < left.cols(); ++k)
                {
                    const double value = left(row, k);
                    for (int col = 0; col < right.cols(); ++col)
                    {
                        result(row, col) += value * right(k, col);
                    }
                }
            }
            return result;
        }

        Eigen::MatrixXd upper_gram(const Eigen::MatrixXd& upper)
        {
            Eigen::MatrixXd result = Eigen::MatrixXd::Zero(upper.cols(), upper.cols());
            for (int row = 0; row < upper.cols(); ++row)
            {
                for (int col = 0; col <= row; ++col)
                {
                    double sum = 0.0;
                    for (int k = 0; k <= col; ++k)
                    {
                        sum += upper(k, row) * upper(k, col);
                    }
                    result(row, col) = sum;
                    result(col, row) = sum;
                }
            }
            return result;
        }

        Eigen::MatrixXd positive_upper_factor(Eigen::MatrixXd matrix)
        {
            Eigen::MatrixXd upper = Eigen::MatrixXd::Zero(matrix.cols(), matrix.cols());
            for (int col = 0; col < matrix.cols(); ++col)
            {
                for (int row = 0; row < matrix.rows(); ++row)
                {
                    const double value = matrix(row, col);
                    for (int next_col = col; next_col < matrix.cols(); ++next_col)
                    {
                        upper(col, next_col) += value * matrix(row, next_col);
                    }
                }

                const double diagonal = std::sqrt(upper(col, col));
                upper(col, col) = diagonal;
                if (diagonal == 0.0)
                {
                    upper.row(col).segment(col + 1, matrix.cols() - col - 1).setZero();
                    matrix.col(col).setZero();
                    continue;
                }

                upper.row(col).segment(col + 1, matrix.cols() - col - 1) /= diagonal;
                for (int row = 0; row < matrix.rows(); ++row)
                {
                    matrix(row, col) /= diagonal;
                    const double value = matrix(row, col);
                    for (int next_col = col + 1; next_col < matrix.cols(); ++next_col)
                    {
                        matrix(row, next_col) -= upper(col, next_col) * value;
                    }
                }
            }
            return upper;
        }
    }

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

        Eigen::MatrixXd SS;

        const Eigen::MatrixXd lower = lower_cholesky(Qx);
        if (lower.size() == 0)
        {
            return;
        }
        SS = lower.transpose();

        Eigen::MatrixXd SA = multiply(SS, A.transpose());
        Eigen::MatrixXd SRF = Eigen::MatrixXd::Zero(nObs + nPar, nObs + nPar);
        for (int ii = 0; ii < nObs; ++ii)
        {
            SRF(ii, ii) = 1.0 / sqrt(Pl.diagonal()(ii));
        }

        SRF.bottomLeftCorner(nPar, nObs) = SA;
        SRF.bottomRightCorner(nPar, nPar) = SS;

        Eigen::MatrixXd UU = positive_upper_factor(SRF);

        SS = UU.bottomRightCorner(nPar, nPar);
        Eigen::MatrixXd SH_rt = UU.topLeftCorner(nObs, nObs);
        Eigen::MatrixXd YY = UU.topRightCorner(nObs, nPar);

        Eigen::MatrixXd SHi = invert_upper(SH_rt);

        Eigen::MatrixXd KT = multiply(SHi, YY);
        dx = multiply(KT.transpose(), l);
        Qx = upper_gram(SS);
    }

    void t_SRF::update(const Eigen::MatrixXd& A, const Eigen::MatrixXd& Pl, const Eigen::VectorXd& l, Eigen::VectorXd& dx, Eigen::MatrixXd& Qx)
    {
        int nObs = A.rows();
        int nPar = A.cols();

        Eigen::MatrixXd SS;

        const Eigen::MatrixXd lower = lower_cholesky(Qx);
        if (lower.size() == 0)
        {
            return;
        }
        SS = lower.transpose();

        Eigen::MatrixXd SA = multiply(SS, A.transpose());

        Eigen::MatrixXd SRF = Eigen::MatrixXd::Zero(nObs + nPar, nObs + nPar);

        Eigen::MatrixXd Pli = Pl.inverse();
        const Eigen::MatrixXd observation_lower = lower_cholesky(Pli);
        if (observation_lower.size() == 0)
        {
            return;
        }
        Eigen::MatrixXd SPl = observation_lower.transpose();

        SRF.topLeftCorner(nObs, nObs) = SPl;
        SRF.bottomLeftCorner(nPar, nObs) = SA;
        SRF.bottomRightCorner(nPar, nPar) = SS;

        Eigen::MatrixXd UU = positive_upper_factor(SRF);

        SS = UU.bottomRightCorner(nPar, nPar);
        Eigen::MatrixXd SH_rt = UU.topLeftCorner(nObs, nObs);
        Eigen::MatrixXd YY = UU.topRightCorner(nObs, nPar);

        Eigen::MatrixXd SHi = invert_upper(SH_rt);

        Eigen::MatrixXd KT = multiply(SHi, YY);
        dx = multiply(KT.transpose(), l);
        Qx = upper_gram(SS);
    }

    void t_SRIF::update(const Eigen::MatrixXd& A, const Eigen::DiagonalMatrix<double, Eigen::Dynamic>& Pl, const Eigen::VectorXd& l, Eigen::VectorXd& dx, Eigen::MatrixXd& Qx)
    {
        int nObs = A.rows();
        int nPar = A.cols();
        Eigen::MatrixXd INF = Qx.inverse();
        const Eigen::MatrixXd information_lower = lower_cholesky(INF);
        if (information_lower.size() == 0)
        {
            return;
        }
        Eigen::MatrixXd R = information_lower.transpose();

        Eigen::VectorXd z = Eigen::VectorXd::Zero(nPar);

        Eigen::MatrixXd SRIF = Eigen::MatrixXd::Zero(nObs + nPar, nPar + 1);

        SRIF.topLeftCorner(nPar, nPar) = R;
        SRIF.bottomLeftCorner(nObs, nPar) = A;
        SRIF.col(nPar).head(nPar) = z;
        SRIF.col(nPar).tail(nObs) = l;

        Eigen::MatrixXd MU = positive_upper_factor(SRIF);

        R = MU.topLeftCorner(nPar, nPar);
        z = MU.col(nPar).head(nPar);

        INF = R * R.transpose();

        dx = R.inverse() * z;
        Qx = INF.inverse();
    }

} // namespace gnut
