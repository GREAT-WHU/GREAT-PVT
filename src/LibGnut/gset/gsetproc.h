/**
*
* @verbatim
    History

    @endverbatim
*
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

*
* @file        gsetproc.h
* @brief       implements process setting class
* @author      Jan Dousa
* @version     1.0.0
* @date        2012-10-23
*
*/

#ifndef GSETPROC_H
#define GSETPROC_H

#define XMLKEY_PROC "process" ///< The defination of process node in XML
#define XMLKEY_ROM "read_ofile_mode"

#include <string>
#include <iostream>

#include "gutils/gtypeconv.h"
#include "gutils/gobs.h"
#include "gset/gsetbase.h"

using namespace std;
using namespace pugi;

namespace gnut
{

    enum class CONSTRPAR
    {
        EST,
        FIX,
        KIN,
        CONSTRPAR_UNDEF
    };

    enum class GRDMPFUNC
    {
        DEF_GRDMPFUNC,
        TILTING,
        CHEN_HERRING,
        BAR_SEVER
    };

    enum class ZTDMPFUNC
    {
        DEF_ZTDMPFUNC,
        COSZ,
        GMF,
        NO_MF
    }; 

    enum class IONMPFUNC
    {
        DEF_IONMPFUNC,
        ICOSZ,
        QFAC,
        NONE
    };

    enum class OBSWEIGHT
    {
        DEF_OBSWEIGHT,
        EQUAL,
        SINEL,
        SINEL2,
        SINEL4,
        PARTELE,
        SNR,
        SNRELE          
    };

    enum class TROPMODEL
    {
        DEF_TROPMODEL,
        SAASTAMOINEN,
        DAVIS,
        HOPFIELD,
        EXTERN
    };

    enum class RESIDTYPE
    {
        DEF_RESIDTYPE,
        RES_ORIG,
        RES_NORM,
        RES_ALL
    };

    enum class OBSCOMBIN
    {
        DEF_OBSCOMBIN,
        IONO_FREE,
        RAW_SINGLE,    ///< nonsupport
        RAW_DOUBLE,    ///< nonsupport
        RAW_ALL,
        RAW_MIX
    };

    enum class ATTITUDES
    {
        DEF_YAWMODEL,
        YAW_NOMI,
        YAW_RTCM
    };

    enum class CBIASCHAR
    {
        DEF_CBIASCHAR,
        ORIG,
        CHAR2,
        CHAR3
    };

    enum class BASEPOS
    {
        SPP,
        CFILE
    };

    enum RECEIVERTYPE
    {
        DEF,
        F9P,
        And
    };
  
    enum class SLIPMODEL
    {
        DEF_DETECT_MODEL, 
        SLIPMODEL_UNDEF
    };

    enum IONMODEL
    {
        VION,
        SION,
        DEF_ION,
        IONMODEL_UNDEF
    };

    enum class IFCB_MODEL
    {
        EST,
        COR,
        DEF,
        IFCB_MODEL_UNDEF
    };

    class LibGnut_LIBRARY_EXPORT t_gobscombtype
    {

    public:
        /** @brief constructor */
        t_gobscombtype();
        t_gobscombtype(const t_gobscombtype &other);
        explicit t_gobscombtype(const string &obscombtype);
        t_gobscombtype(const t_gobs &obstype, OBSCOMBIN combtype);
        t_gobscombtype(const t_gobs &obstype, GOBSBAND b1, FREQ_SEQ freq_1, OBSCOMBIN combtype);
        t_gobscombtype(const t_gobs &obstype, GOBSBAND b1, GOBSBAND b2, FREQ_SEQ freq_1, FREQ_SEQ freq_2, OBSCOMBIN combtype);
        t_gobscombtype(GOBSTYPE t, GOBSBAND b, OBSCOMBIN obscomb);

        /** @brief convert2str */
        string convert2str() const;

        bool operator==(const t_gobscombtype &g) const;
        bool operator<(const t_gobscombtype &g) const;

        /** @brief get Band */
        GOBSBAND getBand_1() const { return _obs_band_1; };
        GOBSBAND getBand_2() const { return _obs_band_2; };

