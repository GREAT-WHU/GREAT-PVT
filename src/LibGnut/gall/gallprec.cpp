/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "gutils/gtypeconv.h"
#include "gall/gallprec.h"

using namespace std;

namespace gnut
{

    t_gallprec::t_gallprec() : t_gallnav(),
                               _degree_sp3(9),
                               _sec(3600.0 * 6),
                               _ref(t_gtime::GPS),
                               _clkref(t_gtime::GPS),
                               _clkrnx(true),
                               _clksp3(false),
                               _clknav(false),
                               _posnav(false)
    {
        id_type(t_gdata::ALLPREC);
        id_group(t_gdata::GRP_EPHEM);
        _com = 1;
        _tend = LAST_TIME;
        _tend_clk = LAST_TIME;
        _tend_sp3 = LAST_TIME;
    }
    t_gallprec::t_gallprec(t_spdlog spdlog) : t_gallnav(spdlog),
                                              _degree_sp3(9),
                                              _sec(3600.0 * 6),
                                              _ref(t_gtime::GPS),
                                              _clkref(t_gtime::GPS),
                                              _clkrnx(true),
                                              _clksp3(false),
                                              _clknav(false),
                                              _posnav(false)
    {

        id_type(t_gdata::ALLPREC);
        id_group(t_gdata::GRP_EPHEM);

        _com = 1;
        _tend = LAST_TIME;
        _tend_clk = LAST_TIME;
        _tend_sp3 = LAST_TIME;
    }

    t_gallprec::~t_gallprec()
    {

        _mapprec.clear();
    }

    bool t_gallprec::health(const string &sat, const t_gtime &t)
    {

        if (_mapsat.size() > 0)
            return t_gallnav::health(sat, t);

        return true; // default healthy if no presence of NAV
    }

    int t_gallprec::pos(const string &sat, const t_gtime &t, double xyz[3], double var[3], double vel[3], const bool &chk_mask)
    {
        _gmutex.lock();

        shared_ptr<t_geph> tmp;
        tmp = t_gallprec::_find(sat, t);
        if (tmp == _null)
        {
            for (int i = 0; i < 3; i++)
            {
                xyz[i] = 0.0;
                if (var)
                    var[i] = 0.0;
                if (vel)
                    vel[i] = 0.0;
            }
            _gmutex.unlock();
            if (_posnav && t_gallnav::pos(sat, t, xyz, var, vel, chk_mask) >= 0)
            {
                return 1;
            }
            return -1;
        }

        int irc = tmp->pos(t, xyz, var, vel, _chkHealth && chk_mask);
        _gmutex.unlock();
        return irc;
    }

    int t_gallprec::clk(const string &sat, const t_gtime &t, double *clk, double *var, double *dclk, const bool &chk_mask)
    {
        _gmutex.lock();

        if (!_clkrnx || _get_clkdata(sat, t) < 0)
        {
            *clk = 0.0;
            if (var)
                *var = 0.0;
            if (dclk)
                *dclk = 0.0;

            _gmutex.unlock();

            if (_clksp3 && this->clk_int(sat, t, clk, var, dclk) >= 0)
            {
                return 1;
            }
            if (_clknav && t_gallnav::clk(sat, t, clk, var, dclk, chk_mask) >= 0)
            {
                return 1;
            }
            return -1;
        }

        t_gpoly poly;
        poly.interpolate(_CT, _C, t.diff(_clkref), *clk, *dclk);
        *dclk = *dclk / (_CT.back() - _CT.front());

        _gmutex.unlock();
        return 1;
    }

    int t_gallprec::clk_int(const string &sat, const t_gtime &t, double *clk, double *var, double *dclk)
    {
        _gmutex.lock();

        shared_ptr<t_geph> tmp = t_gallprec::_find(sat, t);

        if (tmp == _null)
        {
            *clk = 0.0;
            if (var)
                *var = 0.0;
            if (dclk)
                *dclk = 0.0;
            _gmutex.unlock();
            return -1;
        }
        int irc = dynamic_pointer_cast<t_gephprec>(tmp)->clk_int(t, clk, var, dclk);

        _gmutex.unlock();
        return irc;
    }

