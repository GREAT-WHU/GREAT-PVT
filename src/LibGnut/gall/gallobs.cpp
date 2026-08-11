/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
 -*/

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "gall/gallobs.h"
#include "gio/great_log.h"


namespace gnut
{

    t_gallobs::t_gallobs() :
        t_gdata(),
        _set(0),
        _nepoch(0),
        _overwrite(false)
    {
        id_type(t_gdata::ID_TYPE::ALLOBS);
        id_group(t_gdata::ID_GROUP::GRP_OBSERV);
    }
    t_gallobs::t_gallobs(t_gsetbase* set) :
        t_gdata(),
        _set(set),
        _nepoch(0),
        _overwrite(false)
    {
        if (nullptr == set)
        {
            throw_logical_error("your set ptr is nullptr");
        }
        id_type(t_gdata::ID_TYPE::ALLOBS);
        id_group(t_gdata::ID_GROUP::GRP_OBSERV);
    }

    t_gallobs::~t_gallobs()
    {
        _mapobj.clear();
        _filter.clear();
    }

    void t_gallobs::gset(t_gsetbase* gset)
    {
        _set = gset;
        _sys = dynamic_cast<t_gsetgen*>(_set)->sys();
        _smp = dynamic_cast<t_gsetgen*>(_set)->sampling();
        _scl = dynamic_cast<t_gsetgen*>(_set)->sampling_scalefc(); // scaling 10^decimal-digits

        return;
    }

    std::set<std::string> t_gallobs::stations()
    {
        _gmutex.lock();

        std::set<std::string> all_sites;
        t_map_oobj::const_iterator itSITE = _mapobj.begin();

        while (itSITE != _mapobj.end())
        {
            all_sites.insert(itSITE->first);
            ++itSITE;
        }
        _gmutex.unlock();
        return all_sites;
    }

    std::set<GSAT> t_gallobs::sats(const std::string& site, const t_gtime& t, GSYS gnss)
    {
        _gmutex.lock();

        std::set<GSAT> all_sats = _sats(site, t, gnss);
        _gmutex.unlock();
        return all_sats;
    }

    std::set<GSAT> t_gallobs::_sats(const std::string& site, const t_gtime& t, GSYS gnss)
    {
        std::set<GSAT> all_sats;

        if (_mapobj.find(site) == _mapobj.end() || _mapobj[site].find(t) == _mapobj[site].end())
        {
            return all_sats;
        }

        t_map_osat::const_iterator itSAT = _mapobj[site][t].begin();
        while (itSAT != _mapobj[site][t].end())
        {
            GSYS sys = itSAT->second->gsys();
            if (gnss == GSYS::GNS)
            {
                all_sats.insert(itSAT->first);
            }
            else if (gnss == GSYS::GPS && sys == GSYS::GPS)
            {
                all_sats.insert(itSAT->first);
            }
            else if (gnss == GSYS::GLO && sys == GSYS::GLO)
            {
                all_sats.insert(itSAT->first);
            }
            else if (gnss == GSYS::GAL && sys == GSYS::GAL)
            {
                all_sats.insert(itSAT->first);
            }
            else if (gnss == GSYS::BDS && sys == GSYS::BDS)
            {
                all_sats.insert(itSAT->first);
            }
            else if (gnss == GSYS::QZS && sys == GSYS::QZS)
            {
                all_sats.insert(itSAT->first);
            }
            else if (gnss == GSYS::SBS && sys == GSYS::SBS)
            {
                all_sats.insert(itSAT->first);
            }
            itSAT++;
        }
        return all_sats;
    }

    std::vector<t_gsatdata> t_gallobs::obs(const std::string& site, const t_gtime& t)
    {
        _gmutex.lock();

        std::vector<t_gsatdata> all_obs = _gobs(site, t);

        _gmutex.unlock();
        return all_obs;
    }

    std::vector<t_gsatdata> t_gallobs::_gobs(const std::string& site, const t_gtime& t)
    {
        std::vector<t_gsatdata> all_obs;
        t_gtime tt(t_gtime::t_tsys::GPS);

        if (_find_epo(site, t, tt) < 0)
        {
            return all_obs;
        }

        t_map_osat::const_iterator itSAT = _mapobj[site][tt].begin();
        while (itSAT != _mapobj[site][tt].end())
        {
            t_gsatdata tmp(*itSAT->second);
            all_obs.push_back(tmp);
            itSAT++;
        }

        return all_obs;
    }

