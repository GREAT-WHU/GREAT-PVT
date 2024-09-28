
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

using namespace std;

namespace gnut
{
    void Matrix_remRC(SymmetricMatrix &Q, int row, int col)
    {
        SymmetricMatrix Qt(Q.Nrows());
        Qt = Q;
        Q.ReSize(Q.Nrows() - 1);
        int rr = 1;
        for (int r = 1; r <= Qt.Nrows(); r++)
        {

            if (r == row)
                continue;
            int cc = 1;

            for (int c = 1; c <= Qt.Ncols(); c++)
            {

                if (c == col)
                    continue;
                Q(rr, cc) = Qt(r, c);
                cc++;
            }
            rr++;
        }
    }

    void Matrix_rem(SymmetricMatrix &Q, vector<int> &ind)
    {
        vector<int>::iterator it;
        vector<int>::iterator it2;
        for (it = ind.begin(); it != ind.end(); it++)
        {
            Matrix_remRC(Q, *it, *it);
            for (it2 = it; it2 != ind.end(); it2++)
                (*it2)--;
        }
    }

    void Matrix_addRC(SymmetricMatrix &Q, int row, int col)
    {
        SymmetricMatrix Qt(Q.Nrows());
        Qt = Q;
        Q.ReSize(Q.Nrows() + 1);
        Q = 0.0;

        bool bc = false;
        bool br = false;
        int rr = 1;

        for (int r = 1; r <= Q.Nrows(); r++)
        {
            if (r == row)
            {
                br = true;
                continue;
            }

            int cc = 1;
            for (int c = 1; c <= Q.Ncols(); c++)
            {
                if (c == col)
                {
                    bc = true;
                    continue;
                }

                if (br && bc)
                    Q(r, c) = Qt(rr, cc);
                if (!br && bc)
                    Q(r, c) = Qt(rr, cc);
                if (br && !bc)
                    Q(r, c) = Qt(rr, cc);
                if (!br && !bc)
                    Q(r, c) = Qt(rr, cc);

                cc++;
            }
            rr++;
        }
    }

    void Matrix_addRC(Matrix &Q, int row, int col)
    {
        Matrix Qt(Q.Nrows(), Q.Ncols());
        Qt = Q;
        if (row == 0)
        {
            Q.ReSize(Q.Nrows(), Q.Ncols() + 1);
            Q = 0;
        }
        else if (col == 0)
        {
            Q.ReSize(Q.Nrows() + 1, Q.Ncols());
            Q = 0;
        }
        else
        {
            Q.ReSize(Q.Nrows() + 1, Q.Ncols() + 1);
            Q = 0;
        }

        bool bc = false;
        bool br = false;
        int rr = 1;
        for (int r = 1; r <= Q.Nrows(); r++)
        {
            if (r == row)
            {
                br = true;
                continue;
            }

            int cc = 1;
            for (int c = 1; c <= Q.Ncols(); c++)
            {
                if (c == col)
                {
                    bc = true;
                    continue;
                }

                if (br && bc)
                    Q(r, c) = Qt(rr, cc);
                if (!br && bc)
                    Q(r, c) = Qt(rr, cc);
                if (br && !bc)
                    Q(r, c) = Qt(rr, cc);
                if (!br && !bc)
                    Q(r, c) = Qt(rr, cc);
                cc++;
            }
            rr++;
        }
    }

    void Matrix_remR(Matrix &A, int row)
    {
        int nrow = A.Nrows(), ncols = A.Ncols();
        Matrix At(nrow - 1, ncols);
        At = 0.0;
        At.submatrix(1, row - 1, 1, ncols) = A.submatrix(1, row - 1, 1, ncols);
        At.submatrix(row, nrow - 1, 1, ncols) = A.submatrix(row + 1, nrow, 1, ncols);
        A = At;
    }

    LibGnut_LIBRARY_EXPORT void Matrix_remR(Matrix &A, vector<int> &ind)
    {
        vector<int>::iterator it;
        vector<int>::iterator it2;
        for (it = ind.begin(); it != ind.end(); it++)
        {
            Matrix_remR(A, *it);
            for (it2 = it; it2 != ind.end(); it2++)
                (*it2)--;
        }
    }

