/**
 * @file         gfltmatrix.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Matrix for NEQ,W,observ_equations
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gproc/gfltmatrix.h"
#include <Eigen/Dense>
#include <stdexcept>
#include <algorithm>
#include <emmintrin.h>
#include <assert.h>

#ifdef USE_OPENBLAS
#include "cblas.h"
#endif

namespace great
{

    t_gfltEquationMatrix::t_gfltEquationMatrix()
    {
    }

    t_gfltEquationMatrix::~t_gfltEquationMatrix()
    {
    }

    void t_gfltEquationMatrix::add_equ(const vector<pair<int, double>> &B_value, const double &P_value, const double &l_value, const string &stie_name, const string &sat_name, const t_gobscombtype &obscombtype, const bool &is_newamb)
    {
        B.push_back(B_value);
        P.push_back(P_value);
        l.push_back(l_value);
        this->_site_sat_pairlist.push_back(make_pair(stie_name, sat_name));
        this->_obstypelist.push_back(obscombtype);
        this->_newamb_list.push_back(is_newamb);
    }

    void t_gfltEquationMatrix::add_equ(const t_gfltEquationMatrix &Other)
    {
        for (int i = 0; i < Other.num_equ(); i++)
        {
            this->add_equ(Other.B[i], Other.P[i], Other.l[i], Other._site_sat_pairlist[i].first, Other._site_sat_pairlist[i].second, Other._obstypelist[i], false);
        }
    }

    void t_gfltEquationMatrix::chageNewMat(Matrix &B_value, SymmetricMatrix &P_value, ColumnVector &l_value, const int &par_num)
    {
        B_value.ReSize(B.size(), par_num);
        B_value = 0.0;
        for (unsigned int row = 0; row < B.size(); row++)
        {
            for (unsigned int col = 0; col < B[row].size(); col++)
            {
                B_value(row + 1, B[row][col].first) = B[row][col].second;
            }
        }
        P_value.ReSize(P.size());
        P_value = 0.0;
        for (unsigned int row = 0; row < P.size(); row++)
        {
            P_value(row + 1, row + 1) = P[row];
        }
        l_value.ReSize(l.size());
        l_value = 0.0;
        for (unsigned int row = 0; row < l.size(); row++)
        {
            l_value(row + 1) = l[row];
        }
    }

    int t_gfltEquationMatrix::num_equ() const
    {
        return B.size();
    }

    double t_gfltEquationMatrix::res_equ() const
    {
        double ans = 0;
        for (int i = 0; i < num_equ(); i++)
        {
            ans += P[i] * l[i] * l[i];
        }
        return ans;
    }

    string t_gfltEquationMatrix::get_satname(int equ_idx) const
    {
        return _site_sat_pairlist[equ_idx].second;
    }

    t_gobscombtype t_gfltEquationMatrix::get_obscombtype(int equ_idx) const
    {
        return _obstypelist[equ_idx];
    }


}