/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include "gall/gallbias.h"


namespace gnut
{
    t_gallbias::t_gallbias() :
        t_gdata(),
        _isOverWrite(false)
    {
        id_type(t_gdata::ID_TYPE::ALLBIAS);
        id_group(t_gdata::ID_GROUP::GRP_MODEL);

        _acOrder["COD_A"] = 1;
        _acOrder["CAS_A"] = 2;
        _acOrder["WHU_A"] = 3;
        _acOrder["DLR_A"] = 4;
        _acOrder["CAS_R"] = 5;
        _acOrder["COD_R"] = 6;
        _acOrder["WHU_R"] = 7;
        _acOrder["DLR_R"] = 8;
        _acOrder["CNT_A"] = 9;
        _acOrder["RTB_A"] = 10;
    }

    t_gallbias::~t_gallbias()
    {
        _mapBias.clear();
    }

    void t_gallbias::add(const std::string& ac, const t_gtime& epo, GSAT sat, t_spt_bias pt_cb)
    {

        if (pt_cb == nullptr)
        {
            return;
        }

        if (pt_cb->ref() == gnut::GOBS::X)
        {
            _mapBias[ac][epo][sat][pt_cb->gobs()] = pt_cb;
        }
        else
        {
            if (_mapBias[ac][epo][sat].size() == 0)
            {
                std::shared_ptr<t_gbias> pt_ref = std::make_shared<t_gbias>(); // create first reference bias
                pt_ref->set(pt_cb->beg(), pt_cb->end(), 0.0, pt_cb->ref(),
                            pt_cb->ref());                       // reference bias is set up to zero
                _mapBias[ac][epo][sat][pt_ref->gobs()] = pt_ref; // store new bias (reference)
                _mapBias[ac][epo][sat][pt_cb->gobs()] = pt_cb;   // store new bias
            }
            else
            {
                t_spt_bias pt_obs1 = _find(ac, epo, sat, pt_cb->gobs());
                t_spt_bias pt_obs2 = _find(ac, epo, sat, pt_cb->ref());
                if (pt_obs1 != nullptr && pt_obs2 == nullptr)
                { // connection with first signal
                    _connect_first(pt_obs1, pt_cb);
                    _mapBias[ac][epo][sat][pt_cb->gobs()] = pt_cb; // store modified bias
                }
                else if (pt_obs1 == nullptr && pt_obs2 != nullptr)
                { // connection with second signal
                    _connect_second(pt_obs2, pt_cb);
                    _mapBias[ac][epo][sat][pt_cb->gobs()] = pt_cb; // store modified bias
                }
                else if (pt_obs1 != nullptr && pt_obs2 != nullptr)
                {
                    // connectin two groups with different reference signal
                    // connection with first signal
                    _connect_first(pt_obs1, pt_cb);
                    // all biases connected with second signal need to be consolidated
                    _consolidate(ac, sat, pt_cb, pt_obs2);
                }
                else
                {
                    // glfeng add for GAL to store Q & X DCB bias
                    std::shared_ptr<t_gbias> pt_ref = std::make_shared<t_gbias>(); // create first reference bias
                    pt_ref->set(pt_cb->beg(), pt_cb->end(), 0.0, pt_cb->ref(),
                                pt_cb->ref());                       // reference bias is set up to zero
                    _mapBias[ac][epo][sat][pt_ref->gobs()] = pt_ref; // store new bias (reference)
                    _mapBias[ac][epo][sat][pt_cb->gobs()] = pt_cb;   // store new bias
                }
            }
        }

        return;
    }

    double t_gallbias::get(const std::string& prd, const t_gtime& epo, GSAT sat, const GOBS& gobs, const bool& meter)
    {

        double bias = 999.0;
        auto itAC = _mapBias.find(prd);
        if (itAC != _mapBias.end())
        {
            auto itEPO = itAC->second.upper_bound(epo);
            if (itEPO != itAC->second.begin() && itEPO != itAC->second.end())
            {
                itEPO--; // between epochs
            }
            if (itEPO == itAC->second.end() && itAC->second.size() != 0)
            {
                itEPO--; // no epochs
            }

            if (itEPO != itAC->second.end())
            {
                auto itSAT = itEPO->second.find(sat);
                if (itSAT != itEPO->second.end() && itSAT->second.find(gobs) != itSAT->second.end())
                {
                    t_spt_bias pobs1 = itSAT->second.find(gobs)->second;
                    bias = pobs1->bias();
                }
            }
        }

        return bias;
    }