    void t_gallprec::add_interval(const string &sat, int intv)
    {
        _intvm[sat] = intv;
    }

    void t_gallprec::add_agency(const string &agency)
    {
        _agency = agency;
    }

    int t_gallprec::addpos(const string &sat, const t_gtime &ep, const t_gtriple &xyz, const double &t,
                           const t_gtriple &dxyz, const double &dt)
    {
        _gmutex.lock();

        if (_overwrite || _mapsp3[sat].find(ep) == _mapsp3[sat].end())
        {

            _mapsp3[sat][ep]["X"] = xyz[0];
            _mapsp3[sat][ep]["Y"] = xyz[1];
            _mapsp3[sat][ep]["Z"] = xyz[2];
            _mapsp3[sat][ep]["C"] = t;

            _mapsp3[sat][ep]["SX"] = dxyz[0];
            _mapsp3[sat][ep]["SY"] = dxyz[1];
            _mapsp3[sat][ep]["SZ"] = dxyz[2];
            _mapsp3[sat][ep]["SC"] = dt;
        }
        else
        {
            _gmutex.unlock();
            return -1;
        }
        _gmutex.unlock();
        return 0;
    }

    int t_gallprec::addvel(const string &sat, const t_gtime &ep, double xyzt[4], double dxyz[4])
    {

        _gmutex.lock();

        if (_overwrite || _mapsp3[sat].find(ep) == _mapsp3[sat].end())
        {

            _mapsp3[sat][ep]["VX"] = xyzt[0];
            _mapsp3[sat][ep]["VY"] = xyzt[1];
            _mapsp3[sat][ep]["VZ"] = xyzt[2];
            _mapsp3[sat][ep]["VC"] = xyzt[3];

            _mapsp3[sat][ep]["SVX"] = dxyz[0];
            _mapsp3[sat][ep]["SVY"] = dxyz[1];
            _mapsp3[sat][ep]["SVZ"] = dxyz[2];
            _mapsp3[sat][ep]["SVC"] = dxyz[3];
        }
        else
        {
            _gmutex.unlock();
            return -1;
        }
        _gmutex.unlock();
        return 0;
    }

    int t_gallprec::addclk(const string &sat, const t_gtime &ep, double clk[3], double dxyz[3])
    {

        _gmutex.lock();

        if (_overwrite || _mapclk[sat].find(ep) == _mapclk[sat].end())
        {

            t_map_dat data = {
                {"C0", clk[0]},
                {"C1", clk[1]},
                {"C2", clk[2]},
                {"SC0", dxyz[0]},
                {"SC1", dxyz[1]},
                {"SC2", dxyz[2]},
            };
            _mapclk[sat][ep] = data;

            if (sat.length() == 3)
            {
                _clk_type_list.insert(AS);
            }
            else
            {
                _clk_type_list.insert(AR);
            }
        }
        else
        {
            _gmutex.unlock();
            return 1;
        }

        _gmutex.unlock();
        return 0;
    }

    int t_gallprec::addclk_tri(const string &sat, const t_gtime &ep, double clk[3], double dxyz[3])
    {

        _gmutex.lock();

        if (_overwrite || _mapclk[sat].find(ep) == _mapclk[sat].end())
        {

            t_map_dat data = {
                {"C0", clk[0]},
                {"C1", clk[1]},
                {"C2", clk[2]},
                {"IFCB_F3", clk[3]},
                {"SC0", dxyz[0]},
                {"SC1", dxyz[1]},
                {"SC2", dxyz[2]},
            };
            _mapclk[sat][ep] = data;

            if (sat.length() == 3)
            {
                _clk_type_list.insert(AS);
            }
            else
            {
                _clk_type_list.insert(AR);
            }
        }
        else
        {
            _gmutex.unlock();
            return 1;
        }

        _gmutex.unlock();
        return 0;
    }

