/**
*
* @verbatim
     (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  @endverbatim
*
* @file        gsetgen.cpp
* @brief       implements common general settings
* @author      Jan Dousa
* @version     1.0.0
* @date        2012-10-23
*
*/

#include "gset/gsetgen.h"

#include <iomanip>
#include <sstream>
#include <algorithm>
#include "gdata/grec.h"
#include "gutils/gnss.h"
#include "gutils/gsys.h"
#include "gutils/gtypeconv.h"

#ifndef XMLKEY_GEN
#define XMLKEY_GEN "gen"   ///< The defination of gen node
#endif
#ifndef DEF_RECEIVER
#define DEF_RECEIVER "   " ///< Default receiver : all !
#endif
#ifndef DEF_SAMPLING
#define DEF_SAMPLING 30    ///< Default sampling : 30s !
#endif

using namespace pugi;

namespace gnut
{
    t_gsetgen::t_gsetgen(bool gnss) :
        t_gsetbase(),
        _gnss(gnss),
        _dec(0)
    {
        _set.insert(XMLKEY_GEN);

        if (_gnss)
        {
            t_map_sats gnss_sats = GNSS_SATS();
            for (auto itGNS = gnss_sats.begin(); itGNS != gnss_sats.end(); ++itGNS)
            {
                _sys += " " + t_gsys::gsys2str(itGNS->first);
            }
        }
    }

    t_gsetgen::~t_gsetgen()
    {
    }

    t_gtime t_gsetgen::beg(bool conv)
    {
        _gmutex.lock();

        std::string str = _doc.child(XMLKEY_ROOT).child(XMLKEY_GEN).child_value("beg");
        substitute(str, "\n", "");
        substitute(str, "\"", "");

        t_gtime gt(t_gtime::t_tsys::GPS);

        if (str.empty())
        {
            _gmutex.unlock();
            t_gtime tmp(FIRST_TIME);
            return tmp;
        }

        gt.from_str("%Y-%m-%d %H:%M:%S", trim(str), conv);
        _gmutex.unlock();
        return gt;
    }

    t_gtime t_gsetgen::end(bool conv)
    {
        _gmutex.lock();

        std::string str = _doc.child(XMLKEY_ROOT).child(XMLKEY_GEN).child_value("end");
        substitute(str, "\n", "");
        substitute(str, "\"", "");

        t_gtime gt(t_gtime::t_tsys::GPS);

        if (str.empty())
        {
            _gmutex.unlock();
            t_gtime tmp(LAST_TIME);
            return tmp;
        }

        gt.from_str("%Y-%m-%d %H:%M:%S", trim(str), conv);
        _gmutex.unlock();
        return gt;
    }

