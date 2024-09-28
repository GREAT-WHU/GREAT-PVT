
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  
  (c) 2011-2017 Geodetic Observatory Pecny, http://www.pecny.cz (gnss@pecny.cz)
      Research Institute of Geodesy, Topography and Cartography
      Ondrejov 244, 251 65, Czech Republic

  This file is part of the G-Nut C++ library.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3 of
  the License, or (at your option) any later version.
 
  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses>.

-*/

#include <iomanip>
#include <memory>

#include "gproc/gpreproc.h"
#include "gutils/gcommon.h"
#include "gutils/gtimesync.h"
#include "gutils/gtypeconv.h"
#include "gmodels/gbancroft.h"

using namespace gnut;

namespace gnut
{
    t_gpreproc::t_gpreproc(t_gallobs *obs, t_gsetbase *settings)
        : _spdlog(nullptr)
    {
        if (nullptr == settings)
        {
            spdlog::critical("your set pointer is nullptr !");
            throw logic_error("");
        }
        else
        {
            _set = settings;
        }
        _obs = obs;
        _beg_end = true;

        _sys = dynamic_cast<t_gsetgen *>(_set)->sys();
        _scl = dynamic_cast<t_gsetgen *>(_set)->sampling_scalefc();
        _sat = dynamic_cast<t_gsetgnss *>(_set)->sat();
        if (_sat.size() == 0)
        {
            t_map_sats gnss_sats = GNSS_SATS();
            t_map_sats::const_iterator itGNS;
            for (itGNS = gnss_sats.begin(); itGNS != gnss_sats.end(); ++itGNS)
            {
                GSYS gsys = itGNS->first;
                set<string> sats = gnss_sats[gsys];
                for (set<string>::iterator it = sats.begin(); it != sats.end(); ++it)
                    _sat.insert(*it);
            }
        }
        _sigCode = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(GPS);
        _sigPhase = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(GPS);
        _sigCode_GLO = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(GLO);
        _sigPhase_GLO = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(GLO);
        _sigCode_GAL = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(GAL);
        _sigPhase_GAL = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(GAL);
        _sigCode_BDS = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(BDS);
        _sigPhase_BDS = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(BDS);
        _sigCode_QZS = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(QZS);
        _sigPhase_QZS = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(QZS);
        _sigCode_IRN = dynamic_cast<t_gsetgnss *>(_set)->sigma_C(IRN);
        _sigPhase_IRN = dynamic_cast<t_gsetgnss *>(_set)->sigma_L(IRN);

        set<string> rec_list = dynamic_cast<t_gsetgen *>(_set)->rec_all();
        for (auto rec : rec_list)
        {
            for (set<string>::iterator it = _sys.begin(); it != _sys.end(); it++)
            {
                _dI[rec][*it] = 0.0;
            }
            for (set<string>::iterator it = _sat.begin(); it != _sat.end(); it++)
            {
                _dI[rec][*it] = 0.0;
            }
            _firstEpo[rec] = true;
        }
    }

    t_gpreproc::~t_gpreproc()
    {
    }

