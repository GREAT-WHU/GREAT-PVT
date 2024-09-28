/*
*
* @verbatim
     (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  @endverbatim
*
* @file        gsetproc.cpp
* @brief       implements process setting class
* @author      Jan Dousa
* @version     1.0.0
* @date        2012-10-23
*
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "gset/gsetproc.h"
#include "gutils/gmutex.h"



using namespace std;
using namespace pugi;

#define XMLKEY_PROC_IFCB "ifcb_model"

namespace gnut
{

    t_gsetproc::t_gsetproc()
        : t_gsetbase()
    {
        _set.insert(XMLKEY_PROC);
        _phase = true;
        _tropo = true;
        _iono = true;
        _tropo_grad = false;
        _tropo_slant = false;
        _tropo_model = TROPMODEL::SAASTAMOINEN;
        _tropo_mf = ZTDMPFUNC::GMF;
        _iono_mf = IONMPFUNC::ICOSZ;
        _grad_mf = GRDMPFUNC::TILTING;
        _obs_weight = OBSWEIGHT::SINEL2;
        _res_type = RESIDTYPE::RES_NORM;
        _obs_combin = OBSCOMBIN::IONO_FREE;
        _attitudes = ATTITUDES::DEF_YAWMODEL;
        _cbiaschar = CBIASCHAR::DEF_CBIASCHAR;

        _sig_init_ztd = 0.1;
        _sig_init_vion = 10;
        _sig_init_grd = 0.0005;
        _sig_init_amb = 1000.0;
        _sig_init_crd = 100.0;
        _sig_init_vel = 10.0;
        _sig_init_glo = 1000.0;
        _sig_init_gal = 1000.0;
        _sig_init_bds = 1000.0;
        _sig_init_qzs = 1000.0;
        _minimum_elev = 10;
        _max_res_norm = 10;
        _crd_est = "EST";
        _pos_kin = false;
        _frequency = 2;
        _sd_sat = false;
        _basepos = BASEPOS::SPP;
        _minsat = static_cast<size_t>(6); 

        _meanpolemodel = modeofmeanpole::cubic;
    }

    modeofmeanpole t_gsetproc::mean_pole_model()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("mean_pole_model");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        modeofmeanpole TM = str2meanpolemodel(tmp);
        _gmutex.unlock();
        return TM;
    }

    modeofmeanpole t_gsetproc::str2meanpolemodel(const string &tm)
    {
        if (tm == "linear" || tm == "LINEAR")
        {
            return modeofmeanpole::linear;
        }
        else if (tm == "cubic" || tm == "CUBIC")
        {
            return modeofmeanpole::cubic;
        }
        else
        {
            return modeofmeanpole::cubic;
        }
    }

    t_gsetproc::~t_gsetproc() = default;

    bool t_gsetproc::tropo()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("tropo");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        bool tmp_bool = (tmp == "FALSE" ? false : true); 
        _gmutex.unlock();
        return tmp_bool;
    }

    bool t_gsetproc::iono()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("iono");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        bool tmp_bool = tmp == "FALSE" ? false : true; 
        _gmutex.unlock();
        return tmp_bool;
    }

    bool t_gsetproc::tropo_slant()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("tropo_slant");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        bool tmp_bool = tmp == "TRUE" ? true : false; 
        _gmutex.unlock();
        return tmp_bool;
    }

    bool t_gsetproc::tropo_grad()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("gradient");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        bool tmp_bool = (tmp == "TRUE" ? true : false); 
        _gmutex.unlock();
        return tmp_bool;
    }

    bool t_gsetproc::phase()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("phase");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        bool tmp_bool = (tmp == "TRUE" ? true : false); 
        _gmutex.unlock();
        return tmp_bool;
    }

    bool t_gsetproc::doppler()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("doppler");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        bool tmp_bool = (tmp == "TRUE" ? true : false); 
        _gmutex.unlock();
        return tmp_bool;
    }

    bool t_gsetproc::pos_kin()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("pos_kin");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        bool tmp_bool = (tmp == "TRUE" ? true : false); 
        _gmutex.unlock();
        return tmp_bool;
    }

    BASEPOS t_gsetproc::basepos()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("basepos");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        BASEPOS type = str2basepos(tmp); 
        _gmutex.unlock();
        return type;
    }

    BASEPOS t_gsetproc::str2basepos(const string &str)
    {
        if (str == "CFILE")
            return BASEPOS::CFILE;
        else
            return BASEPOS::SPP; 
    }

    double t_gsetproc::sig_init_ztd()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_ztd");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_ztd; 
        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::sig_init_vion()
    {
        _gmutex.lock();

        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_vion");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_vion; 

        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::sig_init_grd()
    {
        _gmutex.lock();

        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_grd");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_grd; 

        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::sig_init_crd()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_crd");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_crd; 
        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::sig_init_vel()
    {
        _gmutex.lock();

        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_vel");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_vel; 

        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::sig_init_amb()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_amb");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_amb; 
        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::sig_init_glo()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_glo");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_glo; 
        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::sig_init_gal()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_gal");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_gal; 
        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::sig_init_bds()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_bds");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_bds; 
        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::sig_init_qzs()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("sig_init_qzs");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _sig_init_qzs; 
        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::minimum_elev()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("minimum_elev");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _minimum_elev; 
        _gmutex.unlock();
        return tmp_double;
    }

    double t_gsetproc::max_res_norm()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("max_res_norm");
        str_erase(tmp);
        double tmp_double;
        if (tmp != "")
            tmp_double = std::stod(tmp);
        else
            tmp_double = _max_res_norm; 
        _gmutex.unlock();
        return tmp_double;
    }

    int t_gsetproc::minsat()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("min_sat");
        str_erase(tmp);
        int tmp_int;
        if (tmp != "")
            tmp_int = std::stoi(tmp);
        else
            tmp_int = _minsat; 
        _gmutex.unlock();
        return tmp_int;
    }

    string t_gsetproc::ref_clk()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("ref_clk");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        _gmutex.unlock();
        return tmp;
    }

    SLIPMODEL t_gsetproc::slip_model()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("slip_model");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        _gmutex.unlock();
        if (tmp == "default")
            return SLIPMODEL::DEF_DETECT_MODEL;
        else
            return SLIPMODEL::DEF_DETECT_MODEL;  
    } 

    IONMODEL t_gsetproc::ion_model()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("ion_model");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        _gmutex.unlock();
        if (tmp == "SION")
            return IONMODEL::SION;
        else if (tmp == "VION")
            return IONMODEL::VION;
        else
            return IONMODEL::DEF_ION; 
    }

    TROPMODEL t_gsetproc::tropo_model()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("tropo_model");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        TROPMODEL TM = str2tropmodel(tmp);
        if (TM == TROPMODEL::DEF_TROPMODEL)
        {
            xml_node parent = _doc.child(XMLKEY_ROOT);
            xml_node node = _default_node(parent, XMLKEY_PROC);
            tmp = tropmodel2str(_tropo_model);
            _default_node(node, "tropo_model", tmp.c_str());
            TM = _tropo_model; 
        }
        _gmutex.unlock();
        return TM;
    }

    ATTITUDES t_gsetproc::attitudes()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("attitudes");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        _gmutex.unlock();
        return str2attitudes(tmp); 
    }

    CBIASCHAR t_gsetproc::cbiaschar()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("cbiaschar");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        _gmutex.unlock();
        return str2cbiaschar(tmp); 
    }

    GRDMPFUNC t_gsetproc::grad_mf()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("grad_mf");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);

        GRDMPFUNC MF = str2grdmpfunc(tmp);
        if (MF == GRDMPFUNC::DEF_GRDMPFUNC)
        {
            xml_node parent = _doc.child(XMLKEY_ROOT);
            xml_node node = _default_node(parent, XMLKEY_PROC);
            tmp = grdmpfunc2str(_grad_mf);
            _default_node(node, "grad_mf", tmp.c_str());

            MF = _grad_mf; 
        }

        _gmutex.unlock();
        return MF;
    }

    OBSWEIGHT t_gsetproc::weighting()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("obs_weight");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);

        OBSWEIGHT WG = str2obsweight(tmp);
        if (WG == OBSWEIGHT::DEF_OBSWEIGHT)
        {
            xml_node parent = _doc.child(XMLKEY_ROOT);
            xml_node node = _default_node(parent, XMLKEY_PROC);
            tmp = obsweight2str(_obs_weight);
            _default_node(node, "obs_weight", tmp.c_str());
            WG = _obs_weight; 
        }

        _gmutex.unlock();
        return WG;
    }

    RESIDTYPE t_gsetproc::residuals()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("residuals");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);

        RESIDTYPE RS = str2residtype(tmp);
        if (RS == RESIDTYPE::DEF_RESIDTYPE)
        {
            xml_node parent = _doc.child(XMLKEY_ROOT);
            xml_node node = _default_node(parent, XMLKEY_PROC);
            tmp = residtype2str(_res_type);
            _default_node(node, "residuals", tmp.c_str());
            RS = _res_type; 
        }

        _gmutex.unlock();
        return RS;
    }

    OBSCOMBIN t_gsetproc::obs_combin()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("obs_combination");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);

        OBSCOMBIN OC = str2obscombin(tmp);
        if (OC == OBSCOMBIN::DEF_OBSCOMBIN)
        {
            xml_node parent = _doc.child(XMLKEY_ROOT);
            xml_node node = _default_node(parent, XMLKEY_PROC);
            tmp = obscombin2str(_obs_combin);
            _default_node(node, "obs_combination", tmp.c_str());
            OC = _obs_combin; 
        }

        _gmutex.unlock();
        return OC;
    }

    ZTDMPFUNC t_gsetproc::tropo_mf()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("tropo_mf");
        str_erase(tmp);
        _gmutex.unlock();
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);

        ZTDMPFUNC MF = str2ztdmpfunc(tmp);
        if (MF == ZTDMPFUNC::DEF_ZTDMPFUNC)
        {
            xml_node parent = _doc.child(XMLKEY_ROOT);
            xml_node node = _default_node(parent, XMLKEY_PROC);
            tmp = ztdmpfunc2str(_tropo_mf);
            _default_node(node, "tropo_mf", tmp.c_str());
            MF = _tropo_mf; 
        }

        return MF;
    }

    CONSTRPAR t_gsetproc::crd_est()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("crd_constr");
        str_erase(tmp);
        _gmutex.unlock();

        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        if (tmp.empty())
            tmp = _crd_est; 

        if (tmp == "EST")
            return CONSTRPAR::EST;
        else if (tmp == "KIN")
            return CONSTRPAR::KIN;
        else
            return CONSTRPAR::FIX;
    }

    void t_gsetproc::check()
    {
        _gmutex.lock();

        xml_node parent = _doc.child(XMLKEY_ROOT);
        xml_node node = _default_node(parent, XMLKEY_PROC);
        _default_node(node, "phase", _phase == true ? "true" : "false");
        _default_node(node, "tropo", _tropo == true ? "true" : "false");
        _default_node(node, "iono", _iono == true ? "true" : "false");
        _default_node(node, "gradient", _tropo_grad == true ? "true" : "false");
        _default_node(node, "tropo_model", (tropmodel2str(_tropo_model)).c_str());
        _default_node(node, "tropo_slant", _tropo_slant == true ? "true" : "false");
        _default_node(node, "sig_init_crd", (to_string(_sig_init_crd)).c_str());
        _default_node(node, "sig_init_vel", (to_string(_sig_init_vel)).c_str());
        _default_node(node, "sig_init_ztd", (to_string(_sig_init_ztd)).c_str());
        _default_node(node, "sig_init_grd", (to_string(_sig_init_grd)).c_str());
        _default_node(node, "sig_init_vion", (to_string(_sig_init_vion)).c_str());
        _default_node(node, "sig_init_amb", (to_string(_sig_init_amb)).c_str());
        _default_node(node, "sig_init_glo", (to_string(_sig_init_glo)).c_str());
        _default_node(node, "sig_init_gal", (to_string(_sig_init_gal)).c_str());
        _default_node(node, "sig_init_bds", (to_string(_sig_init_bds)).c_str());
        _default_node(node, "sig_init_qzs", (to_string(_sig_init_qzs)).c_str());
        _default_node(node, "minimum_elev", (to_string(_minimum_elev)).c_str());
        _default_node(node, "max_res_norm", (to_string(_max_res_norm)).c_str());
        _default_node(node, "crd_constr", _crd_est.c_str());
        _default_node(node, "pos_kin", _pos_kin == true ? "true" : "false");
        _default_node(node, "sd_sat", _sd_sat == true ? "true" : "false");
        _default_node(node, "min_sat", (to_string(_minsat)).c_str());
        _default_node(node, "basepos", basepos2str(_basepos).c_str());

        _gmutex.unlock();
    }

    void t_gsetproc::help()
    {
        _gmutex.lock();

        cerr << " <process \n"
             << "   phase=\"" << _phase << "\" \n"
             << "   tropo=\"" << _tropo << "\" \n"
             << "   iono=\"" << _iono << "\" \n"
             << "   tropo_grad=\"" << _tropo_grad << "\" \n"
             << "   tropo_model=\"" << tropmodel2str(_tropo_model) << "\" \n"
             << "   tropo_slant=\"" << _tropo_slant << "\" \n"
             << "   tropo_mf=\"" << ztdmpfunc2str(_tropo_mf) << "\" \n"
             << "   iono_mf=\"" << ionmpfunc2str(_iono_mf) << "\" \n"
             << "   grad_mf=\"" << grdmpfunc2str(_grad_mf) << "\" \n"
             << "   obs_weight=\"" << obsweight2str(_obs_weight) << "\" \n"
             << "   residuals=\"" << residtype2str(_res_type) << "\" \n"
             << "   obs_combination=\"" << obscombin2str(_obs_combin) << "\" \n"
             << "   sig_init_crd=\"" << _sig_init_crd << "\" \n"
             << "   sig_init_ztd=\"" << _sig_init_ztd << "\" \n"
             << "   sig_init_vion=\"" << _sig_init_vion << "\" \n"
             << "   minimum_elev=\"" << _minimum_elev << "\" \n"
             << "   max_res_norm=\"" << _max_res_norm << "\" \n"
             << "   basepos=\"" << basepos2str(_basepos) << "\" \n"
             << " />\n";

        cerr << "\t<!-- process description:\n"
             << "\t phase [0,1]   .. use carrier phase data\n"
             << "\t tropo [0,1]   .. estimate troposphere\n"
             << "\t tropo_grad    .. tropospheric horizontal gradient models\n"
             << "\t tropo_model   .. tropospheric model (SAAS, GPT, ...)\n"
             << "\t tropo_slant   .. tropospheric slant delays produced\n"
             << "\t tropo_mf      .. tropospheric mapping function (COSZ, GMF, ...)\n"
             << "\t grad_mf       .. tropo gradient mapping function (TILTING,CHEN_HERRING, BAR_SEVER ...)\n"
             << "\t obs_weight    .. observation elevation dependant weighting (EQUAL, SINEL, SINEL2, SINEL4, CODPHA, MLTPTH)\n"
             << "\t residuals     .. type of residuals (RES_ORIG, RES_NORM, RES_ALL)\n"
             << "\t sig_init_crd  .. accuracy of initial coordinates [m]\n"
             << "\t sig_init_ztd  .. accuracy of initial zenith path delay [m]\n"
             << "\t sig_init_vion .. accuracy of initial vertical iono path delay [m]\n"
             << "\t minimum_elev  .. elevation angle cut-off [degree]\n"
             << "\t max_res_norm  .. maximal normalized residuals\n"
             << "\t basepos  .. base site coordinate\n"
             << "\t -->\n\n";

        _gmutex.unlock();
    }

    ATTITUDES t_gsetproc::str2attitudes(const string &ati)
    {
        if (ati == "NOMINAL")
        {
            return ATTITUDES::YAW_NOMI;
        }
        else if (ati == "RTCM")
        {
            return ATTITUDES::YAW_RTCM;
        }
        else
        {
            stringstream ostr;
            ostr << "Unsupported attitude model (" << attitude2str(_attitudes) << ")! Used default value ( YAW_MODEL )";
            _add_log("gsetproc", ostr.str());
            return ATTITUDES::DEF_YAWMODEL;
        }
    }

    GRDMPFUNC t_gsetproc::str2grdmpfunc(const string &mf)
    {
        if (mf == "TILTING")
        {
            return GRDMPFUNC::TILTING;
        }
        else if (mf == "CHEN_HERRING")
        {
            return GRDMPFUNC::CHEN_HERRING;
        }
        else if (mf == "BAR_SEVER")
        {
            return GRDMPFUNC::BAR_SEVER;
        }
        else
        {
            stringstream ostr;
            ostr << "Unsupported GRD mapping function (" << grdmpfunc2str(_grad_mf) << ")! Used default value ("
                 << grdmpfunc2str(_grad_mf) << ")";
            _add_log("gsetproc", ostr.str());
            return GRDMPFUNC::DEF_GRDMPFUNC;
        }
    }

    CBIASCHAR t_gsetproc::str2cbiaschar(const string &cb)
    {
        if (cb == "2CHAR")
        {
            return CBIASCHAR::CHAR2;
        }
        else if (cb == "3CHAR")
        {
            return CBIASCHAR::CHAR3;
        }
        else if (cb == "ORIG")
        {
            return CBIASCHAR::ORIG;
        }
        else
        {
            stringstream ostr;
            ostr << "Unsupported forcing code bias signals! Used default value (" << cbiaschar2str(_cbiaschar) << ")";
            _add_log("gsetproc", ostr.str());
            return CBIASCHAR::DEF_CBIASCHAR;
        }
    }

    ZTDMPFUNC t_gsetproc::str2ztdmpfunc(const string &mf)
    {
        if (mf == "COSZ")
        {
            return ZTDMPFUNC::COSZ;
        }
        else if (mf == "GMF")
        {
            return ZTDMPFUNC::GMF;
        }
        else if (mf == "NO_MF")
        {
            return ZTDMPFUNC::NO_MF;
        }
        else
        {
            stringstream ostr;
            ostr << "Unsupported ZTD mapping function (" << mf << ")! Used default value (" << ztdmpfunc2str(_tropo_mf) << ")";
            _add_log("gsetproc", ostr.str());
            return ZTDMPFUNC::DEF_ZTDMPFUNC;
        }
    }

    IONMPFUNC t_gsetproc::str2ionmpfunc(const string &mf)
    {
        if (mf == "COSZ")
        {
            return IONMPFUNC::ICOSZ;
        }
        else if (mf == "QFAC")
        {
            return IONMPFUNC::QFAC;
        }
        else if (mf == "NONE")
        {
            return IONMPFUNC::NONE;
        }
        else
        {
            stringstream ostr;
            ostr << "Unsupported ION mapping function (" << mf << ")! Used default value (" << ionmpfunc2str(_iono_mf) << ")";
            _add_log("gsetproc", ostr.str());
            return IONMPFUNC::DEF_IONMPFUNC;
        }
    }

    OBSWEIGHT t_gsetproc::str2obsweight(const string &wg)
    {
        if (wg == "EQUAL")
        {
            return OBSWEIGHT::EQUAL;
        }
        else if (wg == "SINEL" || wg == "SINEL1")
        {
            return OBSWEIGHT::SINEL;
        }
        else if (wg == "SINEL2")
        {
            return OBSWEIGHT::SINEL2;
        }
        else if (wg == "SINEL4")
        {
            return OBSWEIGHT::SINEL4;
        }
        else if (wg == "PARTELE")
        {
            return OBSWEIGHT::PARTELE;
        }
        else if (wg == "SNR")
        {
            return OBSWEIGHT::SNR;
        }
        else
        {
            stringstream ostr;
            ostr << "Unsupported observation weighting model (" << wg << ")! Used default value (" << obsweight2str(_obs_weight)
                 << ")";
            _add_log("gsetproc", ostr.str());
            return OBSWEIGHT::DEF_OBSWEIGHT;
        }
    }

    TROPMODEL t_gsetproc::str2tropmodel(const string &tm)
    {
        if (tm == "SAASTAMOINEN")
        {
            return TROPMODEL::SAASTAMOINEN;
        }
        else if (tm == "DAVIS")
        {
            return TROPMODEL::DAVIS;
        }
        else if (tm == "HOPFIELD")
        {
            return TROPMODEL::HOPFIELD;
        }
        else if (tm == "EXTERN")
        {
            return TROPMODEL::EXTERN;
        }
        else
        {
            stringstream ostr;
            ostr << "Unsupported tropospheric model (" << tm << ")! Used default value (" << tropmodel2str(_tropo_model) << ")";
            _add_log("gsetproc", ostr.str());
            return TROPMODEL::DEF_TROPMODEL;
        }
    }

    RESIDTYPE t_gsetproc::str2residtype(const string &rs)
    {
        if (rs == "RES_ORIG")
        {
            return RESIDTYPE::RES_ORIG;
        }
        else if (rs == "RES_NORM")
        {
            return RESIDTYPE::RES_NORM;
        }
        else if (rs == "RES_ALL")
        {
            return RESIDTYPE::RES_ALL;
        }
        else
        {
            stringstream ostr;
            ostr << "Unsupported type of residuals (" << rs << ")! Used default value (" << residtype2str(_res_type) << ")";
            _add_log("gsetproc", ostr.str());
            return RESIDTYPE::DEF_RESIDTYPE;
        }
    }

    OBSCOMBIN t_gsetproc::str2obscombin(const string &oc)
    {
        if (oc == "IONO_FREE")
        {
            return OBSCOMBIN::IONO_FREE;
        }
        else if (oc == "RAW_ALL")
        {
            return OBSCOMBIN::RAW_ALL;
        }
        else if (oc == "RAW_MIX")
        {
            return OBSCOMBIN::RAW_MIX;
        }
        else
        {
            stringstream ostr;
            ostr << "Unsupported observations combination (" << oc << ")! Used default value (" << obscombin2str(_obs_combin)
                 << ")";
            _add_log("gsetproc", ostr.str());
            return OBSCOMBIN::DEF_OBSCOMBIN;
        }
    }

    string t_gsetproc::grdmpfunc2str(GRDMPFUNC MF)
    {
        switch (MF)
        {
        case GRDMPFUNC::TILTING:
            return "TILTING";
        case GRDMPFUNC::CHEN_HERRING:
            return "CHEN_HERRING";
        case GRDMPFUNC::BAR_SEVER:
            return "BAR_SEVER";
        case GRDMPFUNC::DEF_GRDMPFUNC:
            return "NOT DEFINED";
        default:
            return "";
        }
    }

    string t_gsetproc::ztdmpfunc2str(ZTDMPFUNC MF)
    {
        switch (MF)
        {
        case ZTDMPFUNC::COSZ:
            return "COSZ";
        case ZTDMPFUNC::GMF:
            return "GMF";
        case ZTDMPFUNC::NO_MF:
            return "NO_MF";
        case ZTDMPFUNC::DEF_ZTDMPFUNC:
            return "NOT DEFINED";
        default:
            return "";
        }
    }

    string t_gsetproc::ionmpfunc2str(IONMPFUNC MF)
    {
        switch (MF)
        {
        case IONMPFUNC::ICOSZ:
            return "ICOSZ";
        case IONMPFUNC::QFAC:
            return "QFAC";
        case IONMPFUNC::NONE:
            return "NONE";
        case IONMPFUNC::DEF_IONMPFUNC:
            return "NOT DEFINED";
        default:
            return "";
        }
    }

    string t_gsetproc::obsweight2str(OBSWEIGHT WG)
    {
        switch (WG)
        {
        case OBSWEIGHT::EQUAL:
            return "EQUAL";
        case OBSWEIGHT::SINEL:
            return "SINEL";
        case OBSWEIGHT::SINEL2:
            return "SINEL2";
        case OBSWEIGHT::SINEL4:
            return "SINEL4";
        case OBSWEIGHT::PARTELE:
            return "PARTELE";
        case OBSWEIGHT::SNR:
            return "SNR";
        case OBSWEIGHT::DEF_OBSWEIGHT:
            return "NOT DEFINED";
        default:
            return "";
        }
    }

    string t_gsetproc::tropmodel2str(TROPMODEL TM)
    {
        switch (TM)
        {
        case TROPMODEL::SAASTAMOINEN:
            return "SAASTAMOINEN";
        case TROPMODEL::DAVIS:
            return "DAVIS";
        case TROPMODEL::HOPFIELD:
            return "HOPFIELD";
        case TROPMODEL::EXTERN:
            return "EXTERN";
        case TROPMODEL::DEF_TROPMODEL:
            return "NOT DEFINED";
        default:
            return "";
        }
    }

    string t_gsetproc::residtype2str(RESIDTYPE RS)
    {
        switch (RS)
        {
        case RESIDTYPE::RES_ORIG:
            return "RES_ORIG";
        case RESIDTYPE::RES_NORM:
            return "RES_NORM";
        case RESIDTYPE::RES_ALL:
            return "RES_ALL";
        case RESIDTYPE::DEF_RESIDTYPE:
            return "NOT DEFINED";
        default:
            return "";
        }
    }

    string t_gsetproc::attitude2str(ATTITUDES AT)
    {
        switch (AT)
        {
        case ATTITUDES::YAW_NOMI:
            return "YAW_NOMI";
        case ATTITUDES::YAW_RTCM:
            return "YAW_RTCM";
        case ATTITUDES::DEF_YAWMODEL:
            return "DEF_YAWMODEL";
        default:
            return "";
        }
    }

    string t_gsetproc::obscombin2str(OBSCOMBIN OC)
    {
        switch (OC)
        {
        case OBSCOMBIN::IONO_FREE:
            return "IONO_FREE";
        case OBSCOMBIN::RAW_ALL:
            return "RAW_ALL";
        case OBSCOMBIN::DEF_OBSCOMBIN:
            return "NOT DEFINED";
        case OBSCOMBIN::RAW_MIX:
            return "RAW_MIX";
        default:
            return "";
        }
    }

    string t_gsetproc::cbiaschar2str(CBIASCHAR CB)
    {
        switch (CB)
        {
        case CBIASCHAR::CHAR2:
            return "2CHAR";
        case CBIASCHAR::CHAR3:
            return "3CHAR";
        case CBIASCHAR::ORIG:
            return "ORIG";
        case CBIASCHAR::DEF_CBIASCHAR:
            return "NOT DEFINED";
        default:
            return "";
        }
    }

    string t_gsetproc::basepos2str(BASEPOS BP)
    {
        switch (BP)
        {
        case BASEPOS::CFILE:
            return "CFILE";
        case BASEPOS::SPP:
            return "SPP";
        default:
            return "";
        }
    }

    t_gobscombtype::t_gobscombtype() : _obs_type(GOBSTYPE::TYPE),
                                       _obs_band_1(GOBSBAND::BAND),
                                       _obs_combine(OBSCOMBIN::DEF_OBSCOMBIN)
    {
    }

    t_gobscombtype::t_gobscombtype(const t_gobscombtype &other) : _obs_type(other._obs_type),
                                                                  _obs_band_1(other._obs_band_1),
                                                                  _obs_band_2(other._obs_band_2),
                                                                  _obs_freq_1(other._obs_freq_1),
                                                                  _obs_freq_2(other._obs_freq_2),
                                                                  _obs_combine(other._obs_combine)
    {
    }

    t_gobscombtype::t_gobscombtype(const string &obscombtype) : _obs_type(GOBSTYPE::TYPE),
                                                                _obs_band_1(GOBSBAND::BAND),
                                                                _obs_combine(OBSCOMBIN::DEF_OBSCOMBIN)
    {
        if (obscombtype.length() < 2)
        {
            return;
        }
        string str_type = obscombtype.substr(0, 1);
        if (str_type == "P")
            str_type = "C";
        _obs_type = str2gobstype(str_type);

        string str_band = obscombtype.substr(1);
        if (str_band.substr(0, 1) == "C")
        {
            _obs_combine = OBSCOMBIN::IONO_FREE;
            _obs_freq_1 = str2gnssfreq(obscombtype.substr(2, 1));
            _obs_freq_2 = str2gnssfreq(obscombtype.substr(3, 1));
        }
        else
        {
            _obs_freq_1 = str2gnssfreq(obscombtype.substr(1, 1));
        }
    }

    t_gobscombtype::t_gobscombtype(const t_gobs &obstype, OBSCOMBIN combtype) : _obs_type(obstype.type()),
                                                                                _obs_band_1(obstype.band()),
                                                                                _obs_combine(combtype)
    {
    }

    t_gobscombtype::t_gobscombtype(const t_gobs &obstype, GOBSBAND b1, FREQ_SEQ freq_1, OBSCOMBIN combtype) : _obs_type(obstype.type()),
                                                                                                              _obs_band_1(b1),
                                                                                                              _obs_freq_1(freq_1),
                                                                                                              _obs_combine(combtype)
    {
    }

    t_gobscombtype::t_gobscombtype(const t_gobs &obstype,
                                   GOBSBAND b1,
                                   GOBSBAND b2,
                                   FREQ_SEQ freq_1,
                                   FREQ_SEQ freq_2,
                                   OBSCOMBIN combtype) : _obs_type(obstype.type()),
                                                         _obs_band_1(b1),
                                                         _obs_band_2(b2),
                                                         _obs_freq_1(freq_1),
                                                         _obs_freq_2(freq_2),
                                                         _obs_combine(combtype)
    {
    }

    t_gobscombtype::t_gobscombtype(GOBSTYPE t, GOBSBAND b, OBSCOMBIN obscomb) : _obs_type(t),
                                                                                _obs_band_1(b),
                                                                                _obs_combine(obscomb)
    {
    }

    string t_gobscombtype::convert2str() const
    {
        string ans(gobstype2str(_obs_type == TYPE_C ? TYPE_P : _obs_type));
        if (_obs_combine == OBSCOMBIN::IONO_FREE)
        {
            ans += "C";
            if (_obs_freq_1 != FREQ_X)
                ans += gfreqseq2str(_obs_freq_1);
            if (_obs_freq_2 != FREQ_X)
                ans += gfreqseq2str(_obs_freq_2);
        }
        else
        {
            if (_obs_freq_1 != FREQ_X)
                ans += gfreqseq2str(_obs_freq_1);
        }
        return ans;
    }

    bool t_gobscombtype::operator==(const t_gobscombtype &g) const
    {
        return this->convert2str() == g.convert2str();
    }

    bool t_gobscombtype::operator<(const t_gobscombtype &g) const
    {
        return this->convert2str() < g.convert2str();
    }

    bool t_gobscombtype::is_code() const
    {
        return (_obs_type == TYPE_C || _obs_type == TYPE_P);
    }

    IFCB_MODEL t_gsetproc::ifcb_model()
    {
        //get rkf model node
        string ifcb = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value(XMLKEY_PROC_IFCB);

        if (ifcb.empty())
            return IFCB_MODEL::COR; 

        str_erase(ifcb);
        transform(ifcb.begin(), ifcb.end(), ifcb.begin(), ::toupper);

        if (ifcb == "EST")
            return IFCB_MODEL::EST;
        if (ifcb == "COR")
            return IFCB_MODEL::COR;

        return IFCB_MODEL::DEF;
    }

    bool t_gsetproc::bds_code_bias_correction()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("bds_code_bias_corr");
        str_erase(tmp);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        bool tmp_bool = (tmp == "TRUE" ? true : false); 
        _gmutex.unlock();
        return tmp_bool;
    }

    int t_gsetproc::frequency()
    {
        _gmutex.lock();
        string tmp = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("frequency");
        str_erase(tmp);
        int tmp_int = 2; 
        if (tmp != "")
            tmp_int = std::stoi(tmp);
        _gmutex.unlock();
        return tmp_int;
    }

    RECEIVERTYPE t_gsetproc::get_receiverType()
    {
        _gmutex.lock();

        string recType = _doc.child(XMLKEY_ROOT).child(XMLKEY_PROC).child_value("receiverType");
        str_erase(recType);
        transform(recType.begin(), recType.end(), recType.begin(), ::toupper);

        if (recType.empty() || recType == "DEF")
        {
            _gmutex.unlock();
            return RECEIVERTYPE::DEF;
        }

        if (recType == "AND") 
        {
            _gmutex.unlock();
            return RECEIVERTYPE::And;
        }
        else if (recType == "F9P")
        {
            _gmutex.unlock();
            return RECEIVERTYPE::F9P;
        }
        else
        {
            _gmutex.unlock();
            return RECEIVERTYPE::DEF;
        }
    }

} // namespace