        /** @brief is code */
        bool is_code() const;

    protected:
        GOBSTYPE _obs_type = GOBSTYPE::TYPE;
        GOBSBAND _obs_band = BAND;
        GOBSBAND _obs_band_1 = BAND;   
        GOBSBAND _obs_band_2 = BAND;   
        FREQ_SEQ _obs_freq_1 = FREQ_X; 
        FREQ_SEQ _obs_freq_2 = FREQ_X; 
        OBSCOMBIN _obs_combine = OBSCOMBIN::DEF_OBSCOMBIN;
    };

    class LibGnut_LIBRARY_EXPORT t_gsetproc : public virtual t_gsetbase
    {
    public:
        /** @brief constructor */
        t_gsetproc();

        /** @brief destructor */
        ~t_gsetproc() override;

        /**@brief settings check */
        void check() override;

        /**@brief settings help */
        void help() override;

        /**@brief if ues (true/false) */
        bool tropo();
        bool iono();
        bool tropo_slant();
        bool tropo_grad();
        bool phase();
        bool doppler();
        bool pos_kin();

        /**@brief frequency number */
        int frequency();

        /**@brief initial sigma */
        double sig_init_ztd();
        double sig_init_vion();
        double sig_init_grd();
        double sig_init_crd();
        double sig_init_vel();
        double sig_init_amb();
        double sig_init_glo();
        double sig_init_gal();
        double sig_init_bds();
        double sig_init_qzs();

        /**@brief cut-off satellite elevation(deg) */
        double minimum_elev();

        /**@brief posterior residual threshold */
        double max_res_norm();

        /**@brief min satellite number */
        int minsat();

        /**@brief set process */
        string ref_clk();
        SLIPMODEL slip_model();
        IONMODEL ion_model();
        TROPMODEL tropo_model();
        ZTDMPFUNC tropo_mf();
        GRDMPFUNC grad_mf();
        CONSTRPAR crd_est();
        OBSWEIGHT weighting();
        RESIDTYPE residuals();
        OBSCOMBIN obs_combin();
        ATTITUDES attitudes();
        CBIASCHAR cbiaschar();

        /**@brief str convert */
        GRDMPFUNC str2grdmpfunc(const string &mf);
        ZTDMPFUNC str2ztdmpfunc(const string &mf);
        IONMPFUNC str2ionmpfunc(const string &mf);
        OBSWEIGHT str2obsweight(const string &wg);
        TROPMODEL str2tropmodel(const string &tm);
        RESIDTYPE str2residtype(const string &rs);
        OBSCOMBIN str2obscombin(const string &oc);
        ATTITUDES str2attitudes(const string &at);
        CBIASCHAR str2cbiaschar(const string &cb);

        /**@brief str convert */
        static string grdmpfunc2str(GRDMPFUNC MF);
        static string ztdmpfunc2str(ZTDMPFUNC MF);
        static string ionmpfunc2str(IONMPFUNC MF);
        static string obsweight2str(OBSWEIGHT WG);
        static string tropmodel2str(TROPMODEL TM);
        static string residtype2str(RESIDTYPE RS);
        static string obscombin2str(OBSCOMBIN OC);
        static string attitude2str(ATTITUDES AT);
        static string cbiaschar2str(CBIASCHAR CB);
        static string basepos2str(BASEPOS BP);

        /**@brief ifcb_model */
        IFCB_MODEL ifcb_model(); 

        /**@brief base position */
        BASEPOS basepos(); 

        /**@brief str convert */
        BASEPOS str2basepos(const string &str);

        /**@brief bds code bias correction */
        bool bds_code_bias_correction();

        /**@brief get receiverType */
        RECEIVERTYPE get_receiverType();

        /**@brief mean pole model */
        modeofmeanpole mean_pole_model();

        /**@brief str convert */
        modeofmeanpole str2meanpolemodel(const string &tm);

