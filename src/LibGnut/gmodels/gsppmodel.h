
/**
*
* @verbatim
    History
    2014-11-27  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gsppmodel.h
* @brief      Purpose: various SPP models
*.
* @author     PV
* @version    1.0.0
* @date       2014-11-27
*
*/

#ifndef GSPPMODEL_H
#define GSPPMODEL_H

#include <string>
#include <map>
#include <cmath>

#include "gmodels/gmodel.h"

#include <Eigen/Dense>
#include "gset/gsetproc.h"
#include "gset/gsetgnss.h"
#include "gset/gsetrec.h"
#include "gutils/gsysconv.h"
#include "gutils/gnss.h"
#include "gmodels/ggmf.h"
#include "gutils/gnss.h"


namespace gnut
{
    /** @brief class for t_gsppmodel derive from t_godel. */
    class LibGnut_LIBRARY_EXPORT t_gsppmodel : public t_gmodel
    {
    public:
        /** @brief constructor 1. */
        t_gsppmodel(std::string site, t_gsetbase* settings);

        /** @brief default constructor. */
        t_gsppmodel();

        /** @brief default destructor. */
        virtual ~t_gsppmodel();

        /** @brief Outliers detection. */
        virtual int outlierDetect(std::vector<t_gsatdata>& data, Eigen::MatrixXd& Qx, const Eigen::MatrixXd&);

        /** @brief model for computed range value. */
        virtual double cmpObs(t_gtime& epoch, t_gallpar& param, t_gsatdata&, t_gobs& gobs, bool com = false);

        /** @brief model computed D range value (phase/code). */
        virtual double cmpObsD(t_gtime& epoch, t_gallpar& param, t_gsatdata& gsatdata, t_gobs& gobs);

        /** @brief get trop Delay. */
        double tropoDelay(t_gtime& epoch, t_gallpar& param, t_gtriple site_ell, t_gsatdata& satdata);

        // jdhuang add
        /** @brief is Correction. */
        double isbCorrection(t_gallpar& param, GSAT sat, std::string& rec, t_gobs& gobs);

        /** @brief is reset observation */
        virtual void reset_observ(OBSCOMBIN observ) override;

        /** @brief is set rec */
        virtual void setrec(std::shared_ptr<t_gobj> rec);

        /** @brief get outlier satellite */
        std::vector<GSAT> get_outlier_sat()
        {
            return _outlier_sat;
        }

    protected:
        /** @brief Find maximal residual */
        double _maxres(bool phase, std::vector<t_gsatdata>& data, std::vector<t_gsatdata>::iterator& itDATA, RESIDTYPE res_type, GSYS gs = GSYS::GNS);

        /** @brief check maximal residual */
        bool _check_outl(bool phase,
                         double& maxresNORM,
                         std::vector<t_gsatdata>::iterator& itDataNORM,
                         double& maxresORIG,
                         std::vector<t_gsatdata>::iterator& itDataORIG,
                         std::vector<t_gsatdata>::iterator& itDataErase,
                         std::vector<t_gsatdata>& data);

        /** @brief logging outlier */
        void _logOutl(bool phase, GSAT sat, int data_size, double maxres, double ele, t_gtime epo, RESIDTYPE resid_type);

        std::map<GSYS, double> _maxres_C;                    ///< code maximal residual
        std::map<GSYS, double> _maxres_L;                    ///< phase maximal residual
        double _maxres_norm;                            ///< normal maximal residual
        std::shared_ptr<t_gobj> _grec;                       ///< grec
        TROPMODEL _trpModStr;                           ///< trop mod(str)
        ZTDMPFUNC _tropo_mf;                            ///< trop mf
        RESIDTYPE _resid_type;                          ///< residual type
        OBSCOMBIN _observ;                              ///< observation
        CBIASCHAR _cbiaschar;                           ///< bias(char)
        std::map<GSYS, std::map<FREQ_SEQ, GOBSBAND>> _band_index; ///< band
        std::map<GSYS, std::map<GOBSBAND, FREQ_SEQ>> _freq_index; ///< freq
        std::vector<GSAT> _outlier_sat;                      ///< outlier satellite
    };

} // namespace gnut

#endif //  GSPPMODEL_H
