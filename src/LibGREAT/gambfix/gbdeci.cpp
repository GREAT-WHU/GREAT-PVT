/**
 * @file         gbdeci.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Judge whether ambiguities meet the requirement of fixing. 
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gambfix/gbdeci.h"
#include "math.h"
#include <string>
#include <stdio.h>
#include <iostream>
using namespace std;

namespace great
{
    //!Input:
    //!est : estimated(real) bias value
    //    !sigma : estimated uncertainty of bias value, scaled by nrms for
    //    !widelane, unscaled for narrow lane
    //    !ih : control for receiver ambiguity
    //    != 1  unit = one cycle
    //    != 2  unit = half cycle
    //    !cutdev : threshold deviation for taper function(= 0.4 in Dong and Bock;
    //!default still 0.4 here and in FIXDRV)
    //    !cutsig : threshold sigma for taper function(= 0.33 in Dong and Bock;
    //!default still 0.4 here and in FIXDRV)

    t_gbdeci::t_gbdeci()
    {
    }

    t_gbdeci::t_gbdeci(const double &cutdev, const double &cutsig, const double &cutprob, const double &cutdeci)
    {
        this->_cutdev = cutdev;
        this->_cutsig = cutsig;
        this->_cutprob = cutprob;
        this->_cutdeci = cutdeci;
    }

    bool t_gbdeci::bdeci(double dEst, double dSigma, int iIh)
    {
        double prob = 0.0;
        double deci = 0.0;
        bdeci(dEst, dSigma, iIh, _cutdev, _cutsig, prob, deci);
        if (deci > _cutdeci)
            return true;
        else
            return false;
    }

    //    !Output :
    //    !prob : area of decision - function region, set = 1.0 for
    //    !each bias on input to BDECI, reduced by the probability
    //    !of an error in rounding each bias. (Since we now search
    //        !only one bias at a time, the cumulative probability is
    //        !not calculated by NBIASR or NBIASP.)
    //    !deci : decision function d(x, sigma) = FT / Q, inverse of
    //    !1. - allowable rate for a type 1 error(alpha),
    //    !compared with input wlcut or nlcut in NBIASR.
    //    !(But F is no longer used because we search one bias at
    //        !a time.)
    void t_gbdeci::bdeci(double dEst, double dSigma, int iIh, double dCutdev, double dCutsig, double &dProb, double &dDeci)
    {

        double dDev, dTerm1, dTerm2, dC, dD1, dA1, dBint, dTaper, dAdd, dCdev, dCsig, dTrun, dS1;
        double dB1, dB2, dErfcb1, dErfcb2;
        int j;

        try
        {
            static double dS2 = 1.414213562373095;
            dAdd = dEst * double(iIh);
            dBint = double(int(dAdd + 0.5 * (1.0 * (dAdd / fabs(dAdd))))) / double(iIh);
            dDev = fabs(dBint - dEst);
            dCdev = dCutdev;

            if (dCutdev < 1.0e-3)
            {
                dCdev = 0.15;
            }

            if (iIh == 2)
            {
                dCdev = 0.5 * dCdev;
            }

            if (dDev >= dCdev)
            {
                dProb = 1.0;
                dDeci = 0.0;
                return;
            }

            dS1 = 1.0 / (dSigma * dS2);
            dTrun = 1.e-9;
            dTerm1 = 1.0 - dDev / dCdev;
            dCsig = dCutsig;

            if (dCutsig < 1.0e-3)
            {
                dCsig = 0.15;
            }
            dTerm2 = (dCsig - dSigma) * 3.0;
            if (dTerm2 < 0)
            {
                dTerm2 = 0.0;
            }
            dTaper = dTerm1 * dTerm1 * dTerm2;
            dC = 0.0;
            for (j = 0; j < 50; j++)
            {
                dA1 = double(j + 1);
                dB1 = (dA1 - dDev) * dS1;
                dB2 = (dA1 + dDev) * dS1;
                if (dB1 < 0 || dB1 > 15.0)
                {
                    dErfcb1 = 0.0;
                }
                else
                {
                    dErfcb1 = t_gbdeci::_erfc(dB1);
                }
                if (dB2 < 0 || dB2 > 15.0)
                {
                    dErfcb2 = 0.0;
                }
                else
                {
                    dErfcb2 = t_gbdeci::_erfc(dB2);
                }

                dD1 = dErfcb1 - dErfcb2;
                dC = dC + dD1;
                if (dD1 < dTrun)
                {
                    dProb = 1.0 - dC;
                    if (dC - 1.0e-9 < 0)
                    {
                        dC = 1.0e-9;
                    }
                    dDeci = dTaper / dC;
                    return;
                }
            }

            dProb = 1.0 - dC;
            if (dC - 1.0e-9 < 0)
            {
                dC = 1.0e-9;
            }
            dDeci = dTaper / dC;
            return;
        }
        catch (...)
        {
            cout << "ERROR : t_gbdeci::bdeci throw exception" << endl;
        }
    }

    void t_gbdeci::_gser(double &dGamser, double dA, double dX, double &dGln)
    {
        double dAp, dSum, dDel;
        int n;
        const int itmax = 100;
        const double dEps = 3.0e-7;

        try
        {
            dGln = _gammln(dA);
            if (dX <= 0)
            {
                if (dX < 0)
                {
                    std::cout << "***ERROR: x < 0 " << endl;
                }
                dGamser = 0.0;
                return;
            }

            dAp = dA;
            dSum = 1.0 / dA;
            dDel = dSum;
            for (n = 0; n < itmax; n++)
            {
                dAp = dAp + 1.0;
                dDel = dDel * dX / dAp;
                dSum = dSum + dDel;
                if (fabs(dDel) < (fabs(dSum) * dEps))
                {
                    dGamser = dSum * exp(-dX + dA * log(dX) - dGln);
                    return;
                }
            }
        }
        catch (...)
        {
            cout << "ERROR : t_gbdeci::_gser throw exception" << endl;
        }
    }

    double t_gbdeci::_gammln(double dXx)
    {
        double cof[6] = {76.18009173, -86.50532033, 24.01409822, -1.231739516, 0.120858003e-2, -0.536382e-5};
        double stp = 2.50662827465;
        double half = 0.5;
        double fpf = 5.5;
        double one = 1.0;
        double x, tmp, ser, gammln;
        int j;

        try
        {
            x = dXx - one;
            tmp = x + fpf;
            tmp = (x + half) * log(tmp) - tmp;
            ser = one;
            for (j = 0; j < 6; j++)
            {
                x = x + one;
                ser = ser + cof[j] / x;
            }
            gammln = tmp + log(stp * ser);
            return gammln;
        }
        catch (...)
        {
            cout << "ERROR : t_gbdeci::_gammln throw exception" << endl;
        }
    }

    void t_gbdeci::_gcf(double &dGammcf, double dA, double dX, double &dGln)
    {

        double anf, gold, a0, a1, b0, b1, fac, an, ana, g;
        int n;
        const int itmax = 100;
        const double dEps = 3.0e-7;
        dGln = t_gbdeci::_gammln(dA);

        g = 0.0;
        gold = 0.0;
        a0 = 1.0;
        a1 = dX;
        b0 = 0.0;
        b1 = 1.0;
        fac = 1.0;

        try
        {
            for (n = 0; n < itmax; n++)
            {
                an = float(n + 1);
                ana = an - dA;
                a0 = (a1 + a0 * ana) * fac;
                b0 = (b1 + b0 * ana) * fac;
                anf = an * fac;
                a1 = dX * a0 + anf * a1;
                b1 = dX * b0 + anf * b1;
                if (a1 != 0)
                {
                    fac = 1.0 / a1;
                    g = b1 * fac;
                    if (fabs((g - gold) / g) < dEps)
                    {
                        dGammcf = exp(-dX + dA * log(dX) - dGln) * g;
                        return;
                    }
                    gold = g;
                }
            }
        }
        catch (...)
        {
            cout << "ERROR : t_gbdeci::_gcf throw exception" << endl;
        }
    }

    double t_gbdeci::_gammq(double dA, double dX)
    {
        double gammq, gamser, gln;
        try
        {
            if (dX < 0 || dA <= 0)
            {
            }
            if (dX < (dA + 1))
            {
                t_gbdeci::_gser(gamser, dA, dX, gln);
                gammq = 1.0 - gamser;
            }
            else
            {
                t_gbdeci::_gcf(gammq, dA, dX, gln);
            }
            return gammq;
        }
        catch (...)
        {
            cout << "ERROR : t_gbdeci::_gammq throw exception" << endl;
            return 0.0;
        }
    }

    double t_gbdeci::_gammp(double dA, double dX)
    {
        double gammp, gln, gammcf;
        try
        {
            if (dX < 0 || dA <= 0)
            {
            }
            if (dX < (dA + 1))
            {
                t_gbdeci::_gser(gammp, dA, dX, gln);
            }
            else
            {
                t_gbdeci::_gcf(gammcf, dA, dX, gln);
                gammp = 1.0 - gammcf;
            }
            return gammp;
        }
        catch (...)
        {
            cout << "ERROR : t_gbdeci::_gammp throw exception" << endl;
            return 0.0;
        }
    }

    double t_gbdeci::_erf(double dX)
    {
        double erf, half;
        half = 0.5;
        try
        {
            if (dX < 0)
            {
                erf = -t_gbdeci::_gammp(half, dX * dX);
            }
            else
            {
                erf = t_gbdeci::_gammp(half, dX * dX);
            }
            return erf;
        }
        catch (...)
        {
            cout << "ERROR : t_gbdeci::_erf throw exception" << endl;
            return 0.0;
        }
    }

    double t_gbdeci::_erfc(double dX)
    {

        double erfc, half;
        half = 0.5;
        if (dX < 0)
        {
            erfc = 1.0 + t_gbdeci::_gammp(half, dX * dX);
        }
        else
        {
            erfc = t_gbdeci::_gammq(half, dX * dX);
        }
        return erfc;
    }
}