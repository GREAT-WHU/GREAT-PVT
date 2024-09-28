/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <stdio.h>
#include <math.h>

#include "gdata/grec.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{

    t_grec::t_grec()
        : t_gobj()
    {
        id_type(REC);
        _overwrite = true;
    }

    t_grec::t_grec(t_spdlog spdlog)
        : t_gobj(spdlog)
    {
        id_type(REC);
        _overwrite = true;
    }

    t_grec::~t_grec()
    {
        _gmutex.lock();
        _maprec.clear();
        _gmutex.unlock();
    }

    void t_grec::rec(string rec, const t_gtime &beg, const t_gtime &end)
    {
        _gmutex.lock();

        _rec(rec, beg, end);

        _gmutex.unlock();
        return;
    }

    void t_grec::_rec(string rec, const t_gtime &beg, const t_gtime &end)
    {
        t_maprec::iterator it = _maprec.find(beg);

        if (!(beg < end))
        {
            string lg = "Warning: " + _id + " not valid end time (end<beg) for receiver (beg:" + beg.str_ymdhms() + " -> end:" + end.str_ymdhms() + ")";
            if (_spdlog)
                SPDLOG_LOGGER_WARN(_spdlog, lg);
            return;
        }

        // begin record
        if (it == _maprec.end())
        { // not exists
            _maprec[beg] = rec;
        }
        else
        { // record exists
            if (it->first == LAST_TIME ||
                it->second.empty())
            {

                _maprec[beg] = rec;
            }
            else
            {
                return;
            }
        }

        // control end of record (with new beg search)
        it = _maprec.find(beg);
        it++;

        // beg was last in map (add final empty record)
        if (it == _maprec.end())
        {
            _maprec[end] = "";
        }
        else
        { // process end according to next value
            if (end < it->first)
            { // only if end is smaller then existing
                if (fabs(it->first - end) > 3600)
                { // significantly smaller!
                    if (it->second.empty())
                        _maprec.erase(it); // remove obsolete empty record
                    _maprec[end] = "";
                }
                else
                { // too close to next record
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "Warning: " + _id + " 'rec' end tied to the existing value " + end.str("%Y-%m-%d %H:%M:%S -> ") + it->first.str("%Y-%m-%d %H:%M:%S"));
                }
            }
            else if (end != it->first)
            {

                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Warning: object " + _id + " 'rec' " + rec + " end cut and tied to the existing value " + end.str("%Y-%m-%d %H:%M:%S -> ") + it->first.str("%Y-%m-%d %H:%M:%S"));
            }
        }

        // remove duplicated empty records
        t_maprec::iterator itNEW = _maprec.begin();
        t_maprec::iterator itOLD = itNEW;
        while (itOLD != _maprec.end())
        {
            if (++itNEW != _maprec.end())
            {
                if ((itNEW->second.empty() && itOLD->second.empty()))
                {
                    _maprec.erase(itNEW++);
                }
            }
            itOLD = itNEW;
        }

        return;
    }

    string t_grec::rec(const t_gtime &t) const
    {
        _gmutex.lock();

        string tmp = "";
        tmp = _rec(t);

        _gmutex.unlock();
        return tmp;
    }

    string t_grec::_rec(const t_gtime &t) const
    {

        t_maprec::const_iterator it = _maprec.upper_bound(t);
        if (it == _maprec.begin())
        {
            return "";
        } // not found (or not exists!)
        it--;

        return it->second;

    }

    // return validity for receiver at epoch t
    void t_grec::rec_validity(const t_gtime &t, t_gtime &beg, t_gtime &end) const
    {
        t_maprec::const_iterator it = _maprec.upper_bound(t);

        if (it != _maprec.end())
        {
            if (it != _maprec.begin())
            {
                end = it->first;
                it--;
                beg = it->first;
            }
        }
        else
        {
            beg = FIRST_TIME;
            end = LAST_TIME;
        }

        return;
    }

    vector<t_gtime> t_grec::rec_id() const
    {
        _gmutex.lock();

        vector<t_gtime> tmp;
        t_maprec::const_iterator itMAP = _maprec.begin();
        while (itMAP != _maprec.end())
        {

            tmp.push_back(itMAP->first);
            itMAP++;
        }
        _gmutex.unlock();
        return tmp;
    }

    void t_grec::addhdr(const t_rnxhdr &hdr, const t_gtime &epo, string path)
    {
        _gmutex.lock();

        if (_maphdr.find(epo) == _maphdr.end())
        {
            _maphdr[epo] = hdr;
            _maphdr[epo].path(path);
        }

        _gmutex.unlock();
    }

    void t_grec::changehdr(const t_rnxhdr &hdr, const t_gtime &epo, string path)
    {
        _maphdr.erase(_maphdr.begin(), _maphdr.end());
        _maphdr[epo] = hdr;
        _maphdr[epo].path(path);
    }

    t_grec::t_maphdr t_grec::gethdr()
    {
        return _maphdr;
    }

    t_rnxhdr t_grec::gethdr(const t_gtime &epo)
    {
        _gmutex.lock();

        t_rnxhdr rnxhdr = _gethdr(epo);

        _gmutex.unlock();
        return rnxhdr;
    }

    void t_grec::compare(shared_ptr<t_grec> grec, const t_gtime &tt, string source)
    {

        _gmutex.lock();

        _gmutex.unlock();
        t_gobj::compare(grec, tt, source);
        _gmutex.lock();

        string old, alt;
        old = trim(_rec(tt));
        alt = trim(grec->rec(tt));
        t_gtime beg, end;
        grec->rec_validity(tt, beg, end);

        if (old != alt && !alt.empty())
        {
            if (old.empty())
            {
                _rec(alt, beg, end);
                this->rec_validity(tt, beg, end);
            }
            else if (_overwrite)
            {
                _rec(alt, beg, end);
            }
            else
            {
            }
        }

        // add hdr from grec if not exists at TT
        if (_maphdr.find(tt) == _maphdr.end())
        {
            t_maphdr oth_head = grec->gethdr();
            if (oth_head.find(tt) != oth_head.end())
            {
                t_rnxhdr head = oth_head[tt];
                _maphdr[tt] = head;
            }
        }

        _gmutex.unlock();
        return;
    }

    void t_grec::fill_rnxhdr(const t_rnxhdr &rnxhdr)
    {
        _gmutex.lock();

        _fill_rnxhdr(rnxhdr);

        _gmutex.unlock();
        return;
    }

    t_rnxhdr t_grec::_gethdr(const t_gtime &epo)
    {
        t_rnxhdr rnxhdr;
        auto it = _maphdr.upper_bound(epo);

        if (it == _maphdr.begin())
        {
            return rnxhdr;
        }

        return (--it)->second;
    }

    void t_grec::_fill_rnxhdr(const t_rnxhdr &rnxhdr)
    {
        t_gtime epo = rnxhdr.first();

        if (_name.empty())
            _name = rnxhdr.markname(); // NOT IF EXISTS! does not need to be the same

        _domes = rnxhdr.marknumb();
        _eccxyz(rnxhdr.antxyz(), epo);
        _eccneu(rnxhdr.antneu(), epo);
        _ant(rnxhdr.anttype(), epo);
        _rec(rnxhdr.rectype(), epo);

        t_gtriple std(10, 10, 10); // 10 m for Rinex header aprox coordinates
        _crd(rnxhdr.aprxyz(), std, epo);
    }

} // namespace
