/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/

#include <iostream>
#include <iomanip>
#include <cmath>
#include <assert.h>

#include "gall/gallpar.h"
#include <vector>

using namespace std;

namespace gnut
{
    void t_gallpar::addParam(const t_gpar &newPar)
    {

        this->_vParam.push_back(newPar);
        this->_point_par.push_back(_max_point++);
        this->_index_par[newPar.get_head()][newPar.get_timearc()] = this->_point_par[this->_point_par.size() - 1];
    }

    void t_gallpar::delParam(const int &i)
    {

        auto &all = this->_index_par.at(_vParam[i].get_head());
        for (auto iter = all.begin(); iter != all.end(); ++iter)
        {
            if (iter->second == _point_par[i])
            {
                all.erase(iter);
                break;
            }
        }
        if (this->_index_par[_vParam[i].get_head()].size() == 0)
        {
            this->_index_par.erase(_vParam[i].get_head());
        }
        _point_par.erase(_point_par.begin() + i);

        _vParam.erase(_vParam.begin() + i);
    }

    int t_gallpar::getParam(const string &mark, const par_type &type, const string &prn,
                            const t_gtime &beg, const t_gtime &end) const
    {
        if (this->_index_par.count(t_gparhead(type, mark, prn)) == 0)
        {
            return -1;
        }
        else
        {
            const auto &all = this->_index_par.find(t_gparhead(type, mark, prn))->second;
            t_gtimearc dst_timearc(beg, end);

            auto all_end = all.end();
            auto par_beg = _point_par.begin();
            auto par_end = _point_par.end();
            for (auto iter = all.begin(); iter != all_end; ++iter)
            {
                if (iter->first.inside(dst_timearc))
                {
                    auto ans = lower_bound(par_beg, par_end, iter->second);
                    assert(ans != par_end && *ans == iter->second);
                    return ans - par_beg;
                }
            }
            return -1;
        }
        return -1;
    }

    int t_gallpar::getParam(const int &index)
    {

        for (unsigned int i = 0; i <= _vParam.size() - 1; i++)
        {
            if (_vParam[i].index == index)
            {
                return i;
            }
        }
        return -1;
    }

    int t_gallpar::getParIndex(const int &idx)
    {
        if (idx >= 0 && idx < _vParam.size())
        {
            return _vParam[idx].index;
        }
        else
        {
            return -1;
        }
    }

    double t_gallpar::getParValue(const int &idx)
    {
        if (idx >= 0 && idx < _vParam.size())
        {
            return _vParam[idx].value();
        }
        else
        {
            return 0.0;
        }
    }

    void t_gallpar::setParValue(const int &idx, const double &value)
    {
        if (idx >= 0 && idx < _vParam.size())
        {
            _vParam[idx].value(value);
        }
    }

    void t_gallpar::reIndex()
    {

        int index_new = 1;
        for (unsigned int iPar = 0; iPar <= _vParam.size() - 1; iPar++)
        {
            _vParam[iPar].index = index_new;
            index_new++;
        }
    }

    unsigned int t_gallpar::parNumber() const
    {

        return _vParam.size();
    }

    unsigned int t_gallpar::orbParNumber() const
    {

        unsigned int orbparnum = 0;
        for (auto sat : _vOrbParam)
        {
            orbparnum += sat.second.size();
        }

        return orbparnum;
    }

    int t_gallpar::getCrdParam(const string &station, t_gtriple &crd, const t_gtime &Tbeg, const t_gtime &Tend) const
    {

        int found = 0;

        int idx = this->getParam(station, par_type::CRD_X, "", Tbeg, Tend);
        int idy = this->getParam(station, par_type::CRD_Y, "", Tbeg, Tend);
        int idz = this->getParam(station, par_type::CRD_Z, "", Tbeg, Tend);

        if (idx != -1)
        {
            crd.set(0, _vParam[idx].value());
            found++;
        }
        if (idy != -1)
        {
            crd.set(1, _vParam[idy].value());
            found++;
        }
        if (idz != -1)
        {
            crd.set(2, _vParam[idz].value());
            found++;
        }

        if (found == 3)
            return 1; // all three crd were found
        if (found == 1)
            return -1; // just one crd was found
        if (found == 2)
            return -2; // just two crd was found

        if (found == 0)
            return -3; // crd not found

        return -1;
    }

