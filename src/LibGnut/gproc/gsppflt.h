/**
*
* @verbatim
    History
    2014-11-24  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*.
* @file       gsppflt.h
* @brief      Purpose: implements spp client
*.
* @author     PV
* @version    1.0.0
* @date       2014-11-24
*
*/

#ifndef GSPPFLT_H
#define GSPPFLT_H

#include "gproc/gspp.h"
#include "gmodels/gdop.h"
#include "gproc/gflt.h"
#include "gall/gallpar.h"
#include "gall/gallbias.h"
#include "gall/gallprod.h"
#include "gutils/gsysconv.h"
#include "gmodels/gstochasticmodel.h"
#include "gset/gsetflt.h"

namespace gnut
{

#define SPP_MINSAT (6) ///< minimum number of satellites

    /** @brief class for t_gsppflt derive from t_gspp. */
    class LibGnut_LIBRARY_EXPORT t_gsppflt : public virtual t_gspp
    {
    public:
        /** @brief constructor 1. */
        t_gsppflt(string mark, t_gsetbase *set);

        t_gsppflt(string mark, t_gsetbase *set, t_spdlog spdlog, string mode="");
        /** @brief default destructor. */
        virtual ~t_gsppflt();

        /** @brief Process observation batch. */
        virtual int processBatch(const t_gtime &beg, const t_gtime &end);

        /** @brief min sat. */
        void minsat(size_t minsat) { _minsat = (minsat < 5) ? 5 : minsat; }

        /** @brief get crd. */
        t_gtriple getCrd(const t_gtime &time);

        /** @brief get outlier sat. */
        vector<string> get_outlier_sat() { return _outlier_sat; } 

    protected:
        /** @brief Predict state vector and covariance matrix. */
        virtual void _predict();

        /** @brief Restore state and covariance matrix. */
        virtual void _restore(const SymmetricMatrix &Qsav, const t_gallpar &Xsav);

        /** @brief Satelite position. */
        virtual int _satPos(t_gtime &, t_gsatdata &);

        /** @brief Prepare data: filter, bancroft, members in gsatdata. */
        virtual int _prepareData();

        /** @brief Tides are not applied in SPP. */
        virtual int _apply_tides(t_gtime &_epoch, t_gtriple &xRec);

        /** @brief Process one epoch. */
        virtual int _processEpoch(const t_gtime &runEpoch);

        /** @brief add data to A, l, P - code. */
        int _addObsP(t_gsatdata &satdata, unsigned int &iobs, t_gtriple &ell, Matrix &A, ColumnVector &l, DiagonalMatrix &P);

        /** @brief add data to A, l, P - carrier phase. */
        virtual int _addObsL(t_gsatdata &satdata, unsigned int &iobs, t_gtriple &ell, Matrix &A, ColumnVector &l, DiagonalMatrix &P);

        /** @brief add pseudo observations as a constrains. */
        virtual int _addPseudoZTD(unsigned int &iobs, t_gtriple &ell, Matrix &A, ColumnVector &l, DiagonalMatrix &P);

        /** @brief weight coef for observations. */
        double _weightObs(t_gsatdata &satdata, t_gobs &go);

        /** @brief Update time for RDW processes. */
        virtual void _timeUpdate(const t_gtime &epo);

        /** @brief Sync inter-GNSS systems bias. */
        void _syncSys();

        /** @brief Add/Remove ionosphere delay. */
        void _syncIono();

        /** @brief Add/Remove inter-freq. biases. */
        void _syncIFB();

        /** @brief save observations residuals. */
        void _save_residuals(ColumnVector &v, vector<t_gsatdata> &satdata, RESIDTYPE restype);

        /** @brief Apply DCB correction. */
        int _applyDCB(t_gsatdata &satdata, double &P, t_gobs *gobs1, t_gobs *gobs2 = 0);

        vector<t_gsatdata> _data; ///< data
        map<string, int> _newAMB; ///< newAMB
        set<string> _slips;       ///< slips
        unsigned int _minsat;     ///< minsat
        double _sig_unit;         ///< sig unit
        int _frequency;           ///< frequency

        // Models
        t_randomwalk *_trpStoModel; ///< trop stochastic model
        t_randomwalk *_ionStoModel; ///< iono stochastic model
        t_randomwalk *_gpsStoModel; ///< gps stochastic model
        t_randomwalk *_gloStoModel; ///< glo stochastic model
        t_randomwalk *_galStoModel; ///< gal stochastic model
        t_randomwalk *_bdsStoModel; ///< bds stochastic model
        t_randomwalk *_qzsStoModel; ///< qzs stochastic model
        t_whitenoise *_clkStoModel; ///< clk stochastic model
        t_whitenoise *_crdStoModel; ///< crd stochastic model

        t_gallpar _param;
        SymmetricMatrix _Qx;     ///< Parameters and covariance matrix
        DiagonalMatrix _Noise;   ///< Noise matrix
        t_gtime _epoch;          ///< Epoch time
        t_gflt *_filter;         ///< Estimation objects
        int _numSat(GSYS gsys);  ///< num sat
        ColumnVector _vBanc;     ///< v
        t_gdop _dop;             ///< dop
        int _cntrep;             ///< cntrep
        int _numcor;             ///< num cor
        bool _smooth;            ///< smooth
        unsigned int _n_NPD_flt; ///< NPD flt number
        unsigned int _n_ALL_flt; ///< NPD smt number
        unsigned int _n_NPD_smt; ///< NPD smt number
        unsigned int _n_ALL_smt; ///< ALL smt number

        RESIDTYPE _resid_type; ///< res type

        /** @brief get obs. */
        int _getgobs(string prn, GOBSTYPE type, GOBSBAND band, t_gobs &gobs);
        map<string, int> _frqNum;      ///< frequency number
        map<string, t_gtime> _lastEcl; ///< last ecl

        bool _ifb3_init; ///< ifb3
        bool _ifb4_init; ///< ifb4
        bool _ifb5_init; ///< ifb5

        bool _auto_band;      ///< auto band
        CBIASCHAR _cbiaschar; ///< bias(char)

        map<t_gtime, t_gtriple> _map_crd;

        vector<string> _outlier_sat;  ///< outlier sat

		map<GSYS, map<FREQ_SEQ, GOBSBAND>> _band_index;  ///< band index
		map<GSYS, map<GOBSBAND, FREQ_SEQ>> _freq_index;  ///< freq index
    };

} // namespace

#endif // GSPPFLT_H
