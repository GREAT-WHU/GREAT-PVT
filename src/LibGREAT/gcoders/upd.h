/**
 * @file         upd.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        decode and enconde upd file
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef UPD_H
#define UPD_H

#include <string>
#include <vector>

#include "gcoders/gcoder.h"
#include "gdata/gupd.h"

using namespace std;
using namespace gnut;

namespace great
{

    /**
    *@brief       Class for decode/encode upd file
    */
    class LibGREAT_LIBRARY_EXPORT t_upd : public t_gcoder
    {

    public:
        /**
        * @brief constructor.
        * @param[in]  s        setbase control
        * @param[in]  version  version of the gcoder
        * @param[in]  sz       size of the buffer
        */
        explicit t_upd(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief default destructor. */
        virtual ~t_upd();

        /**
        * @brief decode header of upd file
        * @param[in]  buff        buffer of the data
        * @param[in]  sz          buffer size of the data
        * @param[in]  errmsg      error message of the data decoding
        * @return consume size of header decoding
        */
        virtual int decode_head(char *buff, int sz, vector<string> &errmsg) override;

        /**
        * @brief decode data of upd file
        * @param[in]  buff        buffer of the data
        * @param[in]  sz          buffer size of the data
        * @param[in]  errmsg      error message of the data decoding
        * @return consume size of header decoding
        */
        virtual int decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg) override;

    protected:
        UPDTYPE _updtype; ///< upd mode EWL,WL and NL.
        t_gtime _epoch;   ///< current epoch
    private:
    };

} // namespace

#endif
