/**
 * @file         gpoleut1.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        The class for storaging poleut1 data.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GPOLEUT1_H
#define GPOLEUT1_H

#include <vector>
#include <map>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "gdata/gdata.h"
#include "gutils/gtime.h"
#include "gexport/ExportLibGREAT.h"

using namespace std;
using namespace gnut;

namespace great
{
    /**
    *@brief       Class for storaging poleut1 file data.
    */
    class LibGREAT_LIBRARY_EXPORT t_gpoleut1 : public t_gdata
    {
    public:
        /** @brief constructor. */
        t_gpoleut1();
        t_gpoleut1(t_spdlog spdlog);

        /** @brief  destructor. */
        virtual ~t_gpoleut1(){};

        /**
        *@brief      the map stored the record in the poleut1 file
        */
        typedef map<t_gtime, map<string, double>> t_mapdata;

        /**
         * @brief add begin and end time of file data.
         * @param[in]   beg     the begin time of data
         * @param[in]   end     the end time of data
         */
        void setBegEndTime(int beg, int end);

        /**
         * @brief add data for map of pole and ut1 data.
         * @param[in]   mjdtime   the time of data to store
         * @param[in]   data      the data record
         * @param[in]   mode      the time mode
         * @param[in]   intv      the interval of data
         */
        void setEopData(t_gtime mjdtime, map<string, double> data, string mode, double intv);

        /**
        * @brief whether the poleut1 data is empty
        * @return  
            @retval true the poleut1 data is empty
            @retval false the poleut1 data is existent
        */
        bool isEmpty();

        /**
        * @brief return the map of record.
        * @return  the map of record
        */
        t_mapdata *getPoleUt1DataMap() { return &_poleut1_data; };

        /**
        * @brief return begin time of data in the poleut1 file.
        * @return  begin time
        */
        int getBegTime() { return _beg_time; };

        /**
        * @brief return end time of data in the poleut1 file.
        * @return  end time
        */
        int getEndTime() { return _end_end; };

        /**
        * @brief return mode of ut1.
        * @return  mode of ut1
        */
        string getUt1Mode() { return _UT1_mode; };

        /**
        * @brief return interval of poleut1 data
        * @return  interval
        */
        double getIntv() { return _intv; };

    protected:
        t_mapdata _poleut1_data; ///< map of pole and ut1 data.
        string _UT1_mode;        ///< UT1 type.
        int _beg_time;           ///< begin time.
        int _end_end;            ///< end time.
        double _intv;            ///< interval.
    };
} //namespace

#endif