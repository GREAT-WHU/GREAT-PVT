
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
 
-*/

#include <iostream>
#include <cmath>

#include "newmat/newmat.h"
#include "gmodels/gpar.h"
#include "gmodels/ggmf.h"

using namespace std;

namespace gnut
{

    const string t_gpar::PAR_STR_SEP = "_";

    const vector<string> t_gpar::PAR_STR =
        {
            "CRD_X",
            "CRD_Y",
            "CRD_Z", // coordinates
            "TRP",
            "GRD_N",
            "GRD_E",
            "SION",
            "VION", // atmospheric parameters
            "CLK",
            "CLK_SAT", // clocks
            "CLK13_G",
            "CLK13_E",
            "CLK13_C",
            "CLK13_J", // receiver clocks for different systems in IF_mode
            "CLK14_G",
            "CLK14_E",
            "CLK14_C",
            "CLK14_J", // receiver clocks for different systems in IF_mode
            "CLK15_G",
            "CLK15_E",
            "CLK15_C",
            "CLK15_J", // receiver clocks for different systems in IF_mode
            "CLK15_G_SAT",
            "E15CLK_SAT",
            "C15CLK_SAT",
            "J15CLK_SAT", // satellite clocks for 13 combination 
            "CLK_G",
            "CLK_E",
            "CLK_C",
            "CLK_R",
            "CLK_J", // receiver clocks for different systems 
            "CLK_VPW",
            "CLK_VR",
            "CLK_VQ",
            "IFB_C3",
            "IFB_C4",
            "IFB_C5",
            "IFCB_F3",
            "IFCB_F4",
            "IFCB_F5", // inter-freq. code biases for FREQ_3, FREQ_4, FREQ_5, inter-freq. clock bias for GPS FREQ_3
            "SAT_IFB_C3",
            "SAT_IFB_C4",
            "SAT_IFB_C5",
            "GPS_REC_IFB_C3",
            "GPS_REC_IFB_C4",
            "GPS_REC_IFB_C5",
            "GAL_REC_IFB_C3",
            "GAL_REC_IFB_C4",
            "GAL_REC_IFB_C5",
            "BDS_REC_IFB_C3",
            "BDS_REC_IFB_C4",
            "BDS_REC_IFB_C5",
            "QZS_REC_IFB_C3",
            "QZS_REC_IFB_C4",
            "QZS_REC_IFB_C5",
            "GPS_IFB_C3",
            "GPS_IFB_C4",
            "GPS_IFB_C5",
            "GAL_IFB_C3",
            "GAL_IFB_C4",
            "GAL_IFB_C5",
            "BDS_IFB_C3",
            "BDS_IFB_C4",
            "BDS_IFB_C5",
            "QZS_IFB_C3",
            "QZS_IFB_C4",
            "QZS_IFB_C5",
            "CLK_ICB",
            "CLUSTERB", // initial clock bias, cluster-dependent bias
            "AMB_IF",
            "AMB13_IF",
            "AMB14_IF",
            "AMB15_IF",
            "AMB_WL",
            "AMB_L1",
            "AMB_L2",
            "AMB_L3",
            "AMB_L4",
            "AMB_L5", // ambiguities for indiv. freq. (number indicates freq not band)
            "FCBS_IF",
            "FCBS_L1",
            "FCBS_L2",
            "FCBS_L3",
            "FCBS_L4",
            "FCBS_L5", // satellite fractional cycle biases for indiv. freq.
            "FCBR_IF",
            "FCBR_L1",
            "FCBR_L2",
            "FCBR_L3",
            "FCBR_L4",
            "FCBR_L5", // receiver fractional cycle biases for indiv. freq.
            "GLO_ISB",
            "GLO_IFCB",
            "GLO_IFPB",
            "GAL_ISB",
            "BDS_ISB",
            "QZS_ISB", // multi-GNSS
            "LEO_ISB",
            "IFB_GPS", "IFB_BDS", "IFB_QZS", "IFB_GAL", "IFB_GAL_2", "IFB_GAL_3",
            "IFB_BDS_2", "IFB_BDS_3",
    };

