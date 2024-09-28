/**
 * @file         poleut1.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        The base class used to decode poleut1 file information.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef POLEUT1_H
#define POLEUT1_H

#include "gutils/gtime.h"
#include "gcoders/gcoder.h"
#include "gdata/gpoleut1.h"

using namespace std;
using namespace gnut;

namespace great
{
    /**
    *@brief       Class for decoding the poleut1 data
    */
    class LibGREAT_LIBRARY_EXPORT t_poleut1 : public t_gcoder
    {
    public:
        /**
         * @brief default constructor.
         *
         * @param[in]  s        setbase control
         * @param[in]  version  version of the gcoder
         * @param[in]  sz       size of the buffer
         */
        explicit t_poleut1(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief default destructor. */
        virtual ~t_poleut1(){};

        /**
         * @brief decode the header of the poleut1 data file.
         *
         * The function is used for decoding the head of poleut1 file.\n
         * pay attention to the buff and the size of buff which may cause some trouble when
            using the wrong value in decoding.
         *
         * @param[in]  buff        buffer of the data
         * @param[in]  sz          buffer size of the data
         * @param[in]  errmsg      error message of the data decoding
         * @return
            @retval >=0 consume size of header decoding
            @retval <0  finish reading
         */
        virtual int decode_head(char *buff, int sz, vector<string> &errmsg);

        /**
         * @brief decode the data body of the poleut1 data file.
         *
         * decode data body of poleut1 file, all the data read will store in the gpoleut1
         *
         * @param[in]  buff        buffer of the data
         * @param[in]  sz          buffer size of the data
         * @param[in]  errmsg      error message of the data decoding
         * @return
            @retval >=0 consume size of body decoding
            @retval <0  finish reading
         */
        virtual int decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg);

    protected:
        string _timetype;        ///< type of time(UT1R)
        double _begtime;         ///< beg time(modified julian day)   
        double _endtime;         ///< end time(modified julian day)   
        double _interval;        ///< time interval(unit: day)
        int _parnum;             ///< number of parameters
        vector<double> _parunit; ///< unit of each parameter
        vector<string> _parname; ///< name of each parameter
    };
} // namespace

#endif