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

using namespace std;

namespace gnut
{
    t_gallbias::t_gallbias() : t_gdata(), _isOverWrite(false)
    {
        id_type(t_gdata::ALLBIAS);
        id_group(t_gdata::GRP_MODEL);

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

    t_gallbias::t_gallbias(t_spdlog spdlog)
        : t_gdata(spdlog), _isOverWrite(false)
    {
        id_type(t_gdata::ALLBIAS);
        id_group(t_gdata::GRP_MODEL);

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

    void t_gallbias::add(const string &ac, const t_gtime &epo, const string &obj, t_spt_bias pt_cb)
    {

        _gmutex.lock();

        if (pt_cb == nullptr)
        {
            _gmutex.unlock();
            return;
        }

        if (pt_cb->ref() == X)
        { 
            _mapBias[ac][epo][obj][pt_cb->gobs()] = pt_cb;
        }
        else
        { 
            if (_mapBias[ac][epo][obj].size() == 0)
            {
                shared_ptr<t_gbias> pt_ref = make_shared<t_gbias>(_spdlog);               // create first reference bias
                pt_ref->set(pt_cb->beg(), pt_cb->end(), 0.0, pt_cb->ref(), pt_cb->ref()); // reference bias is set up to zero
                _mapBias[ac][epo][obj][pt_ref->gobs()] = pt_ref;                          // store new bias (reference)
                _mapBias[ac][epo][obj][pt_cb->gobs()] = pt_cb;                            // store new bias
            }
            else
            {
                t_spt_bias pt_obs1 = _find(ac, epo, obj, pt_cb->gobs());
                t_spt_bias pt_obs2 = _find(ac, epo, obj, pt_cb->ref());
                if (pt_obs1 != nullptr && pt_obs2 == nullptr)
                { // connection with first signal
                    _connect_first(pt_obs1, pt_cb);
                    _mapBias[ac][epo][obj][pt_cb->gobs()] = pt_cb; // store modified bias
                }
                else if (pt_obs1 == nullptr && pt_obs2 != nullptr)
                { // connection with second signal
                    _connect_second(pt_obs2, pt_cb);
                    _mapBias[ac][epo][obj][pt_cb->gobs()] = pt_cb; // store modified bias
                }
                else if (pt_obs1 != nullptr && pt_obs2 != nullptr)
                {
                    // connectin two groups with different reference signal
                    // connection with first signal
                    _connect_first(pt_obs1, pt_cb);
                    // all biases connected with second signal need to be consolidated
                    _consolidate(ac, obj, pt_cb, pt_obs2);
                }
                else
                {
                    // glfeng add for GAL to store Q & X DCB bias
                    shared_ptr<t_gbias> pt_ref = make_shared<t_gbias>(_spdlog);               // create first reference bias
                    pt_ref->set(pt_cb->beg(), pt_cb->end(), 0.0, pt_cb->ref(), pt_cb->ref()); // reference bias is set up to zero
                    _mapBias[ac][epo][obj][pt_ref->gobs()] = pt_ref;                          // store new bias (reference)
                    _mapBias[ac][epo][obj][pt_cb->gobs()] = pt_cb;                            // store new bias
                }
            }
        }

        _gmutex.unlock();
        return;
    }

    double t_gallbias::get(const string &prd, const t_gtime &epo, const string &prn, const GOBS &gobs, const bool &meter)
    {
        _gmutex.lock();

        double bias = 999.0;
        auto itAC = _mapBias.find(prd);
        if (itAC != _mapBias.end())
        {
            auto itEPO = itAC->second.upper_bound(epo);
            if (itEPO != itAC->second.begin() && itEPO != itAC->second.end())
                itEPO--; // between epochs
            if (itEPO == itAC->second.end() && itAC->second.size() != 0)
                itEPO--; // no epochs

            if (itEPO != itAC->second.end())
            {
                auto itSAT = itEPO->second.find(prn);
                if (itSAT != itEPO->second.end() && itSAT->second.find(gobs) != itSAT->second.end())
                {
                    t_spt_bias pobs1 = itSAT->second.find(gobs)->second;
                    bias = pobs1->bias();
                }
            }
        }

        _gmutex.unlock();
        return bias;
    }

    vector<string> t_gallbias::get_ac()
    {
        vector<string> ac_list;
        for (const auto &item : _mapBias)
        {
            ac_list.push_back(item.first);
        }
        return ac_list;
    }

    string t_gallbias::get_ac_priority()
    {
        string used_ac;
        int loc = 999;
        const map<string, int> ac_order{
            {"COD_A", 1}, {"CAS_A", 2}, {"WHU_A", 3}, {"DLR_A", 4}, {"CAS_R", 5}, {"COD_R", 6}, {"WHU_R", 7}, {"DLR_R", 8}, {"CNT_A", 9}, {"RTB_A", 10}, {"SGG_A", 11}};
        for (auto item : _mapBias)
        {
            string ac = (item.first == "WHU_A_PHASE") ? "WHU_A" : item.first;
            if (ac_order.at(ac) < loc)
            {
                used_ac = ac;
                loc = ac_order.at(item.first);
            }
        }
        return used_ac;
    }

    string t_gallbias::get_used_ac()
    {
        if (_acUsed.empty())
            _acUsed = get_ac_priority();
        return _acUsed;
    }

    double t_gallbias::get(const t_gtime &epo, const string &obj, const GOBS &gobs1, const GOBS &gobs2, const string &tmp)
    {

        _gmutex.lock();

        double dcb = 0.0;
        string ac(tmp);
        if (ac == "" && _isOrdered == true)
            ac = _acPri;

        if (ac == "" && _isOrdered == false)
        {
            int loc = 999;
            for (const auto &item : _mapBias)
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
            this->_convert_obstype(ac, obj, gobs1_convert);
            t_spt_bias pobs1 = _find(ac, epo, obj, gobs1_convert);
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
            this->_convert_obstype(ac, obj, gobs1_convert);
            this->_convert_obstype(ac, obj, gobs2_convert);

            t_spt_bias pobs1 = _find(ac, epo, obj, gobs1_convert);
            t_spt_bias pobs2 = _find(ac, epo, obj, gobs2_convert);

            if (pobs1 != nullptr && pobs2 != nullptr && pobs1->ref() == pobs2->ref())
            {
                dcb = pobs1->bias() - pobs2->bias();
            }
            if (pobs1 != nullptr && pobs2 != nullptr && pobs2->gobs()==C6C)
            {
                dcb = pobs1->bias() - pobs2->bias();
            }
        }

        _gmutex.unlock();
        return dcb;
    }

    t_spt_bias t_gallbias::_find(const string &ac, const t_gtime &epo, const string &obj, const GOBS &gobs)
    {
        t_spt_bias pt_bias = nullptr;
        GOBS obs = gobs;
        auto itAC = _mapBias.find(ac);
        if (itAC != _mapBias.end())
        {
            auto itEPO = itAC->second.upper_bound(epo);
            if (itEPO != itAC->second.begin() && itEPO != itAC->second.end())
                itEPO--; // between epochs
            if (itEPO == itAC->second.end() && itAC->second.size() != 0)
                itEPO--; // no epochs

            if (itEPO != itAC->second.end())
            {
                auto itOBJ = itEPO->second.find(obj);
                if (itOBJ != itEPO->second.end())
                {

                    auto itGOBS = itOBJ->second.find(obs);
                    if (itGOBS == itOBJ->second.end() && obs==C6X)
                    {
                        obs = C6C;
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

    vector<t_spt_bias> t_gallbias::_find_ref(const string &ac, const t_gtime &epo, const string &obj, const GOBS &ref)
    {
        vector<t_spt_bias> vec_bias;

        auto itAC = _mapBias.find(ac);
        if (itAC != _mapBias.end())
        {
            auto itEPO = itAC->second.find(epo);
            if (itEPO != itAC->second.end())
            {
                auto itOBJ = itEPO->second.find(obj);
                if (itOBJ != itEPO->second.end())
                {
                    for (auto itGOBS = itOBJ->second.begin(); itGOBS != itOBJ->second.end(); itGOBS++)
                    {
                        if (itGOBS->second->ref() == ref)
                            vec_bias.push_back(itGOBS->second);
                    }
                }
            }
        }

        return vec_bias;
    }

    void t_gallbias::_convert_obstype(const string &ac, const string &obj, GOBS &obstype)
    {
        if (ac == "COD_R")
        {
            if (obj[0] == 'G' || obj[0] == 'R' || obj[0] == '2' || obj[0] == '3' || obj[0] == '4' || obj[0] == '5')
            {
                switch (obstype)
                {
                case C1C:
                    obstype = C1;
                    break;
                case C1P:
                case C1Y:
                case C1W:
                    obstype = P1;
                    break;
                case C2C:
                    obstype = C2;
                    break;
                case C2P:
                case C2Y:
                case C2W:
                    obstype = P2;
                    break;
                default:
                    break;
                }
            }
        }
        else if (ac == "CAS_R")
        {
            if (obj[0] == 'G' || obj[0] == '2' || obj[0] == '3' || obj[0] == '4' || obj[0] == '5')
            {
                switch (obstype)
                {
                case P1:
                    obstype = C1W;
                    break;
                case P2:
                    obstype = C2W;
                    break;
                case C1:
                    obstype = C1C;
                    break;
                case C2:
                    obstype = C2C;
                    break;
                default:
                    break;
                }
            }
            if (obj[0] == 'R')
            {
                switch (obstype)
                {
                case P1:
                    obstype = C1P;
                    break;
                case P2:
                    obstype = C2P;
                    break;
                case C1:
                    obstype = C1C;
                    break;
                case C2:
                    obstype = C2C;
                    break;
                default:
                    break;
                }
            }
        }
    }

    void t_gallbias::_connect_first(const t_spt_bias &pt_cb1, const t_spt_bias &pt_cb2)
    {
        double newval = pt_cb1->bias() - pt_cb2->bias();
        pt_cb2->set(newval, pt_cb2->ref(), pt_cb1->ref());
    }

    void t_gallbias::_connect_second(const t_spt_bias &pt_cb1, const t_spt_bias &pt_cb2)
    {
        double newval = pt_cb1->bias() + pt_cb2->bias();
        pt_cb2->set(newval, pt_cb2->gobs(), pt_cb1->ref());
    }

    void t_gallbias::_consolidate(const string &ac, const string &obj, const t_spt_bias &pt_cb1, const t_spt_bias &pt_cb2)
    {
        double diff = pt_cb2->val() - pt_cb1->val();
        t_gtime epo = pt_cb1->beg();
        vector<t_spt_bias> vec = _find_ref(ac, epo, obj, pt_cb2->ref());

        for (auto itSPT = vec.begin(); itSPT != vec.end(); itSPT++)
        {
            double newval = (*itSPT)->bias() - diff;
            GOBS gobs = (*itSPT)->gobs();
            GOBS newref = pt_cb1->ref();
            (*itSPT)->set(newval, gobs, newref);
        }
    }

} // namespace
