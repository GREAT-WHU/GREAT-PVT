/**
 * @file         gambcommon.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        declare some classes and some mathematical method.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GAMBCOMMON_H
#define GAMBCOMMON_H

#include "gexport/ExportLibGREAT.h"
#include <string>
#include <map>
#include <memory>
#include <tuple>
#include <list>
#include "gutils/gtime.h"
#include "gutils/gtypeconv.h"

using namespace std;
using namespace gnut;

namespace great
{
    /**
    *@ brief zero-difference ambiguity structure  for arc.
    */
    class LibGREAT_LIBRARY_EXPORT t_oneway_ambiguity
    {
    public:
        /** @brief default constructor. */
        explicit t_oneway_ambiguity();

        /** @brief default destructor. */
        virtual ~t_oneway_ambiguity(){};

        string ambtype;     ///< C/1/2/W
        string sat;         ///< satellite name
        int ipt = 0;        ///< index of par
        int beg_epo = 0;    ///< start time
        int end_epo = 0;    ///< t_end time
        double rwl = 0.0;   ///< real value widelane(cyc) 
        double srwl = 0.0;  ///< its sigma
        double rewl = 0.0;  ///< real value extrawidelane(cyc)
        double srewl = 0.0; ///< its sigma
        double rlc = 0.0;   ///< real value lc from slution(m)
        double srlc = 0.0;  ///< its sigma
        double ele = 0.0;   ///< elevation
        double r = 0.0;     ///< ambiguity in UCUD mode
        double sr = 0.0;    ///< sigma of ambiguity
    };

    /**
    * @brief  single-differece ambiguity  for arc.
    */
    class t_dd_ambiguity
    {
    public:
        /** @brief default constructor. */
        t_dd_ambiguity(){};

        /** @brief default destructor. */
        virtual ~t_dd_ambiguity(){};

        string ambtype; ///< C/1/2/W

        bool isEwlFixed = false;   ///< extral widelane fixed
        bool isEwl24Fixed = false; ///< extral frequency24 fixed
        bool isEwl25Fixed = false; ///< extral frequency25 fixed
        bool isWlFixed = false;    ///< widelane fixed
        bool isNlFixed = false;    ///< narrowlane fixed
        string site = "default";
        //map<string, int> ipt2ow;
        vector<tuple<string, int, int>> ddSats; ///< sat_name, index in all_pars, index in amb_pars
        t_gtime beg_epo;                        ///< begin time
        t_gtime end_epo;                        ///< end time
        t_gtime end_epo_save;                   ///< save end epoch time
        double rwl_R1 = 0.0;                    ///< real widelane for glonass
        double rwl_R2 = 0.0;                    ///< real widelane for glonass
        double srwl_R1 = 0.0;                   ///< real widelane sigma for glonass
        double srwl_R2 = 0.0;                   ///< real widelane sigma for glonass
        double rewl = 0.0;                      ///< real Extra widelane and its sigma
        double srewl = 0.0;                     ///< real Extra widelane and its sigma
        double rewl24 = 0.0;                    ///< real Extra widelane and its sigma
        double srewl24 = 0.0;                   ///< real Extra widelane and its sigma
        double rewl25 = 0.0;                    ///< real Extra widelane and its sigma
        double srewl25 = 0.0;                   ///< real Extra widelane and its sigma
        double rwl = 0.0;                       ///< real widelane and its sigma
        double srwl = 0.0;                      ///< real widelane and its sigma
        double rwl_q1 = 0.0;                    ///< real widelane and its q1
        double rwl_q2 = 0.0;                    ///< real widelane and its q2
        double rnl = 0.0;                       ///< real narrowlane and its sigma
        double srnl = 0.0;                      ///< real narrowlane and its sigma
        double rlc = 0.0;                       ///< lc ambiguity and its sigma
        double srlc = 0.0;                      ///< lc ambiguity and its sigma
        int iwl = 0;                            ///< integer wide - and wide lane
        int inl = 0;                            ///< integer wide - and narrow lane
        int iewl = 0;                           ///< integer wide - and extral wide lane
        int iewl24 = 0;                         ///< integer wide - and extral wide lane24
        int iewl25 = 0;                         ///< integer wide - and extral wide lane25
        double factor = 0.0;                    ///< TODO
        double sd_rnl_cor = 0.0;                ///< correction of SD, narrowlane
        double sd_rwl_cor = 0.0;                ///< correction of SD, widelane
        double sd_rewl_cor = 0.0;               ///< correction of SD, extra-widelane
        double sd_rewl24_cor = 0.0;             ///< correction of SD, extra-widelane
        double sd_rewl25_cor = 0.0;             ///< correction of SD, extra-widelane
        double sd_r1_cor = 0.0;                 ///< correction of N1
        double sd_r2_cor = 0.0;                 ///< correction of N2
        double sd_r3_cor = 0.0;                 ///< correction of N3
        double sd_r4_cor = 0.0;                 ///< correction of N4
        double sd_r5_cor = 0.0;                 ///< correction of N5
        double sigcor = 0.0;                    ///<sigma
        int fix_epoch = 1;                      /// fixed epochs of dd ambiguity
        bool isSngleFreq = false;               /// exist single frequency satellites or not
    };

    /**
    * @brief ambiguity info for one site one satellite one epoch  used in nl upd.
    */
    class LibGREAT_LIBRARY_EXPORT epoch_amb
    {
    public:
        /** @brief default constructor. */
        epoch_amb();

        /** @brief default destructor. */
        virtual ~epoch_amb();

        t_gtime epo;           ///< current epoch
        bool isnewarc = false; ///< new arc or not
        int nepo = 0;          ///< number of epoch
        double bc = 0.0;       ///< ambiguity value
        double sbc = 0.0;      ///< IF ambiguity sigma
        double bw = 0.0;       ///< Widelane ambiguitiy value
        double sbw = 0.0;      ///< Widelane ambiguitiy sigma value
        double bw0 = 0.0;      ///< Widelane ambiguitiy init value
        double bn = 0.0;       ///< narrowlane ambiguity
        double elev = 0.0;     ///< elevation
        double bwi = 0.0;      ///< TODO
        double bewi = 0.0;     ///< TODO
    };

    /** @brief ambiguity residual. */
    class LibGREAT_LIBRARY_EXPORT epoch_amb_res
    {
    public:
        /** @brief default constructor. */
        epoch_amb_res();

        /** @brief default destructor. */
        virtual ~epoch_amb_res();
        bool wl_fixed = false;  ///< wide lane fixed
        bool ewl_fixed = false; ///< extral wide lane fixed
        bool nl_fixed = false;  ///< narrow lane fixed
        double nl_res = 0.0;    ///< narrow lane residuals
        double wl_res = 0.0;    ///< wide lane reiduals
    };

    // UPD
    /** @brief map for storaging ambiguity info ,t_epoch_amb[site][res] = epoch_amb. */
    typedef map<string, map<string, epoch_amb>> LibGREAT_LIBRARY_EXPORT t_epoch_amb;

    /** @brief map for storaging ambiguity residual ,t_epoch_amb_res[site][res] = epoch_amb_res. */
    typedef map<string, map<string, epoch_amb_res>> LibGREAT_LIBRARY_EXPORT t_epoch_amb_res;

    // PPP AR/ WL+EWL UPD
    /** @brief map for storaging oneway_ambiguity  */
    typedef vector<shared_ptr<t_oneway_ambiguity>> LibGREAT_LIBRARY_EXPORT t_OW_amb; //satellites OW;

    /** @brief map for storaging oneway_ambiguity , one station/all satellite/all epoch. */
    typedef map<string, t_OW_amb> LibGREAT_LIBRARY_EXPORT t_all_sats_OW; //satellites OW;

    //** @brief vector for storaging double-differece ambiguity. */
    typedef vector<t_dd_ambiguity> LibGREAT_LIBRARY_EXPORT t_DD_amb;

    // WL/EWL UPD
    //** @brief map for storaging ambiguity value, one station/all epo/all satellite. */
    typedef map<int, map<string, double>> LibGREAT_LIBRARY_EXPORT t_epo_sat_amb;

    // MW in PPP
    //** @brief map for storaging ambiguity valid time. */
    typedef map<string, vector<pair<t_gtime, t_gtime>>> LibGREAT_LIBRARY_EXPORT t_map_amb_time;

    //** @brief map for storaging MW obs value, one station/all epo/all satellite/idx=(1,2,3,4,5). */
    typedef map<t_gtime, map<string, map<int, double>>> LibGREAT_LIBRARY_EXPORT t_map_MW;

    /**
    * @brief Compute mean of the fractional ambiguities.
    * @param[in]  values    list pairs contain data and it's weight.
    * @param[in]  mean        data mean.
    * @param[in]  sigma        data sigma.
    * @param[in]  sigx        sigma's sigma.
    * @param[out] mean
    * @param[out] sigma
    * @param[out] sigx
    * @return      void
    */
    LibGREAT_LIBRARY_EXPORT void getMeanFract(list<pair<double, double>> &values, double &mean, double &sigma, double &sigx);

    /**
    * @brief Get mean, sigma and sigma of the mean of a set sampled data.
    * @param[in]  is_edit   whether to eliminate errors.
    * @param[in]  wx         list pairs contain data and it's weight.
    * @param[in]  mean        data mean.
    * @param[in]  sigma        data sigma.
    * @param[in]  sigx        sigma's sigma.
    * @param[out] mean
    * @param[out] sigma
    * @param[out] sigx
    * @return      void
    */
    LibGREAT_LIBRARY_EXPORT void getMeanWgt(bool is_edit, list<pair<double, double>> &wx, double &mean, double &sigma, double &mean_sig);

    /**
    * @brief Adjust ambiguities between -0.5~0.5 cycle.
    * @param[in]  x     real value
    * @param[in]  min   [0,1), [-0,5,0.5), [-1.0,1.0)
    * @return      fraction of ambiguities
    */
    LibGREAT_LIBRARY_EXPORT double getFraction(double x, double min);

}

#endif
