/**
 * @file         gsetamb.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        control set from XML
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gset/gsetamb.h"
#include <sstream>

using namespace std;
using namespace pugi;

namespace great
{
    t_gsetamb::t_gsetamb()
        : t_gsetbase()
    {
        _gmutex.lock();

        _set.insert(XMLKEY_AMBIGUITY);

        _gmutex.unlock();
    }
    t_gsetamb::~t_gsetamb()
    {
    }
    void t_gsetamb::check()
    {
        _gmutex.lock();
        _gmutex.unlock();
        return;
    }
    void t_gsetamb::help()
    {
        _gmutex.lock();
        cerr << "<ambiguity>\n"
             << "<upd_mode> upd </upd_mode>\n"
             << "<fix_mode> SEARCH/NO </fix_mode>\n"
             << "<ratio> 3.0 </ratio>\n"
             << "<all_baselines> NO </all_baselines>\n"
             << "<min_common_time> 30 </min_common_time>\n"
             << "<widelane_decision     maxdev = \"0.15\" maxsig = \"0.10\" alpha = \"1000\"/>\n"
             << "<narrowlane_decision   maxdev = \"0.15\" maxsig = \"0.10\" alpha = \"1000\"/>\n";

        _gmutex.unlock();
        return;
    }

    FIX_MODE t_gsetamb::fix_mode()
    {
        if (!_doc)
            str2fixmode(string());
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("fix_mode");
        return str2fixmode(trim(tmp));
    }

    UPD_MODE t_gsetamb::upd_mode()
    {
        if (!_doc)
            str2upd_mode(string());
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("upd_mode");
        return str2upd_mode(trim(tmp));
    }

    double t_gsetamb::lambda_ratio()
    {
        _gmutex.lock();

        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("ratio");

        _gmutex.unlock();
        return str2dbl(tmp);
    }

    double t_gsetamb::bootstrapping()
    {
        _gmutex.lock();

        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("boot");

        if (tmp.empty())
        {
            _gmutex.unlock();
            return -0.001;
        }

        _gmutex.unlock();
        return str2dbl(tmp);
    }

    double t_gsetamb::min_common_time()
    {
        _gmutex.lock();

        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("min_common_time");

        _gmutex.unlock();
        return str2dbl(tmp);
    }

    map<string, double> t_gsetamb::get_amb_decision(string type)
    {
        _gmutex.lock();
        map<string, string> type2child = {
            {"EWL", "extra_widelane_decision"}, {"WL", "widelane_decision"}, {"NL", "narrowlane_decision"}};
        map<string, double> amb_decision;
        if (_default_decision.find(type) == _default_decision.end())
            return amb_decision;
        amb_decision = _default_decision[type];
        for (auto iter = amb_decision.begin(); iter != amb_decision.end(); ++iter)
        {
            double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child(type2child[type].c_str()).attribute(iter->first.c_str()).as_double();
            if (!double_eq(tmp, 0))
                iter->second = tmp;
        }
        _gmutex.unlock();
        return amb_decision;
    }

    bool t_gsetamb::part_ambfix()
    {
        _gmutex.lock();

        istringstream is(_doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("part_fix"));
        string tmp;
        is >> tmp;
        bool is_part_fix = (tmp == "YES" || tmp == "yes");

        _gmutex.unlock();
        return is_part_fix;
    }

    int t_gsetamb::part_ambfix_num()
    {
        _gmutex.lock();

        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("part_fix_num");

        if (tmp.empty())
        {
            _gmutex.unlock();
            return 2;
        }
        _gmutex.unlock();
        return str2dbl(tmp);
    }

    int t_gsetamb::full_fix_num()
    {
        _gmutex.lock();

        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("full_fix_num");

        if (tmp.empty())
        {
            _gmutex.unlock();
            return 3;
        }
        _gmutex.unlock();
        return str2int(tmp);
    }

    FIX_MODE t_gsetamb::str2fixmode(string str)
    {
        if (str == "NO")
        {
            return FIX_MODE::NO;
        }
        else if (str == "SEARCH")
        {
            return FIX_MODE::SEARCH;
        }
        else
        {
            cout << "*** warning: not defined ambiguity fixing mode [" << str << "]\n";
            cout.flush();
        }
        return FIX_MODE::NO;
    }

    string t_gsetamb::fixmode2str(FIX_MODE mode)
    {
        switch (mode)
        {
        case FIX_MODE::NO:
            return "NO";
        case FIX_MODE::SEARCH:
            return "SEARCH";
        }
    }

    UPD_MODE t_gsetamb::str2upd_mode(string str)
    {
        if (str == "UPD" || str == "upd")
        {
            return UPD_MODE::UPD;
        }
        else
        {
            spdlog::warn("warning: not defined upd mode[" + str + "]");
            return UPD_MODE::UPD;
        }
    }

    bool t_gsetamb::isSetRefSat()
    {
        _gmutex.lock();

        istringstream is(_doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("set_refsat"));
        string tmp;
        is >> tmp;
        bool isSetRefsat = (tmp == "YES" || tmp == "yes");

        _gmutex.unlock();
        return isSetRefsat;
    }

    string updmode2str(UPDTYPE mode)
    {
        string str;
        switch (mode)
        {
        case UPDTYPE::EWL:
            str = "EWL";
            break;
        case UPDTYPE::EWL24:
            str = "EWL24";
            break;
        case UPDTYPE::EWL25:
            str = "EWL25";
            break;
        case UPDTYPE::NL:
            str = "NL";
            break;
        case UPDTYPE::WL:
            str = "WL";
            break;
        case UPDTYPE::IFCB:
            str = "IFCB";
            break;
        case UPDTYPE::NONE:
            str = "NONE";
            break;
        default:
            str = "NONE";
            break;
        }
        return str;
    }

    UPDTYPE str2updmode(string mode)
    {
        if (mode == "IFCB")
            return UPDTYPE::IFCB;
        else if (mode == "EWL")
            return UPDTYPE::EWL;
        else if (mode == "EWL24")
            return UPDTYPE::EWL24;
        else if (mode == "EWL25")
            return UPDTYPE::EWL25;
        else if (mode == "WL")
            return UPDTYPE::WL;
        else if (mode == "NL")
            return UPDTYPE::NL;
        else
            return UPDTYPE::NONE;
    }

}
