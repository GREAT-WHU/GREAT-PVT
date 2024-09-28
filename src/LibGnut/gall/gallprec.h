/**
*
* @verbatim
History
*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     gallprec.h
* @brief    Purpose: implementation of SP3 ephemeris container
*
*
* @author   JD
* @version  1.0.0
* @date     2011-01-10
*
*/

#ifndef GALLPREC_H
#define GALLPREC_H

#include "gall/gallnav.h"
#include "gdata/gephprec.h"
#include "gutils/gtriple.h"
#include "gmodels/gpoly.h"

using namespace std;

namespace gnut
{
    /** @brief single data for a single satellite. */
    typedef map<string, double> t_map_dat;     

    /** @brief all data for a single satellite. */
    typedef map<t_gtime, t_map_dat> t_map_epo; 

    /** @brief all data for all satellites. */
    typedef map<string, t_map_epo> t_map_prn;  

    /**
    *@brief Class for t_gallprec derive from t_gallnav
    */
    class LibGnut_LIBRARY_EXPORT t_gallprec : public t_gallnav
    {
    public:
        /** @brief clk type. */
        enum clk_type
        {
            AS,   ///< sitellite
            AR,   ///< receiver
            UNDEF ///< undefined
        };

    public:
        /** @brief default constructor. */
        t_gallprec();

