/**
*
* @verbatim
    History
    2011-09-04  JD: created
    2012-09-24  JD: selections of code, phase etc.
    2020-10-11  Jiande Huang : Optimizing the efficiency of "select" function

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file      gobsgnss.h
* @brief     Purpose: implementation of GNSS observation element
*            Todo: multi-GNSS & various LCs !
*            gobj implementation
*.
* @author     JD
* @version    1.0.0
* @date       2011-09-04
*
*/

#ifndef GOBSGNSS_H
#define GOBSGNSS_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

#include "gdata/gdata.h"
#include "gutils/gnss.h"
#include "gutils/gobs.h"
#include "gutils/gtime.h"
#include "gutils/gsys.h"

#define DEF_CHANNEL 255

using namespace std;

namespace gnut
{

    const static double NULL_GOBS = 0.0;

    ///< priority tables for choice of available signals (code [m])
    const static GOBS code_choise[9][19] = {
        {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X},                               //
        {C1A, C1B, C1C, X, C1I, C1L, C1M, X, C1P, C1Q, C1S, C1W, C1X, C1Y, C1Z, P1, C1, CA, CB}, //  C1
        {X, X, C2C, C2D, C2I, C2L, C2M, X, C2P, C2Q, C2S, C2W, C2X, C2Y, X, P2, C2, CC, CD},     //  C2
        {X, X, X, X, C3I, X, X, X, X, C3Q, X, X, C3X, X, X, X, X, X, X},                         //
        {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X},                               //
        {X, X, X, X, C5I, X, X, X, X, C5Q, X, X, C5X, X, X, P5, C5, X, X},                       //  C5
        {C6A, C6B, C6C, X, C6I, X, X, X, X, C6Q, X, X, C6X, X, C6Z, X, C6, X, X},                //  C6
        {X, X, X, X, C7I, X, X, X, X, C7Q, X, X, C7X, X, X, X, C7, X, X},                        //  C7
        {X, X, X, X, C8I, X, X, X, X, C8Q, X, X, C8X, X, X, X, C8, X, X}                         //  C8
    };

    ///< priority tables for choice of available signals (phase [full-cycles])
    const static GOBS phase_choise[9][19] = {
        {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X},                                //
        {L1A, L1B, L1C, X, L1I, L1L, L1M, L1N, L1P, L1Q, L1S, L1W, L1X, L1Y, L1Z, X, L1, LA, LB}, //  L1
        {X, X, L2C, L2D, L2I, L2L, L2M, L2N, L2P, L2Q, L2S, L2W, L2X, L2Y, X, X, L2, LC, LD},     //  L2
        {X, X, X, X, L3I, X, X, X, X, L3Q, X, X, L3X, X, X, X, X, X, X},                          //
        {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X},                                //
        {X, X, X, X, L5I, X, X, X, X, L5Q, X, X, L5X, X, X, X, L5, X, X},                         //  L5
        {L6A, L6B, L6C, X, L6I, X, X, X, X, L6Q, X, X, L6X, X, L6Z, X, L6, X, X},                 //  L6
        {X, X, X, X, L7I, X, X, X, X, L7Q, X, X, L7X, X, X, X, L7, X, X},                         //  L7
        {X, X, X, X, L8I, X, X, X, X, L8Q, X, X, L8X, X, X, X, L8, X, X}                          //  L8
    };

    const static map<GSYS, map<GOBSBAND, string>> range_order_attr_raw = {
        {GPS, {{BAND_1, "CPW"}, {BAND_2, "CLXPW"}, {BAND_5, "QX"}}},
        {GAL, {{BAND_1, "CX"}, {BAND_5, "IQX"}, {BAND_7, "IQX"}, {BAND_8, "IQX"}, {BAND_6, "ABCXZ"}}},
        {BDS, {{BAND_2, "IQX"}, {BAND_7, "IQX"}, {BAND_6, "IQX"}, {BAND_5, "DPX"}, {BAND_9, "DPZ"}, {BAND_8, "DPX"}, {BAND_1, "DPX"}}},
        {GLO, {{BAND_1, "CP"}, {BAND_2, "CP"}}},
        {QZS, {{BAND_1, "CSLX"}, {BAND_2, "LX"}, {BAND_5, "IQX"}}}};

