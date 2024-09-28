/**
 *
 * @verbatim
     History
      -1.0    Qian Zhang     2020-04-01    creat the file.
   @endverbatim
 * Copyright (c) 2018, Wuhan University. All rights reserved.
 *
 * @file        gallopl.h
 * @brief        Storage the oceanpoleload data.
 *
 * @author      Qian Zhang, Wuhan University
 * @version        1.0.0
 * @date        2020-04-01
 */

#ifndef GALLOPL_H
#define GALLOPL_H

#include <string>
#include <map>

#include "newmat/newmat.h"
#include "gdata/gdata.h"
#include "gutils/gtime.h"
#include "gutils/gsysconv.h"

using namespace std;

namespace gnut
{
    class LibGnut_LIBRARY_EXPORT t_gallopl : public t_gdata
    {
    public:
        /** @brief default constructor. */
        t_gallopl();
        /** @brief default destructor. */
        ~t_gallopl();

        /**
        * @brief get opl data from _ren_r and _ren_l.
        * @return  is successful or not.
        * @param [in]  ell    antenna coordinates.
        * @param [in]  rne_r    ocean pole load tide coefficients from Desai (2002): u_r^R, u_n^R, u_e^R.
        * @param [in]  rne_i    ocean pole load tide coefficients from Desai (2002): u_r^I, u_n^I, u_e^I.
        */
        int data(const t_gtriple &ell, t_gtriple &rne_r, t_gtriple &rne_i);

        /** @brief kart2ell. */
        t_gtriple kart2ell(const t_gtriple &p, const double &a, const double &b);

    protected:
        map<t_gpair, t_gtriple> _rne_r;    ///< ocean pole load tide coefficients from Desai (2002): u_r^R, u_n^R, u_e^R.
        map<t_gpair, t_gtriple> _rne_i;    ///< ocean pole load tide coefficients from Desai (2002): u_r^I, u_n^I, u_e^I.
    };
}

#endif