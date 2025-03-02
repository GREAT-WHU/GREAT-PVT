/**
 * @file         glambda.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        integer estimation with the LAMBDA method.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GLAMBDA_H
#define GLAMBDA_H

#include "gexport/ExportLibGREAT.h"
using namespace std;

namespace great
{

    /**
    *@ brief class for LAMADA search finding fixing ambiguity.
    */
    class LibGREAT_LIBRARY_EXPORT t_glambda
    {
    public:
        /** @brief default constructor. */
        t_glambda(){};

        /** @brief default destructor. */
        virtual ~t_glambda()
        {
            if (pDia)
                delete[] pDia;
        };

        /**
        * @brief integer estimation with the LAMBDA method
        * @param[in] iMaxCan
        * @param[in] iN        dimension of matrix
        * @param[in] pdQ
        * @param[in] pdA
        * @param[in] piNcan
        * @param[in] piPos
        * @param[in] pdCands   2-dimensional array to store the candidates
        * @param[in] boot   bootstrapping rate in amb fix
        * @param[in] pdDisall  according squared norms \hat{a}-\check{a} dA, piNcan, piPos, pdCands, pdDisal
        * @return void
        */
        void LAMBDA4(int iMaxCan, int iN, double *pdQ, double *pdA, int *piNcan, int *piPos, double *pdCands, double *pdDisall, double *boot);

        /*static void LAMBDA4(int iMaxcan, int n, double *arrdQ, double* arrdA, int* iNcan, int* iPos, double* arrdCands, double* arrdIsall);
        static void STOREs(int iCan, int iPos, int *piMax, double t, double *pdTmax, double *arrpdDist, double *arrpdDisall,
        double *arrpdCands, int iMaxCan, int n);*/

        /**
        * @brief backtrack in the search tree, internal subroutine for FI71.
        * @param[in] iN      dimension of matrix
        * @param[in] piI     level in the tree
        * @param[in] pdEnd   dp work vector
        * @param[in] pdDist  difference between the integer tried and \hat{a}_i
        * @param[in] pdLef   dp work vector
        * @param[in] pdLeft  dp work vector
        * @param[in] pdEnde  if .true., then search is done
        * @param[out] piI pdDist pdLeft pdEnde
        * return void
        */
        void BACKTs(int iN, int *piI, double *pdEnd, double *pdDist, double *pdLef, double *pdLeft, bool *pdEnde);

        /**
        * @brief Updates integral Z (transposed inverse); only column `first' until `last'.
        *    see: the LAMBDA method for integer ambiguity estimation: implementation aspects (section 3.5 & 3.9).
        * @param[in] iFirst   first column to be updated
        * @param[in] iLast      last column to be updated
        * @param[in] iN          dimension of the system
        * @param[in] pdL      lower triangular matrix L
        * @param[in] pdA      Z (transposed) a, with a the vector of unknowns
        * @param[in] pdZti      Z (transposed inverse) matrix
        * @param[out] pdL pdA pdZti
        * @return void
        */
        void ZTRANi(int iFirst, int iLast, int iN, double *pdL, double *pdA, double *pdZti);

        /**
        * @brief stores integer vectors and corresponding distances.
        * @param[in] iCan       Min (number of vectors found until now, MaxCan)
        * @param[in] iPos        position in disall/cands to put the new found vec
        * @param[in] piMax        position in disall/cands of the vector with the largest distance of the ican vectors with minimum distance found until now
        * @param[in] dT         distance of the new found vector
        * @param[in] pdTmax        the largest distance of the ican vectors with minimum distance found until now
        * @param[in] pdDist        difference between the integer tried and \hat{a}_n
        * @param[in] pdDisall    2d-array to store the integer vectors
        * @param[in] pdCands    distance of the MaxCan integer vectors
        * @param[in] iMaxCan    number of integer vectors required
        * @param[in] iN            dimension of the system (number of DD ambiguities)
        * @param[out] piMax  pdTmax  pdDist  pdDisall
        * @return void
        */
        void STOREs(int iCan, int iPos, int *piMax, double dT, double *pdTmax, double *pdDist, double *pdDisall, double *pdCands, int iMaxCan, int iN);

        /**
        * @brief collects integer vectors and corresponding squared distances.
        * @param[in] iN         dimension of the system
        * @param[in] iMaxCan    number of minimum integer vectors requiered
        * @param[in] dD_1        first element of diagonal matrix D
        * @param[in] dLef_1     first element of dp work vector lef
        * @param[in] dLeft_1    first element of dp work vector left
        * @param[in] dRight_1   first element of dp work vector right
        * @param[in] dChic        Chi squared
        * @param[in] pdDist        difference between the integer tried and \hat{a}_n
        * @param[in] pdEnd_1    first element of dp work vector end
        * @param[in] piNcan        number of integer vectors found
        * @param[in] pdDisall | 2-dimensional array to store the candidates
        * @param[in] pdCands  | according squared norms \hat{a}-\check{a}
        * @param[in] pdTmax     the largest distance of the Min (ncan,MaxCan) vectors with minimum distance found until now
        * @param[in] piMax  position in disall/cands of the vector with the largest distance of the Min (ncan,MaxCan) vectors
        *                    with minimum distance found until now
        * @param[out] pdDist pdEnd_1 piNcan pdDisall  pdCands pdTmax  piMax
        * @return void
        */
        void COLLECTs(int iN, int iMaxCan, double dD_1, double dLef_1, double dLeft_1, double dRight_1, double dChic, double *pdDist,
                      double *pdEnd_1, int *piNcan, double *pdDisall, double *pdCands, double *pdTmax, int *piMax);

        /**
        * @brief transpose(L) D L factorization of Q, L over-writes Q bordering method, computation of Googe number (section 3.3)s.
        * @param[in] pdL   symmetric lower triangular matrix Q to be factored
        * @param[in] pdD   diagonal
        * @param[in] iN    dimension of the matrix
        * @param[in] dEps  if the Googe number is smaller than eps the matrix is considered to be singular
        * @param[out] pdL pdD
        * @return void
        */
        int FMFAC6(double *pdL, double *pdD, int iN, double dEps);

        /**
        * @brief compute the inverse of a lower triangular matrix Lm may over write L.
        * @param[in] iN    dimension of the matrix
        * @param[in] pdL   lower triangular matrix
        * @param[in] pdLm  inverse of L (also lower triangular)
        * @param[in] pdVec double precision work array with lenght n
        * @param[out] pdL, pdLm, pdVec
        * @return void
        */
        void INVLT2d(int iN, double *pdL, double *pdLm, double *pdVec);

        /**
        * @brief Computation of the Z matrix .
        * @param[in] iN      dimension of the system
        * @param[in] pdL     lower triangular matrix L
        * @param[in] pdD     diagonal matrix D stored as a one-dimensional array
        * @param[in] pdA     Z (transposed) a, with a the original vector of unknowns
        * @param[in] pdZti   Z (transposed inverse) transformation matrix
        * @param[out] pdL, pdD, pdA, pdZti
        * @return void
        */
        void SRC1i(int iN, double *pdL, double *pdD, double *pdA, double *pdZti);

        /**
        * @brief computes squared distance of partially rounded float vectors to
        *               the float vector in the metric of the variance - covariance
        *               matrix(see `The LAMBDA method for integer ambiguity estimation :
        *               implementation aspects', 5.8: The volume of the ellipsoidal region.
        * @param[in] iN      number of ambiguities
        * @param[in] pdL     lower triangular matrix: Q   = L D L, although L is lower triangular, in this example,
        *                    program L is stored column-wise in a 2-dimensional array, to avoid the necessity of a dedicated storage scheme.
        * @param[in] pdD     diagonal matrix
        * @param[in] pdDist  double precision vector of length n
        * @param[in] pdE     double precision vector of length n
        * @param[in] pdA     float solution
        * @param[in] dTm     tm(1) is smallest norm, tm(2) one-but-smallest
        * @param[in] dT         double precision vector of length n
        * @param[out] pdL, pdD, pdA, pdDist,pdE,pdT
        * @return void
        */
        void CHIstrt4(int iN, double *pdD, double *pdL, double *pdDist, double *pdE, double *pdA, double dTm[2], double *pdT);

        /**
        * @brief takes inproduct of column 'icol' of triangular unit matrix L and vector a (exclusive the diagonal element).
        * @param[in] iN      length of a
        * @param[in] pdL     triangular matrix L
        * @param[in] pdA     vector a
        * @param[in] iCol    see rol
        * @param[out] pdL, pdA
        * @return dDINKi
        */
        double DINKi(int iN, double *pdL, double *pdA, int iCol);

        /**
        * @brief 'ceiling' (rounding towards +infinity) of a double precision number.
        * @param[in]  dD     double precision number
        * @param[out] iJNT2  ceiled number
        * @return iJNT2
        */
        int JNT2(double dD);

        /**
        * @brief finds 'MaxCan' integer vectors whose distance to the the real vector 'a' is minimal in the metric of Q=transpose(L) D L. Only
        *        integer vectors with a distance less than sqrt(Chic) are regarded.
        * @param[in] dChic      Chi squared
        * @param[in] iMaxCan    number of minimum integer vectors requiered
        * @param[in] iN            dimension of matrix
        * @param[in] pdA        the vector with real valued estimates \hat{a} (float solution)
        * @param[in] pdD        | diagonal matrix           -1        *
        * @param[in] pdL        | lower triangular matrix: Q   = L D L
        *                        | although L is lower triangular, in this example
        *                        | program L is stored column-wise in a 2-dimensional
        *                        | array, to avoid the necessity of a dedicated
        *                        | storage scheme.
        * @param[in] pdLef        dp work vector with length = n
        * @param[in] pdLeft     dp work vector with length = n+1
        * @param[in] pdRight    dp work vector with length = n+1
        * @param[in] pdDis        difference between the integer tried and \hat{a}_i,length = n
        * @param[in] pdEnd        dp work vector with length = n
        * @param[in] pdDq        dp work vector with length = n
        * @param[in] piNcan        number of integer vectors found
        * @param[in] pdDisall    | 2-dimensional array to store the candidate
        * @param[in] pdCands    | according squared norms \hat{a}-\check{a}s
        * @param[in] piPos        column number in 'cands' where the candidate belonging to the minimum distance is stored
        * @param[out] pdA, pdD, dpdL, pdLef, pdLeft, pdRight,pdDist, pdEnd, pdDq, ipiNcan, pdDisall, pdCands, piPos
        * @return void
        */
        void FI71(double dChic, int iMaxCan, int iN, double *pdA, double *pdD, double *pdL, double *pdLef, double *pdLeft, double *pdRight,
                  double *pdDist, double *pdEnd, double *pdDq, int *piNcan, double *pdDisall, double *pdCands, int *piPos);

        double pBootStrapping(const double &sig);

    public:
        double *pDia = NULL;
    };

}

#endif