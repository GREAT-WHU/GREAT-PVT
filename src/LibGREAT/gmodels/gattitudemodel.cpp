/**
 * @file         gattitudemodel.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        attitude model class
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gattitudemodel.h"

namespace great
{
    int t_gattitude_model::attitude_old(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        if (satdata.gsys() == GAL)
        {
            _ysm(satdata, i, j, k);
        }
        else
        {
            int irc = _yaw(satdata, antype, i, j, k);
            if (irc == 0)
                return 0;
        }

        return 1;
    }

    int t_gattitude_model::attitude(t_gsatdata &satdata, double yaw, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        _yaw2ijk(satdata, yaw, i, j, k);
        return 1;
    }

    int t_gattitude_model::attitude(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        Eigen::Vector3d satcrd = satdata.satcrd().crd_cvect_Eigen();
        Eigen::Vector3d satvel = satdata.satvel().crd_cvect_Eigen();

        if (satcrd.norm() == 0 || satvel.norm() == 0)
        {
            return -1;
        }

        if (satdata.gsys() == GPS)
        {
            if (antype == "BLOCK II")
            {
                _attitude_GPSIIA(satdata, antype, i, j, k);
            }
            else if (antype == "BLOCK IIA")
            {
                _attitude_GPSIIA(satdata, antype, i, j, k);
            }
            else if (antype.find("BLOCK IIR") != string::npos)
            {
                _attitude_GPSIIR(satdata, antype, i, j, k);
            }
            else if (antype == "BLOCK IIF")
            {
                _attitude_GPSIIF(satdata, antype, i, j, k);
            }
            else if (antype == "BLOCK III")
            { // added by yqyuan for GPSIII (BLOCK III/IIIA/IIIF???)
                _attitude_GPSIII(satdata, antype, i, j, k);
            }
            else
                _ysm(satdata, i, j, k);
        }
        else if (satdata.gsys() == GLO)
        {
            _attitude_GLO(satdata, antype, i, j, k);
        }
        else if (satdata.gsys() == GAL)
        {
            if (antype == "GALILEO-1")
            {
                _attitude_GAL1(satdata, antype, i, j, k);
            }
            else if (antype == "GALILEO-2")
            {
                _attitude_GAL2(satdata, antype, i, j, k);
            }
            else
                _ysm(satdata, i, j, k);
        }
        else if (satdata.gsys() == BDS)
        {
            _attitude_BDS(satdata, antype, i, j, k);
        }
        else if (satdata.gsys() == QZS)
        {
            _attitude_QZS(satdata, antype, i, j, k);
        }

        if (i.norm() == 0 || j.norm() == 0 || k.norm() == 0)
            return 0;

        return 1;
    }

    int t_gattitude_model::attitude(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        ColumnVector satcrd = xsat;
        ColumnVector satvel = vsat;

        double orb_angle = _orb_angle(xsat, vsat, xsun);
        double beta = _beta(xsat, vsat, xsun);

        if (satcrd.NormFrobenius() == 0 || satvel.NormFrobenius() == 0)
        {
            return -1;
        }

        if (prn.find("G") != string::npos)
        {
            if (antype == "BLOCK II")
            {
                _attitude_GPSIIA(antype, prn, xsat, vsat, xsun, i, j, k);
            }
            else if (antype == "BLOCK IIA")
            {
                _attitude_GPSIIA(antype, prn, xsat, vsat, xsun, i, j, k);
            }
            else if (antype.find("BLOCK IIR") != string::npos)
            {
                _attitude_GPSIIR(antype, prn, xsat, vsat, xsun, i, j, k);
            }
            else if (antype == "BLOCK IIF")
            {
                _attitude_GPSIIF(antype, prn, xsat, vsat, xsun, i, j, k);
            }
            else if (antype == "BLOCK III")
            { 
                _attitude_GPSIII(antype, prn, xsat, vsat, xsun, i, j, k);
            }
            else
                _ysm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
        else if (prn.find("R") != string::npos)
        {
            _attitude_GLO(antype, prn, xsat, vsat, xsun, i, j, k);
        }
        else if (prn.find("E") != string::npos)
        {
            if (antype == "GALILEO-1")
            {
                _attitude_GAL1(antype, prn, xsat, vsat, xsun, i, j, k);
            }
            else if (antype == "GALILEO-2")
            {
                _attitude_GAL2(antype, prn, xsat, vsat, xsun, i, j, k);
            }
            else
                _ysm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
        else if (prn.find("C") != string::npos)
        {
            _attitude_BDS(antype, prn, xsat, vsat, xsun, i, j, k);
        }
        else if (prn.find("J") != string::npos)
        {
            _attitude_QZS(antype, prn, xsat, vsat, xsun, i, j, k);
        }

        if (i.NormFrobenius() == 0 || j.NormFrobenius() == 0 || k.NormFrobenius() == 0)
            return 0;
        return 1;
    }

    double t_gattitude_model::_orb_angle(ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun)
    {
        double mi = 0.0;
        ColumnVector Sun = xsun; //ICRF
        ColumnVector Satcrd = xsat;
        ColumnVector Satvel = vsat;

        ColumnVector n = crossproduct(Satcrd, Satvel);

        ColumnVector es = Satcrd / Satcrd.NormFrobenius();

        ColumnVector eSun = Sun / Sun.NormFrobenius();

        ColumnVector p = crossproduct(Sun, n);
        p /= p.NormFrobenius();

        double E = acos(DotProduct(es, p));

        double SunSat = acos(DotProduct(es, eSun));

        if (SunSat > G_PI / 2)
        {
            if (E <= G_PI / 2)
                mi = G_PI / 2 - E;
            else
                mi = G_PI / 2 - E;
        }
        else
        {
            if (E <= G_PI / 2)
                mi = G_PI / 2 + E;
            else
                mi = E - G_PI - G_PI / 2;
        }

        return mi;
    }

    double t_gattitude_model::_beta(ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun)
    {
        double beta = 0.0;

        ColumnVector Sun = xsun;

        ColumnVector Satcrd = xsat;
        ColumnVector Satvel = vsat;

        ColumnVector n = crossproduct(Satcrd, Satvel);

        n /= n.NormFrobenius();

        ColumnVector nSun = Sun / Sun.NormFrobenius();

        double cosa = DotProduct(nSun, n);

        beta = G_PI / 2.0 - acos(cosa);

        return beta;
    }

    void t_gattitude_model::_ysm(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double MJD = satdata.epoch().dmjd();

        t_gtriple satcrd_t = satdata.satcrd();
        Eigen::Vector3d satcrd = satcrd_t.crd_cvect_Eigen();

        // Satelite-Earth unit vector
        k = -satcrd;

        // Along solar panel unit vector
        Eigen::Vector3d sun = _ephplan.sunPos(MJD).crd_cvect_Eigen();
        j = k.cross(sun);

        // complete to satelite fixed right-hand coord system
        i = j.cross(k);

        i = i / i.norm();
        j = j / j.norm();
        k = k / k.norm();

        _last_beta[satdata.sat()] = satdata.beta();

        double yaw = atan2(-satdata.beta(), sin(satdata.orb_angle()));
        satdata.yaw(yaw);
    }

    void t_gattitude_model::_ysm(string prn, double bata, double mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        ColumnVector satcrd = xsat;

        // Satelite-Earth unit vector
        k = -satcrd;

        // Along solar panel unit vector
        ColumnVector sun = xsun;
        j = crossproduct(k, sun);
        // there is a little difference for Y-axis: Xsun vs Xsun-Xsat.
        // The resulting error can be ignored. yqyuan
        //ColumnVector sat2sun = _ephplan.sunPos(MJD).crd_cvect() - satcrd;
        //j = crossproduct(k, sat2sun);

        // complete to satelite fixed right-hand coord system
        i = crossproduct(j, k);

        i = i / i.NormFrobenius();
        j = j / j.NormFrobenius();
        k = k / k.NormFrobenius();

        _last_beta[prn] = bata;

    }

    void t_gattitude_model::_onm(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double yaw = 0;
        _yaw2ijk(satdata, yaw, i, j, k);
    }

    void t_gattitude_model::_onm(ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double yaw = 0;
        _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
    }

    int t_gattitude_model::_yaw(t_gsatdata &satdata, string antype, Eigen::Vector3d &xs, Eigen::Vector3d &ys, Eigen::Vector3d &zs)
    {
#if 0
        double exs[3], eys[3], ezs[3], r[3];
        int i;

        int opt = 2; //2 :precise yaw

        bool yawFix = t_gsys::bds_geo(satdata.sat()) || (satdata.beta() * R2D <= 4 && satdata.beta() * R2D >= -4);
        if (satdata.gsys() == GSYS::BDS && yawFix) {  //Only for BDS, opt=3
            opt = 3;
        }

        double rs[6];
        for (int i = 0; i < 3; i++)  rs[i] = satdata.satcrd()[i];
        for (int i = 3; i < 6; i++)  rs[i] = satdata.satvel()[i - 3];

        string prn = satdata.sat();
        t_gtime epoch = satdata.epoch();

        for (i = 0; i < 3; i++) r[i] = -rs[i];
        if (!normv3(r, ezs)) return 0;
        /* satellite yaw attitude model */
        double  ep[6];
        ep[0] = epoch.year(); ep[1] = epoch.mon(); ep[2] = epoch.day();
        ep[3] = epoch.hour(); ep[4] = epoch.mins(); ep[5] = epoch.secs();
        gtime_t time = epoch2time(ep);

        if (!normv3(r, ezs)) return 0;
        if (!sat_yaw(time, prn.c_str(), antype.c_str(), opt, rs, exs, eys)) return 0;
        for (int i = 0; i < 3; i++)
        {
            xs(i + 1) = exs[i];
            ys(i + 1) = eys[i];
            zs(i + 1) = ezs[i];
        }