    std::vector<std::string> t_gallbias::get_ac()
    {
        std::vector<std::string> ac_list;
        for (const auto& item : _mapBias)
        {
            ac_list.push_back(item.first);
        }
        return ac_list;
    }

    std::string t_gallbias::get_ac_priority()
    {
        std::string used_ac;
        int loc = 999;
        const std::map<std::string, int> ac_order{{"COD_A", 1},
                                        {"CAS_A", 2},
                                        {"WHU_A", 3},
                                        {"DLR_A", 4},
                                        {"CAS_R", 5},
                                        {"COD_R", 6},
                                        {"WHU_R", 7},
                                        {"DLR_R", 8},
                                        {"CNT_A", 9},
                                        {"RTB_A", 10},
                                        {"SGG_A", 11}};
        for (auto item : _mapBias)
        {
            std::string ac = (item.first == "WHU_A_PHASE") ? "WHU_A" : item.first;
            if (ac_order.at(ac) < loc)
            {
                used_ac = ac;
                loc = ac_order.at(item.first);
            }
        }
        return used_ac;
    }

    std::string t_gallbias::get_used_ac()
    {
        if (_acUsed.empty())
        {
            _acUsed = get_ac_priority();
        }
        return _acUsed;
    }

    double t_gallbias::get(const t_gtime& epo, GSAT sat, const GOBS& gobs1, const GOBS& gobs2, const std::string& tmp)
    {

        double dcb = 0.0;
        std::string ac(tmp);
        if (ac == "" && _isOrdered == true)
        {
            ac = _acPri;
        }

        if (ac == "" && _isOrdered == false)
        {
            int loc = 999;
            for (const auto& item : _mapBias)
            {
                if (_acOrder.at(item.first) < loc)
                {
                    ac = item.first;
                    loc = _acOrder.at(item.first);
                }
            }

            _isOrdered = true;
            _acPri = ac;
        }

        if (gobs2 == gobs1)
        {
            GOBS gobs1_convert = gobs1;
            this->_convert_obstype(ac, sat, gobs1_convert);
            t_spt_bias pobs1 = _find(ac, epo, sat, gobs1_convert);
            if (pobs1 != nullptr)
            {
                dcb = pobs1->bias();
            }
        }
        else
        {
            GOBS gobs1_convert = gobs1;
            GOBS gobs2_convert = gobs2;

            // align obstype to ac(code or cas)
            this->_convert_obstype(ac, sat, gobs1_convert);
            this->_convert_obstype(ac, sat, gobs2_convert);

            t_spt_bias pobs1 = _find(ac, epo, sat, gobs1_convert);
            t_spt_bias pobs2 = _find(ac, epo, sat, gobs2_convert);

            if (pobs1 != nullptr && pobs2 != nullptr && pobs1->ref() == pobs2->ref())
            {
                dcb = pobs1->bias() - pobs2->bias();
            }
            if (pobs1 != nullptr && pobs2 != nullptr && pobs2->gobs() == gnut::GOBS::C6C)
            {
                dcb = pobs1->bias() - pobs2->bias();
            }
        }

        return dcb;
    }

    t_spt_bias t_gallbias::_find(const std::string& ac, const t_gtime& epo, GSAT sat, const GOBS& gobs)
    {
        t_spt_bias pt_bias = nullptr;
        GOBS obs = gobs;
        auto itAC = _mapBias.find(ac);
        if (itAC != _mapBias.end())
        {
            auto itEPO = itAC->second.upper_bound(epo);
            if (itEPO != itAC->second.begin() && itEPO != itAC->second.end())
            {
                itEPO--; // between epochs
            }
            if (itEPO == itAC->second.end() && itAC->second.size() != 0)
            {
                itEPO--; // no epochs
            }

            if (itEPO != itAC->second.end())
            {
                auto itOBJ = itEPO->second.find(sat);
                if (itOBJ != itEPO->second.end())
                {
                    auto itGOBS = itOBJ->second.find(obs);
                    if (itGOBS == itOBJ->second.end() && obs == gnut::GOBS::C6X)
                    {
                        obs = gnut::GOBS::C6C;
                        itGOBS = itOBJ->second.find(obs);
                    }
                    if (itGOBS != itOBJ->second.end())
                    {
                        if (itGOBS->second->valid(epo))
                        {
                            pt_bias = itGOBS->second;
                        }
                    }
                }
            }
        }

        return pt_bias;
    }