    unsigned int t_gallprec::nepochs(const string &prn)
    {

        _gmutex.lock();

        unsigned int tmp = 0;
        if (_mapsp3.find(prn) != _mapsp3.end())
            tmp = _mapsp3[prn].size();

        _gmutex.unlock();
        return tmp;
    }

    set<string> t_gallprec::satellites() const
    {

        set<string> all_sat = t_gallnav::satellites();

        _gmutex.lock();

        t_map_prn::const_iterator itSP3 = _mapsp3.begin();
        while (itSP3 != _mapsp3.end())
        {
            all_sat.insert(itSP3->first);
            ++itSP3;
        }

        _gmutex.unlock();
        return all_sat;
    }

    void t_gallprec::clean_outer(const t_gtime &beg, const t_gtime &end)
    {

        if (end < beg)
            return;

        t_gallnav::clean_outer(beg, end);

        _gmutex.lock();

        map<string, t_map_epo>::const_iterator itPRN = _mapsp3.begin();
        while (itPRN != _mapsp3.end())
        {
            string prn = itPRN->first;

            // find and CLEAN all data (epochs) out of the specified period !
            map<t_gtime, t_map_dat>::iterator itFirst = _mapsp3[prn].begin();
            map<t_gtime, t_map_dat>::iterator itLast = _mapsp3[prn].end();
            map<t_gtime, t_map_dat>::iterator itBeg = _mapsp3[prn].lower_bound(beg); // greater|equal
            map<t_gtime, t_map_dat>::iterator itEnd = _mapsp3[prn].upper_bound(end); // greater only!

            // remove before BEGIN request
            if (itBeg != itFirst)
            {

                // begin is last
                if (itBeg == itLast)
                {
                    itBeg--;
                    _mapsp3[prn].erase(itFirst, itLast);

                    // begin is not last
                }
                else
                {
                    _mapsp3[prn].erase(itFirst, itBeg);
                }
            }

            // remove after END request
            if (itEnd != itLast)
            {
                _mapsp3[prn].erase(itEnd, itLast);
            }
            ++itPRN;
        }

        itPRN = _mapclk.begin();
        while (itPRN != _mapclk.end())
        {
            string prn = itPRN->first;

            // find and CLEAN all data (epochs) out of the specified period !
            map<t_gtime, t_map_dat>::iterator itFirst = _mapclk[prn].begin();
            map<t_gtime, t_map_dat>::iterator itLast = _mapclk[prn].end();
            map<t_gtime, t_map_dat>::iterator itBeg = _mapclk[prn].lower_bound(beg); // greater|equal
            map<t_gtime, t_map_dat>::iterator itEnd = _mapclk[prn].upper_bound(end); // greater only!

            // remove before BEGIN request
            if (itBeg != itFirst)
            {

                // begin is last
                if (itBeg == itLast)
                {
                    itBeg--;

                    _mapclk[prn].erase(itFirst, itLast);

                    // begin is not last
                }
                else
                {
                    _mapclk[prn].erase(itFirst, itBeg);
                }
            }

            // remove after END request
            if (itEnd != itLast)
            { // && ++itEnd != itLast ){

                _mapclk[prn].erase(itEnd, itLast);
            }
            itPRN++;
        }
        _gmutex.unlock();
        return;
    }

