
/**
*
* @verbatim
    History
    2011-03-25  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gprod.h
* @brief      product base class
*.
* @author     JD
* @version    1.0.0
* @date       2011-03-25
*
*/

#ifndef GPROD_H
#define GPROD_H

#include <set>
#include <iostream>

#include "gdata/gdata.h"
#include "gdata/gobj.h"
#include "gutils/gcommon.h"

using namespace std;

namespace gnut
{

    static shared_ptr<t_gobj> nullobj;

    /** @brief class for t_gprod derive from t_gdata. */
    class LibGnut_LIBRARY_EXPORT t_gprod : public t_gdata
    {

    public:
        /** @brief constructor 1. */
        explicit t_gprod(const t_gtime &t, shared_ptr<t_gobj> pt = nullobj);

        t_gprod(t_spdlog spdlog, const t_gtime &t, shared_ptr<t_gobj> pt = nullobj);
        /** @brief default destructor. */
        virtual ~t_gprod();

        typedef map<string, pair<double, double>> t_map_prod; ///< map of prod

        /** @brief get/set epo. */
        t_gtime epoch() const { return _epo; }

        /** @brief get/set obj. */
        string obj_id() const
        {
            if (_obj)
                return _obj->id();
            else
                return "";
        }

        /** @brief get/set val. */
        int set_val(const string &str, const double &val, const double &rms = 0.0);

        /** @brief get/set nSat. */
        void nSat(const int &n) { _nSat = n; }

        /** @brief get/set nSat_excl. */
        void nSat_excl(const int &n) { _nSat_excl = n; }

    protected:
        t_gtime _epo;                      ///< epo
        shared_ptr<t_gobj> _obj;           ///< object
        t_map_prod _prod;                  ///< prod
        t_map_prod::const_iterator itPROD; ///< it_prod

        int _nSat;      ///< number of sat
        int _nSat_excl; ///< number of sat excl

    private:
    };

} // namespace

#endif
