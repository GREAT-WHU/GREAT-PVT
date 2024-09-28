/**
 * @file         gsppbias.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        mainly about spp bias
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef gsppbias_h
#define gsppbias_h

#include "gexport/ExportLibGREAT.h"
#include "gmodels/gbiasmodel.h"

namespace great
{
    /** @brief model of bias.*/
    class LibGREAT_LIBRARY_EXPORT t_gsppbias : public t_gbiasmodel
    {
    public:
        /** @brief constructor. 
        *
        *param[in] spdlog                set spdlog control
        *param[in] settings            set base control
        */
        t_gsppbias(t_spdlog spdlog, t_gsetbase *settings);

        /** @brief default destructor. */
        virtual ~t_gsppbias();

        /**
        * @brief combine equation
        * @param [in]  epoch       epoch
        * @param [in]  params      parameters
        * @param [in]  obs           observation data
        * @param [out] result      B P L matrix
        * @return
               *    @retval   false     combine equation unsuccessfully
               *    @retval   true      combine equation successfully
        */
        bool cmb_equ(t_gtime &epoch, t_gallpar &params, t_gsatdata &obsdata, t_gobs &gobs, t_gbaseEquation &result) override;

        /**
         * @brief calculate rec/sat clk for epoch
         * @param [in]  obj         object
         * @param [in]  epo         epoch
         * @param [out] obj_clk     clk of object
         * @return
                *    @retval   false     no rec or sat clk
                *    @retval   true
         */
        void update_obj_clk(const string &obj, const t_gtime &epo, double clk) override;

        /**
         * @brief get satellite clk correction
         * @param [in]  sat_epoch   satellite epoch
         * @param [in]  sat         satellite
         * @param [in]  clk         satellite clk
         * @param [out]  dclk       satellite clk diff
         * @return
                *    @retval   true
         */
        double get_rec_clk(const string &obj) override; //add by xiongyun

    protected:
        /**
         * @brief calculate delay of tropo
         * @param [in]  epoch        current epoch
         * @param [in]  rec         satellite
         * @param [in]  param       parameter
         * @param [in]  site_ell    position of satellite
         * @param [in]  satdata     satellite data
         * @return double            delay of tropo
         */
        double tropoDelay(t_gtime &epoch, string &rec, t_gallpar &param, t_gtriple site_ell, t_gsatdata &satdata);

        /**
         * @brief calculate delay of isb
         * @param [in]  rec         reciever
         * @param [in]  param       parameter
         * @param [in]  sat            satellite
         * @param [in]  gobs        observation data
         * @return double            delay of isb
         */
        double isbDelay(t_gallpar &param, string &sat, string &rec, t_gobs &gobs);

        /**
         * @brief calculate delay of ifb
         * @param [in]  rec         reciever
         * @param [in]  param       parameter
         * @param [in]  sat            satellite
         * @param [in]  gobs        observation data
         * @return double            delay of ifb
         */
        double ifbDelay(t_gallpar &param, string &sat, string &rec, t_gobs &gobs);

        /**
         * @brief calculate rec/sat clk for epoch
         * @param [in]  epoch       epoch
         * @param [in]  sat         satellite
         * @param [in]  rec         receiver
         * @param [in]  param       all parameters
         * @param [out] gsatdata    data of satellite
         * @param [out] gobs        observation
         * @return      calculate theoretical value
         */
        double cmpObs(t_gtime &epoch, string &sat, string &rec, t_gallpar &param, t_gsatdata &gsatdata, t_gobs &gobs) override;

        map<GSYS, map<FREQ_SEQ, GOBSBAND>> _band_index; ///< index of band
        map<GSYS, map<GOBSBAND, FREQ_SEQ>> _freq_index; ///< index of frequency

        string _crt_rec; ///< current recievers
        string _crt_sat; ///< current satellite
        GSYS _crt_sys;   ///< current system
    };
}

#endif