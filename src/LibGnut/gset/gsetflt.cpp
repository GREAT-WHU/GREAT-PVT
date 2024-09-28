/**
*
* @verbatim
     (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  @endverbatim
*
* @file       gsetflt.cpp
* @brief      implements filter setting
* @author     Jan Dousa
* @version    1.0.0
* @date       2012-10-23
*
*/

#include <iomanip>
#include <sstream>

#include "gset/gsetflt.h"

using namespace std;
using namespace pugi;

namespace gnut
{
    t_gsetflt::t_gsetflt()
        : t_gsetbase()
    {
        _set.insert(XMLKEY_FLT);
        _noise_clk = 1000.0;
        _noise_crd = 100.0;
        _noise_vel = 100.0;
        _noise_dclk = 1000.0;
        _rndwk_gps = 20.0;
        _rndwk_glo = 20.0;
        _rndwk_gal = 20.0;
        _rndwk_bds = 20.0;
        _rndwk_qzs = 20.0;
        _rndwk_ztd = 3.0;
        _rndwk_vion = 1000;
        _noise_vion = 0.1;
        _rndwk_grd = 0.3;
        _rndwk_amb = 0.1;
        _method_flt = "kalman";
        _reset_amb = 0;
        _reset_par = 0;
    }

    t_gsetflt::~t_gsetflt()
    {
    }

    string t_gsetflt::method_flt()
    {
        _gmutex.lock();

        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("method_flt").value();
        if (tmp.empty())
        {
            tmp = "kalman";
        }
        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::noise_clk()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_clk").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::noise_crd()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_crd").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::noise_dclk()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_dclk").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::noise_vel()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_vel").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::rndwk_gps()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_gps").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::rndwk_glo()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_glo").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::rndwk_gal()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_gal").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::rndwk_bds()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_bds").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::rndwk_amb()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_amb").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::rndwk_qzs()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_qzs").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::rndwk_ztd()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_ztd").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::noise_vion()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_vion").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::rndwk_vion()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_vion").as_double();

        _gmutex.unlock();
        return tmp;
    }

    double t_gsetflt::rndwk_grd()
    {
        _gmutex.lock();

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_grd").as_double();

        _gmutex.unlock();
        return tmp;
    }

    int t_gsetflt::reset_amb()
    {
        _gmutex.lock();

        int tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("reset_amb").as_int();

        _gmutex.unlock();
        return tmp;
    }

    int t_gsetflt::reset_par()
    {
        _gmutex.lock();

        int tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("reset_par").as_int();

        _gmutex.unlock();
        return tmp;
    }

    int t_gsetflt::reset_par(double d)
    {
        _gmutex.lock();
        xml_node flt = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT);
        flt.remove_attribute("reset_par");
        xml_attribute attri = flt.append_attribute("reset_par");
        attri.set_value(to_string(d).c_str());
        _gmutex.unlock();
        return 0;
    }

    void t_gsetflt::check()
    {
        _gmutex.lock();

        xml_node parent = _doc.child(XMLKEY_ROOT);
        xml_node node = _default_node(parent, XMLKEY_FLT);

        _default_attr(node, "method_flt", _method_flt);
        _default_attr(node, "noise_clk", _noise_clk);
        _default_attr(node, "noise_dclk", _noise_dclk);
        _default_attr(node, "noise_crd", _noise_crd);
        _default_attr(node, "noise_vel", _noise_vel);
        _default_attr(node, "rndwk_gps", _rndwk_gps);
        _default_attr(node, "rndwk_glo", _rndwk_glo);
        _default_attr(node, "rndwk_gal", _rndwk_gal);
        _default_attr(node, "rndwk_bds", _rndwk_bds);
        _default_attr(node, "rndwk_qzs", _rndwk_qzs);
        _default_attr(node, "rndwk_ztd", _rndwk_ztd);
        _default_attr(node, "noise_vion", _noise_vion);
        _default_attr(node, "rndwk_grd", _rndwk_grd);
        _default_attr(node, "rndwk_vion", _rndwk_vion);
        _default_attr(node, "reset_amb", _reset_amb);
        _default_attr(node, "reset_par", _reset_par);

        _gmutex.unlock();
        return;
    }

    void t_gsetflt::help()
    {
        _gmutex.lock();

        cerr << " <filter \n"
             << "   method_flt=\"" << _method_flt << "\" \n"
             << "   noise_clk=\"" << _noise_clk << "\" \n"
             << "   noise_crd=\"" << _noise_crd << "\" \n"
             << "   rndwk_ztd=\"" << _rndwk_ztd << "\" \n"
             << "   noise_vion=\"" << _noise_vion << "\" \n"
             << "   rndwk_grd=\"" << _rndwk_grd << "\" \n"
             << "  />\n";

        cerr << "\t<!-- filter description:\n"
             << "\t method_flt    .. type of filtering method (SRCF, Kalman)\n"
             << "\t noise_clk     .. white noise for clocks \n"
             << "\t noise_crd     .. white noise for coordinates \n"
             << "\t rndwk_ztd     .. random walk process for ZTD [mm/sqrt(hour)] \n"
             << "\t noise_vion    .. white noise process for VION [mm/sqrt(hour)] \n"
             << "\t -->\n\n";

        _gmutex.unlock();
        return;
    }

} // namespace