    const static map<GSYS, map<GOBSBAND, string>> range_order_attr_cmb = {
        {GPS, {{BAND_1, "CPW"}, {BAND_2, "CLXPW"}, {BAND_5, "QX"}}},
        {GAL, {{BAND_1, "CX"}, {BAND_5, "IQX"}, {BAND_7, "IQX"}, {BAND_8, "IQX"}, {BAND_6, "ABCXZ"}}},
        {BDS, {{BAND_2, "IQX"}, {BAND_7, "IQX"}, {BAND_6, "IQX"}, {BAND_5, "DPX"}, {BAND_9, "DPZ"}, {BAND_8, "DPX"}, {BAND_1, "DPX"}}},
        {GLO, {{BAND_1, "CP"}, {BAND_2, "CP"}}},
        {QZS, {{BAND_1, "CSLX"}, {BAND_2, "LX"}, {BAND_5, "IQX"}}}};

    const static map<GSYS, map<GOBSBAND, string>> phase_order_attr_raw = {
        {GPS, {{BAND_1, "CSLXPWYM"}, {BAND_2, "CDLXPWYM"}, {BAND_5, "IQX"}}},
        {GAL, {{BAND_1, "ABCXZ"}, {BAND_5, "IQX"}, {BAND_7, "IQX"}, {BAND_8, "IQX"}, {BAND_6, "ABCXZ"}}},
        //LX changed for BDS3
        {BDS, {{BAND_2, "XIQ"}, {BAND_7, "IQX"}, {BAND_6, "IQX"}, {BAND_5, "DPX"}, {BAND_9, "DPZ"}, {BAND_8, "DPX"}, {BAND_1, "DPX"}}},
        {GLO, {{BAND_1, "PC"}, {BAND_2, "CP"}}},                   // fix bugs change Band_1 CP->PC  glfeng
        {QZS, {{BAND_1, "CSLX"}, {BAND_2, "LX"}, {BAND_5, "IQX"}}} //glfeng
    };

    const static map<GSYS, map<GOBSBAND, string>> phase_order_attr_cmb =
        {
            {GPS, {{BAND_1, "CSLXPWYM"}, {BAND_2, "CDLXPWYM"}, {BAND_5, "IQX"}}},
            {GAL, {{BAND_1, "ABCXZ"}, {BAND_5, "IQX"}, {BAND_7, "IQX"}, {BAND_8, "IQX"}, {BAND_6, "ABCXZ"}}},
            {BDS, {{BAND_2, "QXI"}, {BAND_7, "IQX"}, {BAND_6, "IQX"}, {BAND_5, "DPX"}, {BAND_9, "DPZ"}, {BAND_8, "DPX"}, {BAND_1, "DPX"}}},
            {GLO, {{BAND_1, "PC"}, {BAND_2, "CP"}}},                   // fix bugs change Band_1 CP->PC  glfeng
            {QZS, {{BAND_1, "CSLX"}, {BAND_2, "LX"}, {BAND_5, "IQX"}}} //glfeng

    };

    class t_obscmb;
    class t_gallbias;

    /** @brief class for t_gobsgnss derive from t_gdata. */
    class LibGnut_LIBRARY_EXPORT t_gobsgnss : public t_gdata
    {

    public:
        /** @brief default constructor. */
        t_gobsgnss();
        /**
         * @brief Construct a new t gobsgnss object
         * 
         * @param spdlog 
         */
        t_gobsgnss(t_spdlog spdlog);
        /** @brief constructor 1. */
        t_gobsgnss(t_spdlog spdlog, const string &sat);

        /** @brief constructor 2. */
        t_gobsgnss(t_spdlog spdlog, const string &site, const string &sat, const t_gtime &t);

        /** @brief default destructor. */
        virtual ~t_gobsgnss();

