
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <iostream>

#include "gmodels/gdop.h"
#include "gutils/gsysconv.h"

using namespace std;

namespace gnut
{

    t_gdop::t_gdop()
    {
        _gnav = 0;
        _gobs = 0;
        _site = "";
        _Qx.ReSize(4);
    }
    t_gdop::t_gdop(t_spdlog spdlog)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }

        _gnav = 0;
        _gobs = 0;
        _site = "";
        _Qx.ReSize(4);
    }

    t_gdop::t_gdop(t_spdlog spdlog, t_gallnav *gnav, t_gallobs *gobs, string site)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }
        _gnav = gnav;
        _gobs = gobs;
        _site = site;
        _Qx.ReSize(4);
    }

    t_gdop::t_gdop(t_gallnav *gnav, t_gallobs *gobs, string site)
    {

        _gnav = gnav;
        _gobs = gobs;
        _site = site;
        _Qx.ReSize(4);
    }

    t_gdop::t_gdop(t_gallnav *gnav, set<string> sats)
    {
        _gnav = gnav;
        _sats = sats;
        _Qx.ReSize(4);
        _site = "";
        _gobs = 0;
    }

    t_gdop::t_gdop(t_spdlog spdlog, t_gallnav *gnav, set<string> sats)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }

        _gnav = gnav;
        _sats = sats;
        _Qx.ReSize(4);
        _site = "";
        _gobs = 0;
    }

    t_gdop::~t_gdop()
    {
    }

    void t_gdop::set_data(t_gallnav *gnav, t_gallobs *gobs, string site)
    {

        _gnav = gnav;
        _gobs = gobs;
        _site = site;
    }

    void t_gdop::set_log(t_spdlog spdlog)
    {
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }
    }

    void t_gdop::set_sats(set<string> &sats)
    {

        _sats = sats;
    }

    int t_gdop::calculate(const t_gtime &epoch, t_gtriple &rec, GSYS gnss)
    {

        _Qx = 0;

        _rec = rec;

        if (_sats.size() == 0)
        {
            if (_gobs)
                _sats = _gobs->sats(_site, epoch, gnss);
            else if (_sats.size() == 0)
            {
                string msg = "WARNING - not selected satellites for DOP calculation!";
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, msg);
                return -1;
            }
        }

        if (_sats.size() == 0)
            return -1;
        unsigned int Nsat = _sats.size();

        Matrix A(Nsat, 4);
        A = 0;
        int i = 0;
        for (set<string>::iterator it = _sats.begin(); it != _sats.end(); it++)
        {

            double xyz[3] = {0.0, 0.0, 0.0};
            double vel[3] = {0.0, 0.0, 0.0};
            double var[3] = {0.0, 0.0, 0.0};

            int irc = _gnav->pos(*it, epoch, xyz, var, vel);
            if (irc < 0)
                continue;

            t_gtriple satpos(xyz);
            t_gtriple xyz_rho = satpos - _rec;
            t_gtriple ell_site;
            xyz2ell(_rec, ell_site, false);

            // select only visible satellites
            t_gtriple neu_sat;
            xyz2neu(ell_site, xyz_rho, neu_sat);
            if (neu_sat[2] < 0)
                continue;

            double rho = (_rec.crd_cvect() - satpos.crd_cvect()).norm_Frobenius();

            i++;
            A(i, 1) = (_rec[0] - satpos[0]) / rho;
            A(i, 2) = (_rec[1] - satpos[1]) / rho;
            A(i, 3) = (_rec[2] - satpos[2]) / rho;
            A(i, 4) = 1.0;
        }
        A = A.Rows(1, i); // delete zero rows
        if (A.Nrows() < 4)
            return -1;

        Matrix NN = A.t() * A;

        _Qx << NN.i();

        return 1;
    }

    double t_gdop::pdop()
    {

        if (_Qx.Ncols() != _Qx.Nrows())
            return -1.0;
        if (_Qx.Ncols() != 4)
            return -1.0;

        return sqrt(_Qx(1, 1) + _Qx(2, 2) + _Qx(3, 3));
    }

    double t_gdop::gdop()
    {

        if (_Qx.Ncols() != _Qx.Nrows())
            return -1.0;
        if (_Qx.Ncols() != 4)
            return -1.0;

        return sqrt(_Qx(1, 1) + _Qx(2, 2) + _Qx(3, 3) + _Qx(4, 4));
    }

    double t_gdop::tdop()
    {

        if (_Qx.Ncols() != _Qx.Nrows())
            return -1.0;
        if (_Qx.Ncols() != 4)
            return -1.0;

        return sqrt(_Qx(4, 4));
    }

    double t_gdop::hdop()
    {

        if (_Qx.Ncols() != _Qx.Nrows())
            return -1.0;
        if (_Qx.Ncols() != 4)
            return -1.0;

        SymmetricMatrix Qp = _Qx.SymSubMatrix(1, 3);
        SymmetricMatrix Qneu;
        t_gtriple neu;

        xyz2neu(_rec, _Qx, Qneu);

        return sqrt(Qneu(1, 1));
    }

    double t_gdop::vdop()
    {

        if (_Qx.Ncols() != _Qx.Nrows())
            return -1.0;
        if (_Qx.Ncols() != 4)
            return -1.0;

        SymmetricMatrix Qp = _Qx.SymSubMatrix(1, 3);
        SymmetricMatrix Qneu;
        t_gtriple neu;

        xyz2neu(_rec, _Qx, Qneu);

        return sqrt(Qneu(3, 3));
    }

} // namespace
