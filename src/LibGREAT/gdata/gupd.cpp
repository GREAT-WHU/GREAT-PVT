/**
 * @file         gupd.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        Storage the upd files' data include wide-lane, narrow-lane and ex-wide-lane
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gdata/gupd.h"

using namespace std;
namespace great
{
    t_updrec::t_updrec()
    {
        obj = " ";
        npoint = 0;
        value = 0.0;
        sigma = 1E4;
        isRef = false;
    }

    t_gupd::t_gupd() : t_gdata()
    {
        id_type(t_gdata::UPD);
        _ewl_flag = t_gtime(EWL_IDENTIFY);
        _ewl24_flag = t_gtime(EWL24_IDENTIFY);
        _ewl25_flag = t_gtime(EWL25_IDENTIFY);
        _wl_flag = t_gtime(WL_IDENTIFY);
        _valid_beg[UPDTYPE::EWL] = FIRST_TIME;
        _valid_beg[UPDTYPE::EWL24] = FIRST_TIME;
        _valid_beg[UPDTYPE::EWL25] = FIRST_TIME;
        _valid_beg[UPDTYPE::WL] = FIRST_TIME;
        _valid_beg[UPDTYPE::NL] = FIRST_TIME;
        _valid_beg[UPDTYPE::IFCB] = FIRST_TIME;
        _tend = FIRST_TIME;
        _est_upd_type = UPDTYPE::NONE;
        _wl_epo_mode = false;
        _wait_stream = true;
    }
    t_gupd::t_gupd(t_spdlog spdlog) : t_gdata(spdlog)
    {
        id_type(t_gdata::UPD);

        _ewl_flag = t_gtime(EWL_IDENTIFY);
        _ewl24_flag = t_gtime(EWL24_IDENTIFY);
        _ewl25_flag = t_gtime(EWL25_IDENTIFY);
        _wl_flag = t_gtime(WL_IDENTIFY);

        _valid_beg[UPDTYPE::EWL] = FIRST_TIME;
        _valid_beg[UPDTYPE::EWL24] = FIRST_TIME;
        _valid_beg[UPDTYPE::EWL25] = FIRST_TIME;
        _valid_beg[UPDTYPE::WL] = FIRST_TIME;
        _valid_beg[UPDTYPE::NL] = FIRST_TIME;
        _valid_beg[UPDTYPE::IFCB] = FIRST_TIME;

        _tend = FIRST_TIME;

        _est_upd_type = UPDTYPE::NONE;
        _wl_epo_mode = false;

        _wait_stream = true;
    }

    t_gupd::~t_gupd()
    {
        _upd.clear();
    }

    void t_gupd::add_sat_upd(UPDTYPE upd_type, t_gtime epoch, string prn, t_updrec one_sat_upd)
    {
        _upd[upd_type][epoch][prn] = make_shared<t_updrec>(one_sat_upd);
        if (upd_type == UPDTYPE::WL && _upd[upd_type].size() > 1)
            wl_epo_mode(true);
    }

    void t_gupd::re_init_upd(const UPDTYPE &upd_type, const t_gtime &t, string str)
    {
        t_updrec one_upd;
        one_upd.npoint = 0;
        one_upd.value = 0.0;
        one_upd.sigma = 1E4;
        one_upd.isRef = false;
        t_gupd::add_sat_upd(upd_type, t, str, one_upd);
    }

    one_epoch_upd &t_gupd::get_epo_upd(const UPDTYPE &upd_type, const t_gtime &t)
    {
        if (_upd[upd_type].find(t) != _upd[upd_type].end())
        {
            return _upd[upd_type][t];
        }
        else
        {
            auto it_epo = _upd[upd_type].lower_bound(t);
            if (it_epo != _upd[upd_type].end())
            {
                if (it_epo->first.diff(t) >= 30)
                {
                    _wait_stream = false;
                    return _null_epoch_upd;
                }
                else
                {
                    return it_epo->second;
                }
            }
            else
            {
                return _null_epoch_upd;
            }
        }
    }

} //namespace