    int t_gallpar::getVelParam(const string &station, t_gtriple &vel, const t_gtime &Tbeg, const t_gtime &Tend) const
    {

        int found = 0;
        vector<t_gpar>::const_iterator iter;
        for (iter = _vParam.begin(); iter != _vParam.end(); ++iter)
        {
            if (iter->parType == par_type::VEL_X && iter->site.compare(station) == 0 &&
                iter->beg == Tbeg && iter->end == Tend)
            {
                vel.set(0, iter->value());
                found++;
            }
            else if (iter->parType == par_type::VEL_Y && iter->site.compare(station) == 0 &&
                     iter->beg == Tbeg && iter->end == Tend)
            {
                vel.set(1, iter->value());
                found++;
            }
            else if (iter->parType == par_type::VEL_Z && iter->site.compare(station) == 0 &&
                     iter->beg == Tbeg && iter->end == Tend)
            {
                vel.set(2, iter->value());
                found++;
            }
        }
        if (found == 3)
            return 1; // all three crd were found
        if (found == 1)
            return -1; // just one crd was found
        if (found == 2)
            return -2; // just two crd was found

        if (found == 0)
            return -3; // crd not found

        return -1;
    }

    vector<int> t_gallpar::getPartialIndex(const string &site, const string &sat)
    {
        _update_partial_index();

        vector<int> ans;
        pair<string, string> type_list[4] =
            {
                make_pair(site, sat),
                make_pair(site, ""),
                make_pair("", sat),
                make_pair("", "")};

        _allpar_mtx.lock();
        for (int i = 0; i < 4; i++)
        {
            ans.insert(ans.end(), _index_for_parital[type_list[i]].begin(), _index_for_parital[type_list[i]].end());
        }
        _allpar_mtx.unlock();

        return ans;
    }

    const t_gpar &t_gallpar::getPar(const int &idx) const
    {
        return _vParam[idx];
    };

    t_gpar &t_gallpar::operator[](const size_t idx)
    {
        return _vParam[idx];
    }

    t_gallpar t_gallpar::operator-(const t_gallpar &gallpar)
    {
        t_gallpar diff;
        if (this->parNumber() != gallpar.parNumber())
        {
            cerr << "t_gallpar::operator-: Incompatible dimension ("
                 << this->parNumber() << ", " << gallpar.parNumber() << ")"
                 << endl;
            return diff;
        }

        diff = (*this);
        vector<t_gpar>::const_iterator iter;
        for (iter = _vParam.begin(); iter != _vParam.end(); ++iter)
        {
            int i = gallpar.getParam(iter->site, iter->parType, iter->prn, iter->beg, iter->end);
            if (i >= 0)
            {
                diff[i] = (*iter) - gallpar.getPar(i);
            }
        }
        return diff;
    }

    t_gallpar t_gallpar::operator+(const t_gallpar &gallpar)
    {
        t_gallpar diff;

        if (this->parNumber() != gallpar.parNumber())
        {
            cerr << "t_gallpar::operator+: Incopatible dimension ("
                 << this->parNumber() << ", " << gallpar.parNumber() << ")"
                 << endl;
            return diff;
        }

        diff = (*this);
        vector<t_gpar>::const_iterator iter;
        for (iter = _vParam.begin(); iter != _vParam.end(); ++iter)
        {
            int i = gallpar.getParam(iter->site, iter->parType, iter->prn, iter->beg, iter->end);
            if (i >= 0)
            {
                diff[i] = (*iter) + gallpar.getPar(i);
            }
        }
        return diff;
    }

    void t_gallpar::delAllParam()
    {

        _vParam.clear();
        this->_index_par.clear();
        this->_point_par.clear();
        this->_max_point = 0;
        this->_last_point = make_pair(0, 0);
    }

    vector<int> t_gallpar::delAmb()
    {

        vector<int> ind;
        vector<t_gpar>::iterator iter;
        iter = _vParam.begin();
        while (iter != _vParam.end())
        {
            if (iter->parType == par_type::AMB_IF || iter->parType == par_type::AMB_L1 || iter->parType == par_type::AMB_L2 || iter->parType == par_type::AMB_L3 || iter->parType == par_type::AMB_L4 || iter->parType == par_type::AMB_L5 || iter->parType == par_type::AMB_WL)
            {
                ind.push_back(iter->index);
                int i = iter - _vParam.begin();

                auto &all = this->_index_par[_vParam[i].get_head()];
                for (auto iter = all.begin(); iter != all.end(); ++iter)
                {
                    if (iter->second == _point_par[i])
                    {
                        all.erase(iter);
                        break;
                    }
                }
                if (this->_index_par[_vParam[i].get_head()].size() == 0)
                {
                    this->_index_par.erase(_vParam[i].get_head());
                }
                _point_par.erase(_point_par.begin() + i);

                iter = _vParam.erase(iter);
            }
            else
                ++iter;
        }
        return ind;
    }

    void t_gallpar::setSite(const string &site)
    {

        vector<t_gpar>::iterator iter;
        for (iter = _vParam.begin(); iter != _vParam.end(); ++iter)
            iter->site = site;
    }

