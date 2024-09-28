/**
*
* @verbatim
    History
*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     gallpar.h
* @brief    Purpose: parametres container
*
* @author   PV
* @version  1.0.0
* @date     2011-04-18
*
*/

#ifndef GALLPAR_H
#define GALLPAR_H

#include "gexport/ExportLibGnut.h"
#include <string>
#include <set>

#include "gdata/gsatdata.h"
#include "gutils/gtriple.h"
#include "gmodels/gpar.h"
#include <unordered_map>
#include <algorithm>

using namespace std;

namespace gnut
{
    /**
     *@brief Class for t_gallpar
     */
    class LibGnut_LIBRARY_EXPORT t_gallpar
    {
    public:
        /**
        *@brief add parameter
        */
        void addParam(const t_gpar &par);

        /**
        *@brief delete parameter
        */
        void delParam(const int &i);

        /**
        *@brief delete all parameter
        */
        void delAllParam();

        /**
        *@brief del amb
        */
        vector<int> delAmb();

        /**
        *@brief get parameter
        */
        int getParam(const string &site, const par_type &type, const string &prn,
                     const t_gtime &beg = FIRST_TIME, const t_gtime &end = LAST_TIME) const;

        /**
         * @brief Get the Param object
         * 
         * @param index 
         * @return int 
         */
        int getParam(const int &index);

        /**
        *@brief get par index
        */
        int getParIndex(const int &idx);

        /**
        *@brief get par value
        */
        double getParValue(const int &idx);

        /**
        *@brief set par value
        */
        void setParValue(const int &idx, const double &value);

        /**
        *@brief get par/amb/orbPar Number
        */
        unsigned int parNumber() const;

        /**
         * @brief 
         * 
         * @return unsigned int 
         */
        unsigned int orbParNumber() const;

        /**
        *@brief get Crd Param
        */
        int getCrdParam(const string &site, t_gtriple &crd,
                        const t_gtime &beg = FIRST_TIME, const t_gtime &end = LAST_TIME) const;

        /**
        *@brief get Vel Param
        */
        int getVelParam(const string &site, t_gtriple &crd,
                        const t_gtime &beg = FIRST_TIME, const t_gtime &end = LAST_TIME) const;

        /**
        *@brief get partial Index
        */
        vector<int> getPartialIndex(const string &site, const string &sat);

        /**
         * @brief Get the Par object
         * 
         * @param idx 
         * @return const t_gpar& 
         */
        const t_gpar &getPar(const int &idx) const;

        /**
        *@brief override operator
        */
        t_gpar &operator[](const size_t idx);

        /**
         * @brief 
         * 
         * @param par 
         * @return t_gallpar 
         */
        t_gallpar operator-(const t_gallpar &par);

        /**
         * @brief 
         * 
         * @param par 
         * @return t_gallpar 
         */
        t_gallpar operator+(const t_gallpar &par);

        /**
         * @brief 
         * 
         */
        void reIndex();

        /**
         * @brief Set the Site object
         * 
         * @param site 
         */
        void setSite(const string &site);

        /**
         * @brief 
         * 
         * @return set<string> 
         */
        set<string> amb_prns();

        /**
         * @brief 
         * 
         * @param X1 
         * @param X2 
         * @return int 
         */
        int sum(t_gallpar &X1, t_gallpar &X2);

        /**
         * @brief Get the All Par object
         * 
         * @return vector<t_gpar> 
         */
        vector<t_gpar> getAllPar();

        /**
         * @brief 
         * 
         * @param os 
         * @param x 
         * @return ostream& 
         */
        friend ostream &operator<<(ostream &os, t_gallpar &x);

        /**
         * @brief 
         * 
         * @param freq 
         * @return map<string, int> 
         */
        map<string, int> freq_sats_num(const int &freq);

    private:
        vector<t_gpar> _vParam;
        map<string, vector<t_gpar>> _vOrbParam; ///< add for orb param
        map<t_gparhead, map<t_gtimearc, long>> _index_par; /// index -> point
        long _max_point = 0;
        vector<long> _point_par; /// point -> vParam
        map<pair<string, string>, vector<int>> _index_for_parital; /// index -> param
        pair<long, int> _last_point;

        /** @brief update partial index. */
        void _update_partial_index();

        t_gmutex _allpar_mtx;
    };

} // namespace

#endif
