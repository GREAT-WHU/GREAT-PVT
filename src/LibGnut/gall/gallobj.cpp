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


namespace gnut
{

    t_gallobj::t_gallobj() :
        t_gdata(),
        _gpcv(0),
        _gotl(0)
    {
        id_type(t_gdata::ID_TYPE::ALLOBJ);
        id_group(t_gdata::ID_GROUP::GRP_OBJECT);
        _aloctrn();
    }
    t_gallobj::t_gallobj(t_gallpcv* pcv, t_gallotl* otl) :
        t_gdata(),
        _gpcv(pcv),
        _gotl(otl)
    {
        id_type(t_gdata::ID_TYPE::ALLOBJ);
        id_group(t_gdata::ID_GROUP::GRP_OBJECT);
        _aloctrn();
    }

    t_gallobj::~t_gallobj()
    {
        _mapobj.clear();
        return;
    }

    int t_gallobj::add(std::shared_ptr<t_gobj> obj)
    {

        std::string s = obj->id();

        // object empty
        if (obj->id().empty())
        {
            return -1;
        }

        if (_mapobj.find(s) == _mapobj.end())
        {
            _mapobj[s] = obj;
            GREAT_DEBUG("add new obj " + s);
        }
        else
        {
            GREAT_DEBUG("warning - cannot overwrite object: " + s);
            return -1;
        }

        _mapobj[s]->sync_pcv(_gpcv);

        return 0;
    }

    std::shared_ptr<t_gobj> t_gallobj::obj(const std::string& s)
    {

        std::shared_ptr<t_gobj> p_obj;

        t_map_obj::iterator it = _mapobj.find(s);
        if (it == _mapobj.end())
        {
            return p_obj;
        }
        else
        {
            p_obj = it->second;
        }

        return p_obj;
    }

    std::map<std::string, std::shared_ptr<t_gobj>> t_gallobj::objects(const t_gdata::ID_TYPE& id)
    {

        std::map<std::string, std::shared_ptr<t_gobj>> all_obj;
        t_map_obj::const_iterator itOBJ = _mapobj.begin();

        while (itOBJ != _mapobj.end())
        {
            if (id != t_gdata::ID_TYPE::NONE && id != itOBJ->second->id_type())
            {
                ++itOBJ;
                continue;
            }
            std::string site = itOBJ->second->id();
            all_obj[site] = itOBJ->second;
            ++itOBJ;
        }

        return all_obj;
    }

    void t_gallobj::sync_pcvs()
    {

        t_map_obj::const_iterator itOBJ = _mapobj.begin();

        while (itOBJ != _mapobj.end())
        {
            itOBJ->second->sync_pcv(_gpcv);
            ++itOBJ;
        }
        return;
    }

    void t_gallobj::_aloctrn()
    {
        for (const auto& system : GNSS_SATS())
        {
            for (GSAT sat : system.second)
            {
                std::string satID = gsat2str(sat);
                std::shared_ptr<t_gtrn> trn_new = std::make_shared<t_gtrn>();
                trn_new->id(satID);
                _mapobj[satID] = trn_new;
            }
        }
    }

    void t_gallobj::read_satinfo(t_gtime& epo)
    {
        std::cout << _mapobj["G01"]->name();
        t_map_obj::iterator it;
        for (it = _mapobj.begin(); it != _mapobj.end(); ++it)
        {
            if (it->second->id_type() == t_gdata::ID_TYPE::TRN)
            {
                std::string ID = it->first;
                it->second->ant(ID, epo);
            }
        }
    }

} // namespace gnut
