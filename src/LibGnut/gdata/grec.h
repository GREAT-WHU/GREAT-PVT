/**
*
* @verbatim
    History
    2011-01-10  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       grec.h
* @brief
*.
* @author     JD
* @version    1.0.0
* @date       2011-01-10
*
*/

#ifndef GREC_H
#define GREC_H

#include "gexport/ExportLibGnut.h"
#include <stdio.h>
#include <string>

#ifdef BMUTEX
#include <boost/thread/mutex.hpp>
#endif

#include "gdata/gobj.h"
#include "gdata/gdata.h"
#include "gdata/grnxhdr.h"

using namespace std;

namespace gnut
{

    /** @brief class for grec. */
    class LibGnut_LIBRARY_EXPORT t_grec : public t_gobj
    {

    public:
        /** @brief default constructor. */
        t_grec();
        //   t_grec(const t_grec& obj);
        t_grec(t_spdlog spdlog);

        /** @brief default destructor. */
        virtual ~t_grec();

        /** @brief map rec. */
        typedef map<t_gtime, string> t_maprec;

        /** @brief map header. */
        typedef map<t_gtime, t_rnxhdr> t_maphdr;

        /** @brief add rinex header. */
        virtual void addhdr(const t_rnxhdr &hdr, const t_gtime &epo, string path);

        /** @brief change rinex header. */
        void changehdr(const t_rnxhdr &hdr, const t_gtime &epo, string path);

        /** @brief get all rinex headr. */
        t_maphdr gethdr();

        /** @brief get one rinex headr. */
        t_rnxhdr gethdr(const t_gtime &epo);

        /** @brief get maprec. */
        t_maprec get_maprec() { return _maprec; }

        /** @brief set receiver name. */
        void rec(string rec, const t_gtime &beg, const t_gtime &end = LAST_TIME);

        /** @brief get receiver name. */
        string rec(const t_gtime &t) const; // set/get receiver

        /** @brief return validity for receiver at epoch t. */
        void rec_validity(const t_gtime &t, t_gtime &beg, t_gtime &end) const;

        /** @brief return isrec. */
        virtual bool isrec() override { return true; }

        /** @brief return istrn. */
        virtual bool istrn() override { return false; }

        /** @brief check consistency. */
        virtual void compare(shared_ptr<t_grec> grec, const t_gtime &tt, string source);

        /** @brief get time tags. */
        virtual vector<t_gtime> rec_id() const;

        /** @brief get time tags. */
        void fill_rnxhdr(const t_rnxhdr &rnxhdr);

    protected:
        /** @brief fill data members form rinex header. */
        void _fill_rnxhdr(const t_rnxhdr &rnxhdr);

        /** @brief get one rinex headr. */
        t_rnxhdr _gethdr(const t_gtime &epo);

        /** @brief set receiver name. */
        void _rec(string rec, const t_gtime &beg, const t_gtime &end = LAST_TIME);

        /** @brief get receiver name (>=t). */
        string _rec(const t_gtime &t) const;

        t_maprec _maprec; ///< map of receviers
        t_maphdr _maphdr; ///< map of rinex header information

    private:
    };

} // namespace

#endif
