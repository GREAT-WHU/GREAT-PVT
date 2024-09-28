/**
*
* @verbatim
    History
    2011-01-10  JD: created
    2019-04-05  HJ_Zheng: add partial function for orbit

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gobj.h
* @brief      Purpose: implements gobject class
*.
* @author     JD
* @version    1.0.0
* @date       2011-01-10
*
*/
#ifndef GOBJ_H
#define GOBJ_H

#include "gexport/ExportLibGnut.h"
#include <stdio.h>
#include <string>
#include <memory>

#ifdef BMUTEX
#include <boost/thread/mutex.hpp>
#endif

#include "gdata/gdata.h"
#include "gdata/grnxhdr.h"
#include "gall/gallpcv.h"
#include "gmodels/gpcv.h"
#include "gutils/gtriple.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_gobj. */
    class LibGnut_LIBRARY_EXPORT t_gobj : public t_gdata
    {

    public:
        /** @brief default constructor. */
        explicit t_gobj();

        /** @brief default constructor. */
        explicit t_gobj(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_gobj();

        typedef map<t_gtime, shared_ptr<t_gpcv>> t_mappcv;
        typedef map<t_gtime, t_gtriple> t_mapecc_xyz;
        typedef map<t_gtime, t_gtriple> t_mapecc_neu;
        typedef map<t_gtime, pair<t_gtriple, t_gtriple>> t_mapcrd;
        typedef map<t_gtime, string> t_mapant;

        /** @brief set/get id (uniq internal id). */
        virtual void id(string str);
        /**
         * @brief 
         * 
         * @return string 
         */
        virtual string id() const;

        /** @brief set/get name (name). */
        virtual void name(string str);
        /**
         * @brief 
         * 
         * @return string 
         */
        virtual string name() const;

        /** @brief set/get name (domes). */
        virtual void domes(string str);
        virtual string domes() const;

        /** @brief set/get description (full name). */
        virtual void desc(string str);

         /** @brief get description. */
        virtual string desc() const;

         /** @set ecc offsets w.r.t.center of mass / reference point. */
        virtual void eccxyz(const t_gtriple &ecc, const t_gtime &beg, const t_gtime &end = LAST_TIME);

        /** @brief set/get ecc offsets (>=t) w.r.t. center of mass/reference point. */
        virtual t_gtriple eccxyz(const t_gtime &t) const;

        /**
         * @brief return validity for eccxyx at epoch t
         * 
         * @param t 
         * @param beg 
         * @param end 
         */
        virtual void eccxyz_validity(const t_gtime &t, t_gtime &beg, t_gtime &end) const;

        /**
         * @brief set ecc offsets w.r.t. center of mass/reference point
         * 
         * @param ecc 
         * @param beg 
         * @param end 
         */
        virtual void eccneu(const t_gtriple &ecc, const t_gtime &beg, const t_gtime &end = LAST_TIME);

        /** @brief set/get ecc offsets (>=t) w.r.t. center of mass/reference point. */
        virtual t_gtriple eccneu(const t_gtime &t) const;

        /**
         * @brief return validity for eccneu at epoch t
         * 
         * @param t 
         * @param beg 
         * @param end 
         */
        virtual void eccneu_validity(const t_gtime &t, t_gtime &beg, t_gtime &end) const;

        /**
         * @brief set pcv element
         * 
         * @param pcv 
         * @param beg 
         * @param end 
         */
        virtual void pcv(shared_ptr<t_gpcv> pcv, const t_gtime &beg, const t_gtime &end = LAST_TIME);

        /** @brief set/get external pointer to pcv element (>=t). */
        virtual shared_ptr<t_gpcv> pcv(const t_gtime &t) const;

        /**
         * @brief set antenna name
         * 
         * @param ant 
         * @param beg 
         * @param end 
         */
        virtual void ant(string ant, const t_gtime &beg, const t_gtime &end = LAST_TIME);

        /** @brief set/get antenna. */
        virtual string ant(const t_gtime &t) const;

        /**
         * @brief return validity for antenna at epoch t
         * 
         * @param t 
         * @param beg 
         * @param end 
         */
        virtual void ant_validity(const t_gtime &t, t_gtime &beg, t_gtime &end) const;

        /** @brief get the value of channel. */
        virtual void channel(int chk){};
        virtual int channel() const { return 255; };

        /**
         * @brief set object position
         * 
         * @param crd 
         * @param std 
         * @param beg 
         * @param end 
         * @param overwrite 
         */
        virtual void crd(const t_gtriple &crd, const t_gtriple &std, const t_gtime &beg, const t_gtime &end = LAST_TIME, bool overwrite = false);

        /** @brief set crd & std. */
        virtual void crd(const t_gtriple &crd, const t_gtriple &std);

        /** @brief get crd of Marker point. */
        virtual t_gtriple crd(const t_gtime &t) const;

        /** @brief get std of crd. */
        virtual t_gtriple std(const t_gtime &t) const;

        /** @brief get crd of ARP (ARP = MARKER + ECC). */
        virtual t_gtriple crd_arp(const t_gtime &t) const;

        /**
         * @brief return validity for crd at epoch t
         * 
         * @param t 
         * @param beg 
         * @param end 
         */
        virtual void crd_validity(const t_gtime &t, t_gtime &beg, t_gtime &end) const;

        /**
         * @brief Get the recent crd object
         * 
         * @param t 
         * @param ref_std 
         * @param crd 
         * @param std 
         * @return true 
         * @return false 
         */
        virtual bool get_recent_crd(const t_gtime &t, const double &ref_std, t_gtriple &crd, t_gtriple &std);

        /** @brief get adjacent snx crd. */
        virtual bool get_adjacent_crd(const t_gtime &t, const double &ref_std, t_gtriple &crd, t_gtriple &std);

        /** @brief get time tags. */
        virtual vector<t_gtime> pcv_id() const;

        /**
         * @brief get time tags
         * 
         * @return vector<t_gtime> 
         */
        virtual vector<t_gtime> ant_id() const;

        /**
         * @brief get time tags
         * 
         * @return vector<t_gtime> 
         */
        virtual vector<t_gtime> crd_id() const;

        /**
         * @brief check consistency
         * 
         * @param gobj 
         * @param tt 
         * @param source 
         */
        virtual void compare(shared_ptr<t_gobj> gobj, const t_gtime &tt, string source);

        /** @brief get the value of isrec. */
        virtual bool isrec() = 0;

        /** @brief get the value of istrn. */
        virtual bool istrn() = 0;

        /**
         * @brief get overwrite
         * 
         * @return true 
         * @return false 
         */
        bool overwrite();
        void overwrite(bool overwrite);

        /**
         * @brief set mappcv for all t_gobj
         * 
         * @param pcvs 
         */
        virtual void sync_pcv(t_gallpcv *pcvs);

        /**
         * @brief 
         * 
         * @param t 
         * @return true 
         * @return false 
         */
        virtual bool operator<(const t_gobj &t) const;

        /**
         * @brief 
         * 
         * @param t 
         * @return true 
         * @return false 
         */
        virtual bool operator==(const t_gobj &t) const;

    protected:
        string _id;              ///< object id (internal)
        string _name;            ///< object name
        string _domes;           ///< object domes
        string _desc;            ///< object description (full name)
        t_mapcrd _mapcrd;        ///< object position
        t_mappcv _mappcv;        ///< map of pco+pcv
        t_mapecc_xyz _mapeccxyz; ///< map of xyz eccentricities (to center of mass or reference point)
        t_mapecc_neu _mapeccneu; ///< map of neu eccentricities (to center of mass or reference point)
        t_mapant _mapant;        ///< map of antennas + dome

        bool _overwrite;

        // source for public (mutexed) interfaces
        shared_ptr<t_gpcv> _pcv(const t_gtime &t) const;
        void _pcv(shared_ptr<t_gpcv> pcv, const t_gtime &beg, const t_gtime &end = LAST_TIME);
        void _ant(string ant, const t_gtime &beg, const t_gtime &end = LAST_TIME);
        string _ant(const t_gtime &t) const;
        vector<t_gtime> _ant_id() const;
        void _crd(const t_gtriple &crd, const t_gtriple &std, const t_gtime &beg, const t_gtime &end = LAST_TIME, bool overwrite = false); ///< overwrite for solution must over write; by ZHJ
        t_gtriple _crd(const t_gtime &t) const;
        t_gtriple _std(const t_gtime &t) const;
        void _eccxyz(const t_gtriple &ecc, const t_gtime &beg, const t_gtime &end = LAST_TIME);
        t_gtriple _eccxyz(const t_gtime &t) const;
        void _eccneu(const t_gtriple &ecc, const t_gtime &beg, const t_gtime &end = LAST_TIME);
        t_gtriple _eccneu(const t_gtime &t) const;
        shared_ptr<t_gpcv> _pcvnull;

    private:
    };

} // namespace

#endif