    std::vector<t_spt_gobs> t_gallobs::obs_pt(const std::string& site, const t_gtime& t)
    {
        _gmutex.lock();

        std::vector<t_spt_gobs> all_obs;
        t_gtime tt(t_gtime::t_tsys::GPS);

        if (_find_epo(site, t, tt) < 0)
        {
            _gmutex.unlock();
            return all_obs;
        }

        t_map_osat::iterator itSAT = _mapobj[site][tt].begin();
        while (itSAT != _mapobj[site][tt].end())
        {
            GSAT sat = itSAT->first;

            // TESTING NEW METHOD
            all_obs.push_back(std::dynamic_pointer_cast<t_gobsgnss>(itSAT->second));

            itSAT++;
        }

        _gmutex.unlock();
        return all_obs;
    }

    std::vector<t_gtime> t_gallobs::epochs(const std::string& site)
    {
        _gmutex.lock();

        std::vector<t_gtime> all_epochs;

        if (_mapobj.find(site) == _mapobj.end())
        {
            _gmutex.unlock();
            return all_epochs;
        }

        t_map_oref::iterator it = _mapobj[site].begin();

        while (it != _mapobj[site].end())
        {
            all_epochs.push_back(it->first);
            ++it;
        }

        _gmutex.unlock();
        return all_epochs;
    }

    t_gtime t_gallobs::beg_obs(const std::string& site, double smpl)
    {
        _gmutex.lock();

        t_gtime tmp = LAST_TIME;

        if (_mapobj.find(site) != _mapobj.end() && _mapobj[site].begin() != _mapobj[site].end())
        {
            tmp = _mapobj[site].begin()->first;
        }

        // get first synchronized obs
        if (smpl > 0.0)
        {
            auto itEpoB = _mapobj[site].begin();
            auto itEpoE = _mapobj[site].end();
            int sod = static_cast<int>(dround(itEpoB->first.sod() + itEpoB->first.dsec()));

            while (sod % static_cast<int>(smpl) != 0 && ++itEpoB != itEpoE)
            {
                sod = static_cast<int>(dround(itEpoB->first.sod() + itEpoB->first.dsec()));
                tmp = itEpoB->first;
                tmp.reset_sod();
                tmp.add_secs(sod);
            }
        }

        _gmutex.unlock();
        return tmp;
    }

    t_gtime t_gallobs::end_obs(const std::string& site)
    {
        _gmutex.lock();

        t_gtime tmp = FIRST_TIME;
        if (_mapobj.find(site) != _mapobj.end() && _mapobj[site].begin() != _mapobj[site].end())
        {
            tmp = _mapobj[site].rbegin()->first;
        }

        _gmutex.unlock();
        return tmp;
    }

    int t_gallobs::addobs(t_spt_gobs obs)
    {
        _gmutex.lock();

        // repair small out-sync ( < 10 ms )
        double outsync = fmod(obs->epoch().dsec(), _smp) - round(fmod(obs->epoch().dsec(), _smp));
        if (fabs(outsync) < 0.014 && fabs(outsync) > 1e-6)
        {
            obs->epo(obs->epoch() - outsync);
            std::vector<GOBS> v_obs = obs->obs();
            std::vector<GOBS>::iterator itOBS = v_obs.begin();
            for (; itOBS != v_obs.end(); ++itOBS)
            {
                GOBSTYPE obstype = str2gobstype(gobs2str(*itOBS));
                if (obstype == gnut::GOBSTYPE::TYPE_P || obstype == gnut::GOBSTYPE::TYPE_C)
                {
                    obs->resetobs(*itOBS, obs->getobs(*itOBS) - CLIGHT * outsync);
                }
                else if (obstype == gnut::GOBSTYPE::TYPE_L)
                {
                    obs->resetobs(*itOBS, obs->getobs(*itOBS) - CLIGHT * outsync / obs->wavelength(str2gobsband(gobs2str(*itOBS))));
                }
                else if (obstype == gnut::GOBSTYPE::TYPE_S)
                { // add wh
                    obs->resetobs(*itOBS, obs->getobs(*itOBS));
                }
            }
        }

        t_gtime t(obs->epoch()), tt = t;
        std::string site = obs->site();
        GSAT sat = obs->sat();

        if (_map_sites.find(site) == _map_sites.end())
        {
            _map_sites.insert(site);
        }

        int epo_found = _find_epo(site, t, tt);
        auto itSAT = _mapobj[site][tt].find(sat);

        if (_overwrite || epo_found < 0 || itSAT == _mapobj[site][tt].end())
        {
            if (_overwrite && epo_found > 0 && itSAT != _mapobj[site][tt].end())
            {
                _mapobj[site][tt].erase(sat);
            }

            if (_nepoch > 0 && _mapobj[site].size() > _nepoch + 10)
            {
                auto itEPO = _mapobj[site].begin();
                while (itEPO != _mapobj[site].end())
                {
                    if (_mapobj[site].size() <= _nepoch)
                    {
                        break;
                    }

                    t_gtime t = itEPO->first;

                    _mapobj[site][t].erase(_mapobj[site][t].begin(), _mapobj[site][t].end());
                    _mapobj[site].erase(itEPO++);
                }
            }

            if (obs->id_type() == t_gdata::ID_TYPE::OBSGNSS)
            {
                _mapobj[site][tt][sat] = obs;
            }
            else
            {
                GREAT_WARN("warning: t_gobsgnss record not identified!");
                _gmutex.unlock();
                return 1;
            }
        }
        else
        {
            _gmutex.unlock();
            return 0;
        }

        _gmutex.unlock();
        return 0;
    }

