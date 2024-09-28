/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include "gall/gallobj.h"

using namespace std;

namespace gnut
{

    t_gallobj::t_gallobj()
        : t_gdata(),
          _gpcv(0),
          _gotl(0)
    {
        id_type(t_gdata::ALLOBJ);
        id_group(t_gdata::GRP_OBJECT);
        _aloctrn();
    }
    t_gallobj::t_gallobj(t_spdlog spdlog)
        : t_gdata(spdlog),
          _gpcv(0),
          _gotl(0)
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
        id_type(t_gdata::ALLOBJ);
        id_group(t_gdata::GRP_OBJECT);
        _aloctrn();
    }

    t_gallobj::t_gallobj(t_gallpcv *pcv, t_gallotl *otl) : t_gdata(),
                                                           _gpcv(pcv),
                                                           _gotl(otl)
    {

        id_type(t_gdata::ALLOBJ);
        id_group(t_gdata::GRP_OBJECT);
        _aloctrn();
    }

    t_gallobj::t_gallobj(t_spdlog spdlog, t_gallpcv *pcv, t_gallotl *otl) : t_gdata(spdlog),
                                                                            _gpcv(pcv),
                                                                            _gotl(otl)
    {
        id_type(t_gdata::ALLOBJ);
        id_group(t_gdata::GRP_OBJECT);
        if (nullptr == spdlog)
        {
            spdlog::critical("your spdlog is nullptr !");
            throw logic_error("");
        }
        else
        {
            _spdlog = spdlog;
        }
        _aloctrn();
    }

    t_gallobj::~t_gallobj()
    {
        _gmutex.lock();
        _mapobj.clear();
        _gmutex.unlock();
        return;
    }

    int t_gallobj::add(shared_ptr<t_gobj> obj)
    {

        _gmutex.lock();

        string s = obj->id();

        // object empty
        if (obj->id().empty())
        {
            _gmutex.unlock();
            return -1;
        }

        if (_mapobj.find(s) == _mapobj.end())
        {

            _mapobj[s] = obj;

            _mapobj[s]->spdlog(_spdlog);
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "add new obj " + s);
        }
        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "warning - cannot overwrite object: " + s);
            _gmutex.unlock();
            return -1;
        }

        _mapobj[s]->sync_pcv(_gpcv);

        _gmutex.unlock();
        return 0;
    }

    shared_ptr<t_gobj> t_gallobj::obj(const string &s)
    {
        _gmutex.lock();

        shared_ptr<t_gobj> p_obj;

        t_map_obj::iterator it = _mapobj.find(s);
        if (it == _mapobj.end())
        {
            _gmutex.unlock();
            return p_obj;
        }
        else
        {
            p_obj = it->second;
        }

        _gmutex.unlock();
        return p_obj;
    }

    map<string, shared_ptr<t_gobj>> t_gallobj::objects(const t_gdata::ID_TYPE &id)
    {
        _gmutex.lock();

        map<string, shared_ptr<t_gobj>> all_obj;
        t_map_obj::const_iterator itOBJ = _mapobj.begin();

        while (itOBJ != _mapobj.end())
        {
            if (id != NONE && id != itOBJ->second->id_type())
            {
                ++itOBJ;
                continue;
            } 
            string site = itOBJ->second->id();
            all_obj[site] = itOBJ->second;
            ++itOBJ;
        }

        _gmutex.unlock();
        return all_obj;
    }

    void t_gallobj::sync_pcvs()
    {
        _gmutex.lock();

        t_map_obj::const_iterator itOBJ = _mapobj.begin();

        while (itOBJ != _mapobj.end())
        {
            itOBJ->second->sync_pcv(_gpcv);
            ++itOBJ;
        }
        _gmutex.unlock();
        return;
    }

    void t_gallobj::_aloctrn()
    {

        for (int i = 0; i != GNS; ++i)
        {
            GSYS sys = static_cast<GSYS>(i);
            set<string> sats = GNSS_SATS()[sys];
            for (auto iter = sats.begin(); iter != sats.end(); ++iter)
            {
                string satID = *iter;
                shared_ptr<t_gtrn> trn_new = make_shared<t_gtrn>(_spdlog);
                trn_new->id(satID);
                _mapobj[satID] = trn_new;
            }
        }
    }

    void t_gallobj::read_satinfo(t_gtime &epo)
    {

        cout << _mapobj["G01"]->name();
        t_map_obj::iterator it;
        for (it = _mapobj.begin(); it != _mapobj.end(); ++it)
        {
            if (it->second->id_type() == TRN)
            {
                string ID = it->first;
                it->second->ant(ID, epo);
            }
        }
    }

} // namespace
