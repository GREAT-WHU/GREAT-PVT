
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
 
-*/

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <cstring>

#include "gutils/gtime.h"
#include "gutils/gcommon.h"
#include "gmodels/gpoly.h"

using namespace std;

namespace gnut
{

    t_gpoly::t_gpoly()
        : _valid(false),
          _ncoeff(0),
          _rms(0.0),
          _xref(0.0),
          _span(0.0)
    {
        _coef.resize(0);
    }

    t_gpoly::~t_gpoly()
    {
    }

    void t_gpoly::reset()
    {
        _ncoeff = 0;
        _rms = 0.0;
        _span = 0.0;
        _valid = false;
        _coef.erase(_coef.begin(), _coef.end());
    }

    int t_gpoly::interpolate(const vector<double> &X,
                             const vector<double> &Y,
                             const double &x,
                             double &y,
                             double &dy)
    {
        y = dy = 0.0; 

        int N = X.size();
        if (N <= 0 || X.size() != Y.size())
            return -1;
        if (N == 1)
        {
            y = Y[0];
            return 1;
        }

        double den = 0.0, ho = 0.0, hp = 0.0, w = 0.0;

        vector<double> c, d;
        for (int i = 0; i < N; i++)
        {
            c.push_back(0.0);
            d.push_back(0.0);
        }

        bool reverse = false;

        if (fabs(x - X[0]) > fabs(x - X[N - 1]))
            reverse = true;

        for (int i = 0; i < N; i++)
        {
            if (reverse)
                c[i] = d[i] = Y[N - 1 - i]; // reverse order/interpolation
            else
                c[i] = d[i] = Y[i]; // standard order/interpolation
        }

        if (reverse)
            y = Y[0];
        else
            y = Y[N - 1];

        int ns = N - 1;

        // for each column of the table loop over c' and d' and update them
        for (int m = 1; m < N; m++)
        {
            for (int i = 0; i < N - m; i++)
            {

                if (reverse)
                { // reverse order
                    ho = X[N - 1 - i] - x;
                    hp = X[N - 1 - i - m] - x;
                }
                else
                { // standard order
                    ho = X[i] - x;
                    hp = X[i + m] - x;
                }

                w = c[i + 1] - d[i];

                if ((den = ho - hp) == 0.0)
                {
                    cerr << " Error in routine [interpolate]." << endl;
                    return -1;
                }

                den = w / den;
                d[i] = hp * den;
                c[i] = ho * den;
            }

            y += dy = d[--ns]; // update y
        }
        return 1;
    }

    int t_gpoly::polynomials(const vector<double> &X,
                             const vector<double> &Y)
    {
        _ncoeff = X.size(); // # polynomials
        if (_ncoeff <= 0)
            return 1;

        int N = _ncoeff - 1; // vector dimension (upper index)

        double b, phi, ff;
        vector<double> s;

        _valid = true;
        _rms = 0.0;
        _span = X[N] - X[0];

        if (!_coef.empty())
            _coef.erase(_coef.begin(), _coef.end());

        for (int i = 0; i < _ncoeff; i++)
        {
            s.push_back(0.0);
            _coef.push_back(0.0);
        }

        s[N] -= (X[0] - _xref);
        for (int i = 1; i <= N; i++)
        {
            for (int j = N - i; j <= N - 1; j++)
                s[j] -= (X[i] - _xref) * s[j + 1];
            s[N] -= (X[i] - _xref);
        }

        for (int j = 0; j <= N; j++)
        {
            phi = N + 1;

            for (int k = N; k >= 1; k--)
                phi = k * s[k] + (X[j] - _xref) * phi;

            ff = Y[j] / phi;
            b = 1.0;

            for (int k = N; k >= 0; k--)
            {
                _coef[k] += b * ff;
                b = s[k] + (X[j] - _xref) * b;
            }
        }

        int verb = 0;
        if (verb > 2)
        {
            for (int j = 0; j < N; j++)
                cerr << _span
                     << fixed
                     << " N=" << N
                     << " R=" << _xref
                     << " X=" << setprecision(3) << setw(12) << X[j] - _xref
                     << " Y=" << setprecision(6) << setw(16) << Y[j]
                     << " C=" << setprecision(6) << setw(16) << _coef[j]
                     << endl;
        }
        return 0;
    }

    void t_gpoly::evaluate(double x, int I, double &y)
    {
        y = 0.0;
        if (_valid)
        {
            for (int j = I; j < _ncoeff; j++)
            {
                int c = 1; // 0-derivation ==> c=1
                for (int k = 1; k <= I; k++)
                {
                    c *= (j - (I - k)); // i-derivation ==> c=c*
                }
                y += c * _coef[j] * pow(x, j - I);
            }
        }
        else
        {
            cerr << "Error: polynomials can not be used, not valid." << endl;
        }
    }

