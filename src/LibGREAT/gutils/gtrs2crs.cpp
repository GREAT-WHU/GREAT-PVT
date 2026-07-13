/**
 * @file         gtrs2crs.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        calculate the rotation matrix from TRS to CRS
 * @version      1.0
 * @date         2024-08-29
 *
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 *
 */
#include "gutils/gtrs2crs.h"
#include "sofa.h"

#include <vector>

using namespace std;

namespace great
{
    namespace
    {
        constexpr double SOFA_DJM0 = 2400000.5;

    } // namespace

    t_pudaily::t_pudaily()
    {
        xpole = 0.0;
        ypole = 0.0;
        ut1 = 0.0;
    }

    t_zonaltide::t_zonaltide()
    {
        ut1 = 0.0;
    }

    t_gtrs2crs::t_gtrs2crs() :
        _poleut1(nullptr)
    {
        _cver = "06";
    }

    t_gtrs2crs::t_gtrs2crs(string cver)
    {
        _cver = cver;
    }

    t_gtrs2crs::t_gtrs2crs(bool erptab, t_gpoleut1* poleut1)
    {
        _erptab = erptab;
        _poleut1 = poleut1;
        _cver = "06";
    }

    t_gtrs2crs::t_gtrs2crs(bool erptab, t_gpoleut1* poleut1, string cver)
    {
        _erptab = erptab;
        _poleut1 = poleut1;
        _cver = cver;
    }

    t_gtrs2crs& t_gtrs2crs::operator=(const t_gtrs2crs& Other)
    {
        _erptab = Other._erptab;
        _poleut1 = Other._poleut1;
        _tdt = Other._tdt;
        _xpole = Other._xpole;
        _ypole = Other._ypole;
        _rotmat = Other._rotmat;

        return *this;
    }

    // return the values needed by other functions
    Matrix& t_gtrs2crs::getRotMat()
    {
        return _rotmat;
    };
    double t_gtrs2crs::getXpole()
    {
        return _xpole;
    };
    double t_gtrs2crs::getYpole()
    {
        return _ypole;
    };

