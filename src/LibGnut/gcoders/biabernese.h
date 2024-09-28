/**
*
* @verbatim
    History
    -1.0    PV        2011-04-20    creat the file.
    -1.1    glfeng    2019-04-08    add decode_data for BSX file from CAS & change the function combination
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     biabernese.h
* @brief    The base class used to decode biabernese file information.
*
* @author   PV
* @version  1.1.0
* @date     2019-04-08
*
*/

#ifndef BIABERNESE_H
#define BIABERNESE_H

#include "gcoders/gcoder.h"
#include "gutils/gtime.h"
#include "gall/gallbias.h"

using namespace std;

namespace gnut
{
    /**
    *@brief       Class for decoding the biabernese data derive from t_gcoder
    */
    class LibGnut_LIBRARY_EXPORT t_biabernese : public t_gcoder
    {
    public:
        /**
        * @brief default constructor.
        *
        * @param[in]  s            setbase control
        * @param[in]  version    version of the gcoder
        * @param[in]  sz        size of the buffer
        * @param[in]  id        string for reporting
        */
        explicit t_biabernese(t_gsetbase *s, string version, int sz = DEFAULT_BUFFER_SIZE, string id = "biabernese");

        /** @brief default destructor. */
        virtual ~t_biabernese(){};

        /**
        * @brief decode the header of the biabernese data file.
        *
        * The function is used for decoding the head of biabernese file.
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
        * @brief decode the data body of the biabernese data file.
        *
        * decode data body of biabernese file, all the data read will store in the gbias???
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
        /**
        * @brief decode the data body of the data file.
        *
        * decode data body of file, all the data read will store in the gbias
        *
        * @param[in]  buff        buffer of the data
        * @param[in]  sz        buffer size of the data
        * @param[out] cnt        number of line
        * @param[out] errmsg    error message of the data decoding
        * @return
        @retval >=0 consume size of body decoding
        @retval <0  finish reading
        */
        int _decode_data_CODE(char *buff, int sz, int &cnt, vector<string> &errmsg); 

        /**
        * @brief decode the data body of the data file.
        *
        * decode data body of file, all the data read will store in the gbias
        *
        * @param[in]  buff        buffer of the data
        * @param[in]  sz        buffer size of the data
        * @param[out] cnt        number of line
        * @param[out] errmsg    error message of the data decoding
        * @return
        @retval >=0 consume size of body decoding
        @retval <0  finish reading
        */
        int _decode_data_sinex(char *buff, int sz, int &cnt, vector<string> &errmsg); 

        /**
        * @brief decode the data body of the data file.
        *
        * decode data body of file, all the data read will store in the gbias
        *
        * @param[in]  buff        buffer of the data
        * @param[in]  sz        buffer size of the data
        * @param[out] cnt        number of line
        * @param[out] errmsg    error message of the data decoding
        * @return
        @retval >=0 consume size of body decoding
        @retval <0  finish reading
        */
        int _decode_data_sinex_0(char *buff, int sz, int &cnt, vector<string> &errmsg);

        t_gallbias *_allbias; ///< allbias
        t_gtime _beg;         ///< beg
        t_gtime _end;         ///< end

        string _ac;                ///< 
        double _version;           ///< 1.00 or 0.01
        bool _is_bias = false;     ///< sinex bias file
        bool _is_absolute = false; ///< relative/absolute bias
    };

} // namespace

#endif
