
/**
*
* @verbatim
    History
    2011-04-26  PV: created
    2012-04-06  JD: extracted matrix conversion utilities from old utils.h

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gmatrixconv.h
* @brief       Purpose: matrix conversion utilities
* @author      PV
* @version     1.0.0
* @date        2011-04-26
*
*/

#ifndef GMATRIXCONV_H
#define GMATRIXCONV_H

#include <ostream>
#include <vector>

#include <Eigen/Dense>
#include "gexport/ExportLibGnut.h"

namespace gnut
{

    /**
     * @brief remove   r_th row and c_th column in SymMatrix
     * @param[in]     &        a Eigen::MatrixXd
     * @param[in]     row        the row
     * @param[in]     col        the column
     * @return void
     */
    LibGnut_LIBRARY_EXPORT void Matrix_remRC(Eigen::MatrixXd&, int row, int col);

    /** @brief remove rows and columns stored in set. */
    LibGnut_LIBRARY_EXPORT void Matrix_rem(Eigen::MatrixXd&, std::vector<int>&);

    /**
     * @brief add zero r_th row and c_th column in Eigen::MatrixXd
     * @param[in]     &        a Eigen::MatrixXd
     * @param[in]     row        the row
     * @param[in]     col        the column
     * @return void
     */
    LibGnut_LIBRARY_EXPORT void Matrix_addRC(Eigen::MatrixXd&, int row, int col);

    /**
     * @brief remove r_th row in Eigen::MatrixXd (zhshen).
     * @param[in]     &        a Eigen::MatrixXd
     * @param[in]     row        the row
     * @return void
     */
    LibGnut_LIBRARY_EXPORT void Matrix_remR(Eigen::MatrixXd&, int row);

    /**
     * @brief remove rows row stored in set (zhshen).
     * @param[in]     &        a Eigen::MatrixXd
     * @param[in]     &        a int vector
     * @return void
     */
    LibGnut_LIBRARY_EXPORT void Matrix_remR(Eigen::MatrixXd&, std::vector<int>&);

    /** @brief remove r_th row in Eigen::VectorXd. */
    LibGnut_LIBRARY_EXPORT void Matrix_remR(Eigen::VectorXd&, int row);

    /** @brief remove rows stored in set from Eigen::VectorXd. */
    LibGnut_LIBRARY_EXPORT void Matrix_remR(Eigen::VectorXd&, std::vector<int>&);

    /**
     * @brief add zero r_th row and r_th column in DiagMatrix.
     * @param[in]     &        a Eigen::DiagonalMatrix<double, Eigen::Dynamic>
     * @param[in]     row        the row
     * @return void
     */
    LibGnut_LIBRARY_EXPORT void Matrix_addRC(Eigen::DiagonalMatrix<double, Eigen::Dynamic>&, int row);

    /**
     * @brief swap a_th and b_th row and column in SymMatrix.
     * @param[in]     &        a Eigen::MatrixXd
     * @param[in]     a        the row
     * @param[in]     b        the column
     * @return void
     */
    LibGnut_LIBRARY_EXPORT void Matrix_swap(Eigen::MatrixXd&, int a, int b);

    /**
     * @brief copy row and col.
     * @param[in]     &        a Eigen::MatrixXd
     * @param[in]     r        the row
     * @param[in]     c        the column
     * @return void
     */
    LibGnut_LIBRARY_EXPORT void Matrix_cpRC(Eigen::MatrixXd, Eigen::MatrixXd&, int r, int c);

    /** @brief add zero r_th row in Column Vector. */
    LibGnut_LIBRARY_EXPORT void Vector_add(Eigen::VectorXd&, int row);

    /** @brief Rotation Eigen::MatrixXd. */
    LibGnut_LIBRARY_EXPORT Eigen::MatrixXd rotX(double Angle);
    LibGnut_LIBRARY_EXPORT Eigen::MatrixXd rotY(double Angle);
    LibGnut_LIBRARY_EXPORT Eigen::MatrixXd rotZ(double Angle);
} // namespace gnut

#endif