    int t_gpoly::fitpolynom(const vector<double> &X, // x data  time-difference
                            const vector<double> &Y, // y data
                            int N, double tunit,     // degree of polynom and X-time units [sec]
                            const t_gtime &t)        // reference time
    {
        _valid = true;
        _ncoeff = N + 1;
        _rms = 0;
        t_gtime epo;

        double Sum = 0.0;
        double SumP = 0.0;

        int unknwn = _ncoeff;
        int observ = X.size();

        if (!_coef.empty())
            _coef.erase(_coef.begin(), _coef.end());
        for (int i = 0; i < _ncoeff; i++)
            _coef.push_back(0.0);

        if (observ - unknwn < 0)
        {
            cout << " Observ < Unknwn !! : " << observ << " < " << unknwn << endl;
            reset();
            return -1;
        }

        // Copy over to correctly sized matrices
        Matrix A(observ, unknwn);
        Matrix APA(unknwn, unknwn);
        DiagonalMatrix P(observ);
        DiagonalMatrix C(unknwn);
        ColumnVector L(observ);
        ColumnVector RES(observ);
        ColumnVector XXX(unknwn);

        for (int n = 0; n < observ; ++n)
        {
            for (int k = 0; k < unknwn; ++k)
                A(n + 1, k + 1) = pow(X[n], k);
            L(n + 1) = Y[n];
            P(n + 1) = 1.0;
            SumP += P(n + 1);
        }

        for (int k = 0; k < unknwn; ++k)
        {
            C(k + 1) = 1.0;
        } // normalization (fixed for SP3)

        APA = C.t() * A.t() * P * A * C;
        XXX = APA.i() * (C * A.t() * P * L);
        RES = (A * C * XXX - L);

        for (int k = 0; k < unknwn; ++k)
            _coef[k] = C(k + 1) * XXX(k + 1);
        for (int n = 0; n < observ; ++n)
        {
            epo = t + X[n] * tunit;
            Sum += P(n + 1) * RES(n + 1) * RES(n + 1);

            double y = 0.0;
            this->evaluate((epo - t) / tunit, 0, y);
            _rms = sqrt(Sum / SumP); // mm     or Sum/(observ-unknown)
        }

        return 0;
    }

    double lagrange_interpolate(const vector<double> &X, const vector<double> &Y, double x, const bool &lvel)
    {
        // check
        const int maxorder = 30;
        if (X.size() != Y.size())
        {
            cout << "wrong" << endl;
            throw "interpolate Wrong!!number wrong";
        }
        int Num = X.size();
        if (Num >= maxorder)
        {
            throw "interpolate order too big!!";
        }
        static bool first = true;
        static double coeff[maxorder][maxorder];
        // temp down
        if (first)
        {
            // for order
            for (int i = 1; i < maxorder; i++)
            {
                for (int j = 0; j < i; j++)
                {
                    // compute coeff
                    coeff[i][j] = 1.0;
                    ;
                    for (int k = 0; k < i; k++)
                    {
                        if (k != j)
                        {
                            coeff[i][j] *= j - k;
                        }
                    }
                }
            }

            first = false;
        }

        // temp up
        double up = 1.0;
        int zero_idx = -1;
        for (int i = 0; i < Num; i++)
        {
            up *= x - X[i];
            if (x == X[i])
            {
                zero_idx = i;
            }
        }
        double up_vel = 0.0;
        if (lvel && up != 0)
        {
            for (int i = 0; i < Num; i++)
            {
                up_vel += 1 / (x - X[i]);
            }
        }

        double result = 0;
        double temp = 1.0;
        double temp_up = 1.0, temp_down = 1.0;

        for (int i = 0; i < Num; i++)
        {
            temp = 1.0;
            temp_up = 0.0;
            temp_down = coeff[Num][i];
            if (lvel) 
            {
                if (up != 0)
                {
                    temp_up = up / (x - (X[i])) * (up_vel - 1 / (x - X[i]));
                }
                else
                {
                    if (x == X[i])
                    {
                        for (int j = 0; j < Num; j++)
                        {
                            if (i != j)
                            {
                                temp_up += temp_down / (x - X[j]);
                            }
                        }
                    }
                    else
                    {
                        temp_up = coeff[Num][zero_idx] / (x - X[i]);
                    }
                }
            }
            else
            {
                // QYJ
                if (x == X[i])
                {
                    temp_up = temp_down;
                }
                else
                {
                    temp_up = up / (x - X[i]);
                }
            }
            temp = temp_up / temp_down;
            result += temp * Y[i];
        }
        return result;
    }

} // namespace
