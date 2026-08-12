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

#include <Eigen/Dense>

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
        t_gpreproc(t_gallobs* obs, t_gsetbase* settings);

        /** @brief default destructor. */
        ~t_gpreproc();

        /** @brief set/get glog pointer. */
        /** @brief for Cycle slip detection. */
        int ProcessBatch(std::string site, const t_gtime& beg, const t_gtime& end, double sampl, bool sync, bool save = false);

        /** @brief for set the priviate values. */
        void setNav(t_gallnav* nav);

        typedef std::map<int, std::map<t_gobs_pair, double>> t_map_slp;
        typedef std::map<int, std::vector<t_gobs_pair>> t_vec_slp;

    protected:
        t_gallobs* _obs;      ///< observation
        t_gallnav* _nav;      ///< navigation
        std::string _site;         ///< site
        t_gsetbase* _set;     ///< setting
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
        std::set<std::string> _sys;     ///< system
        std::set<GSAT> _sat;       ///< satellite

        bool _beg_end;

        std::map<std::string, std::map<GSAT, double>> _dI; ///< ionospheric delay (site/satellite)
        // map<string, int>     _msoffset;
        std::map<std::string, std::map<t_gtime, int>> _mbreaks; ///< for logging (site time CJ)

        // --- global settings
        std::map<std::string, bool> _firstEpo;                 ///< fisrt epoch
        std::map<std::string, std::vector<t_gobsgnss>> _epoDataPre; ///< epoch data prepare

        t_map_slp _m_lcslp; ///< m_lcslp
        t_vec_slp _v_lcslp; ///< v_lcslp

        std::map<std::string, std::map<t_gtime, std::map<GSAT, std::map<GOBS, double>>>> _mslips; ///< for logging - slip due to true CS
        std::map<std::string, std::map<t_gtime, std::map<GSAT, std::map<GOBS, int>>>>
            _mslipsGap; ///  for logging - slip due to data gap => 1 (epoch gap), 2(sat gap), 3(GOBS gap) (site ...)

        // for npp
        std::vector<std::shared_ptr<t_gobsgnss>> _inputEpoData;   ///< input epoch data

        /** @brief check phase cycl slips. */
        int _slip(t_gobsgnss* gobs1, t_spt_gobs gobs2);

        /** @brief check coherency between range and phase caused by clock jump. */
        int _jumps(t_gobsgnss* gobs1, t_spt_gobs gobs2);

        /** @brief Repair phase due to clk jump. */
        void _repair(std::vector<t_spt_gobs> epoData, double dL);

        /** @brief Transform dN to slips on individual band. */
        int _transform(t_spt_gobs gobs, bool save);

        /** @brief Save estimated cycle slips. */
        void _save(t_spt_gobs gobs, const std::map<GOBS, double>& slips);

        /** @brief remove slips from gobsgnss* and _mslips. */
        void _remove_slip(std::vector<t_spt_gobs> gobs);

        /** @brief only common items remain. */
        void _common(std::set<GOBSBAND>& set1, std::set<GOBSBAND>& set2);
        void _common(std::set<GOBS>& set1, std::set<GOBS>& set2);

        /** @brief ionosphere scale factor: wl - nl. */
        double _disf(t_gobsgnss* gobs1, t_spt_gobs gobs2, t_gobs& s1, t_gobs& s2);

        /** @brief ionosphere. */
        void _iono(t_gobsgnss* gobs1, t_spt_gobs gobs2, t_gobs& s1, t_gobs& s2);

        /** @brief find wl slip. */
        double _findSlp(int& i, t_gobs_pair& gpair);

        /** @brief report epo data as slips due to epoch data gap. */
        void _gapReport(std::vector<std::shared_ptr<t_gobsgnss>> epoData);
        void _gapReport(std::shared_ptr<t_gobsgnss> data);

        /** @brief compare two epoch data. */
        void _compare(std::vector<t_gobsgnss> data1, std::vector<std::shared_ptr<t_gobsgnss>> data2);
    };

    /** @brief class for t_gobs_pair. */
    class t_gobs_pair
    {
    public:
        /** @brief obs pair. */
        t_gobs_pair(t_gobs& gobs1, t_gobs& gobs2);
        t_gobs obs1; ///< obs1
        t_gobs obs2; ///< obs2
        double val;  ///< val

        /** @brief override operator <. */
        bool operator<(const t_gobs_pair& t) const;
    };

} // namespace gnut

#endif
