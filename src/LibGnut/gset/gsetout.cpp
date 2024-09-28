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

using namespace std;
using namespace pugi;

namespace gnut
{
    OFMT t_gsetout::str2ofmt(const string &s)
    {
        string tmp = s;
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        if (tmp == "OUT")
            return XXX_OUT;
        if (tmp == "LOG")
            return LOG_OUT;
        if (tmp == "PPP")
            return PPP_OUT;
        if (tmp == "FLT")
            return FLT_OUT;
        return OFMT(-1);
    }

    string t_gsetout::ofmt2str(const OFMT &f)
    {
        switch (f)
        {
        case XXX_OUT:
            return "OUT";
        case LOG_OUT:
            return "LOG";
        case PPP_OUT:
            return "PPP";
        case FLT_OUT:
            return "FLT";
        default:
            return "UNDEF";
        }
        return "UNDEF";
    }

    t_gsetout::t_gsetout()
        : t_gsetbase(),
          _append(false),
          _verb(0)
    {
        _set.insert(XMLKEY_OUT);
    }

    t_gsetout::~t_gsetout()
    {
    }

    int t_gsetout::output_size(const string &fmt)
    {
        _gmutex.lock();

        int tmp = _outputs(fmt).size();

        _gmutex.unlock();
        return tmp;
    }

    int t_gsetout::verb()
    {
        _gmutex.lock();

        int tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).attribute("verb").as_int();

        _gmutex.unlock();
        return tmp;
    }

    bool t_gsetout::append()
    {
        _gmutex.lock();

        bool tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).attribute("append").as_bool();

        _gmutex.unlock();
        return tmp;
    }

    string t_gsetout::outputs(const string &fmt)
    {
        _gmutex.lock();

        string tmp = _outputs(fmt);

        _gmutex.unlock();
        return tmp;
    }

    string t_gsetout::log_type()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child("log").attribute("type").as_string();
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        _gmutex.unlock();
        if (tmp.empty())
        {
            return "CONSOLE";
        }
        else
        {
            return tmp;
        }
    }

    string t_gsetout::log_name()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child("log").attribute("name").as_string();
        str_erase(tmp);
        _gmutex.unlock();
        if (tmp.empty())
        {
            return "my_logger";
        }
        else
        {
            return tmp;
        }
    }

    string t_gsetout::log_pattern()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child("log").attribute("pattern").as_string();
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        _gmutex.unlock();
        if (tmp.empty())
        {
            return string("[%Y-%m-%d %H:%M:%S] <thread %t> [%l] [%@] %v");
        }
        else
        {
            return tmp;
        }
    }

    level::level_enum t_gsetout::log_level()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child("log").attribute("level").as_string();
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        _gmutex.unlock();
        if (tmp.empty())
        {
            return level::level_enum::info;
        }
        else
        {
            if (tmp.find("ERROR") != string::npos)
            {
                return level::level_enum::err;
            }
            else if (tmp.find("DEBUG") != string::npos)
            {
                return level::level_enum::debug;
            }
            else if (tmp.find("WARN") != string::npos)
            {
                return level::level_enum::warn;
            }
            else if (tmp.find("CRITICAL") != string::npos)
            {
                return level::level_enum::critical;
            }
            else if (tmp.find("TRACE") != string::npos)
            {
                return level::level_enum::trace;
            }
            else if (tmp.find("INFO") != string::npos)
            {
                return level::level_enum::info;
            }
            else
            {
                return level::level_enum::off;
            }
        }
    }

    string t_gsetout::version(const string &fmt)
    {
        _gmutex.lock();

        string ver = DEFAULT_FILE_VER;
        xml_node node = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).child(fmt.c_str());
        if (!fmt.empty() &&
            !node.attribute("ver").empty())
        {
            ver = node.attribute("ver").as_string();
        }

        _gmutex.unlock();
        return ver;
    }

    set<string> t_gsetout::oformats()
    {
        return _oformats();
    }

    set<string> t_gsetout::_oformats()
    {
        set<string> tmp;
        for (xml_node node = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).first_child(); node; node = node.next_sibling())
        {
            tmp.insert(node.name());
        }
        return tmp;
    }

    string t_gsetout::_outputs(const string &fmt)
    {
        string str;
        for (xml_node node = _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).first_child(); node; node = node.next_sibling())
        {
            if (node.name() == fmt)
            {
                istringstream is(node.child_value());
                while (is >> str && !is.fail())
                {
                    if (str.find("://") == string::npos)
                        str = GFILE_PREFIX + str;
                    return str;
                }
            }
        }
        return "";
    }

    void t_gsetout::check()
    {
        _gmutex.lock();

        // check existence of nodes/attributes
        xml_node parent = _doc.child(XMLKEY_ROOT);
        xml_node node = _default_node(parent, XMLKEY_OUT);

        // check existence of attributes
        _default_attr(node, "append", _append);

        // check supported input formats (see OFMT enum !)
        set<string> ofmt = _oformats();
        set<string>::const_iterator itFMT = ofmt.begin();
        while (itFMT != ofmt.end())
        {
            string fmt = *itFMT;
            OFMT ofmt = str2ofmt(fmt);
            if (ofmt < 0)
            {
                _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).remove_child(node.child(fmt.c_str()));
                cout << "Warning: " + fmt + " out format not implemented [gsetout::check()]!" << endl;
                itFMT++;
                continue;
            }

            // check application-specific output format
            if (_OFMT_supported.find(ofmt) == _OFMT_supported.end())
            {
                _doc.child(XMLKEY_ROOT).child(XMLKEY_OUT).remove_child(node.child(fmt.c_str()));

                cout << "Warning: " + fmt + " out format not supported by this application!" << endl;
            }
            itFMT++;
        }

        _gmutex.unlock();
        return;
    }

    void t_gsetout::help()
    {
        _gmutex.lock();

        cerr << " <outputs append=\"" << _append << "\" verb=\"" << _verb << "\" >\n"
             << "   <flt> file://dir/name </flt>    \t\t <!-- filter output encoder -->\n"
             << " </outputs>\n";

        _gmutex.unlock();
        return;
    }

} // namespace
