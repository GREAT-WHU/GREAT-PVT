/**
 * @file         gambiguity.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Judge whether ambiguities meet the requirement of fixing. 
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GBDECI_H
#define GBDECI_H

#include "gexport/ExportLibGREAT.h"
using namespace std;

namespace great
{
    /**
     * @brief class for Judge whether ambiguities meet the requirement of fixing. 
     */
    class LibGREAT_LIBRARY_EXPORT t_gbdeci
    {
    public:
        /**
         * @brief Construct a new t gbdeci object
         */
        t_gbdeci();
        /**
         * @brief Construct a new t gbdeci object
         * @param[in]  cutdev    current deviation
         * @param[in]  cutsig    current sigma
         * @param[in]  cutprob   current probability
         * @param[in]  cutdeci   current decision
         */
        t_gbdeci(const double &cutdev, const double &cutsig,
                 const double &cutprob, const double &cutdeci);
        /**
         * @brief Destroy the t gbdeci object
         */
        virtual ~t_gbdeci(){};
        /**
         * @brief judge whether ambiguities meet the requirement of fixing
         * @param[in]  dEst      estimated(real) bias value
         * @param[in]  dSigma    estimated uncertainty of bias value, scaled by nrms for widelane, unscaled for narrow lane
         * @param[in]  iIh       control for receiver ambiguity
         * @param[in]  dCutdev   dd deviation
         * @param[in]  dCutsig   dd sigma
         * @param[in]  dProb     dd probability
         * @param[in]  dDeci     dd decision
         */
        void bdeci(double dEst, double dSigma, int iIh, double dCutdev, double dCutsig, double &dProb, double &dDeci);

    private:

        /**
         * @brief get erfc
         * @param[in]  dX        
         * @return double 
         */
        double _erfc(double dX);
        /**
         * @brief get gammp
         * @param[in]  dA        
         * @param[in]  dX        
         * @return double
         */
        double _gammp(double dA, double dX);
        /**
         * @brief get gammq
         * @param[in]  dA        
         * @param[in]  dX        
         * @return double
         */
        double _gammq(double dA, double dX);
        /**
         * @brief get gammln
         * @param[in]  dX        
         * @return double
         */
        double _gammln(double dXx);
        /**
         * @brief get erfc
         * @param[in]  dGammcf   
         * @param[in]  dA        
         * @param[in]  dX        
         * @param[in]  dGln      
         */
        void _gcf(double &dGammcf, double dA, double dX, double &dGln);
        /**
         * @brief get gser
         * @param[in]  dGamser
         * @param[in]  dA
         * @param[in]  dX
         * @param[in]  dGln
         * @return double
         */
        void _gser(double &dGamser, double dA, double dX, double &dGln);

    private:
        double _cutdev;  ///< deviation
        double _cutsig;  ///< sigma
        double _cutprob; ///< probability
        double _cutdeci; ///< decision
    };
}

#endif
