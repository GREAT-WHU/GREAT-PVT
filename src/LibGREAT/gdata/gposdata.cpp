/**
 * @file         gposdata.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        pos data structure for storing pos data
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gposdata.h"
#include "gutils/gtypeconv.h"

gsins::t_gposdata::t_gposdata()
{
    id_type(LCI_POS);
    _ptr = 0;
}
gsins::t_gposdata::t_gposdata(t_spdlog spdlog) : t_gdata(spdlog)
{
    id_type(LCI_POS);
    _ptr = 0;
}

gsins::t_gposdata::~t_gposdata()
{
}