    int t_gallprec::nav(const string &sat, const t_gtime &t, double xyz[3], double var[3], double vel[3], const bool &chk_mask)
    {

        int fitdat = 24; // fitting samples
        int fitdeg = 12; // fitting degree

        for (int i = 0; i < 3; i++)
        {
            xyz[i] = 0.0;
            if (var)
                var[i] = 0.0;
            if (vel)
                vel[i] = 0.0;
        }

        // alternative use of gnav
        if (_mapsp3[sat].size() == 0)
            return ((_clknav && t_gallnav::nav(sat, t, xyz, var, vel, chk_mask) >= 0) ? 1 : -1);

        _gmutex.lock();

        t_gtime beg(_mapsp3[sat].begin()->first);
        t_gtime end(_mapsp3[sat].rbegin()->first);

        if (t < beg - 900 || t > end + 900)
        {
            _gmutex.unlock();
            return ((_clknav && t_gallnav::nav(sat, t, xyz, var, vel) >= 0) ? 1 : -1);
        }

        if (_poly_beg.find(sat) == _poly_beg.end())
            _poly_beg[sat] = LAST_TIME;
        if (_poly_end.find(sat) == _poly_end.end())
            _poly_end[sat] = FIRST_TIME;

        // use existing approximative estimates from cached polynomials
        if (!(t > _poly_end[sat] || t < _poly_beg[sat]))
        {
            _sec = _poly_end[sat] - _poly_beg[sat];
            _ref = _poly_beg[sat] + _sec / 2;

            _poly_x[sat].evaluate(t.diff(_ref) / _sec, 0, xyz[0]);
            _poly_y[sat].evaluate(t.diff(_ref) / _sec, 0, xyz[1]);
            _poly_z[sat].evaluate(t.diff(_ref) / _sec, 0, xyz[2]);
            _gmutex.unlock();
            return 1;
        }

        // prepare approximative estimates
        _PT.clear();
        _X.clear();
        _Y.clear();
        _Z.clear();

        map<t_gtime, t_map_dat>::iterator itBeg = _mapsp3[sat].begin();
        map<t_gtime, t_map_dat>::iterator itEnd = _mapsp3[sat].end();
        map<t_gtime, t_map_dat>::iterator itReq = _mapsp3[sat].upper_bound(t);

        int dst = distance(itBeg, itEnd); // tab values [#]
        if (dst < fitdeg)
        {
            _gmutex.unlock();
            return ((_clknav && t_gallnav::nav(sat, t, xyz, var, vel, chk_mask) >= 0) ? 1 : -1);
        }
        if (dst < fitdat)
        {
            fitdat = dst;
        } // shorten window

        int sign = 1; // towards future
        double diffEnd = t - _poly_end[sat];
        double diffBeg = t - _poly_beg[sat];
        if (_poly_beg[sat] == LAST_TIME)
        {
            itReq = _mapsp3[sat].upper_bound(t);
            --itReq;
        } // towards future, initialize
        else if (diffEnd > 0 && diffEnd < +900 * fitdat)
        {
            sign = 1;
            itReq = _mapsp3[sat].lower_bound(_poly_end[sat]);
        } // towards future
        else if (diffBeg < 0 && diffBeg > -900 * fitdat)
        {
            sign = -1;
            itReq = _mapsp3[sat].lower_bound(_poly_beg[sat]);
        } // towards past

        if (sign > 0 && distance(itReq, itEnd) <= fitdat)
        {
            itReq = itEnd;
            advance(itReq, -fitdat - 1);
        } // towards future, but shift
        else if (sign < 0 && distance(itBeg, itReq) <= fitdat)
        {
            itReq = itBeg;
            advance(itReq, +fitdat);
        } // towards past,   but shift

        _poly_beg[sat] = itReq->first;
        advance(itReq, +fitdat);
        _poly_end[sat] = itReq->first;
        advance(itReq, -fitdat);

        _sec = _poly_end[sat] - _poly_beg[sat];
        _ref = _poly_beg[sat] + _sec / 2;

        while (_PT.size() < static_cast<unsigned int>(fitdat))
        {
            ++itReq;
            t_gtime tt = itReq->first;
            _PT.push_back(tt.diff(_ref) / _sec);
            _X.push_back(_mapsp3[sat][tt]["X"]);
            _Y.push_back(_mapsp3[sat][tt]["Y"]);
            _Z.push_back(_mapsp3[sat][tt]["Z"]);
        }

        _poly_x[sat].fitpolynom(_PT, _X, fitdeg, _sec, _ref);
        _poly_x[sat].evaluate(t.diff(_ref) / _sec, 0, xyz[0]);
        _poly_y[sat].fitpolynom(_PT, _Y, fitdeg, _sec, _ref);
        _poly_y[sat].evaluate(t.diff(_ref) / _sec, 0, xyz[1]);
        _poly_z[sat].fitpolynom(_PT, _Z, fitdeg, _sec, _ref);
        _poly_z[sat].evaluate(t.diff(_ref) / _sec, 0, xyz[2]);


        _gmutex.unlock();
        return 1;
    }

