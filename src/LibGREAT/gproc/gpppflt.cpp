/**
 * @file         gpppflt.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Matrix for NEQ,W,observ_equations
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include <iostream>
#include <sstream>
#include <chrono>
#include "gproc/gpppflt.h"
#include "gproc/gpreproc.h"
#include "gmodels/gpppmodel.h"
#include "gutils/gmatrixconv.h"
#include "gutils/gtimesync.h"
#include "gall/gallprec.h"

using namespace std;

namespace gnut
{
    t_gpppflt::t_gpppflt(string mark, t_gsetbase *gset)
        : t_gspp(mark, gset),
        t_gppp(mark, gset),
        t_gsppflt(mark, gset),
        t_gxml("kml"),
        _read(false),
        _flt(0),
        _kml(false),
        _beg_end(true)
    {
        t_gppp::_get_settings();

        this->_setOut(); 

        _epoch.tsys(t_gtime::GPS);

        _grdStoModel = new t_randomwalk();
        _ambStoModel = new t_randomwalk();
        _grdStoModel->setq(dynamic_cast<t_gsetflt *>(_set)->rndwk_grd());
        _ambStoModel->setq(dynamic_cast<t_gsetflt *>(_set)->rndwk_amb());

        int npar = _param.parNumber();

        if (_tropo_grad)
        {
            t_gpar par_grdN(_site, par_type::GRD_N, ++npar, "");
            t_gpar par_grdE(_site, par_type::GRD_E, ++npar, "");
            par_grdN.setMF(_grd_mf);
            par_grdN.setMF(_ztd_mf);
            par_grdE.setMF(_grd_mf);
            par_grdE.setMF(_ztd_mf);
            _param.addParam(par_grdN);
            _param.addParam(par_grdE);
        }
    }

    t_gpppflt::t_gpppflt(string mark, t_gsetbase *gset, t_spdlog spdlog)
        : t_gspp(mark, gset, spdlog),
        t_gppp(mark, gset, spdlog),
        t_gsppflt(mark, gset, spdlog),
        t_gxml("kml"),
        _read(false),
        _flt(0),
        _kml(false),
        _beg_end(true)
    {
        this->_setOut(); 
        _epoch.tsys(t_gtime::GPS);
        _grdStoModel = new t_randomwalk();
        _ambStoModel = new t_randomwalk();
        _grdStoModel->setq(dynamic_cast<t_gsetflt *>(_set)->rndwk_grd());
        _ambStoModel->setq(dynamic_cast<t_gsetflt *>(_set)->rndwk_amb());
        int npar = _param.parNumber();

        if (_tropo_grad)
        {
            t_gpar par_grdN(_site, par_type::GRD_N, ++npar, "");
            t_gpar par_grdE(_site, par_type::GRD_E, ++npar, "");
            par_grdN.setMF(_grd_mf);
            par_grdN.setMF(_ztd_mf);
            par_grdE.setMF(_grd_mf);
            par_grdE.setMF(_ztd_mf);
            _param.addParam(par_grdN);
            _param.addParam(par_grdE);
        }

        npar = _param.parNumber();

        SymmetricMatrix tmp = _Qx;
        _Qx.ReSize(_param.parNumber());
        _Qx = 0.0;

        for (int i = 1; i <= tmp.Nrows(); i++)
            _Qx(i, i) = tmp(i, i);

        for (size_t i = 1; i <= _param.parNumber(); i++)
        {
            if (_param[i - 1].parType == par_type::GRD_N)
                _Qx(i, i) = _sig_init_grd * _sig_init_grd;
            else if (_param[i - 1].parType == par_type::GRD_E)
                _Qx(i, i) = _sig_init_grd * _sig_init_grd;
            else if (_param[i - 1].parType == par_type::P1P2G_REC)
                _Qx(i, i) = 100.0 * 100;
            else if (_param[i - 1].parType == par_type::P1P2E_REC)
                _Qx(i, i) = 100.0 * 100;
        }

        _reset_amb = dynamic_cast<t_gsetflt *>(_set)->reset_amb();
        _reset_par = dynamic_cast<t_gsetflt *>(_set)->reset_par();

    }

    t_gpppflt::~t_gpppflt()
    {

        if (_grdStoModel)
            delete _grdStoModel;

        if (_flt)
        {
            if (_flt->is_open())
            {
                _flt->close();
            };
            delete _flt;
        }

        this->write(_kml_name);
    }

    void t_gpppflt::_timeUpdate(const t_gtime &epo)
    {
        t_gsppflt::_timeUpdate(epo);

        _grdStoModel->updateTime(epo);
        _grdStoModel->setTcurr(epo);

        _ambStoModel->updateTime(epo);
        _ambStoModel->setTcurr(epo);
    }

    int t_gpppflt::_apply_tides(t_gtime &_epoch, t_gtriple &xRec)
    {
        if (_tides)
        {
            xRec = xRec + _tides->tide_searth(_epoch, xRec) + _tides->load_ocean(_epoch, _site, xRec) + _tides->tide_pole() + _tides->load_atmosph();
        }
        else
            return -1;
        return 1;
    }

    int t_gpppflt::_satPos(t_gtime &epo, t_gsatdata &gsatdata)
    {
        if (t_gsppflt::_satPos(epo, gsatdata) < 0)
            return -1;
        return 1;
    }

    void t_gpppflt::_saveApr(t_gtime &epoch, t_gallpar &X, const SymmetricMatrix &Q)
    {
        // get CRD params
        t_gtriple xyz;
        X.getCrdParam(_site, xyz);

        // CRD using eccentricities
        t_gtriple xyz_ecc = xyz - _grec->eccxyz(epoch); 

        // get CRD rms  (XYZ)
        double Xrms = 0.0, Yrms = 0.0, Zrms = 0.0;
        int icrdx = _param.getParam(_site, par_type::CRD_X, "");
        int icrdy = _param.getParam(_site, par_type::CRD_Y, "");
        int icrdz = _param.getParam(_site, par_type::CRD_Z, "");
        if (Q(icrdx + 1, icrdx + 1) < 0)
            Xrms = -1;
        else
            Xrms = sqrt(Q(icrdx + 1, icrdx + 1));
        if (Q(icrdy + 1, icrdy + 1) < 0)
            Yrms = -1;
        else
            Yrms = sqrt(Q(icrdy + 1, icrdy + 1));
        if (Q(icrdz + 1, icrdz + 1) < 0)
            Zrms = -1;
        else
            Zrms = sqrt(Q(icrdz + 1, icrdz + 1));
        t_gtriple crd_rms(Xrms, Yrms, Zrms);

        if (_allprod != 0)
        {
            shared_ptr<t_gprod> prdcrd = _allprod->get(_site, t_gdata::POS, epoch);
            if (prdcrd)
            {
                dynamic_pointer_cast<t_gprodcrd>(prdcrd)->apr(xyz_ecc);
                dynamic_pointer_cast<t_gprodcrd>(prdcrd)->apr_rms(crd_rms);
            }
            else
            {
                prdcrd = make_shared<t_gprodcrd>(_spdlog, epoch);
                dynamic_pointer_cast<t_gprodcrd>(prdcrd)->apr(xyz_ecc);
                dynamic_pointer_cast<t_gprodcrd>(prdcrd)->apr_rms(crd_rms);
                _allprod->add(prdcrd, _site);
            }
        }
    }

    void t_gpppflt::_removeApr(t_gtime &epoch)
    {

        if (_allprod != 0)
        {
            _allprod->rem(_site, t_gdata::POS, epoch);
        }
    }

    void t_gpppflt::_setOut()
    {
        string tmp;
        tmp = dynamic_cast<t_gsetout *>(_set)->outputs("flt");
        if (!tmp.empty() && !_read)
        {
            substitute(tmp, "$(rec)", _site, false);
            _flt = new t_giof;
            _flt->tsys(t_gtime::GPS);
            _flt->mask(tmp);
            _flt->append(dynamic_cast<t_gsetout *>(_set)->append());
        }

        tmp = dynamic_cast<t_gsetout *>(_set)->outputs("kml");
        if (!tmp.empty() && !_read)
        {
            substitute(tmp, "$(rec)", _site, false);
            _kml_name = tmp;
            _kml = true;

            xml_node node, document, placemark, multigeometry;
            xml_node root = _doc;
            node = this->_default_node(root, _root.c_str());

            document = this->_default_node(node, "Document");

            string mode;
            string smooth = dynamic_cast<t_gsetout *>(_set)->outputs("smt");
            if (!smooth.empty())
                mode = "_SMT";
            else
                mode = "_FLT";
            string desc = _site + mode;
            this->_default_node(document, "name", desc.c_str(), false);

            xml_node doc_style1 = this->_default_node(document, "Style");
            string id1 = "P1";
            this->_default_attr(doc_style1, "id", id1);
            xml_node IconStyle = this->_default_node(doc_style1, "IconStyle");
            this->_default_node(IconStyle, "color", "ff00ff64");  
            this->_default_node(IconStyle, "scale", "0.7");
            xml_node Icon = this->_default_node(IconStyle, "Icon");
            this->_default_node(Icon, "href", "http://maps.google.com/mapfiles/kml/shapes/placemark_circle.png");

            xml_node BalloonStyle = this->_default_node(doc_style1, "BalloonStyle");
            this->_default_node(BalloonStyle, "color", "ffd5f3fa");  
            xml_node text = BalloonStyle.append_child("text");
            string str("<b><font color=\"#CC0000\" size=\"+3\">$[name]</font></b><br>$[description]</font><br/>");
            text.append_child(pugi::node_cdata).set_value(str.c_str());

            xml_node doc_style2 = document.append_copy(doc_style1);
            string id2 = "P2";
            this->_default_attr(doc_style2, "id", id2, true);
            IconStyle = this->_default_node(doc_style2, "IconStyle");
            this->_default_node(IconStyle, "color", "ff78c800", true);

            xml_node doc_style3 = document.append_copy(doc_style1);
            string id3 = "P3";
            this->_default_attr(doc_style3, "id", id3, true);
            IconStyle = this->_default_node(doc_style3, "IconStyle");
            this->_default_node(IconStyle, "color", "ffff9600", true);

            xml_node doc_style4 = document.append_copy(doc_style1);
            string id4 = "P4";
            this->_default_attr(doc_style4, "id", id4, true);
            IconStyle = this->_default_node(doc_style4, "IconStyle");
            this->_default_node(IconStyle, "color", "ffff6496", true);

            xml_node doc_style5 = document.append_copy(doc_style1);
            string id5 = "P5";
            this->_default_attr(doc_style5, "id", id5, true);
            IconStyle = this->_default_node(doc_style5, "IconStyle");
            this->_default_node(IconStyle, "color", "ffff00ff", true);

            xml_node doc_style6 = document.append_copy(doc_style1);
            string id6 = "P6";
            this->_default_attr(doc_style6, "id", id6, true);
            IconStyle = this->_default_node(doc_style6, "IconStyle");
            this->_default_node(IconStyle, "color", "ff0000ff", true);

            placemark = this->_default_node(document, "Placemark");
            this->_default_node(placemark, "name", "Trajection", false);
            xml_node style = this->_default_node(placemark, "Style");
            xml_node LineStyle = this->_default_node(style, "LineStyle");
            this->_default_node(LineStyle, "color", "ff00ffff", false);
            this->_default_node(LineStyle, "width", "2", false);
            line = this->_default_node(placemark, "LineString");
            this->_default_node(line, "coordinates");
        }

        _read = true;
        return;
    }

} // namespace

void t_gpppflt::_reset_param()
{
    vector<int> ind = _param.delAmb();
    Matrix_rem(_Qx, ind);
    _initialized = false;
}
