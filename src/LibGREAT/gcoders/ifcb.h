/**
 * @file         ifcb.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        decode and enconde ifcb file
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef IFCB_H
#define IFCB_H

#include "gexport/ExportLibGREAT.h"
#include <string>
#include <vector>

#include "gcoders/gcoder.h"
#include "gdata/gifcb.h"

using namespace std;
using namespace gnut;

namespace great
{

    /**
    *@brief       Class for decode/encode ifcb file
    */
    class LibGREAT_LIBRARY_EXPORT t_ifcb : public t_gcoder
    {

    public:
        /**
        * @brief constructor.
        * @param[in]  s        setbase control
        * @param[in]  version  version of the gcoder
        * @param[in]  sz       size of the buffer
        */
        t_ifcb(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief default destructor. */
        virtual ~t_ifcb();

        /**
        * @brief decode header of ifcb file
        * @param[in]  buff        buffer of the data
        * @param[in]  sz          buffer size of the data
        * @param[in]  errmsg      error message of the data decoding
        * @return 
            @retval int consume size of header decoding
        */
        virtual int decode_head(char *buff, int sz, vector<string> &errmsg) override;

        /**
        * @brief decode data of ifcb file
        * @param[in]  buff        buffer of the data
        * @param[in]  sz          buffer size of the data
        * @param[in]  errmsg      error message of the data decoding
        * @return 
            @retval int consume size of header decoding
        */
        virtual int decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg) override;

    protected:
        string _ifcbmode; ///< ifcb mode EWL,WL and NL.
        t_gtime _epoch;   ///< current epoch
    private:
    };

} // namespace

#endif
