/**
 * @file         gpvtflt.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        main control class for RTK/PPP Processing
 * @version      1.0
 * @date         2024-08-29
 *
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 *
 */
#ifndef GPVTFLT_H
#define GPVTFLT_H

#include "gexport/ExportLibGREAT.h"
#include "gproc/gpppflt.h"
#include "gproc/gqualitycontrol.h"
#include "gmodels/gpppmodel.h"
#include "gambfix/gambiguity.h"
#include "gdata/gposdata.h"
#include "gmodels/gcombmodel.h"
#include "gproc/gpreproc.h"
#include "gproc/gfltmatrix.h"

using namespace gsins;

namespace great
{
    /**
     * @brief class for gpvtflt, derive from gpppflt
     */
    class LibGREAT_LIBRARY_EXPORT t_gpvtflt : public t_gpppflt
    {
    public:
        /** @brief default constructor. */
        t_gpvtflt(std::string mark, std::string mark_base, t_gsetbase* set, t_gallproc* allproc = nullptr);

        /** @brief default destructor. */
        virtual ~t_gpvtflt();

        /** @brief processBatch. */
        virtual int processBatch(const t_gtime& beg, const t_gtime& end, bool prtOut);

        /** @brief Initializing some settings. */
        virtual bool InitProc(const t_gtime& begT, const t_gtime& endT, double* subint = NULL);

        /**
         * @brief One epoch processing
         * @note virtual function=
         * @param[in] Now    current time
         * @param[in] data_rover
         * @param[in] data_base
         * @return -1,failed; 0,float; 1,fixed.
         */
        virtual int ProcessOneEpoch(const t_gtime& now, std::vector<t_gsatdata>* data_rover = NULL, std::vector<t_gsatdata>* data_base = NULL);

        /** @brief Add UPD. */
        virtual void Add_UPD(t_gupd* gupd);

        /**
         * @brief calculate azel and rho
         * @Note modified for PVT
         * @param[in] current site name
         * @param[in] current satellite position
         * @param[in] current site  position
         * @param[in/out] current site  data
         * @return void
         */
        void Add_rho_azel(/*const t_gtime &runEpoch, */ const std::string& site_name,
                          t_gtriple& xyz_s,
                          const t_gtriple& xyz_r,
                          t_gsatdata& obs_sat);

        /**
         * @brief Setting given reference satellites.
         * @param[in] current reference satellites
         */
        void setRefsat(std::map<GSYS, std::string> satref)
        {
            _ipSatRep.clear();
            for (const auto& item : satref) _ipSatRep[item.first] = str2gsat(item.second, item.first);
        }

    protected:
        /**  @brief initiate RTK parameters */
        int _rtkinit();

        /**  @brief Setting base or rover coordinates */
        int _setCrd();

        /**  @brief update RTK SD ambiguity parameters */
        void _udsdAmb();

        /** @brief adapting baseline changing. */
        virtual void _delPar(const par_type par);

        /**
         * @brief select common sats between base and rover sites
         * @param[in/out] base-site satellite data
         * @param[in/out] rover-site satellite data
         * @return number of common sats
         */
        int _selcomsat(std::vector<t_gsatdata>& data_base, std::vector<t_gsatdata>& data_rover);

        /**
         * @brief To jude whether exist UD Phase/Range residuals in current satellite
         * @return true: exist; false: no exist
         */
        bool _valid_residual(bool phase_process, GSAT sat, FREQ_SEQ& f, std::map<GSAT, std::map<FREQ_SEQ, std::pair<int, int>>>& index_l);

        /**
         * @brief add pseudorange and carrier-phase measurement to model. Rewritten for RTK application.
         * @note,Lvhb created  for RTK/SD-PPP
         * @param[in/out] A            Coff Eigen::MatrixXd
         * @param[in/out] l            measurement vector
         * @param[in/out] P            measurement variance
         * @return retval > 0,success; retval < 0,fail
         */
        int _combineDD(Eigen::MatrixXd& A, Eigen::MatrixXd& P, Eigen::VectorXd& l);

        /** @brief filter time update. */
        void _syncAmb();