    void Matrix_addRC(DiagonalMatrix &Q, int row)
    {

        DiagonalMatrix Qt(Q.Nrows());
        Qt = Q;
        Q.ReSize(Q.Nrows() + 1);
        Q = 0;

        int rr = 1;
        for (int r = 1; r <= Q.Nrows(); r++)
        {
            if (r == row)
                continue;
            else
            {
                Q(r, r) = Qt(rr, rr);
                rr++;
            }
        }
    }

    void Vector_add(ColumnVector &V, int row)
    {
        ColumnVector Vt(V.Nrows());
        Vt = V;
        V.ReSize(V.Nrows() + 1);
        V = 0;

        int rr = 1;
        for (int r = 1; r <= V.Nrows(); r++)
        {
            if (r == row)
                continue;
            else
            {
                V(r) = Vt(rr);
                rr++;
            }
        }
    }

    Matrix rotX(double Angle)
    {
        const double C = cos(Angle);
        const double S = sin(Angle);
        Matrix UU(3, 3);
        UU[0][0] = 1.0;
        UU[0][1] = 0.0;
        UU[0][2] = 0.0;
        UU[1][0] = 0.0;
        UU[1][1] = +C;
        UU[1][2] = +S;
        UU[2][0] = 0.0;
        UU[2][1] = -S;
        UU[2][2] = +C;
        return UU;
    }

    Matrix rotY(double Angle)
    {
        const double C = cos(Angle);
        const double S = sin(Angle);
        Matrix UU(3, 3);
        UU[0][0] = +C;
        UU[0][1] = 0.0;
        UU[0][2] = -S;
        UU[1][0] = 0.0;
        UU[1][1] = 1.0;
        UU[1][2] = 0.0;
        UU[2][0] = +S;
        UU[2][1] = 0.0;
        UU[2][2] = +C;
        return UU;
    }

    Matrix rotZ(double Angle)
    {
        const double C = cos(Angle);
        const double S = sin(Angle);
        Matrix UU(3, 3);
        UU[0][0] = +C;
        UU[0][1] = +S;
        UU[0][2] = 0.0;
        UU[1][0] = -S;
        UU[1][1] = +C;
        UU[1][2] = 0.0;
        UU[2][0] = 0.0;
        UU[2][1] = 0.0;
        UU[2][2] = 1.0;
        return UU;
    }

    void Matrix_swap(SymmetricMatrix &Q, int a, int b)
    {
        // Copy symmetric matrix to normal matrix
        Matrix T(Q.Nrows(), Q.Ncols());
        for (int r = 1; r <= Q.Nrows(); r++)
        {
            for (int c = 1; c <= Q.Ncols(); c++)
            {
                T(r, c) = Q(r, c);
            }
        }

        // Swap rows
        for (int i = 1; i <= Q.Ncols(); i++)
        {
            double temp = T(a, i);
            T(a, i) = T(b, i);
            T(b, i) = temp;
        }

        // Swap columns
        for (int i = 1; i <= Q.Nrows(); i++)
        {
            double temp = T(i, a);
            T(i, a) = T(i, b);
            T(i, b) = temp;
        }

        // Back to symmetric matrix
        for (int r = 1; r <= T.Nrows(); r++)
        {
            for (int c = 1; c <= T.Ncols(); c++)
            {
                Q(r, c) = T(r, c);
            }
        }
    }

    void Matrix_cpRC(SymmetricMatrix Q1, SymmetricMatrix &Q2, int r, int c)
    {
        if (Q1.Nrows() != Q2.Nrows())
        {
            cerr << "Matrix_cpRC: not compatible dimension" << endl;
        }

        for (int i = 1; i <= Q1.Nrows(); i++)
        {
            for (int j = 1; j <= Q1.Ncols(); j++)
            {
                if (i == r || j == c)
                    Q2(i, j) = Q1(i, j);
            }
        }
    }

} // namespace
