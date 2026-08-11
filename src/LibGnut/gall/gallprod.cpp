/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include "gall/gallprod.h"


namespace gnut
{
    t_gallprod::t_gallprod() :
        t_gdata()
    {
        id_type(t_gdata::ID_TYPE::ALLPROD);
        id_group(t_gdata::ID_GROUP::GRP_PRODUCT);
    }
    t_gallprod::~t_gallprod()
    {
        this->clear();
    }

    void t_gallprod::clear()
    {
        _map_prod.clear();
    }

    int t_gallprod::add(std::shared_ptr<t_gprod> prod, std::string site)
    {
        ID_TYPE type = prod->id_type();

        std::string id = prod->obj_id();

        if (id.empty())
        {
            id = site;
        }
        if (id.empty() && !site.empty() && id != site)
        {
            std::cerr << "warning [gallprod] - adding product with both [and different] non-zero string and object id!\n";
        }

        _map_prod[id][type][prod->epoch()] = prod; // allocate in heap

        return 0;
    }

    std::shared_ptr<t_gprod> t_gallprod::get(const std::string& site, ID_TYPE type, const t_gtime& t)
    {

        std::shared_ptr<t_gprod> obj_pt = _find(site, type, t);

        return obj_pt;
    }

    std::set<std::string> t_gallprod::prod_sites()
    {
        std::set<std::string> site_list;
        t_map_prd::const_iterator it;
        for (it = _map_prod.begin(); it != _map_prod.end(); ++it)
        {
            site_list.insert(it->first);
        }
        return site_list;
    }

    std::set<t_gtime> t_gallprod::prod_epochs(const std::string& site, ID_TYPE type)
    {
        std::set<t_gtime> epo_list;


        t_map_epo::const_iterator itEPO;
        t_map_id::const_iterator itTYPE;
        t_map_prd::const_iterator it = _map_prod.find(site);

        if (it == _map_prod.end())
        {
            return epo_list;
        }
        else
        {
            itTYPE = it->second.find(type);
            if (itTYPE == it->second.end())
            {
                return epo_list;
            }
            else
            {
                for (itEPO = itTYPE->second.begin(); itEPO != itTYPE->second.end(); ++itEPO)
                {
                    epo_list.insert(itEPO->first);
                }
            }
        }

        return epo_list;
    }

    void t_gallprod::clean_outer(const t_gtime& beg, const t_gtime& end)
    {
        if (end < beg)
        {
            return;
        }


        // loop over all sites
        t_map_prd::iterator itKEY = _map_prod.begin();
        while (itKEY != _map_prod.end())
        {
            std::string key = itKEY->first;

            t_map_id::iterator itID = itKEY->second.begin();
            while (itID != itKEY->second.end())
            {
                std::string id = t_gdata::type2str(itID->first);

                t_map_epo::iterator it;
                t_map_epo::iterator itFirst = itID->second.begin();
                t_map_epo::iterator itLast = itID->second.end();
                t_map_epo::iterator itBeg = itID->second.lower_bound(beg);
                t_map_epo::iterator itEnd = itID->second.upper_bound(end);

                // remove before BEGIN request
                if (itBeg != itFirst)
                {
                    it = itFirst;

                    // begin is last
                    if (itBeg == itLast)
                    {
                        itBeg--;

                        while (it != itLast)
                        {
                            if ((it->second).use_count() == 1)
                            {
                                itID->second.erase(it++);
                            }
                            else
                            {
                                it++;
                            }
                        }

                        // begin is not last
                    }
                    else
                    {
                        while (it != itBeg)
                        { // itID->second.erase(it++);
                            if ((it->second).use_count() == 1)
                            {
                                itID->second.erase(it++);
                            }
                            else
                            {
                                it++;
                            }
                        }
                    }
                }

                // remove after END request
                if (itEnd != itLast)
                {
                    it = itEnd;

                    while (it != itLast)
                    { // itID->second.erase(it++);
                        if ((it->second).use_count() == 1)
                        {
                            itID->second.erase(it++);
                        }
                        else
                        {
                            it++;
                        }
                    }
                }
                itID++;
            }
            itKEY++;
        }
        return;
    }

    void t_gallprod::rem(const std::string& site, ID_TYPE type, const t_gtime& t)
    {

        t_map_epo::iterator itEP;
        t_map_id::iterator itID;
        t_map_prd::iterator it = _map_prod.find(site);
        if (it != _map_prod.end())
        {
            itID = it->second.find(type);
            if (itID != it->second.end())
            {
                itEP = itID->second.find(t);
                if (itEP != itID->second.end())
                {
                    _map_prod[site][type].erase(itEP);
                }
            }
        }

        return;
    }

    std::shared_ptr<t_gprod> t_gallprod::_find(const std::string& site, ID_TYPE type, const t_gtime& t)
    {
        t_map_epo::iterator itEP;
        t_map_id::iterator itID;
        t_map_prd::iterator it = _map_prod.find(site);
        if (it != _map_prod.end())
        {
            itID = it->second.find(type);
            if (itID != it->second.end())
            {
                itEP = itID->second.find(t);
                if (itEP != itID->second.end())
                {
                    return itEP->second;
                }
            }
        }

        std::shared_ptr<t_gprod> tmp;
        return tmp;
    }

} // namespace gnut
