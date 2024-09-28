/**
*
* @verbatim
    History
    2012-09-03  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*           (c) 2011-2017 Geodetic Observatory Pecny, http://www.pecny.cz (gnss@pecny.cz)
      Research Institute of Geodesy, Topography and Cartography
      Ondrejov 244, 251 65, Czech Republic
* @file       gpreproc.h
* @brief      Purpose: preprocessing, header
*.
* @author     PV
* @version    1.0.0
* @date       2012-09-03
*
*/

#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <sstream>

#include "newmat/newmat.h"

#include "gall/gallobs.h"
#include "gall/gallnav.h"
#include "gset/gsetgen.h"
#include "gset/gsetgnss.h"
#include "gset/gsetproc.h"

using namespace gnut;

namespace gnut
{

    class t_gobs_pair;

    /** @brief class for t_gpreproc derive from t_gmonit. */
    class LibGnut_LIBRARY_EXPORT t_gpreproc
    {
    public:
        /** @brief constructor 1. */
        t_gpreproc(t_gallobs *obs, t_gsetbase *settings);

        /** @brief default destructor. */
        ~t_gpreproc();

        /** @brief set/get glog pointer. */
        void spdlog(t_spdlog spdlog);
        t_spdlog spdlog() const { return _spdlog; }

        /** @brief for Cycle slip detection. */
        int ProcessBatch(string site, const t_gtime &beg, const t_gtime &end, double sampl, bool sync, bool save = false);

        /** @brief for set the priviate values. */
        void setNav(t_gallnav *nav);

        typedef map<int, map<t_gobs_pair, double>> t_map_slp;
        typedef map<int, vector<t_gobs_pair>> t_vec_slp;

    protected:
        t_gallobs *_obs;      ///< observation
        t_gallnav *_nav;      ///< navigation
        string _site;         ///< site
        t_spdlog _spdlog;     ///< spdlog
        t_gsetbase *_set;     ///< setting
        double _sigCode;      ///< sigma of code observation
        double _sigPhase;     ///< sigma of phase observation
        double _sigCode_GLO;  ///< sigma of GLONASS code observation
        double _sigPhase_GLO; ///< sigma of GLONASS phase observstion
        double _sigCode_GAL;  ///< sigma of GAL code observation
        double _sigPhase_GAL; ///< sigma of GAL phase observation
        double _sigCode_BDS;  ///< sigma of BDS code observation
        double _sigPhase_BDS; ///< sigma of BDS phase observation
        double _sigCode_QZS;  ///< sigma of QZS code observation
        double _sigPhase_QZS; ///< sigma of QZS phase observation
        double _sigCode_IRN;  ///< sigma of IRN code observation
        double _sigPhase_IRN; ///< sigma of IRN phase observation
        double _sumS;         ///< sumS
        double _scl;          ///< scl
        t_gtriple _pos;       ///< position
        set<string> _sys;     ///< system
        set<string> _sat;     ///< satellite

        bool _beg_end;

        map<string, map<string, double>> _dI; ///< ionospheric delay (site sys value)
        //map<string, int>     _msoffset;
        map<string, map<t_gtime, int>> _mbreaks; ///< for logging (site time CJ)

        // --- global settings
        map<string, bool> _firstEpo;                 ///< fisrt epoch
        map<string, vector<t_gobsgnss>> _epoDataPre; ///< epoch data prepare

        t_map_slp _m_lcslp; ///< m_lcslp
        t_vec_slp _v_lcslp; ///< v_lcslp

        map<string, map<t_gtime, map<string, map<GOBS, double>>>> _mslips; ///< for logging - slip due to true CS  (site ....)
        map<string, map<t_gtime, map<string, map<GOBS, int>>>> _mslipsGap; ///  for logging - slip due to data gap => 1 (epoch gap), 2(sat gap), 3(GOBS gap) (site ...)

        // for npp
        vector<shared_ptr<t_gobsgnss>> _inputEpoData;   ///< input epoch data
        map<string, map<string, int>> _map_nppdata_idx; ///< site sat idx

        /** @brief check phase cycl slips. */
        int _slip(t_gobsgnss *gobs1, t_spt_gobs gobs2);

        /** @brief check coherency between range and phase caused by clock jump. */
        int _jumps(t_gobsgnss *gobs1, t_spt_gobs gobs2);

        /** @brief Repair phase due to clk jump. */
        void _repair(vector<t_spt_gobs> epoData, double dL);

        /** @brief Transform dN to slips on individual band. */
        int _transform(t_spt_gobs gobs, bool save);

        /** @brief Save estimated cycle slips. */
        void _save(t_spt_gobs gobs, const map<GOBS, double> &slips);

        /** @brief remove slips from gobsgnss* and _mslips. */
        void _remove_slip(vector<t_spt_gobs> gobs);

        /** @brief only common items remain. */
        void _common(set<GOBSBAND> &set1, set<GOBSBAND> &set2);
        void _common(set<GOBS> &set1, set<GOBS> &set2);

        /** @brief ionosphere scale factor: wl - nl. */
        double _disf(t_gobsgnss *gobs1, t_spt_gobs gobs2, t_gobs &s1, t_gobs &s2);

        /** @brief ionosphere. */
        void _iono(t_gobsgnss *gobs1, t_spt_gobs gobs2, t_gobs &s1, t_gobs &s2);

        /** @brief find wl slip. */
        double _findSlp(int &i, t_gobs_pair &gpair);

        /** @brief report epo data as slips due to epoch data gap. */
        void _gapReport(vector<shared_ptr<t_gobsgnss>> epoData);
        void _gapReport(shared_ptr<t_gobsgnss> data);

        /** @brief compare two epoch data. */
        void _compare(vector<t_gobsgnss> data1, vector<shared_ptr<t_gobsgnss>> data2);
    };

    /** @brief class for t_gobs_pair. */
    class t_gobs_pair
    {
    public:
        /** @brief obs pair. */
        t_gobs_pair(t_gobs &gobs1, t_gobs &gobs2);
        t_gobs obs1; ///< obs1
        t_gobs obs2; ///< obs2
        double val;  ///< val

        /** @brief override operator <. */
        bool operator<(const t_gobs_pair &t) const;
    };

} // namespace

#endif
