/**
 * @file         dvpteph405.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        decode the binary jpleph_de405 file data 
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef DVPTEPH405_H
#define DVPTEPH405_H

#include "gcoders/gcoder.h"
#include "gset/gsetbase.h"

namespace great
{
    /**
     * @brief       Class for decoding the jpleph_de405 data
     */
    class LibGREAT_LIBRARY_EXPORT t_dvpteph405 : public t_gcoder
    {
    public:
        /** 
         * @brief default constructor. 
         * 
         * @param[in]  s        setbase control
         * @param[in]  version  version of the gcoder
         * @param[in]  sz       size of the buffer
         */
        explicit t_dvpteph405(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** 
         * @brief default destructor. 
         */
        virtual ~t_dvpteph405(){};

        /** 
         * @brief decode the header of the jpleph_de405 data file. 
         * @param[in]  buff        buffer of the data
         * @param[in]  bufLen      buffer size of the data
         * @param[in]  errmsg      error message of the data decoding
         * @return consume size of header decoding
            @retval >=0 consume size of header decoding
            @retval <0  finish reading
         */
        virtual int decode_head(char *buff, int bufLen, vector<string> &errmsg);

        /** 
         * @brief decode the data body of the jpleph_de405 data file. 
         * 
         * @param[in]  buff        buffer of the data
         * @param[in]  bufLen      buffer size of the data
         * @param[in]  errmsg      error message of the data decoding
         * @return consume size for data body decoding
            @retval >=0 consume size of body decoding
             @retval <0  finish reading
         */
        virtual int decode_data(char *buff, int bufLen, int &cnt, vector<string> &errmsg);

    protected:
    private:
        int _flag;       ///< flag for decoder
        int _version_de; ///< JPL file version
        t_gtime _beg_de; ///< start time for JPL data
        t_gtime _end_de; ///< end time for JPL data
        double _days;    ///< Sampling interval of data(unit: day)
        int _ipt[3][13]; ///< index of planets for calculating position and speed
    };
}

#endif // !DVPTEPH405_H