        /** @brief add a single observation. */
        void addobs(const GOBS &obs, const double &d);

        /** @brief add a lost-of-lock indicator. */
        void addlli(const GOBS &obs, const int &i);

        /** @brief add an estimated cycle slip. */
        void addslip(const GOBS &obs, const int &i);

        /** @brief approximate elevation. */
        void addele(const double &d);

        /** @brief add range outliers. */
        void addoutliers(const GOBS &obs, const int &i); 

        /**
         * @brief set range state
         * 
         * @param name 
         * @param b 
         */
        void setrangestate(const string &name, const bool &b);

        /**
         * @brief get range state
         * 
         * @param name 
         * @return true 
         * @return false 
         */
        bool getrangestate(const string &name);

        /**
         * @brief set obs Level Flag
         * 
         * @param gobs 
         * @param flag 
         */
        void setobsLevelFlag(const GOBS &gobs, const int &flag);

        /**
         * @brief get obs Level Flag
         * 
         * @param gobs 
         * @return int 
         */
        int getobsLevelFlag(const GOBS &gobs);

        /**
         * @brief obs With Corr
         * 
         * @return true 
         * @return false 
         */
        bool obsWithCorr();

        /** @brief get number of observations. */
        size_t size() const;

        /** @brief get system-specific frequency  for band i. */
        double frequency(const GOBSBAND &b) const;

        /** @brief get system-specific frequency  for band i. */
        double frequency(const int &b) const;

        /** @brief get system-specific wavelength for band i. */
        double wavelength(const GOBSBAND &b) const;

        /** @brief get wavelength for iono-free LC. */
        double wavelength_L3(const GOBSBAND &b1 = BAND_1, const GOBSBAND &b2 = BAND_2) const;

        /** @brief get wavelength for wild lane. */
        double wavelength_WL(const GOBSBAND &b1 = BAND_1, const GOBSBAND &b2 = BAND_2) const;

        /** @brief get wavelength for narrow lane. */
        double wavelength_NL(const GOBSBAND &b1 = BAND_1, const GOBSBAND &b2 = BAND_2) const;

        /** @brief get GNSS system from satellite IDOC. */
        GSYS gsys() const;

        /** @brief gppflt NEW! */
        GOBS id_range(const GOBSBAND &b) const { return _id_range(b); };

        /**
         * @brief phase id
         * 
         * @param b 
         * @return GOBS 
         */
        GOBS id_phase(const GOBSBAND &b) const { return _id_phase(b); };

        /** @brief get code obs of selected band (Used the same as Panda). */
        GOBS select_range(const GOBSBAND &b, const bool &isRawAll = true) const;

        /** @brief get pahse obs of selected band (Used the same as Panda). */
        GOBS select_phase(const GOBSBAND &b, const bool &isRawAll = true) const;

        /** @brief get vector of available observations. */
        vector<GOBS> obs() const;

        /** @brief get set of available phase observations for a band. */
        set<GOBS> obs_phase(const int &band) const;

        /** @brief get a single observation (DIFFERENT UNITS!). */
        double getobs(const GOBS &obs) const;

        /** @brief get a single observation (DIFFERENT UNITS!). */
        double getobs(const string &obs) const;

        /** @brief get approximate elevation. */
        const double &getele() const;

        /** @brief get a lost-of-lock indicator. */
        int getlli(const GOBS &obs) const;

        /** @brief get an estimated cycle slip. */
        int getslip(const GOBS &obs) const;

        /** @brief get a lost-of-lock indicator. */
        int getlli(const string &obs) const;

        /** @brief get number of code/phase available bands. */
        void nbands(pair<int, int> &nb);

        /**
         * @brief get range outliers
         * 
         * @param obs 
         * @return int 
         */
        int getoutliers(const GOBS &obs) const; 

        /**
         * @brief reset OBS - code [m], phase [whole cycles], dopler[cycles/sec], snr [DBHZ], ...  observations (double)
         * 
         * @param obs 
         * @param v 
         */
        void resetobs(const GOBS &obs, const double &v); 

