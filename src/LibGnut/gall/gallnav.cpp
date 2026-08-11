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


namespace gnut
{
    t_gallnav::t_gallnav() :
        t_gdata(),
        _com(false),
        _nepoch(static_cast<int>(t_gtime::t_tsys::GPS)),
        _multimap(false),
        _overwrite(false),
        _chkHealth(true),
        _chkNavig(true),
        _chkTot(false)
    {
        id_type(t_gdata::ID_TYPE::ALLNAV);
        id_group(t_gdata::ID_GROUP::GRP_EPHEM);
    }

    t_gallnav::~t_gallnav()
    {
        _mapsat.clear();
    }

    std::shared_ptr<t_geph> t_gallnav::find(GSAT sat, const t_gtime& t, const bool& chk_mask)
    {
        _gmutex.lock();

        std::shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, _chkHealth && chk_mask);

        _gmutex.unlock();
        return tmp;
    };
    int t_gallnav::pos(GSAT sat, const t_gtime& t, double xyz[3], double var[3], double vel[3],
                       const bool& chk_mask) // [m]
    {
        _gmutex.lock();

        std::shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, _chkHealth && chk_mask);

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
                {
                    var[i] = 0.0;
                }
                if (vel)
                {
                    vel[i] = 0.0;
                }
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

    int t_gallnav::pos(GSAT sat,
                       const int& iod,
                       const t_gtime& t,
                       double xyz[3],
                       double var[3],
                       double vel[3],
                       const bool& chk_mask) // [m]
    {
        _gmutex.lock();

        std::shared_ptr<t_geph> tmp = t_gallnav::_find(sat, iod, t, _chkHealth && chk_mask);

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
                {
                    var[i] = 0.0;
                }
                if (vel)
                {
                    vel[i] = 0.0;
                }
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

    bool t_gallnav::health(GSAT sat, const t_gtime& t)
    {
        _gmutex.lock();

        std::shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, false);

        if (tmp == _null)
        {
            _gmutex.unlock();
            return false;
        }

        bool status = tmp->healthy();

        _gmutex.unlock();
        return status;
    }

    int t_gallnav::nav(GSAT sat, const t_gtime& t, double xyz[3], double var[3], double vel[3],
                       const bool& chk_mask) // [m]
    {
        _gmutex.lock();

        std::shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, _chkHealth && chk_mask);

        if (tmp == _null)
        {
            for (int i = 0; i < 3; i++)
            {
                xyz[i] = 0.0;
                if (var)
                {
                    var[i] = 0.0;
                }
                if (vel)
                {
                    vel[i] = 0.0;
                }
            }
            _gmutex.unlock();
            return -1;
        }
        int irc = tmp->nav(t, xyz, var, vel, _chkHealth && chk_mask);

        _gmutex.unlock();
        return irc;
    }

    // ----------
    int t_gallnav::clk(GSAT sat, const t_gtime& t, double* clk, double* var, double* dclk,
                       const bool& chk_mask) // [s]
    {
        _gmutex.lock();

        std::shared_ptr<t_geph> tmp = t_gallnav::_find(sat, t, _chkHealth && chk_mask);

        if (tmp == _null)
        {
            *clk = 0.0;
            if (var)
            {
                *var = 0.0;
            }
            if (dclk)
            {
                *dclk = 0.0;
            }
            _gmutex.unlock();
            return -1;
        }

        int irc = tmp->clk(t, clk, var, dclk, _chkHealth && chk_mask);
        _gmutex.unlock();
        return irc;
    }

    int t_gallnav::clk(const std::string& id, const t_gtime& t, double* clk, double* var, double* dclk,
                       const bool& chk_mask)
    {
        GSAT sat = str2gsat(id);
        return sat == GSAT::NONE ? -1 : this->clk(sat, t, clk, var, dclk, chk_mask);
    }

    std::set<GSAT> t_gallnav::satellites() const
    {
        _gmutex.lock();

        std::set<GSAT> all_sat;
        auto itPRN = _mapsat.begin();

        while (itPRN != _mapsat.end())
        {
            all_sat.insert(itPRN->first);
            itPRN++;
        }

        _gmutex.unlock();
        return all_sat;
    }

    int t_gallnav::add(std::shared_ptr<t_gnav> nav)
    {
        _gmutex.lock();

        t_gtime ep(nav->epoch());
        GSAT sat = nav->sat();

        if (_chkNavig)
        {
            std::set<std::string> msg;
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
        else if (nav->id_type() == t_gdata::ID_TYPE::EPHGAL)
        { // check nav-type
            auto itB = _mapsat[sat].lower_bound(ep);
            auto itE = _mapsat[sat].upper_bound(ep);
            while (itB != itE)
            {
                if (std::dynamic_pointer_cast<t_gnav>(itB->second)->gnavtype() == nav->gnavtype())
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
        {
            add = false; // validity test
        }

        if (add)
        {
            _mapsat[sat].insert(std::make_pair(ep, nav));
            if (gsat2gsys(sat) == GSYS::GLO)
            {
                _gloFreqNum[sat] = nav->freq_num();
            }
        }

        _gmutex.unlock();
        return 0;
    }

    unsigned int t_gallnav::nepochs(GSAT prn)
    {
        _gmutex.lock();

        unsigned int tmp = 0;
        if (_mapsat.find(prn) != _mapsat.end())
        {
            tmp = _mapsat[prn].size();
        }

        _gmutex.unlock();
        return tmp;
    }

    void t_gallnav::clean_outer(const t_gtime& beg, const t_gtime& end)
    {
        if (end < beg)
        {
            return;
        }
        if (beg == FIRST_TIME)
        {
            return;
        }
        if (end == LAST_TIME)
        {
            return;
        }

        _gmutex.lock();

        // loop over all satellites
        auto itPRN = _mapsat.begin();
        while (itPRN != _mapsat.end())
        {
            GSAT prn = itPRN->first;

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

    int t_gallnav::nsat(const GSYS& gs) const
    {
        int nsatell = 0;
        for (auto itSAT = _mapsat.begin(); itSAT != _mapsat.end(); ++itSAT)
        {
            GSAT sat = itSAT->first;
            if (gsat2gsys(sat) == gs || gs == GSYS::GNS)
            {
                nsatell++;
            }
        }
        return nsatell;
    }

    t_iono_corr t_gallnav::get_iono_corr(const IONO_CORR& c) const
    {
        if (_brdc_iono_cor.find(c) != _brdc_iono_cor.end())
        {
            return _brdc_iono_cor.at(c);
        }
        t_iono_corr io;
        // default settings , according rtklib->rtkcmn.c->ionmodel
        if (c == IONO_CORR::IO_GPSA)
        {
            io.x0 = 0.1118e-07;
            io.x1 = -0.7451e-08;
            io.x2 = -0.5961e-07;
            io.x3 = 0.1192e-06;
        }
        else if (c == IONO_CORR::IO_GPSB)
        {
            io.x0 = 0.1167e+06;
            io.x1 = -0.2294e+06;
            io.x2 = -0.1311e+06;
            io.x3 = 0.1049e+07;
        }

        return io;
    }

    void t_gallnav::add_iono_corr(const IONO_CORR& c, const t_iono_corr& io)
    {
        if (_brdc_iono_cor.find(c) == _brdc_iono_cor.end())
        {
            _brdc_iono_cor[c] = io;
        }
    }

    std::shared_ptr<t_geph> t_gallnav::_find(GSAT sat, const t_gtime& t, const bool& chk_mask)
    {
        if (_mapsat.find(sat) == _mapsat.end())
        {
            return _null; // make_shared<t_geph>();
        }

        GSYS gnss = gsat2gsys(sat);

        if (_mapsat[sat].size() == 0)
        {
            return _null;
        }

        auto it = _mapsat[sat].lower_bound(t); // greater|equal  (can be still end())
        if (it == _mapsat[sat].end())
        {
            it--; // size() > 0 already checked above
        }

        double maxdiff = t_gnav::nav_validity(gnss) * 1.1;

        if (gnss == GSYS::GAL)
        {
            // Galileo ephemerides are valid for toc+10min -> toc+20...180min
            for (int bck = 1; bck <= 5; bck++)
            { // max eph for going back is 5
                t_gtime toc = it->second->epoch();
                if ((t < toc + 10 * 60 || t > toc + maxdiff) || (_chkTot && !it->second->chktot(t)))
                { // check ToT for using past messages only
                    if (_mapsat[sat].size() > 0 && it != _mapsat[sat].begin())
                    {
                        it--; // one more step back
                    }
                    else
                    {
                        break;
                    }
                }
                if ((t >= toc + 10 * 60 && t <= toc + maxdiff) && (_chkTot && it->second->chktot(t)))
                {
                    break;
                }
            }
        }
        else if (gnss == GSYS::BDS)
        {
            // BeiDou ephemerides are valid for toc -> toc+60min
            for (int bck = 1; bck <= 5; bck++)
            { // max eph for going back is 5
                t_gtime toc = it->second->epoch();
                if ((t < toc || t > toc + maxdiff) || (_chkTot && !it->second->chktot(t)))
                { // check ToT for using past messages only
                    if (_mapsat[sat].size() > 0 && it != _mapsat[sat].begin())
                    {
                        it--; // one more step back
                    }
                    else
                    {
                        break;
                    }
                }
                if ((t >= toc && t <= toc + maxdiff) && (_chkTot && it->second->chktot(t)))
                {
                    break;
                }
            }
        }
        else if (gnss == GSYS::GLO)
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
            {
                it = itt;
            }
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
                {
                    it--; // one more step back
                }
            }
        }

        // tested found ephemeris
        if (fabs(t - it->second->epoch()) > maxdiff || (_chkTot && !it->second->chktot(t)))
        {
            return _null;
        }

        if (_chkHealth && chk_mask)
        {
            if (it->second->healthy())
            {
                return it->second;
            }
            else
            {
                return _null;
            }
        }
        return it->second;
    }

    std::shared_ptr<t_geph> t_gallnav::_find(GSAT sat, const int& iod, const t_gtime& t, const bool& chk_mask)
    {
        if (_mapsat.find(sat) == _mapsat.end())
        {
            return _null; // make_shared<t_geph>();
        }

        GSYS gnss = gsat2gsys(sat);
        double maxdiff = t_gnav::nav_validity(gnss);

        auto it = _mapsat.at(sat).rbegin();
        while (it != _mapsat.at(sat).rend())
        {
            std::shared_ptr<t_gnav> pt_nav;
            if ((pt_nav = std::dynamic_pointer_cast<t_gnav>(it->second)) != nullptr)
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
            {
                return it->second;
            }
            else
            {
                return _null;
            }
        }

        return it->second;
    }

} // namespace gnut