#endif
        return 1;
    }

    void t_gattitude_model::_attitude_GPSIIA(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        const double R_GPSIIA[] = {
            0.1046, 0.1230, 0.1255, 0.1249, 0.1003, 0.1230, 0.1136, 0.1169, 0.1253, 0.0999,
            0.1230, 0.1230, 0.1230, 0.1230, 0.1092, 0.1230, 0.1230, 0.1230, 0.1230, 0.1230,
            0.1230, 0.1230, 0.1230, 0.0960, 0.0838, 0.1284, 0.1183, 0.1230, 0.1024, 0.1042,
            0.1230, 0.1100, 0.1230};
        int sat = str2int(satdata.sat().substr(1, 2));
        double R = R_GPSIIA[sat - 1] * D2R;

        double beta0 = atan2(MUDOT_GPS, R);

        if (fabs(satdata.orb_angle()) > G_PI / 2 && fabs(satdata.beta()) < beta0)
        {
            // noon maneuver
            _noon_turn(satdata, R, i, j, k);
        }
        else if (fabs(satdata.orb_angle()) < G_PI / 2 && fabs(satdata.beta()) < beta0)
        {
            // midnight maneuver
            _midnight_turn_GPSIIA(satdata, R, i, j, k);
        }
        else
        {
            _ysm(satdata, i, j, k);
        }
    }

    void t_gattitude_model::_attitude_GPSIIA(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        const double R_GPSIIA[] = {
            0.1046, 0.1230, 0.1255, 0.1249, 0.1003, 0.1230, 0.1136, 0.1169, 0.1253, 0.0999,
            0.1230, 0.1230, 0.1230, 0.1230, 0.1092, 0.1230, 0.1230, 0.1230, 0.1230, 0.1230,
            0.1230, 0.1230, 0.1230, 0.0960, 0.0838, 0.1284, 0.1183, 0.1230, 0.1024, 0.1042,
            0.1230, 0.1100, 0.1230};
        int sat = str2int(prn.substr(1, 2));
        double R = R_GPSIIA[sat - 1] * D2R;

        double beta0 = atan2(MUDOT_GPS, R);
        double orb_angle = _orb_angle(xsat, vsat, xsun);
        double beta = _beta(xsat, vsat, xsun);

        if (fabs(orb_angle) > G_PI / 2 && fabs(beta) < beta0)
        {
            // noon maneuver
            _noon_turn(prn, beta, orb_angle, xsat, vsat, xsun, R, i, j, k);
        }
        else if (fabs(orb_angle) < G_PI / 2 && fabs(beta) < beta0)
        {
            // midnight maneuver
            _midnight_turn_GPSIIA(prn, beta, orb_angle, xsat, vsat, xsun, R, i, j, k);
        }
        else
        {
            _ysm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
    }

    void t_gattitude_model::_attitude_GPSIIR(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        const double R = 0.2 * D2R; // maximal yaw hardware rate
        double beta0 = atan2(MUDOT_GPS, R);

        if (fabs(satdata.orb_angle()) > G_PI / 2 && fabs(satdata.beta()) < beta0)
        {
            // noon maneuver
            _noon_turn(satdata, R, i, j, k);
        }
        else if (fabs(satdata.orb_angle()) < G_PI / 2 && fabs(satdata.beta()) < beta0)
        {
            // midnight maneuver
            _midnight_turn(satdata, R, i, j, k);
        }
        else
        {
            _ysm(satdata, i, j, k);
        }

        i = -1 * i; // X away from the Sum
        j = -1 * j;
        satdata.yaw(satdata.yaw() + G_PI);
    }

    void t_gattitude_model::_attitude_GPSIIR(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        const double R = 0.2 * D2R; // maximal yaw hardware rate
        double beta0 = atan2(MUDOT_GPS, R);
        double orb_angle = _orb_angle(xsat, vsat, xsun);
        double beta = _beta(xsat, vsat, xsun);

        if (fabs(orb_angle) > G_PI / 2 && fabs(beta) < beta0)
        {
            // noon maneuver
            _noon_turn(prn, beta, orb_angle, xsat, vsat, xsun, R, i, j, k);
        }
        else if (fabs(orb_angle) < G_PI / 2 && fabs(beta) < beta0)
        {
            // midnight maneuver
            _midnight_turn_GPSIIA(prn, beta, orb_angle, xsat, vsat, xsun, R, i, j, k);
        }
        else
        {
            _ysm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }

        i = -1 * i; // X away from the Sum
        j = -1 * j;
    }

    void t_gattitude_model::_noon_turn(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta0 = atan2(MUDOT_GPS, R);
        double beta = satdata.beta();
        double mi = satdata.orb_angle();

        // test of beta sign change
        auto itSAT = _last_beta.find(satdata.sat());
        if (itSAT != _last_beta.end())
        {
            if (itSAT->second * beta < 0)
                beta *= -1;
        }

        if (beta >= 0)
            R *= -1;

        double mi_s = G_PI - sqrt(beta0 * fabs(beta) - beta * beta);
        double yaw;
        double yaw_nom = atan2(-tan(beta), sin(mi));

        if (mi >= mi_s || mi <= 0)
        {
            if (mi < 0)
                mi += 2.0 * G_PI;
            yaw = atan2(-tan(beta), sin(mi_s)) + R * (mi - mi_s) / MUDOT_GPS;
            if ((beta >= 0 && yaw < yaw_nom) || (beta < 0 && yaw > yaw_nom))
                yaw = yaw_nom;
        }
        else
            yaw = yaw_nom;

        _yaw2ijk(satdata, yaw, i, j, k);
    }

    void t_gattitude_model::_midnight_turn(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta0 = atan2(MUDOT_GPS, R);
        double beta = satdata.beta();
        double mi = satdata.orb_angle();

        // test of beta sign change
        auto itSAT = _last_beta.find(satdata.sat());
        if (itSAT != _last_beta.end())
        {
            if (itSAT->second * beta < 0)
                beta *= -1;
        }

        if (beta < 0)
            R *= -1;

        double mi_s = -sqrt(beta0 * fabs(beta) - beta * beta);
        double yaw;
        double yaw_nom = atan2(-tan(beta), sin(mi));

        if (mi >= mi_s)
        {
            yaw = atan2(-tan(beta), sin(mi_s)) + R * (mi - mi_s) / MUDOT_GPS;
            if ((beta >= 0 && yaw > yaw_nom) || (beta < 0 && yaw < yaw_nom))
                yaw = yaw_nom;
        }
        else
            yaw = yaw_nom;

        _yaw2ijk(satdata, yaw, i, j, k);
    }

    void t_gattitude_model::_noon_turn(string _prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta0 = atan2(MUDOT_GPS, R);
        double beta = _beta;
        double mi = _mi;

        // test of beta sign change
        auto itSAT = _last_beta.find(_prn);
        if (itSAT != _last_beta.end())
        {
            if (itSAT->second * beta < 0)
                beta *= -1;
        }

        if (beta >= 0)
            R *= -1;

        double mi_s = G_PI - sqrt(beta0 * fabs(beta) - beta * beta);
        double yaw;
        double yaw_nom = atan2(-tan(beta), sin(mi));

        if (mi >= mi_s || mi <= 0)
        {
            if (mi < 0)
                mi += 2.0 * G_PI;
            yaw = atan2(-tan(beta), sin(mi_s)) + R * (mi - mi_s) / MUDOT_GPS;
            if ((beta >= 0 && yaw < yaw_nom) || (beta < 0 && yaw > yaw_nom))
                yaw = yaw_nom;
        }
        else
            yaw = yaw_nom;

        _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
    }

    void t_gattitude_model::_midnight_turn_GPSIIA(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta = satdata.beta();
        double mi = satdata.orb_angle();
        double mi_s = -sqrt(EPS0_GPS * EPS0_GPS - beta * beta);
        double mi_e = -mi_s;

        // test of beta sign change
        auto itSAT = _last_beta.find(satdata.sat());
        if (itSAT != _last_beta.end())
        {
            if (itSAT->second * beta < 0)
                beta *= -1;
        }

        if (beta < 0)
            R *= -1;

        double yaw;

        if (mi_s <= mi && mi < mi_e)
        {
            yaw = atan2(-tan(beta), sin(mi_s)) + R * (mi - mi_s) / MUDOT_GPS;
        }
        else if (mi_e <= mi && mi < mi_e + POST_SHADOW * MUDOT_GPS)
            satdata.setecl(true);
        else
            yaw = atan2(-tan(beta), sin(mi));

        _yaw2ijk(satdata, yaw, i, j, k);
    }

    void t_gattitude_model::_midnight_turn_GPSIIA(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta = _beta;
        double mi = _mi;
        double mi_s = -sqrt(EPS0_GPS * EPS0_GPS - beta * beta);
        double mi_e = -mi_s;

        // test of beta sign change
        auto itSAT = _last_beta.find(prn);
        if (itSAT != _last_beta.end())
        {
            if (itSAT->second * beta < 0)
                beta *= -1;
        }

        if (beta < 0)
            R *= -1;

        double yaw;

        if (mi_s <= mi && mi < mi_e)
        {
            yaw = atan2(-tan(beta), sin(mi_s)) + R * (mi - mi_s) / MUDOT_GPS;
        }
        else if (mi_e <= mi && mi < mi_e + POST_SHADOW * MUDOT_GPS)
            ; //satdata.setecl(true);
        else
            yaw = atan2(-tan(beta), sin(mi));

        _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
    }

    void t_gattitude_model::_midnight_turn_GPSIIF(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta = satdata.beta();
        double tan_beta = tan(beta);
        double mi = satdata.orb_angle();

        // test of beta sign change
        auto itSAT = _last_beta.find(satdata.sat());
        if (itSAT != _last_beta.end())
        {
            if (itSAT->second * beta < 0)
                beta *= -1;
        }

        if (beta < 0)
            R *= -1;

        double mi_s = -acos(cos(EPS0_GPS) / cos(beta));
        double mi_e = -mi_s;
        double sin_mi_s = sin(mi_s);
        double mi_f = MUDOT_GPS * (atan2(-tan_beta, -sin_mi_s) - atan2(-tan_beta, sin_mi_s)) / R + mi_s;

        double yaw;

        if (mi_s <= mi && mi < mi_f)
        {
            yaw = atan2(-tan_beta, sin_mi_s) + R * (mi - mi_s) / MUDOT_GPS;
            _yaw2ijk(satdata, yaw, i, j, k);
        }
        else if (mi_f <= mi && mi < mi_e)
        {
            yaw = atan2(-tan_beta, -sin_mi_s);
            _yaw2ijk(satdata, yaw, i, j, k);
        }
        else
            _ysm(satdata, i, j, k);
    }

    void t_gattitude_model::_midnight_turn_GPSIIF(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta = _beta;
        double tan_beta = tan(beta);
        double mi = _mi;

        // test of beta sign change
        auto itSAT = _last_beta.find(prn);
        if (itSAT != _last_beta.end())
        {
            if (itSAT->second * beta < 0)
                beta *= -1;
        }

        if (beta < 0)
            R *= -1;

        double mi_s = -acos(cos(EPS0_GPS) / cos(beta));
        double mi_e = -mi_s;
        double sin_mi_s = sin(mi_s);
        double mi_f = MUDOT_GPS * (atan2(-tan_beta, -sin_mi_s) - atan2(-tan_beta, sin_mi_s)) / R + mi_s;

        double yaw;

        if (mi_s <= mi && mi < mi_f)
        {
            yaw = atan2(-tan_beta, sin_mi_s) + R * (mi - mi_s) / MUDOT_GPS;
            _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
        }
        else if (mi_f <= mi && mi < mi_e)
        {
            yaw = atan2(-tan_beta, -sin_mi_s);
            _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
        }
        else
            _ysm(prn, beta, mi, xsat, vsat, xsun, i, j, k);
    }

    void t_gattitude_model::_midnight_turn_GLOM(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta = satdata.beta();
        double tan_beta = tan(beta);
        double mi = satdata.orb_angle();

        if (beta < 0)
            R *= -1;

        double mi_s = -acos(cos(EPS0_GLO) / cos(beta));
        double mi_e = -mi_s;
        double sin_mi_s = sin(mi_s);
        double mi_f = MUDOT_GLO * (atan2(-tan_beta, -sin_mi_s) - atan2(-tan_beta, sin_mi_s)) / R + mi_s;

        // debug by ZHJ initial
        double yaw = 0.0;

        if (mi_s <= mi && mi < mi_f)
        {
            yaw = atan2(-tan_beta, sin_mi_s) + R * (mi - mi_s) / MUDOT_GLO;
        }
        else if (mi_f <= mi && mi < mi_e)
        {
            yaw = atan2(-tan_beta, -sin_mi_s);
        }

        _yaw2ijk(satdata, yaw, i, j, k);
    }

    void t_gattitude_model::_midnight_turn_GLOM(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta = _beta;
        double tan_beta = tan(beta);
        double mi = _mi;

        if (beta < 0)
            R *= -1;

        double mi_s = -acos(cos(EPS0_GLO) / cos(beta));
        double mi_e = -mi_s;
        double sin_mi_s = sin(mi_s);
        double mi_f = MUDOT_GLO * (atan2(-tan_beta, -sin_mi_s) - atan2(-tan_beta, sin_mi_s)) / R + mi_s;

        // debug by ZHJ initial
        double yaw = 0.0;

        if (mi_s <= mi && mi < mi_f)
        {
            yaw = atan2(-tan_beta, sin_mi_s) + R * (mi - mi_s) / MUDOT_GLO;
        }
        else if (mi_f <= mi && mi < mi_e)
        {
            yaw = atan2(-tan_beta, -sin_mi_s);
        }

        _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
    }

    void t_gattitude_model::_noon_turn_GLOM(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta = satdata.beta();
        double mi = satdata.orb_angle();

        if (beta >= 0)
            R = -R;

        double mi_s, mi_e, sin_mi_s, B, yaw;
        int c = 0;

        for (mi_s = 178.6 * D2R; c < 4; c++)
        {
            sin_mi_s = sin(mi_s);
            B = -beta * cos(mi_s) / (beta * beta + sin_mi_s * sin_mi_s);
            mi_s = (atan(-beta / sin_mi_s) + mi_s * B + G_PI * R / MUDOT_GLO - G_PI / 2.0) / (R / MUDOT_GLO + B);
        }
        if (beta >= 0)
            mi_s = 2.0 * G_PI - mi_s;
        mi_e = 2.0 * G_PI - mi_s;

        if (mi_s <= mi && mi < mi_e)
        {
            yaw = atan2(-tan(beta), sin(mi_s)) + R * (mi - mi_s) / MUDOT_GLO;
        }

        _yaw2ijk(satdata, yaw, i, j, k);
    }

    void t_gattitude_model::_noon_turn_GLOM(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta = _beta;
        double mi = _mi;

        if (beta >= 0)
            R = -R;

        double mi_s, mi_e, sin_mi_s, B, yaw;
        int c = 0;

        for (mi_s = 178.6 * D2R; c < 4; c++)
        {
            sin_mi_s = sin(mi_s);
            B = -beta * cos(mi_s) / (beta * beta + sin_mi_s * sin_mi_s);
            mi_s = (atan(-beta / sin_mi_s) + mi_s * B + G_PI * R / MUDOT_GLO - G_PI / 2.0) / (R / MUDOT_GLO + B);
        }
        if (beta >= 0)
            mi_s = 2.0 * G_PI - mi_s;
        mi_e = 2.0 * G_PI - mi_s;

        if (mi_s <= mi && mi < mi_e)
        {
            yaw = atan2(-tan(beta), sin(mi_s)) + R * (mi - mi_s) / MUDOT_GLO;
        }

        _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
    }

    void t_gattitude_model::_attitude_GPSIIRM(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        _attitude_GPSIIR(satdata, antype, i, j, k);
    }

    void t_gattitude_model::_attitude_GPSIIF(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        const double R_noon = 0.11 * D2R; // maximal yaw hardware rate during noon turn
        const double R_midn = 0.06 * D2R; // maximal yaw hardware rate during midnight turn
        double beta0 = atan2(MUDOT_GPS, R_noon);

        if (fabs(satdata.orb_angle()) > G_PI / 2 && fabs(satdata.beta()) < beta0)
        {
            // noon maneuver
            _noon_turn(satdata, R_noon, i, j, k);
        }
        else if (fabs(satdata.orb_angle()) < EPS0_GPS && fabs(satdata.beta()) < EPS0_GPS)
        {
            // midnight maneuver
            _midnight_turn_GPSIIF(satdata, R_midn, i, j, k);
        }
        else
        {
            _ysm(satdata, i, j, k);
        }
    }

    void t_gattitude_model::_attitude_GPSIIF(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        const double R_noon = 0.11 * D2R; // maximal yaw hardware rate during noon turn
        const double R_midn = 0.06 * D2R; // maximal yaw hardware rate during midnight turn
        double beta0 = atan2(MUDOT_GPS, R_noon);

        double orb_angle = _orb_angle(xsat, vsat, xsun);
        double beta = _beta(xsat, vsat, xsun);

        if (fabs(orb_angle) > G_PI / 2 && fabs(beta) < beta0)
        {
            // noon maneuver
            _noon_turn(prn, beta, orb_angle, xsat, vsat, xsun, R_noon, i, j, k);
        }
        else if (fabs(orb_angle) < EPS0_GPS && fabs(beta) < EPS0_GPS)
        {
            // midnight maneuver
            _midnight_turn_GPSIIF(prn, beta, orb_angle, xsat, vsat, xsun, R_midn, i, j, k);
        }
        else
        {
            _ysm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
    }

    void t_gattitude_model::_attitude_GPSIII(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        // to be added;
        _ysm(satdata, i, j, k);
    }

    void t_gattitude_model::_attitude_GPSIII(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
    }

    void t_gattitude_model::_attitude_GLO(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        const double R = 0.25 * D2R; // maximal yaw hardware rate
        double beta0 = 2.0 * D2R;
        double mi = satdata.orb_angle();
        double mi_s = 176.8 * D2R;

        if (mi > mi_s && mi < 2.0 * G_PI - mi_s && fabs(satdata.beta()) < beta0)
        {
            // noon maneuver
            _noon_turn_GLOM(satdata, R, i, j, k);
        }
        else if (fabs(satdata.orb_angle()) < EPS0_GLO && fabs(satdata.beta()) < EPS0_GLO)
        {
            // midnight maneuver
            _midnight_turn_GLOM(satdata, R, i, j, k);
        }
        else
        {
            _ysm(satdata, i, j, k);
        }
    }

    void t_gattitude_model::_attitude_GLO(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        const double R = 0.25 * D2R; // maximal yaw hardware rate
        double beta0 = 2.0 * D2R;

        double beta = _beta(xsat, vsat, xsun);
        double mi = _orb_angle(xsat, vsat, xsun);
        double mi_s = 176.8 * D2R;

        if (mi > mi_s && mi < 2.0 * G_PI - mi_s && fabs(beta) < beta0)
        {
            // noon maneuver
            _noon_turn_GLOM(prn, beta, mi, xsat, vsat, xsun, R, i, j, k);
        }
        else if (fabs(mi) < EPS0_GLO && fabs(beta) < EPS0_GLO)
        {
            // midnight maneuver
            _midnight_turn_GLOM(prn, beta, mi, xsat, vsat, xsun, R, i, j, k);
        }
        else
        {
            _ysm(prn, beta, mi, xsat, vsat, xsun, i, j, k);
        }
    }

    void t_gattitude_model::_noon_turn_GAL1(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta = satdata.beta();
        double mi = satdata.orb_angle();

        double beta_y = 2 * D2R;
        double beta_x = 15 * D2R;

        double Sx = sin(mi) * cos(beta);
        double Sy = -sin(beta);

        double sinby = sin(beta_y);
        double sinbx = sin(beta_x);
        if (Sy < 0)
            sinby *= -1;

        double Shy = (sinby + Sy) / 2 + cos(G_PI * fabs(Sx) / sinbx) * (sinby - Sy) / 2;

        double yaw = atan2(Shy, Sx);

        // test of beta sign change
        auto itSAT = _last_beta.find(satdata.sat());
        if (itSAT != _last_beta.end())
        {
            if (itSAT->second * beta < 0)
                yaw *= -1;
        }

        _yaw2ijk(satdata, yaw, i, j, k);
    }

    void t_gattitude_model::_noon_turn_GAL1(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta = _beta;
        double mi = _mi;

        double beta_y = 2 * D2R;
        double beta_x = 15 * D2R;

        double Sx = sin(mi) * cos(beta);
        double Sy = -sin(beta);

        double sinby = sin(beta_y);
        double sinbx = sin(beta_x);
        if (Sy < 0)
            sinby *= -1;

        double Shy = (sinby + Sy) / 2 + cos(G_PI * fabs(Sx) / sinbx) * (sinby - Sy) / 2;

        double yaw = atan2(Shy, Sx);

        // test of beta sign change
        auto itSAT = _last_beta.find(prn);
        if (itSAT != _last_beta.end())
        {
            if (itSAT->second * beta < 0)
                yaw *= -1;
        }

        _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
    }

    void t_gattitude_model::_attitude_GAL1(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta0 = 2.0 * D2R;
        double mi_n0 = (180 - 15) * D2R;
        double mi_m0 = 15 * D2R;

        if (fabs(satdata.orb_angle()) > mi_n0 && fabs(satdata.beta()) < beta0)
        {
            // noon maneuver
            _noon_turn_GAL1(satdata, i, j, k);
        }
        else if (fabs(satdata.orb_angle()) < mi_m0 && fabs(satdata.beta()) < beta0)
        {
            // midnight maneuver
            _noon_turn_GAL1(satdata, i, j, k);
        }
        else
        {
            _ysm(satdata, i, j, k);
        }

    }

    void t_gattitude_model::_attitude_GAL1(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta0 = 2.0 * D2R;
        double mi_n0 = (180 - 15) * D2R; // noon
        double mi_m0 = 15 * D2R;         // midnight

        double orb_angle = _orb_angle(xsat, vsat, xsun);
        double beta = _beta(xsat, vsat, xsun);

        if (fabs(orb_angle) > mi_n0 && fabs(beta) < beta0)
        {
            // noon maneuver
            _noon_turn_GAL1(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k); // in IGS frame
        }
        else if (fabs(orb_angle) < mi_m0 && fabs(beta) < beta0)
        {
            // midnight maneuver
            _noon_turn_GAL1(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
        else
        {
            _ysm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k); // in IGS frame
        }
    }

    void t_gattitude_model::_noon_turn_GAL2(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta = satdata.beta();
        double mi = satdata.orb_angle();

        // test of beta sign change
        auto it = _last_beta.find(satdata.sat());
        if (it != _last_beta.end())
        {
            if (it->second * beta < 0)
                beta *= -1;
        }

        double init = G_PI / 2;
        if (beta > 0)
            init *= -1;

        double yaw_s;
        t_gtime epo_s;
        t_gtime epo = satdata.epoch();

        auto itYAW = _last_yaw.find(satdata.sat());
        if (itYAW != _last_yaw.end())
        {
            yaw_s = itYAW->second;
        }
        else
        {
            yaw_s = atan2(-tan(beta), sin(mi));
            _last_yaw[satdata.sat()] = yaw_s;
        }

        auto itEPO = _last_epo.find(satdata.sat());
        if (itEPO != _last_epo.end())
        {
            epo_s = itEPO->second;
        }
        else
        {
            epo_s = epo;
            _last_epo[satdata.sat()] = epo_s;
        }

        double yaw = init + (yaw_s - init) * cos((2.0 * G_PI / 5656) * (epo - epo_s));

        _yaw2ijk(satdata, yaw, i, j, k);
    }

    void t_gattitude_model::_noon_turn_GAL2(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        // the origional edition needs time info; updated with our model: Li et al. 2019
        double beta = _beta;
        double u = _mi;
        double yawangle = atan2(-tan(beta), sin(u)); // in IGS frame

        double ub_candidate_foc_1 = -9.1 * D2R;
        double ub_candidate_foc_2 = 170.9 * D2R;

        double yaw = 0.0;

        ColumnVector satvel = vsat;
        double satvel_norm = satvel.NormFrobenius();
        ColumnVector satcrd = xsat;
        double satcrd_norm = satcrd.NormFrobenius();
        double murate = (satvel_norm / satcrd_norm); // in radius

        double ub = 0.0;
        double du = 0.0;
        if (u >= ub_candidate_foc_1 && u <= ub_candidate_foc_2)
        {
            ub = ub_candidate_foc_1;
            du = u - ub_candidate_foc_1;
        }
        else
        {
            ub = ub_candidate_foc_2;
            if (u > 0)
            {
                du = u - ub_candidate_foc_2;
            }
            else
            {
                du = 2 * G_PI + u - ub_candidate_foc_2;
            }
        }

        if (fabs(beta) < 4.1 * D2R && fabs(du) < 18.2 * D2R)
        {
            double tmod = fabs(du) / murate;
            double beta0 = beta;
            double u0 = ub;
            double yangle0 = atan2(-tan(beta0), sin(u0));
            yaw = 0.5 * G_PI * sign(1, yangle0) + (yangle0 - 0.5 * G_PI * sign(1, yangle0)) * cos(fabs(2 * G_PI / 5656 * tmod));
        }
        else
        {
            yaw = yawangle;
        }
        _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
    }

    void t_gattitude_model::_attitude_GAL2(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta0 = 4.1 * D2R;
        double mi_n0 = (180 - 10) * D2R;
        double mi_m0 = 10 * D2R;

        if (fabs(satdata.orb_angle()) > mi_n0 && fabs(satdata.beta()) < beta0)
        {
            // noon maneuver
            _noon_turn_GAL2(satdata, i, j, k);
        }
        else if (fabs(satdata.orb_angle()) < mi_m0 && fabs(satdata.beta()) < beta0)
        {
            // midnight maneuver
            _noon_turn_GAL2(satdata, i, j, k);
        }
        else
        {
            _ysm(satdata, i, j, k);
            _last_yaw[satdata.sat()] = satdata.yaw();
            _last_epo[satdata.sat()] = satdata.epoch();
        }

    }

    void t_gattitude_model::_attitude_GAL2(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta0 = 4.1 * D2R;
        double mi_n0 = (180 - 10) * D2R;
        double mi_m0 = 10 * D2R;

        double orb_angle = _orb_angle(xsat, vsat, xsun);
        double beta = _beta(xsat, vsat, xsun);

        if (fabs(orb_angle) > mi_n0 && fabs(beta) < beta0)
        {
            // noon maneuver
            _noon_turn_GAL2(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
        else if (fabs(orb_angle) < mi_m0 && fabs(beta) < beta0)
        {
            // midnight maneuver
            _noon_turn_GAL2(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
        else
        {
            _ysm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
            _last_yaw[prn] = atan2(-beta, sin(orb_angle));
            //_last_epo[prn] = epoch;
        }
    }

    void t_gattitude_model::_cys_cast(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta_threshold_cast = 2.8;
        double d_constant = 80000.0;
        if (fabs(satdata.beta()) <= beta_threshold_cast * D2R)
        {
            double sinu = sin(satdata.orb_angle());
            double f_function = 1.0 / (1.0 + d_constant * (sinu) * (sinu) * (sinu) * (sinu));
            double beta_modified = satdata.beta() + f_function * (sign(beta_threshold_cast * D2R, satdata.beta()) - satdata.beta());
            double yaw = atan2(-tan(beta_modified), sinu);
            _yaw2ijk(satdata, yaw, i, j, k);
        }
        else
        {
            _ysm(satdata, i, j, k);
        }
    }

    void t_gattitude_model::_cys_cast(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta_threshold_cast = 2.8;
        double d_constant = 80000.0;

        double beta = _beta;
        double mi = _mi;
        if (fabs(beta) <= beta_threshold_cast * D2R)
        {
            double sinu = sin(mi);
            double f_function = 1.0 / (1.0 + d_constant * (sinu) * (sinu) * (sinu) * (sinu));
            double beta_modified = beta + f_function * (sign(beta_threshold_cast * D2R, beta) - beta);
            double yaw = atan2(-tan(beta_modified), sinu);
            _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
        }
        else
        {
            _ysm(prn, beta, mi, xsat, vsat, xsun, i, j, k);
        }
    }

    void t_gattitude_model::_cys_secm(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        double beta_threshold_secm = 3.0;
        if (fabs(satdata.beta()) <= beta_threshold_secm * D2R)
        {
            double sinu = sin(satdata.orb_angle());
            double Sox = -sinu * cos(satdata.beta());
            // double Soy = -sin(satdata.beta());
            double yaw = atan2(-sin(sign(beta_threshold_secm * D2R, satdata.beta())), -Sox);
            _yaw2ijk(satdata, yaw, i, j, k);
        }
        else
        {
            _ysm(satdata, i, j, k);
        }
    }

    void t_gattitude_model::_cys_secm(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double beta_threshold_secm = 3.0;

        double beta = _beta;
        double mi = _mi;

        if (fabs(beta) <= beta_threshold_secm * D2R)
        {
            double sinu = sin(mi);
            double Sox = -sinu * cos(beta);
            double yaw = atan2(-sin(sign(beta_threshold_secm * D2R, beta)), -Sox);
            _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
        }
        else
        {
            _ysm(prn, beta, mi, xsat, vsat, xsun, i, j, k);
        }
    }

    void t_gattitude_model::_attitude_BDS(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {

        if (t_gsys::bds_geo(satdata.sat()))
        {
            _onm(satdata, i, j, k);
        }
        // added by yqyuan for CYS mode of four BDS-2 and all BDS-3 CAST M/I satellites
        else if (t_gsys::bds_cast(satdata.sat()))
        {
            _cys_cast(satdata, i, j, k);
        }
        // added by yqyuan for CYS mode of 10 BDS-3 SECM M satellites
        else if (t_gsys::bds_secm(satdata.sat()))
        {
            _cys_secm(satdata, i, j, k);
        }
        // else using YS/ON switch mode
        else
        {
            if (fabs(satdata.beta()) <= 4.0 * D2R)
            { // could be updated; ref Dai X. 2015 and eclips_Dec2017.f by J. Kouba
                _onm(satdata, i, j, k);
            }
            else
                _ysm(satdata, i, j, k);
        }
    }

    void t_gattitude_model::_attitude_BDS(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double orb_angle = _orb_angle(xsat, vsat, xsun);
        double beta = _beta(xsat, vsat, xsun);

        if (t_gsys::bds_geo(prn))
        {
            _onm(xsat, vsat, xsun, i, j, k);
        }
        else if (t_gsys::bds_cast(prn))
        {
            _cys_cast(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
        else if (t_gsys::bds_secm(prn))
        {
            _cys_secm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
        else
        {
            if (fabs(beta) <= 4.0 * D2R)
            { // could be updated; ref Dai X. 2015 and eclips_Dec2017.f by J. Kouba
                _onm(xsat, vsat, xsun, i, j, k);
            }
            else
                _ysm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
    }

    void t_gattitude_model::_cys_qzs(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        // genernal settings and thresholds
        double beta_threshold_qzs = 5.0 * D2R; // radius
        double yrate = 0.055 * D2R;            // radius
        double us_noon_init = 173.0 * D2R;     // radius
        double ue_noon_init = 187.0 * D2R;     // radius
        double us_night_init = -7 * D2R;       // radius
        double ue_night_init = 7 * D2R;        // radius

        // loca vars. used in iters to compute us and ue
        double phi_us = 0.0;
        double phi_ue = 0.0;
        double yaw = 0.0;
        int ieclips = 0;
        double rotation_direction = 0.0;

        // sat_pos and sat_vel for murate
        t_gtriple satvel_t = satdata.satvel();
        Eigen::Vector3d satvel = satvel_t.crd_cvect_Eigen();
        double satvel_norm = satvel.norm();
        t_gtriple satcrd_t = satdata.satcrd();
        Eigen::Vector3d satcrd = satcrd_t.crd_cvect_Eigen();
        double satcrd_norm = satcrd.norm();
        double murate = (satvel_norm / satcrd_norm); // in radius
        // beta and u angle
        double beta = satdata.beta();   // in radius
        double u = satdata.orb_angle(); // in radius

        if (fabs(satdata.beta()) <= beta_threshold_qzs && murate / fabs(tan(beta)) >= yrate)
        {
            if (fabs(u <= 20 * D2R))
            { // mid-night turns
                double us_night = us_night_init;
                double ue_night = ue_night_init;
                // compute us and ue
                for (int iter_night = 1; iter_night <= 10; iter_night++)
                {
                    phi_us = atan2(tan(beta), -sin(us_night));
                    phi_ue = atan2(tan(beta), -sin(ue_night));
                    us_night = murate / yrate * (fabs(phi_us) - G_PI / 2);
                    ue_night = murate / yrate * (fabs(phi_ue) - G_PI / 2);
                }
                if ((u >= us_night) && (u <= ue_night))
                {
                    ieclips = 1;
                    phi_us = atan2(tan(beta), -sin(us_night));
                    phi_ue = atan2(tan(beta), -sin(ue_night));
                    rotation_direction = sign(1.0, phi_ue - phi_us);
                    yaw = phi_us + rotation_direction * yrate * (u - us_night) / (murate);
                }
            }
            if (fabs(u >= 160 * D2R))
            { // noon turns
                double us_noon = us_noon_init;
                double ue_noon = ue_noon_init;
                for (int iter_noon = 1; iter_noon <= 10; iter_noon++)
                {
                    phi_us = atan2(tan(beta), -sin(us_noon));
                    phi_ue = atan2(tan(beta), -sin(ue_noon));
                    us_noon = murate / yrate * (fabs(phi_us) - G_PI / 2) + G_PI;
                    ue_noon = murate / yrate * (fabs(phi_ue) - G_PI / 2) + G_PI;
                }
                if (u >= 0)
                {
                    if ((u >= us_noon) && (u <= ue_noon))
                    {
                        ieclips = 1;
                        phi_us = atan2(tan(beta), -sin(us_noon));
                        phi_ue = atan2(tan(beta), -sin(ue_noon));
                        rotation_direction = sign(1.0, phi_ue - phi_us);
                        yaw = phi_us + rotation_direction * yrate * (u - us_noon) / (murate);
                    }
                }
                else
                {
                    if ((u + 2 * G_PI >= us_noon) && (u + 2 * G_PI <= ue_noon))
                    {
                        ieclips = 1;
                        phi_us = atan2(tan(beta), -sin(us_noon));
                        phi_ue = atan2(tan(beta), -sin(ue_noon));
                        rotation_direction = sign(1.0, phi_ue - phi_us);
                        yaw = phi_us + rotation_direction * yrate * (u + 2 * G_PI - us_noon) / (murate);
                    }
                }
            }
            if (ieclips == 1)
            {
                _yaw2ijk(satdata, yaw, i, j, k);
            }
        }
        else
        {
            _ysm(satdata, i, j, k);
        }
    }

    void t_gattitude_model::_cys_qzs(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        // genernal settings and thresholds
        double beta_threshold_qzs = 5.0 * D2R; // radius
        double yrate = 0.055 * D2R;            // radius
        double us_noon_init = 173.0 * D2R;     // radius
        double ue_noon_init = 187.0 * D2R;     // radius
        double us_night_init = -7 * D2R;       // radius
        double ue_night_init = 7 * D2R;        // radius

        // loca vars. used in iters to compute us and ue
        double phi_us = 0.0;
        double phi_ue = 0.0;
        double yaw = 0.0;
        // int ieclips = 0;
        double rotation_direction = 0.0;

        // sat_pos and sat_vel for murate
        ColumnVector satvel = vsat;
        double satvel_norm = satvel.NormFrobenius();
        ColumnVector satcrd = xsat;
        double satcrd_norm = satcrd.NormFrobenius();
        double murate = (satvel_norm / satcrd_norm); // in radius
                                                     // beta and u angle
        double beta = _beta;
        double u = _mi;

        yaw = atan2(tan(beta), -sin(u)); // in QZS SCF

        if (fabs(beta) <= beta_threshold_qzs && murate / fabs(tan(beta)) >= yrate)
        {
            if (fabs(u <= 20 * D2R))
            { // mid-night turns
                double us_night = us_night_init;
                double ue_night = ue_night_init;
                // compute us and ue
                for (int iter_night = 1; iter_night <= 10; iter_night++)
                {
                    phi_us = atan2(tan(beta), -sin(us_night)); // in QZS SCF
                    phi_ue = atan2(tan(beta), -sin(ue_night)); // in QZS SCF
                    us_night = murate / yrate * (fabs(phi_us) - G_PI / 2);
                    ue_night = murate / yrate * (fabs(phi_ue) - G_PI / 2);
                }
                if ((u >= us_night) && (u <= ue_night))
                {
                    // ieclips = 1;
                    phi_us = atan2(tan(beta), -sin(us_night)); // in QZS SCF
                    phi_ue = atan2(tan(beta), -sin(ue_night)); // in QZS SCF
                    rotation_direction = sign(1.0, phi_ue - phi_us);
                    yaw = phi_us + rotation_direction * yrate * (u - us_night) / (murate); // in QZS SCF
                }
            }
            if (fabs(u >= 160 * D2R))
            { // noon turns
                double us_noon = us_noon_init;
                double ue_noon = ue_noon_init;
                for (int iter_noon = 1; iter_noon <= 10; iter_noon++)
                {
                    phi_us = atan2(tan(beta), -sin(us_noon)); // in QZS SCF
                    phi_ue = atan2(tan(beta), -sin(ue_noon)); // in QZS SCF
                    us_noon = murate / yrate * (fabs(phi_us) - G_PI / 2) + G_PI;
                    ue_noon = murate / yrate * (fabs(phi_ue) - G_PI / 2) + G_PI;
                }
                if (u >= 0)
                {
                    if ((u >= us_noon) && (u <= ue_noon))
                    {
                        // ieclips = 1;
                        phi_us = atan2(tan(beta), -sin(us_noon)); // in QZS SCF
                        phi_ue = atan2(tan(beta), -sin(ue_noon)); // in QZS SCF
                        rotation_direction = sign(1.0, phi_ue - phi_us);
                        yaw = phi_us + rotation_direction * yrate * (u - us_noon) / (murate); // in QZS SCF
                    }
                }
                else
                {
                    if ((u + 2 * G_PI >= us_noon) && (u + 2 * G_PI <= ue_noon))
                    {
                        // ieclips = 1;
                        phi_us = atan2(tan(beta), -sin(us_noon)); // in QZS SCF
                        phi_ue = atan2(tan(beta), -sin(ue_noon)); // in QZS SCF
                        rotation_direction = sign(1.0, phi_ue - phi_us);
                        yaw = phi_us + rotation_direction * yrate * (u + 2 * G_PI - us_noon) / (murate); // in QZS SCF
                    }
                }
            }
        }
        // QZS SCF to IGS SCF
        if (beta > 0)
        {
            // yaw_qzsscf > 0 and yaw_igsscf < 0
            yaw -= G_PI;
        }
        else
        {
            // yaw_qzsscf < 0 and yaw_igsscf > 0
            yaw += G_PI;
        }
        _yaw2ijk(xsat, vsat, xsun, yaw, i, j, k);
    }

    void t_gattitude_model::_attitude_QZS(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        if (satdata.sat().find("J01") != string::npos)
        {
            if (fabs(satdata.beta()) <= 20 * D2R)
            {                           // may be better 17 or 18 deg; to be investegated; yqyuan.
                _onm(satdata, i, j, k); // i oposite velocity direction //wrong comment. i == velocity direction
                                        // bug fixed.(yqyuan) _yaw2ijk calculate scf2crs using Xsat and Vsat (in IGS convention); independet of SCF oriention.
                                        //i = -1 * i;
                                        //j = -1 * j;
            }
            else
                _ysm(satdata, i, j, k);
        }
        // GEO satellite following ON mode, added by yqyuan
        else if (satdata.sat().find("J07") != string::npos)
        {
            _onm(satdata, i, j, k);
        }
        // QZS-2 IGSO satellites follows CYS mode, added by yqyuan
        else
        {
            _cys_qzs(satdata, i, j, k);
        }
    }

    void t_gattitude_model::_attitude_QZS(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        double orb_angle = _orb_angle(xsat, vsat, xsun);
        double beta = _beta(xsat, vsat, xsun);

        if (prn.find("J01") != string::npos)
        {
            if (fabs(beta) <= 20 * D2R)
            { // may be better 17 or 18 deg; to be investegated; yqyuan.
                _onm(xsat, vsat, xsun, i, j, k);
            }
            else
                _ysm(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
        // GEO satellite following ON mode, added by yqyuan
        else if (prn.find("J07") != string::npos)
        {
            _onm(xsat, vsat, xsun, i, j, k);
        }
        // QZS-2 IGSO satellites follows CYS mode, added by yqyuan
        else
        {
            _cys_qzs(prn, beta, orb_angle, xsat, vsat, xsun, i, j, k);
        }
    }

    void t_gattitude_model::_yaw2ijk(t_gsatdata &satdata, double &yaw, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k)
    {
        satdata.yaw(yaw); // store yaw angle

        Eigen::Vector3d satcrd = satdata.satcrd().crd_cvect_Eigen();
        Eigen::Vector3d satvel = satdata.satvel().crd_cvect_Eigen();

        if (satcrd.norm() == 0 || satvel.norm() == 0)
        {
            return;
        }

        // ITRF -> ICRF velocity
        satvel(0) -= OMEGA * satcrd(1);
        satvel(1) += OMEGA * satcrd(0);

        Eigen::Vector3d n = satcrd.cross(satvel);

        // Satelite-Earth unit vector
        k = -satcrd;
        k /= k.norm();

        Eigen::Vector3d ex = n.cross(satcrd);

        ex /= ex.norm();
        n /= n.norm();

        double cosy = cos(yaw);
        double siny = sin(yaw);
        for (int r = 0; r <= 2; r++)
        {
            i(r) = -siny * n(r) + cosy * ex(r);
            j(r) = -cosy * n(r) - siny * ex(r);
        }
    }

    void t_gattitude_model::_yaw2ijk(ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double &yaw, ColumnVector &i, ColumnVector &j, ColumnVector &k)
    {
        //satdata.yaw(yaw);   // store yaw angle
        ColumnVector satcrd = xsat;
        ColumnVector satvel = vsat;

        if (satcrd.NormFrobenius() == 0 || satvel.NormFrobenius() == 0)
        {
            return;
        }

        // ITRF -> ICRF velocity
        satvel(1) -= OMEGA * satcrd(2);
        satvel(2) += OMEGA * satcrd(1);

        ColumnVector n = crossproduct(satcrd, satvel);

        // Satelite-Earth unit vector
        k = -satcrd;
        k /= k.NormFrobenius();

        ColumnVector ex = crossproduct(n, satcrd);

        ex /= ex.NormFrobenius();
        n /= n.NormFrobenius();

        double cosy = cos(yaw);
        double siny = sin(yaw);
        for (int r = 1; r <= 3; r++)
        {
            i(r) = -siny * n(r) + cosy * ex(r);
            j(r) = -cosy * n(r) - siny * ex(r);
        }
    }

    double t_gattitude_model::sign(double a, double b)
    {
        double value = fabs(a);
        int sign_of_b = 1;
        if (b >= 0)
        {
            sign_of_b = 1;
        }
        else
        {
            sign_of_b = -1;
        }
        return value * sign_of_b;
    }

}
