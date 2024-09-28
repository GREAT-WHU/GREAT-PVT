
/**
*
* @verbatim
    History
    2011-12-05  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gpcv.h
* @brief      Purpose: implements antenna phase offsets and variations
*.
* @author     JD
* @version    1.0.0
* @date       2011-12-05
*
*/

#ifndef GPCV_H
#define GPCV_H

#include <vector>

#include "gexport/ExportLibGnut.h"
#include "newmat/newmat.h"
#include "gdata/gdata.h"
#include "gutils/gconst.h"
#include "gutils/gsys.h"
#include "gutils/gtime.h"
#include "gmodels/gephplan.h"
#include "gdata/gsatdata.h"

using namespace std;

namespace gnut
{
    /** @brief class for t_gpcv based t_gdata. */
    class LibGnut_LIBRARY_EXPORT t_gpcv : public t_gdata
    {

    public:
        /** @brief default constructor. */
        t_gpcv();

        t_gpcv(t_spdlog spdlog);
        /** @brief default destructor. */
        virtual ~t_gpcv();

        typedef map<double, double> t_map_Z;  ///< map of ZEN-dependence
        typedef map<double, t_map_Z> t_map_A; ///< map of AZI-dependence & ZEN-dependence

        typedef map<GFRQ, t_gtriple> t_map_pco; ///< North/East/Up phase centre offsets     (N-freqs)
        typedef map<GFRQ, t_map_Z> t_map_zen;   ///< map of AZI-dependence                  (N-freqs)
        typedef map<GFRQ, t_map_A> t_map_azi;   ///< map of AZI-dependence & ZEN-dependence (N-freqs)

        /** @brief model is for transmitter (satellite). */
        bool is_transmitter() const { return _trans; }

        /** @brief model is for receiver . */
        bool is_receiver() const { return !_trans; }

        /** @brief set/get antenna type definition (ATX field 0-19). */
        void anten(const string &s) { _anten = s; }
        const string &anten() const { return _anten; }

        /** @brief set/get antenna identification (ATX field 20-39). */
        void ident(const string &s) { _ident = s; }
        const string &ident() const { return _ident; }

        /** @brief set/get satellite (svn) code (ATX field 40-59). */
        void svcod(const string &s)
        {
            _svcod = s;
            if (s.empty())
                _trans = false;
            else
                _trans = true;
        } 
        const string &svcod() const { return _svcod; }

        /** @brief pcvall first map key (antenna/satellite prn). */
        string pcvkey() const
        {
            if (is_transmitter())
                return ident();
            else
                return anten();
        }

        /** @brief pcvall second map key (individual/type callibration). */
        string pcvtyp() const
        {
            if (is_transmitter())
                return "";
            else
                return ident();
        }

        /** @brief set/get method of callibration. */
        void method(const string &s) { _method = s; }
        const string &method() const { return _method; }

        /** @brief set/get source of callibration. */
        void source(const string &s) { _source = s; }
        const string &source() const { return _source; }

        /** @brief set/get sinex code. */
        void snxcod(const string &s) { _snxcod = s; }
        const string &snxcod() const { return _snxcod; }

        /** @brief set/get valid from. */
        void beg(const t_gtime &t) { _beg = t; }
        const t_gtime &beg() const { return _beg; }

        /** @brief set/get valid until. */
        void end(const t_gtime &t) { _end = t; }
        const t_gtime &end() const { return _end; }

        /** @brief set/get azimuth sampling. */
        void dazi(const double &d) { _dazi = d; }
        const double &dazi() const { return _dazi; }

        /** @brief set/get zenith sampling. */
        void dzen(const double &d) { _dzen = d; }
        const double &dzen() const { return _dzen; }

        /** @brief set/get zenith1. */
        void zen1(const double &d) { _zen1 = d; }
        const double &zen1() const { return _zen1; }

        /** @brief set/get zenith2. */
        void zen2(const double &d) { _zen2 = d; }
        const double &zen2() const { return _zen2; }

        /** @brief correct satellite coord. */
        double pco(const double &zen, const double &azi, const GFRQ &f); 

        // for gnss muti-freqs
        // ===========================================================================
        int pcoS(t_gsatdata &satdata, t_gtriple &pco, OBSCOMBIN lc, GOBSBAND &b1, GOBSBAND &b2);
        int pcoR(t_gsatdata &satdata, t_gtriple &pco, OBSCOMBIN lc, GOBSBAND &b1, GOBSBAND &b2);