        /**
         * @brief erase OBS - code [m], phase [whole cycles], dopler[cycles/sec], snr [DBHZ], ...  observations (double)
         * 
         * @param obs 
         */
        void eraseobs(const GOBS &obs); 
        void eraseband(const GOBSBAND &b); 

        /**
         * @brief apply bias
         * 
         * @param allbias 
         * @return true 
         * @return false 
         */
        bool apply_bias(t_gallbias *allbias);

        /**
         * @brief apply code phase bias
         * 
         * @param allbias 
         * @return true 
         * @return false 
         */
        bool apply_code_phase_bias(t_gallbias *allbias);

        /**
         * @brief apply dcb
         * 
         * @param allbias 
         * @return true 
         * @return false 
         */
        bool apply_dcb(t_gallbias *allbias);

        /**
         * @brief inter Freq Dcb
         * 
         * @param allbias 
         * @param band1 
         * @param band2 
         * @return double 
         */
        double interFreqDcb(t_gallbias &allbias, const GOBSBAND &band1, const GOBSBAND &band2) const;

        /**
         * @brief reset dcb mark
         * 
         * @param mark 
         */
        void reset_dcbmark(const bool &mark) { _dcb_correct_mark = mark; }; 

        /** @brief set channel number for Glonass satellites. */
        void channel(const int &canal);

        /** @brief get channel number for Glonass satellites. */
        const int &channel() const;

        /** @brief get code observation [m] only requested type! */
        double obs_C(const t_gobs &gobs) const; 
        double obs_C(const GOBS &gobs) const;

        /** @brief get phase observation [m] only requested type! */
        double obs_L(const t_gobs &gobs) const; 
        double obs_L(const GOBS &gobs) const;

        /** @brief get doppler observation [m/s] only requested type! */
        double obs_D(const t_gobs &gobs) const;
        double obs_D(const GOBS &gobs) const;

        /** @brief get snr  observation [dbHz] only requested type!*/
        double obs_S(const t_gobs &gobs) const; 

        /** @brief get code observation [m] only requested type */
        double obs_C(const t_gband &gb) const; 

        /** @brief get phase observation [m] only requested type */
        double obs_L(const t_gband &gb) const; 

        /** @brief get doppler observation [m/s] only requested type */
        double obs_D(const t_gband &gb) const; 

        /** @brief get snr observation [dbHz] only requested type */
        double obs_S(const t_gband &gb) const; 

        /** @brief modify phase observation by adding dL: i = 0 [cycles], i = 1 [m] */
        int mod_L(const double &dL, const GOBS &gobs = X, const int i = 1);

        /** @brief modify phase observation by adding dL: i = 0 [cycles], i = 1 [m], i = 2 [set val to 0] */
        int mod_L(const double &dL, const GOBSBAND &band, const int i = 1);

        /**
         * @brief return value of carrier-phase linear commbination frequency
         * 
         * @param band1 
         * @param coef1 
         * @param band2 
         * @param coef2 
         * @param band3 
         * @param coef3 
         * @return double 
         */
        double frequency_lc(const int &band1,    // return value [Hz] of phase linear combination frequency (c1*O1 + ... )
                            const double &coef1, // for 2 or 3 bands with given coefficients
                            const int &band2,
                            const double &coef2,
                            const int &band3 = 0,
                            const double &coef3 = 0) const; // -->> OLD INTERFACE !!

        /**
         * @brief return value of phase linear combination ionospheric scale factor (c1*O1 + ... )
         * 
         * @param band1 
         * @param coef1 
         * @param band2 
         * @param coef2 
         * @return double 
         */
        double isf_lc(const int &band1,    
                      const double &coef1, 
                      const int &band2,
                      const double &coef2) const;

        /**
         * @brief return value of linear combination phase noise factor
         * 
         * @param band1 
         * @param coef1 
         * @param band2 
         * @param coef2 
         * @param band3 
         * @param coef3 
         * @return double 
         */
        double pnf_lc(const int &band1,    
                      const double &coef1, 
                      const int &band2,
                      const double &coef2,
                      const int &band3 = 0,
                      const double &coef3 = 0) const;

