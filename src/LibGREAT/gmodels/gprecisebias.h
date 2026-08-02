/**
 * @file         gprecisebias.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        mainly about precise bias
 * @version      1.0
 * @date         2024-08-29
 *
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 *
 */
#ifndef precisebias_h
#define precisebias_h

#include "gexport/ExportLibGREAT.h"
#include "gset/gsetbase.h"
#include "gset/gsetgen.h"
#include "gmodels/gbiasmodel.h"
#include "gall/gallproc.h"
#include "gall/gallobj.h"
#include "gmodels/gtide.h"
#include "gmodels/gattitudemodel.h"
#include "gdata/gpoleut1.h"
#include "gdata/gifcb.h"
#include "gall/gallopl.h"

namespace great
{
    /** @brief class for precise bias    */
    class LibGREAT_LIBRARY_EXPORT t_gprecisebias : public t_gbiasmodel
    {
    public:
        t_gprecisebias(t_gallproc* data, t_gsetbase* setting);
        ~t_gprecisebias();

        /** @brief Combined equation */
        bool cmb_equ(t_gtime& epoch, t_gallpar& params, t_gsatdata& obsdata, t_gobs& gobs, t_gbaseEquation& result) override;

        /** @brief update obj clk */
        void update_obj_clk(const string& obj, const t_gtime& epo, double clk) override;

        /** @brief get clk of reciever */
        double get_rec_clk(const string& obj) override;

        /** @brief get tropoDelay */
        double tropoDelay(t_gtime& epoch, string& rec, t_gallpar& param, t_gtriple site_ell, t_gsatdata& satdata);

        /** @brief get ionoDelay */
        double ionoDelay(t_gtime& epoch, t_gallpar& param, t_gsatdata& satdata, IONMODEL& ion_model, GOBSBAND& band_1, t_gobs& gobs);

        /** @brief get isbDelay */
        double isbDelay(t_gallpar& param, t_gsatdata& satdata, string& sat, string& rec, t_gobs& gobs);

        /** @brief get ifbDelay */
        double ifbDelay(t_gallpar& param, t_gsatdata& satdata, string& sat, string& rec, t_gobs& gobs);

        /** @brief get ifcbDelay */
        double ifcbDelay(t_gsatdata& satdata, t_gifcb* ifcb, OBSCOMBIN obscombin);

        /** @brief get relDelay */
        double relDelay(t_gtriple& crd_site, t_gtriple& vel_site, t_gtriple& crd_sat, t_gtriple& vel_sat);

        /** @brief get update obs info */
        bool _update_obs_info(t_gsatdata& obsdata);

        /** @brief reset SatPCO */
        virtual void reset_SatPCO(bool cal = true)
        {
            _isCalSatPCO = cal;
        };

        /** @brief get crt obs */
        virtual t_gsatdata get_crt_obs()
        {
            return _crt_obs;
        }

    protected:
        /** @brief apply rec */
        bool _apply_rec(const t_gtime& crt_epo, const t_gtime& rec_epo, t_gallpar& pars);

        /** @brief apply sat */
        bool _apply_sat(const t_gtime& rec_epo, t_gtime& sat_epo, t_gallnav* nav);

        /** @brief apply rec tides */
        bool _apply_rec_tides(const t_gtime& epoch, t_gtriple& rec);

        /** @brief get satellite position and velocity in TRS */
        bool _get_trs_sat_crdvel(const t_gtime& sat_epoch,
                                 const string& sat,
                                 t_gallnav* nav,
                                 t_gtriple& trs_sat_crd,
                                 t_gtriple& trs_sat_vel);

    protected:
        /**
         * @brief compute windup correciton
         * @note override for fixed bugs in computing satellite attitude
         * @param[in] satdata observ data info
         * @param[in] rRec coord of receiver in TRS
         * @return correction of windup
         */
        double windUp(const GOBSBAND& freq_2, t_gsatdata& satdata, const Eigen::Vector3d& rRec);

        /** @brief get PCV */
        double PCV(bool corrt_sat,
                   bool corrt_rec,
                   t_gtime& epoch,
                   t_gtime& crt_sat_epo,
                   t_gtriple& trs_rec_crd,
                   t_gsatdata& satdata,
                   t_gobs& gobs);

        /**
         * @brief get rotmatrix from antenna frame to TRS
         * @param[in] obsdata observ data info
         * @param[in] epoch antenna calibration epoch
         * @param[in] obj pcv info for site
         * @return Rotmatrix
         */
        Matrix _RotMatrix_Ant(t_gsatdata& obsdata, const t_gtime& epoch, shared_ptr<t_gobj> obj);

    protected:
        t_gtime _crt_epo;            ///< epoch
        t_gsatdata _crt_obs;         ///< obs
        shared_ptr<t_gobj> _crt_obj; ///< obj
        t_gtime _crt_rec_epo;        ///< rec epoch
        t_gtime _crt_sat_epo;        ///< transmit epoch
        double _crt_rec_clk;         ///< rec clk
        double _crt_sat_clk;         ///< sat clk

        CONSTRPAR _crd_est = CONSTRPAR::FIX;

        ATTITUDES _attitudes;
        t_gattitude_model _gattitude_model;
        bool _trop_est = true;      ///< estimate tropo or not
        bool _is_flt = false;       ///< flt or lsq
        bool _corrt_sat_pcv = true; ///< correct sat pcv or not
        bool _corrt_rec_pcv = true; ///< correct rec pcv or not
        GRDMPFUNC _grad_mf;         ///< grad mf
        shared_ptr<t_gtide> _tide;  ///< tide correction model

        t_gpoleut1* _gdata_erp = nullptr; ///< all poleut1 data
        t_gallnav* _gall_nav = nullptr;   ///< all nav data include rinexn,sp3,clk
        t_gallobj* _gallobj = nullptr;    ///< all obj
        t_gallopl* _opl = nullptr;        ///< opl
        modeofmeanpole _mean_pole_model = modeofmeanpole::cubic;

        double _minElev; ///< min ele for prepare

        map<string, map<string, map<t_gtime, double>>> _phase_windup; ///< recording for calculating windup

        t_gifcb* _gifcb = nullptr;                      ///< ifcb
        map<string, pair<t_gtime, double>> _obj_clk;    ///< obj clk
        map<string, double> _rec_clk;                   ///< rec clk
        bool _isCalSatPCO = true;                       ///< is Cal Sat PCO
        tuple<string, string, t_gtime> _rec_sat_before; ///< rec sat before
    };
} // namespace great

#endif
