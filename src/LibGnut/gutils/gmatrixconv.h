
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

#include "newmat/newmat.h"
#include "gexport/ExportLibGnut.h"

using namespace std;
namespace gnut
{

    /**
    * @brief remove   r_th row and c_th column in SymMatrix
    * @param[in]     &        a SymmetricMatrix
    * @param[in]     row        the row
    * @param[in]     col        the column
    * @return void
    */
    LibGnut_LIBRARY_EXPORT void Matrix_remRC(SymmetricMatrix &, int row, int col);

    /** @brief remove rows and columns stored in set. */
    /**
    * @brief remove rows and columns stored in set
    * @param[in]     &        a SymmetricMatrix
    * @param[in]     &        a int vector
    * @return void
    */
    LibGnut_LIBRARY_EXPORT void Matrix_rem(SymmetricMatrix &, vector<int> &);

    /**
    * @brief remove rows and columns stored in set
    * @param[in]     &        a SymmetricMatrix
    * @param[in]     row        the row
    * @param[in]     col        the column
    * @return void
    */
    LibGnut_LIBRARY_EXPORT void Matrix_addRC(SymmetricMatrix &, int row, int col);

    /**
    * @brief add zero r_th row and c_th column in Matrix
    * @param[in]     &        a Matrix
    * @param[in]     row        the row
    * @param[in]     col        the column
    * @return void
    */
    LibGnut_LIBRARY_EXPORT void Matrix_addRC(Matrix &, int row, int col);

    /**
    * @brief remove r_th row in Matrix (zhshen).
    * @param[in]     &        a Matrix
    * @param[in]     row        the row
    * @return void
    */
    LibGnut_LIBRARY_EXPORT void Matrix_remR(Matrix &, int row);

    /**
    * @brief remove rows row stored in set (zhshen).
    * @param[in]     &        a Matrix
    * @param[in]     &        a int vector
    * @return void
    */
    LibGnut_LIBRARY_EXPORT void Matrix_remR(Matrix &, vector<int> &);

    /**
    * @brief add zero r_th row and r_th column in DiagMatrix.
    * @param[in]     &        a DiagonalMatrix
    * @param[in]     row        the row
    * @return void
    */
    LibGnut_LIBRARY_EXPORT void Matrix_addRC(DiagonalMatrix &, int row);

    /**
    * @brief swap a_th and b_th row and column in SymMatrix.
    * @param[in]     &        a SymmetricMatrix
    * @param[in]     a        the row
    * @param[in]     b        the column
    * @return void
    */
    LibGnut_LIBRARY_EXPORT void Matrix_swap(SymmetricMatrix &, int a, int b);

    /**
    * @brief copy row and col.
    * @param[in]     &        a SymmetricMatrix
    * @param[in]     r        the row
    * @param[in]     c        the column
    * @return void
    */
    LibGnut_LIBRARY_EXPORT void Matrix_cpRC(SymmetricMatrix, SymmetricMatrix &, int r, int c);

    /** @brief add zero r_th row in Column Vector. */
    LibGnut_LIBRARY_EXPORT void Vector_add(ColumnVector &, int row);

    /** @brief Rotation Matrix. */
    LibGnut_LIBRARY_EXPORT Matrix rotX(double Angle);
    LibGnut_LIBRARY_EXPORT Matrix rotY(double Angle);
    LibGnut_LIBRARY_EXPORT Matrix rotZ(double Angle);
} // namespace

#endif