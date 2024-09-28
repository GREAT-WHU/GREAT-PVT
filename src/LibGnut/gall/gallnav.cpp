/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "gdata/gnavgps.h"
#include "gdata/gnavglo.h"
#include "gdata/gnavgal.h"
#include "gdata/gnavqzs.h"
#include "gdata/gnavsbs.h"
#include "gdata/gnavbds.h"
#include "gdata/gnavirn.h"
#include "gall/gallnav.h"
#include "gutils/gsys.h"
#include "gutils/gtimesync.h"
#include "gutils/gtypeconv.h"
#include "gutils/gfileconv.h"

using namespace std;

namespace gnut
{
    t_gallnav::t_gallnav()
        : t_gdata(),
          _com(false),
          _nepoch(t_gtime::GPS),
          _multimap(false),
          _overwrite(false),
          _chkHealth(true), 
          _chkNavig(true),
          _chkTot(false)
    {

        id_type(t_gdata::ALLNAV);
        id_group(t_gdata::GRP_EPHEM);
    }

    t_gallnav::t_gallnav(t_spdlog spdlog)
        : t_gdata(spdlog),
          _com(false),
          _nepoch(t_gtime::GPS),
          _multimap(false),
          _overwrite(false),
          _chkHealth(true), 
          _chkNavig(true),
          _chkTot(false)
    {
        id_type(t_gdata::ALLNAV);
        id_group(t_gdata::GRP_EPHEM);
    }

    t_gallnav::~t_gallnav()
    {

        _mapsat.clear();
    }

    shared_ptr<t_geph> t_gallnav::find(const string &sat, const t_gtime &t, const bool &chk_mask)
    {

        _gmutex.lock();

        shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, _chkHealth && chk_mask);