        /** @brief update PPP UD ambiguity parameters */
        void _udAmb();

        /** @obtain satcrd,satclk,satele,satazi,rho, it must be two bands*/
        virtual int _preprocess(const std::string& ssite, std::vector<t_gsatdata>& sdata);

        /** @brief prepare data.    */
        virtual int _prepareData();

        /** @brief Process one epoch. */
        virtual int _processEpoch(const t_gtime& runEpoch);

        /** @brief ambiguity resolution. */
        virtual int _amb_resolution();

        /** @brief is get ref sat. */
        bool _getSatRef();

        /** @brief Detect outlier. */
        virtual int _outlierDetect(const Eigen::VectorXd& v, const Eigen::MatrixXd& Qsav, GSAT& sat);

        /**
         * @brief add doppler measurement to model
         * @param[in] satdata    satellite data
         * @param[in] iobs        index
         * @param[in] XYZ        ECEF coordinate
         * @param[out] A            Coff Eigen::MatrixXd
         * @param[out] l            measurement vector
         * @param[out] P            measurement variance
         * @return retval > 0,success; retval < 0,fail
         */
        int _addObsD(t_gsatdata& satdata,
                     unsigned int& iobs,
                     t_gallpar& param,
                     t_gtriple& XYZ,
                     Eigen::MatrixXd& A,
                     Eigen::VectorXd& l,
                     Eigen::DiagonalMatrix<double, Eigen::Dynamic>& P);

        /** @brief post residual for rtk */
        int _postRes(const Eigen::MatrixXd& A, const Eigen::MatrixXd& P, const Eigen::VectorXd& l, const Eigen::VectorXd& dx);

        /** @brief modified for PPP/RTK processing */
        virtual int _combineMW(t_gsatdata& satdata);

        /** @brief process one epoch Velocity. */
        virtual int _processEpochVel();

        /** @brief kml description. */
        virtual std::string _gen_kml_description(const t_gtime& epoch, const t_gposdata::data_pos& pos);

        /** @brief quality grade. */
        virtual std::string _quality_grade(const t_gposdata::data_pos& pos);

        /** @brief filter time update. */
        virtual void _predict(const t_gtime& runEpoch);

        /** @brief print  the result. */
        virtual void _prtOut(t_gtime& epo,
                             t_gallpar& X,
                             const Eigen::MatrixXd& Q,
                             std::vector<t_gsatdata>& data,
                             std::ostringstream& os,
                             xml_node& node,
                             bool saveProd = true);
        virtual void _prt_port(t_gtime& epo, t_gallpar& X, const Eigen::MatrixXd& Q, std::vector<t_gsatdata>& data);

        /** @brief print  the result. */
        void _prtOutHeader();

        /** @brief generate Obs Index. */
        void _generateObsIndex(t_gfltEquationMatrix& equ);

        /** @brief slip detect. */
        bool _slip_detect(const t_gtime& now);

        /** @brief add filter epoch data. */
        int _getData(const t_gtime& now, std::vector<t_gsatdata>* data, bool isBase);

        /** @brief avoid same name with _valid_crd_xml. */
        bool _crd_xml_valid();

        /** @brief remove sat. */
        void _remove_sat(GSAT sat);

        /** @brief check sat. */
        bool _check_sat(const std::string& ssite, t_gsatdata* const iter, Eigen::MatrixXd& BB, int& iobs);

        /** @brief compute rec crd. */
        bool _cmp_rec_crd(const std::string& ssite, Eigen::MatrixXd& BB);

        /** @brief compute sat information. */
        bool _cmp_sat_info(const std::string& ssite, t_gsatdata* const iter);

        /** @brief predict Crd. */
        void _predictCrd();

        /** @brief predict Clk. */
        void _predictClk();

        /** @brief predict Bias. */
        void _predictBias();

        /** @brief predictIono. */
        void _predictIono(const double& bl, const t_gtime& runEpoch);

        /** @brief predict Tropo. */
        void _predictTropo();

        /** @brief predict Amb. */
        void _predictAmb();