        /**
         * @brief Construct a new t gallprec object
         * 
         * @param spdlog 
         */
        t_gallprec(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gallprec();

        /** @brief map sp3. */
        typedef map<string, shared_ptr<t_gephprec>> t_map_sp3; 

        /**
         * @brief inherited from gallnav to fix healthy
         * 
         * @param sat 
         * @param t 
         * @return true 
         * @return false 
         */
        bool health(const string &sat, const t_gtime &t) override; 

        /**
         * @brief pos
         * 
         * @param sat 
         * @param t 
         * @param xyz 
         * @param var 
         * @param vel 
         * @param chk_mask 
         * @return int 
         */
        int pos(const string &sat, const t_gtime &t, double xyz[3], double var[3] = NULL, double vel[3] = NULL, const bool &chk_mask = true); 

        /**
         * @brief  GNAV quality
         * 
         * @param sat 
         * @param t 
         * @param xyz 
         * @param var 
         * @param vel 
         * @param chk_mask 
         * @return int 
         */
        int nav(const string &sat, const t_gtime &t, double xyz[3], double var[3] = NULL, double vel[3] = NULL, const bool &chk_mask = true) override; 

        /**
         * @brief clk
         * 
         * @param sat 
         * @param t 
         * @param clk 
         * @param var 
         * @param dclk 
         * @param chk_mask 
         * @return int 
         */
        int clk(const string &sat, const t_gtime &t, double *clk, double *var = NULL, double *dclk = NULL, const bool &chk_mask = true) override; 

        /**
         * @brief clk_int
         * 
         * @param sat 
         * @param t 
         * @param clk 
         * @param var 
         * @param dclk 
         * @return int 
         */
        int clk_int(const string &sat, const t_gtime &t, double *clk, double *var = NULL, double *dclk = NULL); // [s]

        /**
         * @brief add interval
         * 
         * @param sat 
         * @param intv 
         */
        void add_interval(const string &sat, int intv);

        /**
         * @brief add agency
         * 
         * @param producer 
         */
        void add_agency(const string &producer);

        /**
         * @brief add vel
         * 
         * @param sat 
         * @param ep 
         * @param xyzt 
         * @param var 
         * @return int 
         */
        int addvel(const string &sat, const t_gtime &ep, double xyzt[4], double var[4]);

        /**
         * @brief add clk
         * 
         * @param sat 
         * @param ep 
         * @param clk 
         * @param var 
         * @return * int 
         */
        int addclk(const string &sat, const t_gtime &ep, double clk[3], double var[3]);

        /**
         * @brief add clk_tri
         * 
         * @param sat 
         * @param ep 
         * @param clk 
         * @param var 
         * @return * int 
         */
        int addclk_tri(const string &sat, const t_gtime &ep, double clk[3], double var[3]);

        /**
         * @brief add pos
         * 
         * @param sat 
         * @param ep 
         * @param xyz 
         * @param t 
         * @param dxyz 
         * @param dt 
         * @return int 
         */
        int addpos(const string &sat, const t_gtime &ep, const t_gtriple &xyz, const double &t, const t_gtriple &dxyz, const double &dt); 

        /**
         * @brief use clksp3
         * 
         * @param b 
         */
        void use_clksp3(const bool &b) { _clksp3 = b; }

        /**
         * @brief use clknav
         * 
         * @param b 
         */
        void use_clknav(const bool &b) { _clknav = b; }

        /**
         * @brief use posnav
         * 
         * @param b 
         */
        void use_posnav(const bool &b) { _posnav = b; }

        /**
         * @brief clean outer
         * 
         * @param beg 
         * @param end 
         */
        void clean_outer(const t_gtime &beg = FIRST_TIME, const t_gtime &end = LAST_TIME) override;

        /**
         * @brief get all satellites
         * 
         * @return set<string> 
         */
        virtual set<string> satellites() const override; 

        /**
         * @brief get number of epochs
         * 
         * @param prn 
         * @return unsigned int 
         */
        virtual unsigned int nepochs(const string &prn) override;

    protected:
        /**
         * @brief find appropriate t_geph element
         * 
         * @param sat 
         * @param t 
         * @return shared_ptr<t_geph> 
         */
        virtual shared_ptr<t_geph> _find(const string &sat, const t_gtime &t); 

        /**
         * @brief fill PT,X,Y,Z vectors
         * 
         * @param sat 
         * @param t 
         * @return int 
         */
        virtual int _get_crddata(const string &sat, const t_gtime &t); 

        /**
         * @brief fill CT,C vectors
         * 
         * @param sat 
         * @param t 
         * @return int 
         */
        virtual int _get_clkdata(const string &sat, const t_gtime &t); 

        /**
         * @brief return the ssr position and velocity correction.
         * 
         * @param sat 
         * @param t 
         * @param iod 
         * @param tRef 
         * @param orbcorr 
         * @return int 
         */
        virtual int _get_delta_pos_vel(const string &sat, const t_gtime &t, int iod, t_gtime &tRef, t_map_dat &orbcorr);

        /**
         * @brief 
         * 
         * @param sat 
         * @param t 
         * @param iod 
         * @param tRef 
         * @param clkcorr 
         * @return int 
         */
        virtual int _get_delta_clk(const string &sat, const t_gtime &t, int iod, t_gtime &tRef, t_map_dat &clkcorr);

        t_map_sat _mapprec; // map of sp3 polynomials
        t_map_prn _mapsp3;  // precise orbits&clocks (SP3) - full discrete data sets
        t_map_prn _mapclk;  // precise clocks (CLOCK-RINEX) - full discrete data sets

    private:
        t_map_sp3 _prec;          ///< CACHE: single SP3 precise ephemeris for all satellites
        unsigned int _degree_sp3; ///< polynom degree for satellite sp3 position and clocks
        double _sec;              ///< default polynomial units
        t_gtime _ref;             ///< selected reference epoch for crd data/polynomials
        t_gtime _clkref;          ///< selected reference epoch for clk data/polynomials
        bool _clkrnx;             ///< true: use               clk from Rinex Clocks
        bool _clksp3;             ///< true: use alternatively clk from sp3 (~15min!)
        bool _clknav;             ///< true: use alternatively nav (low-precise clocks)
        bool _posnav;             ///< true: use alternatively nav (low-precise orbits)
        map<string, int> _intvm;
        string _agency;
        t_gtime _tbeg;
        t_gtime _tend, _tend_clk, _tend_sp3;
        // CACHE for approximative solutions
        map<string, t_gtime> _poly_beg;
        map<string, t_gtime> _poly_end;
        map<string, t_gpoly> _poly_x;
        map<string, t_gpoly> _poly_y;
        map<string, t_gpoly> _poly_z;

        // BEGIN OF TEMPORARY (ALTERNATIVE for direct interpolation)
        vector<double> _PT; ///< vector of time-difference (X for polynomials)
        vector<t_gtime> _T; ///< vector of full time       (X for polynomials)
        vector<double> _X;  ///< vector of x-coordinate    (Y for polynomials)
        vector<double> _Y;  ///< vector of y-coordinate    (Y for polynomials)
        vector<double> _Z;  ///< vector of z-coordinate    (Y for polynomials)

        vector<double> _CT;      ///< vector of time-difference (X for polynomials)
        vector<double> _C;       ///< vector of clk correction  (Y for polynomials)
        vector<double> _IFCB_F3; ///< vector of ifcb_f3 

        set<clk_type> _clk_type_list; ///< CLK TYPE
    };

} // namespace

#endif
