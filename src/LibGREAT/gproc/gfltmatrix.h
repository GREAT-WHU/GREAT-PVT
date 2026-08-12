/**
 * @file         gfltmatrix.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Eigen::MatrixXd for NEQ,W,observ_equations
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
#include <Eigen/Dense>
#include "gutils/gtypeconv.h"
#include "gmodels/gbasemodel.h"

using namespace gnut;
namespace great
{

    /**
     * @brief  Eigen::MatrixXd for storage observe equations of B,P,L
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
        void add_equ(const std::vector<std::pair<int, double>>& B_value,
                     const double& P_value,
                     const double& l_value,
                     const std::string& stie_name,
                     GSAT sat,
                     const t_gobscombtype& obscombtype,
                     const bool& is_newamb);
        void add_equ(const t_gfltEquationMatrix& Other);

        /**
         * @brief convert stored equations to Eigen matrices
         * @param[out] B_value coefficient matrix
         * @param[out] P_value weight matrix
         * @param[out] l_value residual vector
         */
        void toEigen(Eigen::MatrixXd& B_value, Eigen::MatrixXd& P_value, Eigen::VectorXd& l_value, const int& par_num);

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
         * @return satellite PRN
         */
        GSAT get_sat(int equ_idx) const;

        /**
         * @brief Get the obscombtype
         * @param[in]  equ_idx   index of equ
         * @return t_gobscombtype type of observation
         */
        t_gobscombtype get_obscombtype(int equ_idx) const;

    protected:
        std::vector<std::pair<std::string, GSAT>> _site_sat_pairlist; ///< station & satellite
        std::vector<t_gobscombtype> _obstypelist;             ///< observation type
        std::vector<bool> _newamb_list;                       ///< whether new arc
    };

} // namespace great

#endif /*  GLSQMAT_H  */
