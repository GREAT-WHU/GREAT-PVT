/**
*
* @verbatim
(c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
Ondrejov 244, 251 65, Czech Republic
@endverbatim
*
* @file        gsetinp.cpp
* @brief       implements input setting class
* @author      Jan Dousa
* @version     1.0.0
* @date        2012-10-23
*
*/

#include <iomanip>
#include <sstream>
#include <algorithm>

#include "gset/gsetinp.h"
#include "gutils/gfileconv.h"
#include "gutils/gmutex.h"
using namespace std;
using namespace pugi;

namespace gnut
{
    IFMT t_gsetinp::str2ifmt(const string &s)
    {
        string tmp = s;
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        if (tmp == "RINEXC" || tmp == "RNC")
            return IFMT::RINEXC_INP;
        if (tmp == "RINEXO" || tmp == "RNO")
            return IFMT::RINEXO_INP;
        if (tmp == "RINEXN" || tmp == "RNN")
            return IFMT::RINEXN_INP;
        if (tmp == "SP3")
            return IFMT::SP3_INP;
        if (tmp == "ATX")
            return IFMT::ATX_INP;
        if (tmp == "BLQ")
            return IFMT::BLQ_INP;
        if (tmp == "BIASINEX")
            return IFMT::BIASINEX_INP;
        if (tmp == "BIAS" || tmp == "BIABERN")
            return IFMT::BIAS_INP;
        if (tmp == "DE")
            return IFMT::DE_INP;
        if (tmp == "EOP" || tmp == "POLEUT1") 
            return IFMT::EOP_INP; // optinal for xml node
        if (tmp == "LEAPSECOND")
            return IFMT::LEAPSECOND_INP;
        if (tmp == "UPD")
            return IFMT::UPD_INP;
        if (tmp == "IFCB")
            return IFMT::IFCB_INP;

        string message = "The Type : " + tmp + " is not support, check your xml";
        spdlog::warn(message);
        throw logic_error(message);
    }

    string t_gsetinp::ifmt2str(const IFMT &f)
    {
        switch (f)
        {
        case IFMT::RINEXO_INP:
            return "RINEXO";
        case IFMT::RINEXC_INP:
            return "RINEXC";
        case IFMT::RINEXN_INP:
            return "RINEXN";
        case IFMT::SP3_INP:
            return "SP3";
        case IFMT::ATX_INP:
            return "ATX";
        case IFMT::BLQ_INP:
            return "BLQ";
        case IFMT::BIASINEX_INP:
            return "BIASINEX";
        case IFMT::BIAS_INP:
            return "BIAS";
        case IFMT::UPD_INP:
            return "UPD";
        case IFMT::IFCB_INP:
            return "IFCB";
        case IFMT::LEAPSECOND_INP:
            return "LEAPSECOND";
        case IFMT::DE_INP:
            return "DE";
		case IFMT::EOP_INP:
			return "EOP";
        default:
            spdlog::critical("No fmt for {}, check your inp.", f);
            throw logic_error("check your inp");
        }
    }

    t_gsetinp::t_gsetinp()
        : t_gsetbase()
    {
        _set.insert(XMLKEY_INP);
        _chkNavig = true;
        _chkHealth = true;
        _corrStream = "";
    }

    t_gsetinp::~t_gsetinp()
    {
    }

    int t_gsetinp::input_size(const string &fmt)
    {
        _gmutex.lock();

        int tmp = _inputs(fmt).size();

        _gmutex.unlock();
        return tmp;
    }

    bool t_gsetinp::check_input(const string &fmt)
    {
        _gmutex.lock();
        int tmp = _inputs(fmt).size();
        _gmutex.unlock();
        return (tmp > 0);
    }

    void t_gsetinp::check_input(const string &fmt, const string &message)
    {
        _gmutex.lock();
        int tmp = _inputs(fmt).size();
        _gmutex.unlock();
        if (tmp < 0)
        {
            spdlog::critical(message);
            throw logic_error(message);
        }
    }

    multimap<IFMT, string> t_gsetinp::inputs_all()
    {
        _gmutex.lock();

        multimap<IFMT, string> map;

        set<string> ifmt = _iformats();
        set<string>::const_iterator itFMT = ifmt.begin();

        while (itFMT != ifmt.end())
        {
            string fmt = *itFMT;
            if (fmt.empty())
            {
                itFMT++;
                continue; 
            }

            IFMT ifmt = str2ifmt(fmt);
            vector<string> inputs = _inputs(fmt); //get file name in input node
            vector<string>::const_iterator itINP = inputs.begin();
            while (itINP != inputs.end())
            {
                map.insert(map.end(), pair<IFMT, string>(ifmt, *itINP));
                itINP++;
            }
            itFMT++;
        }
        _gmutex.unlock();
        return map;
    }