    void t_gpreproc::spdlog(t_spdlog spdlog)
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
    }

    int t_gpreproc::ProcessBatch(string site, const t_gtime &beg_r, const t_gtime &end_r, double sampl, bool sync, bool save)
    {

        int sign = 1;
        t_gtime beg;
        t_gtime end;

        if (!_beg_end)
        {
            beg = end_r;
            end = beg_r;
            sign = -1;
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Preprocessing in end -> begin direction!");
        }
        else
        {
            beg = beg_r;
            end = end_r;
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Preprocessing in begin -> end direction!");
        }

        this->_site = site;
        if (_obs->nepochs(_site) <= 1)
            return -1;

        vector<shared_ptr<t_gobsgnss>> epoData;

        double CJ = 0.0; // integer ms clk jump

        double subint = 0.1;
        if (_scl > 0)
            subint = 1.0 / _scl;
        if (sampl > 1)
            subint = pow(10, floor(log10(sampl)));

        bool time_loop = true;
        t_gtime epoch(beg);
        while (time_loop)
        {
            if (_beg_end && (epoch < end || epoch == end))
                time_loop = true;
            else if (_beg_end && epoch > end)
                time_loop = false;

            if (!_beg_end && (epoch > end || epoch == end))
                time_loop = true;
            else if (!_beg_end && epoch < end)
                time_loop = false;

            // synchronization
            if (sync)
            {
                if (!time_sync(epoch, sampl, _scl, _spdlog))
                {
                    epoch.add_dsec(subint / 100); // add_dsec used for synchronization!
                    continue;
                }
            }

            if (sampl >= 1)
                epoch.reset_dsec(); //  LOW-RATE (>=1Hz), i.e. if not HIGH-RATE !!

            if (_inputEpoData.size() == 0)
                epoData = _obs->obs_pt(_site, epoch);
            else
                epoData = _inputEpoData;

            if (epoData.size() == 0 || !time_loop)
            {
                if (sampl >= 1)
                    epoch.add_secs(sign * (int)sampl); // =<1Hz data
                else
                    epoch.add_dsec(sign * sampl); //  >1Hz data
                continue;
            }

            if (_firstEpo[_site])
            {
                for (unsigned int i = 0; i < epoData.size(); i++)
                    _epoDataPre[_site].push_back(*epoData[i]);
                _firstEpo[_site] = false;
            }

            // test data gap
            if (save)
            {
                shared_ptr<t_gobsgnss> g = *(epoData.begin());
                double diffEpo = g->epoch() - _epoDataPre[_site].begin()->epoch();
                if (!_firstEpo[_site] && diffEpo > sampl + DIFF_SEC(sampl))
                {
                    _gapReport(epoData);
                }
                _compare(_epoDataPre[_site], epoData);
            }

            double Ns = epoData.size(); // total number of viewed satellites
            double Na = Ns;             // number of satellite involved into clk jump detection
            double n = 0;               // number of detected jumps in particular epoch
            _sumS = 0;

            shared_ptr<t_gobsgnss> obs2;

            for (vector<shared_ptr<t_gobsgnss>>::iterator it = epoData.begin(); it != epoData.end(); ++it)
            {
                if (*it == 0)
                {
                    Na--;
                    continue;
                }

                obs2 = *it;
                t_gobsgnss *obs1 = 0;

                vector<t_gobsgnss>::iterator itPre;
                for (itPre = _epoDataPre[_site].begin(); itPre != _epoDataPre[_site].end(); ++itPre)
                {
                    if ((*it)->sat() == (itPre)->sat())
                    {
                        obs1 = &(*itPre);
                    }
                }

                if (obs1 == 0)
                {
                    Na--;
                    continue;
                } // sat not found in previous epoch

                t_gsys gsys(obs1->gsys());
                if (gsys.gsys() != GPS)
                    Na--; // clk jump is determined only from GPS

                string satname = obs1->sat();
                if (_sat.find(satname) == _sat.end())
                    continue; // sat excluded in config

                // Cycle slip detection and repair
                if (sync)
                {
                    if (abs(obs1->epoch() - obs2->epoch()) <= sampl + 1 && _slip(obs1, obs2) > -1)
                    {
                        _transform(obs2, save); // 1 second reserve for obs sync
                    }
                }
                else if (_slip(obs1, obs2) > -1)
                    _transform(obs2, save);

                // Clock jump detection and repair
                int irc_jmp = _jumps(obs1, obs2);
                if (irc_jmp == 1)
                    n++;
                if (irc_jmp == -1)
                    Na--;
            } // end sats loop

            _epoDataPre[_site].clear();

            for (unsigned int i = 0; i < epoData.size(); i++)
                _epoDataPre[_site].push_back(*epoData[i]);

            if (n == Na)
            { // clk jump effect at all sats in the same way
                double M = (1e3 * _sumS) / (CLIGHT * n);
                double Js = abs(M - floor(M + 0.5));

                if (Js <= 1e-5)
                {                         // it is clock jump instead of common slip
                    CJ += floor(M + 0.5); // ms jump
                    if (save)
                        _mbreaks[_site][epoch] = (int)CJ; // store for logging
                    _remove_slip(epoData);
                }
            }

            if (CJ != 0.0)
            {
                _repair(epoData, 1e-3 * CJ * CLIGHT); // corrected carrier phase
            }

            if (sampl >= 1)
                epoch.add_secs(sign * (int)sampl); // =<1Hz data
            else
                epoch.add_dsec(sign * sampl); //  >1Hz data
        }

        return 1;
    }

    int t_gpreproc::_jumps(t_gobsgnss *gobs1, t_spt_gobs gobs2)
    {

        string prn = gobs2->sat();
        GSYS GS = gobs1->gsys();

        if (GS != GPS)
            return -1;

        set<GOBSBAND> freq_1 = gobs1->band_avail();
        set<GOBSBAND> freq_2 = gobs2->band_avail();

        if (freq_1.size() < 2 || freq_2.size() < 2)
        {
            return -1;
        }

        GOBSBAND band;
        set<GOBSBAND>::reverse_iterator it = freq_1.rbegin();
        band = *it; 
        t_gband b1_1(band, ATTR);
        it++;
        band = *it; 
        t_gband b2_1(band, ATTR);

        it = freq_2.rbegin();
        band = *it; 
        t_gband b1_2(band, ATTR);
        it++;
        band = *it; 
        t_gband b2_2(band, ATTR);

        // j-epoch
        double L1j = gobs2->obs_L(b1_2); // [m]
        double L2j = gobs2->obs_L(b2_2); // [m]
        double P1j = gobs2->obs_C(b1_2); // [m]
        double P2j = gobs2->obs_C(b2_2); // [m]

        // i-epoch
        double L1i = gobs1->obs_L(b1_1); // [m]
        double L2i = gobs1->obs_L(b2_1); // [m]
        double P1i = gobs1->obs_C(b1_1); // [m]
        double P2i = gobs1->obs_C(b2_1); // [m]

        int nl = 0;
        double dl = 0.0;
        if (!double_eq(L1j, 0.0) && !double_eq(L1i, 0.0))
        {
            dl += L1j - L1i;
            nl++;
        }
        if (!double_eq(L2j, 0.0) && !double_eq(L2i, 0.0))
        {
            dl += L2j - L2i;
            nl++;
        }
        if (nl == 0)
            return -1;
        dl /= nl;

        int np = 0;
        double dp = 0.0;
        if (!double_eq(P1j, 0.0) && !double_eq(P1i, 0.0))
        {
            dp += P1j - P1i;
            np++;
        }
        if (!double_eq(P2j, 0.0) && !double_eq(P2i, 0.0))
        {
            dp += P2j - P2i;
            np++;
        }
        if (np == 0)
            return -1;
        dp /= np;

        double S = dp - dl;                 // jump detection observable
        double sig = 5;                     // sigma of S
        double k = 1e-3 * CLIGHT - 3 * sig; // jump threshold


        if (abs(S) >= k)
        { // candidate of clock jump
            _sumS += S;
            return 1;
        }
        return 0;
    }

    void t_gpreproc::setNav(t_gallnav *nav)
    {
        this->_nav = nav;
    }

    void t_gpreproc::_repair(vector<t_spt_gobs> epoData, double dL)
    {

        for (vector<t_spt_gobs>::iterator it = epoData.begin(); it != epoData.end(); it++)
        {
            (*it)->mod_L(dL, X); // modify all phases by dL [m]
        }
    }

    int t_gpreproc::_transform(t_spt_gobs gobs, bool save)
    {

        int d = _v_lcslp.size();
        if (d <= 1)
            return -1;

        unsigned int maxit = 1;
        for (t_vec_slp::iterator it = _v_lcslp.begin(); it != _v_lcslp.end(); it++)
        {
            if (it->second.size() > maxit)
                maxit = it->second.size();
        }

        for (unsigned int tmp = 1; tmp <= maxit; tmp++)
        {
            Matrix M(d, d);
            M = 0;
            ColumnVector S(d);
            S = 0;
            map<GOBS, double> m_orig;

            bool trans = false;

            for (int i = 1; i < d; i++)
            {
                if (_v_lcslp.find(i + 10) != _v_lcslp.end())
                {
                    S(d) = -_v_lcslp[i + 10].begin()->val;
                    if (!double_eq(S(d), 0.0))
                        trans = true;
                    m_orig[_v_lcslp[i + 10].begin()->obs1.gobs()] = 0;
                    m_orig[_v_lcslp[i + 10].begin()->obs2.gobs()] = 0;

                    if (_v_lcslp[i + 10].size() >= 2)
                    {
                        _v_lcslp[i + 10].erase(_v_lcslp[i + 10].begin());
                    }

                    M(d, 1) = 2;
                    M(d, 2) = -1;
                }

                if (_v_lcslp.find(i) != _v_lcslp.end())
                {

                    M(i, d) = -1;
                    M(i, d - i) = 1;

                    S(i) = -_v_lcslp[i].begin()->val;

                    m_orig[_v_lcslp[i].begin()->obs1.gobs()] = 0;
                    m_orig[_v_lcslp[i].begin()->obs2.gobs()] = 0;
                    if (_v_lcslp[i].size() >= 2)
                    {
                        _v_lcslp[i].erase(_v_lcslp[i].begin());
                    }

                    if (!double_eq(S(d - i), 0.0))
                        trans = true;
                }
            }

            ColumnVector O(S);
            O = 0;

            if (trans)
            {
                O = M.i() * S;
                set<GOBSBAND> bands;
                for (map<GOBS, double>::iterator it = m_orig.begin(); it != m_orig.end(); it++)
                {
                    t_gobs s;
                    s.gobs(it->first);
                    bands.insert(s.band());
                }

                vector<GOBSBAND> vec_bnd = sort_band(gobs->gsys(), bands);

                int i = 1;
                for (auto it = vec_bnd.begin(); it != vec_bnd.end(); it++, i++)
                {
                    for (map<GOBS, double>::iterator it2 = m_orig.begin(); it2 != m_orig.end(); it2++)
                    {
                        t_gobs s;
                        s.gobs(it2->first);

                        if (s.band() == *it)
                        {
                            it2->second = O(i);
                            gobs->addlli(it2->first, 1);
                            gobs->addslip(it2->first, int(it2->second));
                        }
                    }
                }

                if (save)
                    _save(gobs, m_orig);
            }
        }

        return 1;
    }

    void t_gpreproc::_save(t_spt_gobs gobs, const map<GOBS, double> &slips)
    {
        t_gtime epo = gobs->epoch();
        string prn = gobs->sat();

        for (map<GOBS, double>::const_iterator it = slips.begin(); it != slips.end(); it++)
        {
            GOBS g = it->first;
            double slp = it->second;
            if (!double_eq(slp, 0.0))
                _mslips[_site][epo][prn][g] = slp;
        }
    }

    void t_gpreproc::_remove_slip(vector<t_spt_gobs> gobs)
    {

        t_gtime epo = gobs[0]->epoch();

        map<t_gtime, map<string, map<GOBS, double>>>::iterator itEPO;
        itEPO = _mslips[_site].find(epo);
        if (itEPO != _mslips[_site].end())
            _mslips[_site].erase(itEPO);

        for (vector<t_spt_gobs>::iterator itG = gobs.begin(); itG != gobs.end(); itG++)
        {
            set<GOBSBAND> freq = (*itG)->band_avail();
            for (auto itF = freq.begin(); itF != freq.end(); itF++)
            {
                t_gband(*itF, ATTR);
                GOBS g = (*itG)->id_phase(*itF);
                (*itG)->addlli(g, 0);
            }
        }
    }

    void t_gpreproc::_common(set<GOBSBAND> &set1, set<GOBSBAND> &set2)
    {

        for (auto it1 = set1.begin(); it1 != set1.end();)
        {
            auto it2 = set2.find(*it1);
            if (it2 == set2.end())
            {
                auto tmp = it1;
                ++it1;
                set1.erase(tmp);
            }
            else
                ++it1;
        }

        for (auto it1 = set2.begin(); it1 != set2.end();)
        {
            auto it2 = set1.find(*it1);
            if (it2 == set1.end())
            {
                auto tmp = it1;
                ++it1;
                set2.erase(tmp);
            }
            else
                ++it1;
        }
    }

    void t_gpreproc::_common(set<GOBS> &set1, set<GOBS> &set2)
    {

        for (set<GOBS>::iterator it1 = set1.begin(); it1 != set1.end();)
        {
            set<GOBS>::iterator it2 = set2.find(*it1);
            if (it2 == set2.end())
            {
                set<GOBS>::iterator tmp = it1;
                ++it1;
                set1.erase(tmp);
            }
            else
                ++it1;
        }

        for (set<GOBS>::iterator it1 = set2.begin(); it1 != set2.end();)
        {
            set<GOBS>::iterator it2 = set1.find(*it1);
            if (it2 == set1.end())
            {
                set<GOBS>::iterator tmp = it1;
                ++it1;
                set2.erase(tmp);
            }
            else
                ++it1;
        }
    }

    int t_gpreproc::_slip(t_gobsgnss *gobs1, t_spt_gobs gobs2)
    {

        bool slip = false;

        set<GOBSBAND> bands_t1 = gobs1->band_avail();
        set<GOBSBAND> bands_t2 = gobs2->band_avail();

        this->_common(bands_t1, bands_t2);

        if (bands_t1.size() != bands_t2.size())
        {
            cerr << "ERROR: problem in t_gpreprocc::_common" << endl;
            return -1;
        }

        if (bands_t1.size() <= 1)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, gobs1->epoch().str_ymdhms("Not enough bands available: ") + " " + gobs1->sat());
            return -1;
        }

        int nfreq = bands_t1.size();

        t_gobs s1;
        t_gobs s2;
        t_gobs sF;
        t_gobs s_narr;

        // sort according to wavelenght
        vector<GOBSBAND> sorted_t1 = sort_band(gobs1->gsys(), bands_t1);
        vector<GOBSBAND> sorted_t2 = sort_band(gobs2->gsys(), bands_t2);

        vector<GOBSBAND>::reverse_iterator itFRQ = sorted_t1.rbegin();

        set<GOBS> gf2_t1 = gobs1->obs_phase(*itFRQ);
        set<GOBS> gf2_t2 = gobs2->obs_phase(*itFRQ);
        this->_common(gf2_t1, gf2_t2); // signals for wide-lane

        //second freq: for narrow-lane
        vector<GOBSBAND>::iterator it_narr = sorted_t1.begin();
        if (nfreq >= 2)
            it_narr++;

        set<GOBS> gNL_t1 = gobs1->obs_phase(*it_narr);
        set<GOBS> gNL_t2 = gobs2->obs_phase(*it_narr);
        this->_common(gNL_t1, gNL_t2); // signals for wide-lane

        _m_lcslp.clear();
        _v_lcslp.clear();

        for (int i = 1; i < nfreq; i++)
        {
            ++itFRQ;

            set<GOBS> gf1_t1 = gobs1->obs_phase(*itFRQ);
            set<GOBS> gf1_t2 = gobs2->obs_phase(*itFRQ);

            this->_common(gf1_t1, gf1_t2); // reference signal - last one

            if (gf2_t1.size() == 0)
            {
                gf2_t1 = gobs1->obs_phase(*itFRQ);
                gf2_t2 = gobs2->obs_phase(*itFRQ);
                this->_common(gf2_t1, gf2_t2); // signals for wide-lane
                i--;
                nfreq--;
                continue;
            }

            // find freq for extra-wide-lane
            vector<GOBSBAND>::reverse_iterator itFF;
            set<GOBS> gFF_t1;
            set<GOBS> gFF_t2;
            if (i > 1)
            {
                vector<GOBSBAND>::reverse_iterator itFF = itFRQ;
                --itFF;
                gFF_t1 = gobs1->obs_phase(*itFF);
                gFF_t2 = gobs2->obs_phase(*itFF);
                this->_common(gFF_t1, gFF_t2); // freq for extra-widelane
            }

            set<GOBS>::iterator itGOBSFF = gFF_t1.begin();
            set<GOBS>::iterator itGOBSf2 = gf2_t1.begin();
            set<GOBS>::iterator itGOBSNL = gNL_t1.begin();

            bool endFF = true;
            bool endNL = true;

            for (set<GOBS>::iterator itGOBSf1 = gf1_t1.begin();;)
            {
                if (gf1_t1.size() == 0 || gf2_t1.size() == 0)
                {
                    _v_lcslp.clear();
                    break;
                }

                s1.gobs(*itGOBSf1);
                s2.gobs(*itGOBSf2);

                t_gobs_pair gobs_pair(s1, s2);

                double diff = 0;
                double lam = CLIGHT / gobs1->frequency_lc(s1.band(), 1, s2.band(), -1);

                if (i == 1)
                {
                    double lct1 = gobs1->MW(s1, s2);
                    double lct2 = gobs2->MW(s1, s2);
                    diff = (lct2 - lct1) / lam;
                    t_gband band1 = s1.gband();
                    t_gband band2 = s2.gband();
                    double wlSlp = 0;
                    if (fabs(diff) > 2)
                    { //thr) {
                        slip = true;
                        wlSlp = round(diff);
                    }
                    gobs_pair.val = wlSlp;
                    _v_lcslp[i].push_back(gobs_pair);
                    _m_lcslp[i][gobs_pair] = wlSlp;
                }
                else
                {
                    double lct1 = gobs1->LWL(s1, s2);
                    double lct2 = gobs2->LWL(s1, s2);
                    double dWL = lct2 - lct1;

                    t_gband band1 = s1.gband();
                    t_gband band2 = s2.gband();

                    // Compute extra-wide-lane time differenced observatins
                    sF.gobs(*itGOBSFF);
                    lct1 = gobs1->LWL(sF, s2);
                    lct2 = gobs2->LWL(sF, s2);
                    double dEWL = lct2 - lct1;
                    gobs_pair.obs1 = sF;

                    // find extra-wide-lane cycle slip from previous cascade
                    double ewlSlp = 0;
                    int x = i - 1;
                    ewlSlp = _findSlp(x, gobs_pair);
                    double elam = 0;
                    elam = CLIGHT / gobs1->frequency_lc(gobs_pair.obs1.band(), 1, gobs_pair.obs2.band(), -1);
                    t_gband bandF = sF.gband();
                    diff = (dEWL - dWL + ewlSlp * elam) / lam;
                    gobs_pair.obs1 = s1;
                    double wlSlp = 0;
                    if (fabs(diff) > 2)
                    { 
                        slip = true;
                        wlSlp = round(diff);
                    }
                    gobs_pair.val = wlSlp;
                    _v_lcslp[i].push_back(gobs_pair);
                    _m_lcslp[i][gobs_pair] = wlSlp;
                }

                // narrow-lane slip
                if (i == nfreq - 1)
                {
                    if (gf1_t1.size() == 0 || gNL_t1.size() == 0)
                        break;
                    double lct1 = gobs1->LWL(s1, s2);
                    double lct2 = gobs2->LWL(s1, s2);
                    double dWL = lct2 - lct1;

                    // find wide-lane slip from last cascade
                    double wlSlp = 0;
                    wlSlp = _findSlp(i, gobs_pair);

                    s_narr.gobs(*itGOBSNL);
                    lct1 = gobs1->LNL(s1, s_narr);
                    lct2 = gobs2->LNL(s1, s_narr);
                    double dNL = lct2 - lct1;
                    gobs_pair.obs1 = s1;
                    gobs_pair.obs2 = s_narr;
                    double nlam = CLIGHT / gobs1->frequency_lc(gobs_pair.obs1.band(), 2, gobs_pair.obs2.band(), -1);
                    double disf = _disf(gobs1, gobs2, s1, s_narr);
                    if (!slip)
                        _iono(gobs1, gobs2, s1, s_narr);
                    string prn = gobs1->sat();

                    diff = (dWL - dNL - disf * _dI[_site][prn] + wlSlp * lam) / nlam;
                    double nlSlp = 0;
                    if (fabs(diff) > 2)
                    {
                        nlSlp = round(diff);
                    }
                    gobs_pair.val = nlSlp;
                    _v_lcslp[i + 10].push_back(gobs_pair);
                    _m_lcslp[i + 10][gobs_pair] = nlSlp;
                }

                if (i > 1)
                {
                    ++itGOBSFF;
                    if (itGOBSFF == gFF_t1.end())
                        endFF = true;
                    else
                        endFF = false;
                }
                if (i == nfreq - 1)
                {
                    ++itGOBSNL;
                    if (itGOBSNL == gNL_t1.end())
                        endNL = true;
                    else
                        endNL = false;
                }

                ++itGOBSf1;
                ++itGOBSf2;
                if (itGOBSf1 == gf1_t1.end() && itGOBSf2 == gf2_t1.end() && endFF && endNL)
                    break;
                if (itGOBSf1 == gf1_t1.end())
                    --itGOBSf1;
                if (itGOBSf2 == gf2_t1.end())
                    --itGOBSf2;
                if (i > 1 && endFF)
                    --itGOBSFF;
                if (i == nfreq - 1 && endNL)
                    --itGOBSNL;
            }
        }

        return 1;
    }

    t_gobs_pair::t_gobs_pair(t_gobs &gobs1, t_gobs &gobs2)
        : obs1(gobs1),
          obs2(gobs2)
    {
    }

    bool t_gobs_pair::operator<(const t_gobs_pair &t) const
    {
        return (this->obs1.attr() < t.obs1.attr() ||

                this->obs2.attr() < t.obs2.attr());
    }

    double t_gpreproc::_disf(t_gobsgnss *gobs1, t_spt_gobs gobs2, t_gobs &s1, t_gobs &s2)
    {
        double isf_wl = gobs1->isf_lc(s1.band(), 1, s2.band(), -1);
        double isf_nl = gobs1->isf_lc(s1.band(), 2, s2.band(), -1);
        double disf = isf_wl - isf_nl;

        return disf;
    }

    void t_gpreproc::_iono(t_gobsgnss *gobs1, t_spt_gobs gobs2, t_gobs &s1, t_gobs &s2)
    {
        double k = pow(gobs1->frequency(s1.band()), 2) / pow(gobs1->frequency(s2.band()), 2);

        double Lb1_1 = gobs1->obs_L(s1);
        double Lb2_1 = gobs1->obs_L(s2);
        double Lb1_2 = gobs2->obs_L(s1);
        double Lb2_2 = gobs2->obs_L(s2);
        double dL = Lb1_2 - Lb2_2 - Lb1_1 + Lb2_1;

        double dI = -dL / (k - 1);

        string prn = gobs1->sat();

        _dI[_site][prn] = dI;
    }

    double t_gpreproc::_findSlp(int &i, t_gobs_pair &gpair)
    {
        double wlSlp = 0;
        if (_m_lcslp.find(i) != _m_lcslp.end())
        {
            if (_m_lcslp[i].find(gpair) != _m_lcslp[i].end())
            {
                wlSlp = _m_lcslp[i][gpair];
            }
        }

        return wlSlp;
    }

    void t_gpreproc::_gapReport(vector<shared_ptr<t_gobsgnss>> epoData)
    {

        for (vector<shared_ptr<t_gobsgnss>>::iterator it = epoData.begin(); it != epoData.end(); it++)
        {
            t_gtime epo = (*it)->epoch();
            string prn = (*it)->sat();
            vector<GOBS> allobs = (*it)->obs();
            for (vector<GOBS>::iterator itGOBS = allobs.begin(); itGOBS != allobs.end(); itGOBS++)
            {
                if (gobs_phase(*itGOBS))
                    _mslipsGap[_site][epo][prn][*itGOBS] = 1;
            }
        }
    }

    void t_gpreproc::_gapReport(shared_ptr<t_gobsgnss> data)
    {

        t_gtime epo = data->epoch();
        string prn = data->sat();
        vector<GOBS> allobs = data->obs();
        for (vector<GOBS>::iterator itGOBS = allobs.begin(); itGOBS != allobs.end(); itGOBS++)
        {
            if (gobs_phase(*itGOBS))
                _mslipsGap[_site][epo][prn][*itGOBS] = 2;
        }
    }

    void t_gpreproc::_compare(vector<t_gobsgnss> data1, vector<shared_ptr<t_gobsgnss>> data2)
    {

        for (vector<shared_ptr<t_gobsgnss>>::iterator it2 = data2.begin(); it2 != data2.end(); it2++)
        {
            t_gtime epo = (*it2)->epoch();
            string prn = (*it2)->sat();
            bool foundPRN = false;
            for (vector<t_gobsgnss>::iterator it1 = data1.begin(); it1 != data1.end(); it1++)
            {
                if ((*it2)->sat() == it1->sat())
                {
                    foundPRN = true;
                    vector<GOBS> vgobs1 = it1->obs();
                    vector<GOBS> vgobs2 = (*it2)->obs();
                    for (vector<GOBS>::iterator itGOBS2 = vgobs2.begin(); itGOBS2 != vgobs2.end(); itGOBS2++)
                    {
                        bool foundGOBS = false;
                        for (vector<GOBS>::iterator itGOBS1 = vgobs1.begin(); itGOBS1 != vgobs1.end(); itGOBS1++)
                        {
                            if (*itGOBS2 == *itGOBS1)
                            {
                                foundGOBS = true;
                                break;
                            }
                        }
                        if (!foundGOBS && gobs_phase(*itGOBS2))
                            _mslipsGap[_site][epo][prn][*itGOBS2] = 3;
                    }
                    break;
                }
            }
            if (!foundPRN)
                _gapReport(*it2);
        }
    }

} // namespace
