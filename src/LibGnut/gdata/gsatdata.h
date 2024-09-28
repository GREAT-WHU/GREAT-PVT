/**
*
* @verbatim
    History
    2014-04-18 /PV: created
    2018-09-28 /JD: revised

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gsatdata.h
* @brief      Purpose: statistical function (1D)
*.
* @author     PV
* @version    1.0.0
* @date       2014-04-18
*
*/

#ifndef GSATDATA_H
#define GSATDATA_H

#include <string>
#include <vector>
#include "newmat/newmat.h"
#include "gdata/gdata.h"
#include "gdata/gobsgnss.h"
#include "gutils/gtime.h"
#include "gutils/gtriple.h"
#include "gall/gallnav.h"
#include "gset/gsetproc.h"
#include "gall/gallprec.h"

using namespace std;

namespace gnut
{
    /** @brief class for satellite data. */
    class LibGnut_LIBRARY_EXPORT t_gsatdata : public t_gobsgnss
    {

    public:
        /** @brief default constructor. */
        t_gsatdata();

        t_gsatdata(t_spdlog spdlog);
        /** @brief constructor 1. */
        t_gsatdata(t_spdlog spdlog,
                   const string &site,
                   const string &sat,
                   const t_gtime &t);

        /** @brief copy constructor 3. */
        explicit t_gsatdata(const t_gobsgnss &obs);

        /** @brief default destructor. */
        virtual ~t_gsatdata();

        /** @brief add satellite position. */
        void addcrd(const t_gtriple &crd);

        /** @brief add satellite position in crs. */
        void addcrdcrs(const t_gtriple &crd);

        /** @brief add satellite pco. */
        void addpco(const t_gtriple &pco);

        /** @brief add satellite velocity. */
        void addvel(const t_gtriple &vel);

        /** @brief add by glfeng. */
        void addvel_crs(const t_gtriple &vel);

        /** @brief add satellite clocks at the transmision time. */
        void addclk(const double &d);

        /** @brief add satellite relative delay. */
        void addreldelay(const double &d);

        /** @brief add add the TRS2CRS Matrix. */
        void addSCF2CRS(const Matrix &scf2crs, const Matrix &scf2trs);

        /** @brief add add the rate. */
        void adddrate(const double &drate);

        /** @brief add add the rec Time. */
        void addrecTime(const t_gtime &recTime);

        /** @brief add add the sat Time. */
        void addsatTime(const t_gtime &satTime);

        /** @brief determine wheather the satellite is eclipsed (postshadow is considered). */
        void addecl(map<string, t_gtime> &lastEcl);

        /** @brief reset the eclipsing time. */
        void setecl(const bool &ecl);

        /** @brief true  to support PPP/SPP (set by INP:chk_health). */
        int addprd(t_gallnav *gnav, const bool &corrTOT = true, const bool &msk_health = true);

        /** @brief false to support QC (combines INP:chk_health+QC:use_health for Anubis). */
        int addprd_nav(t_gallnav *gnav, const bool &corrTOT = true, const bool &msk_health = false);

        /** @brief add satellite elevation. */
        void addele(const double &d);

        /** @brief add satellite elevation. */
        void addele_leo(const double &d);

        /** @brief add receiver azimuth. */
        void addazi_rec(const double &d);

        /** @brief add receiver zenith. */
        void addzen_rec(const double &d);

        /** @brief add satellite-side azimuth ang. */
        void addazi_sat(const double &d);

        /** @brief add satellite-side zenith ang*/
        void addzen_sat(const double &d);

        /** @brief add satellite-side nadir angle. */
        void addnadir(const double &d);

        /** @brief add satellite rho-vector. */
        void addrho(const double &d);

        /** @brief add res. */
        void addres(const RESIDTYPE &restype, const GOBSTYPE &type, const double &res);

        /** @brief add mfH. */
        void addmfH(const double &d);

        /** @brief add mfW. */
        void addmfW(const double &d);