        /**
         * @brief get ionosphere-free combination for code  [m]
         * 
         * @param g1 
         * @param g2 
         * @return double 
         */
        double P3(const t_gobs &g1, const t_gobs &g2) const; 
        double P3(const GOBS &g1, const GOBS &g2) const;
        /**
         * @brief get geometry-free combination for code    [m]
         * 
         * @param g1 
         * @param g2 
         * @return double 
         */
        double P4(const t_gobs &g1, const t_gobs &g2) const;

        /**
         * @brief get ionosphere-free combination for phase [m]
         * 
         * @param g1 
         * @param g2 
         * @return double 
         */
        double L3(const t_gobs &g1, const t_gobs &g2) const; 
        double L3(const GOBS &g1, const GOBS &g2) const;
        /**
         * @brief get ionosphere-free combination for phase [CYCLE]
         * 
         * @param g1 
         * @param g2 
         * @return double 
         */
        double L3_cycle(const t_gobs &g1, const t_gobs &g2) const; 
        /**
         * @brief get geometry-free combination for phase   [m]
         * 
         * @param g1 
         * @param g2 
         * @return double 
         */
        double L4(const t_gobs &g1, const t_gobs &g2) const; 
        /**
         * @brief get geometry-free combination for phase   [cycle]
         * 
         * @param g1 
         * @param g2 
         * @return double 
         */
        double L4_cycle(const t_gobs &g1, const t_gobs &g2) const; 
        /**
         * @brief get multipath for C-obs + Li/Lj
         * 
         * @param code 
         * @param L1 
         * @param L2 
         * @return double 
         */
        double MP(const t_gobs &code,
                  const t_gobs &L1, const t_gobs &L2) const; 
        /**
         * @brief GFIF meter
         * 
         * @param gL1 
         * @param gL2 
         * @param gL3 
         * @return double 
         */
        double GFIF_meter(const t_gobs &gL1, const t_gobs &gL2, const t_gobs &gL3) const;
        /**
         * @brief get wide-lane combination for code [m]
         * 
         * @param g1 
         * @param g2 
         * @return double 
         */
        double PWL(const t_gobs &g1, const t_gobs &g2) const;

        /**
         * @brief get wide-lane combination for phase [m]!
         * 
         * @param g1 
         * @param g2 
         * @return double 
         */
        double LWL(const t_gobs &g1, const t_gobs &g2) const; 

        /**
         * @brief get narrow-lane combination for phase [m]!
         * 
         * @param g1 
         * @param g2 
         * @return double 
         */
        double LNL(const t_gobs &g1, const t_gobs &g2) const; 

        /**
         * @brief get Melbourne-Wuebenna combination for phase & code [m]!
         * 
         * @param g1 
         * @param g2 
         * @param phacode_consistent 
         * @return double 
         */
        double MW(const t_gobs &g1, const t_gobs &g2,     
                  bool phacode_consistent = false) const; 

        /**
         * @brief get Melbourne-Wuebenna combination for phase & code [cycle]!
         * 
         * @param gL1 
         * @param gL2 
         * @param gC1 
         * @param gC2 
         * @return double 
         */
        double MW_cycle(const t_gobs &gL1, const t_gobs &gL2,
                        const t_gobs &gC1, const t_gobs &gC2) const;

        /**
         * @brief get extra-wide-lane Melbourne-Wuebenna combination for phase & code [cycle]
         * 
         * @param gL1 
         * @param gL2 
         * @param gL3 
         * @param gC1 
         * @param gC2 
         * @return double 
         */
        double EWL_cycle(const t_gobs &gL1, const t_gobs &gL2, const t_gobs &gL3,
                         const t_gobs &gC1, const t_gobs &gC2) const; 

        /**
         * @brief get LW combination for phase [meter]
         * 
         * @param gL1 
         * @param gL2 
         * @return double 
         */
        double LW_meter(const t_gobs &gL1, const t_gobs &gL2) const; 