    shared_ptr<t_geph> t_gallprec::_find(const string &sat, const t_gtime &t)
    {

        if (_mapsp3.find(sat) == _mapsp3.end())
            return _null; 
        else if (_mapsp3[sat].size() == 0)
            return _null;
        // if not exists satellite not in cache
        t_map_sp3::iterator it = _prec.find(sat);
        if (it == _prec.end())
        {
            if (_get_crddata(sat, t) < 0)
                return _null; 
        }

        // could not find the data at all --- SHOULDN'T OCCURE, SINCE _get_crddata already return !
        it = _prec.find(sat);
        if (it == _prec.end())
        {
            return _null; 
        }

        double t_minus_ref = t - (it->second)->epoch();

        // standard case: cache - satellite found and cache still valid!
        if (fabs((float)t_minus_ref) < (it->second)->interval() / _degree_sp3 && 
            (it->second)->valid(t)                                              
        )
        {
            return it->second;
        }
        else
        {

            t_gtime beg(_mapsp3[sat].begin()->first);
            t_gtime end(_mapsp3[sat].rbegin()->first);

            // update cache only if not close to the prec data boundaries
            if ((fabs(t.diff(beg)) > (it->second)->interval() / 2 &&
                 fabs(t.diff(end)) > (it->second)->interval() / 2) ||
                !(it->second)->valid(t))
            {
                if (_get_crddata(sat, t) < 0)
                    return _null; 
                it = _prec.find(sat);
            }
        }

        return it->second;
    }

    int t_gallprec::_get_crddata(const string &sat, const t_gtime &t)
    {

        _T.clear();
        _PT.clear();
        _X.clear();
        _Y.clear();
        _Z.clear();
        _CT.clear();
        _C.clear();

        if (_mapsp3.find(sat) == _mapsp3.end())
            return -1;

        map<t_gtime, t_map_dat>::iterator itReq = _mapsp3[sat].lower_bound(t); // 1st equal|greater [than t]

        if (itReq == _mapsp3[sat].end())
            return -1;

        map<t_gtime, t_map_dat>::iterator itReq_tmp = --(_mapsp3[sat].lower_bound(t));

        if (itReq_tmp != std::end(_mapsp3[sat]))
        {
            if (abs(t.diff(itReq_tmp->first)) < abs(t.diff(itReq->first)))
                itReq = itReq_tmp;
        }

        _ref = itReq->first; // get the nearest epoch after t as reference

        map<t_gtime, t_map_dat>::iterator itBeg = _mapsp3[sat].begin();
        map<t_gtime, t_map_dat>::iterator itEnd = _mapsp3[sat].end();
        map<t_gtime, t_map_dat>::iterator it = itReq;

        if (itReq == itEnd)
        {
            return -1;
        }


        // DISTANCE() NEED TO BE A POSITIVE DIFFERENCE !!!
        int limit = static_cast<int>(_degree_sp3 / 2); // round (floor)

        // too few data
        if (distance(itBeg, itEnd) < static_cast<int>(_degree_sp3))
        {
            return -1;
        }
        else if (distance(itBeg, itReq) < limit)
        {
            it = itBeg;
        }
        else if (distance(itReq, itEnd) <= static_cast<int>(_degree_sp3 - limit))
        {
            it = itEnd;
            for (int i = 0; i <= static_cast<int>(_degree_sp3); i++)
                it--;
        }
        else
        {
            for (int i = 0; i < limit; i++)
                it--;
        }

        if (it == itEnd)
        {
            return -1;
        }

        // vector for polynomial
        for (unsigned int i = 0; i <= _degree_sp3; it++, i++)
        {
            double tdiff = it->first - _ref;

            // check maximum interval allowed between reference and sta/end epochs
            if (fabs(tdiff) > static_cast<double>(_degree_sp3 * MAXDIFF_EPH))
                continue;

            if (it->second["X"] != UNDEFVAL_POS)
            {

                _PT.push_back(tdiff);
                _T.push_back(it->first);
                _X.push_back(it->second["X"]);
                _Y.push_back(it->second["Y"]);
                _Z.push_back(it->second["Z"]);
                _CT.push_back(tdiff);
                _C.push_back(it->second["C"]);

            }
        }

        if (_X.size() != _degree_sp3 + 1)
        {
            return -1;
        }

        if (_prec.find(sat) != _prec.end())
        {
            _prec[sat]->degree(_degree_sp3);
            _prec[sat]->add(sat, _T, _X, _Y, _Z, _C);
        }
        else
        {
            shared_ptr<t_gephprec> tmp(new t_gephprec(_spdlog));
            tmp->spdlog(_spdlog);
            tmp->degree(_degree_sp3);
            tmp->add(sat, _T, _X, _Y, _Z, _C);
            _prec[sat] = tmp;
        }

        return 1;
    }

