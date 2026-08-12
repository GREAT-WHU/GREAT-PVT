
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <iostream>

#include "gmodels/gdop.h"
#include "gutils/gsysconv.h"


namespace gnut
{

    t_gdop::t_gdop()
    {
        _gnav = 0;
        _gobs = 0;
        _site = "";
        _Qx.setZero(4, 4);
    }
    t_gdop::t_gdop(t_gallnav* gnav, t_gallobs* gobs, std::string site)
    {
        _gnav = gnav;
        _gobs = gobs;
        _site = site;
        _Qx.setZero(4, 4);
    }

    t_gdop::t_gdop(t_gallnav* gnav, std::set<GSAT> sats)
    {
        _gnav = gnav;
        _sats = sats;
        _Qx.setZero(4, 4);
        _site = "";
        _gobs = 0;
    }

    t_gdop::~t_gdop()
    {
    }

    void t_gdop::set_data(t_gallnav* gnav, t_gallobs* gobs, std::string site)
    {
        _gnav = gnav;
        _gobs = gobs;
        _site = site;
    }

    void t_gdop::set_sats(std::set<GSAT>& sats)
    {
        _sats = sats;
    }

    int t_gdop::calculate(const t_gtime& epoch, t_gtriple& rec, GSYS gnss)
    {
        _Qx.setZero();

        _rec = rec;

        if (_sats.size() == 0)
        {
            if (_gobs)
            {
                _sats = _gobs->sats(_site, epoch, gnss);
            }
            else if (_sats.size() == 0)
            {
                std::string msg = "WARNING - not selected satellites for DOP calculation!";
                GREAT_DEBUG(msg);
                return -1;
            }
        }

        if (_sats.size() == 0)
        {
            return -1;
        }
        unsigned int Nsat = _sats.size();

        Eigen::MatrixXd A = Eigen::MatrixXd::Zero(Nsat, 4);
        int i = 0;
        for (std::set<GSAT>::iterator it = _sats.begin(); it != _sats.end(); it++)
        {
            double xyz[3] = {0.0, 0.0, 0.0};
            double vel[3] = {0.0, 0.0, 0.0};
            double var[3] = {0.0, 0.0, 0.0};

            int irc = _gnav->pos(*it, epoch, xyz, var, vel);
            if (irc < 0)
            {
                continue;
            }

            t_gtriple satpos(xyz);
            t_gtriple xyz_rho = satpos - _rec;
            t_gtriple ell_site;
            xyz2ell(_rec, ell_site, false);

            // select only visible satellites
            t_gtriple neu_sat;
            xyz2neu(ell_site, xyz_rho, neu_sat);
            if (neu_sat[2] < 0)
            {
                continue;
            }

            double rho = (_rec.crd_cvect() - satpos.crd_cvect()).norm();

            A(i, 0) = (_rec[0] - satpos[0]) / rho;
            A(i, 1) = (_rec[1] - satpos[1]) / rho;
            A(i, 2) = (_rec[2] - satpos[2]) / rho;
            A(i, 3) = 1.0;
            i++;
        }
        A = A.topRows(i).eval(); // delete zero rows
        if (A.rows() < 4)
        {
            return -1;
        }

        Eigen::MatrixXd NN = A.transpose() * A;

        _Qx = NN.inverse();

        return 1;
    }

    double t_gdop::pdop()
    {
        if (_Qx.cols() != _Qx.rows())
        {
            return -1.0;
        }
        if (_Qx.cols() != 4)
        {
            return -1.0;
        }

        return sqrt(_Qx(0, 0) + _Qx(1, 1) + _Qx(2, 2));
    }

    double t_gdop::gdop()
    {
        if (_Qx.cols() != _Qx.rows())
        {
            return -1.0;
        }
        if (_Qx.cols() != 4)
        {
            return -1.0;
        }

        return sqrt(_Qx(0, 0) + _Qx(1, 1) + _Qx(2, 2) + _Qx(3, 3));
    }

    double t_gdop::tdop()
    {
        if (_Qx.cols() != _Qx.rows())
        {
            return -1.0;
        }
        if (_Qx.cols() != 4)
        {
            return -1.0;
        }

        return sqrt(_Qx(3, 3));
    }

    double t_gdop::hdop()
    {
        if (_Qx.cols() != _Qx.rows())
        {
            return -1.0;
        }
        if (_Qx.cols() != 4)
        {
            return -1.0;
        }

        Eigen::MatrixXd Qneu;

        xyz2neu(_rec, _Qx, Qneu);

        return sqrt(Qneu(0, 0));
    }

    double t_gdop::vdop()
    {
        if (_Qx.cols() != _Qx.rows())
        {
            return -1.0;
        }
        if (_Qx.cols() != 4)
        {
            return -1.0;
        }

        Eigen::MatrixXd Qneu;

        xyz2neu(_rec, _Qx, Qneu);

        return sqrt(Qneu(2, 2));
    }

} // namespace gnut