        /**
         * @brief get LE combination for phase [meter]
         * 
         * @param gL2 
         * @param gL3 
         * @return double 
         */
        double LE_meter(const t_gobs &gL2, const t_gobs &gL3) const; 

        /**
         * @brief get LWL combination factor
         * 
         * @param gL1 
         * @param gL3 
         * @return double 
         */
        double LWL_factor13(const t_gobs &gL1, const t_gobs &gL3) const; 

        /**
         * @brief get reference epoch
         * 
         * @return t_gtime 
         */
        const t_gtime &epoch() const { return _epoch; } 
        /**
         * @brief get station id
         * 
         * @return string 
         */
        const string &site() const { return _staid; }
        /**
         * @brief get satellite id
         * 
         * @return string 
         */
        const string &sat() const { return _satid; } 
        /**
         * @brief get satellite system id
         * 
         * @return string 
         */
        string sys() const { return _satid.substr(0, 1); }
        /**
         * @brief get satellite lost-of-lock identifications
         * 
         * @return map<GOBS, int> 
         */
        const map<GOBS, int> &lli() const { return _glli; } 
        /**
         * @brief get maps of estimated cycle slips
         * 
         * @return map<GOBS, int> 
         */
        const map<GOBS, int> &slip() const { return _gslip; } 

        /**
         * @brief set satellite id
         * 
         * @param id 
         */
        void sat(const string &id)
        {
            _satid = id;
            _gsys = t_gsys::char2gsys(id[0]);
        } 

        /**
         * @brief set site id
         * 
         * @param id 
         */
        void site(const string &id) { _staid = id; }

        /**
         * @brief set epoch
         * 
         * @param t 
         */
        void epo(const t_gtime &t) { _epoch = t; } 

        /**
         * @brief clear
         * 
         */
        void clear();

        /**
         * @brief valid
         * 
         * @return true 
         * @return false 
         */
        bool valid() const;

        /**
         * @brief obs empty
         * 
         * @return true 
         * @return false 
         */
        bool obs_empty() const;

        /**
         * @brief get available freq 
         * 
         * @return set<GFRQ> 
         */
        set<GFRQ> freq_avail() const; 

        /**
         * @brief get available band
         * 
         * @param _phase 
         * @return set<GOBSBAND> 
         */
        set<GOBSBAND> band_avail(bool _phase = true) const; 

        /**
         * @brief 
         * 
         * @param freq 
         * @return true 
         * @return false 
         */
        bool contain_freq(const FREQ_SEQ &freq) const;

        /**
         * @brief get ionosphere-free combination for code  [m]
         * 
         * @param b1 
         * @param b2 
         * @return double 
         */
        double P3(const GOBSBAND &b1 = BAND_1, const GOBSBAND &b2 = BAND_2) const; 

        /**
         * @brief get geometry-free combination for code    [m]
         * 
         * @param b1 
         * @param b2 
         * @return double 
         */
        double P4(const GOBSBAND &b1 = BAND_1, const GOBSBAND &b2 = BAND_2) const; 

        /**
         * @brief get ionosphere-free combination for phase [m]
         * 
         * @param b1 
         * @param b2 
         * @return double 
         */
        double L3(const GOBSBAND &b1 = BAND_1, const GOBSBAND &b2 = BAND_2) const; 

        /**
         * @brief get geometry-free combination for phase   [m]
         * 
         * @param b1 
         * @param b2 
         * @return double 
         */
        double L4(const GOBSBAND &b1 = BAND_1, const GOBSBAND &b2 = BAND_2) const; 