    int t_gallprec::_get_clkdata(const string &sat, const t_gtime &t)
    {

        _CT.clear();
        _C.clear();
        _IFCB_F3.clear();

        if (_mapclk.find(sat) == _mapclk.end())
            return -1;
        map<t_gtime, t_map_dat>::iterator itBeg = _mapclk[sat].begin();
        map<t_gtime, t_map_dat>::iterator itEnd = _mapclk[sat].end();
        map<t_gtime, t_map_dat>::iterator itReq = _mapclk[sat].lower_bound(t); // 1st equal|greater [than t]

        if (itReq == _mapclk[sat].end())
            return -1; // too old products
        if (t < itBeg->first)
            return -1; // too new products

        _clkref = itReq->first; // get the nearest epoch after t as reference

        map<t_gtime, t_map_dat>::iterator it = itReq;

        if (itReq == itEnd)
        {
            return -1;
        }


        unsigned int degree_clk = 1; // MOZNA DAT 3 degree polinom kvuli 1Hz datum (ale cekovat ktery 3)

        int limit = static_cast<int>(degree_clk / 2); // round (floor)

        bool flag_left = false;
        auto itleft = itReq;
        for (int i = 0; i <= limit; i++)
        {
            itleft--;
            if (itleft == _mapclk[sat].end())
            {
                flag_left = true;
                break;
            }
        }
        bool flag_right = false;
        auto itright = itReq;
        for (int i = 0; i < static_cast<int>(degree_clk - limit); i++)
        {
            itright++;
            if (itright == _mapclk[sat].end())
            {
                flag_right = true;
                break;
            }
        }

        if (_mapclk[sat].size() < static_cast<unsigned int>(degree_clk))
        {
            return -1;
        }
        else if (flag_left)
        {
            it = itBeg;
        }
        else if (flag_right)
        {
            it = itEnd;
            for (int i = 0; i <= static_cast<int>(degree_clk); i++)
                it--;
        }
        else
        {
            for (int i = 0; i <= limit; i++)
                it--;
        }

        // calculate
        if (it->second["C1"] < UNDEFVAL_CLK)
        {
            if (it->second["C2"] < UNDEFVAL_CLK)
            {
                if (it->second.find("IFCB_F3") == it->second.end())
                {
                    double tdiff = it->first - t; // seconds !!!!!!!!!!!!!!!!!!!! instead of _clkref should be ReqT = t !!
                    _CT.push_back(tdiff);
                    _C.push_back(it->second["C0"] + it->second["C1"] * tdiff + it->second["C2"] * tdiff * tdiff);
                }
                else
                {
                    it++;
                    double tdiff = it->first - t; // seconds !!!!!!!!!!!!!!!!!!!! instead of _clkref should be ReqT = t !!
                    _CT.push_back(tdiff);
                    _C.push_back(it->second["C0"]);
                    _IFCB_F3.push_back(it->second["IFCB_F3"]);
                }
                return 1;
            }
            else
            {
                double tdiff = it->first - t; // seconds !!!!!!!!!!!!!!!!!!!! instead of _clkref should be ReqT = t !!
                _CT.push_back(tdiff);
                _C.push_back(it->second["C0"] + it->second["C1"] * tdiff);
                return 1;
            }

            // interpolate
        }
        else
        {

            // vector for polynomial
            for (unsigned int i = 0; i <= degree_clk; it++, i++)
            {
                double tdiff = it->first - _clkref;

                // check maximum interval allowed between reference and sta/end epochs
                if (fabs(tdiff) > static_cast<double>(degree_clk * MAXDIFF_CLK))
                {
                    continue;
                }

                if (it->second["C0"] != UNDEFVAL_CLK)
                {
                    _CT.push_back(tdiff);
                    _C.push_back(it->second["C0"]);
                }
            }

            if (_C.size() != degree_clk + 1)
            {

                if (itReq != itBeg)
                {
                    _C.clear();
                    _CT.clear();
                    auto itFirst = itReq;
                    itFirst--;
                    _C.push_back(itFirst->second["C0"]);
                    if (fabs(itFirst->first - _clkref) > static_cast<double>(degree_clk * MAXDIFF_CLK))
                    {
                        return -1;
                    }
                    _CT.push_back(itFirst->first - _clkref);

                    _C.push_back(itReq->second["C0"]);
                    _CT.push_back(0.0);

                    return 1;
                }

                return -1;
            }
        }

        return 1;
    }