    vector<string> t_gsetinp::inputs(const string &fmt)
    {
        IFMT ifmt = str2ifmt(fmt);
        return _inputs(ifmt);
    }

    vector<string> t_gsetinp::inputs(const IFMT &ifmt)
    {
        return _inputs(ifmt);
    }

    vector<string> t_gsetinp::_inputs(const string &fmt)
    {
        vector<string> tmp;
        set<string> list;
        string str;

        for (xml_node node = _doc.child(XMLKEY_ROOT).child(XMLKEY_INP).first_child(); node; node = node.next_sibling())
        {
            if (node.name() == fmt)
            {
                istringstream is(node.child_value());
                while (is >> str && !is.fail())
                {
                    if (str.find("://") == string::npos)
                        str = GFILE_PREFIX + str;
                    if (list.find(str) == list.end())
                    {
                        tmp.push_back(str);
                        list.insert(str);
                    }
                    else
                    {
                        cout << "READ : " + str + " multiple request ignored" << endl;
                    }
                }
            }
        }
        return tmp;
    }

    vector<string> t_gsetinp::_inputs(const IFMT &fmt)
    {
        vector<string> tmp;
        set<string> list;
        string str;

        for (xml_node node = _doc.child(XMLKEY_ROOT).child(XMLKEY_INP).first_child(); node; node = node.next_sibling())
        {
            // jdhuang
            IFMT ifmt = str2ifmt(node.name());
            if (ifmt == IFMT::UNDEF)
                continue;
            if (ifmt == fmt)
            {
                istringstream is(node.child_value());
                while (is >> str && !is.fail())
                {
                    if (str.find("://") == string::npos)
                        str = GFILE_PREFIX + str;
                    if (list.find(str) == list.end())
                    {
                        tmp.push_back(str);
                        list.insert(str);
                    }
                    else
                    {
                        cout << "READ : " + str + " multiple request ignored" << endl;
                    }
                }
            }
        }
        return tmp;
    }

    set<string> t_gsetinp::_iformats()
    {
        set<string> tmp;
        for (xml_node node = _doc.child(XMLKEY_ROOT).child(XMLKEY_INP).first_child(); node; node = node.next_sibling())
        {
            tmp.insert(node.name());
        }
        return tmp;
    }

    void t_gsetinp::check()
    {
        _gmutex.lock();

        // check existence of nodes/attributes
        xml_node parent = _doc.child(XMLKEY_ROOT);
        xml_node node = _default_node(parent, XMLKEY_INP);

        // check supported input formats (see IFMT enum !)
        set<string> ifmt = _iformats();
        set<string>::const_iterator itFMT = ifmt.begin();
        while (itFMT != ifmt.end())
        {
            string fmt = *itFMT;
            try
            {
                str2ifmt(fmt);
            }
            catch (const std::exception &e)
            {
                _doc.child(XMLKEY_ROOT).child(XMLKEY_INP).remove_child(node.child(fmt.c_str()));
                itFMT++;
                continue;
            }
            // check application-specific output format
            if (_IFMT_supported.find(str2ifmt(fmt)) == _IFMT_supported.end())
            {
                _doc.child(XMLKEY_ROOT).child(XMLKEY_INP).remove_child(node.child(fmt.c_str()));
                spdlog::warn(fmt + " inp format not supported by this application!");
            }
            check_input(fmt, "your fmt : " + fmt + " is empty");
            itFMT++;
        }

        _default_attr(node, "chk_nav", _chkNavig);
        _default_attr(node, "chk_health", _chkHealth);

        xml_node nodeBNCRTCM = _doc.child(XMLKEY_ROOT).child(XMLKEY_INP).child("bncrtcm");
        _default_attr(nodeBNCRTCM, "_corrStream", _corrStream);

        _gmutex.unlock();
        return;
    }

    void t_gsetinp::help()
    {
        _gmutex.lock();

        cerr << " <inputs>\n"
             << "   <rinexo> file://dir/name </rinexo> \t\t <!-- obs RINEX decoder -->\n"
             << "   <rinexn> file://dir/name </rinexn> \t\t <!-- nav RINEX decoder -->\n"
             << " </inputs>\n";

        cerr << "\t<!-- inputs description:\n"
             << "\t <decoder> path1 path2 path3  </decoder>\n"
             << "\t ... \n"
             << "\t where path(i) contains [file,tcp,ntrip]:// depending on the application\n"
             << "\t -->\n\n";

        _gmutex.unlock();
        return;
    }

} // namespace