        /**
         * @brief return coefficients (c1,c2) of the ionosphere-free linear combination
         * 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int coef_ionofree(const GOBSBAND &b1, double &c1,        
                          const GOBSBAND &b2, double &c2) const; 

        /**
         * @brief return coefficients for carrier phase observtion
         * 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int coef_ionofree_phi(const GOBSBAND &b1, double &c1,
                              const GOBSBAND &b2, double &c2) const; 

        /**
         * @brief return coefficients (c1,c2) of the geometry-free linear combination
         * 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int coef_geomfree(const GOBSBAND &b1, double &c1,        
                          const GOBSBAND &b2, double &c2) const; 

        /**
         * @brief return coefficients (c1,c2) of the narrow-lane linear combination
         * 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int coef_narrlane(const GOBSBAND &b1, double &c1,       
                          const GOBSBAND &b2, double &c2) const; 

        /**
         * @brief return coefficients (c1,c2) of the wide-lane linear combination
         * 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int coef_widelane(const GOBSBAND &b1, double &c1,        
                          const GOBSBAND &b2, double &c2) const; 

        /**
         * @brief get sat health
         * 
         * @return true 
         * @return false 
         */
        const bool &health() const { return _health; } 

        /**
         * @brief set sat health
         * 
         * @param health 
         */
        void health(bool health) { _health = health; } 

    protected:
        /**
         * @brief get  code observation [m] only requested type NEW + AUTO!
         * 
         * @param go 
         * @return double 
         */
        double _obs_range(const t_gobs &go) const; 

        /**
         * @brief get  code observation [m] only requested type NEW + AUTO!
         * 
         * @param gb 
         * @return double 
         */
        double _obs_range(const t_gband &gb) const;

        /**
         * @brief get phase observation [m] only requested type NEW + AUTO !
         * 
         * @param gb 
         * @return double 
         */
        double _obs_phase(const t_gband &gb) const; 

        /**
         * @brief get doppler observation [m] only requested type NEW + AUTO !
         * 
         * @param gb 
         * @return double 
         */
        double _obs_doppler(const t_gband &gb) const;

        /**
         * @brief get snr observation [dbhz] only requested type NEW + AUTO !
         * 
         * @param gb 
         * @return double 
         */
        double _obs_snr(const t_gband &gb) const;

        /**
         * @brief return value [m] of general pseudo-range
         * 
         * @param g1 
         * @param cf1 
         * @param g2 
         * @param cf2 
         * @param g3 
         * @param cf3 
         * @return double 
         */
        double _lcf_range(const t_gobs *g1, const int &cf1,                
                          const t_gobs *g2, const int &cf2,                
                          const t_gobs *g3 = 0, const int &cf3 = 0) const; 

        /**
         * @brief return value [m] of general carrier-phase
         * 
         * @param g1 
         * @param cf1 
         * @param g2 
         * @param cf2 
         * @param g3 
         * @param cf3 
         * @return double 
         */
        double _lcf_phase(const t_gobs *g1, const int &cf1,                
                          const t_gobs *g2, const int &cf2,                
                          const t_gobs *g3 = 0, const int &cf3 = 0) const; 

        /**
         * @brief return value [m] of general pseudo-range
         * 
         * @param g1 
         * @param cf1 
         * @param g2 
         * @param cf2 
         * @param g3 
         * @param cf3 
         * @return double 
         */
        double _lc_range(const t_gobs *g1, const double &cf1,                
                         const t_gobs *g2, const double &cf2,                
                         const t_gobs *g3 = 0, const double &cf3 = 0) const; 

        /**
         * @brief return value [m] of general carrier-phase
         * 
         * @param g1 
         * @param cf1 
         * @param g2 
         * @param cf2 
         * @param g3 
         * @param cf3 
         * @return double 
         */
        double _lc_phase(const t_gobs *g1, const double &cf1,                
                         const t_gobs *g2, const double &cf2,               
                         const t_gobs *g3 = 0, const double &cf3 = 0) const; 