        /** @brief add mfG. */
        void addmfG(const double &d);

        /** @brief add wind. */
        void addwind(const double &d);

        /** @brief get satellite position. */
        const t_gtriple &satcrd() const;

        /**
         * @brief sat vel
         * 
         * @return const t_gtriple& 
         */
        const t_gtriple &satvel() const;

        /** @brief get satellite clocks at the transmision time. */
        const double &clk() const;

        /** @brief get satellite clocks drift at the transmision time. */
        const double &dclk() const;

        /**
         * @brief get sat index
         * 
         * @return int 
         */
        int satindex(); 

        /**
         * @brief drate
         * 
         * @return const double& 
         */
        const double &drate() const;

        /**
         * @brief get reciver coord
         * 
         * @return const t_gtriple& 
         */
        const t_gtriple &reccrd() const; 

        /**
         * @brief get reciver coord
         * 
         * @return const t_gtriple& 
         */
        const t_gtriple &sat2reccrs() const; 

        /**
         * @brief get satellite funct
         * 
         * @return const Matrix& 
         */
        const Matrix &orbfunct() const; 

        /**
         * @brief rotmat
         * 
         * @return const Matrix& 
         */
        const Matrix &rotmat() const;

        /**
         * @brief drdxpole
         * 
         * @return const Matrix& 
         */
        const Matrix &drdxpole() const;

        /**
         * @brief drdypole
         * 
         * @return const Matrix& 
         */
        const Matrix &drdypole() const;

        /**
         * @brief drdut1
         * 
         * @return const Matrix& 
         */
        const Matrix &drdut1() const;

        /**
         * @brief get the site receiver time
         * 
         * @return const t_gtime& 
         */
        const t_gtime &recTime() const;

        /**
         * @brief get satellite elevation
         * 
         * @return const double& 
         */
        const double &ele() const; 

        /**
         * @brief get satellite elevation
         * 
         * @return const double& 
         */
        const double &ele_leo() const; 

        /**
         * @brief get satellite elevation [deg]
         * 
         * @return double 
         */
        double ele_deg() const; 

        /**
         * @brief get satellite elevation [deg]
         * 
         * @return double 
         */
        double ele_leo_deg() const;

        /**
         * @brief get satellite azimuth
         * 
         * @return const double& 
         */
        const double &azi() const; 

        /**
         * @brief get satellite-side azimuth ang
         * 
         * @return const double& 
         */
        const double &azi_sat() const; 

        /**
         * @brief get satellite rho-vector
         * 
         * @return const double& 
         */
        const double &rho() const; 

        /**
         * @brief get eclipsing
         * 
         * @return true 
         * @return false 
         */
        const bool &ecl() const; 

        /**
         * @brief get stored wind up
         * 
         * @return double 
         */
        const double &wind() const;

        /**
         * @brief add slip
         * 
         * @param flag 
         */
        void addslip(const bool &flag);

        /**
         * @brief islip
         * 
         * @return true 
         * @return false 
         */
        const bool &islip() const;

        /**
         * @brief Sun elevation relative to orbital plane
         * 
         * @return double 
         */
        double beta(); 

        /**
         * @brief Orbit angle
         * 
         * @return double 
         */
        double orb_angle(); 

        /**
         * @brief get yaw angle
         * 
         * @return double 
         */
        const double &yaw() const { return _yaw; } 

        /**
         * @brief set yaw angle
         * 
         * @param yaw 
         */
        void yaw(const double &yaw) { _yaw = yaw; } 

        /**
         * @brief get postfit residuals
         * 
         * @param restype 
         * @param type 
         * @return vector<double> 
         */
        vector<double> residuals(const RESIDTYPE &restype, const GOBSTYPE &type);

        /**
         * @brief clean residuals
         * 
         * @param restype 
         */
        void clear_res(const RESIDTYPE &restype);

        /**
         * @brief clean data
         * 
         */
        void clear();