    t_gallobs::t_map_osat t_gallobs::find(const std::string& site, const t_gtime& t)
    {
        _gmutex.lock();

        t_gtime tt(t_gtime::t_tsys::GPS);
        t_map_osat tmp;
        if (_find_epo(site, t, tt) < 0)
        {
            _gmutex.unlock();
            return tmp;
        }

        _gmutex.unlock();
        return _mapobj[site][tt];
    }

    t_gallobs::t_map_frq t_gallobs::frqobs(const std::string& site)
    {
        _gmutex.lock();

        t_map_frq mfrq;

        if (_mapobj.find(site) == _mapobj.end())
        {
            _gmutex.unlock();
            return mfrq;
        }

        t_map_oref::const_iterator itEpo = _mapobj[site].begin();
        t_map_osat::iterator itSat;

        t_gtime t;

        while (itEpo != _mapobj[site].end())
        {
            t = itEpo->first;
            for (itSat = _mapobj[site][t].begin(); itSat != _mapobj[site][t].end(); itSat++)
            { // loop over satellites
                GSAT prn = itSat->first;
                t_spt_gobs obs = itSat->second;

                std::vector<GOBS> vgobs = obs->obs();
                for (std::vector<GOBS>::iterator it = vgobs.begin(); it != vgobs.end(); ++it)
                {
                    GOBS gobs = *it;
                    int bnd = gobs2band(gobs);
                    GOBSBAND bend = int2gobsband(bnd);
                    mfrq[prn][bend][gobs]++;
                }
            }
            itEpo++;
        }

        _gmutex.unlock();
        return mfrq;
    }

    void t_gallobs::xdata(const std::string& site, const std::string& file, const t_xfilter& xflt)
    {
        _gmutex.lock();

        _filter[site][file].xdat = xflt.xdat;
        _filter[site][file].beg = xflt.beg;
        _filter[site][file].end = xflt.end;

        _gmutex.unlock();
    }

    unsigned int t_gallobs::nepochs(const std::string& site)
    {
        _gmutex.lock();

        if (_mapobj.find(site) == _mapobj.end())
        {
            _gmutex.unlock();
            return 0;
        }

        unsigned int tmp = _mapobj[site].size();

        _gmutex.unlock();
        return tmp;
    }

    bool t_gallobs::isSite(const std::string& site)
    {
        _gmutex.lock();

        bool tmp = false;
        if (_mapobj.find(site) != _mapobj.end())
        {
            tmp = true;
        }

        _gmutex.unlock();
        return tmp;
    }

    int t_gallobs::_find_epo(const std::string& site, const t_gtime& epo, t_gtime& tt)
    {
        if (_mapobj.find(site) == _mapobj.end())
        {
            return -1;
        }

        t_map_oref::iterator it1 = _mapobj[site].lower_bound(epo); // greater || equal
        t_map_oref::iterator it0 = it1;                            // previous value

        if (it0 != _mapobj[site].begin())
        {
            it0--;
        }
        if (it1 == _mapobj[site].end())
        {
            it1 = it0;
        }

        if (it1 == _mapobj[site].end() && it0 == _mapobj[site].end())
        {
            return -1;
        }
        if (fabs(it1->first - epo) <= DIFF_SEC(_smp))
        {
            tt = it1->first; // it = it1;                 // set closest value
        }
        else if (fabs(it0->first - epo) <= DIFF_SEC(_smp))
        {
            tt = it0->first; // it = it0;                 // set closest value
        }
        else
        {
            return -1; // not found !
        }

        return 1;
    }

} // namespace gnut