        _gmutex.unlock();
        return tmp;
    };
    int t_gallnav::pos(const string &sat,
                       const t_gtime &t,
                       double xyz[3],
                       double var[3],
                       double vel[3],
                       const bool &chk_mask) // [m]
    {
        _gmutex.lock();

        shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, _chkHealth && chk_mask);
        
        if (tmp == _null)
        {
            tmp = t_gallnav::_find(sat, t, false);
        }
        
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
            return -1;
        }

        int irc = tmp->pos(t, xyz, var, vel, _chkHealth && chk_mask);
        
        if (irc == -1)
        {
            irc = tmp->pos(t, xyz, var, vel, false);
        }
        

        _gmutex.unlock();
        return irc;
    }

    int t_gallnav::pos(const string &sat,
                       const int &iod,
                       const t_gtime &t,
                       double xyz[3],
                       double var[3],
                       double vel[3],
                       const bool &chk_mask) // [m]
    {
        _gmutex.lock();

        shared_ptr<t_geph> tmp = t_gallnav::_find(sat, iod, t, _chkHealth && chk_mask);
        
        if (tmp == _null)
        {
            tmp = t_gallnav::_find(sat, iod, t, false);
        }
        
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
            return -1;
        }

        int irc = tmp->pos(t, xyz, var, vel, _chkHealth && chk_mask);
        
        if (irc == -1)
        {
            irc = tmp->pos(t, xyz, var, vel, false);
        }
        

        _gmutex.unlock();
        return irc;

    }

    bool t_gallnav::health(const string &sat, const t_gtime &t)
    {
        _gmutex.lock();

        shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, false);

        if (tmp == _null)
        {
            _gmutex.unlock();
            return false;
        }

        bool status = tmp->healthy();

        _gmutex.unlock();
        return status;
    }

    int t_gallnav::nav(const string &sat,
                       const t_gtime &t,
                       double xyz[3],
                       double var[3],
                       double vel[3],
                       const bool &chk_mask) // [m]
    {
        _gmutex.lock();

        shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, _chkHealth && chk_mask);

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
            return -1;
        }
        int irc = tmp->nav(t, xyz, var, vel, _chkHealth && chk_mask);

        _gmutex.unlock();
        return irc;

    }

    // ----------
    int t_gallnav::clk(const string &sat,
                       const t_gtime &t,
                       double *clk,
                       double *var,
                       double *dclk,
                       const bool &chk_mask) // [s]
    {
        _gmutex.lock();

        shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, _chkHealth && chk_mask);

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

        int irc = tmp->clk(t, clk, var, dclk, _chkHealth && chk_mask);
        _gmutex.unlock();
        return irc;

    }

    set<string> t_gallnav::satellites() const
    {
        _gmutex.lock();

        set<string> all_sat;
        auto itPRN = _mapsat.begin();

        while (itPRN != _mapsat.end())
        {
            all_sat.insert(itPRN->first);
            itPRN++;
        }

        _gmutex.unlock();
        return all_sat;
    }

    int t_gallnav::add(shared_ptr<t_gnav> nav)
    {

        _gmutex.lock();

        t_gtime ep(nav->epoch());
        string sat = nav->sat();

        if (_chkNavig)
        {
            set<string> msg;
            nav->chk(msg);
        }

        // test navigation type
        bool add = false;
        if (_multimap)
        {
            add = true;
        } // multimap
        else if (_mapsat[sat].find(ep) == _mapsat[sat].end())
        {
            add = true;
        } // non-existent
        else if (nav->id_type() == t_gdata::EPHGAL)
        { // check nav-type
            auto itB = _mapsat[sat].lower_bound(ep);
            auto itE = _mapsat[sat].upper_bound(ep);
            while (itB != itE)
            {
                if (dynamic_pointer_cast<t_gnav>(itB->second)->gnavtype() == nav->gnavtype())
                {
                    add = false;
                    break;
                } // exclude the message and skip!
                else
                {
                    add = true;
                } // ok
                ++itB;
            }
        }
        if (!nav->valid())
            add = false; // validity test

        if (add)
        {
            _mapsat[sat].insert(make_pair(ep, nav));
            if (sat.substr(0, 1) == "R")
            {
                _gloFreqNum[sat] = nav->freq_num();
            }
        }

        _gmutex.unlock();
        return 0;
    }

    unsigned int t_gallnav::nepochs(const string &prn)
    {
        _gmutex.lock();

        unsigned int tmp = 0;
        if (_mapsat.find(prn) != _mapsat.end())
            tmp = _mapsat[prn].size();

        _gmutex.unlock();
        return tmp;
    }

    void t_gallnav::clean_outer(const t_gtime &beg, const t_gtime &end)
    {

        if (end < beg)
            return;
        if (beg == FIRST_TIME)
            return;
        if (end == LAST_TIME)
            return;

        _gmutex.lock();

        // loop over all satellites
        auto itPRN = _mapsat.begin();
        while (itPRN != _mapsat.end())
        {
            string prn = itPRN->first;

            // find and CLEAN all data (epochs) out of the specified period !
            auto itFirst = _mapsat[prn].begin();
            auto itLast = _mapsat[prn].end();
            auto itBeg = _mapsat[prn].lower_bound(beg); // greater only   old: // greater|equal
            auto itEnd = _mapsat[prn].upper_bound(end); // greater only!

            // remove before BEGIN request
            if (itBeg != itFirst)
            {
                auto it = itFirst;

                while (it != itBeg && it != itLast)
                {
                    if ((it->second).use_count() == 1)
                    {
                        _mapsat[prn].erase(it++);
                    }
                    else
                    {
                        it++;
                    }
                }
            }

            // remove after END request
            if (itEnd != itLast)
            {
                auto it = itEnd;

                while (it != itLast)
                {
                    if ((it->second).use_count() == 1)
                    {
                        _mapsat[prn].erase(it++);
                    }
                    else
                    {
                        it++;
                    }
                }
            }
            itPRN++;
        }
        _gmutex.unlock();
    }

    int t_gallnav::nsat(const GSYS &gs) const
    {

        int nsatell = 0;
        for (auto itSAT = _mapsat.begin(); itSAT != _mapsat.end(); ++itSAT)
        {
            string sat = itSAT->first;
            if (t_gsys::char2gsys(sat[0]) == gs || gs == GNS)
                nsatell++;
        }
        return nsatell;
    }

    t_iono_corr t_gallnav::get_iono_corr(const IONO_CORR &c) const
    {
        if (_brdc_iono_cor.find(c) != _brdc_iono_cor.end())
        {
            return _brdc_iono_cor.at(c);
        }
        t_iono_corr io;
        // default settings , according rtklib->rtkcmn.c->ionmodel
        if (c == IO_GPSA)
        {
            io.x0 = 0.1118e-07;
            io.x1 = -0.7451e-08;
            io.x2 = -0.5961e-07;
            io.x3 = 0.1192e-06;
        }
        else if (c == IO_GPSB)
        {
            io.x0 = 0.1167e+06;
            io.x1 = -0.2294e+06;
            io.x2 = -0.1311e+06;
            io.x3 = 0.1049e+07;
        }

        return io;
    }

    void t_gallnav::add_iono_corr(const IONO_CORR &c, const t_iono_corr &io)
    {
        if (_brdc_iono_cor.find(c) == _brdc_iono_cor.end())
        {
            _brdc_iono_cor[c] = io;
        }
    }

    shared_ptr<t_geph> t_gallnav::_find(const string &sat, const t_gtime &t, const bool &chk_mask)
    {

        if (_mapsat.find(sat) == _mapsat.end())
            return _null; // make_shared<t_geph>();

        GSYS gnss = t_gsys::str2gsys(sat.substr(0, 1));

        if (_mapsat[sat].size() == 0)
        {
            return _null;
        }

        auto it = _mapsat[sat].lower_bound(t); // greater|equal  (can be still end())
        if (it == _mapsat[sat].end())
            it--; // size() > 0 already checked above

        double maxdiff = t_gnav::nav_validity(gnss) * 1.1;

        if (gnss == GAL)
        {
            // Galileo ephemerides are valid for toc+10min -> toc+20...180min
            for (int bck = 1; bck <= 5; bck++)
            { // max eph for going back is 5
                t_gtime toc = it->second->epoch();
                if ((t < toc + 10 * 60 || t > toc + maxdiff) ||
                    (_chkTot && !it->second->chktot(t)))
                { // check ToT for using past messages only
                    if (_mapsat[sat].size() > 0 && it != _mapsat[sat].begin())
                        it--; // one more step back
                    else
                        break;
                }
                if ((t >= toc + 10 * 60 && t <= toc + maxdiff) &&
                    (_chkTot && it->second->chktot(t)))
                    break;
            }
        }
        else if (gnss == BDS)
        {
            // BeiDou ephemerides are valid for toc -> toc+60min
            for (int bck = 1; bck <= 5; bck++)
            { // max eph for going back is 5
                t_gtime toc = it->second->epoch();
                if ((t < toc || t > toc + maxdiff) ||
                    (_chkTot && !it->second->chktot(t)))
                { // check ToT for using past messages only
                    if (_mapsat[sat].size() > 0 && it != _mapsat[sat].begin())
                        it--; // one more step back
                    else
                        break;
                }
                if ((t >= toc && t <= toc + maxdiff) &&
                    (_chkTot && it->second->chktot(t)))
                    break;
            }
        }
        else if (gnss == GLO)
        {
            t_gtime tt = t - maxdiff; // time span of Glonass ephemerides is 15 min

            auto itt = _mapsat[sat].lower_bound(tt); // greater|equal
            if (itt == _mapsat[sat].end())
            { // size() > 0 already checked above
                return _null;
            }

            double dt1 = itt->first - t;
            double dt2 = it->first - t;
            if (fabs(dt1) < fabs(dt2))
                it = itt;
        }
        else
        {
            auto itLAST = _mapsat[sat].end();
            itLAST--;
            auto itSAVE = it;
            bool found = false;
            while (_chkTot && !itSAVE->second->chktot(t) && itSAVE->second->epoch().sod() % 3600 != 0 && itSAVE != itLAST)
            {
                itSAVE++; // one more step forward (special case for non-regular ephemerides (e.g. XX:59:44))
            }
            if (_chkTot && itSAVE->second->chktot(t))
            { // check found non-regular ephemeris
                found = true;
                it = itSAVE;
            }

            // 2 conditions for all cases when NAV is in fugure and the iterator should be moved back (it--)
            if (fabs(t - it->second->epoch()) > maxdiff ||    // too far navigation message in future!
                (_chkTot && !it->second->chktot(t) && !found) // check ToT for using past messages only
            )
            {
                if (_mapsat[sat].size() > 0 && it != _mapsat[sat].begin())
                    it--; // one more step back
            }
        }

        // tested found ephemeris
        if (fabs(t - it->second->epoch()) > maxdiff ||
            (_chkTot && !it->second->chktot(t)))
        { 
            return _null;
        }

        if (_chkHealth && chk_mask)
        {
            if (it->second->healthy())
                return it->second;
            else
                return _null;
        }
        return it->second;
    }

    shared_ptr<t_geph> t_gallnav::_find(const string &sat, const int &iod, const t_gtime &t, const bool &chk_mask)
    {
        if (_mapsat.find(sat) == _mapsat.end())
            return _null; // make_shared<t_geph>();

        GSYS gnss = t_gsys::str2gsys(sat.substr(0, 1));
        double maxdiff = t_gnav::nav_validity(gnss);

        auto it = _mapsat.at(sat).rbegin();
        while (it != _mapsat.at(sat).rend())
        {
            shared_ptr<t_gnav> pt_nav;
            if ((pt_nav = dynamic_pointer_cast<t_gnav>(it->second)) != nullptr)
            {
                if (pt_nav->iod() == iod && abs(pt_nav->epoch().diff(t)) <= maxdiff)
                {
                    break; // found
                }
            }
            it++;
        }

        // not found !
        if (it == _mapsat.at(sat).rend())
        {
            return _null; // make_shared<t_geph>();
        }

        if (_chkHealth && chk_mask)
        {
            if (it->second->healthy())
                return it->second;
            else
                return _null;
        }

        return it->second;
    }

} // namespace
