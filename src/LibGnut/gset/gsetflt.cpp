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

using namespace pugi;

namespace gnut
{
    t_gsetflt::t_gsetflt() :
        t_gsetbase()
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

    std::string t_gsetflt::method_flt()
    {

        std::string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("method_flt").value();
        if (tmp.empty())
        {
            tmp = "kalman";
        }
        return tmp;
    }

    double t_gsetflt::noise_clk()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_clk").as_double();

        return tmp;
    }

    double t_gsetflt::noise_crd()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_crd").as_double();

        return tmp;
    }

    double t_gsetflt::noise_dclk()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_dclk").as_double();

        return tmp;
    }

    double t_gsetflt::noise_vel()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_vel").as_double();

        return tmp;
    }

    double t_gsetflt::rndwk_gps()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_gps").as_double();

        return tmp;
    }

    double t_gsetflt::rndwk_glo()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_glo").as_double();

        return tmp;
    }

    double t_gsetflt::rndwk_gal()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_gal").as_double();

        return tmp;
    }

    double t_gsetflt::rndwk_bds()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_bds").as_double();

        return tmp;
    }

    double t_gsetflt::rndwk_amb()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_amb").as_double();

        return tmp;
    }

    double t_gsetflt::rndwk_qzs()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_qzs").as_double();

        return tmp;
    }

    double t_gsetflt::rndwk_ztd()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_ztd").as_double();

        return tmp;
    }

    double t_gsetflt::noise_vion()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("noise_vion").as_double();

        return tmp;
    }

    double t_gsetflt::rndwk_vion()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_vion").as_double();

        return tmp;
    }

    double t_gsetflt::rndwk_grd()
    {

        double tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("rndwk_grd").as_double();

        return tmp;
    }

    int t_gsetflt::reset_amb()
    {

        int tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("reset_amb").as_int();

        return tmp;
    }

    int t_gsetflt::reset_par()
    {

        int tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT).attribute("reset_par").as_int();

        return tmp;
    }

    int t_gsetflt::reset_par(double d)
    {
        xml_node flt = _doc.child(XMLKEY_ROOT).child(XMLKEY_FLT);
        flt.remove_attribute("reset_par");
        xml_attribute attri = flt.append_attribute("reset_par");
        attri.set_value(std::to_string(d).c_str());
        return 0;
    }

    void t_gsetflt::check()
    {

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

        return;
    }

    void t_gsetflt::help()
    {

        std::cerr << " <filter \n"
             << "   method_flt=\"" << _method_flt << "\" \n"
             << "   noise_clk=\"" << _noise_clk << "\" \n"
             << "   noise_crd=\"" << _noise_crd << "\" \n"
             << "   rndwk_ztd=\"" << _rndwk_ztd << "\" \n"
             << "   noise_vion=\"" << _noise_vion << "\" \n"
             << "   rndwk_grd=\"" << _rndwk_grd << "\" \n"
             << "  />\n";

        std::cerr << "\t<!-- filter description:\n"
             << "\t method_flt    .. type of filtering method (SRCF, Kalman)\n"
             << "\t noise_clk     .. white noise for clocks \n"
             << "\t noise_crd     .. white noise for coordinates \n"
             << "\t rndwk_ztd     .. random walk process for ZTD [mm/sqrt(hour)] \n"
             << "\t noise_vion    .. white noise process for VION [mm/sqrt(hour)] \n"
             << "\t -->\n\n";

        return;
    }

} // namespace gnut
