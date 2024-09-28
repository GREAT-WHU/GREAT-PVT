/**
*
* @verbatim
    History
    -1.0    JD    2011-04-20 creat the file.
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     atx.h
* @brief    The base class used to decode Antenna Exchange format (ATX) file information.
* @author   JD
* @version  1.0.0
* @date     2011-04-20
*
*/

#ifndef ATX_H
#define ATX_H

#include <vector>

#include "gcoders/gcoder.h"

using namespace std;

namespace gnut
{
    /**
    *@brief       Class for decoding the atx data derive from t_gcoder
    */
    class LibGnut_LIBRARY_EXPORT t_atx : public t_gcoder
    {
    public:
        /**
        * @brief default constructor.
        *
        * @param[in]  s        setbase control
        * @param[in]  version  version of the gcoder
        * @param[in]  sz       size of the buffer
        */
        explicit t_atx(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief default destructor. */
        virtual ~t_atx(){};

        /**
        * @brief decode the header of the atx data file.
        *
        * The function is used for decoding the head of atx file.
        * pay attention to the buff and the size of buff which may cause some trouble when
        using the wrong value in decoding.
        *
        * @param[in]  buff        buffer of the data
        * @param[in]  bufLen      buffer size of the data
        * @param[out] errmsg      error message of the data decoding
        * @return
        @retval >=0 consume size of header decoding
        @retval <0  finish reading
        */
        virtual int decode_head(char *buff, int bufLen, vector<string> &errmsg);

        /**
        * @brief decode the data body of the atx data file.
        *
        * decode data body of atx file, all the data read will store in the gpcv???
        *
        * @param[in]  buff        buffer of the data
        * @param[in]  bufLen      buffer size of the data
        * @param[out] cnt          number of line
        * @param[out] errmsg      error message of the data decoding
        * @return
        @retval >=0 consume size of body decoding
        @retval <0  finish reading
        */
        virtual int decode_data(char *buff, int bufLen, int &cnt, vector<string> &errmsg);

    protected:
    private:
    };

} // namespace

#endif
