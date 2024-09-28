/**
*
* @verbatim
    History
    2012-10-23  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gxml.h
* @brief      Purpose: implements base XML class
*.
* @author     JD
* @version    1.0.0
* @date       2012-10-23
*
*/
#ifndef GXML_H
#define GXML_H

#include <set>
#include <string>
#include <ctype.h>
#include <iostream>

#ifdef BMUTEX
#include <boost/thread/mutex.hpp>
#endif

#include "gutils/gmutex.h"
#include "gutils/gtypeconv.h"
#include "pugixml/src/pugixml.hpp"

using namespace std;
using namespace pugi;

namespace gnut
{

    /** @brief class for t_gxml. */
    class LibGnut_LIBRARY_EXPORT t_gxml : public pugi::xml_node
    {
    public:
        /** @brief constructor 1. */
        explicit t_gxml(string s, bool upper = true);

        /** @brief default destructor. */
        virtual ~t_gxml();

        /** @brief read glog file and set it. */
        virtual int glog_set(t_spdlog spdlog);

        /** @brief read from file. */
        int read(const string &file);

        /** @brief read from istream. */
        int read_istream(istream &is);

        /** @brief write in file. */
        int write(const string &file);

        /** @brief write in ostream. */
        int write_ostream(ostream &os);

        /** @brief settings check. */
        virtual void check(){};

        /** @brief settings help. */
        virtual void help(){};

    protected:
        /** @brief default node. */
        xml_node _default_node(xml_node &node, const char *n, const char *val = "", bool reset = false);

        /** @brief default attr. */
        void _default_attr(xml_node &node, const char *n, const string &val, bool reset = false);
        void _default_attr(xml_node &node, const char *n, const bool &val, bool reset = false);
        void _default_attr(xml_node &node, const char *n, const int &val, bool reset = false);
        void _default_attr(xml_node &node, const char *n, const double &val, bool reset = false);

        xml_document _doc;     ///< root document
        xml_parse_result _irc; ///< result status

        t_spdlog _logxml;  ///< pointer to spdlog file
        string _name;      ///< file name
        string _root;      ///< root directory
        string _delimiter; ///< delimiter for writting nodes/elements
        bool _ucase;       ///< upper/lower case for keywords
        t_gmutex _gmutexxml;
    private:
    };

} // namespace

#endif
