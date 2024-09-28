/**
  *
  * @verbatim
    History
    -1.0    Jiande Huang : Optimizing the efficiency of "get" function
  *
  @endverbatim
  * Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  *
  * @file     gallbias.h
  * @brief    container for all biases
  *
  * @author   JD
  * @version  1.0.0
  * @date     2012-11-05
  *
  */

#ifndef GALLBIAS_H
#define GALLBIAS_H

#include "gdata/gdata.h"
#include "gmodels/gbias.h"
#include "gutils/gtime.h"

using namespace std;

namespace gnut
{
    typedef shared_ptr<t_gbias> t_spt_bias;

    /**
     *@brief Class for bias setting derive from t_gdata
     */
    class LibGnut_LIBRARY_EXPORT t_gallbias : public t_gdata
    {
        typedef map<GOBS, t_spt_bias> t_map_gobs;     ///< first : GNSS Observations, second : bias
        typedef map<string, t_map_gobs> t_map_sat;    ///< first : sat name, second : observations
        typedef map<t_gtime, t_map_sat> t_map_epo;    ///< first : time, second : sat data
        typedef map<string, t_map_epo> t_map_ac;      ///< first : ac name, second : epoch

    public:
        /** @brief default constructor. */
        explicit t_gallbias();

        /** @brief default constructor. */
        explicit t_gallbias(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gallbias();

        /**
        * @brief set single bias element value.
        *
        * @param[in]  ac        ac name of the data
        * @param[in]  epo       epoch of the data
        * @param[in]  obj        object of the data
        * @param[in]  pt_bias    pt bias of the data
        * @return void
        */
        void add(const string &ac, const t_gtime &epo, const string &obj, t_spt_bias pt_bias);

        /**
        * @brief get DCB.
        *
        * @param[in]  epo       epoch of the data
        * @param[in]  obj        object of the data
        * @param[in]  gobs1        first observation of the data
        * @param[in]  gobs2        second observation bias of the data
        * @param[in]  ac            ac of the data
        * @return DCB
        */
        double get(const t_gtime &epo, const string &obj, const GOBS &gobs1, const GOBS &gobs2, const string &ac = "");

        /**
        * @brief get single bias element.
        *
        * @param[in]  prd       
        * @param[in]  epo       epoch of the data
        * @param[in]  obj        object of the data
        * @param[in]  gobs1        observation of the data
        * @param[in]  meter        unit
        * @return single bias
        */
        double get(const string &prd, const t_gtime &epo, const string &obj, const GOBS &gobs1, const bool &meter = true);

        /**
        * @brief get ac list.
        * @return ac list
        */
        vector<string> get_ac();

        /**
        * @brief set ac priority.
        * @return priority of ac
        */
        string get_ac_priority();

        /**
        * @brief get used av.
        * @return used ac
        */
        string get_used_ac();

    protected:
        /**
        * @brief get single bias element pointer.
        *
        * @param[in]  ac            ac of the data
        * @param[in]  epo       epoch of the data
        * @param[in]  obj        object of the data
        * @param[in]  gobs        observation of the data
        * @return    pt bias
        */
        t_spt_bias _find(const string &ac, const t_gtime &epo, const string &obj, const GOBS &gobs);

        /**
        * @brief get single bias element pointer.
        *
        * @param[in]  ac            ac of the data
        * @param[in]  epo       epoch of the data
        * @param[in]  obj        object of the data
        * @param[in]  ref        
        * @return    vec bias
        */
        vector<t_spt_bias> _find_ref(const string &ac, const t_gtime &epo, const string &obj, const GOBS &ref);

        /**
        * @brief convert type of observations.
        *
        * @param[in]  ac            ac of the data
        * @param[in]  obj        object of the data
        * @param[in]  obstype    observation type
        * @return    void
        */
        void _convert_obstype(const string &ac, const string &obj, GOBS &obstype);

        /**
        * @brief connect DCB pt_cb2 with first GOBS.
        *
        * @param[in]  pt_cb1    
        * @param[in]  pt_cb2
        * @return    void
        */
        void _connect_first(const t_spt_bias &pt_cb1, const t_spt_bias &pt_cb2);

        /**
        * @brief connect DCB pt_cb2 with second GOBS.
        *
        * @param[in]  pt_cb1
        * @param[in]  pt_cb2
        * @return    void
        */
        void _connect_second(const t_spt_bias &pt_cb1, const t_spt_bias &pt_cb2);

        /**
        * @brief consolidate all biases with reference signal of pt_cb2.
        *
        * @param[in]  ac            ac of the data
        * @param[in]  obj        object of the data
        * @param[in]  pt_cb1
        * @param[in]  pt_cb2
        * @return    void
        */
        void _consolidate(const string &ac, const string &obj, const t_spt_bias &pt_cb1, const t_spt_bias &pt_cb2);

    protected:
        string _acUsed;            ///<
        string _acPri = "DLR_R";   ///< primary AC
        map<string, int> _acOrder; ///< map of all ACs
        int _udbiaInt = 99999;     ///< flag of undifferentiated bia
        bool _isOverWrite = false; ///< flag of overwrite
        bool _isOrdered = false;   ///< if AC is ordered
        t_map_ac _mapBias;         ///< map of all satellite biases (all ACs & all period & all objects)
    };

} // namespace

#endif
