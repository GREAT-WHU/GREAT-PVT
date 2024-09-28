/**
 * @file         gambiguity.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        ambiguity fixing by using LAMBDA.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gambfix/gambiguity.h"
#include "gset/gsetgen.h"
#include "gutils/gobs.h"
#include "gmodels/gpar.h"
#include <gproc/gfltmatrix.h>
using namespace std;

namespace great
{
    t_gambiguity::t_gambiguity()
    {
        _pdE = nullptr;
        _pdC = nullptr;
        _ratiofile = nullptr;
        _bootfile = nullptr;
        _ewl_Upd_time = t_gtime(EWL_IDENTIFY);
        _ewl24_Upd_time = t_gtime(EWL24_IDENTIFY);
        _ewl25_Upd_time = t_gtime(EWL25_IDENTIFY);
        _wl_Upd_time = t_gtime(WL_IDENTIFY);
    }

    t_gambiguity::t_gambiguity(string site, t_gsetbase *gset)
        : _part_fix(false),
          _is_first(true),
          _is_first_wl(true),
          _is_first_nl(true),
          _is_first_ewl(true),
          _is_first_ewl24(true),
          _is_first_ewl25(true),
          _amb_fixed(false),
          _obstype(OBSCOMBIN::IONO_FREE),
          _upd_mode(UPD_MODE::UPD),
          _fix_mode(FIX_MODE::SEARCH)
    {
        _site = site;
        _gset = gset;
        _beg = dynamic_cast<t_gsetgen *>(_gset)->beg();
        _end = dynamic_cast<t_gsetgen *>(_gset)->end();
        _sys = dynamic_cast<t_gsetgen *>(_gset)->sys();
        _sat_rm = dynamic_cast<t_gsetgen *>(_gset)->sat_rm();
        _interval = dynamic_cast<t_gsetgen *>(_gset)->sampling();

        _band_index[gnut::GPS] = dynamic_cast<t_gsetgnss *>(_gset)->band_index(gnut::GPS);
        _band_index[gnut::GAL] = dynamic_cast<t_gsetgnss *>(_gset)->band_index(gnut::GAL);
        _band_index[gnut::GLO] = dynamic_cast<t_gsetgnss *>(_gset)->band_index(gnut::GLO);
        _band_index[gnut::BDS] = dynamic_cast<t_gsetgnss *>(_gset)->band_index(gnut::BDS);
        _band_index[gnut::QZS] = dynamic_cast<t_gsetgnss *>(_gset)->band_index(gnut::QZS);

        _fix_mode = dynamic_cast<t_gsetamb *>(_gset)->fix_mode();
        _upd_mode = dynamic_cast<t_gsetamb *>(_gset)->upd_mode();

        _part_fix = dynamic_cast<t_gsetamb *>(_gset)->part_ambfix();
        _part_fix_num = dynamic_cast<t_gsetamb *>(_gset)->part_ambfix_num();
        _min_common_time = dynamic_cast<t_gsetamb *>(_gset)->min_common_time();
        _map_EWL_decision = dynamic_cast<t_gsetamb *>(_gset)->get_amb_decision("EWL");
        _map_WL_decision = dynamic_cast<t_gsetamb *>(_gset)->get_amb_decision("WL");
        _map_NL_decision = dynamic_cast<t_gsetamb *>(_gset)->get_amb_decision("NL");
        _ratio = dynamic_cast<t_gsetamb *>(_gset)->lambda_ratio();
        _boot = dynamic_cast<t_gsetamb *>(_gset)->bootstrapping();
        _frequency = dynamic_cast<t_gsetproc *>(_gset)->frequency();
        _full_fix_num = dynamic_cast<t_gsetamb *>(_gset)->full_fix_num();

        //init ratio and boot file
        if (_fix_mode == FIX_MODE::SEARCH)
            _initRatiofile(); 
        if (_boot > 0.0)
            _initBootfile();

        _ewl_Upd_time = t_gtime(EWL_IDENTIFY);
        _ewl24_Upd_time = t_gtime(EWL24_IDENTIFY);
        _ewl25_Upd_time = t_gtime(EWL25_IDENTIFY);
        _wl_Upd_time = t_gtime(WL_IDENTIFY);
    }

    t_gambiguity::~t_gambiguity()
    {
        if (_ratiofile)
        {
            if (_ratiofile->is_open())
                _ratiofile->close();
            delete _ratiofile;
            _ratiofile = NULL;
        }
        if (_bootfile)
        {
            if (_bootfile->is_open())
                _bootfile->close();
            delete _bootfile;
            _bootfile = nullptr;
        }
        if (_pdE)
        {
            delete _pdE;
            _pdE = nullptr;
        }
        if (_pdC)
        {
            delete _pdC;
            _pdC = nullptr;
        }
    }

    bool t_gambiguity::amb_fixed()
    {
        return _amb_fixed;
    }

    int t_gambiguity::processBatch(const t_gtime &t, t_gflt *gflt, string mode)
    {
        if (_gupd && _gupd->wl_epo_mode())
        { 
            _ewl_Upd_time = t;
            _wl_Upd_time = t;
            _ewl24_Upd_time = t;
            _ewl25_Upd_time = t;
        }
        for (auto it : _fix_epo_num[mode])
        {

            if (abs(t.diff(_last_fix_time[mode][it.first])) > _interval + 0.01)
                it.second = 0;
        }
        _sats_index.clear();
        _lock_epo_num.clear();

        vector<int> fixed_amb;
        _amb_freqs.clear();
        _DD.clear();
        _total_amb_num = _fixed_amb_num = 0;
        _outRatio = 0.0;
        t_gamb_cmn amb_cmn(t, gflt);
        amb_cmn.set_mode(mode);
        _crt_time = amb_cmn.now();

        int parnum = amb_cmn.param().parNumber();

        int koder = 0;
        if (mode == "EWL")
        {
            _is_first = _is_first_ewl;
            koder = 3;
        }
        else if (mode == "EWL24")
        {
            _is_first = _is_first_ewl24;
            koder = 4;
        }
        else if (mode == "EWL25")
        {
            _is_first = _is_first_ewl25;
            koder = 5;
        }
        else if (mode == "WL")
        {
            _is_first = _is_first_wl;
            koder = 2;
        }
        else if (mode == "NL")
        {
            _is_first = _is_first_nl;
            koder = 1;
        }

        // check if a new ambiguity is dependant of the already selected
        if (_is_first) 
        {
            int maxamb;
            maxamb = _max_active_amb_one_epo + parnum;
            _is_first = _checkAmbDepend(_is_first, 0, 0, 0, 0, maxamb, maxamb);
            _is_first = false;
        }

        if (mode == "EWL")
            _is_first_ewl = _is_first;
        else if (mode == "EWL24")
            _is_first_ewl24 = _is_first;
        else if (mode == "EWL25")
            _is_first_ewl25 = _is_first;
        else if (mode == "WL")
            _is_first_wl = _is_first;
        else if (mode == "NL")
            _is_first_nl = _is_first;

        // define double difference ambiguities over one baseline (PPP sd amb.)
        int namb = _defineDDAmb(&amb_cmn);
        if (namb < 0)
            return -1;
        _total_amb_num = namb;

        // calulate widelane upd for iono_free
        if (_obstype == OBSCOMBIN::RAW_ALL || _obstype == OBSCOMBIN::RAW_MIX)
        {
            if (mode != "NL" && !_calDDAmbWLALL(&amb_cmn, mode))
                return -1;
        }
        else if (_obstype == OBSCOMBIN::IONO_FREE)
        {
            if (!_calDDAmbWL())
                return -1;
        }

        // find DD that contains reference satellite
        if (!_sat_refs.empty() && !_findRefSD())
            return -1;

        // apply UPD correction
        // fix widelane and narrowlane ambiguities
        if (mode == "NL")
        {
            if (!_applyUpd(_crt_time))
                return -1;
            if (_obstype == OBSCOMBIN::IONO_FREE)
            {
                if (!_fixAmbIF())
                    return -1;
            }
            else if (_obstype == OBSCOMBIN::RAW_ALL || _obstype == OBSCOMBIN::RAW_MIX)
            {
                if (!_fixAmbUDUC())
                    return -1;
            }
        }
        else
        {
            if (!_applyWLUpd(_crt_time, mode))
                return -1;
            if (!_fixAmbWL(mode))
                return -1;
        }
        _DD_save = _DD;

        // select usable amb.
        int ndef = _selectAmb(koder, namb);
        if (ndef < 0)
            return -1;

        if (mode == "WL" || mode == "EWL")
        {
            for (auto itdd = _DD.begin(); itdd != _DD.end();)
            {
                if (itdd->ddSats.size() < 4)
                {
                    itdd = _DD.erase(itdd);
                    continue;
                }
                itdd++;
            }
        }

        // lambda search
        if (_fix_mode != FIX_MODE::NO && mode == "NL") 
        {
            if (!_ambSolve(&amb_cmn, fixed_amb, mode))
                return -1;
        }

        if (_last_fix_time[mode].find("sum") != _last_fix_time[mode].end())
        {
            if (t.diff(_last_fix_time[mode]["sum"]) >= _ctrl_last_fixepo_gap * _interval && fixed_amb.size() < _ctrl_min_fixed_num)
                return -1;
        }

        for (auto it_dd = _DD.begin(); it_dd != _DD.end(); it_dd++) 
        {
            if (mode == "NL")
            {
                it_dd->iwl = _IWL[get<0>(it_dd->ddSats[0])][get<0>(it_dd->ddSats[1])][it_dd->site];
                it_dd->iewl = _IEWL[get<0>(it_dd->ddSats[0])][get<0>(it_dd->ddSats[1])][it_dd->site];
                it_dd->iewl24 = _IEWL24[get<0>(it_dd->ddSats[0])][get<0>(it_dd->ddSats[1])];
                it_dd->iewl25 = _IEWL25[get<0>(it_dd->ddSats[0])][get<0>(it_dd->ddSats[1])];
            }
            if (mode == "WL")
                _IWL[get<0>(it_dd->ddSats[0])][get<0>(it_dd->ddSats[1])][it_dd->site] = it_dd->iwl;
            if (mode == "EWL")
                _IEWL[get<0>(it_dd->ddSats[0])][get<0>(it_dd->ddSats[1])][it_dd->site] = it_dd->iewl;
            if (mode == "EWL24")
                _IEWL24[get<0>(it_dd->ddSats[0])][get<0>(it_dd->ddSats[1])] = it_dd->iewl24;
            if (mode == "EWL25")
                _IEWL25[get<0>(it_dd->ddSats[0])][get<0>(it_dd->ddSats[1])] = it_dd->iewl25;
        }

        t_gflt fltpreAMB(*gflt);

        // add constraint
        if (mode == "NL")
        {
            if (!_addFixConstraint(gflt))
                return -1;
        }
        else
        {
            if (!_addFixConstraintWL(gflt, mode))
                return -1;
        }

        // amb state should be determined after addFixConstraint.
        if (amb_cmn.get_ratio() > _ratio && amb_cmn.get_boot() > _boot)
        {
            _amb_fixed = true;
            _fixed_amb_num = fixed_amb.size();

            _DD_previous[mode] = _DD;
            for (auto it_dd = _DD.begin(); it_dd != _DD.end(); it_dd++)
            {
                _fix_epo_num[mode][get<0>(it_dd->ddSats[0])]++;
                _last_fix_time[mode][get<0>(it_dd->ddSats[0])] = t;
                _fix_epo_num[mode][get<0>(it_dd->ddSats[1])]++;
                _last_fix_time[mode][get<0>(it_dd->ddSats[1])] = t;
            }
            _last_fix_time[mode]["sum"] = t;
        }
        else
        {
            _amb_fixed = false;
            _fixed_amb_num = 0;
        }
        _outRatio = amb_cmn.get_ratio(); 
        auto tmp_param = gflt->param();
        auto tmp_dx = gflt->dx();

        updateFixParam(tmp_param, tmp_dx); 

        return 1;
    }


    t_gallpar &t_gambiguity::getFinalParams()
    {
        return _param;
    }

    void t_gambiguity::setSatRef(set<string> &satRef)
    {
        _sat_refs = satRef;

        _EWL_flag.clear();
        _IEWL.clear();
        _EWL24_flag.clear();
        _IEWL24.clear();
        _EWL25_flag.clear();
        _IEWL25.clear();
        _WL_flag.clear();
        _IWL.clear();
    }

    void t_gambiguity::updateFixParam(t_gallpar &param, ColumnVector &dx, ColumnVector *stdx)
    {
        _param.delAllParam();
        _param = param;

        for (unsigned int i = 0; i < param.parNumber(); i++)
        {
            _param[i].value(param[i].value() + dx(i + 1));
            if (stdx)
                _param[i].apriori((*stdx)(i + 1));
        }
    }

    double t_gambiguity::lambdaSolve(double &ratio, const Matrix &anor, const ColumnVector &fltpar, ColumnVector &ibias, bool parlamb)
    {
        double boot_temp = 0.0; //by qzy
        _ratio = ratio;
        if (parlamb)
            _part_fix = true;
        else
            _part_fix = false;

        int namb = fltpar.Nrows();
        vector<double> fltpar_;
        vector<int> ibias_;

        for (int i = 1; i <= namb; i++)
        {
            fltpar_.push_back(fltpar(i));
        }
        double ratio_post = _lambdaSearch(anor, fltpar_, ibias_, &boot_temp);

        if (ratio_post > 0)
        {
            if (ibias_.size() == namb)
            {
                for (int i = 1; i <= namb; i++)
                {
                    ibias(i) = ibias_[i - 1];
                }
            }
        }
        else
            ratio_post = 0.0;

        return ratio_post;
    }

    void t_gambiguity::setWaveLength(map<string, int> &glofrq_num)
    {
        set<string> waveLenSats;

        // get system/satellite need to calculate wavelength
        for (auto strsys : _sys)
        {
            GSYS gsys = t_gsys::str2gsys(strsys);
            switch (gsys)
            {
            case GPS:
                waveLenSats.insert("G");
                break;
            case GAL:
                waveLenSats.insert("E");
                break;
            case BDS:
                waveLenSats.insert("C");
                break;
            case GLO: // FDMA
                for (auto it_R : glofrq_num)
                {
                    waveLenSats.insert(it_R.first);
                }
                break;
            case QZS:
                waveLenSats.insert("J");
                break;
            default:
                break;
            }
        }

        t_gobsgnss *gnss = new t_gobsgnss(_spdlog);
        // Loop waveLenSats
        for (auto iter : waveLenSats)
        {
            if (iter == "2" || iter == "3" || iter == "4" || iter == "5") //xjhan
                gnss->sat("G");
            else
                gnss->sat(iter);

            GSYS gsys = gnss->gsys();
            if (iter.substr(0, 1) == "R")
                gnss->channel(glofrq_num.at(iter));

            _sys_wavelen[iter]["L1"] = gnss->wavelength(_band_index[gsys][FREQ_1]);
            _sys_wavelen[iter]["L2"] = gnss->wavelength(_band_index[gsys][FREQ_2]);
            _sys_wavelen[iter]["WL"] = gnss->wavelength_WL(_band_index[gsys][FREQ_1], _band_index[gsys][FREQ_2]);
            _sys_wavelen[iter]["NL"] = gnss->wavelength_NL(_band_index[gsys][FREQ_1], _band_index[gsys][FREQ_2]);
            if (_frequency >= 3)
            {
                _sys_wavelen[iter]["L3"] = gnss->wavelength(_band_index[gsys][FREQ_3]);
                _sys_wavelen[iter]["EWL"] = gnss->wavelength_WL(_band_index[gsys][FREQ_2], _band_index[gsys][FREQ_3]);
            }
            if (_frequency >= 4)
            {
                _sys_wavelen[iter]["L4"] = gnss->wavelength(_band_index[gsys][FREQ_4]);
                _sys_wavelen[iter]["EWL24"] = gnss->wavelength_WL(_band_index[gsys][FREQ_2], _band_index[gsys][FREQ_4]);
            }
            if (_frequency >= 5)
            {
                _sys_wavelen[iter]["L5"] = gnss->wavelength(_band_index[gsys][FREQ_5]);
                _sys_wavelen[iter]["EWL25"] = gnss->wavelength_WL(_band_index[gsys][FREQ_2], _band_index[gsys][FREQ_5]);
            }
        }

        delete gnss;
        gnss = nullptr;

        return;
    }

    bool t_gambiguity::_checkAmbDepend(bool isFirst, int iNamb, int *iNdef, int iN_oneway, int *arriIpt2ow, int iMaxamb_ow, int iMaxamb_for_check)
    {

        const string strCprogName = "check_amb_depend";
        const double dEPS = 1e-12;
        static int iNdim_ow = 0;
        static int iNdim_for_check = 0;
        int i, j;
        const double dOper[4] = {1.0, -1.0, -1.0, 1.0};
        double dC_dot;
        bool isLdepend;

        try
        {
            if (isFirst)
            {
                if (_pdE)
                {
                    delete _pdE;
                    _pdE = nullptr;
                }
                if (_pdC)
                {
                    delete _pdC;
                    _pdC = nullptr;
                }

                _pdE = new double[iMaxamb_ow * (iMaxamb_for_check + 1 + 100)];
                _pdC = new double[iMaxamb_for_check + 1 + 100];
                if (iMaxamb_ow == 0 || iMaxamb_for_check == 0)
                {
                    string err = "***ERROR: memory allocatation for e&c ";
                    throw err;
                }
                iNdim_ow = iMaxamb_ow;
                iNdim_for_check = iMaxamb_for_check;
                return false;
            }

            if (*iNdef >= iNdim_for_check)
            {
                string err2 = "***ERROR: independent ones already reaches the allocated memory ";
                throw err2;
            }

            dC_dot = 0.0;
            for (i = 0; i < *iNdef; i++)
            {
                _pdC[i] = 0.0;
                for (j = 0; j < iN_oneway; j++)
                {
                    if (arriIpt2ow[j] > iNdim_ow)
                    {
                        string err3 = "***ERROR: base element beyond the allocated memory ";
                        throw err3;
                    }
                    _pdC[i] = _pdC[i] + _pdE[(arriIpt2ow[j] - 1) * (iNdim_for_check + 1) + i] * dOper[j];
                }
                dC_dot = dC_dot + _pdC[i] * _pdC[i];
            }

            isLdepend = false;
            if (abs(dC_dot - iN_oneway) <= dEPS)
            {
                isLdepend = true;
                return isLdepend;
            }

            for (j = 0; j < iNamb; j++)
            {
                _pdE[j * (iNdim_for_check + 1) + *iNdef] = 0.0;
            }
            for (j = 0; j < iN_oneway; j++)
            {
                _pdE[(arriIpt2ow[j] - 1) * (iNdim_for_check + 1) + *iNdef] = dOper[j];
            }

            if (dC_dot > dEPS)
            {
                for (i = 0; i < *iNdef; i++)
                {
                    if (abs(_pdC[i]) < dEPS)
                    {
                        continue;
                    }
                    for (j = 0; j < iNamb; j++)
                    {
                        _pdE[j * (iNdim_for_check + 1) + *iNdef] = _pdE[j * (iNdim_for_check + 1) + *iNdef] - _pdC[i] * _pdE[j * (iNdim_for_check + 1) + i];
                    }
                }

                dC_dot = 0.0;
                for (j = 0; j < iNamb; j++)
                {
                    dC_dot = dC_dot + _pdE[j * (iNdim_for_check + 1) + *iNdef] * _pdE[j * (iNdim_for_check + 1) + *iNdef];
                }
                dC_dot = sqrt(dC_dot);
                for (j = 0; j < iNamb; j++)
                {
                    _pdE[j * (iNdim_for_check + 1) + *iNdef] = _pdE[j * (iNdim_for_check + 1) + *iNdef] / dC_dot;
                }
            }
            else
            {
                dC_dot = sqrt(iN_oneway * 1.0);
                for (j = 0; j < iN_oneway; j++)
                {
                    _pdE[(arriIpt2ow[j] - 1) * (iNdim_for_check + 1) + *iNdef] = _pdE[(arriIpt2ow[j] - 1) * (iNdim_for_check + 1) + *iNdef] / dC_dot;
                }
            }

            _pdC[*iNdef] = dC_dot;
            *iNdef = *iNdef + 1;
            return false;
        }
        catch (...)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR[t_gambiguity::_check_amb_depend] : throw exception");
            return false;
        }
    }

    int t_gambiguity::_defineDDAmb(t_gamb_cmn *amb_cmn)
    {
        vector<t_gpar> params_ALL = amb_cmn->param().getAllPar();
        vector<t_gpar> params;
        int amb_idx1, amb_idx2;

        // prepare amb par
        for (auto it_par = params_ALL.begin(); it_par != params_ALL.end(); it_par++)
        {
            it_par->index = distance(params_ALL.begin(), it_par) + 1;
            it_par->pred = it_par->value() + amb_cmn->dx()(it_par->index); 
            if (it_par->str_type().find("AMB") == string::npos ||
                it_par->str_type().find("AMB13") != string::npos ||
                _sat_rm.find(it_par->prn) != _sat_rm.end() ||
                double_eq(it_par->value(), 0.0))
            {
                continue;
            }
            else
            {
                if (it_par->str_type().find("AMB_L1") != string::npos)
                {
                    _amb_freqs[it_par->prn][it_par->site].push_back(FREQ_1);                
                }
                else if (it_par->str_type().find("AMB_L2") != string::npos)
                {
                    _amb_freqs[it_par->prn][it_par->site].push_back(FREQ_2);
                }

                params.push_back(*it_par);
                _sats_index[it_par->prn][it_par->str_type().substr(0, 6)] = it_par->index;
            }

            _lock_epo_num[it_par->prn] = round((it_par->end - it_par->beg) / _interval) + 1;
            if (_last_fix_time[amb_cmn->get_mode()].find(it_par->prn) == _last_fix_time[amb_cmn->get_mode()].end())
                _last_fix_time[amb_cmn->get_mode()][it_par->prn] = FIRST_TIME;
            if (_fix_epo_num[amb_cmn->get_mode()].find(it_par->prn) == _fix_epo_num[amb_cmn->get_mode()].end())
                _fix_epo_num[amb_cmn->get_mode()][it_par->prn] = 0;
        }

        if (params.size() < 2)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR : Ambguity's number is less than 2");
            return -1;
        }

        vector<t_gpar>::const_iterator OuterLoopBeg = params.begin();
        vector<t_gpar>::const_iterator InnerLoopEnd = params.end();
        vector<t_gpar>::const_iterator OuterLoopEnd = --(params.end());
        for (auto itsat1 = OuterLoopBeg; itsat1 != OuterLoopEnd; itsat1++)
        {
            amb_idx1 = distance(OuterLoopBeg, itsat1) + 1;
            for (auto itsat2 = itsat1 + 1; itsat2 != InnerLoopEnd; itsat2++)
            {
                // whether same system
                if (t_gsys::sat2gsys(itsat1->prn) != t_gsys::sat2gsys(itsat2->prn)
                    || itsat1->site != itsat2->site)
                    continue;

                // whether same satellite
                if (itsat1->prn == itsat2->prn)
                    continue;

                // the same ambiguity type
                if (itsat1->parType != itsat2->parType)
                    continue;

                t_dd_ambiguity dd;

                // common time
                dd.beg_epo = (itsat1->beg > itsat2->beg ? itsat1->beg : itsat2->beg);
                dd.end_epo = (itsat1->end < itsat2->end ? itsat1->end : itsat2->end);
                dd.end_epo = (_crt_time < dd.end_epo ? _crt_time : dd.end_epo);

                // judge whether this combination is valid
                if ((dd.end_epo - dd.beg_epo) < _min_common_time)
                    continue;

                dd.ambtype = itsat1->str_type().substr(0, 6);
                dd.isEwlFixed = false;
                dd.isEwl24Fixed = false;
                dd.isEwl25Fixed = false;
                dd.isWlFixed = false;
                dd.isNlFixed = false;

                amb_idx2 = distance(OuterLoopBeg, itsat2) + 1;
                dd.ddSats.push_back(make_tuple(itsat1->prn, itsat1->index, amb_idx1));
                dd.ddSats.push_back(make_tuple(itsat2->prn, itsat2->index, amb_idx2));

                dd.site = itsat1->site;

                double sigma = 0.0;
                dd.rlc = itsat1->pred - itsat2->pred; // Unit:meter
                if (itsat1->prn.substr(0, 1) == "R")
                {
                    if (_sys_wavelen[itsat1->prn]["NL"] == 0)
                        continue;
                    dd.rlc = itsat1->pred / _sys_wavelen[itsat1->prn]["NL"] - itsat2->pred / _sys_wavelen[itsat2->prn]["NL"]; // Unit:Cycle
                    if (_obstype == OBSCOMBIN::RAW_ALL)
                    {
                        dd.rwl_R1 = itsat2->pred / _sys_wavelen[itsat2->prn]["NL"]; // temp save, it is not its real meaning!
                    }
                }

                if (_fix_mode != FIX_MODE::NO)
                {
                    // get dd ambiguity _sigma
                    sigma = amb_cmn->Qx()(itsat1->index, itsat1->index) - 2 * amb_cmn->Qx()(itsat1->index, itsat2->index) + amb_cmn->Qx()(itsat2->index, itsat2->index);
                    if (double_eq(sigma, 0.0))
                        continue;
                    if (amb_cmn->sigma0() < 0)
                        continue;
                    dd.srlc = amb_cmn->sigma0() * sqrt(abs(sigma)) * sigma / abs(sigma);
                }
                _DD.push_back(dd);
            }
        }

        if (_DD.empty())
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR[t_gambiguity::_defineDDAmb] : double-Difference ambiguity is empty");
            return -1;
        }
        else
            return params.size();
    }

    bool t_gambiguity::_calDDAmbWL()
    {
        string sat1, sat2;
        string sat11, sat12, sat21, sat22;
        //widelane ambiguities from MW-combination
        for (auto itdd = _DD.begin(); itdd != _DD.end();)
        {
            sat1 = get<0>(itdd->ddSats[0]);
            sat2 = get<0>(itdd->ddSats[1]);
            if (itdd->ddSats.size() != 2 || _MW.find(sat1) == _MW.end() || _MW.find(sat2) == _MW.end() || _MW[sat1][1] <= 2.0 || _MW[sat2][1] <= 2.0)
            {
                itdd = _DD.erase(itdd);
                continue;
            }
            else
            {
                itdd->rwl = _MW[sat1][2] - _MW[sat2][2];
                itdd->srwl = sqrt(abs(_MW[sat1][3] / _MW[sat1][1]) + abs(_MW[sat2][3] / _MW[sat2][1]));

                if (sat1.substr(0, 1) == "R")
                {
                    itdd->rwl_R1 = _MW[sat1][2];
                    itdd->srwl_R1 = sqrt(abs(_MW[sat1][3] / _MW[sat1][1]));
                }
                if (sat2.substr(0, 1) == "R")
                {
                    itdd->rwl_R2 = _MW[sat2][2];
                    itdd->srwl_R2 = sqrt(abs(_MW[sat2][3] / _MW[sat2][1]));
                }
                itdd++;
            }
        }
        if (_DD.empty())
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR[t_gambiguity::_calDDAmbWL] : Double-Difference ambiguity is empty");
            return false;
        }
        else
            return true;
    }
    bool t_gambiguity::_calDDAmbWLALL(t_gamb_cmn *amb_cmn, string mode)
    {

        string sat11, sat12, sat21, sat22, ambtype1, ambtype2;
        double qq, lambda_1 = 0, lambda_2 = 0, lambda_21 = 0, lambda_22 = 0;

        for (auto itdd = _DD.begin(); itdd != _DD.end(); itdd++)
        {
            sat11 = get<0>(itdd->ddSats[0]);
            sat12 = get<0>(itdd->ddSats[1]);

            if (mode == "WL")
            {
                if (sat11.substr(0, 1) != "R")
                {
                    lambda_1 = _sys_wavelen[sat11.substr(0, 1)]["L1"];
                    lambda_2 = _sys_wavelen[sat11.substr(0, 1)]["L2"];
                }
                else
                {
                    lambda_1 = _sys_wavelen[sat11]["L1"];
                    lambda_2 = _sys_wavelen[sat11]["L2"];
                    lambda_21 = _sys_wavelen[sat12]["L1"];
                    lambda_22 = _sys_wavelen[sat12]["L2"];
                }
                ambtype1 = "AMB_L1";
                ambtype2 = "AMB_L2";
            }
            else if (mode == "EWL")
            {
                lambda_1 = _sys_wavelen[sat11.substr(0, 1)]["L2"];
                lambda_2 = _sys_wavelen[sat11.substr(0, 1)]["L3"];
                ambtype1 = "AMB_L2";
                ambtype2 = "AMB_L3";
            }
            else if (mode == "EWL24")
            {
                lambda_1 = _sys_wavelen[sat11.substr(0, 1)]["L2"];
                lambda_2 = _sys_wavelen[sat11.substr(0, 1)]["L4"];
                ambtype1 = "AMB_L2";
                ambtype2 = "AMB_L4";
            }
            else if (mode == "EWL25")
            {
                lambda_1 = _sys_wavelen[sat11.substr(0, 1)]["L2"];
                lambda_2 = _sys_wavelen[sat11.substr(0, 1)]["L5"];
                ambtype1 = "AMB_L2";
                ambtype2 = "AMB_L5";
            }
            if (itdd->ambtype != ambtype1)
                continue;

            for (auto itdd1 = _DD.begin(); itdd1 != _DD.end(); itdd1++)
            {
                sat21 = get<0>(itdd1->ddSats[0]);
                sat22 = get<0>(itdd1->ddSats[1]);

                if (itdd1->ambtype != ambtype2)
                    continue;
                
                if (itdd1->site != itdd->site)
                    continue;

                if ((sat11 == sat21) && (sat12 == sat22))
                {
                    if (sat11.substr(0, 1) != "R")
                    {
                        itdd->rwl = itdd->rlc / lambda_1 - itdd1->rlc / lambda_2;
                        
                    }
                    else
                    {
                        itdd->rwl = (itdd->rlc + itdd->rwl_R1) * _sys_wavelen[sat11]["NL"] / lambda_1 - itdd->rwl_R1 * _sys_wavelen[sat12]["NL"] / lambda_21; 
                        itdd->rwl -= (itdd1->rlc + itdd1->rwl_R1) * _sys_wavelen[sat11]["NL"] / lambda_2 - itdd1->rwl_R1 * _sys_wavelen[sat12]["NL"] / lambda_22;
                    }
                    qq = amb_cmn->Qx()(get<1>(itdd->ddSats[0]), get<1>(itdd1->ddSats[0])) / (lambda_1 * lambda_1) - amb_cmn->Qx()(get<1>(itdd->ddSats[0]), get<1>(itdd1->ddSats[1])) / (lambda_1 * lambda_2) -
                         amb_cmn->Qx()(get<1>(itdd->ddSats[1]), get<1>(itdd1->ddSats[0])) / (lambda_1 * lambda_2) + amb_cmn->Qx()(get<1>(itdd->ddSats[1]), get<1>(itdd1->ddSats[1])) / (lambda_2 * lambda_2);
                    if (double_eq(qq, 0.0))
                        continue;
                    if (amb_cmn->sigma0() < 0)
                        continue;

                    itdd->srwl = itdd->srlc / (lambda_1 * lambda_1) + itdd1->srlc / (lambda_2 * lambda_2) - 2 * (amb_cmn->sigma0() * sqrt(abs(qq)) * qq / abs(qq));
                    itdd->ddSats.push_back(make_tuple(sat21, get<1>(itdd1->ddSats[0]), get<2>(itdd1->ddSats[0])));
                    itdd->ddSats.push_back(make_tuple(sat22, get<1>(itdd1->ddSats[1]), get<2>(itdd1->ddSats[1])));
                    break;
                }
            }
        }

        if (_DD.empty())
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR[t_gambiguity::_calDDAmbWL] : Double-Difference ambiguity is empty");
            return false;
        }
        else
            return true;
    }

    bool t_gambiguity::_getSingleUpd(string mode, t_gtime t, string sat, double &value, double &sigma)
    {
        UPDTYPE upd_type = str2updmode(trim(mode));
        one_epoch_upd epoch_upd = _gupd->get_epo_upd(upd_type, t);
        value = 0.0;
        if (epoch_upd.find(sat) == epoch_upd.end())
            return false;
        if (mode == "EWL" || mode == "EWL24" || mode == "EWL25")
        {
            if (epoch_upd[sat]->sigma > 0.2 || epoch_upd[sat]->npoint <= 2)
                return false;
            value = epoch_upd[sat]->value;
        }
        else if (mode == "EWL_epoch")
        {
            //one_epoch_upd_epoch epoch_upd_epoch = _gupd_epoch->get_epo_upd_epoch(t);
            if (epoch_upd[sat]->sigma > 0.2 || epoch_upd[sat]->npoint <= 2)
                return false;
            value = epoch_upd[sat]->value;
        }
        else if (mode == "WL")
        {
            if (epoch_upd[sat]->sigma > 0.2 || epoch_upd[sat]->npoint <= 2)
                return false;
            value = epoch_upd[sat]->value;
        }
        else if (mode == "NL")
        {
            if (epoch_upd[sat]->sigma > 0.1 || epoch_upd[sat]->npoint <= 3)
                return false;
            value = epoch_upd[sat]->value;
            sigma += pow(epoch_upd[sat]->sigma, 2);
        }
        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applySingleUpd] : Undefined UPD Mode (WL/NL) : " + mode);
            return false;
        }
        return true;
    }

    bool t_gambiguity::_applyUpd(t_gtime t)
    {
        string sat1, sat2;
        double upd_wl1, upd_wl2, upd_ewl1, upd_ewl2, upd_nl1, upd_nl2, sig, rec_upd;
        double upd_ewl24_1, upd_ewl24_2, upd_ewl25_1, upd_ewl25_2;
        upd_wl1 = upd_wl2 = rec_upd = 0.0;

        if (_gupd)
        {
            if (_sys.find("GLO") != _sys.end())
            {
                rec_upd = _glonassRecUpd(_site);
            }
        }

        for (auto itdd = _DD.begin(); itdd != _DD.end();)
        {
            sat1 = get<0>(itdd->ddSats[0]);
            sat2 = get<0>(itdd->ddSats[1]);

            upd_nl1 = 0.0, upd_nl2 = 0.0, sig = 0.0;
            upd_wl1 = 0.0, upd_wl2 = 0.0;
            upd_ewl1 = 0.0, upd_ewl2 = 0.0;
            upd_ewl24_1 = 0.0, upd_ewl24_2 = 0.0;
            upd_ewl25_1 = 0.0, upd_ewl25_2 = 0.0;

            if (_gupd)
            {
                // get Extrawidelane UPD
                if (_frequency >= 3 && _obstype == OBSCOMBIN::RAW_ALL)
                {
                    if ((!_getSingleUpd("EWL", _ewl_Upd_time, sat1, upd_ewl1, sig) || !_getSingleUpd("EWL", _ewl_Upd_time, sat2, upd_ewl2, sig)))
                    {
                        if (itdd->ambtype == "AMB_L3")
                        {
                            if (_spdlog)
                                SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applyUpd] : _getSingleUpd Wrong : EWL, Sat: " + sat1 + " " + sat2);
                            itdd = _DD.erase(itdd);
                            continue;
                        }
                        else
                        {
                            upd_ewl1 = 0.0, upd_ewl2 = 0.0;
                        }
                    }
                }
                // get Extrawidelane-24 UPD
                if (_frequency >= 4 && _obstype == OBSCOMBIN::RAW_ALL)
                {
                    if ((!_getSingleUpd("EWL24", _ewl24_Upd_time, sat1, upd_ewl24_1, sig) || !_getSingleUpd("EWL24", _ewl24_Upd_time, sat2, upd_ewl24_2, sig)))
                    {
                        if (itdd->ambtype == "AMB_L4")
                        {
                            if (_spdlog)
                                SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applyUpd] : _getSingleUpd Wrong : EWL24, Sat: " + sat1 + " " + sat2);
                            itdd = _DD.erase(itdd);
                            continue;
                        }
                        else
                        {
                            upd_ewl24_1 = 0.0, upd_ewl24_2 = 0.0;
                        }
                    }
                }
                // get Extrawidelane-25 UPD
                if (_frequency == 5 && _obstype == OBSCOMBIN::RAW_ALL)
                {
                    if ((!_getSingleUpd("EWL25", _ewl25_Upd_time, sat1, upd_ewl25_1, sig) || !_getSingleUpd("EWL25", _ewl25_Upd_time, sat2, upd_ewl25_2, sig)))
                    {
                        if (itdd->ambtype == "AMB_L5")
                        {
                            if (_spdlog)
                                SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applyUpd] : _getSingleUpd Wrong : EWL25, Sat: " + sat1 + " " + sat2);
                            itdd = _DD.erase(itdd);
                            continue;
                        }
                        else
                        {
                            upd_ewl25_1 = 0.0, upd_ewl25_2 = 0.0;
                        }
                    }
                }
                // get Widelane UPD
                if (_upd_mode == UPD_MODE::UPD)
                {
                    if (!_getSingleUpd("WL", _wl_Upd_time, sat1, upd_wl1, sig) || !_getSingleUpd("WL", _wl_Upd_time, sat2, upd_wl2, sig))
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applyUpd] : _getSingleUpd Wrong : WL, Sat: " + sat1 + " " + sat2);
                        itdd = _DD.erase(itdd);
                        continue;
                    }
                }
                // get Narrowlane UPD
                if (_upd_mode == UPD_MODE::UPD)
                {
                    if (!_getSingleUpd("NL", t, sat1, upd_nl1, sig) || !_getSingleUpd("NL", t, sat2, upd_nl2, sig))
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applyUpd] : _getSingleUpd Wrong : NL, Sat: " + sat1 + " " + sat2);
                        itdd = _DD.erase(itdd);
                        continue;
                    }
                }
            }

            itdd->sd_rnl_cor = upd_nl1 - upd_nl2;
            itdd->sigcor = sqrt(sig);

            if (_obstype == OBSCOMBIN::RAW_ALL || _obstype == OBSCOMBIN::RAW_MIX)
            {
                // get Narrowlane Ambiguity + apply Widelane + Narrowlane UPD
                if (sat1.substr(0, 1) != "R")
                {
                    double lambda_1, lambda_2, lambda_3, lambda_4, lambda_5;
                    lambda_1 = _sys_wavelen[sat1.substr(0, 1)]["L1"];
                    lambda_2 = _sys_wavelen[sat2.substr(0, 1)]["L2"];
                    lambda_3 = _sys_wavelen[sat2.substr(0, 1)]["L3"];
                    lambda_4 = _sys_wavelen[sat2.substr(0, 1)]["L4"];
                    lambda_5 = _sys_wavelen[sat2.substr(0, 1)]["L5"];
                    itdd->sd_r1_cor = itdd->sd_rnl_cor - lambda_1 / (lambda_2 - lambda_1) * (-upd_wl1 + upd_wl2);
                    itdd->sd_r2_cor = itdd->sd_rnl_cor - lambda_2 / (lambda_2 - lambda_1) * (-upd_wl1 + upd_wl2);
                    if (upd_ewl1 != 0.0 && upd_ewl2 != 0.0)
                    {
                        itdd->sd_r3_cor = itdd->sd_r2_cor - (-upd_ewl1 + upd_ewl2);
                    }
                    if (upd_ewl24_1 != 0.0 && upd_ewl24_2 != 0.0)
                    {
                        itdd->sd_r4_cor = itdd->sd_r2_cor - (-upd_ewl24_1 + upd_ewl24_2);
                    }
                    if (upd_ewl25_1 != 0.0 && upd_ewl25_2 != 0.0)
                    {
                        itdd->sd_r5_cor = itdd->sd_r2_cor - (-upd_ewl25_1 + upd_ewl25_2);
                    }

                    if (itdd->ambtype == "AMB_L1")
                    {
                        itdd->rnl = itdd->rlc / lambda_1;
                        itdd->rnl += itdd->sd_r1_cor;
                        itdd->srnl = itdd->srlc / (lambda_1);
                        itdd->factor = lambda_1;
                    }
                    else if (itdd->ambtype == "AMB_L2")
                    {
                        itdd->rnl = itdd->rlc / lambda_2;
                        itdd->rnl += itdd->sd_r2_cor;
                        itdd->srnl = itdd->srlc / (lambda_2);
                        itdd->factor = lambda_2;
                    }
                    else if (itdd->ambtype == "AMB_L3")
                    {
                        itdd->rnl = itdd->rlc / lambda_3;
                        itdd->rnl += itdd->sd_r3_cor;
                        itdd->srnl = itdd->srlc / (lambda_3);
                        itdd->factor = lambda_3;
                    }
                    else if (itdd->ambtype == "AMB_L4")
                    {
                        itdd->rnl = itdd->rlc / lambda_4;
                        itdd->rnl += itdd->sd_r4_cor;
                        itdd->srnl = itdd->srlc / (lambda_4);
                        itdd->factor = lambda_4;
                    }
                    else if (itdd->ambtype == "AMB_L5")
                    {
                        itdd->rnl = itdd->rlc / lambda_5;
                        itdd->rnl += itdd->sd_r5_cor;
                        itdd->srnl = itdd->srlc / (lambda_5);
                        itdd->factor = lambda_5;
                    }
                }
                else
                { 
                    double lambda_11, lambda_12, lambda_21, lambda_22;
                    lambda_11 = _sys_wavelen[sat1]["L1"];
                    lambda_12 = _sys_wavelen[sat1]["L2"];
                    lambda_21 = _sys_wavelen[sat2]["L1"];
                    lambda_22 = _sys_wavelen[sat2]["L2"];

                    itdd->sd_r1_cor = upd_nl1 + lambda_11 / (lambda_12 - lambda_11) * upd_wl1;
                    itdd->sd_r1_cor -= (upd_nl2 + lambda_21 / (lambda_22 - lambda_21) * upd_wl2);
                    itdd->sd_r2_cor = upd_nl1 + lambda_12 / (lambda_12 - lambda_11) * upd_wl1;
                    itdd->sd_r2_cor -= (upd_nl2 + lambda_22 / (lambda_22 - lambda_21) * upd_wl2);

                    if (itdd->ambtype == "AMB_L1")
                    {
                        itdd->rnl = (itdd->rlc + itdd->rwl_R1) * _sys_wavelen[sat1]["NL"] / lambda_11;
                        itdd->rnl -= ((itdd->rwl_R1) * _sys_wavelen[sat2]["NL"] / lambda_21);
                        itdd->rnl += itdd->sd_r1_cor;
                        itdd->srnl = itdd->srlc / (lambda_11);
                        itdd->factor = lambda_11;
                    }
                    else if (itdd->ambtype == "AMB_L2")
                    {
                        itdd->rnl = (itdd->rlc + itdd->rwl_R1) * _sys_wavelen[sat1]["NL"] / lambda_12;
                        itdd->rnl -= ((itdd->rwl_R1) * _sys_wavelen[sat2]["NL"] / lambda_22);
                        itdd->rnl += itdd->sd_r2_cor;
                        itdd->srnl = itdd->srlc / (lambda_21);
                        itdd->factor = lambda_21;
                    }
                }
            }

            if (_obstype == OBSCOMBIN::IONO_FREE)
            {
                if (sat1.substr(0, 1) != "R")
                {
                    // apply Widelane UPD
                    if (_upd_mode == UPD_MODE::UPD)
                        itdd->rwl += (-upd_wl1 + upd_wl2);
                    itdd->rnl = itdd->rlc / _sys_wavelen[sat1.substr(0, 1)]["NL"] - round(itdd->rwl) * _sys_wavelen[sat1.substr(0, 1)]["WL"] / _sys_wavelen[sat1.substr(0, 1)]["L2"];
                    itdd->srnl = itdd->srlc / _sys_wavelen[sat1.substr(0, 1)]["NL"];
                    itdd->factor = _sys_wavelen[sat1.substr(0, 1)]["NL"];
                    if (_upd_mode == UPD_MODE::UPD)
                        itdd->rnl += itdd->sd_rnl_cor;
                }
                else
                {
                    if (_upd_mode == UPD_MODE::UPD)
                        itdd->rwl += (-upd_wl1 + upd_wl2);
                    itdd->rnl = itdd->rlc - round(itdd->rwl) * 3.5;
                    itdd->srnl = itdd->srlc / _sys_wavelen[sat1]["NL"];
                    itdd->factor = _sys_wavelen[sat1]["NL"]; 
                    if (_upd_mode == UPD_MODE::UPD)
                        itdd->rnl += itdd->sd_rnl_cor;
                }
            }
            itdd++;
        }

        return true;
    }

    bool t_gambiguity::_applyWLUpd(t_gtime t, string mode)
    {
        string sat1, sat2;
        double upd_wl1, upd_wl2, upd_ewl1, upd_ewl2, sig;
        upd_wl1 = upd_wl2 = upd_ewl1 = upd_ewl2 = 0.0;

        for (auto itdd = _DD.begin(); itdd != _DD.end();)
        {
            sat1 = get<0>(itdd->ddSats[0]);
            sat2 = get<0>(itdd->ddSats[1]);

            sig = 0.0;
            if (_gupd) 
            {
                // get Extrawidelane UPD
                if (_frequency >= 3 && mode == "EWL")
                {
                    if ((!_getSingleUpd("EWL", _ewl_Upd_time, sat1, upd_ewl1, sig)) || 
                       (!_getSingleUpd("EWL", _ewl_Upd_time, sat2, upd_ewl2, sig) && mode == "EWL"))
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applyUpd] : _getSingleUpd Wrong : EWL, Sat: " + sat1 + " " + sat2);
                        itdd = _DD.erase(itdd);
                        continue;
                    }
                }
                else if (_frequency >= 4 && mode == "EWL24")
                {
                    if (!_getSingleUpd("EWL24", _ewl24_Upd_time, sat1, upd_ewl1, sig) || 
                       (!_getSingleUpd("EWL24", _ewl24_Upd_time, sat2, upd_ewl2, sig) && mode == "EWL24"))
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applyUpd] : _getSingleUpd Wrong : EWL24, Sat: " + sat1 + " " + sat2);
                        itdd = _DD.erase(itdd);
                        continue;
                    }
                }
                else if (_frequency == 5 && mode == "EWL25")
                {
                    if (!_getSingleUpd("EWL25", _ewl25_Upd_time, sat1, upd_ewl1, sig) || 
                    (!_getSingleUpd("EWL25", _ewl25_Upd_time, sat2, upd_ewl2, sig) && mode == "EWL25"))
                    {
                        if (_spdlog)
                            SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applyUpd] : _getSingleUpd Wrong : EWL25, Sat: " + sat1 + " " + sat2);
                        itdd = _DD.erase(itdd);
                        continue;
                    }
                }
                // get Widelane UPD
                if (!_getSingleUpd("WL", _wl_Upd_time, sat1, upd_wl1, sig) || !_getSingleUpd("WL", _wl_Upd_time, sat2, upd_wl2, sig))
                {
                    if (_spdlog)
                        SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_applyUpd] : _getSingleUpd Wrong : WL, Sat: " + sat1 + " " + sat2);
                    itdd = _DD.erase(itdd);
                    continue;
                }
            }

            // get Narrowlane Ambiguity + apply Widelane + Narrowlane UPD
            if (sat1.substr(0, 1) != "R")
            {
                if (itdd->ambtype == "AMB_L2")
                {
                    itdd->factor = _sys_wavelen[sat2.substr(0, 1)]["L2"];
                }

                if (mode == "WL" && itdd->rwl != 0)
                {
                    itdd->rwl += (-upd_wl1 + upd_wl2);
                    itdd->sd_rwl_cor = (-upd_wl1 + upd_wl2);
                    itdd->factor = _sys_wavelen[sat1.substr(0, 1)]["WL"];
                    itdd++;

                    continue;
                }
                else if (mode == "EWL" && itdd->rwl != 0)
                {
                    itdd->rewl = itdd->rwl + (-upd_ewl1 + upd_ewl2);
                    itdd->sd_rewl_cor = (-upd_ewl1 + upd_ewl2);
                    itdd->factor = _sys_wavelen[sat1.substr(0, 1)]["EWL"];
                    itdd++;

                    continue;
                }
                else if (mode == "EWL24" && itdd->rwl != 0)
                {
                    itdd->rewl24 = itdd->rwl + (-upd_ewl1 + upd_ewl2);
                    itdd->sd_rewl24_cor = (-upd_ewl1 + upd_ewl2);
                    itdd->factor = _sys_wavelen[sat1.substr(0, 1)]["EWL24"];
                    itdd++;

                    continue;
                }
                else if (mode == "EWL25" && itdd->rwl != 0)
                {
                    itdd->rewl25 = itdd->rwl + (-upd_ewl1 + upd_ewl2);
                    itdd->sd_rewl25_cor = (-upd_ewl1 + upd_ewl2);
                    itdd->factor = _sys_wavelen[sat1.substr(0, 1)]["EWL25"];
                    itdd++;

                    continue;
                }
                else
                {
                    itdd++;
                    continue;
                }
            }
            else
            { 
                if (mode == "WL" && itdd->rwl != 0)
                {
                    itdd->rwl += (-upd_wl1 + upd_wl2);
                    itdd->sd_rwl_cor = (-upd_wl1 + upd_wl2);
                    itdd->factor = _sys_wavelen[sat1]["WL"];
                    itdd++;
                    continue;
                }
                else
                {
                    itdd++;
                    continue;
                }
            }
        }
        return true;
    }

    bool t_gambiguity::_fixAmbIF()
    {
        string sat1, sat2;
        t_gbdeci bdeci;
        double prob, alpha, alpha1, alpha2;

        for (auto itdd = _DD.begin(); itdd != _DD.end();)
        {
            sat1 = get<0>(itdd->ddSats[0]);
            sat2 = get<0>(itdd->ddSats[1]);

            // Judge Widelane Ambiguity whether can be fixed
            if (sat1.substr(0, 1) != "R") //
            {
                bdeci.bdeci(itdd->rwl, itdd->srwl, 1, _map_WL_decision["maxdev"],
                            _map_WL_decision["maxsig"], prob, alpha);
                if (alpha > _map_WL_decision["alpha"])
                    itdd->isWlFixed = true;
                if (_part_fix && abs(itdd->rwl - round(itdd->rwl)) < 0.2 && itdd->srwl < 0.5)
                {
                    itdd->isWlFixed = true;
                }
            }
            else
            {
                bdeci.bdeci(itdd->rwl_R1, itdd->srwl_R1, 1, _map_WL_decision["maxdev"],
                            _map_WL_decision["maxsig"], prob, alpha1);
                bdeci.bdeci(itdd->rwl_R2, itdd->srwl_R2, 1, _map_WL_decision["maxdev"],
                            _map_WL_decision["maxsig"], prob, alpha2);
                if (alpha1 > _map_WL_decision["alpha"] && alpha2 > _map_WL_decision["alpha"])
                {
                    itdd->isWlFixed = true;
                }
                if (_part_fix && abs(itdd->rwl_R1 - round(itdd->rwl_R1)) < 0.25 && abs(itdd->rwl_R2 - round(itdd->rwl_R2)) < 0.25)
                {
                    itdd->isWlFixed = true;
                }
            }

            // Judge Narrowlane Ambiguity whether can be fixed
            if (double_eq(itdd->srnl, 0.0))
                itdd->srnl = 0.05;
            bdeci.bdeci(itdd->rnl, itdd->srnl, 1, _map_NL_decision["maxdev"],
                        _map_NL_decision["maxsig"], prob, alpha);
            if (alpha > _map_NL_decision["alpha"])
                itdd->isNlFixed = true;

            if (sat1.substr(0, 1) != "R")
            {
                if (_part_fix && abs(itdd->rnl - round(itdd->rnl)) < 0.2 && _MW[sat1][5] > 25.0 && _MW[sat2][5] > 25.0)
                {
                    itdd->isNlFixed = true;
                }
            }
            else
            {
                if (_part_fix && abs(itdd->rnl - round(itdd->rnl)) < 0.25 && _MW[sat1][5] > 20.0 && _MW[sat2][5] > 20.0)
                {
                    itdd->isNlFixed = true;
                }
            }
            itdd++;
        }

        return true;
    }

    bool t_gambiguity::_fixAmbUDUC()
    {
        string sat1, sat2;

        for (auto itdd = _DD.begin(); itdd != _DD.end(); itdd++)
        {
            sat1 = get<0>(itdd->ddSats[0]);
            sat2 = get<0>(itdd->ddSats[1]);

            itdd->isEwlFixed = _EWL_flag[sat1][sat2][itdd->site];
            itdd->isEwl24Fixed = _EWL24_flag[sat1][sat2];
            itdd->isEwl25Fixed = _EWL25_flag[sat1][sat2];
            itdd->isWlFixed = _WL_flag[sat1][sat2][itdd->site];

            if (_amb_freqs[sat1][itdd->site].size() == 1 || _amb_freqs[sat2][itdd->site].size() == 1)
                itdd->isSngleFreq = true;

            if (_obstype == gnut::OBSCOMBIN::RAW_MIX && itdd->isSngleFreq == true)
            {
                if (itdd->ambtype == "AMB_L5" || itdd->ambtype == "AMB_L4" || itdd->ambtype == "AMB_L3")
                    continue;
            }
            else
            {
                if (itdd->ambtype == "AMB_L5" || itdd->ambtype == "AMB_L4" || itdd->ambtype == "AMB_L3" || itdd->ambtype == "AMB_L2")
                    continue;
            }

            // Judge Narrowlane Ambiguity whether can be fixed
            if (double_eq(itdd->srnl, 0.0))
                itdd->srnl = 0.05;

            if (abs(itdd->rnl - round(itdd->rnl)) < _map_NL_decision["maxdev"])
            {
                itdd->isNlFixed = true;
            }

            if (_fix_epo_num["NL"][sat1] > 20 && _fix_epo_num["NL"][sat2] > 20 && _lock_epo_num[sat1] > 200 && _lock_epo_num[sat2] > 200)
            {
                if (abs(itdd->rnl - round(itdd->rnl)) < _map_NL_decision["maxdev"] * 1.1)
                {
                    itdd->isNlFixed = true;
                }
            }
        }

        if (_DD.empty())
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR[t_gambiguity::_fixAmbUDUC] : Double-Difference ambiguity is empty");
            return false;
        }
        else
            return true;
    }
    bool t_gambiguity::_fixAmbWL()
    {
        string sat1, sat2;
        t_gbdeci bdeci;
        double prob, alpha;

        for (auto itdd = _DD.begin(); itdd != _DD.end(); itdd++)
        {
            sat1 = get<0>(itdd->ddSats[0]);
            sat2 = get<0>(itdd->ddSats[1]);

            bdeci.bdeci(itdd->rwl, itdd->srwl, 1, _map_WL_decision["maxdev"],
                        _map_WL_decision["maxsig"], prob, alpha);
            if (alpha > _map_WL_decision["alpha"])
            {
                itdd->isWlFixed = true;
                itdd->isNlFixed = true;
            }

            if (_part_fix && abs(itdd->rwl - round(itdd->rwl)) < 0.25)
            {
                itdd->isWlFixed = true;
                itdd->isNlFixed = true;
            }
        }
        return true;
    }

    bool t_gambiguity::_fixAmbWL(string mode)
    {
        string sat1, sat2;

        for (auto itdd = _DD.begin(); itdd != _DD.end(); itdd++)
        {

            sat1 = get<0>(itdd->ddSats[0]);
            sat2 = get<0>(itdd->ddSats[1]);

            if (mode == "WL")
            {
                if (itdd->rwl == 0)
                    continue;
                _WL_flag[sat1][sat2][itdd->site] = false;
                if (double_eq(itdd->srwl, 0.0))
                    itdd->srwl = 0.05;
                if (abs(itdd->rwl - round(itdd->rwl)) < _map_WL_decision["maxdev"])
                {
                    itdd->isWlFixed = true;
                    _WL_flag[sat1][sat2][itdd->site] = true;
                    itdd->iwl = round(itdd->rwl);
                }
                if (_fix_epo_num[mode][sat1] > 20 && _fix_epo_num[mode][sat2] > 20 && _lock_epo_num[sat1] > 200 && _lock_epo_num[sat2] > 200)
                {
                    if (abs(itdd->rwl - round(itdd->rwl)) < _map_WL_decision["maxdev"] * 1.1)
                    {
                        itdd->isWlFixed = true;
                        _WL_flag[sat1][sat2][itdd->site] = true;
                        itdd->iwl = round(itdd->rwl);
                    }
                }
            }
            else if (mode == "EWL")
            {
                if (itdd->rewl == 0)
                    continue;
                _EWL_flag[sat1][sat2][itdd->site] = false;
                if (double_eq(itdd->srewl, 0.0))
                    itdd->srewl = 0.05;
                if (abs(itdd->rewl - round(itdd->rewl)) < _map_EWL_decision["maxdev"])
                {
                    itdd->isEwlFixed = true;
                    _EWL_flag[sat1][sat2][itdd->site] = true;
                    itdd->iewl = round(itdd->rewl);
                }

                if (_fix_epo_num[mode][sat1] > 20 && _fix_epo_num[mode][sat2] > 20 && _lock_epo_num[sat1] > 200 && _lock_epo_num[sat2] > 200)
                {
                    if (abs(itdd->rewl - round(itdd->rewl)) < _map_EWL_decision["maxdev"] * 1.1)
                    {
                        itdd->isEwlFixed = true;
                        _EWL_flag[sat1][sat2][itdd->site] = true;
                        itdd->iewl = round(itdd->rewl);
                    }
                }
            }
            else if (mode == "EWL24")
            {
                if (itdd->rewl24 == 0)
                    continue;
                _EWL24_flag[sat1][sat2] = false;
                if (double_eq(itdd->srewl24, 0.0))
                    itdd->srewl24 = 0.05;
                if (abs(itdd->rewl24 - round(itdd->rewl24)) < _map_EWL_decision["maxdev"])
                {
                    itdd->isEwl24Fixed = true;
                    _EWL24_flag[sat1][sat2] = true;
                    itdd->iewl24 = round(itdd->rewl24);
                }
            }

            else if (mode == "EWL25")
            {
                if (itdd->rewl25 == 0)
                    continue;
                _EWL25_flag[sat1][sat2] = false;
                if (double_eq(itdd->srewl25, 0.0))
                    itdd->srewl25 = 0.05;
                if (abs(itdd->rewl25 - round(itdd->rewl25)) < _map_EWL_decision["maxdev"])
                {
                    itdd->isEwl25Fixed = true;
                    _EWL25_flag[sat1][sat2] = true;
                    itdd->iewl25 = round(itdd->rewl25);
                }
            }
        }
        return true;
    }

    int t_gambiguity::_selectAmb(int korder, int namb)
    {
        int ndef = 0;
        bool is_depend;
        // Sort DD form big to small according to the duration
        sort(_DD.begin(), _DD.end(), _ddCompare);

        /* define independant dd - ambiguity, first dd - ambiguity with widelane and narrow - lane
        near integer, then with widelane with integer, afterwards the others.The last one
        is only to show how many independant dd - ambiguities we have.*/
        for (auto itdd = _DD.begin(); itdd != _DD.end();)
        {
            if (korder == 1 && !itdd->isNlFixed)
            {
                itdd = _DD.erase(itdd);
                continue;
            }
            if (korder == 2 && !itdd->isWlFixed)
            {
                itdd = _DD.erase(itdd);
                continue;
            }
            if (korder == 3 && !itdd->isEwlFixed)
            {
                itdd = _DD.erase(itdd);
                continue;
            }
            if (korder == 4 && !itdd->isEwl24Fixed)
            {
                itdd = _DD.erase(itdd);
                continue;
            }
            if (korder == 5 && !itdd->isEwl25Fixed)
            {
                itdd = _DD.erase(itdd);
                continue;
            }

            int ipt2ow[2];
            ipt2ow[0] = get<2>(itdd->ddSats[0]);
            ipt2ow[1] = get<2>(itdd->ddSats[1]);

            is_depend = _checkAmbDepend(_is_first, namb, &ndef, 2, ipt2ow, 0, 0);

            if (is_depend)
            {
                auto sat1 = get<0>(itdd->ddSats[0]);
                auto sat2 = get<0>(itdd->ddSats[1]);
                switch (korder)
                {
                case 2:
                    _WL_flag[sat1][sat2][itdd->site] = false;
                    break;
                case 3:
                    _EWL_flag[sat1][sat2][itdd->site] = false;
                    break;
                case 4:
                    _EWL24_flag[sat1][sat2] = false;
                    break;
                case 5:
                    _EWL25_flag[sat1][sat2] = false;
                    break;
                default:
                    break;
                }
                itdd = _DD.erase(itdd);
                continue;
            }
            else
            {
                itdd++;
            }
        }

        if (ndef > 0 && ndef <= 999999)
            return ndef;
        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR[t_gambiguity::_selectAmb] : _selectAmb Wrong");
            return -1;
        }
    }

    bool t_gambiguity::_prepareCovariance(t_gamb_cmn *amb_cmn, SymmetricMatrix &covariance, vector<double> &value)
    {
        int row = 0, col = 0;
        // set covariance-matrix
        covariance.resize(_DD.size());
        covariance = 0;
        for (auto itdd1 = _DD.begin(); itdd1 != _DD.end(); itdd1++)
        {
            // Row of covariance-matrix
            value.push_back(itdd1->rnl);
            row = distance(_DD.begin(), itdd1) + 1;

            for (auto itdd2 = itdd1; itdd2 != _DD.end(); itdd2++)
            {

                // Column of covariance-matrix
                col = distance(_DD.begin(), itdd2) + 1;

                Matrix Q(2, 2);
                // Covariance of ambiguity between four satellites
                Q(1, 1) = amb_cmn->Qx()(get<1>(itdd1->ddSats[0]), get<1>(itdd2->ddSats[0]));
                Q(1, 2) = amb_cmn->Qx()(get<1>(itdd1->ddSats[0]), get<1>(itdd2->ddSats[1]));
                Q(2, 1) = amb_cmn->Qx()(get<1>(itdd1->ddSats[1]), get<1>(itdd2->ddSats[0]));
                Q(2, 2) = amb_cmn->Qx()(get<1>(itdd1->ddSats[1]), get<1>(itdd2->ddSats[1]));

                // Combinatorial transformation
                covariance(row, col) = (Q(1, 1) - Q(2, 1) - Q(1, 2) + Q(2, 2)) / itdd1->factor / itdd2->factor; // unit [cycle]
            }
        }
        // Unit weight

        if (value.size() == 0 || covariance.size() == 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR[t_gambiguity::_prepareCovariance] : prepare Double-Difference covariance is Wrong");
            return false;
        }
        else
            return true;
    }

    bool t_gambiguity::_prepareCovarianceWL(t_gamb_cmn *amb_cmn, SymmetricMatrix &covariance, vector<double> &value, string mode)
    {
        int row = 0, col = 0;
        double lambda_1 = 0.0, lambda_2 = 0.0;
        ColumnVector op_dd(4);
        // set covariance-matrix
        covariance.resize(_DD.size());
        covariance = 0;

        for (auto itdd1 = _DD.begin(); itdd1 != _DD.end(); itdd1++)
        {
            string sat = get<0>(itdd1->ddSats[0]);
            if (mode == "WL")
            {
                if (itdd1->rwl == 0)
                    continue;
                if (sat.substr(0, 1) != "R")
                {
                    lambda_1 = _sys_wavelen[sat.substr(0, 1)]["L1"];
                    lambda_2 = _sys_wavelen[sat.substr(0, 1)]["L2"];
                }
                else
                {
                    lambda_1 = _sys_wavelen[sat]["L1"];
                    lambda_2 = _sys_wavelen[sat]["L2"];
                }

                value.push_back(itdd1->rwl);
            }
            else if (mode == "EWL")
            {
                if (itdd1->rewl == 0)
                    continue;
                if (sat.substr(0, 1) != "R")
                {
                    lambda_1 = _sys_wavelen[sat.substr(0, 1)]["L2"];
                    lambda_2 = _sys_wavelen[sat.substr(0, 1)]["L3"];
                }
                else
                {
                    lambda_1 = _sys_wavelen[sat]["L2"];
                    lambda_2 = _sys_wavelen[sat]["L3"];
                }

                value.push_back(itdd1->rewl);
            }

            // Row of covariance-matrix

            row = distance(_DD.begin(), itdd1) + 1;

            for (auto itdd2 = itdd1; itdd2 != _DD.end(); itdd2++)
            {
                if (get<0>(itdd1->ddSats[0]).substr(1) != get<0>(itdd2->ddSats[0]).substr(1))
                    continue;
                // Column of covariance-matrix
                col = distance(_DD.begin(), itdd2) + 1;
                op_dd << 1 / (lambda_1 * lambda_1) << -1 / (lambda_1 * lambda_2) << -1 / (lambda_1 * lambda_2) << 1 / (lambda_2 * lambda_2);
                Matrix Q(2, 2);

                // Covariance of ambiguity between four satellites
                Q(1, 1) = amb_cmn->Qx()(get<1>(itdd1->ddSats[0]), get<1>(itdd2->ddSats[0])) * op_dd(1) + amb_cmn->Qx()(get<1>(itdd1->ddSats[0]), get<1>(itdd2->ddSats[2])) * op_dd(2) + amb_cmn->Qx()(get<1>(itdd1->ddSats[2]), get<1>(itdd2->ddSats[0])) * op_dd(3) + amb_cmn->Qx()(get<1>(itdd1->ddSats[2]), get<1>(itdd2->ddSats[2])) * op_dd(4);
                Q(1, 2) = amb_cmn->Qx()(get<1>(itdd1->ddSats[0]), get<1>(itdd2->ddSats[1])) * op_dd(1) + amb_cmn->Qx()(get<1>(itdd1->ddSats[0]), get<1>(itdd2->ddSats[3])) * op_dd(2) + amb_cmn->Qx()(get<1>(itdd1->ddSats[2]), get<1>(itdd2->ddSats[1])) * op_dd(3) + amb_cmn->Qx()(get<1>(itdd1->ddSats[2]), get<1>(itdd2->ddSats[3])) * op_dd(4);
                Q(2, 1) = amb_cmn->Qx()(get<1>(itdd1->ddSats[1]), get<1>(itdd2->ddSats[0])) * op_dd(1) + amb_cmn->Qx()(get<1>(itdd1->ddSats[1]), get<1>(itdd2->ddSats[2])) * op_dd(2) + amb_cmn->Qx()(get<1>(itdd1->ddSats[3]), get<1>(itdd2->ddSats[0])) * op_dd(3) + amb_cmn->Qx()(get<1>(itdd1->ddSats[3]), get<1>(itdd2->ddSats[2])) * op_dd(4);
                Q(2, 2) = amb_cmn->Qx()(get<1>(itdd1->ddSats[1]), get<1>(itdd2->ddSats[1])) * op_dd(1) + amb_cmn->Qx()(get<1>(itdd1->ddSats[1]), get<1>(itdd2->ddSats[3])) * op_dd(2) + amb_cmn->Qx()(get<1>(itdd1->ddSats[3]), get<1>(itdd2->ddSats[1])) * op_dd(3) + amb_cmn->Qx()(get<1>(itdd1->ddSats[3]), get<1>(itdd2->ddSats[3])) * op_dd(4);

                // Combinatorial transformation
                covariance(row, col) = (Q(1, 1) - Q(2, 1) - Q(1, 2) + Q(2, 2)); // unit [cycle]
            }
        }
        // Unit weight
        covariance = covariance * pow(amb_cmn->sigma0(), 2);

        if (value.size() == 0 || covariance.size() == 0)
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "ERROR[t_gambiguity::_prepareCovariance] : prepare Double-Difference covariance is Wrong");
            return false;
        }
        else
            return true;
    }

    double t_gambiguity::_lambdaSearch(const Matrix &anor, const vector<double> &fltpar, vector<int> &ibias, double *boot)
    {
        const int maxcan = 2;
        int namb = fltpar.size();
        int ncan = 0, ipos = 0;
        double ratio = 0.0;
        int i, j;
        double disall[maxcan] = {0};
        double *fbias = new double[namb]{0};
        double *Q = new double[namb * namb]{0};
        double *cands = new double[namb * maxcan]{0};
        t_glambda *lambda = new t_glambda();

        try
        {
            //row
            for (i = 0; i < namb; i++)
            {
                ibias.push_back(round(fltpar[i]));
                fbias[i] = fltpar[i] - ibias[i];

                //col
                for (j = 0; j < namb; j++)
                {
                    Q[i * namb + j] = anor(i + 1, j + 1);
                    if (i < j)
                        Q[i * namb + j] = 0.0;
                }
            }

            for (i = 0; i < maxcan; i++)
            {
                disall[i] = 0.0;
            }
            lambda->LAMBDA4(maxcan, namb, Q, fbias, &ncan, &ipos, cands, disall, boot);

            if (double_eq(disall[1], 0.0))
            {
                ratio = disall[0];
            }
            else
            {
                ratio = disall[1] / disall[0];
            }

            if (!double_eq(ratio, 0.0) && _part_fix && lambda->pDia != NULL)
            {
                _mDia.ReSize(namb);
                _mDia = 0;
                for (i = 0; i < namb; i++)
                {
                    _mDia(i + 1) = lambda->pDia[i];
                }
            }

            if (ratio < _ratio || *boot < _boot)
                ibias.clear();
            else
            {
                for (i = 0; i < namb; i++)
                {
                    ibias[i] += round(cands[i * maxcan + 0]);
                }
            }
            delete[] fbias;
            fbias = NULL;
            delete[] Q;
            Q = NULL;
            delete[] cands;
            cands = NULL;
            delete lambda;
            lambda = NULL;

            return ratio;
        }
        catch (...)
        {

            if (fbias != NULL)
            {
                delete[] fbias;
                fbias = NULL;
            }
            if (Q != NULL)
            {
                delete[] Q;
                Q = NULL;
            }
            if (cands != NULL)
            {
                delete[] cands;
                cands = NULL;
            }
            if (lambda != NULL)
            {
                delete lambda;
                lambda = NULL;
            }

            ibias.clear();
            return 0.0;
        }
    }

    bool t_gambiguity::_ambSolve(t_gamb_cmn *amb_cmn, vector<int> &fixed_amb, string mode)
    {
        SymmetricMatrix covariance;
        vector<double> value;
        double ratio = 0.0;
        double boot = 0.0;
        int index;
        if (_DD.size() < _full_fix_num)
        {
            for (auto itdd = _DD.begin(); itdd != _DD.end(); itdd++)
            {
                string sat1 = get<0>(itdd->ddSats[0]);
                string sat2 = get<0>(itdd->ddSats[1]);
                if (mode == "WL" && _WL_flag[sat1][sat2][itdd->site])
                    _IWL[sat1][sat2][itdd->site] = itdd->iwl;
                if (mode == "EWL" && _EWL_flag[sat1][sat2][itdd->site])
                    _IEWL[sat1][sat2][itdd->site] = itdd->iewl;
                if (mode == "EWL24" && _EWL24_flag[sat1][sat2])
                    _IEWL24[sat1][sat2] = itdd->iewl24;
                if (mode == "EWL25" && _EWL25_flag[sat1][sat2])
                    _IEWL25[sat1][sat2] = itdd->iewl25;
                continue;
            }
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_ambSolve] : too few candidate ambiguities for LAMBDA Search");
            return false;
        }

        // get covariance-matrix for SD- or DD-ambiguities
        if (mode == "NL")
        {
            if (!_prepareCovariance(amb_cmn, covariance, value))
                return false;
        }
        else
        {
            if (!_prepareCovarianceWL(amb_cmn, covariance, value, mode))
                return false;
        }

        // resolve integer ambiguities using LAMBDA-method
        ratio = _lambdaSearch(covariance, value, fixed_amb, &boot);
        if (ratio > 99.0)
            ratio = 99.0;
        if (mode == "NL")
        {
            _os_ratio << "RATIO" << fixed << setw(10) << setprecision(2) << ratio;
            _os_boot << "BOOTSTRAPPING" << fixed << setw(10) << setprecision(2) << boot;
        }

        // part amb. fix
        int newamb = 0;
        while ((ratio < _ratio || (boot < _boot && mode == "NL")) && _part_fix)
        {
            double max_diag = 0.0;
            index = 1;

            if (newamb < 2)
            {
                for (unsigned int i = 1; i <= value.size(); i++)
                {
                    auto itdd = _DD.begin() + i - 1;
                    if (_fix_epo_num[mode][get<0>(itdd->ddSats[0])] == 0 || _fix_epo_num[mode][get<0>(itdd->ddSats[1])] == 0 || _lock_epo_num[get<0>(itdd->ddSats[0])] < 5 || _lock_epo_num[get<0>(itdd->ddSats[1])] < 5)
                    {
                        if (covariance(i, i) > max_diag)
                        {
                            max_diag = covariance(i, i);
                            index = i;
                        }
                    }
                }
                newamb++;
            }

            if (max_diag == 0.0)
            {
                for (unsigned int i = 1; i <= value.size(); i++)
                {
                    if (covariance(i, i) > max_diag)
                    {
                        max_diag = covariance(i, i);
                        index = i;
                    }
                }
            }
            max_diag = 0.0;
            for (unsigned int i = 1; i <= value.size(); i++)
            {
                auto itdd = _DD.begin() + i - 1;
                string sat1, sat2;
                sat1 = get<0>(itdd->ddSats[0]);
                sat2 = get<0>(itdd->ddSats[1]);
                if (sat1.substr(0, 1) == "R" && sat2.substr(0, 1) == "R")
                {
                    if (covariance(i, i) > max_diag)
                    {
                        max_diag = covariance(i, i);
                        index = i;
                    }
                }
            }
            
            auto itdd_tmp = _DD.begin() + index - 1;

            if (mode == "WL")
            {
                _WL_flag[get<0>(itdd_tmp->ddSats[0])][get<0>(itdd_tmp->ddSats[1])][itdd_tmp->site] = false;
            }
            else if (mode == "EWL")
            {
                _EWL_flag[get<0>(itdd_tmp->ddSats[0])][get<0>(itdd_tmp->ddSats[1])][itdd_tmp->site] = false;
            }
            Matrix_remRC(covariance, index, index);
            value.erase(value.begin() + index - 1);
            _DD.erase(_DD.begin() + index - 1);

            fixed_amb.clear();
            if (value.size() <= _part_fix_num)
                return false;

            ratio = _lambdaSearch(covariance, value, fixed_amb, &boot);

            if (ratio > 99.0)
                ratio = 99.0;
            if (mode == "NL")
            {
                _os_ratio << setw(10) << setprecision(2) << ratio;
                _os_boot << setw(10) << setprecision(2) << boot;
            }
        }
        if (mode == "NL" && ratio > _ratio && boot > _boot)
        {
            _os_ratio << endl;
            _os_boot << endl;
            _writeRatio(ratio);
            if (_boot > 0.0)
                _writeBoot(boot);
        }

        amb_cmn->set_ratio(ratio);
        amb_cmn->set_boot(boot);

        // if nl amb. fixed successfully , replace inl with fixed_nlamb
        if (mode == "WL")
        {
            if (fixed_amb.size() != 0)
            {
                for (auto it_dd = _DD.begin(); it_dd != _DD.end(); it_dd++)
                {
                    if (it_dd->rwl == 0)
                        continue;
                    int ipos = distance(_DD.begin(), it_dd);
                    it_dd->iwl = fixed_amb[ipos];
                }
                return true;
            }

            else
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_ambSolve] : LAMBDA Search can't get candidate fixed ambiguities");
                return false;
            }
        }
        else if (mode == "EWL")
        {
            if (fixed_amb.size() != 0)
            {
                for (auto it_dd = _DD.begin(); it_dd != _DD.end(); it_dd++)
                {
                    if (it_dd->rewl == 0)
                        continue;
                    int ipos = distance(_DD.begin(), it_dd);
                    it_dd->iewl = fixed_amb[ipos];
                }
                return true;
            }
            else
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_ambSolve] : LAMBDA Search can't get candidate fixed ambiguities");
                return false;
            }
        }
        else if (mode == "NL")
        {
            if (fixed_amb.size() != 0)
            {
                for (auto it_dd = _DD.begin(); it_dd != _DD.end(); it_dd++)
                {
                    int ipos = distance(_DD.begin(), it_dd);
                    it_dd->inl = fixed_amb[ipos];
                }
                return true;
            }
            else
            {
                if (_spdlog)
                    SPDLOG_LOGGER_DEBUG(_spdlog, "Warning[t_gambiguity::_ambSolve] : LAMBDA Search can't get candidate fixed ambiguities");
                return false;
            }
        }

        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, "Error[t_gambiguity::_ambSolve] : Unknown Combination[NL/WL/EWL] : " + mode);
            return false;
        }
    }

    bool t_gambiguity::_addFixConstraint(t_gflt *gflt)
    {
        //////========================= Virtual observation equation ===========================================
        double dl = 0, flt= 0, integer= 0, Ba = 1, Bb = -1;
        double p0 = 1E9;
        for (auto itdd = _DD.begin(); itdd != _DD.end(); itdd++)
        {
            bool wl_fix = ((_obstype == gnut::OBSCOMBIN::RAW_MIX && itdd->isSngleFreq != true) || _obstype != gnut::OBSCOMBIN::RAW_MIX);
            if ((!itdd->isWlFixed && wl_fix) || !itdd->isNlFixed)
                continue;
            int index_sat1 = get<1>(itdd->ddSats[0]);
            int index_sat2 = get<1>(itdd->ddSats[1]);

            flt = gflt->param()[index_sat1 - 1].value() - gflt->param()[index_sat2 - 1].value();

            if (_obstype == OBSCOMBIN::IONO_FREE)
            {
                string sys = get<0>(itdd->ddSats[0]).substr(0, 1);
                if (sys != "R")
                {
                    integer = (itdd->inl - itdd->sd_rnl_cor + round(itdd->rwl) * _sys_wavelen[sys]["WL"] / _sys_wavelen[sys]["L2"]) * itdd->factor;
                }
                else
                {
                    string sat1 = get<0>(itdd->ddSats[0]);
                    string sat2 = get<0>(itdd->ddSats[1]);
                    integer = (itdd->inl - itdd->sd_rnl_cor + 3.5 * round(itdd->rwl)); 
                    Ba = 1 / _sys_wavelen[sat1]["NL"];                                 
                    Bb = -1 / _sys_wavelen[sat2]["NL"];
                    flt = gflt->param()[index_sat1 - 1].value() * Ba + gflt->param()[index_sat2 - 1].value() * Bb;
                }
            }
            else if (_obstype == OBSCOMBIN::RAW_ALL || _obstype == OBSCOMBIN::RAW_MIX)
            {
                double Lx_cor = 0.0;
                string sys = get<0>(itdd->ddSats[0]).substr(0, 1);
                if (sys != "R")
                {
                    if (itdd->ambtype == "AMB_L1")
                        Lx_cor = itdd->sd_r1_cor;
                    else if (itdd->ambtype == "AMB_L2")
                        Lx_cor = itdd->sd_r2_cor;
                    else if (itdd->ambtype == "AMB_L3")
                        Lx_cor = itdd->sd_r3_cor;
                    else if (itdd->ambtype == "AMB_L4")
                        Lx_cor = itdd->sd_r4_cor;
                    else if (itdd->ambtype == "AMB_L5")
                        Lx_cor = itdd->sd_r5_cor;
                    integer = (itdd->inl - Lx_cor) * itdd->factor;
                }
                else
                {
                    string sat1 = get<0>(itdd->ddSats[0]);
                    string sat2 = get<0>(itdd->ddSats[1]);
                    if (itdd->ambtype == "AMB_L1")
                    {
                        Ba = 1 / _sys_wavelen[sat1]["L1"]; 
                        Bb = -1 / _sys_wavelen[sat2]["L1"];
                        Lx_cor = itdd->sd_r1_cor;
                    }
                    else if (itdd->ambtype == "AMB_L2")
                    {
                        Ba = 1 / _sys_wavelen[sat1]["L2"]; 
                        Bb = -1 / _sys_wavelen[sat2]["L2"];
                        Lx_cor = itdd->sd_r2_cor;
                    }
                    flt = gflt->param()[index_sat1 - 1].value() * Ba + gflt->param()[index_sat2 - 1].value() * Bb;
                    integer = (itdd->inl - Lx_cor);
                }
            }

            dl = integer - flt;

            vector<pair<int, double>> B;
            B.push_back(make_pair(index_sat1, Ba));
            B.push_back(make_pair(index_sat2, Bb));

            Matrix B_mat;
            SymmetricMatrix P_mat;
            ColumnVector l_mat;
            t_gfltEquationMatrix virtual_equ;
            t_gobscombtype type;
            virtual_equ.add_equ(B, p0, dl, _site, get<0>(itdd->ddSats[0]) + "_" + get<0>(itdd->ddSats[1]), type, false);
            virtual_equ.chageNewMat(B_mat, P_mat, l_mat, gflt->npar_number());

            gflt->resetQ();
            gflt->add_virtual_obs(B_mat, P_mat, l_mat);
        }

        try
        {
            gflt->update();
        }
        catch (exception e)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, e.what(), "Ambiguity Constrain Failed!");
            return false;
        }

        return true;
        //////========================= Virtual observation equation ===========================================
    }



    bool t_gambiguity::_addFixConstraintWL(t_gflt *gflt, string mode)
    {
        //////========================= Virtual observation equation ===========================================
        double dl = 0.0;
        double integer= 0.0;
        double flt= 0.0;
        double lambda_1= 0.0;
        double lambda_2= 0.0;
        double p0 = 1E6;
        double lambda_11= 0.0;
        double lambda_21= 0.0;
        double lambda_12= 0.0;
        double lambda_22= 0.0; // the wave_length for glonass
        double Ba= 0.0;
        double Bb= 0.0;
        double Bc= 0.0;
        double Bd= 0.0;       // the Coefficient of B matrix
        SymmetricMatrix _Qx_tmp;
        t_gflt flttmp(*gflt);
        _Qx_tmp = gflt->Qx();
        if (mode == "EWL" || mode == "EWL24" || mode == "EWL25")
            p0 = 1E4;
        for (auto itdd = _DD.begin(); itdd != _DD.end(); itdd++)
        {

            int index_sat1 = get<1>(itdd->ddSats[0]);
            int index_sat2 = get<1>(itdd->ddSats[1]);
            int index_sat3 = get<1>(itdd->ddSats[2]);
            int index_sat4 = get<1>(itdd->ddSats[3]);
            string sat1 = get<0>(itdd->ddSats[0]);
            string sat2 = get<0>(itdd->ddSats[1]);
            string sat3 = get<0>(itdd->ddSats[2]);
            string sat4 = get<0>(itdd->ddSats[3]);

            if (sat1.substr(0, 1) != "R")
            {
                if (mode == "WL")
                {
                    if (!itdd->isWlFixed || itdd->rwl == 0)
                        continue;
                    lambda_1 = _sys_wavelen[sat1.substr(0, 1)]["L1"];
                    lambda_2 = _sys_wavelen[sat1.substr(0, 1)]["L2"];
                    integer = (itdd->iwl - itdd->sd_rwl_cor);
                }
                else if (mode == "EWL")
                {
                    if (!itdd->isEwlFixed || itdd->rewl == 0)
                        continue;
                    lambda_1 = _sys_wavelen[sat1.substr(0, 1)]["L2"];
                    lambda_2 = _sys_wavelen[sat1.substr(0, 1)]["L3"];
                    integer = (itdd->iewl - itdd->sd_rewl_cor);
                }
                else if (mode == "EWL24")
                {
                    if (!itdd->isEwl24Fixed || itdd->rewl24 == 0)
                        continue;
                    lambda_1 = _sys_wavelen[sat1.substr(0, 1)]["L2"];
                    lambda_2 = _sys_wavelen[sat1.substr(0, 1)]["L4"];
                    integer = (itdd->iewl24 - itdd->sd_rewl24_cor);
                }
                else if (mode == "EWL25")
                {
                    if (!itdd->isEwl25Fixed || itdd->rewl25 == 0)
                        continue;
                    lambda_1 = _sys_wavelen[sat1.substr(0, 1)]["L2"];
                    lambda_2 = _sys_wavelen[sat1.substr(0, 1)]["L5"];
                    integer = (itdd->iewl25 - itdd->sd_rewl25_cor);
                }
                Ba = 1 / lambda_1;
                Bb = -1 / lambda_2;
                Bc = -1 / lambda_1;
                Bd = 1 / lambda_2;
            }
            else
            {
                if (mode == "WL")
                {
                    if (!itdd->isWlFixed || itdd->rwl == 0)
                        continue;
                    lambda_11 = _sys_wavelen[sat1]["L1"];
                    lambda_12 = _sys_wavelen[sat3]["L2"];
                    lambda_21 = _sys_wavelen[sat2]["L1"];
                    lambda_22 = _sys_wavelen[sat4]["L2"];

                    integer = (itdd->iwl - itdd->sd_rwl_cor);
                }
                Ba = 1 / lambda_11;
                Bb = -1 / lambda_12;
                Bc = -1 / lambda_21;
                Bd = 1 / lambda_22;
            }

            flt = gflt->param()[index_sat1 - 1].value() * Ba + gflt->param()[index_sat3 - 1].value() * Bb + gflt->param()[index_sat2 - 1].value() * Bc + gflt->param()[index_sat4 - 1].value() * Bd;
            dl = integer - flt;

            vector<pair<int, double>> B;
            B.push_back(make_pair(index_sat1, Ba));
            B.push_back(make_pair(index_sat3, Bb));
            B.push_back(make_pair(index_sat2, Bc));
            B.push_back(make_pair(index_sat4, Bd));

            Matrix B_mat;
            SymmetricMatrix P_mat;
            ColumnVector l_mat;
            t_gfltEquationMatrix virtual_equ;
            t_gobscombtype type;
            virtual_equ.add_equ(B, p0, dl, _site, get<0>(itdd->ddSats[0]) + "_" + get<0>(itdd->ddSats[2]) + "_" + get<0>(itdd->ddSats[1]) + "_" + get<0>(itdd->ddSats[3]), type, false);
            virtual_equ.chageNewMat(B_mat, P_mat, l_mat, gflt->npar_number());

            gflt->resetQ();
            gflt->add_virtual_obs(B_mat, P_mat, l_mat);
        }
        try
        {
            gflt->update();
        }
        catch (exception e)
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, e.what(), "Solve Equation Fail!");
            return false;
        }

        return true;
    }


    void t_gambiguity::_initRatiofile()
    {
        //output ratio   file
        string ratio_path = dynamic_cast<t_gsetout *>(_gset)->outputs("ratio"); //xjhan
        if (ratio_path.empty())
            ratio_path = "ratio-" + _site;

        substitute(ratio_path, "$(rec)", _site, false);

        _ratiofile = new t_giof;
        _ratiofile->tsys(t_gtime::GPS);
        _ratiofile->mask(ratio_path);
        _ratiofile->append(dynamic_cast<t_gsetout *>(_gset)->append());
        ostringstream os;
        os << " Epoch        Ratio" << endl;
        if (_ratiofile)
        {
            _ratiofile->write(os.str().c_str(), os.str().size());
            _ratiofile->flush();
        }
    }

    void t_gambiguity::_initBootfile()
    {
        //output boot   file
        string bootfile = "boot-" + _site;

        _bootfile = new t_giof;
        _bootfile->tsys(t_gtime::GPS);
        _bootfile->mask(bootfile);
        _bootfile->append(dynamic_cast<t_gsetout *>(_gset)->append());
        ostringstream os;
        os << " Epoch        Boot" << endl;
        if (_bootfile)
        {
            _bootfile->write(os.str().c_str(), os.str().size());
            _bootfile->flush();
        }
    }

    void t_gambiguity::_writeRatio(double ratio)
    {
        ostringstream os;
        int nepoch = (int)(((_crt_time.dmjd() - _beg.dmjd()) * 86400.0 + _interval * 1E-3) / _interval) + 1;

        // write ratio data
        os << " " << setw(4) << nepoch << "     " << fixed << setw(5) << setprecision(2) << ratio << endl;

        // Print ratio results
        if (_ratiofile)
        {
            _ratiofile->write(os.str().c_str(), os.str().size());
            _ratiofile->flush();
        }
        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "have no output file!");
        }

        return;
    }

    void t_gambiguity::_writeBoot(double BootStrapping)
    {
        ostringstream os;
        int nepoch = (int)(((_crt_time.dmjd() - _beg.dmjd()) * 86400.0 + _interval * 1E-3) / _interval) + 1;

        // write bootstrapping data
        os << " " << setw(4) << nepoch << "     " << fixed << setw(5) << setprecision(2) << BootStrapping * 100.0 << "%" << endl;

        // Print bootstrapping results
        if (_bootfile)
        {
            _bootfile->write(os.str().c_str(), os.str().size());
            _bootfile->flush();
        }
        else
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "have no output file!");
        }

        return;
    }

    double t_gambiguity::_glonassRecUpd(string site)
    {
        double upd_rec = 0.0;
        set<string> mapSat;
        list<pair<double, double>> wl_sig;
        double sigx = 0.0, mean = 0.0, sig = 0.0, upd = 0.0;
        int npnt = 0;
        string sat1, sat2;
        for (auto itdd = _DD.begin(); itdd != _DD.end(); itdd++)
        {
            sat1 = get<0>(itdd->ddSats[0]);
            sat2 = get<0>(itdd->ddSats[1]);
            if (site != _site)
                continue;
            if (sat1.substr(0, 1) != "R" || sat2.substr(0, 1) != "R")
                continue;
            if (mapSat.find(sat1) == mapSat.end())
            {
                mapSat.insert(sat1);
                if (_getSingleUpd("WL", _wl_Upd_time, sat1, upd, sig))
                {
                    wl_sig.push_back(make_pair(_MW[sat1][2] - upd, 1.0));
                    npnt++;
                }
            }
            if (mapSat.find(sat2) == mapSat.end())
            {
                mapSat.insert(sat2);
                if (_getSingleUpd("WL", _wl_Upd_time, sat2, upd, sig))
                {
                    wl_sig.push_back(make_pair(_MW[sat2][2] - upd, 1.0));
                    npnt++;
                }
            }
        }
        if (npnt > 2)
        {
            getMeanFract(wl_sig, mean, sig, sigx);
            upd_rec = sigx <= 0.10 ? mean : 0.0;
        }
        return upd_rec;
    }

    bool t_gambiguity::_findRefSD()
    {
        if (_sat_refs.empty())
        {
            if (_spdlog)
                SPDLOG_LOGGER_DEBUG(_spdlog, "_sat_refs is empty");
            return false;
        }

        // erase dds do not contain sat_ref
        for (auto it_dd = _DD.begin(); it_dd != _DD.end();)
        {
            string sat1 = get<0>(it_dd->ddSats[0]);
            string sat2 = get<0>(it_dd->ddSats[1]);

            if (_sat_refs.find(sat1) == _sat_refs.end() &&
                _sat_refs.find(sat2) == _sat_refs.end())
            {
                it_dd = _DD.erase(it_dd);
                continue;
            }
            else
                it_dd++;
        }
        return true;
    }

    bool _ddCompare(const t_dd_ambiguity &dd1, const t_dd_ambiguity &dd2)
    {
        return (dd1.end_epo - dd1.beg_epo) > (dd2.end_epo - dd2.beg_epo);
    }

    t_gamb_cmn::t_gamb_cmn()
    {
    }
    t_gamb_cmn::t_gamb_cmn(const t_gtime &t, t_gflt *flt)
    {

        _now = t;
        _param = flt->param();
        _sigma0 = flt->sigma0();
        _vtpv = flt->vtpv();
        _Qx = flt->Qx();
        _dx = flt->dx();
        _stdx = flt->stdx();
        _nobs_total = flt->nobs_total();
        _npar_number = flt->npar_number();
        _amb_fixed = false;
        _active_amb.insert(make_pair("", 1));
        _ratio = 0.0;
    }

    t_gamb_cmn::~t_gamb_cmn()
    {
    }

    void t_gamb_cmn::active_amb(map<string, int> active_amb)
    {
        _active_amb = active_amb;
    }

    int t_gamb_cmn::active_amb(string site)
    {
        return _active_amb[site];
    }
    void t_gamb_cmn::amb_fixed(bool b)
    {
        _amb_fixed = b;
    }

    bool t_gamb_cmn::amb_fixed()
    {
        return _amb_fixed;
    }

    void t_gamb_cmn::set_ratio(double r)
    {
        _ratio = r;
    }

    double t_gamb_cmn::get_ratio()
    {
        return _ratio;
    }

    void t_gamb_cmn::set_boot(double b)
    {
        _boot = b;
    }

    double t_gamb_cmn::get_boot()
    {
        return _boot;
    }

    void t_gamb_cmn::set_mode(string mode)
    {
        _mode = mode;
    }

    string t_gamb_cmn::get_mode()
    {
        return _mode;
    }

    t_gtime t_gamb_cmn::now() const
    {
        return _now;
    }

    double t_gamb_cmn::sigma0() const
    {
        return _sigma0;
    }

    t_gallpar t_gamb_cmn::param() const
    {
        return _param;
    }

    ColumnVector t_gamb_cmn::dx() const
    {
        return _dx;
    }

    ColumnVector t_gamb_cmn::stdx() const
    {
        return _stdx;
    }

    SymmetricMatrix t_gamb_cmn::Qx() const
    {
        return _Qx;
    }

}
