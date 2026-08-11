/**
 * @file         gqualitycontrol.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        main about quality control
 * @version      1.0
 * @date         2024-08-29
 *
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 *
 */
#ifndef QUALITYCONTROL_H
#define QUALITYCONTROL_H

#include "gexport/ExportLibGREAT.h"
#include "gall/gallnav.h"
#include "gall/gallobs.h"
#include "gdata/gsatdata.h"
#include "gset/gsetgen.h"
#include "gset/gsetrec.h"
#include "gset/gsetgnss.h"
#include "gset/gsetproc.h"

using namespace gnut;
namespace great
{
    enum class SMOOTH_MODEL
    {
        SMT_DOPPLER,
        SMT_PHASE,
        SMT_NONE
    };

    /**
     * @brief Class for smooth
     */
    class LibGREAT_LIBRARY_EXPORT t_gsmooth
    {
    public:
        /**
         * @brief Construct a new t gsmooth object
         * @param[in]  settings  setbase control
         */
        t_gsmooth(t_gsetbase* settings);
        /**
         * @brief Destroy the t gsmooth object
         */
        virtual ~t_gsmooth() {};

    private:
        int _smoothWindow;          ///< window of smooth model
        double _sampling;           ///< sampling
        RECEIVERTYPE _receiverType; ///< type of receiver

        std::map<GSYS, std::map<FREQ_SEQ, GOBSBAND>> _band_index;                  ///< index band
    };

    /**
     * @brief class for BeiDou satellite-induced code pseudorange variations correct
     */
    class LibGREAT_LIBRARY_EXPORT t_gbds_codebias_cor
    {
    public:
        /**
         * @brief Construct a new t gbds codebias cor object
         * @param[in]  settings  setbase control
         */
        t_gbds_codebias_cor(t_gsetbase* settings);
        /**
         * @brief Destroy the t gbds codebias cor object
         */
        virtual ~t_gbds_codebias_cor() {};
        /**
         * @brief apply IGSO_MEO satellite coordinate and velocity
         * @param [in]  rec               station name
         * @param [in]  rec_crd           station coordinate
         * @param [in]  gnav               all navigation
         * @param [in]  obsdata           all observation
         * @return
         *    @retval   false     unsuccessfully apply IGSO
         *    @retval   true      successfully apply MEO
         */
        void apply_IGSO_MEO(const std::string& rec, t_gtriple& rec_crd, t_gallnav* gnav, std::vector<t_gsatdata>& obsdata);

    private:
        t_gsetbase* _set;                                            ///< setbase control
        std::map<GSYS, std::map<FREQ_SEQ, GOBSBAND>> _band_index;              ///< index of band
        bool _correct_bds_code_bias;                                 ///< BEIDOU correct bias
        std::map<GOBSBAND, std::map<std::string, std::map<int, double>>> _IGSO_MEO_Corr; ///< IGSO MEO Correction

        /**
         * @brief Approximate location of the receiver
         */
        bool _recAprCoordinate(const std::string& rec, t_gtriple& rec_crd, t_gallnav* gnav, std::vector<t_gsatdata>& obsdata);
    };

    /**
     * @brief Class for outliers process
     */
    class LibGREAT_LIBRARY_EXPORT t_goutliers_process
    {
    public:
        /**
         * @brief Construct a new t goutliers process object
         * @param[in]  settings  setbase control
         * @param[in]  spdlog      logbase control
         */
        t_goutliers_process(t_gsetbase* settings);

        /**
         * @brief Construct a new t goutliers process object
         * @param[in]  settings  setbase control
         * @param[in]  spdlog      logbase control
         */
        /**
         * @brief Destroy the t goutliers process object
         */
        virtual ~t_goutliers_process();

        /**
         * @brief move bad observation
         * @param[in]  obsdata   all observation data
         */
        void excludeBadObs(std::vector<t_gsatdata>& obsdata);

    private:
        t_gsetbase* _set;                               ///< setbase control
        t_giof* _debug_outliers = nullptr;              ///< debug outliers
        int _frequency;                                 ///< frequency
        OBSCOMBIN _observ;                              ///< type of obsevation
        std::map<GSYS, std::map<GOBSBAND, FREQ_SEQ>> _freq_index; ///< frequncy of type
        std::set<OBSCOMBIN> _single_mix;                     ///< TODO
        RECEIVERTYPE _receiverType;                     ///< type of receiver
    };

    /**
     * @brief Class for qualitycontrol
     */
    class LibGREAT_LIBRARY_EXPORT t_gqualitycontrol
    {
    public:
        /**
         * @brief Construct a new t gqualitycontrol object
         * @param[in]  settings  setbase control
         * @param[in]  gnav      navigation data
         */
        t_gqualitycontrol(t_gsetbase* settings, t_gallnav* gnav);

        /**
         * @brief Destroy the t gqualitycontrol object
         */
        virtual ~t_gqualitycontrol();

        /**
         * @brief process one epoch
         * @param[in]  now       current time
         * @param[in]  rec       station name
         * @param[in]  rec_crd   station coodinate
         * @param[in]  obsdata   observation data
         */
        int processOneEpoch(const t_gtime& now, const std::string& rec, t_gtriple& rec_crd, std::vector<t_gsatdata>& obsdata);

        /**
         * @brief Set the Nav data
         * @param[in]  gnav      navigation data
         */
        void setNav(t_gallnav* gnav)
        {
            _gnav = gnav;
        };

    protected:
        t_gsetbase* _set = nullptr; ///< setbase control
        t_gallnav* _gnav = nullptr; ///< navigation daya

        t_gsmooth _smooth_range;               ///< smooth range
        t_gbds_codebias_cor _bds_codebias_cor; ///< bds codebias correction
        t_goutliers_process _outliers_proc;    ///< outliers process
    };

} // namespace great

#endif