    protected:
        bool _phase;            ///< use phase data [true=1, false=0]
        bool _tropo;            ///< estimate troposphere [true=1, false=0]
        bool _iono;             ///< estimate ionosphere [true=1, false=0]
        bool _tropo_grad;       ///< estimate troposphere gradinet [true=1, false=0]
        bool _tropo_slant;      ///< estimate tropo slant delays
        TROPMODEL _tropo_model; ///< tropospheric model [SAASTAMOINEN, DAVIS, HOPFIELD, ...]
        ZTDMPFUNC _tropo_mf;    ///< tropo mapping function [COSZ, NIELL, GMF, VMF1, ... ]
        IONMPFUNC _iono_mf;     ///< iono mapping function [COSZ, QFAC, NONE, ... ]
        GRDMPFUNC _grad_mf;     ///< grad mapping function [tilt, CHH]
        OBSWEIGHT _obs_weight;  ///< observations weighting
        RESIDTYPE _res_type;    ///< types of residuals
        OBSCOMBIN _obs_combin;  ///< observation combination
        ATTITUDES _attitudes;   ///< satellite attitude model
        CBIASCHAR _cbiaschar;   ///< forcing code bias signals

        double _sig_init_ztd;  ///< accuracy of initial zenith path delays [m]
        double _sig_init_vion; ///< accuracy of initial vertical iono path delays [m]
        double _sig_init_grd;  ///< accuracy of initial tropo gradients [m]
        double _sig_init_crd;  ///< accuracy of initial coordinates [m]
        double _sig_init_vel;  ///< accuracy of initial velocities [m/s]
        double _sig_init_amb;  ///< accuracy of initial ambiguity [m]
        double _sig_init_glo;  ///< accuracy of initial GLONASS system time difference
        double _sig_init_gal;  ///< accuracy of initial Galileo system time difference
        double _sig_init_bds;  ///< accuracy of initial BeiDou system time difference
        double _sig_init_qzs;  ///< accuracy of initial QZSS system time difference
        double _minimum_elev;  ///< elevation angle cut-off [degree]
        double _max_res_norm;  ///< normalized residuals threshold
        string _crd_est;       ///< FIX or estimate CRD
        bool _pos_kin;         ///< static/kinematic receiver (true == kinematic)
        int _frequency;        ///< frequency

        double _sig_init_sat_pcv;       ///< satelite PCV estimaiton
        double _sig_init_rec_pcv;       ///< receiver PCV estimaiton
        double _sig_init_sat_pcox;      ///< satellite X PCO estimaion
        double _sig_init_sat_pcoy;      ///< satellite Y PCO estimaion
        double _sig_init_sat_pcoz;      ///< satellite Z PCO estimaion
        double _sig_init_rec_pcox;      ///< receiver X PCO estimaion
        double _sig_init_rec_pcoy;      ///< receiver Y PCO estimaion
        double _sig_init_rec_pcoz;      ///< receiver Z PCO estimaion
        map<string, bool> _sat_pco_xyz; ///< which component of satellite PCO to be estimated(X/Y/Z)
        map<string, bool> _rec_pco_xyz; ///< which component of receiver PCO to be estimated(X/Y/Z)
        double _sat_azi_beg;            ///< begin azimuth angle of satellite PCV
        double _sat_azi_end;            ///< end azimuth angle of satellite PCV
        double _sat_dazi;               ///< azimuth interval of satellite PCV
        double _sat_zen_beg;            ///< begin zenith angle of satellite PCV
        double _sat_zen_end;            ///< end zenith angle of satellite PCV
        double _sat_dzen;               ///< zenith angle of satellite PCV
        double _rec_azi_beg;            ///< begin azimuth angle of receiver PCV
        double _rec_azi_end;            ///< end azimuth angle of receiver PCV
        double _rec_dazi;               ///< azimuth interval of receiver PCV
        double _rec_zen_beg;            ///< begin zenith angle of receiver PCV
        double _rec_zen_end;            ///< end zenith angle of receiver PCV
        double _rec_dzen;               ///< zenith angle of receiver PCV
        int _minsat;                    ///< minimum satellite number
        BASEPOS _basepos;               ///< base position
        bool _sd_sat;                   ///< single differented between sat and sat_ref
        modeofmeanpole _meanpolemodel;  ///< different mean pole modeling

    private:
    };

} // namespace

#endif
