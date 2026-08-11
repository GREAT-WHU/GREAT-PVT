/**
*
* @verbatim
    History

  @endverbatim
*
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

* @file       gsetbase.h
* @brief      implements base setting class
* @author     Jan Dousa
* @version    1.0.0
* @date       2012-10-23
*
*/

#ifndef GSETBASE_H
#define GSETBASE_H

#include <set>
#include <map>
#include <string>
#include <iostream>

#include "gutils/gtypeconv.h"
#include "gutils/gmutex.h"
#include "pugixml/src/pugixml.hpp"
#include "spdlog/spdlog.h"

#ifndef XMLKEY_ROOT
#define XMLKEY_ROOT "config" ///< The define of ROOT node
#endif

#ifdef WIN32
#pragma warning(disable : 4250)
#endif

using namespace pugi;

namespace gnut
{
    enum class modeofmeanpole
    {
        linear,
        cubic
    };

    /**
     * @brief The base class for xml set
     */
    class LibGnut_LIBRARY_EXPORT t_gsetbase : public pugi::xml_node
    {
    public:
        /** @brief constructor. */
        t_gsetbase();

        /** @brief destructor. */
        virtual ~t_gsetbase();

        /**
         * @brief  read configuration from file
         * @param[in] file file name
         */
        int read(const std::string& file);

        /**
         * @brief  read configuration from istream
         * @param[in] is istream name
         */
        int read_istream(std::istream& is);

        /**
         * @brief  write configuration in file
         * @param[in] file file name
         */
        int write(const std::string& file);

        /**
         * @brief  write configuration in ostream
         * @param[in] os istream name
         */
        int write_ostream(std::ostream& os);

        /** @brief application name. */
        virtual std::string pgm();

        /** @brief application info/version. */
        virtual std::string app();

        /**
         * @brief  get some information of the application
         * @param[out] pgm program name
         * @param[out] ver program version
         * @param[out] dat program running data
         * @param[out] tim program running time
         * @param[out] rev program revision number
         * @param[out] own program owner name
         */
        virtual void app(const std::string& pgm, const std::string& ver, const std::string& dat, const std::string& tim, const std::string& rev, const std::string& own);

        /** @brief application usage. */
        virtual void usage();

        /**
         * @brief  get command line arguments
         * @param[in] argc   argument number
         * @param[in] argv[] command line array of arguments
         */
        virtual void arg(int argc, char* argv[], bool add = false, bool thin = false);

        /** @brief settings help. */
        virtual void help() = 0;

        /** @brief settings check. */
        virtual void check() = 0;

        /** @brief thin execution. */
        bool thin();

        /**
         * @brief remove spaces, tabs and newlines in the string
         * @param[in]  str    string
         */
        void str_erase(std::string& str);

    protected:
        /**
         * @brief get the double value of ROOT.elem.attrib
         * @param[in] elem    element name in xml
         * @param[in] attrib  attribute name in xml
         * @return double : The value of ROOT.elem.attribute
         */
        double _dblatt(const std::string& elem, const std::string& attrib);

        /**
         * @brief get the set<string> value of ROOT.elem.subelem
         * @param[in] elem    element name in xml
         * @param[in] subelem subelem name in xml
         * @return set<string> : The value of ROOT.elem.subelem
         */
        std::set<std::string> _setval(const std::string& elem, const std::string& subelem);
        std::set<std::string> _setvals(const std::string& elem, const std::string& subelem);

        /**
         * @brief get the vector<string> value of ROOT.elem.subelem
         * @param[in] elem    element name in xml
         * @param[in] subelem subelem name in xml
         * @return vector<string> : The value of ROOT.elem.subelem
         */
        std::vector<std::string> _vecval(const std::string& elem, const std::string& subelem);

        /**
         * @brief get default child node of a xml node
         * @param[in] node    the parent node
         * @param[in] n       child node name
         * @param[in] val     child node value
         * @param[in] reset   reset or not
         * @return xml_node : The child node
         */
        xml_node _default_node(xml_node& node, const char* n, const char* val = "", bool reset = false);

        /**
         * @brief get default node of a xml node by the attribute or reset it with v
         * @param[in] node    the parent node
         * @param[in] n       attribute name
         * @param[in] v       reset value
         * @param[in] reset   reset or not
         * @return xml_node : The child node
         */
        void _default_attr(xml_node& node, const char* n, const std::string& v, bool reset = false);

        /**
         * @brief get default node of a xml node by the attribute or reset it with v
         * @param[in] node    the parent node
         * @param[in] n       attribute name
         * @param[in] v       reset value
         * @param[in] reset   reset or not
         * @return xml_node : The child node
         */
        void _default_attr(xml_node& node, const char* n, const bool& value, bool reset = false);

        /**
         * @brief get default node of a xml node by the attribute or reset it with v
         * @param[in] node    the parent node
         * @param[in] n       attribute name
         * @param[in] v       reset value
         * @param[in] reset   reset or not
         * @return xml_node : The child node
         */
        void _default_attr(xml_node& node, const char* n, const int& value, bool reset = false);

        /**
         * @brief get default node of a xml node by the attribute or reset it with v
         * @param[in] node    the parent node
         * @param[in] n       attribute name
         * @param[in] v       reset value
         * @param[in] reset   reset or not
         * @return xml_node : The child node
         */
        void _default_attr(xml_node& node, const char* n, const double& value, bool reset = false);

        /** @brief report element issues. */
        void _add_log(std::string element, std::string msg);

        /** @brief XML settings header. */
        virtual void help_header();

        /** @brief XML settings footer. */
        virtual void help_footer();

        xml_document _doc;     ///< root document
        xml_parse_result _irc; ///< result status

    protected:
        std::string _name;      ///< configuration name
        std::string _delimiter; ///< delimiter for writting nodes/elements

        std::string _pgm; ///< program name
        std::string _ver; ///< program version
        std::string _rev; ///< program revision
        std::string _own; ///< program owner
        std::string _dat; ///< program data
        std::string _tim; ///< program time

        std::set<std::string> _set;                     ///< program settings
        std::map<std::string, std::set<std::string>> _chache_log; ///< The chache of log pointer

        mutable t_gmutex _gmutex; ///< For the thread

    private:
    };

} // namespace gnut

#endif
