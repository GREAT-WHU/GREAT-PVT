/**
*
* @verbatim
History
    -1.0 Zheng Hongjie  2019-10-25  creat the file.
@endverbatim
* Copyright (c) 2018, Wuhan University. All rights reserved.
*
* @file       gcoder_buffer.cpp
* @brief      main decode function for all files in base part
*
*
* @author     Jiande Huang, Wuhan University
* @version    1.0.0
* @date       2019-10-25
*
*/
#include <gcoders/gcoder_buffer.h>
#include <string.h>
#include <iostream>

namespace great
{
    gcoder_buffer::gcoder_buffer()
    {
    }

    gcoder_buffer::~gcoder_buffer()
    {
    }

    int gcoder_buffer::size()
    {
        return _buffer.size();
    }

    int gcoder_buffer::add(char *buff, int size)
    {
        for (int i = 0; i < size; i++)
        {
            _buffer.push_back(buff[i]);
        }

        return size;
    }

    int gcoder_buffer::getline(string &str, int from_pos)
    {
        str = "";
        str.reserve(_buffer.size());
        int ifirst = 0;
        auto iter = _buffer.begin() + from_pos;
        for (; iter != _buffer.end(); iter++, ifirst++)
        {
            str += *iter;
            if (*iter == '\n')
            {
                break;
            }
        }
        if (iter == _buffer.end())
        {
            str = "";
            return -1;
        }

        return str.length();
    }

    void gcoder_buffer::toString(string &str)
    {
        str = "";
        str.reserve(_buffer.size());
        for (auto iter = _buffer.begin(); iter != _buffer.end(); iter++)
        {
            str += *iter;
        }
    }

    gcoder_char_buffer::gcoder_char_buffer() : _begpos(0),
                                               _endpos(0),
                                               _buffsz(1024 * 1000)
    {
        // use malloc instead of new due to realocate function!
        _buffer = (char *)malloc((_buffsz + 1) * sizeof(char)); // due to realocate function!
    }

    gcoder_char_buffer::~gcoder_char_buffer()
    {
        if (_buffer)
        {
            free(_buffer);
            _buffer = NULL; 
        }
    }

    int gcoder_char_buffer::size()
    {
        return _endpos - _begpos;
    }

    int gcoder_char_buffer::add(char *buff, int sz)
    {
        if (_begpos != 0)
        {
            for (int i = _begpos; i <= _endpos; i++)
            {
                _buffer[i - _begpos] = _buffer[i];
            }
            _endpos -= _begpos;
            _begpos = 0;
        }

        if (_endpos + sz > _buffsz)
        {

            // INCREASE BUFFER USING REALLOC !
            _buffsz = (int)((_endpos + sz + 1) * 1.5); //  (int)BUFFER_INCREASE_FAC == 1
            if (!(_buffer = (char *)realloc(_buffer, (_buffsz + 1) * sizeof(char))))
            {
                exit(1);
            }

            // DO NOT EXCEED MAXIMUM BUFFER SIZE !
            if (_buffsz > 10240000)
            {
                consume(sz);
                // --> solved in individual gcoders for individual files!
                return -1;
            }
        }

        memcpy(_buffer + _endpos, buff, sz * sizeof(char));
        _endpos += sz;

        _buffer[_endpos] = '\0'; // temporary set end of string (will be occasionally replace by add2buff)

        return sz;
    }

    int gcoder_char_buffer::getline(string &str, int from_pos)
    {
        str = "";
        from_pos += _begpos;
        if (_endpos == 0 || from_pos >= _endpos)
            return -1;

        int ifirst = -1;
        for (ifirst = from_pos; ifirst < _endpos; ifirst++)
        {
            if (_buffer[ifirst] == '\n')
            {
                break;
            }
        }
        if (ifirst == _endpos)
        {
            return -1;
        }

        str = string(_buffer + from_pos, _buffer + ifirst + 1);

        return str.length();
    }

    int gcoder_char_buffer::getbuffer(const char *&buff)
    {
        buff = _buffer + _begpos;
        return _endpos - _begpos;
    }

    int gcoder_char_buffer::consume(int bytes_to_eat)
    {
        if (bytes_to_eat <= 0)
        {
            return 0;
        }
        if (_begpos + bytes_to_eat > _endpos ||
            bytes_to_eat > _buffsz)
        {

            exit(1);
        }

        _begpos += bytes_to_eat;

        return bytes_to_eat;
    }
    void gcoder_char_buffer::toString(string &str)
    {
        str = string(_buffer + _begpos, _endpos - _begpos);
    }
}