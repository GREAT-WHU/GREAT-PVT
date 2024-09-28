
/**
*
* @verbatim
    History
    2014-11-27  PV: created
    2019-03-25  ZHJ:add new data and compute

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gmodel.h
* @brief      Purpose: base abstract class for models
*.
* @author     PV
* @version    1.0.0
* @date       2014-11-27
*
*/

#ifndef GMODEL_H
#define GMODEL_H

#include <string>
#include <map>
#include <cmath>

#include "gall/gallpar.h"
#include "gutils/gtime.h"
#include "gdata/gsatdata.h"
#include "gset/gsetbase.h"
#include "gmodels/gtropo.h"
#include "gall/gallbias.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_gmodel. */
    class LibGnut_LIBRARY_EXPORT t_gmodel
    {
    public:
        t_gmodel();
        t_gmodel(t_spdlog spdlog);
        /** @brief default destructor. */
        virtual ~t_gmodel();

        /** @brief set site. */
        void setSite(const string &site);

        /** @brief set log. */
        void spdlog(t_spdlog spdlog);

        /** @brief model computed range value (phase/code). */
        virtual double cmpObs(t_gtime &epoch, t_gallpar &param, t_gsatdata &, t_gobs &gobs, bool = false) = 0;

        /** @brief model computed D range value (phase/code). */
        virtual double cmpObsD(t_gtime &epoch, t_gallpar &param, t_gsatdata &gsatdata, t_gobs &gobs) = 0;

        /** @brief get WindUp. */
        virtual double windUp(t_gtime &epoch, const string, const ColumnVector &, const ColumnVector &) { return 0.0; };

        /** @brief get trop Delay. */
        virtual double tropoDelay(t_gtime &epoch, t_gallpar &param, t_gtriple ell, t_gsatdata &satdata) { return 0.0; };

        /** @brief get ZHD. */
        virtual double getZHD(const string &site, const t_gtime &epo) { return 0.0; };

        /** @brief get ZWD. */
        virtual double getZWD(const string &site, const t_gtime &epo) { return 0.0; };

        /** @brief reset observation. */
        virtual void reset_observ(OBSCOMBIN obs){};
        /** @brief reset sat PCO. */
        virtual void reset_SatPCO(bool cal = true){}; 

        /** @brief Outliers detection. */
        virtual int outlierDetect(vector<t_gsatdata> &data, SymmetricMatrix &Qx, const SymmetricMatrix &) = 0;

        /** @brief get tropoModel. */
        shared_ptr<t_gtropo> tropoModel() { return _tropoModel; }

        /** @brief set allbias. */
        void setBIAS(t_gallbias *bia) { _gallbias = bia; }

    protected:
        shared_ptr<t_gtropo> _tropoModel; ///< trop model
        t_gsetbase *_settings;            ///< setting
        string _site;                     ///< site
        t_spdlog _spdlog;                 /// spdlog ptr
        bool _phase;                      ///< phase
        t_gallbias *_gallbias;            ///< allbias
    };

}

#endif //  GMODEL_H
