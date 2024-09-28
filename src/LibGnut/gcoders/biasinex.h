/**
*
* @verbatim
    History
    -1.0    PV        2011-04-20    creat the file.
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     biasinex.h
* @brief    The base class used to decode bias SINEX file information.
*
* @author   PV
* @version  1.0.0
* @date     2018-03-09
*
*/

#ifndef BIASINEX_H
#define BIASINEX_H

#include "gcoders/gcoder.h"
#include "gcoders/sinex.h"
#include "gutils/gtime.h"
#include "gall/gallbias.h"

#define SNX_WINDOW_EXTENTION 6 * 3600 // [s] window for SNX temporal approximation

using namespace std;

namespace gnut
{
    /**
    *@brief       Class for decoding the data
    */
    class LibGnut_LIBRARY_EXPORT t_biasinex : public t_sinex
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
        explicit t_biasinex(t_gsetbase *s, string version, int sz = DEFAULT_BUFFER_SIZE, string id = "biasinex");

        /** @brief default destructor. */
        virtual ~t_biasinex(){};

    protected:
        /**
        * @brief decode  data file.
        *
        * The function is used for decoding  file.
        * pay attention to the buff and the size of buff which may cause some trouble when
        using the wrong value in decoding.
        *
        * @return
        @retval >=0 consume size of header decoding
        @retval <0  finish reading
        */
        virtual int _decode_comm();

        /**
        * @brief decode  data file.
        *
        * The function is used for decoding  file.
        * pay attention to the buff and the size of buff which may cause some trouble when
        using the wrong value in decoding.
        *
        * @return
        @retval >=0 consume size of header decoding
        @retval <0  finish reading
        */
        virtual int _decode_block();

        /**
        * @brief add ALLBIAS data to _allbias
        *
        * @param[in]  id        data type
        * @param[in]  pt_data    ALLBIAS data
        * @return void
        */
        virtual void _add_data(string id, t_gdata *pt_data);

        t_gallbias *_allbias; ///< 
    };

} // namespace

#endif