    void t_gtrs2crs::calcRotMat(const t_gtime& epoch)
    {
        double dUt1_tai = 0.0;
        double dXhelp[3] = {0.0};

        _tdt = epoch;
        _xpole = 0.0;
        _ypole = 0.0;
        if (!_erptab)
        {
            _calPoleut1(_tdt, dXhelp, _poleut1);

            _xpole = dXhelp[0] / RAD2SEC;
            _ypole = dXhelp[1] / RAD2SEC;
            dUt1_tai = dXhelp[2];
        }

        t_gtime sTUT1;
        sTUT1.from_mjd(_tdt.mjd(), int(_tdt.sod() + _tdt.dsec() + (dUt1_tai - 32.184)), (_tdt.sod() + _tdt.dsec() + (dUt1_tai - 32.184)) - int(_tdt.sod() + _tdt.dsec() + (dUt1_tai - 32.184)));

        double rc2t[3][3];
        double tt = (_tdt.sod() + _tdt.dsec()) / 86400.0;
        double ut1 = (sTUT1.sod() + sTUT1.dsec()) / 86400.0;
        if (_cver.find("00") != string::npos)
        {
            iauC2t00a(SOFA_DJM0 + _tdt.mjd(), tt, SOFA_DJM0 + sTUT1.mjd(), ut1, _xpole, _ypole, rc2t);
        }
        else
        {
            iauC2t06a(SOFA_DJM0 + _tdt.mjd(), tt, SOFA_DJM0 + sTUT1.mjd(), ut1, _xpole, _ypole, rc2t);
        }

        _rotmat.ReSize(3, 3);
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                _rotmat(i + 1, j + 1) = rc2t[j][i];
            }
        }

        _xpole = _xpole * RAD2SEC;
        _ypole = _ypole * RAD2SEC;
    }

    void t_gtrs2crs::_calPoleut1(t_gtime& t, double* x, t_gpoleut1* poleut1)
    {
        int i;
        double dutlzonaltide, alpha;
        t_gtriple xyu;
        t_gtime t0;
        t0.from_mjd(_tdt.mjd(), 0, 0.0);

        double dut1_zonaltide = 0.0;
        t_gtime t1;
        t1.from_mjd(_tdt.mjd() + 1, 0, 0.0);

        vector<t_pudaily> rec;
        t_pudaily rec0, rec1;
        rec0.time = t0;
        rec1.time = t1;
        map<string, double> data0, data1;
        data0 = (*(poleut1->getPoleUt1DataMap()))[rec0.time];
        rec0.xpole = data0["XPOLE"];
        rec0.ypole = data0["YPOLE"];
        rec0.ut1 = data0["UT1-TAI"];
        rec.push_back(rec0);
        data1 = (*(poleut1->getPoleUt1DataMap()))[rec1.time];
        rec1.xpole = data1["XPOLE"];
        rec1.ypole = data1["YPOLE"];
        rec1.ut1 = data1["UT1-TAI"];
        rec.push_back(rec1);

        for (i = 0; i < 2; i++)
        {
            if ((*poleut1).getUt1Mode() != "UT1R")
            {
                _tide_corrections(rec[i].time, xyu);
                dutlzonaltide = _tideCor2(rec[i].time.mjd());
            }
            rec[i].xpole = rec[i].xpole - xyu[0];
            rec[i].ypole = rec[i].ypole - xyu[1];
            rec[i].ut1 = rec[i].ut1 - xyu[2] - dut1_zonaltide;
        }
        x[0] = 0;
        x[1] = 0;
        x[2] = 0;
        alpha = (t.dmjd() - rec[0].time.mjd()) / (*poleut1).getIntv();
        x[0] = rec[0].xpole + alpha * (rec[1].xpole - rec[0].xpole);
        x[1] = rec[0].ypole + alpha * (rec[1].ypole - rec[0].ypole);
        x[2] = rec[0].ut1 + alpha * (rec[1].ut1 - rec[0].ut1);
        _tide_corrections(t, xyu);
        dutlzonaltide = _tideCor2(t.dmjd());
        x[0] = x[0] + xyu[0];
        x[1] = x[1] + xyu[1];
        x[2] = x[2] + xyu[2] + dutlzonaltide;
    }

    void t_gtrs2crs::_tide_corrections(t_gtime& t, t_gtriple& xyu)
    {
        double stepsize = 0.015 * 86400.0; // unit sec
        double rmjd = t.dmjd();
        static bool isfirst = true;
        static t_pudaily tb0, tb1;
        if (isfirst)
        {
            tb0.time = LAST_TIME;
            tb1.time = FIRST_TIME;
            isfirst = false;
        }

        while (t > tb1.time || t < tb0.time)
        {
            if (t > (tb1.time + stepsize) || t < (tb0.time - stepsize))
            {
                tb0.time = t - stepsize;
                tb1.time = t + stepsize;

                tb0 = _tideCor1Cal(tb0);
                tb1 = _tideCor1Cal(tb1);
            }

            else if (t < tb0.time)
            {
                tb1 = tb0;
                tb0.time = tb0.time - stepsize;
                tb0 = _tideCor1Cal(tb0);
            }
            else
            {
                tb0 = tb1;
                tb1.time = tb1.time + stepsize;
                tb1 = _tideCor1Cal(tb1);
            }
        }

        double mjds[2] = {tb0.time.dmjd(), tb1.time.dmjd()};
        double xpoles[2] = {tb0.xpole, tb1.xpole};
        double ypoles[2] = {tb0.ypole, tb1.ypole};
        double ut1s[2] = {tb0.ut1, tb1.ut1};

        xyu[0] = _interpolation(1, 2, mjds, xpoles, rmjd);
        xyu[1] = _interpolation(1, 2, mjds, ypoles, rmjd);
        xyu[2] = _interpolation(1, 2, mjds, ut1s, rmjd);

        return;
    }

    t_pudaily& t_gtrs2crs::_tideCor1Cal(t_pudaily& b)
    {
        t_gtriple eop;
        _PMUT1_OCEANS(b.time, eop);
        b.xpole = eop[0];
        b.ypole = eop[1];
        b.ut1 = eop[2];

        double xtemp[2];
        _PMSDNUT2(b.time, xtemp);
        b.xpole = b.xpole + xtemp[0];
        b.ypole = b.ypole + xtemp[1];

        _UTLIBR(b.time, xtemp);
        b.ut1 = b.ut1 + xtemp[0];

        b.xpole = b.xpole * 1e-6;
        b.ypole = b.ypole * 1e-6;
        b.ut1 = b.ut1 * 1e-6;

        return b;
    }

    void t_gtrs2crs::_PMUT1_OCEANS(t_gtime& t, t_gtriple& eop)
    {
        double table[71][12] = {{1.0, -1.0, 0.0, -2.0, -2.0, -2.0, -0.05, 0.94, -0.94, -0.05, 0.396, -0.078},    {1.0, -2.0, 0.0, -2.0, 0.0, -1.0, 0.06, 0.64, -0.64, 0.06, 0.195, -0.059},
                                {1.0, -2.0, 0.0, -2.0, 0.0, -2.0, 0.30, 3.42, -3.42, 0.30, 1.034, -0.314},       {1.0, 0.0, 0.0, -2.0, -2.0, -1.0, 0.08, 0.78, -0.78, 0.08, 0.224, -0.073},
                                {1.0, 0.0, 0.0, -2.0, -2.0, -2.0, 0.46, 4.15, -4.15, 0.45, 1.187, -0.387},       {1.0, -1.0, 0.0, -2.0, 0.0, -1.0, 1.19, 4.96, -4.96, 1.19, 0.966, -0.474},
                                {1.0, -1.0, 0.0, -2.0, 0.0, -2.0, 6.24, 26.31, -26.31, 6.23, 5.118, -2.499},     {1.0, 1.0, 0.0, -2.0, -2.0, -1.0, 0.24, 0.94, -0.94, 0.24, 0.172, -0.090},
                                {1.0, 1.0, 0.0, -2.0, -2.0, -2.0, 1.28, 4.99, -4.99, 1.28, 0.911, -0.475},       {1.0, 0.0, 0.0, -2.0, 0.0, 0.0, -0.28, -0.77, 0.77, -0.28, -0.093, 0.070},
                                {1.0, 0.0, 0.0, -2.0, 0.0, -1.0, 9.22, 25.06, -25.06, 9.22, 3.025, -2.280},      {1.0, 0.0, 0.0, -2.0, 0.0, -2.0, 48.82, 132.91, -132.90, 48.82, 16.020, -12.069},
                                {1.0, -2.0, 0.0, 0.0, 0.0, 0.0, -0.32, -0.86, 0.86, -0.32, -0.103, 0.078},       {1.0, 0.0, 0.0, 0.0, -2.0, 0.0, -0.66, -1.72, 1.72, -0.66, -0.194, 0.154},
                                {1.0, -1.0, 0.0, -2.0, 2.0, -2.0, -0.42, -0.92, 0.92, -0.42, -0.083, 0.074},     {1.0, 1.0, 0.0, -2.0, 0.0, -1.0, -0.30, -0.64, 0.64, -0.30, -0.057, 0.050},
                                {1.0, 1.0, 0.0, -2.0, 0.0, -2.0, -1.61, -3.46, 3.46, -1.61, -0.308, 0.271},      {1.0, -1.0, 0.0, 0.0, 0.0, 0.0, -4.48, -9.61, 9.61, -4.48, -0.856, 0.751},
                                {1.0, -1.0, 0.0, 0.0, 0.0, -1.0, -0.90, -1.93, 1.93, -0.90, -0.172, 0.151},      {1.0, 1.0, 0.0, 0.0, -2.0, 0.0, -0.86, -1.81, 1.81, -0.86, -0.161, 0.137},
                                {1.0, 0.0, -1.0, -2.0, 2.0, -2.0, 1.54, 3.03, -3.03, 1.54, 0.315, -0.189},       {1.0, 0.0, 0.0, -2.0, 2.0, -1.0, -0.29, -0.58, 0.58, -0.29, -0.062, 0.035},
                                {1.0, 0.0, 0.0, -2.0, 2.0, -2.0, 26.13, 51.25, -51.25, 26.13, 5.512, -3.095},    {1.0, 0.0, 1.0, -2.0, 2.0, -2.0, -0.22, -0.42, 0.42, -0.22, -0.047, 0.025},
                                {1.0, 0.0, -1.0, 0.0, 0.0, 0.0, -0.61, -1.20, 1.20, -0.61, -0.134, 0.070},       {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.54, 3.00, -3.00, 1.54, 0.348, -0.171},
                                {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, -77.48, -151.74, 151.74, -77.48, -17.620, 8.548}, {1.0, 0.0, 0.0, 0.0, 0.0, -1.0, -10.52, -20.56, 20.56, -10.52, -2.392, 1.159},
                                {1.0, 0.0, 0.0, 0.0, 0.0, -2.0, 0.23, 0.44, -0.44, 0.23, 0.052, -0.025},         {1.0, 0.0, 1.0, 0.0, 0.0, 0.0, -0.61, -1.19, 1.19, -0.61, -0.144, 0.065},
                                {1.0, 0.0, 0.0, 2.0, -2.0, 2.0, -1.09, -2.11, 2.11, -1.09, -0.267, 0.111},       {1.0, -1.0, 0.0, 0.0, 2.0, 0.0, -0.69, -1.43, 1.43, -0.69, -0.288, 0.043},
                                {1.0, 1.0, 0.0, 0.0, 0.0, 0.0, -3.46, -7.28, 7.28, -3.46, -1.610, 0.187},        {1.0, 1.0, 0.0, 0.0, 0.0, -1.0, -0.69, -1.44, 1.44, -0.69, -0.320, 0.037},
                                {1.0, 0.0, 0.0, 0.0, 2.0, 0.0, -0.37, -1.06, 1.06, -0.37, -0.407, -0.005},       {1.0, 2.0, 0.0, 0.0, 0.0, 0.0, -0.17, -0.51, 0.51, -0.17, -0.213, -0.005},
                                {1.0, 0.0, 0.0, 2.0, 0.0, 2.0, -1.10, -3.42, 3.42, -1.09, -1.436, -0.037},       {1.0, 0.0, 0.0, 2.0, 0.0, 1.0, -0.70, -2.19, 2.19, -0.70, -0.921, -0.023},
                                {1.0, 0.0, 0.0, 2.0, 0.0, 0.0, -0.15, -0.46, 0.46, -0.15, -0.193, -0.005},       {1.0, 1.0, 0.0, 2.0, 0.0, 2.0, -0.03, -0.59, 0.59, -0.03, -0.396, -0.024},
                                {1.0, 1.0, 0.0, 2.0, 0.0, 1.0, -0.02, -0.38, 0.38, -0.02, -0.253, -0.015},       {2.0, -3.0, 0.0, -2.0, 0.0, -2.0, -0.49, -0.04, 0.63, 0.24, -0.089, -0.011},
                                {2.0, -1.0, 0.0, -2.0, -2.0, -2.0, -1.33, -0.17, 1.53, 0.68, -0.224, -0.032},    {2.0, -2.0, 0.0, -2.0, 0.0, -2.0, -6.08, -1.61, 3.13, 3.35, -0.637, -0.177},
                                {2.0, 0.0, 0.0, -2.0, -2.0, -2.0, -7.59, -2.05, 3.44, 4.23, -0.745, -0.222},     {2.0, 0.0, 1.0, -2.0, -2.0, -2.0, -0.52, -0.14, 0.22, 0.29, -0.049, -0.015},
                                {2.0, -1.0, -1.0, -2.0, 0.0, -2.0, 0.47, 0.11, -0.10, -0.27, 0.033, 0.013},      {2.0, -1.0, 0.0, -2.0, 0.0, -1.0, 2.12, 0.49, -0.41, -1.23, 0.141, 0.058},
                                {2.0, -1.0, 0.0, -2.0, 0.0, -2.0, -56.87, -12.93, 11.15, 32.88, -3.795, -1.556}, {2.0, -1.0, 1.0, -2.0, 0.0, -2.0, -0.54, -0.12, 0.10, 0.31, -0.035, -0.015},
                                {2.0, 1.0, 0.0, -2.0, -2.0, -2.0, -11.01, -2.40, 1.89, 6.41, -0.698, -0.298},    {2.0, 1.0, 1.0, -2.0, -2.0, -2.0, -0.51, -0.11, 0.08, 0.30, -0.032, -0.014},
                                {2.0, -2.0, 0.0, -2.0, 2.0, -2.0, 0.98, 0.11, -0.11, -0.58, 0.050, 0.022},       {2.0, 0.0, -1.0, -2.0, 0.0, -2.0, 1.13, 0.11, -0.13, -0.67, 0.056, 0.025},
                                {2.0, 0.0, 0.0, -2.0, 0.0, -1.0, 12.32, 1.00, -1.41, -7.31, 0.605, 0.266},       {2.0, 0.0, 0.0, -2.0, 0.0, -2.0, -330.15, -26.96, 37.58, 195.92, -16.195, -7.140},
                                {2.0, 0.0, 1.0, -2.0, 0.0, -2.0, -1.01, -0.07, 0.11, 0.60, -0.049, -0.021},      {2.0, -1.0, 0.0, -2.0, 2.0, -2.0, 2.47, -0.28, -0.44, -1.48, 0.111, 0.034},
                                {2.0, 1.0, 0.0, -2.0, 0.0, -2.0, 9.40, -1.44, -1.88, -5.65, 0.425, 0.117},       {2.0, -1.0, 0.0, 0.0, 0.0, 0.0, -2.35, 0.37, 0.47, 1.41, -0.106, -0.029},
                                {2.0, -1.0, 0.0, 0.0, 0.0, -1.0, -1.04, 0.17, 0.21, 0.62, -0.047, -0.013},       {2.0, 0.0, -1.0, -2.0, 2.0, -2.0, -8.51, 3.50, 3.29, 5.11, -0.437, -0.019},
                                {2.0, 0.0, 0.0, -2.0, 2.0, -2.0, -144.13, 63.56, 59.23, 86.56, -7.547, -0.159},  {2.0, 0.0, 1.0, -2.0, 2.0, -2.0, 1.19, -0.56, -0.52, -0.72, 0.064, 0.000},
                                {2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.49, -0.25, -0.23, -0.29, 0.027, -0.001},        {2.0, 0.0, 0.0, 0.0, 0.0, 0.0, -38.48, 19.14, 17.72, 23.11, -2.104, 0.041},
                                {2.0, 0.0, 0.0, 0.0, 0.0, -1.0, -11.44, 5.75, 5.32, 6.87, -0.627, 0.015},        {2.0, 0.0, 0.0, 0.0, 0.0, -2.0, -1.24, 0.63, 0.58, 0.75, -0.068, 0.002},
                                {2.0, 1.0, 0.0, 0.0, 0.0, 0.0, -1.77, 1.79, 1.71, 1.04, -0.146, 0.037},          {2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -0.77, 0.78, 0.75, 0.45, -0.064, 0.017},
                                {2.0, 0.0, 0.0, 2.0, 0.0, 2.0, -0.33, 0.62, 0.65, 0.19, -0.049, 0.018}};

        double dt = (t.dmjd() - dJ0) / dJC;

        double arg[6];
        arg[0] = fmod((67310.54841 + (876600.0 * 3600.0 + 8640184.812866) * dt + 0.093104 * dt * dt - 6.2e-6 * dt * dt * dt) * 15.0 + 648000.0, dTURNAS) * dAS2R;
        arg[1] = _iauFal03(dt);
        arg[2] = _iauFalp03(dt);
        arg[3] = _iauFaf03(dt);
        arg[4] = _iauFad03(dt);
        arg[5] = _iauFaom03(dt);

        double darg[6];
        darg[0] = (876600.0 * 3600.0 + 8640184.812866 + 2.0 * 0.093104 * dt - 3.0 * 6.2e-6 * dt * dt) * 15.0 * dAS2R / dJC;
        darg[1] = (-4.0 * 0.00024470 * dt * dt * dt + 3.0 * 0.051635 * dt * dt + 2.0 * 31.8792 * dt + 1717915923.2178) * dAS2R / dJC;
        darg[2] = (-4.0 * 0.00001149 * dt * dt * dt + 3.0 * 0.000136 * dt * dt - 2.0 * 0.5532 * dt + 129596581.0481) * dAS2R / dJC;
        darg[3] = (4.0 * 0.00000417 * dt * dt * dt - 3.0 * 0.001037 * dt * dt - 2.0 * 12.7512 * dt + 1739527262.8478) * dAS2R / dJC;
        darg[4] = (-4.0 * 0.00003169 * dt * dt * dt + 3.0 * 0.006593 * dt * dt - 2.0 * 6.3706 * dt + 1602961601.2090) * dAS2R / dJC;
        darg[5] = (-4.0 * 0.00005939 * dt * dt * dt + 3.0 * 0.007702 * dt * dt + 2.0 * 7.4722 * dt - 6962890.5431) * dAS2R / dJC;

        double cor_x = 0.0;
        double cor_y = 0.0;
        double cor_ut1 = 0.0;
        double cor_lod = 0.0;
        for (int j = 0; j <= 70; j++)
        {
            double ag = 0.0;
            double dag = 0.0;
            for (int i = 0; i <= 5; i++)
            {
                ag += table[j][i] * arg[i];
                dag += table[j][i] * darg[i];
            }
            ag = fmod(ag, d2PI);
            cor_x += table[j][7] * cos(ag) + table[j][6] * sin(ag);
            cor_y += table[j][9] * cos(ag) + table[j][8] * sin(ag);
            cor_ut1 += table[j][11] * cos(ag) + table[j][10] * sin(ag);
            cor_lod -= (table[j][11] * (-sin(ag)) + table[j][10] * cos(ag)) * dag;
        }
        eop[0] = cor_x;
        eop[1] = cor_y;
        eop[2] = cor_ut1;
    }

    void t_gtrs2crs::_PMSDNUT2(t_gtime& t, double* pm)
    {
        double table[10][10] = {{1.0, -1.0, 0.0, -2.0, 0.0, -1.0, -0.44, 0.25, -0.25, -0.44},
                                {1.0, -1.0, 0.0, -2.0, 0.0, -2.0, -2.31, 1.32, -1.32, -2.31},
                                {1.0, 1.0, 0.0, -2.0, -2.0, -2.0, -0.44, 0.25, -0.25, -0.44},
                                {1.0, 0.0, 0.0, -2.0, 0.0, -1.0, -2.14, 1.23, -1.23, -2.14},
                                {1.0, 0.0, 0.0, -2.0, 0.0, -2.0, -11.36, 6.52, -6.52, -11.36},
                                {1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.84, -0.48, 0.48, 0.84},
                                {1.0, 0.0, 0.0, -2.0, 2.0, -2.0, -4.76, 2.73, -2.73, -4.76},
                                {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 14.27, -8.19, 8.19, 14.27},
                                {1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 1.93, -1.11, 1.11, 1.93},
                                {1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.76, -0.43, 0.43, 0.76}};

        pm[0] = 0;
        pm[1] = 0;
        double dmjd = t.dmjd();
        double dt = (dmjd - dJ0) / dJC;
        double arg[6];
        arg[0] = fmod((67310.54841 + (876600.0 * 3600.0 + 8640184.812866) * dt + 0.093104 * dt * dt - 6.2e-6 * dt * dt * dt) * 15.0 + 648000.0, dTURNAS) * dAS2R;
        arg[1] = _iauFal03(dt);
        arg[2] = _iauFalp03(dt);
        arg[3] = _iauFaf03(dt);
        arg[4] = _iauFad03(dt);
        arg[5] = _iauFaom03(dt);

        for (int j = 0; j < 10; j++)
        {
            double angle = 0.0;
            for (int i = 0; i < 6; i++)
            {
                angle += table[j][i] * arg[i];
            }
            angle = fmod(angle, d2PI);
            pm[0] = pm[0] + table[j][6] * sin(angle) + table[j][7] * cos(angle);
            pm[1] = pm[1] + table[j][8] * sin(angle) + table[j][9] * cos(angle);
        }
        return;
    }

    void t_gtrs2crs::_UTLIBR(t_gtime& t, double* temp)
    {
        double table[11][10] = {{2.0, -2.0, 0.0, -2.0, 0.0, -2.0, 0.05, -0.03, -0.3, -0.6},
                                {2.0, 0.0, 0.0, -2.0, -2.0, -2.0, 0.06, -0.03, -0.4, -0.7},
                                {2.0, -1.0, 0.0, -2.0, 0.0, -2.0, 0.35, -0.20, -2.4, -4.2},
                                {2.0, 1.0, 0.0, -2.0, -2.0, -2.0, 0.07, -0.04, -0.5, -0.8},
                                {2.0, 0.0, 0.0, -2.0, 0.0, -1.0, -0.07, 0.04, 0.5, 0.8},
                                {2.0, 0.0, 0.0, -2.0, 0.0, -2.0, 1.75, -1.01, -12.2, -21.3},
                                {2.0, 1.0, 0.0, -2.0, 0.0, -2.0, -0.05, 0.03, 0.3, 0.6},
                                {2.0, 0.0, -1.0, -2.0, 2.0, -2.0, 0.05, -0.03, -0.3, -0.6},
                                {2.0, 0.0, 0.0, -2.0, 2.0, -2.0, 0.76, -0.44, -5.5, -9.5},
                                {2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.21, -0.12, -1.5, -2.6},
                                {2.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.06, -0.04, -0.4, -0.8}};
        double dt = (t.dmjd() - rmjd0) / 36525.0;
        double arg[6];
        arg[0] = fmod((67310.54841 + (876600.0 * 3600.0 + 8640184.812866) * dt + 0.093104 * dt * dt - 6.2e-6 * dt * dt * dt) * 15.0 + 648000.0, dTURNAS) * dAS2R;
        arg[1] = _iauFal03(dt);
        arg[2] = _iauFalp03(dt);
        arg[3] = _iauFaf03(dt);
        arg[4] = _iauFad03(dt);
        arg[5] = _iauFaom03(dt);
        temp[0] = 0;
        temp[1] = 0;
        for (int j = 0; j <= 10; j++)
        {
            double angle = 0;
            for (int i = 0; i <= 5; i++)
            {
                angle += table[j][i] * arg[i];
            }
            angle = fmod(angle, d2PI);
            temp[0] = temp[0] + table[j][6] * sin(angle) + table[j][7] * cos(angle);
            temp[1] = temp[1] + table[j][8] * sin(angle) + table[j][9] * cos(angle);
        }
        return;
    }

    double t_gtrs2crs::_tideCor2(const double& dRmjd)
    {
        static t_zonaltide sTZB[3];
        static double gdStepsize = 0.05;
        static bool isFirst = true;
        double dT;
        double pdUt1;
        if (isFirst)
        {
            sTZB[0].time = LAST_TIME;
            sTZB[2].time = FIRST_TIME;
            isFirst = false;
        }

        while ((dRmjd > sTZB[2].time.dmjd()) || (dRmjd < sTZB[0].time.dmjd()))
        {
            if ((dRmjd > sTZB[2].time.dmjd() + gdStepsize) || (dRmjd < sTZB[0].time.dmjd() - gdStepsize))
            {
                sTZB[0].time.from_mjd((int)(dRmjd - gdStepsize),
                                      (int)((dRmjd - gdStepsize - (int)(dRmjd - gdStepsize)) * 86400),
                                      ((dRmjd - gdStepsize - (int)(dRmjd - gdStepsize)) * 86400) - (int)(((dRmjd - gdStepsize - (int)(dRmjd - gdStepsize)) * 86400)));

                sTZB[1].time.from_mjd((int)(dRmjd), (int)((dRmjd - (int)dRmjd) * 86400), ((dRmjd - (int)dRmjd) * 86400) - (int)((dRmjd - (int)dRmjd) * 86400));

                sTZB[2].time.from_mjd((int)(dRmjd + gdStepsize),
                                      (int)((dRmjd + gdStepsize - (int)(dRmjd + gdStepsize)) * 86400),
                                      ((dRmjd + gdStepsize - (int)(dRmjd + gdStepsize)) * 86400) - (int)(((dRmjd + gdStepsize - (int)(dRmjd + gdStepsize)) * 86400)));
                for (int i = 0; i < 3; i++)
                {
                    dT = (sTZB[i].time.dmjd() - 51544.5) / 36525.0;
                    double lod = 0.0;
                    double omega = 0.0;
                    _RG_ZONT2(dT, &sTZB[i].ut1, &lod, &omega);
                }
            }

            else if (dRmjd < sTZB[0].time.dmjd())
            {
                sTZB[2] = sTZB[1];
                sTZB[1] = sTZB[0];
                sTZB[0].time.from_mjd((int)(sTZB[0].time.dmjd() - gdStepsize),
                                      (int)(((sTZB[0].time.dmjd() - gdStepsize) - (int)(sTZB[0].time.dmjd() - gdStepsize)) * 86400),
                                      (((sTZB[0].time.dmjd() - gdStepsize) - (int)(sTZB[0].time.dmjd() - gdStepsize)) * 86400) -
                                          (int)(((sTZB[0].time.dmjd() - gdStepsize) - (int)(sTZB[0].time.dmjd() - gdStepsize)) * 86400));
                dT = (sTZB[0].time.dmjd() - 51544.5) / 36525.0;
                double lod = 0.0;
                double omega = 0.0;
                _RG_ZONT2(dT, &sTZB[0].ut1, &lod, &omega);
            }

            else
            {
                sTZB[0] = sTZB[1];
                sTZB[1] = sTZB[2];

                sTZB[2].time.from_mjd((int)(sTZB[2].time.dmjd() + gdStepsize),
                                      (int)(((sTZB[2].time.dmjd() + gdStepsize) - (int)(sTZB[2].time.dmjd() + gdStepsize)) * 86400),
                                      (((sTZB[2].time.dmjd() + gdStepsize) - (int)(sTZB[2].time.dmjd() + gdStepsize)) * 86400) -
                                          (int)(((sTZB[2].time.dmjd() + gdStepsize) - (int)(sTZB[2].time.dmjd() + gdStepsize)) * 86400));
                dT = (sTZB[2].time.dmjd() - 51544.5) / 36525.0;
                double lod = 0.0;
                double omega = 0.0;
                _RG_ZONT2(dT, &sTZB[2].ut1, &lod, &omega);
            }
        }

        double dTemp1[3];
        double dTemp2[3];
        for (int i = 0; i < 3; i++)
        {
            dTemp1[i] = sTZB[i].time.dmjd();
            dTemp2[i] = sTZB[i].ut1;
        }

        pdUt1 = _interpolation(2, 3, dTemp1, dTemp2, dRmjd);
        return pdUt1;
    }

    void t_gtrs2crs::_RG_ZONT2(const double& dT, double* DUT, double* DLOD, double* DOMEGA)
    {
        int I;
        double dL;
        double dLP;
        double dF;
        double dD;
        double dOM;
        double dARG;

        /*  ----------------------
         *  Zonal Earth tide model
         *  ----------------------*/
        //*  Number of terms in the zonal Earth tide model
        //  INTEGER NZONT
        const int iNZONT = 62;
        //*  Coefficients for the fundamental arguments
        int iNFUND[5][iNZONT];
        //*  Zonal tide term coefficients
        double dTIDE[6][iNZONT];
        /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
         *  --------------------------------------------------
         *  Tables of multiples of arguments and coefficients
         *  --------------------------------------------------
         *  Luni-Solar argument multipliers*/

        int iNFUNDSUB1[20][5] = {1,  0, 2, 2, 2, 2, 0, 2, 0, 1, 2, 0, 2, 0,  2, 0, 0, 2, 2, 1, 0, 0, 2, 2, 2, 1, 0, 2, 0, 0, 1, 0, 2, 0, 1, 1, 0, 2, 0, 2,  3, 0, 0, 0, 0, -1, 0, 2, 2, 1,
                                 -1, 0, 2, 2, 2, 1, 0, 0, 2, 0, 2, 0, 2, -2, 2, 0, 1, 2, 0, 2, 0, 0, 2, 0, 0, 0, 0, 2, 0, 1, 0, 0, 2, 0, 2, 2, 0, 0, 0, -1, 2, 0, 0, 0, 0, 2,  0, 0, 0, 1};
        int i = 0;
        int j = 0;
        for (i = 0; i <= 19; i++)
        {
            for (j = 0; j <= 4; j++)
            {
                iNFUND[j][i] = iNFUNDSUB1[i][j];
            }
        }

        int iNFUNDSUB2[20][5] = {0,  -1, 2, 0, 2, 0, 0, 0, 2, -1, 0, 0, 0, 2, 0, 0, 0, 0, 2, 1, 0, -1, 0, 2, 0, 1, 0,  2, -2, 1, 1,  0, 2, -2, 2,  1,  1, 0, 0, 0, -1, 0, 2, 0, 0, -1, 0, 2,  0, 1,
                                 -1, 0,  2, 0, 2, 1, 0, 0, 0, -1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0,  0, 1, 0, 1, -1, 0, 0,  0, -1, 0, 0, 2,  -1, -1, 0, 0, 2, 0, -1, 0, 0, 2, 1, 1,  0, -2, 2, -1};
        for (i = 0; i <= 19; i++)
        {
            for (j = 0; j <= 4; j++)
            {
                iNFUND[j][i + 20] = iNFUNDSUB2[i][j];
            }
        }

        int iNFUNDSUB3[20][5] = {-1, -1, 0, 2, 0, 0, 2, 2, -2, 2,  0,  1, 2, -2, 1,  0,  1, 2, -2, 2,  0, 0, 2,  -2, 0,  0, 0, 2,  -2, 1, 0,  0, 2,  -2,
                                 2,  0,  2, 0, 0, 0, 2, 0, 0,  -2, -1, 2, 0, 0,  -2, 0,  2, 0, 0,  -2, 1, 0, -1, 2,  -2, 1, 0, 1,  0,  0, -1, 0, -1, 2,
                                 -2, 2,  0, 1, 0, 0, 0, 0, 1,  0,  0,  1, 1, 0,  0,  -1, 0, 2, 0,  -2, 0, 0, -2, 0,  2,  0, 1, -1, 1,  0, 1,  0};
        for (i = 0; i <= 19; i++)
        {
            for (j = 0; j <= 4; j++)
            {
                iNFUND[j][i + 40] = iNFUNDSUB3[i][j];
            }
        }

        int iNFUNDSUB4[2][5] = {0, 0, 0, 0, 2, 0, 0, 0, 0, 1};
        for (i = 0; i <= 1; i++)
        {
            for (j = 0; j <= 4; j++)
            {
                iNFUND[j][i + 60] = iNFUNDSUB4[i][j];
            }
        }

        double dTIDESUB1[20][6] = {-0.0235, 0.0000, 0.2617,  0.0000, -0.2209,  0.0000,  -0.0404, 0.0000, 0.3706,  0.0000, -0.3128,  0.0000,  -0.0987, 0.0000, 0.9041,  0.0000, -0.7630, 0.0000,
                                   -0.0508, 0.0000, 0.4499,  0.0000, -0.3797,  0.0000,  -0.1231, 0.0000, 1.0904,  0.0000, -0.9203,  0.0000,  -0.0385, 0.0000, 0.2659,  0.0000, -0.2244, 0.0000,
                                   -0.4108, 0.0000, 2.8298,  0.0000, -2.3884,  0.0000,  -0.9926, 0.0000, 6.8291,  0.0000, -5.7637,  0.0000,  -0.0179, 0.0000, 0.1222,  0.0000, -0.1031, 0.0000,
                                   -0.0818, 0.0000, 0.5384,  0.0000, -0.4544,  0.0000,  -0.1974, 0.0000, 1.2978,  0.0000, -1.0953,  0.0000,  -0.0761, 0.0000, 0.4976,  0.0000, -0.4200, 0.0000,
                                   0.0216,  0.0000, -0.1060, 0.0000, 0.0895,   0.0000,  0.0254,  0.0000, -0.1211, 0.0000, 0.1022,   0.0000,  -0.2989, 0.0000, 1.3804,  0.0000, -1.1650, 0.0000,
                                   -3.1873, 0.2010, 14.6890, 0.9266, -12.3974, -0.7820, -7.8468, 0.5320, 36.0910, 2.4469, -30.4606, -2.0652, 0.0216,  0.0000, -0.0988, 0.0000, 0.0834,  0.0000,
                                   -0.3384, 0.0000, 1.5433,  0.0000, -1.3025,  0.0000,  0.0179,  0.0000, -0.0813, 0.0000, 0.0686,   0.0000};
        for (i = 0; i <= 19; i++)
        {
            for (j = 0; j <= 5; j++)
            {
                dTIDE[j][i] = dTIDESUB1[i][j];
            }
        }

        double dTIDESUB2[20][6] = {-0.0244, 0.0000, 0.1082,  0.0000, -0.0913,  0.0000,  0.0470,  0.0000, -0.2004, 0.0000, 0.1692,  0.0000, -0.7341, 0.0000, 3.1240,  0.0000, -2.6367, 0.0000,
                                   -0.0526, 0.0000, 0.2235,  0.0000, -0.1886,  0.0000,  -0.0508, 0.0000, 0.2073,  0.0000, -0.1749, 0.0000, 0.0498,  0.0000, -0.1312, 0.0000, 0.1107,  0.0000,
                                   0.1006,  0.0000, -0.2640, 0.0000, 0.2228,   0.0000,  0.0395,  0.0000, -0.0968, 0.0000, 0.0817,  0.0000, 0.0470,  0.0000, -0.1099, 0.0000, 0.0927,  0.0000,
                                   0.1767,  0.0000, -0.4115, 0.0000, 0.3473,   0.0000,  0.4352,  0.0000, -1.0093, 0.0000, 0.8519,  0.0000, 0.5339,  0.0000, -1.2224, 0.0000, 1.0317,  0.0000,
                                   -8.4046, 0.2500, 19.1647, 0.5701, -16.1749, -0.4811, 0.5443,  0.0000, -1.2360, 0.0000, 1.0432,  0.0000, 0.0470,  0.0000, -0.1000, 0.0000, 0.0844,  0.0000,
                                   -0.0555, 0.0000, 0.1169,  0.0000, -0.0987,  0.0000,  0.1175,  0.0000, -0.2332, 0.0000, 0.1968,  0.0000, -1.8236, 0.0000, 3.6018,  0.0000, -3.0399, 0.0000,
                                   0.1316,  0.0000, -0.2587, 0.0000, 0.2183,   0.0000,  0.0179,  0.0000, -0.0344, 0.0000, 0.0290,  0.0000

        };
        for (i = 0; i <= 19; i++)
        {
            for (j = 0; j <= 5; j++)
            {
                dTIDE[j][i + 20] = dTIDESUB2[i][j];
            }
        }

        double dTIDESUB3[20][6] = {-0.0855,  0.0000, 0.1542,  0.0000, -0.1302,  0.0000,  -0.0573, 0.0000, 0.0395,  0.0000, -0.0333, 0.0000, 0.0329,   0.0000, -0.0173, 0.0000, 0.0146,  0.0000,
                                   -1.8847,  0.0000, 0.9726,  0.0000, -0.8209,  0.0000,  0.2510,  0.0000, -0.0910, 0.0000, 0.0768,  0.0000, 1.1703,   0.0000, -0.4135, 0.0000, 0.3490,  0.0000,
                                   -49.7174, 0.4330, 17.1056, 0.1490, -14.4370, -0.1257, -0.1936, 0.0000, 0.0666,  0.0000, -0.0562, 0.0000, 0.0489,   0.0000, -0.0154, 0.0000, 0.0130,  0.0000,
                                   -0.5471,  0.0000, 0.1670,  0.0000, -0.1409,  0.0000,  0.0367,  0.0000, -0.0108, 0.0000, 0.0092,  0.0000, -0.0451,  0.0000, 0.0082,  0.0000, -0.0069, 0.0000,
                                   0.0921,   0.0000, -0.0167, 0.0000, 0.0141,   0.0000,  0.8281,  0.0000, -0.1425, 0.0000, 0.1202,  0.0000, -15.8887, 0.1530, 2.7332,  0.0267, -2.3068, -0.0222,
                                   -0.1382,  0.0000, 0.0225,  0.0000, -0.0190,  0.0000,  0.0348,  0.0000, -0.0053, 0.0000, 0.0045,  0.0000, -0.1372,  0.0000, -0.0079, 0.0000, 0.0066,  0.0000,
                                   0.4211,   0.0000, -0.0203, 0.0000, 0.0171,   0.0000,  -0.0404, 0.0000, 0.0008,  0.0000, -0.0007, 0.0000};
        for (i = 0; i <= 19; i++)
        {
            for (j = 0; j <= 5; j++)
            {
                dTIDE[j][i + 40] = dTIDESUB3[i][j];
            }
        }

        double dTIDESUB4[2][6] = {7.8998, 0.0000, 0.1460, 0.0000, -0.1232, 0.0000, -1617.2681, 0.0000, -14.9471, 0.0000, 12.6153, 0.0000};
        for (i = 0; i <= 1; i++)
        {
            for (j = 0; j <= 5; j++)
            {
                dTIDE[j][i + 60] = dTIDESUB4[i][j];
            }
        }
        /*  -------------------------------------
         *   Computation of fundamental arguments
         *  -------------------------------------*/

        _FUNDARG(dT, &dL, &dLP, &dF, &dD, &dOM);

        /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
        // Set initial values to zero.
        *DUT = 0;
        *DLOD = 0;
        *DOMEGA = 0;

        //*  Sum zonal tide terms.
        for (I = 0; I <= 40; I++)
        {
            //*     Formation of multiples of arguments.
            dARG = fmod((iNFUND[0][I] * dL + iNFUND[1][I] * dLP + iNFUND[2][I] * dF + iNFUND[3][I] * dD + iNFUND[4][I] * dOM), d2PI);

            if (dARG < 0)
            {
                dARG = dARG + d2PI;
            }
            //*     Evaluate zonal tidal terms.
            *DUT = *DUT + dTIDE[0][I] * sin(dARG) + dTIDE[1][I] * cos(dARG);

            *DLOD = *DLOD + dTIDE[2][I] * cos(dARG) + dTIDE[3][I] * sin(dARG);

            *DOMEGA = *DOMEGA + dTIDE[4][I] * cos(dARG) + dTIDE[5][I] * sin(dARG);
        }
        //*  Rescale corrections so that they are in units involving seconds.

        *DUT = (*DUT) * 1.0E-4;
        *DLOD = (*DLOD) * 1.0E-5;
        *DOMEGA = (*DOMEGA) * 1.0E-14;
        return;
    }

    void t_gtrs2crs::_FUNDARG(const double& T, double* L, double* LP, double* F, double* D, double* OM)
    {
        if (L == NULL || LP == NULL || F == NULL || D == NULL || OM == NULL)
        {
            cout << "invalid FUNDARG pointer";
        }

        const double DAS2R = 4.848136811095359935899141e-6;
        const double TURNAS = 1296000;

        *L = fmod(485868.249036 + T * (1717915923.2178 + T * (31.8792 + T * (0.051635 + T * (-0.00024470)))), TURNAS) * DAS2R;
        *LP = fmod(1287104.79305 + T * (129596581.0481 + T * (-0.5532 + T * (0.000136 + T * (-0.00001149)))), TURNAS) * DAS2R;

        *F = fmod(335779.526232 + T * (1739527262.8478 + T * (-12.7512 + T * (-0.001037 + T * (0.00000417)))), TURNAS) * DAS2R;
        *D = fmod(1072260.70369 + T * (1602961601.2090 + T * (-6.3706 + T * (0.006593 + T * (-0.00003169)))), TURNAS) * DAS2R;
        *OM = fmod(450160.398036 + T * (-6962890.5431 + T * (7.4722 + T * (0.007702 + T * (-0.00005939)))), TURNAS) * DAS2R;
        return;
    }

    /** @brief Mean anomaly of the Moon. */
    double t_gtrs2crs::_iauFal03(const double& t)
    {
        /* Mean anomaly of the Moon (IERS Conventions 2003). */
        return fmod(485868.249036 + t * (1717915923.2178 + t * (31.8792 + t * (0.051635 + t * (-0.00024470)))), dTURNAS) * dAS2R;
    }

    /** @brief Mean anomaly of the Sun. */
    double t_gtrs2crs::_iauFalp03(const double& t)
    {
        /* Mean anomaly of the Sun (IERS Conventions 2003). */
        return fmod(1287104.793048 + t * (129596581.0481 + t * (-0.5532 + t * (0.000136 + t * (-0.00001149)))), dTURNAS) * dAS2R;
    }

    /** @brief Mean argument of the latitude of the Moon. */
    double t_gtrs2crs::_iauFaf03(const double& t)
    {
        /* Mean longitude of the Moon minus that of the ascending node */
        /* (IERS Conventions 2003).                                    */
        return fmod(335779.526232 + t * (1739527262.8478 + t * (-12.7512 + t * (-0.001037 + t * (0.00000417)))), dTURNAS) * dAS2R;
    }

    /** @brief Mean elongation of the Moon from the Sun. */
    double t_gtrs2crs::_iauFad03(const double& t)
    {
        /* Mean elongation of the Moon from the Sun (IERS Conventions 2003). */
        return fmod(1072260.703692 + t * (1602961601.2090 + t * (-6.3706 + t * (0.006593 + t * (-0.00003169)))), dTURNAS) * dAS2R;
    }

    /** @brief Mean longitude of the ascending node of the Moon. */
    double t_gtrs2crs::_iauFaom03(const double& t)
    {
        /* Mean longitude of the Moon's ascending node */
        /* (IERS Conventions 2003).                    */
        return fmod(450160.398036 + t * (-6962890.5431 + t * (7.4722 + t * (0.007702 + t * (-0.00005939)))), dTURNAS) * dAS2R;
    }

    /** @brief Planetary longitudes, Mercury. */
    double t_gtrs2crs::_iauFame03(const double& t)
    {
        /* Mean longitude of Mercury (IERS Conventions 2003). */
        return fmod(4.402608842 + 2608.7903141574 * t, d2PI);
    }

    /** @brief Planetary longitudes, Venus. */
    double t_gtrs2crs::_iauFave03(const double& t)
    {
        /* Mean longitude of Venus (IERS Conventions 2003). */
        return fmod(3.176146697 + 1021.3285546211 * t, d2PI);
    }

    /** @brief Planetary longitudes, Earth. */
    double t_gtrs2crs::_iauFae03(const double& t)
    {
        /* Mean longitude of Earth (IERS Conventions 2003). */
        return fmod(1.753470314 + 628.3075849991 * t, d2PI);
    }

    /** @brief Planetary longitudes, Mars. */
    double t_gtrs2crs::_iauFama03(const double& t)
    {
        /* Mean longitude of Mars (IERS Conventions 2003). */
        return fmod(6.203480913 + 334.0612426700 * t, d2PI);
    }

    /** @brief Planetary longitudes, Jupiter. */
    double t_gtrs2crs::_iauFaju03(const double& t)
    {
        /* Mean longitude of Jupiter (IERS Conventions 2003). */
        return fmod(0.599546497 + 52.9690962641 * t, d2PI);
    }

    /** @brief Planetary longitudes, Saturn. */
    double t_gtrs2crs::_iauFasa03(const double& t)
    {
        /* Mean longitude of Saturn (IERS Conventions 2003). */
        return fmod(0.874016757 + 21.3299104960 * t, d2PI);
    }

    /** @brief Planetary longitudes, Uranus. */
    double t_gtrs2crs::_iauFaur03(const double& t)
    {
        /* Mean longitude of Uranus (IERS Conventions 2003). */
        return fmod(5.481293872 + 7.4781598567 * t, d2PI);
    }

    /** @brief Planetary longitudes, Neptune. */
    double t_gtrs2crs::_iauFane03(const double& t)
    {
        /* Mean longitude of Neptune (IERS Conventions 2003). */
        return fmod(5.311886287 + 3.8133035638 * t, d2PI);
    }

    /** @brief General accumulated precession in longitude. */
    double t_gtrs2crs::_iauFapa03(const double& t)
    {
        /* General accumulated precession in longitude. */
        return (0.024381750 + 0.00000538691 * t) * t;
    }

    /***********************
    *FunctionName:nutation_interpolation
        *Function:
        *InPut:      dRmjd modified JD
        dStepsize stepsize in days, 0.d0 use the default 0.125
        *OutPut:dpi nutation angle of the longitude(radian)
        dep nutation angle of the obliquity(radian)
        *Return:
        *Other:
    ***********************/
