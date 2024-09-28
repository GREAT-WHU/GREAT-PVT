/**
 * @file         gfltmatrix.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Matrix for NEQ,W,observ_equations
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GFLTMATRIX_H
#define GFLTMATRIX_H

#include "gexport/ExportLibGREAT.h"
#include <gset/gsetproc.h>
#include <Eigen/Core>
#include "newmat/newmat.h"
#include "gutils/gtypeconv.h"
#include "gmodels/gbasemodel.h"

using namespace std;
using namespace gnut;
namespace great
{

    /**
    * @brief  Matrix for storage observe equations of B,P,L
    */
    class LibGREAT_LIBRARY_EXPORT t_gfltEquationMatrix : public t_gbaseEquation
    {
    public:
        /** @brief default constructor */
        t_gfltEquationMatrix();

        /** @brief default destructor  */
        ~t_gfltEquationMatrix();

        /**
        * @brief add equations
        * @param[in] B_value coeff of observ equation
        * @param[in] P_value weight of observ equation
        * @param[in] l_value res of observ equaion
        */
        void add_equ(const vector<pair<int, double>> &B_value, const double &P_value, const double &l_value, const string &stie_name, const string &sat_name, const t_gobscombtype &obscombtype, const bool &is_newamb);
        void add_equ(const t_gfltEquationMatrix &Other);
        
        /**
        * @brief change equations to newmat format
        * @param[out] B_value coeff of observ equation newmat format
        * @param[out] P_value weight of observ equation newmat format
        * @param[out] l_value res of observ equaion newmat format
        */
        void chageNewMat(Matrix &B_value, SymmetricMatrix &P_value, ColumnVector &l_value, const int &par_num);

        /**
        * @brief get numbers of equations
        * @return size of equations
        */
        int num_equ() const;

        /**
        * @brief get resiuals of equations
        * @return LTPL of equations
        */
        double res_equ() const;

        /**
         * @brief Get the satname
         * @param[in]  equ_idx   index of equ
         * @return string satellite name
         */
        string get_satname(int equ_idx) const;

        /**
         * @brief Get the obscombtype
         * @param[in]  equ_idx   index of equ
         * @return t_gobscombtype type of observation
         */
        t_gobscombtype get_obscombtype(int equ_idx) const;

    protected:
        vector<pair<string, string>> _site_sat_pairlist; ///< station & satellite name
        vector<t_gobscombtype> _obstypelist;             ///< observation type
        vector<bool> _newamb_list;                       ///< whether new arc
    };

}

#endif /*  GLSQMAT_H  */