    set<string> t_gallpar::amb_prns()
    {

        set<string> prns;
        vector<t_gpar>::const_iterator iter;
        for (iter = _vParam.begin(); iter != _vParam.end(); ++iter)
        {
            if (iter->str_type().find("AMB") != string::npos)
            {
                prns.insert(iter->prn);
            }
        }
        return prns;
    }

    ostream &operator<<(ostream &os, t_gallpar &x)
    {
        for (unsigned int i = 0; i < x.parNumber(); i++)
        {
            if (x[i].parType == par_type::AMB_IF ||
                x[i].parType == par_type::AMB_L1 ||
                x[i].parType == par_type::AMB_L2 ||
                x[i].parType == par_type::AMB_L3 ||
                x[i].parType == par_type::AMB_L4 ||
                x[i].parType == par_type::AMB_L5 ||
                x[i].parType == par_type::SION ||
                x[i].parType == par_type::VION ||
                x[i].parType == par_type::IFCB_F3 ||
                x[i].parType == par_type::IFCB_F4 ||
                x[i].parType == par_type::IFCB_F5)
            {
                os << x[i].str_type() << "_" << x[i].prn << " ";
            }
            else
            {
                os << x[i].str_type() << " ";
            }

            if (x[i].parType == par_type::GRD_N || x[i].parType == par_type::GRD_E)
            {
                os << "value: " << x[i].value() * 1000 << " "
                   << "index:" << x[i].index;
            }
            else
            {
                os << "value: " << x[i].value() << " "
                   << "index:" << x[i].index;
            }
            os << endl;
        }
        return os;
    }

    int t_gallpar::sum(t_gallpar &X1, t_gallpar &X2)
    {
        if (X1.parNumber() != X2.parNumber())
            return -1;

        for (unsigned int i = 0; i < _vParam.size(); i++)
        {
            int id1 = X1.getParam(_vParam[i].site, _vParam[i].parType, _vParam[i].prn, FIRST_TIME, LAST_TIME);
            int id2 = X2.getParam(_vParam[i].site, _vParam[i].parType, _vParam[i].prn, FIRST_TIME, LAST_TIME);
            if (id1 >= 0 && id2 >= 0)
                _vParam[i].value(X1[id1].value() + X2[id2].value());
            else
                return -1;
        }
        return 1;
    }

    vector<t_gpar> t_gallpar::getAllPar()
    {
        return _vParam;
    }

    void t_gallpar::_update_partial_index()
    {
        _allpar_mtx.lock();
        auto point_now = make_pair(_point_par[_point_par.size() - 1], int(_point_par.size() - 1));

        if (point_now != _last_point)
        {
            _last_point = point_now;
            _index_for_parital.clear();
            for (unsigned int i = 0; i < _vParam.size(); i++)
            {
                _index_for_parital[make_pair(_vParam[i].site, _vParam[i].prn)].push_back(i);
            }
        }
        _allpar_mtx.unlock();
        return;
    }

    map<string, int> t_gallpar::freq_sats_num(const int &freq)
    {

        set<string> prns1, prns2, prnsif, prns13if, prns14if, prns15if;
        map<string, int> Nsats;
        vector<t_gpar>::const_iterator iter;
        for (iter = _vParam.begin(); iter != _vParam.end(); ++iter)
        {
            if (iter->parType == par_type::AMB_L1)
            {
                prns1.insert(iter->prn);
            }
            else if (iter->parType == par_type::AMB_L2)
            {
                prns2.insert(iter->prn);
            }
            else if (iter->parType == par_type::AMB_IF)
            {
                prnsif.insert(iter->prn);
            }
            else if (iter->parType == par_type::AMB13_IF)
            {
                prns13if.insert(iter->prn);
            }
            else if (iter->parType == par_type::AMB14_IF)
            {
                prns14if.insert(iter->prn);
            }
            else if (iter->parType == par_type::AMB15_IF)
            {
                prns15if.insert(iter->prn);
            }
        }

        if (prnsif.size() != 0 || prns13if.size() != 0)
        {
            set<string> tmp;
            set_intersection(prnsif.begin(), prnsif.end(), prns13if.begin(), prns13if.end(), inserter(tmp, tmp.begin()));
            Nsats["Triple"] = tmp.size();
            Nsats["Double"] = prnsif.size() + prns13if.size() - 2 * tmp.size();
        }
        else if (prns1.size() != 0 || prns2.size() != 0)
        {
            set<string> tmp;
            set_intersection(prns1.begin(), prns1.end(), prns2.begin(), prns2.end(), inserter(tmp, tmp.begin()));
            Nsats["Double"] = tmp.size();
            Nsats["Single"] = prns1.size() + prns2.size() - 2 * tmp.size();
        }

        return Nsats;
    }

} // namespace