/***********************
    *FunctionName:r_interpolation
    *Function:interpolation of linear of 2-order polynomials
    *InPut: iOrder 1 or 2 for linear or quadratic
    iPoint number of points in the input array (x,y) in case of long table (npoint > norder+1):
    *OutPut:pdX
    pdX
    dXin
    *Return:
    *Other:
    ***********************/
    double t_gtrs2crs::_interpolation(const int& iOrder, const int& iPoint, double* pdX, double* pdY, const double& dXin)
    {
        int i1;
        int i2;
        int i3;
        double dX;
        double dIntv;
        string strTemp1;
        string strTemp2;

        dIntv = (*(pdX + 1)) - (*(pdX));
        if ((dXin < (*(pdX)-0.1 * dIntv)) || (dXin > (*(pdX + iPoint - 1) + 0.1 * dIntv)))
        {
            cout << " ERROR: input variable out of the table (tbeg,tend,tinput): " << setw(10) << (*pdX) << *(pdX + iPoint - 1) << dXin << endl;
        }

        if (iPoint == iOrder + 1)
        {
            i1 = 1;
        }
        else if (iPoint > iOrder + 1)
        {
            i1 = (int)((dXin - *pdX) / dIntv) + 1;
            if (i1 < 1)
            {
                i1 = 1;
            }
            if ((iPoint - iOrder) < i1)
            {
                i1 = iPoint - iOrder;
            }
        }
        else
        {
            cout << " ERROR: table is enough long for interpolation: "
                 << " npoint = " << iPoint << " norder = " << iOrder << endl;
            throw(" ERROR: table is enough long for interpolation");
        }

        double dResult;
        if (iOrder == 1)
        {
            i2 = i1 + 1;
            dX = (dXin - *(pdX + i1 - 1)) / (*(pdX + i2 - 1) - *(pdX + i1 - 1));
            dResult = *(pdY + i1 - 1) + dX * (*(pdY + i2 - 1) - *(pdY + i1 - 1));
            return dResult;
        }
        else if (iOrder == 2)
        {
            i2 = i1 + 1;
            i3 = i2 + 1;
            dX = (dXin - *(pdX + i2 - 1)) / (*(pdX + i2 - 1) - *(pdX + i1 - 1));
            dResult = *(pdY + i2 - 1) + dX * (*(pdY + i3 - 1) - *(pdY + i1 - 1)) * 0.5 + dX * dX * ((*(pdY + i3 - 1) + *(pdY + i1 - 1)) * 0.5 - *(pdY + i2 - 1));
            return dResult;
        }
        else
        {
            cout << "***ERROR: norder > 2 is not supported ";
            throw("***ERROR: norder > 2 is not supported ");
        }
    }

} // namespace great
