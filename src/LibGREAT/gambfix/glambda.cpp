/**
 * @file         glambda.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        integer estimation with the LAMBDA method.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gambfix/glambda.h"
#include "math.h"
#include <string>
#include <string.h>
#include <stdio.h>
using namespace std;

namespace great
{

    void t_glambda::BACKTs(int iN, int *piI, double *pdEnd, double *pdDist, double *pdLef, double *pdLeft, bool *pdEnde)
    {
        int iJ;
        iJ = *piI + 1;
        for ((*piI) = iJ; (*piI) <= iN; (*piI)++)
        {
            if (pdDist[(*piI) - 1] <= pdEnd[(*piI) - 1])
            {
                pdDist[(*piI) - 1] += 1e0;
                pdLeft[(*piI) - 1] = pow(pdDist[(*piI) - 1] + pdLef[(*piI) - 1], 2);
                return;
            }
        }
        *pdEnde = true;
        return;
    }

    void t_glambda::ZTRANi(int iFirst, int iLast, int iN, double *pdL, double *pdA, double *pdZti)
    {
        int i, j, k, iMu;
        for (i = iLast; i >= iFirst; i--)
        {
            for (j = i + 1; j <= iN; j++)
            {
                iMu = round(pdL[(j - 1) * iN + i - 1]);
                if (iMu != 0)
                {
                    for (k = j; k <= iN; k++)
                    {
                        pdL[(k - 1) * iN + i - 1] -= iMu * pdL[(k - 1) * iN + j - 1];
                    }
                    for (k = 1; k <= iN; k++)
                    {
                        pdZti[(k - 1) * iN + j - 1] += iMu * pdZti[(k - 1) * iN + i - 1];
                    }
                    pdA[i - 1] -= iMu * pdA[j - 1];
                }
            }
        }
        return;
    }

    void t_glambda::STOREs(int iCan, int iPos, int *piMax, double dT, double *pdTmax, double *pdDist, double *pdDisall, double *pdCands, int iMaxCan, int iN)
    {
        int i;
        for (i = 1; i <= iN; i++)
        {
            pdCands[(i - 1) * iMaxCan + iPos - 1] = pdDist[i - 1];
        }
        pdDisall[iPos - 1] = dT;
        *pdTmax = dT;
        *piMax = iPos;
        for (i = 1; i <= iCan; i++)
        {
            if (pdDisall[i - 1] > (*pdTmax))
            {
                *piMax = i;
                *pdTmax = pdDisall[i - 1];
            }
        }
        return;
    }

    void t_glambda::COLLECTs(int iN, int iMaxCan, double dD_1, double dLef_1, double dLeft_1, double dRight_1, double dChic, double *pdDist,
                             double *pdEnd_1, int *piNcan, double *pdDisall, double *pdCands, double *pdTmax, int *piMax)
    {
        double dT;
        dT = dChic - (dRight_1 - dLeft_1) * dD_1;
        (*pdEnd_1) += 1e0;

        while (true)
        {
            (*piNcan)++;
            if (*piNcan >= 10000)
                break;
            if (*piNcan <= iMaxCan)
            {
                STOREs(*piNcan, *piNcan, piMax, dT, pdTmax, pdDist, pdDisall, pdCands, iMaxCan, iN);
            }
            else
            {
                if (dT < (*pdTmax))
                {
                    STOREs(iMaxCan, *piMax, piMax, dT, pdTmax, pdDist, pdDisall, pdCands, iMaxCan, iN);
                }
            }
            dT += (2 * (pdDist[0] + dLef_1) + 1) * dD_1;
            pdDist[0] += 1e0;
            if (pdDist[0] > (*pdEnd_1))
            {
                break;
            }
        }

        return;
    }

    int t_glambda::FMFAC6(double *pdL, double *pdD, int iN, double dEps)
    {
        double dGomi = 1e20;
        int i, j, k;
        double dSum;
        double dGooge;
        double dT;

        for (j = iN; j >= 1; j--)
        {
            for (i = iN; i >= j + 1; i--)
            {
                dSum = 0.0;
                for (k = i + 1; k <= iN; k++)
                {
                    dSum += pdL[(k - 1) * iN + j - 1] * pdL[(k - 1) * iN + i - 1];
                }
                pdL[(i - 1) * iN + j - 1] = (pdL[(i - 1) * iN + j - 1] - dSum) / pdL[(i - 1) * iN + i - 1];
            }
            dSum = 0.0;

            for (k = j + 1; k <= iN; k++)
            {
                dSum += pdL[(k - 1) * iN + j - 1] * pdL[(k - 1) * iN + j - 1];
            }
            dT = pdL[(j - 1) * iN + j - 1] - dSum;
            if (dT <= pdL[(j - 1) * iN + j - 1] * dEps)
            {
                return -1;
            }
            dGooge = dT / pdL[(j - 1) * iN + j - 1];
            if (dGooge < dGomi)
            {
                dGomi = dGooge;
            }
            pdL[(j - 1) * iN + j - 1] = sqrt(dT);
        }

        for (i = 1; i <= iN; i++)
        {
            for (j = 1; j <= i - 1; j++)
            {
                pdL[(i - 1) * iN + j - 1] /= pdL[(i - 1) * iN + i - 1];
            }
            pdD[i - 1] = pdL[(i - 1) * iN + i - 1] * pdL[(i - 1) * iN + i - 1];
            pdL[(i - 1) * iN + i - 1] = 1e0;
        }

        return 0;
    }

    void t_glambda::INVLT2d(int iN, double *pdL, double *pdLm, double *pdVec)
    {
        int i, j, k;
        double dAiude;
        for (i = 1; i <= iN; i++)
        {
            for (j = 1; j <= i - 1; j++)
            {
                pdVec[j - 1] = pdL[(i - 1) * iN + j - 1];
            }

            for (j = 1; j <= i - 1; j++)
            {
                dAiude = 0.0;

                for (k = j; k <= i - 1; k++)
                {
                    dAiude += pdLm[(k - 1) * iN + j - 1] * pdVec[k - 1];
                }
                pdLm[(i - 1) * iN + j - 1] = -dAiude / pdL[(i - 1) * iN + i - 1];
            }

            pdLm[(i - 1) * iN + i - 1] = 1e0 / pdL[(i - 1) * iN + i - 1];
        }
        return;
    }

    void t_glambda::SRC1i(int iN, double *pdL, double *pdD, double *pdA, double *pdZti)
    {
        double dt_glambda[3];
        bool isSwap;
        int i1, i, j;
        double dDelta, dEta, dHelp;
        i1 = iN - 1;
        isSwap = true;

        while (true)
        {
            if (isSwap)
            {
                i = iN;
                isSwap = false;

                while (true)
                {
                    if (!isSwap && i > 1)
                    {
                        i--;
                        if (i <= i1)
                        {
                            ZTRANi(i, i, iN, pdL, pdA, pdZti);
                        }

                        dDelta = pdD[i - 1] + pow(pdL[i * iN + i - 1], 2) * pdD[i];
                        if (dDelta < pdD[i])
                        {
                            dt_glambda[2] = pdD[i] * pdL[i * iN + i - 1] / dDelta;
                            dEta = pdD[i - 1] / dDelta;
                            pdD[i - 1] = dEta * pdD[i];
                            pdD[i] = dDelta;
                            for (j = 1; j <= i - 1; j++)
                            {
                                dt_glambda[0] = pdL[(i - 1) * iN + j - 1];
                                dt_glambda[1] = pdL[i * iN + j - 1];
                                pdL[(i - 1) * iN + j - 1] = dt_glambda[1] - pdL[i * iN + i - 1] * dt_glambda[0];
                                pdL[i * iN + j - 1] = dt_glambda[2] * dt_glambda[1] + dEta * dt_glambda[0];
                            }

                            pdL[i * iN + i - 1] = dt_glambda[2];

                            for (j = i + 2; j <= iN; j++)
                            {
                                dHelp = pdL[(j - 1) * iN + i - 1];
                                pdL[(j - 1) * iN + i - 1] = pdL[(j - 1) * iN + i];
                                pdL[(j - 1) * iN + i] = dHelp;
                            }
                            for (j = 1; j <= iN; j++)
                            {
                                dHelp = pdZti[(j - 1) * iN + i - 1];
                                pdZti[(j - 1) * iN + i - 1] = pdZti[(j - 1) * iN + i];
                                pdZti[(j - 1) * iN + i] = dHelp;
                            }
                            dHelp = pdA[i - 1];
                            pdA[i - 1] = pdA[i];
                            pdA[i] = dHelp;
                            i1 = i;
                            isSwap = true;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                break;
            }
        }
        return;
    }

    void t_glambda::CHIstrt4(int iN, double *pdD, double *pdL, double *pdDist, double *pdE, double *pdA, double dTm[2], double *pdT)
    {
        double dBig = 1e10;
        int i, j;
        double dT_0;

        for (i = 1; i <= iN; i++)
        {
            pdDist[i - 1] = round(pdA[i - 1]) - pdA[i - 1];
        }

        for (i = 1; i <= iN; i++)
        {
            pdE[i - 1] = 0e0;
            for (j = i; j <= iN; j++)
            {
                pdE[i - 1] += pdL[(j - 1) * iN + i - 1] * pdDist[j - 1];
            }
        }

        dT_0 = 0e0;
        for (i = 1; i <= iN; i++)
        {
            dT_0 += pdD[i - 1] * pdE[i - 1] * pdE[i - 1];
        }

        dTm[0] = dT_0;
        dTm[1] = dBig;

        for (i = 1; i <= iN; i++)
        {
            if (pdDist[i - 1] < 0.0)
            {
                pdT[i - 1] = dT_0;
                for (j = 1; j <= i; j++)
                {
                    pdT[i - 1] = pdT[i - 1] + pdD[j - 1] * pdL[(i - 1) * iN + j - 1] * (2 * pdE[j - 1] + pdL[(i - 1) * iN + j - 1]);
                }
            }
            else
            {
                pdT[i - 1] = dT_0;
                for (j = 1; j <= i; j++)
                {
                    pdT[i - 1] = pdT[i - 1] - pdD[j - 1] * pdL[(i - 1) * iN + j - 1] * (2 * pdE[j - 1] - pdL[(i - 1) * iN + j - 1]);
                }
            }

            if (pdT[i - 1] < dTm[0])
            {
                dTm[1] = dTm[0];
                dTm[0] = pdT[i - 1];
            }
            else if (pdT[i - 1] < dTm[1])
            {
                dTm[1] = pdT[i - 1];
            }
        }

        return;
    }

    double t_glambda::DINKi(int iN, double *pdL, double *pdA, int iCol)
    {
        double dDINKi;
        int i;
        dDINKi = 0.0;
        for (i = iCol + 1; i <= iN; i++)
        {
            dDINKi += pdL[(i - 1) * iN + iCol - 1] * pdA[i - 1];
        }
        return dDINKi;
    }

    int t_glambda::JNT2(double dD)
    {
        int iJNT2;
        if (dD <= 0.0)
        {
            iJNT2 = ceil(dD); 
        }
        else
        {
            iJNT2 = floor(dD) + 1;
        }
        return iJNT2;
    }

    void t_glambda::FI71(double dChic, int iMaxCan, int iN, double *pdA, double *pdD, double *pdL, double *pdLef, double *pdLeft, double *pdRight,
                         double *pdDist, double *pdEnd, double *pdDq, int *piNcan, double *pdDisall, double *pdCands, int *piPos)
    {
        bool isEnde;
        int i, j, iold, iMax;
        double dReach, dDelta, dTmax, dDminn;
        if (iMaxCan < 1)
        {
            throw("ERROR in FI71: number of requested candidates < 1");
        }
        else if (iN < 2)
        {
            throw("ERROR in FI71: dimension of system < 2");
        }
        else
        {
        }
        isEnde = false;
        pdRight[iN] = dChic;
        pdLeft[iN] = 0.0;
        for (i = 1; i <= iN - 1; i++)
        {
            pdDq[i - 1] = pdD[i] / pdD[i - 1];
        }
        pdDq[iN - 1] = 1e0 / pdD[iN - 1];

        *piNcan = 0;
        i = iN + 1;
        iold = i;

        while (true)
        {
            i--;
            if (i == 2)
                i = i;
            if (iold <= i)
            {
                pdLef[i - 1] += pdL[i * iN + i - 1];
            }
            else
            {
                pdLef[i - 1] = t_glambda::DINKi(iN, pdL, pdDist, i);
            }
            iold = i;

            pdRight[i - 1] = (pdRight[i] - pdLeft[i]) * pdDq[i - 1];
            dReach = sqrt(pdRight[i - 1]);

            dDelta = pdA[i - 1] - dReach - pdLef[i - 1];
            pdDist[i - 1] = t_glambda::JNT2(dDelta) - pdA[i - 1];
            if (pdDist[i - 1] > (dReach - pdLef[i - 1]))
            {
                t_glambda::BACKTs(iN, &i, pdEnd, pdDist, pdLef, pdLeft, &isEnde);
            }
            else
            {
                pdEnd[i - 1] = dReach - pdLef[i - 1] - 1e0;
                pdLeft[i - 1] = pow(pdDist[i - 1] + pdLef[i - 1], 2);
            }

            if (i == 1)
            {
                t_glambda::COLLECTs(iN, iMaxCan, pdD[0], pdLef[0], pdLeft[0], pdRight[0],
                                    dChic, pdDist, &pdEnd[0], piNcan, pdDisall, pdCands, &dTmax, &iMax);
                t_glambda::BACKTs(iN, &i, pdEnd, pdDist, pdLef, pdLeft, &isEnde);
            }
            if (*piNcan > 10000)
                break;
            if (isEnde)
            {
                break;
            }
        }
        dDminn = 1e20;

        for (i = 1; i <= ((*piNcan) < iMaxCan ? (*piNcan) : iMaxCan); i++)
        {
            if (pdDisall[i - 1] < dDminn)
            {
                dDminn = pdDisall[i - 1];
                *piPos = i;
            }
            for (j = 1; j <= iN; j++)
            {
                pdCands[(j - 1) * iMaxCan + i - 1] += pdA[j - 1];
            }
        }

        return;
    }

    void t_glambda::LAMBDA4(int iMaxCan, int iN, double *pdQ, double *pdA, int *piNcan, int *piPos, double *pdCands, double *pdDisall, double *boot)
    {
        double *pdL = new double[iN * iN];
        double *pdD = new double[iN];
        double *pdZt = new double[iN * iN];
        double *pdV1 = new double[iN];
        double *pdV2 = new double[iN + 1];
        double *pdV3 = new double[iN + 1];
        double *pdV4 = new double[iN];
        double *pdV5 = new double[iN];
        double *pdV6 = new double[iN];
        double *pdAk = new double[iN];
        int i, j, k, iz;
        double dH;
        double dEps;
        double dChi_1;

        double dt;

        /* initialize Zt=unit matrix*/
        iz = 0;
        for (i = 1; i <= iN; i++)
        {
            for (j = 1; j <= iN; j++)
            {
                iz++;
                pdL[(i - 1) * iN + j - 1] = pdQ[iz - 1];
                pdZt[(i - 1) * iN + j - 1] = 0.0;
            }
            pdZt[(i - 1) * iN + i - 1] = 1e0;
        }

        /*make estimates in 'a' between - 1 and + 1 by subtracting an
        integer number, store the increments in ak(= shift the centre
        of the ellipsoid over the grid by an integer translation)*/
        for (i = 1; i <= iN; i++)
        {
            if (pdA[i - 1] >= 0)
            {
                pdV1[i - 1] = pdA[i - 1] - floor(pdA[i - 1]);
            }
            else
            {
                pdV1[i - 1] = pdA[i - 1] - ceil(pdA[i - 1]);
            }

            pdAk[i - 1] = pdA[i - 1] - pdV1[i - 1];
            pdA[i - 1] = pdV1[i - 1];
        }

        /*make the L_1   D_1   L_1    decomposition of the variance - covariance matrix Q_hat{ a }*/
        dEps = 1e-9;

        if (t_glambda::FMFAC6(pdL, pdD, iN, dEps) < 0)
        {
            delete[] pdL;
            pdL = nullptr;
            delete[] pdD;
            pdD = nullptr;
            delete[] pdZt;
            pdZt = nullptr;
            delete[] pdV1;
            pdV1 = nullptr;
            delete[] pdV2;
            pdV2 = nullptr;
            delete[] pdV3;
            pdV3 = nullptr;
            delete[] pdV4;
            pdV4 = nullptr;
            delete[] pdV5;
            pdV5 = nullptr;
            delete[] pdV6;
            pdV6 = nullptr;
            delete[] pdAk;
            pdAk = nullptr;

            throw("ERROR in FMFAC6: LD failed");
        }

        /*compute the Z - transformation based on L and D of Q,
        ambiguities are transformed according to \hat{ z } = Z^* \hat{ a }*/
        t_glambda::SRC1i(iN, pdL, pdD, pdA, pdZt);

        *boot = 1.0;
        for (i = 0; i < iN; i++)
        {
            dt = sqrt(pdD[i]);
            *boot *= pBootStrapping(dt);
        }

        /*For the search we need L and D of Q^{ -1 }, see section 4.1, or
        L^{ -1 } and D^{ -1 } of Q here(in our case we use of course
        the t_glambda - transformed L and D as they came from SRC1)*/
        t_glambda::INVLT2d(iN, pdL, pdL, pdV1);

        if (pDia == NULL)
            pDia = new double[iN];
        //... and D_1
        for (i = 1; i <= iN; i++)
        {
            pDia[i - 1] = pdD[i - 1];
            pdD[i - 1] = 1e0 / pdD[i - 1];
        }

        /*find a suitable Chi ^ 2 such that we have two candidates at minimum
        use an eps to make sure the two candidates are inside the ellipsoid*/
        dEps = 1e-6;
        t_glambda::CHIstrt4(iN, pdD, pdL, pdV1, pdV2, pdA, pdV3, pdV4);
        dChi_1 = pdV3[1] + dEps;

        /*find the two candidates with minimum norm*/
        t_glambda::FI71(dChi_1, iMaxCan, iN, pdA, pdD, pdL, pdV1, pdV2, pdV3, pdV4,
                        pdV5, pdV6, piNcan, pdDisall, pdCands, piPos);

        if (*piNcan >= 10000)
        {
            delete[] pdL;
            pdL = nullptr;
            delete[] pdD;
            pdD = nullptr;
            delete[] pdZt;
            pdZt = nullptr;
            delete[] pdV1;
            pdV1 = nullptr;
            delete[] pdV2;
            pdV2 = nullptr;
            delete[] pdV3;
            pdV3 = nullptr;
            delete[] pdV4;
            pdV4 = nullptr;
            delete[] pdV5;
            pdV5 = nullptr;
            delete[] pdV6;
            pdV6 = nullptr;
            delete[] pdAk;
            pdAk = nullptr;

            throw("ERROR in FMFAC6: D failed");
        }
        /*compute a = Z^-*z*/
        for (k = 1; k <= iMaxCan; k++)
        {
            for (i = 1; i <= iN; i++)
            {
                pdV1[i - 1] = 0.0;
                for (j = 1; j <= iN; j++)
                {
                    pdV1[i - 1] += pdZt[(i - 1) * iN + j - 1] * pdCands[(j - 1) * iMaxCan + k - 1];
                }
            }
            for (j = 1; j <= iN; j++)
            {
                pdCands[(j - 1) * iMaxCan + k - 1] = pdV1[j - 1];
            }
        }

        /*...and add the increment to them*/
        for (i = 1; i <= iN; i++)
        {
            pdA[i - 1] = pdCands[(i - 1) * iMaxCan + *piPos - 1] + pdAk[i - 1];
        }

        /*'sort' the vector of squared norms in increasing order
        (if ipos equals 2, the best candidate is in the second place : so reverse disall)*/
        if (*piPos == 2)
        {
            dH = pdDisall[0];
            pdDisall[0] = pdDisall[1];
            pdDisall[1] = dH;

            for (i = 1; i <= iN; i++)
            {
                dH = pdCands[(i - 1) * iMaxCan];
                pdCands[(i - 1) * iMaxCan] = pdCands[(i - 1) * iMaxCan + 1];
                pdCands[(i - 1) * iMaxCan + 1] = dH;
            }
        }
        delete[] pdL;
        pdL = nullptr;
        delete[] pdD;
        pdD = nullptr;
        delete[] pdZt;
        pdZt = nullptr;
        delete[] pdV1;
        pdV1 = nullptr;
        delete[] pdV2;
        pdV2 = nullptr;
        delete[] pdV3;
        pdV3 = nullptr;
        delete[] pdV4;
        pdV4 = nullptr;
        delete[] pdV5;
        pdV5 = nullptr;
        delete[] pdV6;
        pdV6 = nullptr;
        delete[] pdAk;
        pdAk = nullptr;
        return;
    }

    double t_glambda::pBootStrapping(const double &sig)
    {
        const double BootSt_2Phi_sigRecip_1[461] =
            {
                1.00000000000, 1.00000000000, 1.00000000000, 1.00000000000, 1.00000000000, 0.99999999999, 0.99999999999, 0.99999999999, 0.99999999999, 0.99999999998,
                0.99999999997, 0.99999999996, 0.99999999995, 0.99999999994, 0.99999999992, 0.99999999989, 0.99999999985, 0.99999999981, 0.99999999975, 0.99999999968,
                0.99999999959, 0.99999999947, 0.99999999933, 0.99999999915, 0.99999999892, 0.99999999864, 0.99999999830, 0.99999999788, 0.99999999736, 0.99999999672,
                0.99999999596, 0.99999999502, 0.99999999390, 0.99999999255, 0.99999999092, 0.99999998898, 0.99999998667, 0.99999998393, 0.99999998068, 0.99999997684,
                0.99999997233, 0.99999996703, 0.99999996082, 0.99999995357, 0.99999994514, 0.99999993534, 0.99999992399, 0.99999991087, 0.99999989575, 0.99999987835,
                0.99999985840, 0.99999983555, 0.99999980946, 0.99999977971, 0.99999974588, 0.99999970748, 0.99999966399, 0.99999961482, 0.99999955936, 0.99999949690,
                0.99999942670, 0.99999934794, 0.99999925975, 0.99999916117, 0.99999905115, 0.99999892860, 0.99999879229, 0.99999864095, 0.99999847317, 0.99999828748,
                0.99999808226, 0.99999785581, 0.99999760630, 0.99999733179, 0.99999703020, 0.99999669931, 0.99999633680, 0.99999594015, 0.99999550675, 0.99999503379,
                0.99999451832, 0.99999395722, 0.99999334720, 0.99999268480, 0.99999196637, 0.99999118807, 0.99999034588, 0.99998943558, 0.99998845273, 0.99998739270,
                0.99998625064, 0.99998502150, 0.99998369998, 0.99998228058, 0.99998075754, 0.99997912490, 0.99997737643, 0.99997550567, 0.99997350593, 0.99997137025,
                0.99996909141, 0.99996666195, 0.99996407415, 0.99996132003, 0.99995839134, 0.99995527956, 0.99995197591, 0.99994847134, 0.99994475652, 0.99994082187,
                0.99993665752, 0.99993225331, 0.99992759885, 0.99992268343, 0.99991749610, 0.99991202562, 0.99990626047, 0.99990018888, 0.99989379879, 0.99988707788,
                0.99988001356, 0.99987259297, 0.99986480299, 0.99985663023, 0.99984806106, 0.99983908158, 0.99982967764, 0.99981983483, 0.99980953852, 0.99979877381,
                0.99978752559, 0.99977577847, 0.99976351689, 0.99975072503, 0.99973738684, 0.99972348608, 0.99970900630, 0.99969393082, 0.99967824278, 0.99966192513,
                0.99964496062, 0.99962733182, 0.99960902114, 0.99959001080, 0.99957028286, 0.99954981923, 0.99952860168, 0.99950661182, 0.99948383112, 0.99946024094,
                0.99943582250, 0.99941055690, 0.99938442516, 0.99935740815, 0.99932948670, 0.99930064151, 0.99927085322, 0.99924010239, 0.99920836952, 0.99917563504,
                0.99914187933, 0.99910708275, 0.99907122558, 0.99903428811, 0.99899625058, 0.99895709323, 0.99891679629, 0.99887533998, 0.99883270454, 0.99878887021,
                0.99874381725, 0.99869752596, 0.99864997666, 0.99860114972, 0.99855102555, 0.99849958461, 0.99844680744, 0.99839267462, 0.99833716683, 0.99828026482,
                0.99822194940, 0.99816220152, 0.99810100219, 0.99803833254, 0.99797417380, 0.99790850734, 0.99784131462, 0.99777257724, 0.99770227694, 0.99763039558,
                0.99755691517, 0.99748181789, 0.99740508603, 0.99732670206, 0.99724664863, 0.99716490852, 0.99708146471, 0.99699630033, 0.99690939872, 0.99682074337,
                0.99673031799, 0.99663810645, 0.99654409284, 0.99644826144, 0.99635059672, 0.99625108338, 0.99614970631, 0.99604645060, 0.99594130160, 0.99583424482,
                0.99572526604, 0.99561435123, 0.99550148659, 0.99538665856, 0.99526985380, 0.99515105920, 0.99503026190, 0.99490744925, 0.99478260885, 0.99465572855,
                0.99452679643, 0.99439580080, 0.99426273023, 0.99412757353, 0.99399031975, 0.99385095819, 0.99370947841, 0.99356587020, 0.99342012360, 0.99327222891,
                0.99312217668, 0.99296995769, 0.99281556299, 0.99265898388, 0.99250021191, 0.99233923886, 0.99217605681, 0.99201065803, 0.99184303508, 0.99167318077,
                0.99150108813, 0.99132675048, 0.99115016135, 0.99097131455, 0.99079020413, 0.99060682436, 0.99042116979, 0.99023323520, 0.99004301562, 0.98985050631,
                0.98965570280, 0.98945860082, 0.98925919638, 0.98905748571, 0.98885346527, 0.98864713177, 0.98843848214, 0.98822751357, 0.98801422345, 0.98779860942,
                0.98758066935, 0.98736040131, 0.98713780363, 0.98691287485, 0.98668561372, 0.98645601923, 0.98622409057, 0.98598982715, 0.98575322862, 0.98551429480,
                0.98527302576, 0.98502942175, 0.98478348325, 0.98453521091, 0.98428460563, 0.98403166847, 0.98377640072, 0.98351880383, 0.98325887948, 0.98299662952,
                0.98273205600, 0.98246516116, 0.98219594742, 0.98192441738, 0.98165057382, 0.98137441971, 0.98109595820, 0.98081519259, 0.98053212639, 0.98024676323,
                0.97995910696, 0.97966916156, 0.97937693119, 0.97908242016, 0.97878563295, 0.97848657419, 0.97818524867, 0.97788166132, 0.97757581723, 0.97726772163,
                0.97695737991, 0.97664479759, 0.97632998033, 0.97601293393, 0.97569366433, 0.97537217760, 0.97504847994, 0.97472257770, 0.97439447732, 0.97406418539,
                0.97373170862, 0.97339705384, 0.97306022799, 0.97272123813, 0.97238009145, 0.97203679524, 0.97169135688, 0.97134378388, 0.97099408386, 0.97064226453,
                0.97028833371, 0.96993229931, 0.96957416934, 0.96921395192, 0.96885165523, 0.96848728757, 0.96812085733, 0.96775237296, 0.96738184302, 0.96700927615,
                0.96663468106, 0.96625806654, 0.96587944147, 0.96549881479, 0.96511619553, 0.96473159277, 0.96434501569, 0.96395647350, 0.96356597551, 0.96317353108,
                0.96277914962, 0.96238284062, 0.96198461363, 0.96158447823, 0.96118244410, 0.96077852092, 0.96037271847, 0.95996504656, 0.95955551504, 0.95914413383,
                0.95873091286, 0.95831586215, 0.95789899172, 0.95748031167, 0.95705983210, 0.95663756317, 0.95621351508, 0.95578769805, 0.95536012234, 0.95493079825,
                0.95449973610, 0.95406694624, 0.95363243906, 0.95319622495, 0.95275831434, 0.95231871770, 0.95187744550, 0.95143450824, 0.95098991643, 0.95054368061,
                0.95009581134, 0.94964631918, 0.94919521471, 0.94874250853, 0.94828821126, 0.94783233351, 0.94737488592, 0.94691587911, 0.94645532375, 0.94599323049,
                0.94552960997, 0.94506447288, 0.94459782987, 0.94412969162, 0.94366006879, 0.94318897207, 0.94271641211, 0.94224239960, 0.94176694519, 0.94129005956,
                0.94081175336, 0.94033203725, 0.93985092188, 0.93936841788, 0.93888453589, 0.93839928654, 0.93791268045, 0.93742472821, 0.93693544042, 0.93644482767,
                0.93595290052, 0.93545966953, 0.93496514524, 0.93446933818, 0.93397225886, 0.93347391778, 0.93297432541, 0.93247349222, 0.93197142864, 0.93146814510,
                0.93096365201, 0.93045795974, 0.92995107865, 0.92944301910, 0.92893379140, 0.92842340583, 0.92791187269, 0.92739920221, 0.92688540462, 0.92637049012,
                0.92585446889, 0.92533735107, 0.92481914678, 0.92429986613, 0.92377951918, 0.92325811596, 0.92273566650, 0.92221218077, 0.92168766874, 0.92116214032,
                0.92063560541, 0.92010807387, 0.91957955554, 0.91905006022, 0.91851959769, 0.91798817767, 0.91745580989, 0.91692250401, 0.91638826967, 0.91585311649,
                0.91531705404, 0.91478009186, 0.91424223946, 0.91370350631, 0.91316390185, 0.91262343548, 0.91208211658, 0.91153995446, 0.91099695844, 0.91045313777,
                0.90990850167, 0.90936305934, 0.90881681992, 0.90826979253, 0.90772198625, 0.90717341012, 0.90662407314, 0.90607398427, 0.90552315245, 0.90497158656,
                0.90441929545};
        int i, j;
        double dt;
        double dRes = 0.0;

        if (sig >= 0.3)
            dRes = 0.9;
        else if (sig <= 0.07)
            dRes = 1.0;
        else
        {
            dt = (sig - 0.07) * 2000.0; //  /0.0005;
            i = int(dt);
            j = i + 1;

            dRes = (j - dt) * BootSt_2Phi_sigRecip_1[i] + (dt - i) * BootSt_2Phi_sigRecip_1[j];

        }

        return dRes;
    }

}
