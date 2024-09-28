
/**
*
* @verbatim
    History
    2011-04-18  PV: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gpar.h
* @brief      Purpose: parametres class
*.
* @author     PV
* @version    1.0.0
* @date       2011-04-18
*
*/
#ifndef GPAR_H
#define GPAR_H

#include "gexport/ExportLibGnut.h"
#include <string>

#include "gdata/gsatdata.h"
#include "gutils/gtriple.h"
#include "gset/gsetproc.h"

using namespace std;

namespace gnut
{

    enum class LibGnut_LIBRARY_EXPORT par_type
    {
        CRD_X,
        CRD_Y,
        CRD_Z, ///< coordinates
        TRP,
        GRD_N,
        GRD_E,
        SION,
        VION, ///< atmospheric parameters
        CLK,
        CLK_SAT, ///< clocks
        CLK13_G,
        CLK13_E,
        CLK13_C,
        CLK13_J, ///< receiver clocks for different systems in IF_mode
        CLK14_G,
        CLK14_E,
        CLK14_C,
        CLK14_J, ///< receiver clocks for different systems in IF_mode
        CLK15_G,
        CLK15_E,
        CLK15_C,
        CLK15_J, ///< receiver clocks for different systems in IF_mode
        CLK_G,
        CLK_E,
        CLK_C,
        CLK_R,
        CLK_J, ///< receiver clocks for different systems 
        IFCB_F3,
        IFCB_F4,
        IFCB_F5, ///< inter-freq. code biases for FREQ_3, FREQ_4, FREQ_5, inter-freq. clock bias for GPS FREQ_3
        AMB_IF,
        AMB13_IF,
        AMB14_IF,
        AMB15_IF,
        AMB_WL,
        AMB_L1,
        AMB_L2,
        AMB_L3,
        AMB_L4,
        AMB_L5, ///< ambiguities for indiv. freq. (number indicates freq not band)
        GLO_ISB,
        GLO_IFCB,
        GLO_IFPB,
        GAL_ISB,
        BDS_ISB,
        QZS_ISB, ///< multi-GNSS
        P1P2G_REC,
        P1P2E_REC,
        P1P2R_REC,
        P1P2C_REC, ///< GNSS-specific receiver code DCB P1-P2
        IFB_GPS,
        IFB_BDS,
        IFB_QZS,
        IFB_GAL,
        IFB_GAL_2,
        IFB_GAL_3, 
        IFB_BDS_2,
        IFB_BDS_3, ///< inter frequency bias for multi-frquency
        GLO_IFB, ///< inter frequency bias for glonass
        VEL_X,
        VEL_Y,
        VEL_Z, ///< velocity
        CLK_RAT, ///< satellite clock speed
        NO_DEF
    };

    class LibGnut_LIBRARY_EXPORT t_gparhead;

    /** @brief class for t_gtimearc. */
    class LibGnut_LIBRARY_EXPORT t_gtimearc
    {
    public:
        /** @brief constructor 1. */
        t_gtimearc(const t_gtime &beg, const t_gtime &end);

        /** @brief default destructor. */
        ~t_gtimearc();

        /** @brief override operator. */
        bool operator!=(const t_gtimearc &Other) const;
        bool operator==(const t_gtimearc &Other) const;
        bool operator<(const t_gtimearc &Other) const;
        bool operator<=(const t_gtimearc &Other) const;
        bool operator>(const t_gtimearc &Other) const;
        bool operator>=(const t_gtimearc &Other) const;

        /** @brief onside. */
        bool inside(const t_gtimearc &Other) const;

        t_gtime begin; ///< begin time
        t_gtime end;   ///< end time
    };

    /** @brief class for t_gpar. */
    class LibGnut_LIBRARY_EXPORT t_gpar
    {
    public:
        const static string PAR_STR_SEP;
        const static vector<string> PAR_STR;

        /** @brief is amb?. */
        static bool is_amb(par_type tp)
        {
            return (tp == par_type::AMB_IF || tp == par_type::AMB13_IF || tp == par_type::AMB14_IF || tp == par_type::AMB15_IF ||
                    tp == par_type::AMB_L1 || tp == par_type::AMB_L2 || tp == par_type::AMB_L3 || tp == par_type::AMB_L4 || tp == par_type::AMB_L5);
        }

