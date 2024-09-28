/**
*
* @verbatim
    History
    -1.0 Zheng Hongjie  2019-10-25  creat the file.
@endverbatim
* Copyright (c) 2018, Wuhan University. All rights reserved.
*
* @file       gcoder_buffer.h
* @brief      main decode function for all files in base part
*
*
* @author     Jiande Huang, Wuhan University
* @version    1.0.0
* @date       2019-10-25
*
*/

#ifndef GCODER_BUFFER_H
#define GCODER_BUFFER_H

#include "gexport/ExportLibGnut.h"
#include <string>
#include <deque>
#include <queue>

using namespace std;

namespace great
{
    /**
    *@brief Class for gcoder_buffer
    */
    class LibGnut_LIBRARY_EXPORT gcoder_buffer
    {
    public:
        /** @brief default constructor. */
        gcoder_buffer();

        /** @brief default destructor. */
        ~gcoder_buffer();

        /** @brief get size. */
        int size();

        /** @brief add. */
        int add(char *buff, int size);

        /**
        * @brief get single line from the buffer.
        * @param[in]  str        the content of the single line
        * @param[in]  from_pos    the position in the buffer
        * @return      int
        */
        int getline(string &str, int from_pos);

        /** @brief tostring. */
        void toString(string &str);

    private:
        deque<char> _buffer; ///< buffer
    };

    /**
    *@brief Class for gcoder_char_buffer
    */
    class LibGnut_LIBRARY_EXPORT gcoder_char_buffer
    {
    public:
        /** @brief default constructor. */
        gcoder_char_buffer();

        /** @brief default destructor. */
        ~gcoder_char_buffer();

        /** @brief get size. */
        int size();

        /** @brief add. */
        int add(char *buff, int size);

        /**
        * @brief get single line from the buffer.
        * @param[in]  str        the content of the single line
        * @param[in]  from_pos    the position in the buffer
        * @return      int
        */
        int getline(string &str, int from_pos);

        /**
        * @brief get the buffer.
        * @param[in]  buff        buffer
        * @return      int
        */
        int getbuffer(const char *&buff);

        /** @brief remove from buffer. */
        int consume(int bytes_to_eat);

        /** @brief to string. */
        void toString(string &str);

    private:
        int _begpos;   ///< begin position in the buffer
        int _endpos;   ///< end position in the buffer
        int _buffsz;   ///< buffer size
        char *_buffer; ///< buffer
    };
}

#endif