    std::vector<t_spt_bias> t_gallbias::_find_ref(const std::string& ac, const t_gtime& epo, GSAT sat, const GOBS& ref)
    {
        std::vector<t_spt_bias> vec_bias;

        auto itAC = _mapBias.find(ac);
        if (itAC != _mapBias.end())
        {
            auto itEPO = itAC->second.find(epo);
            if (itEPO != itAC->second.end())
            {
                auto itOBJ = itEPO->second.find(sat);
                if (itOBJ != itEPO->second.end())
                {
                    for (auto itGOBS = itOBJ->second.begin(); itGOBS != itOBJ->second.end(); itGOBS++)
                    {
                        if (itGOBS->second->ref() == ref)
                        {
                            vec_bias.push_back(itGOBS->second);
                        }
                    }
                }
            }
        }

        return vec_bias;
    }

    void t_gallbias::_convert_obstype(const std::string& ac, GSAT sat, GOBS& obstype)
    {
        if (ac == "COD_R")
        {
            if (gsat2gsys(sat) == GSYS::GPS || gsat2gsys(sat) == GSYS::GLO)
            {
                switch (obstype)
                {
                    case gnut::GOBS::C1C:
                        obstype = gnut::GOBS::C1;
                        break;
                    case gnut::GOBS::C1P:
                    case gnut::GOBS::C1Y:
                    case gnut::GOBS::C1W:
                        obstype = gnut::GOBS::P1;
                        break;
                    case gnut::GOBS::C2C:
                        obstype = gnut::GOBS::C2;
                        break;
                    case gnut::GOBS::C2P:
                    case gnut::GOBS::C2Y:
                    case gnut::GOBS::C2W:
                        obstype = gnut::GOBS::P2;
                        break;
                    default:
                        break;
                }
            }
        }
        else if (ac == "CAS_R")
        {
            if (gsat2gsys(sat) == GSYS::GPS)
            {
                switch (obstype)
                {
                    case gnut::GOBS::P1:
                        obstype = gnut::GOBS::C1W;
                        break;
                    case gnut::GOBS::P2:
                        obstype = gnut::GOBS::C2W;
                        break;
                    case gnut::GOBS::C1:
                        obstype = gnut::GOBS::C1C;
                        break;
                    case gnut::GOBS::C2:
                        obstype = gnut::GOBS::C2C;
                        break;
                    default:
                        break;
                }
            }
            if (gsat2gsys(sat) == GSYS::GLO)
            {
                switch (obstype)
                {
                    case gnut::GOBS::P1:
                        obstype = gnut::GOBS::C1P;
                        break;
                    case gnut::GOBS::P2:
                        obstype = gnut::GOBS::C2P;
                        break;
                    case gnut::GOBS::C1:
                        obstype = gnut::GOBS::C1C;
                        break;
                    case gnut::GOBS::C2:
                        obstype = gnut::GOBS::C2C;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    void t_gallbias::_connect_first(const t_spt_bias& pt_cb1, const t_spt_bias& pt_cb2)
    {
        double newval = pt_cb1->bias() - pt_cb2->bias();
        pt_cb2->set(newval, pt_cb2->ref(), pt_cb1->ref());
    }

    void t_gallbias::_connect_second(const t_spt_bias& pt_cb1, const t_spt_bias& pt_cb2)
    {
        double newval = pt_cb1->bias() + pt_cb2->bias();
        pt_cb2->set(newval, pt_cb2->gobs(), pt_cb1->ref());
    }

    void t_gallbias::_consolidate(const std::string& ac, GSAT sat, const t_spt_bias& pt_cb1, const t_spt_bias& pt_cb2)
    {
        double diff = pt_cb2->val() - pt_cb1->val();
        t_gtime epo = pt_cb1->beg();
        std::vector<t_spt_bias> vec = _find_ref(ac, epo, sat, pt_cb2->ref());

        for (auto itSPT = vec.begin(); itSPT != vec.end(); itSPT++)
        {
            double newval = (*itSPT)->bias() - diff;
            GOBS gobs = (*itSPT)->gobs();
            GOBS newref = pt_cb1->ref();
            (*itSPT)->set(newval, gobs, newref);
        }
    }

} // namespace gnut