        /** @brief is crd?. */
        static bool is_crd(par_type tp)
        {
            return (tp == par_type::CRD_X || tp == par_type::CRD_Y || tp == par_type::CRD_Z);
        }

        /** @brief constructor 1. */
        t_gpar(const string &site, par_type t, unsigned i, const string &p, bool remove = true);

        /** @brief default constructor. */
        t_gpar();

        /** @brief default destructor. */
        virtual ~t_gpar();

        /** @brief override operator. */
        bool operator==(const t_gpar &) const;
        bool operator<(const t_gpar &) const;
        bool operator>(const t_gpar &) const;
        t_gpar operator-(const t_gpar &) const;
        t_gpar operator+(const t_gpar &) const;

        /** @brief set time. */
        void setTime(const t_gtime &, const t_gtime &);

        /** @brief Partial derivatives. */
        double partial(t_gsatdata &, t_gtime &, t_gtriple, t_gobs &gobs);

        /** @brief Partial doppler. */
        double partial_doppler(t_gsatdata &satData,
                               t_gtriple &groundXYZ,
                               t_gtriple &groundVEL); 

        /** @brief set/get val. */
        void value(double val) { _value = val; }
        double value() const { return _value; }

        /** @brief set/get apriori. */
        void apriori(double apr) { _apriori = apr; }
        double apriori() const { return _apriori; }

        par_type parType; ///< par type
        int index;        ///< index
        string prn;       ///< satellite name
        string site;      ///< site name
        t_gtime beg;      ///< begin time
        t_gtime end;      ///< end time
        t_gtime stime;    ///< s time
        double aprval;    ///< apr value
        double pred;      ///< pred
        double zhd;       ///< for ztd par
        double zwd;       ///< for zwd par

        //for amb
        bool amb_ini = false; ///< add for amb to be initialized

        // for PCV/PCO estimation
        int idx_pcv; ///< index of PCV (zenith dependent: 1, 2; zenith and azimuth dependent: 1, 2, 3, and 4)
        int nazi;    ///< point num of PCV azimuth grids
        int nzen;    ///< point num of PCV zenith grids
        double dzen;
        double dazi;
        double zen1;
        double zen2;
        double azi1;
        double azi2;
        vector<GFRQ> fq; ///< single frequency: G01/G02/C02/R01..., LC combination: G01+G02/C02+C07...
        string str_type() const;
        t_gparhead get_head() const;
        t_gtimearc get_timearc() const;

        void setMF(ZTDMPFUNC MF);
        void setMF(GRDMPFUNC MF);

    protected:
        double _value;     ///< value
        ZTDMPFUNC _mf_ztd; ///< mapping function for ZTD
        GRDMPFUNC _mf_grd; ///< mapping function for GRD
        double _apriori;   ///< apriori
        void _getmf(t_gsatdata &satData,
                    const t_gtriple &crd,
                    const t_gtime &epoch,
                    double &mfw,
                    double &mfh,
                    double &dmfw,
                    double &dmfh);
    };

    /** @brief class for t_gprhead. */
    class LibGnut_LIBRARY_EXPORT t_gparhead
    {
    public:
        /** @brief constructor 1. */
        t_gparhead(par_type type, string site, string sat);

        /** @brief constructor 2. */
        t_gparhead(const t_gparhead &Other);

        /** @brief default destructor. */
        ~t_gparhead();

        /** @brief override operator. */
        bool operator==(const t_gparhead &Other) const;
        bool operator<(const t_gparhead &Other) const;
        bool operator<=(const t_gparhead &Other) const;
        bool operator>(const t_gparhead &Other) const;
        bool operator>=(const t_gparhead &Other) const;

        par_type type; ///< par type
        string site;   ///< site name
        string sat;    ///< satellite name
    };

    /** @brief convert par to string. */
    string LibGnut_LIBRARY_EXPORT gpar2str(const t_gpar &par);

    /** @brief convert partype to string. */
    string LibGnut_LIBRARY_EXPORT ptype2str(const par_type &par);

    /** @brief convert string to par. */
    t_gpar LibGnut_LIBRARY_EXPORT str2gpar(const string &str_par);

} // namespace

#endif