        /**
         * @brief valid
         * 
         * @return true 
         * @return false 
         */
        bool valid();

        /**
         * @brief is_carrier_range
         * 
         * @param band 
         * @return true 
         * @return false 
         */
        bool is_carrier_range(const GOBSBAND& band) const;

        t_gtriple _conf_crd;      ///< velocity direction vector
        t_gtriple _e;             ///< direction vector
        bool is_process = false;  ///< cmb_equ process flag

    private:
        /** @brief add satellite pos, clk and ecl (corrTOT is correction of transmition time). */
        int _addprd(t_gallnav *gnav, const bool &corrTOT = true, const bool &msk_health = true);

        /**
         * @brief Sun elevation relative to orbital plane
         * 
         * @return double 
         */
        double _b();

        /**
         * @brief Orbit angle
         * 
         * @return double 
         */
        double _orb_angle();

        /**
         * @brief clean internal function
         * 
         */
        virtual void _clear() override;

        /**
         * @brief valid
         * 
         * @return true 
         * @return false 
         */
        virtual bool _valid() const override;

        t_gtriple _satcrd;      ///< satellite position (X,Y,Z)
        t_gtriple _satcrdcrs;   ///< satellite position in crs(X,Y,Z)
        t_gtriple _satpco;      ///< satellite pco
        t_gtriple _satvel;      ///< satellite velocity
        t_gtriple _satvel_crs;  ///< satellite velocity_crs
        Matrix _orbfunct;       ///< satellite funct
        int _satindex;          ///< satellite index 
        t_gtriple _reccrd;      ///< reciver positoin(TRS)
        t_gtriple _reccrdcrs;   ///< reciver positoin(TRS)
        t_gtriple _sat2reccrs;  ///< reciver positoin(TRS)
        Matrix _rotmat;         ///< rotmat
        Matrix _drdxpole;       ///< drdxpole
        Matrix _drdypole;       ///< drdypole
        Matrix _drdut1;         ///< drdut1
        Matrix _scf2crs;        ///< scf2crs
        Matrix _scf2trs;        ///< scf2trs
        double _drate;          ///< drate
        t_gtriple _dloudx;      ///< dloudx
        t_gtime _TR;            ///< site recieve time
        t_gtime _TS;            ///< satellite sends
        double _clk = 0.0;      ///< satellite clocks (precise, time of transmision)
        double _dclk = 0.0;     ///< satellite clocks drift (precise, time of transmision)
        double _reldelay = 0.0; ///< satellite releative delay
        double _ele = 0.0;      ///< satellite elevation
        double _ele_leo = 0.0;  ///< satellite elevation
        double _azi_rec = 0.0;  ///< satellite azimuth
        double _azi_sat = 0.0;  ///< azimuth at satellite-side
        double _zen_rec = 0.0;  ///< satellite zenith
        double _zen_sat = 0.0;  ///< zenith at satellite-side
        double _nadir = 0.0;    ///< nadir angle at satellite-side
        double _rho = 0.0;      ///< satellite-station geometrical distance
        bool _eclipse = 0.0;    ///< eclipsed satellite
        double _mfH = 0.0;      ///< mfH
        double _mfW = 0.0;      ///< mfW
        double _mfG = 0.0;      ///< mfG
        double _wind = 0.0;     ///< wind
        bool _low_prec = 0.0;   ///< low precision of sat pos
        bool _slipf = 0.0;      ///< cycle slip flag
        map<GOBSBAND, bool> _is_carrier_range; ///< add to mark carrier-range
        bool _isExistAug = false;

        // normalized residuals
        vector<double> _code_res_norm;
        vector<double> _phase_res_norm;

        // original residuals
        vector<double> _code_res_orig;
        vector<double> _phase_res_orig;

        double _beta_val = 0.0;
        double _orb_angle_val = 0.0;
        double _yaw = 0.0;
    };

} // namespace

#endif
