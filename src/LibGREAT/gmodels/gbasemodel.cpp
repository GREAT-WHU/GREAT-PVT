/**
 * @file         gbasemodel.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        base interface model  
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gmodels/gbasemodel.h"

namespace great
{
    t_gbaseEquation::t_gbaseEquation()
    {
    }

    t_gbaseEquation::~t_gbaseEquation()
    {
    }

    t_gbaseEquation &t_gbaseEquation::operator+(t_gbaseEquation &Other)
    {
        this->B.insert(this->B.begin(), Other.B.begin(), Other.B.end());
        this->P.insert(this->P.begin(), Other.P.begin(), Other.P.end());
        this->l.insert(this->l.begin(), Other.l.begin(), Other.l.end());
        return *this;
    }

    t_gbasemodel::t_gbasemodel()
    {
    }

    t_gbasemodel::~t_gbasemodel()
    {
    }
}