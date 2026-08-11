/**
*
* @verbatim
     (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

     (c) 2011-2017 Geodetic Observatory Pecny, http://www.pecny.cz (gnss@pecny.cz)
        Research Institute of Geodesy, Topography and Cartography
        Ondrejov 244, 251 65, Czech Republic
  @endverbatim
*
* @file        gsetout.cpp
* @brief       implements output setting class
* @author      Jan Dousa
* @version     1.0.0
* @date        2012-10-23
*
*/

#include <iomanip>
#include <sstream>
#include <algorithm>

#include "gset/gsetout.h"
#include "gutils/gfileconv.h"

using namespace pugi;

namespace gnut
{
    OFMT t_gsetout::str2ofmt(const std::string& s)
    {
        std::string tmp = s;
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        if (tmp == "OUT")
        {
            return OFMT::XXX_OUT;
        }
        if (tmp == "LOG")
        {
            return OFMT::LOG_OUT;
        }
        if (tmp == "PPP")
        {
            return OFMT::PPP_OUT;
        }
        if (tmp == "FLT")
        {
            return OFMT::FLT_OUT;
        }
        return static_cast<OFMT>(-1);
    }

    std::string t_gsetout::ofmt2str(const OFMT& f)
    {
        switch (f)
        {
            case OFMT::XXX_OUT:
                return "OUT";
            case OFMT::LOG_OUT:
                return "LOG";
            case OFMT::PPP_OUT:
                return "PPP";
            case OFMT::FLT_OUT:
                return "FLT";
            default:
                return "UNDEF";
        }
        return "UNDEF";
    }

    t_gsetout::t_gsetout() :
        t_gsetbase(),
        _append(false),
        _verb(0)
    {
        _set.insert(XMLKEY_OUT);
    }

    t_gsetout::~t_gsetout()
    {
    }

    int t_gsetout::output_size(const std::string& fmt)
    {

        int tmp = _outputs(fmt).size();

        return tmp;
    }

    int t_gsetout::verb()
    {

        int tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).attribute("verb").as_int();

        return tmp;
    }

    bool t_gsetout::append()
    {

        bool tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).attribute("append").as_bool();

        return tmp;
    }

    std::string t_gsetout::outputs(const std::string& fmt)
    {

        std::string tmp = _outputs(fmt);

        return tmp;
    }

    std::string t_gsetout::log_type()
    {
        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child("log").attribute("type").as_string();
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        if (tmp.empty())
        {
            return "CONSOLE";
        }
        else
        {
            return tmp;
        }
    }

    std::string t_gsetout::log_name()
    {
        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child("log").attribute("name").as_string();
        str_erase(tmp);
        if (tmp.empty())
        {
            return "my_logger";
        }
        else
        {
            return tmp;
        }
    }

    std::string t_gsetout::log_pattern()
    {
        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child("log").attribute("pattern").as_string();
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        if (tmp.empty())
        {
            return std::string("[%Y-%m-%d %H:%M:%S] <thread %t> [%l] [%@] %v");
        }
        else
        {
            return tmp;
        }
    }

    level::level_enum t_gsetout::log_level()
    {
        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child("log").attribute("level").as_string();
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        if (tmp.empty())
        {
            return level::level_enum::info;
        }
        else
        {
            if (tmp.find("ERROR") != std::string::npos)
            {
                return level::level_enum::err;
            }
            else if (tmp.find("DEBUG") != std::string::npos)
            {
                return level::level_enum::debug;
            }
            else if (tmp.find("WARN") != std::string::npos)
            {
                return level::level_enum::warn;
            }
            else if (tmp.find("CRITICAL") != std::string::npos)
            {
                return level::level_enum::critical;
            }
            else if (tmp.find("TRACE") != std::string::npos)
            {
                return level::level_enum::trace;
            }
            else if (tmp.find("INFO") != std::string::npos)
            {
                return level::level_enum::info;
            }
            else
            {
                return level::level_enum::off;
            }
        }
    }

    std::string t_gsetout::version(const std::string& fmt)
    {

        std::string ver = DEFAULT_FILE_VER;
        xml_node node = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child(fmt.c_str());
        if (!fmt.empty() && !node.attribute("ver").empty())
        {
            ver = node.attribute("ver").as_string();
        }

        return ver;
    }

    std::set<std::string> t_gsetout::oformats()
    {
        return _oformats();
    }

    std::set<std::string> t_gsetout::_oformats()
    {
        std::set<std::string> tmp;
        for (xml_node node = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).first_child(); node; node = node.next_sibling())
        {
            tmp.insert(node.name());
        }
        return tmp;
    }

    std::string t_gsetout::_outputs(const std::string& fmt)
    {
        std::string str;
        for (xml_node node = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).first_child(); node; node = node.next_sibling())
        {
            if (node.name() == fmt)
            {
                std::istringstream is(node.child_value());
                while (is >> str && !is.fail())
                {
                    if (str.find("://") == std::string::npos)
                    {
                        str = GFILE_PREFIX + str;
                    }
                    return str;
                }
            }
        }
        return "";
    }

    void t_gsetout::check()
    {

        // check existence of nodes/attributes
        xml_node parent = _doc.child(XMLKEY_ROOT);
        xml_node node = _default_node(parent, XMLKEY_OUT);

        // check existence of attributes
        _default_attr(node, "append", _append);

        // check supported input formats (see OFMT enum !)
        std::set<std::string> ofmt = _oformats();
        std::set<std::string>::const_iterator itFMT = ofmt.begin();
        while (itFMT != ofmt.end())
        {
            std::string fmt = *itFMT;
            OFMT ofmt = str2ofmt(fmt);
            if (static_cast<int>(ofmt) < 0)
            {
                _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).remove_child(node.child(fmt.c_str()));
                std::cout << "Warning: " + fmt + " out format not implemented [gsetout::check()]!" << std::endl;
                itFMT++;
                continue;
            }

            // check application-specific output format
            if (_OFMT_supported.find(ofmt) == _OFMT_supported.end())
            {
                _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).remove_child(node.child(fmt.c_str()));

                std::cout << "Warning: " + fmt + " out format not supported by this application!" << std::endl;
            }
            itFMT++;
        }

        return;
    }

    void t_gsetout::help()
    {

        std::cerr << " <outputs append=\"" << _append << "\" verb=\"" << _verb << "\" >\n"
             << "   <flt> file://dir/name </flt>    \t\t <!-- filter output encoder -->\n"
             << " </outputs>\n";

        return;
    }

} // namespace gnut
