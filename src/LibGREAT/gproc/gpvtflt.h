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
        t_gpvtflt(string mark, string mark_base, t_gsetbase *set, t_gallproc *allproc = nullptr);

        /**
        * @brief Constructor
        * @note set parameter value
        * @param[in] mark            mark
        * @param[in] set             setbase
        * @param[in] allproc         all process
        */
        explicit t_gpvtflt(string mark, string mark_base, t_gsetbase *set, t_spdlog spdlog,t_gallproc *allproc = nullptr);

        /** @brief default destructor. */
        virtual ~t_gpvtflt();

        /** @brief processBatch. */
        virtual int processBatch(const t_gtime &beg, const t_gtime &end, bool prtOut);

        /** @brief Initializing some settings. */
        virtual bool InitProc(const t_gtime &begT, const t_gtime &endT, double *subint = NULL);

        /**
        * @brief One epoch processing
        * @note virtual function=
        * @param[in] Now    current time
        * @param[in] data_rover
        * @param[in] data_base
        * @return -1,failed; 0,float; 1,fixed.
        */
        virtual int ProcessOneEpoch(const t_gtime &now, vector<t_gsatdata> *data_rover = NULL, vector<t_gsatdata> *data_base = NULL);

        /** @brief Add UPD. */
        virtual void Add_UPD(t_gupd *gupd);

        /** 
        * @brief calculate azel and rho
        * @Note modified for PVT
        * @param[in] current site name
        * @param[in] current satellite position
        * @param[in] current site  position
        * @param[in/out] current site  data
        * @return void 
        */
        void Add_rho_azel(/*const t_gtime &runEpoch, */ const string &site_name, t_gtriple &xyz_s, const t_gtriple &xyz_r, t_gsatdata &obs_sat);

        /**
        * @brief Setting given reference satellites.
        * @param[in] current reference satellites
        */
        void setRefsat(map<GSYS, string> satref) { _ipSatRep = satref; }

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
        int _selcomsat(vector<t_gsatdata>& data_base, vector<t_gsatdata>& data_rover);

        /**  
        * @brief To jude whether exist UD Phase/Range residuals in current satellite 
        * @return true: exist; false: no exist
        */
        bool _valid_residual(bool phase_process, string sat_name, enum FREQ_SEQ &f, map<string, map<FREQ_SEQ, pair<int, int>>> &index_l);

        /**
        * @brief add pseudorange and carrier-phase measurement to model. Rewritten for RTK application.
        * @note,Lvhb created  for RTK/SD-PPP
        * @param[in/out] A            Coff Matrix
        * @param[in/out] l            measurement vector
        * @param[in/out] P            measurement variance
        * @return retval > 0,success; retval < 0,fail
        */
        int _combineDD(Matrix &A, SymmetricMatrix &P, ColumnVector &l);

        /** @brief filter time update. */
        void _syncAmb();

        /** @brief update PPP UD ambiguity parameters */
        void _udAmb();

        /** @obtain satcrd,satclk,satele,satazi,rho, it must be two bands*/
        virtual int _preprocess(const string &ssite, vector<t_gsatdata> &sdata); 

        /** @brief prepare data.    */
        virtual int _prepareData();

        /** @brief Process one epoch. */
        virtual int _processEpoch(const t_gtime &runEpoch);

        /** @brief ambiguity resolution. */
        virtual int _amb_resolution();

        /** @brief is get ref sat. */
        bool _getSatRef();

        /** @brief Detect outlier. */
        virtual int _outlierDetect(const ColumnVector &v, const SymmetricMatrix &Qsav, string &sat);

        /**
        * @brief add doppler measurement to model
        * @param[in] satdata    satellite data
        * @param[in] iobs        index
        * @param[in] XYZ        ECEF coordinate
        * @param[out] A            Coff Matrix
        * @param[out] l            measurement vector
        * @param[out] P            measurement variance
        * @return retval > 0,success; retval < 0,fail
        */
        int _addObsD(t_gsatdata &satdata, unsigned int &iobs, t_gallpar &param, t_gtriple &XYZ, Matrix &A, ColumnVector &l, DiagonalMatrix &P);

        /** @brief post residual for rtk */
        int _postRes(const Matrix &A, const SymmetricMatrix &P, const ColumnVector &l,const ColumnVector &dx);

        /** @brief modified for PPP/RTK processing */
        virtual int _combineMW(t_gsatdata &satdata);

        /** @brief process one epoch Velocity. */
        virtual int _processEpochVel();

        /** @brief kml description. */
        virtual string _gen_kml_description(const t_gtime &epoch, const t_gposdata::data_pos &pos);

        /** @brief quality grade. */
        virtual string _quality_grade(const t_gposdata::data_pos &pos);

        /** @brief filter time update. */
        virtual void _predict(const t_gtime& runEpoch); 

        /** @brief print  the result. */
        virtual void _prtOut(t_gtime &epo, t_gallpar &X, const SymmetricMatrix &Q, vector<t_gsatdata> &data, ostringstream &os, xml_node &node, bool saveProd = true);
        virtual void _prt_port(t_gtime &epo, t_gallpar &X, const SymmetricMatrix &Q, vector<t_gsatdata> &data);

        /** @brief print  the result. */
        void _prtOutHeader();

        /** @brief generate Obs Index. */
        void _generateObsIndex(t_gfltEquationMatrix &equ);

        /** @brief slip detect. */
        bool _slip_detect(const t_gtime& now);
        
        /** @brief add filter epoch data. */
        int _getData(const t_gtime& now, vector <t_gsatdata>* data, bool isBase);

        /** @brief avoid same name with _valid_crd_xml. */
        bool _crd_xml_valid(); 

        /** @brief remove sat. */
        void _remove_sat(const string &satid);

        /** @brief check sat. */
        bool _check_sat(const string& ssite, t_gsatdata * const iter, Matrix &BB, int &iobs);

        /** @brief compute rec crd. */
        bool _cmp_rec_crd(const string& ssite, Matrix& BB);

        /** @brief compute sat information. */
        bool _cmp_sat_info(const string& ssite, t_gsatdata* const iter);

        /** @brief predict Crd. */
        void _predictCrd();

        /** @brief predict Clk. */
        void _predictClk();

        /** @brief predict Bias. */
        void _predictBias();
        
        /** @brief predictIono. */
        void _predictIono(const double &bl, const t_gtime& runEpoch);

        /** @brief predict Tropo. */
        void _predictTropo();

        /** @brief predict Amb. */
        void _predictAmb();

        /** @brief compute equation. */
        unsigned int _cmp_equ(t_gfltEquationMatrix &equ);

        /** @brief posteriori Test. */
        void _posterioriTest(const Matrix& A, const SymmetricMatrix& P, const ColumnVector& l,
            const ColumnVector& dx, const SymmetricMatrix& Q, ColumnVector& v_norm, double& vtpv);

    protected:
        t_whitenoise *_dclkStoModel;       ///< clock drift model
        t_whitenoise *_velStoModel;        ///< velocity model
        t_gtime _beg_time;                 ///< beg time of proc
        t_gtime _end_time;                 ///< end time of proc
        bool _isFirstFix;                  ///< is First Fix
        double _max_res_norm;              ///< maximum normalize res 
        bool _isBase;                      ///< if using base,true; other: fasle  
        Matrix _post_A;                    ///< post A P L for RTK  
        SymmetricMatrix _post_P;           ///< post A P L for RTK 
        ColumnVector _post_l;              ///< post A P L for RTK 
        string _site_base;                 ///< base site name; 
        vector<t_gsatdata> _data_base;     ///< base data in every epoch; 
        t_gpppmodel* _gModel_base;         ///< base model 
        set<string> _sat_ref;              ///< reference sat 
        map<GSYS, string> _ipSatRep;       ///< input reference satellite
        t_gupd *_gupd = nullptr;           ///< upd
        t_gambiguity *_ambfix = nullptr;   ///< ambfix
        FIX_MODE _fix_mode;                ///< fix mode
        UPD_MODE _upd_mode = UPD_MODE::UPD;///< upd mode
        t_map_MW _MW, _EWL, _LW, _LE, _LWL, _rMW, _rEWL, _IMW, _IEWL, _PW, _AFIF;///< t_map
        bool _amb_state;                   ///< ambiguity state
        t_gallpar _param_fixed;            ///< param fixed
        t_gtriple _vel;                    ///< vel
        SymmetricMatrix _Qx_vel;           ///< Qx_vel
        map<string, double> _crt_ele;      ///< crt vel
        map<string, map<FREQ_SEQ, double>> _crt_SNR;///< crt SNR
        vector<pair<string, pair<FREQ_SEQ, GOBSTYPE>>> _obs_index;///< obs index
        map<pair<string, par_type>, tuple<GOBS, GOBS, GOBS, GOBS>> _amb_obs;
        ColumnVector _vBanc_base;          ///< _vBanc_base
        map<GSYS, map<FREQ_SEQ, GOBSBAND>> _band_index;///< band index
        map<GSYS, map<GOBSBAND, FREQ_SEQ>> _freq_index;///< freq index
        int _frequency;                    ///< frequency
        t_gallproc *_allproc = nullptr;    ///< process data
        shared_ptr<t_gbasemodel> _base_model = nullptr;///< base model
        SLIPMODEL _slip_model;            ///< slip model
        shared_ptr<t_gpreproc> _gpre;     ///< preprocess
        t_gqualitycontrol _gquality_control;///< gquality control
        t_gtime _wl_Upd_time;             ///< wl Upd time
        t_gtime _ewl_Upd_time;            ///< ewl Upd time
        int _realnobs;                    ///< number of obs
        RECEIVERTYPE _receiverType;       ///< receiver Type
        map<string, string> _sat_freqs;   ///< sat freqs
    };
    template <class T1, class T2>
    void t_out(T1 const& name, T2 const& matrix)
    {
        cout << name << endl;
        cout << fixed << setprecision(6) << setw(15) << matrix << endl;
        return;
    }
}

#endif