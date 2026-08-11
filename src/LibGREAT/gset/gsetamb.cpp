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

using namespace pugi;

namespace great
{
    t_gsetamb::t_gsetamb() :
        t_gsetbase()
    {

        _set.insert(XMLKEY_AMBIGUITY);

    }
    t_gsetamb::~t_gsetamb()
    {
    }
    void t_gsetamb::check()
    {
        return;
    }
    void t_gsetamb::help()
    {
        std::cerr << "<ambiguity>\n"
             << "<upd_mode> upd </upd_mode>\n"
             << "<fix_mode> SEARCH/NO </fix_mode>\n"
             << "<ratio> 3.0 </ratio>\n"
             << "<all_baselines> NO </all_baselines>\n"
             << "<min_common_time> 30 </min_common_time>\n"
             << "<widelane_decision     maxdev = \"0.15\" maxsig = \"0.10\" alpha = \"1000\"/>\n"
             << "<narrowlane_decision   maxdev = \"0.15\" maxsig = \"0.10\" alpha = \"1000\"/>\n";

        return;
    }

    FIX_MODE t_gsetamb::fix_mode()
    {
        if (!_doc)
        {
            str2fixmode(std::string());
        }
        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("fix_mode");
        return str2fixmode(trim(tmp));
    }

    UPD_MODE t_gsetamb::upd_mode()
    {
        if (!_doc)
        {
            str2upd_mode(std::string());
        }
        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("upd_mode");
        return str2upd_mode(trim(tmp));
    }

    double t_gsetamb::lambda_ratio()
    {

        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("ratio");

        return str2dbl(tmp);
    }

    double t_gsetamb::bootstrapping()
    {

        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("boot");

        if (tmp.empty())
        {
            return -0.001;
        }

        return str2dbl(tmp);
    }

    double t_gsetamb::min_common_time()
    {

        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("min_common_time");

        return str2dbl(tmp);
    }

    std::map<std::string, double> t_gsetamb::get_amb_decision(std::string type)
    {
        std::map<std::string, std::string> type2child = {{"EWL", "extra_widelane_decision"}, {"WL", "widelane_decision"}, {"NL", "narrowlane_decision"}};
        std::map<std::string, double> amb_decision;
        if (_default_decision.find(type) == _default_decision.end())
        {
            return amb_decision;
        }
        amb_decision = _default_decision[type];
        for (auto iter = amb_decision.begin(); iter != amb_decision.end(); ++iter)
        {
            double tmp =
                _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child(type2child[type].c_str()).attribute(iter->first.c_str()).as_double();
            if (!double_eq(tmp, 0))
            {
                iter->second = tmp;
            }
        }
        return amb_decision;
    }

    bool t_gsetamb::part_ambfix()
    {

        std::istringstream is(_doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("part_fix"));
        std::string tmp;
        is >> tmp;
        bool is_part_fix = (tmp == "YES" || tmp == "yes");

        return is_part_fix;
    }

    int t_gsetamb::part_ambfix_num()
    {

        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("part_fix_num");

        if (tmp.empty())
        {
            return 2;
        }
        return str2dbl(tmp);
    }

    int t_gsetamb::full_fix_num()
    {

        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("full_fix_num");

        if (tmp.empty())
        {
            return 3;
        }
        return str2int(tmp);
    }

    FIX_MODE t_gsetamb::str2fixmode(std::string str)
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
            std::cout << "*** warning: not defined ambiguity fixing mode [" << str << "]\n";
            std::cout.flush();
        }
        return FIX_MODE::NO;
    }

    std::string t_gsetamb::fixmode2str(FIX_MODE mode)
    {
        switch (mode)
        {
            case FIX_MODE::NO:
                return "NO";
            case FIX_MODE::SEARCH:
                return "SEARCH";
        }
    }

    UPD_MODE t_gsetamb::str2upd_mode(std::string str)
    {
        if (str == "UPD" || str == "upd")
        {
            return UPD_MODE::UPD;
        }
        else
        {
            GREAT_WARN("warning: not defined upd mode[" + str + "]");
            return UPD_MODE::UPD;
        }
    }

    bool t_gsetamb::isSetRefSat()
    {

        std::istringstream is(_doc.child(XMLKEY_ROOT).child(XMLKEY_AMBIGUITY).child_value("set_refsat"));
        std::string tmp;
        is >> tmp;
        bool isSetRefsat = (tmp == "YES" || tmp == "yes");

        return isSetRefsat;
    }

    std::string updmode2str(UPDTYPE mode)
    {
        std::string str;
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

    UPDTYPE str2updmode(std::string mode)
    {
        if (mode == "IFCB")
        {
            return UPDTYPE::IFCB;
        }
        else if (mode == "EWL")
        {
            return UPDTYPE::EWL;
        }
        else if (mode == "EWL24")
        {
            return UPDTYPE::EWL24;
        }
        else if (mode == "EWL25")
        {
            return UPDTYPE::EWL25;
        }
        else if (mode == "WL")
        {
            return UPDTYPE::WL;
        }
        else if (mode == "NL")
        {
            return UPDTYPE::NL;
        }
        else
        {
            return UPDTYPE::NONE;
        }
    }

} // namespace great