    double t_gsetgen::sampling()
    {
        _gmutex.lock();

        std::string str = _doc.child(XMLKEY_ROOT).child(XMLKEY_GEN).child_value("int");

        // delete spaces
        str.erase(remove(str.begin(), str.end(), ' '), str.end());

        double tmp = str2dbl(str);

        if (str.find(".") != std::string::npos)
        {
            _dec = str.substr(str.find(".") + 1).length(); // decimal digits resolution
        }

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetgen::sampling_default() const
    {
        return DEF_SAMPLING;
    }

    std::set<std::string> t_gsetgen::sys()
    {
        _gmutex.lock();

        std::set<std::string> xcl, tmp = t_gsetbase::_setval(XMLKEY_GEN, "sys");
        std::set<std::string>::iterator itSYS, itTMP;
        for (itTMP = tmp.begin(); itTMP != tmp.end();)
        {
            if ((*itTMP)[0] == '-')
            {
                xcl.insert(*itTMP);
                itSYS = itTMP;
                ++itTMP;
                tmp.erase(itSYS);
            }
            else
            {
                ++itTMP;
            }
        }

        // if empty, complete, i.e. if only exclusions listed (and gnss requested!)
        if (tmp.size() == 0 && _gnss)
        {
            t_map_sats gnss_sats = GNSS_SATS();
            t_map_sats::const_iterator itGNS;
            // loop over all systems
            for (itGNS = gnss_sats.begin(); itGNS != gnss_sats.end(); ++itGNS)
            {
                std::string gs = t_gsys::gsys2str(itGNS->first);
                if (xcl.find("-" + gs) == xcl.end())
                {
                    tmp.insert(gs);
                }
            }
        }
        _gmutex.unlock();
        return tmp;
    }

    void t_gsetgen::sys(std::string str)
    {
        _gmutex.lock();
        xml_node proc = _doc.child(XMLKEY_ROOT).child(XMLKEY_GEN);
        proc.remove_child("sys");
        xml_node obs_combination = proc.append_child("sys");
        obs_combination.append_child(xml_node_type::node_pcdata).set_value(str.c_str());
        _gmutex.unlock();
    }

    std::set<std::string> t_gsetgen::recs()
    {
        _gmutex.lock();
        std::set<std::string> tmp = t_gsetbase::_setvals(XMLKEY_GEN, "rec");
        _gmutex.unlock();
        return tmp;
    }

    std::set<std::string> t_gsetgen::rec_all()
    {
        _gmutex.lock();

        std::set<std::string> tmp = t_gsetbase::_setvals(XMLKEY_GEN, "rec");

        _gmutex.unlock();
        return tmp;
    }

    std::vector<std::string> t_gsetgen::list_base()
    {
        _gmutex.lock();
        std::vector<std::string> vals;
        std::string word;
        std::istringstream is(_doc.child(XMLKEY_ROOT).child(XMLKEY_GEN).child_value("base"));
        _gmutex.unlock();

        while (is >> word)
        {
            transform(word.begin(), word.end(), word.begin(), ::toupper);
            vals.push_back(word);
        }
        return vals;
    }

    std::vector<std::string> t_gsetgen::list_rover()
    {
        _gmutex.lock();
        std::vector<std::string> vals;
        std::string word;
        std::istringstream is(_doc.child(XMLKEY_ROOT).child(XMLKEY_GEN).child_value("rover"));

        while (is >> word)
        {
            transform(word.begin(), word.end(), word.begin(), ::toupper);
            vals.push_back(word);
        }
        _gmutex.unlock();
        return vals;
    }

    std::string t_gsetgen::estimator()
    {
        _gmutex.lock();

        std::set<std::string> src = t_gsetbase::_setval(XMLKEY_GEN, "est");
        _gmutex.unlock();

        if (src.empty() || *src.begin() != "FLT")
        {
            std::string tmp("FLT");
            return tmp;
        }
        else
        {
            return *src.begin();
        }
    }

    std::set<GSAT> t_gsetgen::sat_rm()
    {
        _gmutex.lock();

        std::set<std::string> text = t_gsetbase::_setval(XMLKEY_GEN, "sat_rm");
        _gmutex.unlock();
        std::set<GSAT> sats;
        for (const std::string& sat : text)
        {
            GSAT parsed = str2gsat(sat);
            if (parsed != GSAT::NONE)
            {
                sats.insert(parsed);
            }
        }
        return sats;
    }

    void t_gsetgen::sat_rm(std::string str)
    {
        _gmutex.lock();
        xml_node proc = _doc.child(XMLKEY_ROOT).child(XMLKEY_GEN);
        proc.remove_child("sat_rm");
        xml_node obs_combination = proc.append_child("sat_rm");
        obs_combination.append_child(xml_node_type::node_pcdata).set_value(str.c_str());
        _gmutex.unlock();
    }

    void t_gsetgen::check()
    {
        _gmutex.lock();

        // check existence of nodes/attributes
        xml_node parent = _doc.child(XMLKEY_ROOT);
        xml_node node = _default_node(parent, XMLKEY_GEN);

        _default_node(node, "BEG", "");                            // all!
        _default_node(node, "END", "");                            // all!
        _default_node(node, "SYS", "");                            // all!
        _default_node(node, "REC", "");                            // none
        _default_node(node, "INT", int2str(DEF_SAMPLING).c_str()); // default

        // TO CHECK USER-SETUP CONSISTENCY
        _gmutex.unlock();
        if (floor(sampling()) < 1 || _dec > 0)
        {
            if (sampling() < 0.0)
            {
                _default_node(node, "INT", "0.0", true); // reset!

                std::cout << "Warning: sampling rate settings negative: reset to 0" << std::endl;
            }
            else
            {
                std::cout << "gsetgen: sampling rate settings above 1Hz recognized" << std::endl;
            }
        }

        return;
    }

    void t_gsetgen::help()
    {
        _gmutex.lock();
        t_gtime beg(t_gtime::t_tsys::GPS);
        beg = beg - beg.sod();
        t_gtime end(t_gtime::t_tsys::GPS);
        end = beg + 86399;

        std::cerr << "\n <gen>\n"
             << "   <beg> \"" << beg.str_ymdhms() << "\" </beg>\n"  // FIRST_TIME.str("\"%Y-%m-%d %H:%M:%S\"")
             << "   <end> \"" << end.str_ymdhms() << "\" </end>\n"; // LAST_TIME.str("\"%Y-%m-%d %H:%M:%S\"")

        if (_gnss)
        {
            std::cerr << "   <sys> " << _sys << " </sys>\n"; // GNSS systems
        }

        std::cerr << "   <rec> GOPE WTZR POTS                </rec>\n" // list of site identificators
             << "   <int>" + int2str(DEF_SAMPLING) + "</int>\n"
             << " </gen>\n";

        std::cerr << "\t<!-- general description:\n"
             << "\t beg    .. beg time          (default: all)\n"
             << "\t end    .. end time          (default: all)\n"
             << "\t int    .. data sampling     (default: 30s)\n";

        if (_gnss)
        {
            std::cerr << "\t sys    .. GNSS system(s)    (default: all)\n";
        }

        std::cerr << "\t rec    .. GNSS receiver(s)  (rec active list, e.g.: GOPE ONSA WTZR ... )\n"
             << "\t -->\n\n";

        _gmutex.unlock();
        return;
    }

} // namespace gnut
