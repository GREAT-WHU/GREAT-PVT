
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <iostream>
#include <iomanip>
#include <cmath>

#include "gutils/gmatrixconv.h"
#include "gutils/gconst.h"
#include "gutils/gtypeconv.h"


namespace gnut
{
    void Matrix_remRC(Eigen::MatrixXd& Q, int row, int col)
    {
        Eigen::MatrixXd reduced(Q.rows() - 1, Q.cols() - 1);
        int rr = 0;
        for (int r = 0; r < Q.rows(); ++r)
        {
            if (r == row - 1)
            {
                continue;
            }
            int cc = 0;
            for (int c = 0; c < Q.cols(); ++c)
            {
                if (c == col - 1)
                {
                    continue;
                }
                reduced(rr, cc++) = Q(r, c);
            }
            ++rr;
        }
        Q = std::move(reduced);
    }

    void Matrix_rem(Eigen::MatrixXd& Q, std::vector<int>& ind)
    {
        std::vector<int>::iterator it;
        std::vector<int>::iterator it2;
        for (it = ind.begin(); it != ind.end(); it++)
        {
            Matrix_remRC(Q, *it, *it);
            for (it2 = it; it2 != ind.end(); it2++)
            {
                (*it2)--;
            }
        }
    }

    void Matrix_addRC(Eigen::MatrixXd& Q, int row, int col)
    {
        const int row_offset = row == 0 ? 0 : 1;
        const int col_offset = col == 0 ? 0 : 1;
        Eigen::MatrixXd expanded = Eigen::MatrixXd::Zero(Q.rows() + row_offset, Q.cols() + col_offset);
        for (int r = 0; r < Q.rows(); ++r)
        {
            const int target_row = r + (row_offset && r >= row - 1);
            for (int c = 0; c < Q.cols(); ++c)
            {
                const int target_col = c + (col_offset && c >= col - 1);
                expanded(target_row, target_col) = Q(r, c);
            }
        }
        Q = std::move(expanded);
    }

    void Matrix_remR(Eigen::MatrixXd& A, int row)
    {
        Eigen::MatrixXd reduced(A.rows() - 1, A.cols());
        if (row > 1)
        {
            reduced.topRows(row - 1) = A.topRows(row - 1);
        }
        if (row < A.rows())
        {
            reduced.bottomRows(A.rows() - row) = A.bottomRows(A.rows() - row);
        }
        A = std::move(reduced);
    }

    LibGnut_LIBRARY_EXPORT void Matrix_remR(Eigen::MatrixXd& A, std::vector<int>& ind)
    {
        std::vector<int>::iterator it;
        std::vector<int>::iterator it2;
        for (it = ind.begin(); it != ind.end(); it++)
        {
            Matrix_remR(A, *it);
            for (it2 = it; it2 != ind.end(); it2++)
            {
                (*it2)--;
            }
        }
    }

    void Matrix_remR(Eigen::VectorXd& A, int row)
    {
        Eigen::VectorXd At(A.rows() - 1);
        if (row > 1)
        {
            At.head(row - 1) = A.head(row - 1);
        }
        if (row <= At.rows())
        {
            At.tail(At.rows() - row + 1) = A.tail(A.rows() - row);
        }
        A = At;
    }

    void Matrix_remR(Eigen::VectorXd& A, std::vector<int>& ind)
    {
        for (std::vector<int>::iterator it = ind.begin(); it != ind.end(); ++it)
        {
            Matrix_remR(A, *it);
            for (std::vector<int>::iterator shifted = it; shifted != ind.end(); ++shifted)
            {
                --(*shifted);
            }
        }
    }

    void Matrix_addRC(Eigen::DiagonalMatrix<double, Eigen::Dynamic>& Q, int row)
    {
        Eigen::VectorXd diagonal = Eigen::VectorXd::Zero(Q.rows() + 1);
        if (row > 1)
        {
            diagonal.head(row - 1) = Q.diagonal().head(row - 1);
        }
        if (row <= Q.rows())
        {
            diagonal.tail(Q.rows() - row + 1) = Q.diagonal().tail(Q.rows() - row + 1);
        }
        Q.diagonal() = diagonal;
    }

    void Vector_add(Eigen::VectorXd& V, int row)
    {
        Eigen::VectorXd expanded = Eigen::VectorXd::Zero(V.rows() + 1);
        if (row > 1)
        {
            expanded.head(row - 1) = V.head(row - 1);
        }
        if (row <= V.rows())
        {
            expanded.tail(V.rows() - row + 1) = V.tail(V.rows() - row + 1);
        }
        V = std::move(expanded);
    }

    Eigen::MatrixXd rotX(double Angle)
    {
        const double C = cos(Angle);
        const double S = sin(Angle);
        Eigen::MatrixXd UU(3, 3);
        UU << 1.0, 0.0, 0.0,
              0.0, +C, +S,
              0.0, -S, +C;
        return UU;
    }

    Eigen::MatrixXd rotY(double Angle)
    {
        const double C = cos(Angle);
        const double S = sin(Angle);
        Eigen::MatrixXd UU(3, 3);
        UU << +C, 0.0, -S,
              0.0, 1.0, 0.0,
              +S, 0.0, +C;
        return UU;
    }

    Eigen::MatrixXd rotZ(double Angle)
    {
        const double C = cos(Angle);
        const double S = sin(Angle);
        Eigen::MatrixXd UU(3, 3);
        UU << +C, +S, 0.0,
              -S, +C, 0.0,
              0.0, 0.0, 1.0;
        return UU;
    }

    void Matrix_swap(Eigen::MatrixXd& Q, int a, int b)
    {
        Q.row(a - 1).swap(Q.row(b - 1));
        Q.col(a - 1).swap(Q.col(b - 1));
    }

    void Matrix_cpRC(Eigen::MatrixXd Q1, Eigen::MatrixXd& Q2, int r, int c)
    {
        if (Q1.rows() != Q2.rows())
        {
            std::cerr << "Matrix_cpRC: not compatible dimension" << std::endl;
        }

        for (int i = 0; i < Q1.rows(); ++i)
        {
            for (int j = 0; j < Q1.cols(); ++j)
            {
                if (i == r - 1 || j == c - 1)
                {
                    Q2(i, j) = Q1(i, j);
                }
            }
        }
    }

} // namespace gnut
