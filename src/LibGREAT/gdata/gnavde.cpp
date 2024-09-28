/**
 * @file         gnavde.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        The base class for save JPL DE file information.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gdata/gnavde.h"
#include "gutils/gconst.h"
#include <math.h>
#include <algorithm>
using namespace std;

namespace great
{
    t_gnavde::t_gnavde() : _days(0.0), _emrat(0.0)
    {
        id_type(t_gdata::ALLDE);

        _allplanets[PLANET_EART].gm = EARTH_GM; //earth gm
        _allplanets[PLANET_EART].rad = EARTH_R; //radius /km

        _allplanets[PLANET_SUN].gm = SUN_GM; //sun
        _allplanets[PLANET_SUN].rad = SUN_R;

        _allplanets[PLANET_MOON].gm = MOON_GM; //moon
        _allplanets[PLANET_MOON].rad = MOON_R;
    }

    t_gnavde::t_gnavde(t_spdlog spdlog) : t_gdata(spdlog)
    {
        id_type(t_gdata::ALLDE);

        _allplanets[PLANET_EART].gm = EARTH_GM; //earth gm
        _allplanets[PLANET_EART].rad = EARTH_R; //radius /km

        _allplanets[PLANET_SUN].gm = SUN_GM; //sun
        _allplanets[PLANET_SUN].rad = SUN_R;

        _allplanets[PLANET_MOON].gm = MOON_GM; //moon
        _allplanets[PLANET_MOON].rad = MOON_R;
    }

    t_gnavde::~t_gnavde() {}

    void t_gnavde::get_pos(const double &tm, const string &plane_tname, t_gtriple &pos)
    {
        //transfer planetname to (int) num
        int planet = str2planet(plane_tname);
        //get cunrrent time
        double pjd = tm + 2400000.5;
        //get position and velocity
        double rrd[6];
        _pleph(pjd, planet, 2, rrd);
        //transfer double[] to t_gtriple
        pos[0] = rrd[0];
        pos[1] = rrd[1];
        pos[2] = rrd[2];
    }

    void t_gnavde::get_pos(const double &tm, const string &planet_name, ColumnVector &pos)
    {
        //transfer planetname to (int) num
        int planet = str2planet(planet_name);
        //get cunrrent time
        double pjd = tm + 2400000.5;
        //get position and velocity
        double rrd[6];
        _pleph(pjd, planet, 2, rrd);
        //transfer
        pos(1) = rrd[0];
        pos(2) = rrd[1];
        pos(3) = rrd[2];
    }

    void t_gnavde::_pleph(const double &et, const int &planet, const int &center, double *rrd)
    {
        //tag for interpolate
        /*LIST[i]=0, NO INTERPOLATION FOR BODY I
        = 1, POSITION ONLY
        = 2, POSITION AND VELOCITY*/
        int list[12];
        double et2[2];
        double pv[6][13] = {0};
        double pv_sun[6] = {0}; //position and velocity for sun
        et2[0] = et;
        et2[1] = 0;
        //bool isbary = false;
        for (int i = 0; i < 12; i++)
        {
            list[i] = 0;
        }
        for (int i = 0; i < 6; i++)
        {
            rrd[i] = 0.0;
        }
        //planet is center body
        if (planet == center)
        {
            return;
        }
        //nutation
        if (planet == 13)
        {
            list[10] = 2;
            _state(et2, list, pv, rrd);
            return;
        }
        //Lunar libration
        if (planet == 14)
        {
            list[11] = 2;
            _state(et2, list, pv, rrd);
            for (int i = 0; i < 6; i++)
            {
                rrd[i] = pv[i][10];
            }
        }

        //bool isbsave = isbary;
        //isbary = true;
        //set list value
        for (int i = 0; i < 2; i++)
        {
            int k = planet;
            if (i == 1)
                k = center;
            if (k <= 9)
                list[k] = 2;
            if (k == 9)
                list[2] = 2;
            if (k == 2)
                list[9] = 2;
            if (k == 12)
                list[2] = 2;
        }
        //calculate position and velcity(save in rrd)
        _state(et2, list, pv, rrd);
        //planet or center is sun
        if (planet == 10 || center == 10)
        {
            for (int i = 0; i < 6; i++)
            {
                pv[i][10] = pv_sun[i];
            }
        }
        //planet or center body is solar system centroid
        if (planet == 11 || center == 11)
        {
            for (int i = 0; i < 6; i++)
            {
                pv[i][11] = 0;
            }
        }
        ////planet or center body is moon-earth centroid
        if (planet == 12 || center == 12)
        {
            for (int i = 0; i < 6; i++)
            {
                pv[i][12] = pv[i][2];
            }
        }
        //planet is sun (earth),center is earth(sun)
        if (planet * center == 18 && planet + center == 11)
        {
            for (int i = 0; i < 6; i++)
            {
                pv[i][2] = 0;
            }
            for (int i = 0; i < 6; i++)
            {
                rrd[i] = pv[i][planet] - pv[i][center];
                if (i > 2)
                {
                    rrd[i] = rrd[i] * 86400.0;
                }
            }
            return;
        }
        if (list[2] == 2)
        {
            for (int i = 0; i < 6; i++)
            {
                pv[i][2] = pv[i][2] - pv[i][9] / (1.0 + _emrat);
            }
        }
        if (list[9] == 2)
        {
            for (int i = 0; i < 6; i++)
            {
                pv[i][9] = pv[i][2] + pv[i][9];
            }
        }

        for (int i = 0; i < 6; i++)
        {
            rrd[i] = pv[i][planet] - pv[i][center];
            if (i > 2)
            {
                rrd[i] = rrd[i] * 86400.0;
            }
        }
        return;
    }

    int t_gnavde::_state(double et[], int list[], double pv[][13], double pnut[])
    {
        double pv_sun[6][2];
        double dt[2];
        double aufact;
        double start = _start_mjd + 2400000.5;
        double end = _end.dmjd() + 2400000.5;

        //get current time
        double pjd[4];
        double ds = et[0] - 0.5;
        _split(ds, &pjd[0]);
        _split(et[1], &pjd[2]);
        pjd[0] += pjd[2] + 0.5;
        pjd[1] += pjd[3];
        _split(pjd[1], &pjd[2]);
        pjd[0] += pjd[2];

        if (pjd[0] + pjd[3] < start || pjd[0] + pjd[3] > end)
        {
            cout << "ERROR:wrong JPL file,obs time is out of range" << endl;
            return -1;
        }
        //find the correct coffe by time
        int nindex = floor((pjd[0] - start) / _days);
        if (pjd[0] - end == 0)
        {
            nindex--;
        }

        dt[0] = ((pjd[0] - nindex * _days - start) + pjd[3]) / _days;

        vector<double> coeff = _chebycoeff[nindex];

        //unit correct
        dt[1] = _days * 86400;
        aufact = 1;

        //interpolate sun
        _interp(coeff, _allplanets[10].ipt, _allplanets[10].ncf, 3, _allplanets[10].na, 2, dt, pv_sun[0]);
        for (int i = 0; i < 6; i++)
        {
            pv_sun[i][0] = pv_sun[i][0] * aufact;
        }
        //check and interpolate whichever bodies are requested
        double tmp[12];
        for (int i = 0; i < 10; i++)
        {
            if (list[i] == 0)
                continue;
            _interp(coeff, _allplanets[i].ipt, _allplanets[i].ncf, 3, _allplanets[i].na, list[i], dt, tmp);
            for (int j = 0; j < 6; j++)
            {
                pv[j][i] = tmp[2 * j];
                pv[j][i + 1] = 0;
            }
            for (int k = 0; k < 6; k++)
            {
                if (i < 9)
                {
                    pv[k][i] = pv[k][i] * aufact - pv_sun[k][0];
                }
                else
                {
                    pv[k][i] = pv[k][i] * aufact;
                }
            }
        }

        // do nutations if requested(and if on file)
        if (list[10] > 0 && _allplanets[11].ncf > 0)
        {
            _interp(coeff, _allplanets[11].ipt, _allplanets[11].ncf, 2, _allplanets[11].na, list[10], dt, pnut);
        }
        //get librations if requested (and if on file)
        if (list[11] > 0 && _allplanets[12].ncf > 0)
        {
            _interp(coeff, _allplanets[12].ipt, _allplanets[12].ncf, 3, _allplanets[12].na, list[11], dt, &pv[0][10]);
        }
        return 0;
    }

    void t_gnavde::_split(const double &tt, double *fr)
    {
        fr[0] = floor(tt);
        fr[1] = tt - fr[0];
        if (tt >= 0.0 || fr[1] == 0.0)
        {
            return;
        }
        fr[0] -= 1.0;
        fr[1] += 1.0;
        return;
    }

    void t_gnavde::_interp(const vector<double> &coeff, const int &ipt, const int &ncf, const int &ncm, const int &na, const int &ifl, double dt[], double *pv)
    {
        int np = 2;
        int nv = 3;
        double twot = 0.0;
        double pc[18] = {1.0, 0.0};
        double vc[18] = {0.0, 1.0};
        double vfac;

        //entry point.
        //get correct sub-interval number for this set of coefficients and then get
        //    normalized chebyshev time within that subinterval
        double dna = (double)na;
        double t1 = (int)dt[0];
        double tmp = dna * dt[0];
        int l = (int)(tmp - t1) + 1;
        //tc is the normalized chebyshev time
        double tc = 2.0 * (fmod(tmp, 1.0) + t1) - 1.0;
        //check to see whether chebyshev time has changed,
        //and compute new polynomial value if it has.

        if (tc != pc[1])
        {
            np = 2;
            nv = 3;
            pc[1] = tc;
            twot = tc + tc;
        }
        //be sure that at least "ncf" polynomials have been evaluated
        //    and are stored in the array pc[]
        if (np < ncf)
        {
            for (int i = np + 1; i <= ncf; i++)
            {
                pc[i - 1] = twot * pc[i - 2] - pc[i - 3];
            }
            np = ncf;
        }
        //interpolate to get position for each component
        for (int i = 1; i <= ncm; i++)
        {
            pv[(i - 1) * 2] = 0.0;
            for (int j = ncf; j >= 1; j--)
            {
                pv[(i - 1) * 2] = pv[(i - 1) * 2] + pc[j - 1] * coeff[ipt - 1 + (l - 1) * ncf * ncm + (i - 1) * ncf + j - 1];
            }
        }

        if (ifl <= 1)
        {
            return;
        }
        //if velocity interpolation is wanted,be sure enougn
        //derivative polynomials have been generated and stored
        vfac = (dna + dna) / dt[1];
        vc[2] = twot + twot;
        if (nv < ncf)
        {
            for (int i = nv + 1; i <= ncf; i++)
            {
                vc[i - 1] = twot * vc[i - 2] + pc[i - 2] + pc[i - 2] - vc[i - 3];
            }
            nv = ncf;
        }
        //interpolate to get velocity for each component
        for (int i = 1; i <= ncm; i++)
        {
            pv[(i - 1) * 2 + 6] = 0;
            for (int j = ncf; j >= 2; j--)
            {
                pv[(i - 1) * 2 + 6] = pv[(i - 1) * 2 + 6] + vc[j - 1] * coeff[ipt - 1 + (l - 1) * ncm * ncf + (i - 1) * ncf + j - 1];
            }
            pv[(i - 1) * 2 + 6] = pv[(i - 1) * 2 + 6] * vfac;
        }
    }

    void t_gnavde::add_head(double SS[3], const double &au, const double &emrat, int ipt[3][13], const vector<string> &constname, const vector<double> &constval)
    {
        _start.from_mjd(int(SS[0] - 2400000.5), 0, 0);
        _start_mjd = _start.dmjd();
        _end.from_mjd(int(SS[1] - 2400000.5), 0, 0);
        _days = SS[2];

        _au = au * au * au / (86400.0 * 86400.0);
        _emrat = emrat;

        for (int i = 0; i < 13; i++)
        {
            _allplanets[i].ipt = ipt[0][i];
            _allplanets[i].ncf = ipt[1][i];
            _allplanets[i].na = ipt[2][i];
            //get gm and radius
            if (i == 2 || i == 9 || i == 10)
            {
                continue; //sun earth and moon
            }
            _allplanets[i].gm = 0.0;
            _allplanets[i].rad = 0.0;
            string gm = "GM" + to_string(i + 1) + "   ";
            string rad = "RAD" + to_string(i + 1) + "  ";
            for (int j = 0; j < constname.size(); j++)
            {
                string tmp = constname[j];
                if (gm == tmp)
                {
                    _allplanets[i].gm = constval[j];
                    _allplanets[i].gm = _allplanets[i].gm * _au;
                }
                if (rad == tmp)
                {
                    _allplanets[i].rad = constval[j];
                }
            }
        }
    }

    void t_gnavde::add_data(const int &index, const vector<double> &coeff)
    {
        _chebycoeff[index] = coeff;
    }

    int t_gnavde::str2planet(const string &tmp) const
    {
        string planet(tmp);
        transform(planet.begin(), planet.end(), planet.begin(), ::toupper);
        if (planet == "MERCURY" || planet == "MERC")
            return PLANET_MERC;
        else if (planet == "VENUS" || planet == "VENU")
            return PLANET_VENU;
        else if (planet == "EARTH" || planet == "EART")
            return PLANET_EART;
        else if (planet == "MARS")
            return PLANET_MARS;
        else if (planet == "JUPITER" || planet == "JUPI")
            return PLANET_JUPI;
        else if (planet == "SATURN" || planet == "SATU")
            return PLANET_SATU;
        else if (planet == "URANUS" || planet == "URAN")
            return PLANET_URAN;
        else if (planet == "NEPTUNE" || planet == "NEPT")
            return PLANET_NEPT;
        else if (planet == "PLUTO" || planet == "PLUT")
            return PLANET_PLUT;
        else if (planet == "MOON")
            return PLANET_MOON;
        else if (planet == "SUN")
            return PLANET_SUN;
        else if (planet == "SOLAR_SYSTEM")
            return SOLAR_SYSTEM;
        else if (planet == "EARTH_MOON")
            return EARTH_MOON;
        else
        {
            cout << "unknow planet" << endl;
            return -1;
        }
    }
    t_gplanet::t_gplanet()
    {
    }
}