        /**
         * @brief get c1,c2,c3 coefficients of code multipath LC
         * 
         * @param bC 
         * @param cC 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int _coef_multpath(const GOBSBAND &bC, double &cC, 
                           const GOBSBAND &b1, double &c1, 
                           const GOBSBAND &b2, double &c2) const;

        /**
         * @brief return coefficients (c1,c2) of the ionosphere-free linear combination
         * 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int _coef_ionofree(const GOBSBAND &b1, double &c1,        
                           const GOBSBAND &b2, double &c2) const; 

        /**
         * @brief return coefficients (c1,c2) of the geometry-free linear combination
         * 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int _coef_geomfree(const GOBSBAND &b1, double &c1,        
                           const GOBSBAND &b2, double &c2) const; 

        /**
         * @brief return coefficients (c1,c2) of the narrow-lane linear combination
         * 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int _coef_narrlane(const GOBSBAND &b1, double &c1,        
                           const GOBSBAND &b2, double &c2) const; 

        /**
         * @brief return coefficients (c1,c2) of the wide-lane linear combination
         * 
         * @param b1 
         * @param c1 
         * @param b2 
         * @param c2 
         * @return int 
         */
        int _coef_widelane(const GOBSBAND &b1, double &c1,        
                           const GOBSBAND &b2, double &c2) const; 

        /**
         * @brief get  code ID of selected band (according to table)
         * 
         * @param b 
         * @return GOBS 
         */
        GOBS _id_range(const GOBSBAND &b) const; 

        /**
         * @brief get phase ID of selected band (according to table)
         * 
         * @param b 
         * @return GOBS 
         */
        GOBS _id_phase(const GOBSBAND &b) const; 

        /**
         * @brief get doppler ID of selected band (according to table)
         * 
         * @param b 
         * @return GOBS 
         */
        GOBS _id_doppler(const GOBSBAND &b) const; 

        /**
         * @brief get snr ID of selected band (according to table)
         * 
         * @param b 
         * @return GOBS 
         */
        GOBS _id_snr(const GOBSBAND &b) const; 

        /**
         * @brief get  code ID of selected band (according to table)
         * 
         * @param band 
         * @return GOBS 
         */
        GOBS _cod_id(const int &band) const; 

        /**
         * @brief get phase ID of selected band (according to table)
         * 
         * @param band 
         * @return GOBS 
         */
        GOBS _pha_id(const int &band) const; 

        /**
         * @brief clear
         * 
         */
        virtual void _clear();

        /**
         * @brief valid
         * 
         * @return true 
         * @return false 
         */
        virtual bool _valid() const;

        /**
         * @brief valid obs
         * 
         * @return true 
         * @return false 
         */
        virtual bool _valid_obs() const;

        /**
         * @brief get available freq  for phase
         * 
         * @return set<GFRQ> 
         */
        set<GFRQ> _freq_avail() const; 

        /**
         * @brief get available bands for phase
         * 
         * @return set<GOBSBAND> 
         */
        set<GOBSBAND> _band_avail() const; 

        /**
         * @brief get available bands for code
         * 
         * @return set<GOBSBAND> 
         */
        set<GOBSBAND> _band_avail_code() const; 

    protected:
        map<GOBS, double> _gobs;  // maps of observations
        map<GOBS, int> _glli;     // maps of lost-of-lock identifications
        map<GOBS, int> _gslip;    // maps of estimated cycle slips
        map<GOBS, int> _goutlier; // maps of gross error of range/doppler observations
        map<GOBS, int> _gLevel;   // level

        string _staid;          // station id
        string _satid;          // satellite id ["G??", "R??", "E??" ...]
        GSYS _gsys;             // system
        t_gtime _epoch;         // epoch of the observation
        double _apr_ele;        // approximate elevation
        int _channel;           // satellite channel number
        unsigned int _rtcm_end; // RTCM Multiple Message bit (0 = end, 1 = cont.)

        bool _health;

    private:
        bool _isDoubleIf = false;
        bool _dcb_correct_mark;   // mark for correct dcb; true: dcb corrected; false: not corrected;
        bool _phase_correct_mark; // phase correct mark
        bool _range_smooth_mark;  // true, smooth; false, not smooth
        bool _bds_code_bias_mark; // true, correct; false, not correct
    };

    class t_obscmb
    {
    public:
        t_obscmb()
        {
            num = 0.0;
            lam = 0.0;
        };
        double num;
        double lam;
        t_gobs first;
        t_gobs second; 
        bool operator<(const t_obscmb &t) const;
    };

} // namespace

#endif