        /** @brief compute equation. */
        unsigned int _cmp_equ(t_gfltEquationMatrix& equ);

        /** @brief posteriori Test. */
        void _posterioriTest(const Eigen::MatrixXd& A,
                             const Eigen::MatrixXd& P,
                             const Eigen::VectorXd& l,
                             const Eigen::VectorXd& dx,
                             const Eigen::MatrixXd& Q,
                             Eigen::VectorXd& v_norm,
                             double& vtpv);

    protected:
        t_whitenoise* _dclkStoModel;                                              ///< clock drift model
        t_whitenoise* _velStoModel;                                               ///< velocity model
        t_gtime _beg_time;                                                        ///< beg time of proc
        t_gtime _end_time;                                                        ///< end time of proc
        bool _isFirstFix;                                                         ///< is First Fix
        double _max_res_norm;                                                     ///< maximum normalize res
        bool _isBase;                                                             ///< if using base,true; other: fasle
        Eigen::MatrixXd _post_A;                                                           ///< post A P L for RTK
        Eigen::MatrixXd _post_P;                                                  ///< post A P L for RTK
        Eigen::VectorXd _post_l;                                                     ///< post A P L for RTK
        std::string _site_base;                                                        ///< base site name;
        std::vector<t_gsatdata> _data_base;                                            ///< base data in every epoch;
        t_gpppmodel* _gModel_base;                                                ///< base model
        std::set<GSAT> _sat_ref;                                                       ///< reference sat
        std::map<GSYS, GSAT> _ipSatRep;                                                ///< input reference satellite
        t_gupd* _gupd = nullptr;                                                  ///< upd
        t_gambiguity* _ambfix = nullptr;                                          ///< ambfix
        FIX_MODE _fix_mode;                                                       ///< fix mode
        UPD_MODE _upd_mode = UPD_MODE::UPD;                                       ///< upd mode
        t_map_MW _MW, _EWL, _LW, _LE, _LWL, _rMW, _rEWL, _IMW, _IEWL, _PW, _AFIF; ///< t_map
        bool _amb_state;                                                          ///< ambiguity state
        t_gallpar _param_fixed;                                                   ///< param fixed
        t_gtriple _vel;                                                           ///< vel
        Eigen::MatrixXd _Qx_vel;                                                  ///< Qx_vel
        std::map<GSAT, double> _crt_ele;                                               ///< crt vel
        std::map<GSAT, std::map<FREQ_SEQ, double>> _crt_SNR;                                ///< crt SNR
        std::vector<std::pair<GSAT, std::pair<FREQ_SEQ, GOBSTYPE>>> _obs_index;                ///< obs index
        std::map<std::pair<GSAT, par_type>, std::tuple<GOBS, GOBS, GOBS, GOBS>> _amb_obs;
        Eigen::VectorXd _vBanc_base;                       ///< _vBanc_base
        std::map<GSYS, std::map<FREQ_SEQ, GOBSBAND>> _band_index; ///< band index
        std::map<GSYS, std::map<GOBSBAND, FREQ_SEQ>> _freq_index; ///< freq index
        int _frequency;                                 ///< frequency
        t_gallproc* _allproc = nullptr;                 ///< process data
        std::shared_ptr<t_gbasemodel> _base_model = nullptr; ///< base model
        SLIPMODEL _slip_model;                          ///< slip model
        std::shared_ptr<t_gpreproc> _gpre;                   ///< preprocess
        t_gqualitycontrol _gquality_control;            ///< gquality control
        t_gtime _wl_Upd_time;                           ///< wl Upd time
        t_gtime _ewl_Upd_time;                          ///< ewl Upd time
        int _realnobs;                                  ///< number of obs
        RECEIVERTYPE _receiverType;                     ///< receiver Type
        std::map<GSAT, std::string> _sat_freqs;                   ///< sat freqs
    };
    template <class T1, class T2> void t_out(T1 const& name, T2 const& matrix)
    {
        std::cout << name << std::endl;
        std::cout << std::fixed << std::setprecision(6) << std::setw(15) << matrix << std::endl;
        return;
    }
} // namespace great

#endif
