/**
*
* @verbatim
    History
    -1.0    PV        2011-04-20    creat the file.
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     blq.h
* @brief    The base class used to decode blq file information.
*
* @author   PV
* @version  1.0.0
* @date     2013-03-29
*
*/

#ifndef BLQ_H
#define BLQ_H

#include <vector>

#include "newmat/newmat.h"

#include "gcoders/gcoder.h"
#include "gmodels/gotl.h"
#include "gall/gallotl.h"
#include "gutils/gtime.h"

using namespace std;

namespace gnut
{
    /**
    *@brief       Class for decoding the blq data
    */
    class LibGnut_LIBRARY_EXPORT t_blq : public t_gcoder
    {
    public:
        /**
        * @brief default constructor.
        *
        * @param[in]  s            setbase control
        * @param[in]  version    version of the gcoder
        * @param[in]  sz        size of the buffer
        */
        explicit t_blq(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief default destructor. */
        virtual ~t_blq(){};

        /**
        * @brief decode the header of the blq data file.
        *
        * The function is used for decoding the head of blq file.
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
        virtual int decode_head(char *buff, int sz, vector<string> &errmsg);

        /**
        * @brief decode the data body of the blq data file.
        *
        * decode data body of blq file, all the data read will store in the gotl
        *
        * @param[in]  buff        buffer of the data
        * @param[in]  bufLen      buffer size of the data
        * @param[out] cnt          number of line
        * @param[out] errmsg      error message of the data decoding
        * @return
        @retval >=0 consume size of body decoding
        @retval <0  finish reading
        */
        virtual int decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg);

    protected:
        double _lat;     ///< latitude
        double _lon;     ///< longitude
        string _site;    ///< site
        Matrix _blqdata; ///< blqdata

    private:
    };

} // namespace

#endif
