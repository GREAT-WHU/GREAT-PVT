/**
*
* @verbatim
     (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  @endverbatim
*
* @file        gsetgnss.cpp
* @brief       implements data extraction setting
* @author      Jan Dousa
* @version     1.0.0
* @date        2012-10-23
*
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "gset/gsetgnss.h"
#include "gutils/gnss.h"
#include "gutils/gobs.h"
#include "gutils/gsys.h"

using namespace pugi;

namespace gnut
{
    t_gsetgnss::t_gsetgnss() :
        t_gsetbase()
    {
        _set.insert(XMLKEY_GNSS);
        std::stringstream os;
        _sigma_def[GSYS::GPS] = t_gpair(2.0, 0.02);
        _sigma_def[GSYS::GLO] = t_gpair(4.0, 0.04);
        _sigma_def[GSYS::GAL] = t_gpair(3.0, 0.03);
        _sigma_def[GSYS::BDS] = t_gpair(5.0, 0.03);
        _sigma_def[GSYS::QZS] = t_gpair(5.0, 0.03);
        _sigma_def[GSYS::IRN] = t_gpair(5.0, 0.03);
        _sigma_def_doppler[GSYS::GPS] = 0.04;
        _sigma_def_doppler[GSYS::GLO] = 0.08;
        _sigma_def_doppler[GSYS::GAL] = 0.06;
        _sigma_def_doppler[GSYS::BDS] = 0.06;
        _sigma_def_doppler[GSYS::QZS] = 0.06;
        _sigma_def_doppler[GSYS::IRN] = 0.06;

        _maxres_def[GSYS::GPS] = t_gpair(10.0, 0.08);
        _maxres_def[GSYS::GLO] = t_gpair(15.0, 0.08);
        _maxres_def[GSYS::GAL] = t_gpair(15.0, 0.08);
        _maxres_def[GSYS::BDS] = t_gpair(15.0, 0.08);
        _maxres_def[GSYS::QZS] = t_gpair(15.0, 0.08);
        _maxres_def[GSYS::IRN] = t_gpair(15.0, 0.08);
        _maxres_def_doppler[GSYS::GPS] = 0.1;
        _maxres_def_doppler[GSYS::GLO] = 0.1;
        _maxres_def_doppler[GSYS::GAL] = 0.1;
        _maxres_def_doppler[GSYS::BDS] = 0.1;
        _maxres_def_doppler[GSYS::QZS] = 0.1;
        _maxres_def_doppler[GSYS::IRN] = 0.1;

        t_map_gnss gnss_data = GNSS_DATA_PRIORITY();
        for (auto itGNSS = gnss_data.begin(); itGNSS != gnss_data.end(); ++itGNSS)
        {
            GSYS gsys = itGNSS->first;
            std::string gs = t_gsys::gsys2str(gsys);

            for (auto itBAND = gnss_data[gsys].begin(); itBAND != gnss_data[gsys].end(); ++itBAND)
            {
                GOBSBAND gobsband = itBAND->first;
                std::string band = gobsband2str(gobsband);
                _band_str[gsys].push_back(band);

                for (auto itTYPE = gnss_data[gsys][gobsband].begin(); itTYPE != gnss_data[gsys][gobsband].end(); ++itTYPE)
                {
                    GOBSTYPE gobstype = itTYPE->first;
                    std::string type = gobstype2str(gobstype);
                    os << gs << "  band: " << band << "  type: " << type << "  attr:";

                    std::set<std::string> type_search(_type_str[gsys].begin(), _type_str[gsys].end());
                    if (type_search.find(type) == type_search.end())
                    {
                        _type_str[gsys].push_back(type);
                    }

                    for (auto itATTR = gnss_data[gsys][gobsband][gobstype].begin(); itATTR != gnss_data[gsys][gobsband][gobstype].end();
                         ++itATTR)
                    {
                        GOBSATTR gobsattr = *itATTR;
                        std::string attr = gobsattr2str(gobsattr);
                        os << " " << attr;

                        std::set<std::string> attr_search(_attr_str[gsys].begin(), _attr_str[gsys].end());
                        if (attr_search.find(attr) == attr_search.end())
                        {
                            _attr_str[gsys].push_back(attr);
                        }
                    }
                    os << std::endl;
                }
            }
            os << std::endl;
        }
    }

    t_gsetgnss::~t_gsetgnss()
    {
    }

    std::set<GSAT> t_gsetgnss::sat()
    {

        std::set<GSAT> tmp;

        t_map_sats gnss_sats = GNSS_SATS();
        t_map_sats::const_iterator itGNS;
        for (itGNS = gnss_sats.begin(); itGNS != gnss_sats.end(); ++itGNS)
        {
            GSYS gsys = itGNS->first;
            std::string gs = t_gsys::gsys2str(gsys);
            transform(gs.begin(), gs.end(), gs.begin(), ::towlower);
            std::set<std::string> sats = t_gsetbase::_setval(gs, "sat");
            for (const std::string& sat : sats)
            {
                GSAT value = str2gsat(sat, gsys);
                if (value != GSAT::NONE)
                {
                    tmp.insert(value);
                }
            }
        }

        return tmp;
    }

    std::set<GSAT> t_gsetgnss::sat(GSYS gsys, bool def)
    {

        std::set<GSAT> tmp;
        for (const std::string& sat : t_gsetbase::_setval(_gsys(gsys), "sat"))
        {
            GSAT value = str2gsat(sat, gsys);
            if (value != GSAT::NONE)
            {
                tmp.insert(value);
            }
        }
        if (def && tmp.size() == 0)
        {
            tmp = GNSS_SATS()[gsys];
        }

        return tmp;
    }

    std::set<std::string> t_gsetgnss::nav(GSYS gsys, bool def)
    {
        std::set<std::string> tmp = t_gsetbase::_setval(_gsys(gsys), "nav");
        if (def && tmp.size() == 0)
        {
            tmp = GNSS_GNAV()[gsys];
        }

        return tmp;
    }

    std::set<std::string> t_gsetgnss::obs(GSYS gsys, bool def)
    {

        std::string str;
        std::set<std::string> tmp;

        std::vector<std::string> type = t_gsetbase::_vecval(_gsys(gsys), "type");
        std::vector<std::string> band = t_gsetbase::_vecval(_gsys(gsys), "band");
        std::vector<std::string> attr = t_gsetbase::_vecval(_gsys(gsys), "attr");

        std::vector<std::string>::const_iterator itT;
        std::vector<std::string>::const_iterator itB;
        std::vector<std::string>::const_iterator itA;

        std::string gs = t_gsys::gsys2str(gsys);
        std::ostringstream os;
        os << gs << "  band: " << band.size() << "  type: " << type.size() << "  attr: " << attr.size() << std::endl;

        int bset = band.size();
        int tset = type.size();
        int aset = attr.size();

        if (bset == 0)
        {
            band.assign(_band_str[gsys].begin(), _band_str[gsys].end());
        }
        if (tset == 0)
        {
            type.assign(_type_str[gsys].begin(), _type_str[gsys].end());
        }
        if (aset == 0)
        {
            attr.assign(_attr_str[gsys].begin(), _attr_str[gsys].end());
        }

        for (itB = band.begin(); itB != band.end(); ++itB)
        {
            GOBSBAND gband = char2gobsband((*itB)[0]);
            std::string b = gobsband2str(gband);
            if (gband == GOBSBAND::BAND)
            {
                continue;
            }

            for (itT = type.begin(); itT != type.end(); ++itT)
            {
                GOBSTYPE gtype = char2gobstype((*itT)[0]);
                std::string t = gobstype2str(gtype);
                if (gtype == GOBSTYPE::TYPE)
                {
                    continue;
                }

                os << gs << "  band: " << b << "  type: " << t << "  attr:";

                for (itA = attr.begin(); itA != attr.end(); ++itA)
                {
                    GOBSATTR gattr = char2gobsattr((*itA)[0]);
                    std::string a = gobsattr2str(gattr);
                    if (gattr == GOBSATTR::ATTR)
                    {
                        continue;
                    }

                    str = gobstype2str(gtype);
                    str += gobsband2str(gband);
                    str += gobsattr2str(gattr);

                    if (def || (bset || tset || aset))
                    {
                        tmp.insert(str);
                    }

                    os << " " << a;
                }
                os << std::endl;
            }
        }

        return tmp;
    }

    std::set<std::string> t_gsetgnss::gobs(GSYS gsys)
    {

        std::string str;
        std::set<std::string> tmp;

        std::vector<std::string> vgobs = t_gsetbase::_vecval(_gsys(gsys), "gobs");

        for (auto it = vgobs.begin(); it != vgobs.end(); it++)
        {
            tmp.insert(*it);
        }
        return tmp;
    }

    std::vector<GOBSTYPE> t_gsetgnss::type(GSYS gsys)
    {

        std::vector<GOBSTYPE> v_tmp = _type(gsys);

        return v_tmp;
    }

    std::vector<GOBSBAND> t_gsetgnss::band(GSYS gsys)
    {

        std::vector<GOBSBAND> v_tmp = _band(gsys);

        return v_tmp;
    }

    std::vector<FREQ_SEQ> t_gsetgnss::freqs(GSYS gsys)
    {

        std::vector<FREQ_SEQ> v_tmp = _freqs(gsys);

        if (v_tmp.empty())
        {
            v_tmp.push_back(gnut::FREQ_SEQ::FREQ_1);
            v_tmp.push_back(gnut::FREQ_SEQ::FREQ_2);
        }

        return v_tmp;
    }

    std::map<FREQ_SEQ, GOBSBAND> t_gsetgnss::band_index(GSYS gsys)
    {
        auto XmlFreq = freqs(gsys);
        auto XmlBand = band(gsys);

        if (XmlFreq.size() != XmlBand.size())
        {
            throw std::logic_error("Band size should be the same with freq size in xml.");
        }

        std::map<FREQ_SEQ, GOBSBAND> tmp;
        for (unsigned int i = 0; i < XmlBand.size(); i++)
        {
            tmp[XmlFreq[i]] = XmlBand[i];
        }

        return tmp;
    }

    std::map<GOBSBAND, FREQ_SEQ> t_gsetgnss::freq_index(GSYS gsys)
    {
        auto XmlFreq = freqs(gsys);
        auto XmlBand = band(gsys);

        if (XmlFreq.size() != XmlBand.size())
        {
            throw std::logic_error("Band size should be the same with freq size in xml.");
        }

        std::map<GOBSBAND, FREQ_SEQ> tmp;
        for (unsigned int i = 0; i < XmlBand.size(); i++)
        {
            tmp[XmlBand[i]] = XmlFreq[i];
        }

        return tmp;
    }

    std::vector<GOBSATTR> t_gsetgnss::attr(GSYS gsys)
    {

        std::vector<GOBSATTR> v_tmp = _attr(gsys);

        return v_tmp;
    }

    double t_gsetgnss::sigma_C(GSYS gsys)
    {

        double tmp = _sigma_C(gsys);

        return tmp;
    }

    double t_gsetgnss::sigma_D(GSYS gsys)
    {

        double tmp = _sigma_D(gsys);

        return tmp;
    }

    double t_gsetgnss::sigma_L(GSYS gsys)
    {

        double tmp = _sigma_L(gsys);

        return tmp;
    }

    double t_gsetgnss::maxres_C(GSYS gsys)
    {

        double tmp = _maxres_C(gsys);

        return tmp;
    }

    double t_gsetgnss::maxres_D(GSYS gsys)
    {

        double tmp = _maxres_D(gsys);

        return tmp;
    }

    double t_gsetgnss::maxres_L(GSYS gsys)
    {

        double tmp = _maxres_L(gsys);

        return tmp;
    }

    void t_gsetgnss::check()
    {

        return;
    }

    std::string t_gsetgnss::_gsys(GSYS gsys)
    {
        std::string tmp = t_gsys::gsys2str(gsys);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
        return tmp;
    }

    std::vector<GOBSTYPE> t_gsetgnss::_type(GSYS gsys)
    {
        std::vector<std::string> v_str = t_gsetbase::_vecval(_gsys(gsys), "type");
        std::vector<GOBSTYPE> v_tmp;
        for (std::vector<std::string>::const_iterator it = v_str.begin(); it != v_str.end(); ++it)
        {
            std::string str = *it;
            transform(str.begin(), str.end(), str.begin(), ::toupper);
            GOBSTYPE gobstype = str2gobstype(str);
            if (gobstype != gnut::GOBSTYPE::TYPE)
            {
                v_tmp.push_back(gobstype);
            }
        };
        return v_tmp;
    }

    std::vector<GOBSBAND> t_gsetgnss::_band(GSYS gsys)
    {
        std::vector<std::string> v_str = t_gsetbase::_vecval(_gsys(gsys), "band");
        std::vector<GOBSBAND> v_tmp;
        for (std::vector<std::string>::const_iterator it = v_str.begin(); it != v_str.end(); ++it)
        {
            std::string str = *it;
            transform(str.begin(), str.end(), str.begin(), ::toupper);
            GOBSBAND gobsband = str2gobsband(str);
            if (gobsband != gnut::GOBSBAND::BAND)
            {
                v_tmp.push_back(gobsband);
            }
        };

        if (v_tmp.empty())
        {
            throw std::logic_error("You need set your band for " + t_gsys::gsys2str(gsys) + " in xml file");

            switch (gsys)
            {
                case GSYS::GPS:
                    v_tmp.push_back(gnut::GOBSBAND::BAND_1);
                    v_tmp.push_back(gnut::GOBSBAND::BAND_2);
                    break;
                case GSYS::GLO:
                    v_tmp.push_back(gnut::GOBSBAND::BAND_1);
                    v_tmp.push_back(gnut::GOBSBAND::BAND_2);
                    break;
                case GSYS::GAL:
                    v_tmp.push_back(gnut::GOBSBAND::BAND_1);
                    v_tmp.push_back(gnut::GOBSBAND::BAND_5);
                    break;
                case GSYS::BDS:
                    v_tmp.push_back(gnut::GOBSBAND::BAND_2);
                    v_tmp.push_back(gnut::GOBSBAND::BAND_6);
                    break;
                case GSYS::QZS:
                    v_tmp.push_back(gnut::GOBSBAND::BAND_1);
                    v_tmp.push_back(gnut::GOBSBAND::BAND_2);
                default:
                    break;
            }
        }

        return v_tmp;
    }

    std::vector<FREQ_SEQ> t_gsetgnss::_sysfreq(GSYS gsys)
    {
        std::vector<std::string> v_str = t_gsetbase::_vecval(_gsys(gsys), "freq");
        std::vector<FREQ_SEQ> v_tmp;
        for (std::vector<std::string>::const_iterator it = v_str.begin(); it != v_str.end(); ++it)
        {
            std::string str = *it;
            FREQ_SEQ gsysfreq = str2sysfreq(str);
            if (gsysfreq != gnut::FREQ_SEQ::FREQ_X)
            {
                v_tmp.push_back(gsysfreq);
            }
        };

        if (v_tmp.empty())
        {
            switch (gsys)
            {
                case GSYS::GPS:
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_1);
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_2);
                    break;
                case GSYS::GLO:
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_1);
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_2);
                    break;
                case GSYS::GAL:
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_1);
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_2);
                    break;
                case GSYS::BDS:
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_1);
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_2);
                    break;
                case GSYS::QZS:
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_1);
                    v_tmp.push_back(gnut::FREQ_SEQ::FREQ_2);
                default:
                    break;
            }

            std::cout << "Warning : do not set the freq, use default settings" << std::endl;
        }

        return v_tmp;
    }

    std::vector<FREQ_SEQ> t_gsetgnss::_freqs(GSYS gsys)
    {
        std::vector<std::string> v_str = t_gsetbase::_vecval(_gsys(gsys), "freq");
        std::vector<FREQ_SEQ> v_tmp;
        for (std::vector<std::string>::const_iterator it = v_str.begin(); it != v_str.end(); ++it)
        {
            std::string str = *it;
            transform(str.begin(), str.end(), str.begin(), ::toupper);
            FREQ_SEQ gobsband = str2gnssfreq(str);
            if (gobsband != FREQ_SEQ::FREQ_X)
            {
                v_tmp.push_back(gobsband);
            }
        };

        if (v_tmp.empty())
        {
            v_tmp.push_back(gnut::FREQ_SEQ::FREQ_1);
            v_tmp.push_back(gnut::FREQ_SEQ::FREQ_2);
        }

        return v_tmp;
    }

    std::vector<GOBSATTR> t_gsetgnss::_attr(GSYS gsys)
    {
        std::vector<std::string> v_str = t_gsetbase::_vecval(_gsys(gsys), "attr");
        std::vector<GOBSATTR> v_tmp;
        for (std::vector<std::string>::const_iterator it = v_str.begin(); it != v_str.end(); ++it)
        {
            std::string str = *it;
            transform(str.begin(), str.end(), str.begin(), ::toupper);
            GOBSATTR gobsattr = str2gobsattr(str);
            if (gobsattr != gnut::GOBSATTR::ATTR)
            {
                v_tmp.push_back(gobsattr);
            }
        };
        return v_tmp;
    }

    double t_gsetgnss::_sigma_C(GSYS gsys)
    {
        double sig = 0.0;
        std::string sys = _gsys(gsys);
        sig = t_gsetbase::_dblatt(sys, "sigma_C");

        if (double_eq(sig, 0.0))
        {
            sig = _sigma_def[gsys][0];
        }

        return sig;
    }

    double t_gsetgnss::_sigma_D(GSYS gsys)
    {
        double sig = 0.0;

        sig = t_gsetbase::_dblatt(_gsys(gsys), "sigma_D");

        if (double_eq(sig, 0.0))
        {
            sig = _sigma_def_doppler[gsys];
        }

        return sig;
    }

    double t_gsetgnss::_sigma_L(GSYS gsys)
    {
        double sig = 0.0;
        std::string sys = _gsys(gsys);
        sig = t_gsetbase::_dblatt(sys, "sigma_L");

        if (double_eq(sig, 0.0))
        {
            sig = _sigma_def[gsys][1];
        }

        return sig;
    }

    double t_gsetgnss::_maxres_C(GSYS gsys)
    {
        double res = 0.0;

        res = t_gsetbase::_dblatt(_gsys(gsys), "maxres_C");

        if (double_eq(res, 0.0))
        {
            res = _maxres_def[gsys][0];
        }

        return res;
    }

    double t_gsetgnss::_maxres_D(GSYS gsys)
    {
        double sig = 0.0;
        sig = t_gsetbase::_dblatt(_gsys(gsys), "sigma_D");

        if (double_eq(sig, 0.0))
        {
            sig = _maxres_def_doppler[gsys];
        }

        return sig;
    }

    double t_gsetgnss::_maxres_L(GSYS gsys)
    {
        double res = 0.0;

        res = t_gsetbase::_dblatt(_gsys(gsys), "maxres_L");

        if (double_eq(res, 0.0))
        {
            res = _maxres_def[gsys][1];
        }

        return res;
    }

    void t_gsetgnss::help()
    {

        std::cerr << " <gps>            \t\t  <!-- any GNSS constellation: GPS GLO GAL BDS SBS QZS -->\n"
             << "   <sat>  </sat>  \t\t  <!-- list of GPS satellites: G01 G02 .. or empty(ALL) -->\n"
             << "   <type> </type> \t\t  <!-- list of GPS  obs types: C L D S P or empty(ALL)  -->\n"
             << "   <band> </band> \t\t  <!-- list of GPS  obs bands: 1 2 5 or empty(ALL) -->\n"
             << "   <attr> </attr> \t\t  <!-- list of PGS attributes: A B C D I L M N P Q S W X Y Z or empty(ALL) -->\n"
             << " </gps>\n\n";

        std::cerr << " <glo>            \t\t  <!-- any GNSS constellation: GPS GLO GAL BDS SBS QZS -->\n"
             << "   <sat>  </sat>  \t\t  <!-- list of GPS satellites: R01 R02 .. or empty(ALL) -->\n"
             << "   <type> </type> \t\t  <!-- list of GPS  obs types: C L D S P or empty(ALL)  -->\n"
             << "   <band> </band> \t\t  <!-- list of GPS  obs bands: 1 2 3 or empty(ALL) -->\n"
             << "   <attr> </attr> \t\t  <!-- list of PGS attributes: A B C D I L M N P Q S W X Y Z or empty(ALL) -->\n"
             << " </glo>\n\n";

        std::cerr << "\t<!-- gnss observation definition:\n"
             << "\t      .. ID name\n"
             << "\t name   .. full name\n"
             << "\t desc   .. description\n"
             << "\t -->\n\n";

        return;
    }

} // namespace gnut