    t_gparhead::t_gparhead(par_type type, string site, string sat) : type(type),
                                                                     site(site),
                                                                     sat(sat)
    {
    }

    t_gparhead::t_gparhead(const t_gparhead &Other) : type(Other.type),
                                                      sat(Other.sat),
                                                      site(Other.site)
    {
    }

    t_gparhead::~t_gparhead()
    {
    }

    bool t_gparhead::operator==(const t_gparhead &Other) const
    {
        if (this->type == Other.type &&
            this->sat == Other.sat &&
            this->site == Other.site)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool t_gparhead::operator<(const t_gparhead &Other) const
    {
        if (
            this->type < Other.type ||
            (this->type == Other.type && this->site < Other.site) ||
            (this->type == Other.type && this->site == Other.site && this->sat < Other.sat)
            )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool t_gparhead::operator<=(const t_gparhead &Other) const
    {
        if (*this == Other || *this < Other)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool t_gparhead::operator>(const t_gparhead &Other) const
    {
        if (*this <= Other)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    bool t_gparhead::operator>=(const t_gparhead &Other) const
    {
        if (*this < Other)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    t_gtimearc::t_gtimearc(const t_gtime &beg, const t_gtime &end) : begin(beg),
                                                                     end(end)
    {
    }

    t_gtimearc::~t_gtimearc() = default;
    bool t_gtimearc::operator!=(const t_gtimearc &Other) const
    {
        return !(*this == Other);
    }

    bool t_gtimearc::operator==(const t_gtimearc &Other) const
    {
        return (this->begin == Other.begin && this->end == Other.end);
    }

    bool t_gtimearc::operator<(const t_gtimearc &Other) const
    {
        return (this->begin < Other.begin || (this->begin == Other.begin && this->end < Other.end));
    }

    bool t_gtimearc::operator<=(const t_gtimearc &Other) const
    {
        return (*this == Other || *this < Other);
    }

    bool t_gtimearc::operator>(const t_gtimearc &Other) const
    {
        return !(*this <= Other);
    }

    bool t_gtimearc::operator>=(const t_gtimearc &Other) const
    {
        return !(*this < Other);
    }

    bool t_gtimearc::inside(const t_gtimearc &Other) const
    {
        return (this->begin >= Other.begin && this->end <= Other.end);
    }

    t_gpar::t_gpar(const string &site, par_type t, unsigned i, const string &p, bool remove)
    {
        beg = FIRST_TIME;
        end = LAST_TIME;

        this->site = site;
        parType = t;
        index = i;
        prn = p;
        amb_ini = false;
        value(0.0);
        apriori(0.0);
        idx_pcv = 0;
        zen1 = 0.0;
        zen2 = 0.0;
        azi1 = 0.0;
        azi2 = 0.0;
        dzen = 0.0;
        dazi = 0.0;
        nzen = 0;
        nazi = 0;
    }

    t_gpar::t_gpar()
    {
        beg = FIRST_TIME;
        end = LAST_TIME;
    }

    t_gpar::~t_gpar()
    {
    }

    void t_gpar::setMF(ZTDMPFUNC MF)
    {
        _mf_ztd = MF;
    }

    void t_gpar::setMF(GRDMPFUNC MF)
    {
        _mf_grd = MF;
    }

    double t_gpar::partial(t_gsatdata &satData, t_gtime &epoch, t_gtriple ground, t_gobs &gobs)
    {
        double mfw, dmfw, mfh, dmfh;
        mfw = dmfw = mfh = dmfh = 0.0;
        switch (parType)
        {
        case par_type::CRD_X:
            if (satData.site() == this->site)
            {
                return (value() - satData.satcrd().crd(0)) / satData.rho();
            }
            else
                return 0.0;
        case par_type::CRD_Y:
            if (satData.site() == this->site)
            {
                return (value() - satData.satcrd().crd(1)) / satData.rho();
            }
            else
                return 0.0;
        case par_type::CRD_Z:
            if (satData.site() == this->site)
            {
                return (value() - satData.satcrd().crd(2)) / satData.rho();
            }
            else
                return 0.0;
        case par_type::CLK:
        {
            if (satData.site() == this->site)
                return 1.0;
            else
                return 0.0;
        }
        case par_type::CLK_SAT:
        {
            if (satData.sat() == this->prn)
                return -1.0;
            else
                return 0.0;
        }
        case par_type::IFCB_F3:
            if (t_gsys::band2freq(satData.gsys(), gobs.band()) == FREQ_3 && prn == satData.sat())
                return -1.0;
            else
                return 0.0;
        case par_type::IFCB_F4:
            if (t_gsys::band2freq(satData.gsys(), gobs.band()) == FREQ_4 && prn == satData.sat())
                return -1.0;
            else
                return 0.0;
        case par_type::IFCB_F5:
            if (t_gsys::band2freq(satData.gsys(), gobs.band()) == FREQ_5 && prn == satData.sat())
                return -1.0;
            else
                return 0.0;
        case par_type::TRP:
            if (satData.site() == this->site)
            {
                _getmf(satData, ground, epoch, mfw, mfh, dmfw, dmfh);

                return mfw;
            }
            else
                return 0.0;
        case par_type::SION:
        {
            if (satData.site() == this->site)
            {
                double f1 = satData.frequency(t_gsys::band_priority(satData.gsys(), FREQ_1));
                double fk = satData.frequency(gobs.band());
                double alfa = 0.0;
                if (gobs.is_phase() && prn == satData.sat())
                {
                    alfa = -(f1 * f1) / (fk * fk);
                }
                if (gobs.is_code() && prn == satData.sat())
                {
                    alfa = (f1 * f1) / (fk * fk);
                }
                return alfa;
            }
            else
                return 0.0;
        }
        case par_type::VION:
        {
            if (satData.site() == this->site)
            {
                double f1 = satData.frequency(t_gsys::band_priority(satData.gsys(), FREQ_1));
                double fk = satData.frequency(gobs.band());
                double mf = 1.0 / sqrt(1.0 - pow(R_SPHERE / (R_SPHERE + 450000.0) * sin(G_PI / 2.0 - satData.ele()), 2));
                double alfa = 0.0;
                if (gobs.is_phase() && prn == satData.sat())
                {
                    alfa = -(f1 * f1) / (fk * fk);
                }
                if (gobs.is_code() && prn == satData.sat())
                {
                    alfa = (f1 * f1) / (fk * fk);
                }
                return alfa * mf;
            }
            else
                return 0.0;
        }
        case par_type::GRD_N:
            if (satData.site() == this->site)
            {
                _getmf(satData, ground, epoch, mfw, mfh, dmfw, dmfh);
                if (_mf_grd == GRDMPFUNC::CHEN_HERRING)
                {
                    double sinel = sin(satData.ele());
                    double tanel = tan(satData.ele());
                    double cosaz = cos(satData.azi());
                    return (1.0 / (sinel * tanel + 0.0032)) * cosaz;
                }
                else if (_mf_grd == GRDMPFUNC::TILTING)
                {
                    double cosaz = cos(satData.azi());
                    return dmfw * cosaz;
                }
                else if (_mf_grd == GRDMPFUNC::BAR_SEVER)
                {
                    double tanel = tan(satData.ele());
                    double cosaz = cos(satData.azi());
                    return mfw * (1.0 / tanel) * cosaz;
                }
                else
                    cerr << "Grad N mapping function is not set up correctly!!!" << endl;
            }
            else
                return 0.0;
        case par_type::GRD_E:
            if (satData.site() == this->site)
            {
                _getmf(satData, ground, epoch, mfw, mfh, dmfw, dmfh);
                if (_mf_grd == GRDMPFUNC::CHEN_HERRING)
                {
                    double sinel = sin(satData.ele());
                    double tanel = tan(satData.ele());
                    double sinaz = sin(satData.azi());
                    return (1.0 / (sinel * tanel + 0.0032)) * sinaz;
                }
                else if (_mf_grd == GRDMPFUNC::TILTING)
                {
                    double sinaz = sin(satData.azi());
                    return dmfw * sinaz;
                }
                else if (_mf_grd == GRDMPFUNC::BAR_SEVER)
                {
                    double tanel = tan(satData.ele());
                    double sinaz = sin(satData.azi());
                    return mfw * (1 / tanel) * sinaz;
                }
                else
                    cerr << "Grad E mapping function is not set up correctly!!!" << endl;
            }
            else
                return 0.0;

        case par_type::AMB_IF:
            if (satData.site() == this->site && gobs.is_phase() && prn == satData.sat())
                return 1.0;
            else
                return 0.0;
        case par_type::AMB_L1:
            if (satData.site() == this->site && gobs.is_phase() && t_gsys::band2freq(satData.gsys(), gobs.band()) == FREQ_1 && prn == satData.sat())
                return 1.0;
            else
                return 0.0;
        case par_type::AMB_L2:
            if (satData.site() == this->site && gobs.is_phase() && t_gsys::band2freq(satData.gsys(), gobs.band()) == FREQ_2 && prn == satData.sat())
                return 1.0;
            else
                return 0.0;
        case par_type::AMB_L3:
            if (satData.site() == this->site && gobs.is_phase() && t_gsys::band2freq(satData.gsys(), gobs.band()) == FREQ_3 && prn == satData.sat())
                return 1.0;
            else
                return 0.0;
        case par_type::AMB_L4:
            if (satData.site() == this->site && gobs.is_phase() && t_gsys::band2freq(satData.gsys(), gobs.band()) == FREQ_4 && prn == satData.sat())
                return 1.0;
            else
                return 0.0;
        case par_type::AMB_L5:
            if (satData.site() == this->site && gobs.is_phase() && t_gsys::band2freq(satData.gsys(), gobs.band()) == FREQ_5 && prn == satData.sat())
                return 1.0;
            else
                return 0.0;
        case par_type::GLO_ISB:
            if (satData.site() == this->site && satData.gsys() == GLO)
                return 1.0;
            else
                return 0.0;
        case par_type::GLO_IFCB:
            if (satData.site() == this->site && !gobs.is_phase() && satData.gsys() == GLO && prn == satData.sat())
                return 1.0;
            else
                return 0.0;
        case par_type::GLO_IFPB:
            if (satData.site() == this->site && gobs.is_phase() && satData.gsys() == GLO && prn == satData.sat())
                return 1.0;
            else
                return 0.0;
        case par_type::GLO_IFB:
            if (satData.site() == this->site && satData.gsys() == GLO && prn == satData.sat())
                return 1.0;
            else
                return 0.0;
        case par_type::GAL_ISB:
            if (satData.site() == this->site && satData.gsys() == GAL)
                return 1.0;
            else
                return 0.0;
        case par_type::BDS_ISB:
            if (satData.site() == this->site && satData.gsys() == BDS)
                return 1.0;
            else
                return 0.0;
        case par_type::QZS_ISB:
            if (satData.site() == this->site && satData.gsys() == QZS)
                return 1.0;
            else
                return 0.0;
        default:
            return 0.0;
        }

        return 0.0;
    }

    double t_gpar::partial_doppler(t_gsatdata &satData, t_gtriple &groundXYZ, t_gtriple &groundVEL)
    {
        t_gtriple satcrd = satData.satcrd();
        t_gtriple satvel = satData.satvel();
        ColumnVector cSat = satcrd.crd_cvect();
        ColumnVector vSat = satvel.crd_cvect();
        ColumnVector cRec = groundXYZ.crd_cvect();
        ColumnVector vRec = groundVEL.crd_cvect();
        ColumnVector conf_crd = dotproduct(cSat - cRec, vSat - vRec) * (cSat - cRec) / pow(satData.rho(), 3);
        conf_crd -= (vSat - vRec) / satData.rho();
        ColumnVector e = (cSat - cRec) / satData.rho();
        satData._conf_crd.set(conf_crd);
        satData._e.set(e);
        switch (parType)
        {
        case par_type::CRD_X:
            return conf_crd(1);
        case par_type::CRD_Y:
            return conf_crd(2);
        case par_type::CRD_Z:
            return conf_crd(3);
        case par_type::VEL_X:
            return -e(1);
        case par_type::VEL_Y:
            return -e(2);
        case par_type::VEL_Z:
            return -e(3);
        case par_type::CLK_RAT:
            return 1.0;
        default:
            return 0.0;
        }
        return 0.0;
    }

    bool t_gpar::operator==(const t_gpar &par) const
    {
        if (parType == par.parType &&
            beg == par.beg &&
            end == par.end)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool t_gpar::operator<(const t_gpar &par) const
    {
        if ((parType < par.parType) ||
            (parType == par.parType && prn < par.prn) ||
            (parType == par.parType && prn == par.prn && site < par.site) ||
            (parType == par.parType && prn == par.prn && site == par.site && beg < par.beg) ||
            (parType == par.parType && prn == par.prn && site == par.site && beg == par.beg && end < par.end))
        {
            return true;
        }
        return false;
    }

    bool t_gpar::operator>(const t_gpar &par) const
    {
        if ((parType > par.parType) ||
            (parType == par.parType && prn < par.prn) ||
            (parType == par.parType && prn == par.prn && site < par.site) ||
            (parType == par.parType && prn == par.prn && site == par.site && beg < par.beg) ||
            (parType == par.parType && prn == par.prn && site == par.site && beg == par.beg && end < par.end))
        {
            return true;
        }
        return false;
    }

    t_gpar t_gpar::operator-(const t_gpar &p) const
    {
        t_gpar par = (*this);
        par.value(value() - p.value());
        return par;
    }

    t_gpar t_gpar::operator+(const t_gpar &p) const
    {
        t_gpar par = (*this);
        par.value(value() + p.value());
        return par;
    }

    void t_gpar::setTime(const t_gtime &t1, const t_gtime &t2)
    {
        this->beg = t1;
        this->end = t2;
    }

    string t_gpar::str_type() const
    {
        string type;
        switch (parType)
        {
        case par_type::CRD_X:
            type = "CRD_X";
            break;
        case par_type::CRD_Y:
            type = "CRD_Y";
            break;
        case par_type::CRD_Z:
            type = "CRD_Z";
            break;
        case par_type::TRP:
            type = "TRP";
            break;
        case par_type::VEL_X:
            type = "VEL_X";
            break;
        case par_type::VEL_Y:
            type = "VEL_Y";
            break;
        case par_type::VEL_Z:
            type = "VEL_Z";
            break;
        case par_type::CLK_RAT:
            type = "CLK_RAT";
            break;
        case par_type::SION:
            type = "SION_" + prn;
            break;
        case par_type::VION:
            type = "VION_" + prn;
            break;
        case par_type::CLK:
            type = "CLK";
            break;
        case par_type::CLK13_G:
            type = "CLK13_G";
            break;
        case par_type::CLK14_G:
            type = "CLK14_G";
            break;
        case par_type::CLK15_G:
            type = "CLK15_G";
            break;
        case par_type::CLK13_E:
            type = "CLK13_E";
            break;
        case par_type::CLK14_E:
            type = "CLK14_E";
            break;
        case par_type::CLK15_E:
            type = "CLK15_E";
            break;
        case par_type::CLK13_C:
            type = "CLK13_C";
            break;
        case par_type::CLK14_C:
            type = "CLK14_C";
            break;
        case par_type::CLK15_C:
            type = "CLK15_C";
            break;
        case par_type::CLK13_J:
            type = "CLK13_J";
            break;
        case par_type::CLK14_J:
            type = "CLK14_J";
            break;
        case par_type::CLK15_J:
            type = "CLK15_J";
            break;
        case par_type::IFCB_F3:
            type = "IFCB_F3_" + prn;
            break;
        case par_type::IFCB_F4:
            type = "IFCB_F4_" + prn;
            break;
        case par_type::IFCB_F5:
            type = "IFCB_F5_" + prn;
            break;
        case par_type::CLK_SAT:
            type = "CLK_SAT_" + prn;
            break;
        case par_type::AMB_IF:
            type = "AMB_IF_" + prn;
            break;
        case par_type::AMB13_IF:
            type = "AMB13_IF_" + prn;
            break;
        case par_type::AMB14_IF:
            type = "AMB14_IF_" + prn;
            break;
        case par_type::AMB15_IF:
            type = "AMB15_IF_" + prn;
            break;
        case par_type::AMB_WL:
            type = "AMB_WL_" + prn;
            break;
        case par_type::AMB_L1:
            type = "AMB_L1_" + prn;
            break;
        case par_type::AMB_L2:
            type = "AMB_L2_" + prn;
            break;
        case par_type::AMB_L3:
            type = "AMB_L3_" + prn;
            break;
        case par_type::AMB_L4:
            type = "AMB_L4_" + prn;
            break;
        case par_type::AMB_L5:
            type = "AMB_L5_" + prn;
            break;
        case par_type::GRD_N:
            type = "GRD_N";
            break;
        case par_type::GRD_E:
            type = "GRD_E";
            break;
        case par_type::GLO_ISB:
            type = "GLO_ISB";
            break;
        case par_type::GLO_IFCB:
            type = "GLO_IFCB";
            break;
        case par_type::GLO_IFPB:
            type = "GLO_IFPB";
            break;
        case par_type::GLO_IFB:
            type = "GLO_IFB_" + prn;
            break;
        case par_type::GAL_ISB:
            type = "GAL_ISB";
            break;
        case par_type::BDS_ISB:
            type = "BDS_ISB";
            break;
        case par_type::QZS_ISB:
            type = "QZS_ISB";
            break;
        case par_type::IFB_GPS:
            type = "IFB_GPS";
            break;
        case par_type::IFB_BDS:
            type = "IFB_BDS";
            break;
        case par_type::IFB_GAL:
            type = "IFB_GAL";
            break;
        case par_type::IFB_QZS:
            type = "IFB_QZS";
            break;
        case par_type::IFB_GAL_2:
            type = "IFB_GAL_2";
            break;
        case par_type::IFB_GAL_3:
            type = "IFB_GAL_3";
            break;
        case par_type::IFB_BDS_2:
            type = "IFB_BDS_2";
            break;
        case par_type::IFB_BDS_3:
            type = "IFB_BDS_3";
            break;
        default:
            type = "UNDEF";
        }

        return type;
    }

    t_gparhead t_gpar::get_head() const
    {
        return t_gparhead(this->parType, this->site, this->prn);
    }

    t_gtimearc t_gpar::get_timearc() const
    {
        return t_gtimearc(this->beg, this->end);
    }

    void t_gpar::_getmf(t_gsatdata &satData,
                        const t_gtriple &crd,
                        const t_gtime &epoch,
                        double &mfw,
                        double &mfh,
                        double &dmfw,
                        double &dmfh)
    {
        if (parType != par_type::TRP && parType != par_type::GRD_N && parType != par_type::GRD_E)
            return;

        double ele = satData.ele();

        if (_mf_ztd == ZTDMPFUNC::COSZ)
        {
            mfw = mfh = 1.0 / sin(ele);
        }
        else if (_mf_ztd == ZTDMPFUNC::GMF)
        {
            t_gmf mf;
            mf.gmf(epoch.mjd(), crd[0], crd[1], crd[2], G_PI / 2.0 - ele,
                   mfh, mfw, dmfh, dmfw);
        }
        else
            cerr << "ZTD mapping function is not set up correctly!!!" << endl;
    }

    string gpar2str(const t_gpar &par)
    {
        string partype = par.str_type();
        partype = par.site + t_gpar::PAR_STR_SEP + partype;
        if (par.prn == "")
        {
            partype += t_gpar::PAR_STR_SEP;
        }
        return partype;
    }

    string LibGnut_LIBRARY_EXPORT ptype2str(const par_type &parType)
    {
        string type;
        switch (parType)
        {
        case par_type::CRD_X:
            type = "CRD_X";
            break;
        case par_type::CRD_Y:
            type = "CRD_Y";
            break;
        case par_type::CRD_Z:
            type = "CRD_Z";
            break;
        case par_type::TRP:
            type = "TRP";
            break;
        case par_type::SION:
            type = "SION_";
            break;
        case par_type::VION:
            type = "VION_";
            break;
        case par_type::CLK:
            type = "CLK";
            break;
        case par_type::CLK13_G:
            type = "CLK13_G";
            break;
        case par_type::CLK13_E:
            type = "CLK13_E";
            break;
        case par_type::CLK13_C:
            type = "CLK13_C";
            break;
        case par_type::CLK13_J:
            type = "CLK13_J";
            break;
        case par_type::CLK14_G:
            type = "CLK14_G";
            break;
        case par_type::CLK14_E:
            type = "CLK14_E";
            break;
        case par_type::CLK14_C:
            type = "CLK14_C";
            break;
        case par_type::CLK14_J:
            type = "CLK14_J";
            break;
        case par_type::CLK15_G:
            type = "CLK15_G";
            break;
        case par_type::CLK15_E:
            type = "CLK15_E";
            break;
        case par_type::CLK15_C:
            type = "CLK15_C";
            break;
        case par_type::CLK15_J:
            type = "CLK15_J";
            break;
        case par_type::IFCB_F3:
            type = "IFCB_F3";
            break;
        case par_type::IFCB_F4:
            type = "IFCB_F4";
            break;
        case par_type::IFCB_F5:
            type = "IFCB_F5";
            break;
        case par_type::CLK_SAT:
            type = "CLK_SAT";
            break;
        case par_type::AMB_IF:
            type = "AMB_IF";
            break;
        case par_type::AMB13_IF:
            type = "AMB13_IF";
            break;
        case par_type::AMB14_IF:
            type = "AMB14_IF";
            break;
        case par_type::AMB15_IF:
            type = "AMB15_IF";
            break;
        case par_type::AMB_WL:
            type = "AMB_WL";
            break;
        case par_type::AMB_L1:
            type = "AMB_L1_";
            break;
        case par_type::AMB_L2:
            type = "AMB_L2_";
            break;
        case par_type::AMB_L3:
            type = "AMB_L3_";
            break;
        case par_type::AMB_L4:
            type = "AMB_L4_";
            break;
        case par_type::AMB_L5:
            type = "AMB_L5_";
            break;
        case par_type::GRD_N:
            type = "GRD_N";
            break;
        case par_type::GRD_E:
            type = "GRD_E";
            break;
        case par_type::GLO_ISB:
            type = "GLO_ISB";
            break;
        case par_type::GLO_IFCB:
            type = "GLO_IFCB";
            break;
        case par_type::GLO_IFPB:
            type = "GLO_IFPB";
            break;
        case par_type::GLO_IFB:
            type = "GLO_IFB";
            break;
        case par_type::GAL_ISB:
            type = "GAL_ISB";
            break;
        case par_type::BDS_ISB:
            type = "BDS_ISB";
            break;
        case par_type::QZS_ISB:
            type = "QZS_ISB";
            break;
        case par_type::IFB_GPS:
            type = "IFB_GPS";
            break;
        case par_type::IFB_BDS:
            type = "IFB_BDS";
            break;
        case par_type::IFB_GAL:
            type = "IFB_GAL";
            break;
        case par_type::IFB_GAL_2:
            type = "IFB_GAL_2";
            break;
        case par_type::IFB_GAL_3:
            type = "IFB_GAL_3";
            break;
        case par_type::IFB_BDS_2:
            type = "IFB_BDS_2";
            break;
        case par_type::IFB_BDS_3:
            type = "IFB_BDS_3";
            break;
        case par_type::IFB_QZS:
            type = "IFB_QZS";
            break;
        default:
            type = "UNDEF";
        }
        return type;
    }

    t_gpar str2gpar(const string &str_par)
    {
        int sep_first, sep_last = 0;
        string site, sat, str_type;
        sep_first = str_par.find(t_gpar::PAR_STR_SEP);
        site = str_par.substr(0, sep_first);

        sep_last = str_par.rfind(t_gpar::PAR_STR_SEP);
        sat = str_par.substr(sep_last + 1);

        str_type = str_par.substr(sep_first + 1, sep_last - sep_first - 1);

        par_type par_tp(par_type::NO_DEF);

        for (unsigned int i = 0; i < t_gpar::PAR_STR.size(); i++)
        {
            if (t_gpar::PAR_STR[i] == str_type)
            {
                par_tp = par_type(i);
                break;
            }
        }

        return t_gpar(site, par_tp, 0, sat);
    }

} // namespace
