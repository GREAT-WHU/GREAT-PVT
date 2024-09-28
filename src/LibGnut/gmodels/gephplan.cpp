
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include "gutils/gmatrixconv.h"
#include "gutils/gtypeconv.h"
#include "gmodels/gephplan.h"

using namespace std;

namespace gnut
{
    t_gephplan::t_gephplan()
    {
    }

    t_gephplan::~t_gephplan()
    {
    }

    t_gtriple t_gephplan::sunPos(double mjd, bool itrf)
    {
        auto find_result = _record_itrf_sunpos.find(mjd);
        if (itrf && find_result != _record_itrf_sunpos.end())
        {
            return find_result->second;
        }

        t_gtriple ret;

        const double T = (mjd - MJD_J2000) / 36525.0;
        const double eps = (23.43929111 - 0.0130042 * T) * D2R;

        double M = 2.0 * G_PI * _frac(0.9931267 + 99.9973583 * T);
        double L = 2.0 * G_PI * _frac(0.7859444 + M / 2.0 / G_PI + (6892.0 * sin(M) + 72.0 * sin(2.0 * M)) / 1296.0e3);
        double r = 149.619e9 - 2.499e9 * cos(M) - 0.021e9 * cos(2 * M);

        ColumnVector r_Sun(3);
        r_Sun << r * cos(L) << r * sin(L) << 0.0;
        r_Sun = rotX(-eps) * r_Sun;

        ColumnVector tmp;

        if (itrf)
            tmp = rotZ(gmst(mjd)) * _eop.nutMatrix(mjd) * _eop.precMatrix(mjd) * r_Sun;
        else
            tmp = r_Sun;

        for (int i = 0; i < 3; i++)
            ret[i] = tmp(i + 1);

        if (itrf)
            _record_itrf_sunpos[mjd] = ret;
        return ret;
    }

    t_gtriple t_gephplan::moonPos(double mjd)
    {
        t_gtriple ret;

        const double eps = 23.43929111 * D2R;
        const double T = (mjd - MJD_J2000) / 36525.0;

        double L_0 = _frac(0.606433 + 1336.851344 * T);
        double l = 2.0 * G_PI * _frac(0.374897 + 1325.552410 * T);
        double lp = 2.0 * G_PI * _frac(0.993133 + 99.997361 * T);
        double D = 2.0 * G_PI * _frac(0.827361 + 1236.853086 * T);
        double F = 2.0 * G_PI * _frac(0.259086 + 1342.227825 * T);

        double dL = +22640 * sin(l) - 4586 * sin(l - 2 * D) + 2370 * sin(2 * D) + 769 * sin(2 * l) - 668 * sin(lp) - 412 * sin(2 * F) - 212 * sin(2 * l - 2 * D) - 206 * sin(l + lp - 2 * D) + 192 * sin(l + 2 * D) - 165 * sin(lp - 2 * D) - 125 * sin(D) - 110 * sin(l + lp) + 148 * sin(l - lp) - 55 * sin(2 * F - 2 * D);

        double L = 2.0 * G_PI * _frac(L_0 + dL / 1296.0e3);

        double S = F + (dL + 412 * sin(2 * F) + 541 * sin(lp)) / RAD2SEC;
        double h = F - 2 * D;
        double N = -526 * sin(h) + 44 * sin(l + h) - 31 * sin(-l + h) - 23 * sin(lp + h) + 11 * sin(-lp + h) - 25 * sin(-2 * l + F) + 21 * sin(-l + F);

        double B = (18520.0 * sin(S) + N) / RAD2SEC;

        double cosB = cos(B);

        double R = 385000e3 - 20905e3 * cos(l) - 3699e3 * cos(2 * D - l) - 2956e3 * cos(2 * D) - 570e3 * cos(2 * l) + 246e3 * cos(2 * l - 2 * D) - 205e3 * cos(lp - 2 * D) - 171e3 * cos(l + 2 * D) - 152e3 * cos(l + lp - 2 * D);

        ColumnVector r_Moon(3);
        r_Moon << R * cos(L) * cosB << R * sin(L) * cosB << R * sin(B);
        r_Moon = rotX(-eps) * r_Moon;

        ColumnVector tmp = rotZ(gmst(mjd)) * _eop.nutMatrix(mjd) * _eop.precMatrix(mjd) * r_Moon;

        for (int i = 0; i < 3; i++)
            ret[i] = tmp(i + 1);

        return ret;
    }

    double t_gephplan::gmst(double mjd)
    {
        const double Secs = 86400.0;

        double Mjd_0 = floor(mjd);
        double UT1 = Secs * (mjd - Mjd_0);
        double T_0 = (Mjd_0 - MJD_J2000) / 36525.0;
        double T = (mjd - MJD_J2000) / 36525.0;

        double gmst = 24110.54841 + 8640184.812866 * T_0 + 1.002737909350795 * UT1 + (0.093104 - 6.2e-6 * T) * T * T;

        return 2.0 * G_PI * _frac(gmst / Secs);
    }

    double t_gephplan::_frac(double x)
    {
        return x - floor(x);
    }

} // namespace
