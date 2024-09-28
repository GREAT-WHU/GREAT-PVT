/**
 * @file         gsppbias.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        mainly about spp bias
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gmodels/gsppbias.h"
#include "gmodels/ggmf.h"
#include <gutils/gsysconv.h>
namespace great
{
    great::t_gsppbias::t_gsppbias(t_spdlog spdlog, t_gsetbase *settings)
    {
    }

    great::t_gsppbias::~t_gsppbias()
    {
    }

    bool t_gsppbias::cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gobs &gobs, t_gbaseEquation &result)
    {
        return false;
    }

    void t_gsppbias::update_obj_clk(const string &obj, const t_gtime &epo, double clk)
    {
    }

    double t_gsppbias::get_rec_clk(const string &obj)
    {
        return 0.0;
    }

    double t_gsppbias::tropoDelay(t_gtime &epoch, string &rec, t_gallpar &param, t_gtriple ell, t_gsatdata &satdata)
    {
        if (_tropoModel == 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "Tropo Model setting is not correct. Default used! Check config.");
            throw runtime_error("Can not find _tropoModel !");
        }

        double ele = satdata.ele();
        double delay = 0.0;
        double zwd = 0.0;
        double zhd = 0.0;

        if (abs(ell[2]) > 1E4)
        {
            return 0.0;
        }

        int i = param.getParam(rec, par_type::TRP, "");
        if (i >= 0)
        {
            zwd = param[i].value();
            zhd = param[i].apriori();
        }
        else
        {
            if (_tropoModel != 0)
            {
                zwd = _tropoModel->getZWD(ell, epoch);
                zhd = _tropoModel->getZHD(ell, epoch);
            }
        }

        if (_mf_ztd == gnut::ZTDMPFUNC::GMF)
        {
            double gmfh, gmfw, dgmfh, dgmfw;
            t_gmf mf;
            mf.gmf(epoch.mjd(), ell[0], ell[1], ell[2], G_PI / 2.0 - ele,
                   gmfh, gmfw, dgmfh, dgmfw);
            delay = gmfh * zhd + gmfw * zwd;

        }
        else if (_mf_ztd == gnut::ZTDMPFUNC::COSZ)
        {
            double mf = 1 / sin(ele);
            delay = mf * zhd + mf * zwd;
        }

        return delay;
    }

    double t_gsppbias::isbDelay(t_gallpar &param, string &sat, string &rec, t_gobs &gobs)
    {
        return 0.0;
    }

    double t_gsppbias::ifbDelay(t_gallpar &param, string &sat, string &rec, t_gobs &gobs)
    {
        return 0.0;
    }

    double t_gsppbias::cmpObs(t_gtime &epo, string &sat, string &rec, t_gallpar &param, t_gsatdata &gsatdata, t_gobs &gobs)
    {

        // Cartesian coordinates to ellipsodial coordinates
        t_gtriple xyz;
        t_gtriple ell;

        // give crd initial value modified by glfeng
        if (param.getCrdParam(rec, xyz) < 0)
        {
            throw runtime_error("Can not find CRD par in t_gallpar !");
        }
        xyz2ell(xyz, ell, false);

        t_gtriple satcrd = gsatdata.satcrd();
        ColumnVector cSat = satcrd.crd_cvect();

        // Tropospheric wet delay correction
        double trpDelay = 0;
        trpDelay = tropoDelay(epo, rec, param, ell, gsatdata);
        if (fabs(trpDelay) > 50)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "trpDelay > 50");
            return -1;
        }

        // Receiver clock correction
        double clkRec = 0.0;
        int i = param.getParam(rec, par_type::CLK, "");
        if (i >= 0)
        {
            clkRec = param[i].value();
        }
        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, rec + " ! warning:  Receiver Clock is not included in parameters!");
        }

        // system time offset
        double isb_offset = isbDelay(param, sat, rec, gobs);

        double ifb_offset = ifbDelay(param, sat, rec, gobs);

        // Return value
        return gsatdata.rho() +
               clkRec -
               gsatdata.clk() +
               trpDelay +
               isb_offset +
               ifb_offset;
    }
}