        /** @brief pcb correction - satellite (nadir). */
        int pcvS(double &corr, t_gsatdata &satdata, OBSCOMBIN lc, GOBSBAND &b1, GOBSBAND &b2, t_gtriple &site);

        /** @brief pcv correction - site (zenith). */
        int pcvR(double &corr, t_gsatdata &satdata, OBSCOMBIN lc, GOBSBAND &b1, GOBSBAND &b2);
        int pcoS_cmb(t_gsatdata &satdata, t_gtriple &pco, GOBSBAND &b1, GOBSBAND &b2);
        int pcoR_cmb(t_gsatdata &satdata, t_gtriple &pco, GOBSBAND &b1, GOBSBAND &b2);

        /** @brief pcb correction - satellite (nadir). */
        int pcvS_cmb(double &corr, t_gsatdata &sat, GOBSBAND &b1, GOBSBAND &b2, t_gtriple &site);

        /** @brief pcv correction - site (zenith). */
        int pcvR_cmb(double &corr, t_gsatdata &sat, GOBSBAND &b1, GOBSBAND &b2);

        int pcoS_raw(t_gsatdata &satdata, t_gtriple &pco, GOBSBAND &b1);
        int pcoR_raw(t_gsatdata &satdata, t_gtriple &pco, GOBSBAND &b1);

        /** @brief pcb correction - satellite (nadir). */
        int pcvS_raw(double &corr, t_gsatdata &sat, GOBSBAND &b1, t_gtriple &site);
        int pcvR_raw(double &corr, t_gsatdata &sat, GOBSBAND &b1);

        // ===========================================================================

        /** @brief pco correction - satellite. */
        int pcoS(t_gsatdata &satdata, t_gtriple &pco, GOBS_LC lc, GOBSBAND k1, GOBSBAND k2);
        int pcoR(t_gsatdata &satdata, t_gtriple &pco, GOBS_LC lc, GOBSBAND k1, GOBSBAND k2);

        /** @brief pco correction - receiver. */
        int pcoR(t_gsatdata &satdata, t_gtriple &dx, t_gtriple &site, GOBS_LC lc);

        /** @brief pco projection into LoS. */
        int pco_proj(double &corr, t_gsatdata &satdata, t_gtriple &site, t_gtriple &dx);

        /** @brief pcb correction - satellite (nadir). */
        int pcvS(double &corr, t_gsatdata &sat, t_gtriple &site, GOBS_LC lc, GOBSBAND k1, GOBSBAND k2);

        /** @brief pcv correction - site (zenith). */
        int pcvR(double &corr, t_gsatdata &sat, GOBS_LC lc, GOBSBAND k1, GOBSBAND k2);

        /** @brief set/get PCO. */
        void pco(GFRQ f, const t_gtriple &t) { _mappco[f] = t; }
        t_gtriple pco(GFRQ f) { return _mappco[f]; } 
        t_map_pco pco() const { return _mappco; }

        /** @brief set/get PCO zen. */
        void pcvzen(GFRQ f, const t_map_Z &t) { _mapzen[f] = t; }
        t_map_Z pcvzen(GFRQ f) { return _mapzen[f]; }
        t_map_zen pcvzen() const { return _mapzen; }

        /** @brief set/get PCO aiz. */
        void pcvazi(GFRQ f, const t_map_A &t) { _mapazi[f] = t; }
        t_map_A pcvazi(GFRQ f) { return _mapazi[f]; } 
        t_map_azi pcvazi() const { return _mapazi; }
        void is_noazi(bool b) { _pcv_noazi = b; }

    private:
        /** @brief Does the calibration contain azi-depenedant data. */
        bool _azi_dependent(GFRQ f);

        t_gephplan _ephplan;

        bool _trans;   ///< transmitter[true], receiver[false]
        string _anten; ///< antenna type
        string _ident; ///< antenna identification
        string _svcod; ///< svn code

        string _method; ///< method of calibration
        string _source; ///< source of calibration
        string _snxcod; ///< sinex code
        t_gtime _beg;   ///< valid from
        t_gtime _end;   ///< valid until
        double _dazi;   ///< azimuth sampling
        double _dzen;   ///< zenith sampling
        double _zen1;   ///< zenith1
        double _zen2;   ///< zenith2

        t_map_pco _mappco; ///< map of PCOs (all frequencies)
        t_map_zen _mapzen; ///< map of NOAZI values (all frequencies)
        t_map_azi _mapazi; ///< map of AZI-dep values (all frequencies)

        bool _pcv_noazi;
    };

} // namespace

#endif
