/**
*
* @verbatim
    History
     -1.0 ZhengHJ  2019-04-02  created

  @endverbatim
* Copyright (c) 2018, Wuhan University. All rights reserved.
*
* @file            gallproc.h
* @brief           header files of storaging all data
*
* @author          ZhengHJ, Wuhan University
* @version         1.0.0
* @date            2019-04-16
*
*/
#ifndef GALLPROCDATA_H
#define GALLPROCDATA_H

#include "gexport/ExportLibGnut.h"
#include "gdata/gdata.h"
#include "gset/gsetinp.h"
using namespace gnut;

namespace great
{
    /**
    *@brief       Class for storaging all process data
    */
    class LibGnut_LIBRARY_EXPORT t_gallproc : public t_gdata
    {
    public:
        /** @brief default constructor. */
        t_gallproc();

        /**
         * @brief Construct a new t gallproc object
         * 
         * @param spdlog 
         */
        t_gallproc(t_spdlog spdlog);
        /** @brief copy constructor. */
        t_gallproc(const t_gallproc &Other);

        /** @brief default destructor. */
        virtual ~t_gallproc();

        /** @brief assignment operator . */
        void operator=(const t_gallproc &Other);

        /** @brief add one process data. */
        void Add_Data(const string &type, t_gdata *data);

        /** 
        * @brief get the data by type of t_gdata.
        * @param[in] id_type specified type of t_gdata 
        * @return specified data
        */
        t_gdata *operator[](t_gdata::ID_TYPE id_type);

        /** 
        * @brief get the data by group type of t_gdata.
        * @param[in] id_type specified group type of t_gdata 
        * @return specified data
        */
        t_gdata *operator[](t_gdata::ID_GROUP);

    protected:
        map<t_gdata::ID_TYPE, t_gdata *> _mapData; ///< map of type and data pointer  for all process data
    };
}

#endif