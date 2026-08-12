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

#ifndef SPP_MINSAT
#define SPP_MINSAT (6) ///< minimum number of satellites
#endif

    /** @brief class for t_gsppflt derive from t_gspp. */
    class LibGnut_LIBRARY_EXPORT t_gsppflt : public virtual t_gspp
    {
    public:
        /** @brief constructor 1. */
        t_gsppflt(std::string mark, t_gsetbase* set);

        t_gsppflt(std::string mark, t_gsetbase* set, std::string mode);
        /** @brief default destructor. */
        virtual ~t_gsppflt();

        /** @brief Process observation batch. */
        virtual int processBatch(const t_gtime& beg, const t_gtime& end);

        /** @brief min sat. */
        void minsat(std::size_t minsat)
        {
            _minsat = (minsat < 5) ? 5 : minsat;
        }

        /** @brief get crd. */
        t_gtriple getCrd(const t_gtime& time);

        /** @brief get outlier sat. */
        std::vector<GSAT> get_outlier_sat()
        {
            return _outlier_sat;
        }

    protected:
        /** @brief Predict state vector and covariance matrix. */
        virtual void _predict();

        /** @brief Restore state and covariance matrix. */
        virtual void _restore(const Eigen::MatrixXd& Qsav, const t_gallpar& Xsav);

        /** @brief Satelite position. */
        virtual int _satPos(t_gtime&, t_gsatdata&);

        /** @brief Prepare data: filter, bancroft, members in gsatdata. */
        virtual int _prepareData();

        /** @brief Tides are not applied in SPP. */
        virtual int _apply_tides(t_gtime& _epoch, t_gtriple& xRec);

        /** @brief Process one epoch. */
        virtual int _processEpoch(const t_gtime& runEpoch);

        /** @brief add data to A, l, P - code. */
        int _addObsP(t_gsatdata& satdata, unsigned int& iobs, t_gtriple& ell, Eigen::MatrixXd& A, Eigen::VectorXd& l, Eigen::DiagonalMatrix<double, Eigen::Dynamic>& P);

        /** @brief add data to A, l, P - carrier phase. */
        virtual int _addObsL(t_gsatdata& satdata, unsigned int& iobs, t_gtriple& ell, Eigen::MatrixXd& A, Eigen::VectorXd& l, Eigen::DiagonalMatrix<double, Eigen::Dynamic>& P);

        /** @brief add pseudo observations as a constrains. */
        virtual int _addPseudoZTD(unsigned int& iobs, t_gtriple& ell, Eigen::MatrixXd& A, Eigen::VectorXd& l, Eigen::DiagonalMatrix<double, Eigen::Dynamic>& P);

        /** @brief weight coef for observations. */
        double _weightObs(t_gsatdata& satdata, t_gobs& go);

        /** @brief Update time for RDW processes. */
        virtual void _timeUpdate(const t_gtime& epo);

        /** @brief Sync inter-GNSS systems bias. */
        void _syncSys();

        /** @brief Add/Remove ionosphere delay. */
        void _syncIono();

        /** @brief Add/Remove inter-freq. biases. */
        void _syncIFB();

        /** @brief save observations residuals. */
        void _save_residuals(Eigen::VectorXd& v, std::vector<t_gsatdata>& satdata, RESIDTYPE restype);

        /** @brief Apply DCB correction. */
        int _applyDCB(t_gsatdata& satdata, double& P, t_gobs* gobs1, t_gobs* gobs2 = 0);

        std::vector<t_gsatdata> _data; ///< data
        std::map<GSAT, int> _newAMB;   ///< newAMB
        unsigned int _minsat;     ///< minsat
        double _sig_unit;         ///< sig unit
        int _frequency;           ///< frequency

        // Models
        t_randomwalk* _trpStoModel; ///< trop stochastic model
        t_randomwalk* _ionStoModel; ///< iono stochastic model
        t_randomwalk* _gpsStoModel; ///< gps stochastic model
        t_randomwalk* _gloStoModel; ///< glo stochastic model
        t_randomwalk* _galStoModel; ///< gal stochastic model
        t_randomwalk* _bdsStoModel; ///< bds stochastic model
        t_randomwalk* _qzsStoModel; ///< qzs stochastic model
        t_whitenoise* _clkStoModel; ///< clk stochastic model
        t_whitenoise* _crdStoModel; ///< crd stochastic model

        t_gallpar _param;
        Eigen::MatrixXd _Qx;     ///< Parameters and covariance matrix
        Eigen::DiagonalMatrix<double, Eigen::Dynamic> _Noise;   ///< Noise matrix
        t_gtime _epoch;          ///< Epoch time
        t_gflt* _filter;         ///< Estimation objects
        int _numSat(GSYS gsys);  ///< num sat
        Eigen::VectorXd _vBanc;     ///< v
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
        int _getgobs(GSAT sat, GOBSTYPE type, GOBSBAND band, t_gobs& gobs);
        std::map<GSAT, int> _frqNum;        ///< frequency number
        std::map<GSAT, t_gtime> _lastEcl; ///< last ecl

        bool _ifb3_init; ///< ifb3
        bool _ifb4_init; ///< ifb4
        bool _ifb5_init; ///< ifb5

        bool _auto_band;      ///< auto band
        CBIASCHAR _cbiaschar; ///< bias(char)

        std::map<t_gtime, t_gtriple> _map_crd;

        std::vector<GSAT> _outlier_sat; ///< outlier sat

        std::map<GSYS, std::map<FREQ_SEQ, GOBSBAND>> _band_index; ///< band index
        std::map<GSYS, std::map<GOBSBAND, FREQ_SEQ>> _freq_index; ///< freq index
    };

} // namespace gnut

#endif // GSPPFLT_H