    int t_gallprec::_get_delta_pos_vel(const string &sat, const t_gtime &t, int iod, t_gtime &tRef, t_map_dat &orbcorr)
    {

        if (_mapsp3.find(sat) == _mapsp3.end() || _mapsp3[sat].size() == 0)
        {
             return -1;
        }

        map<t_gtime, t_map_dat>::iterator itLast = _mapsp3[sat].lower_bound(t); 

        map<t_gtime, t_map_dat>::iterator itPrev = --(_mapsp3[sat].lower_bound(t)); 

        if (itLast != _mapsp3[sat].end() && int(itLast->second["IOD"]) == iod)
        {
            tRef = itLast->first;
            orbcorr = itLast->second;
             return 1;
        }
        else if (itPrev != _mapsp3[sat].end() && int(itPrev->second["IOD"]) == iod)
        {
            tRef = itPrev->first;
            orbcorr = itPrev->second;
             return 1;
        }
        else
        {
             return -1;
        }
    }

    int t_gallprec::_get_delta_clk(const string &sat, const t_gtime &t, int iod, t_gtime &tRef, t_map_dat &clkcorr)
    {


        if (_mapclk.find(sat) == _mapclk.end() || _mapclk[sat].size() == 0)
        {
             return -1;
        }
        map<t_gtime, t_map_dat>::iterator itLast = _mapclk[sat].lower_bound(t); // 1st equal|greater [than t]

        map<t_gtime, t_map_dat>::iterator itPrev = --_mapclk[sat].lower_bound(t); // 1st equal|greater [than t]

        if (itLast != _mapclk[sat].end() && int(itLast->second["IOD"]) == iod)
        {
            tRef = itLast->first;
            clkcorr = itLast->second;
             return 1;
        }
        else if (itPrev != _mapclk[sat].end() && int(itPrev->second["IOD"]) == iod)
        {
            tRef = itPrev->first;
            clkcorr = itPrev->second;
             return 1;
        }
        else
        {
             return -1;
        }
    }

} // namespace
