
/**
* @verbatim
    History
    2012-09-26  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gobs.h
* @brief       Purpose: definition of GNSS observation types
*.
* @author      JD
* @version     1.0.0
* @date        2012-09-26
*
*/

#ifndef GOBS_H
#define GOBS_H

#include <map>
#include <list>
#include <vector>
#include <string>
#ifdef BMUTEX
#include <boost/thread/mutex.hpp>
#endif

#include "gutils/gnss.h"
#include "gexport/ExportLibGnut.h"

using namespace std;

namespace gnut
{
    /** @brief class for t_gattr. */
    class LibGnut_LIBRARY_EXPORT t_gattr
    {

    public:
        /** @brief constructor 1. */
        t_gattr() { _gattr = ATTR; };

        /** @brief constructor 2. */
        explicit t_gattr(const GOBSATTR &a) { _gattr = a; };

        /** @brief default destructor. */
        ~t_gattr(){};

        /** @brief set attr. */
        virtual void attr(const GOBSATTR &a);

        /** @brief get attr. */
        virtual GOBSATTR attr() const;

        /** @brief override operator ==. */
        virtual bool operator==(const t_gattr &g) const;

        /** @brief get valid. */
        virtual bool valid() const;

    protected:
        GOBSATTR _gattr; ///< gnss attr
    };

    /** @brief class for t_gband derive from t_gattr. */
    class LibGnut_LIBRARY_EXPORT t_gband : public t_gattr
    {

    public:
        /** @brief constructor 1. */
        t_gband() : t_gattr() { _gband = BAND; };

        /** @brief constructor 2. */
        t_gband(GOBSBAND b, GOBSATTR a) : t_gattr(a) { _gband = b; };

        /** @brief default destructor. */
        virtual ~t_gband(){};

        /** @brief set band. */
        virtual void band(const GOBSBAND &g);

        /** @brief get band. */
        virtual GOBSBAND band() const;

        /** @brief set t_gattr. */
        virtual void gattr(const t_gattr &g);

        /** @brief get t_gattr. */
        virtual t_gattr gattr() const;

        /** @brief override operator ==. */
        virtual bool operator==(const t_gband &g) const;

        /** @brief get valid. */
        virtual bool valid() const override;

    protected:
        GOBSBAND _gband; ///< gnss band
    };

    /** @brief class for t_gobs derive from t_gattr. */
    class LibGnut_LIBRARY_EXPORT t_gobs : public t_gband
    {

    public:
        /** @brief constructor 1. */
        t_gobs() : t_gband() { _gtype = TYPE; };

        /** @brief constructor 2. */
        t_gobs(GOBSTYPE t, GOBSBAND b, GOBSATTR a) : t_gband(b, a) { _gtype = t; };

        /** @brief constructor 3. */
        explicit t_gobs(const GOBS &g) { gobs(g); };

        /** @brief default destructor. */
        virtual ~t_gobs(){};

        /** @brief set type (only, inherit band&attr!). */
        virtual void type(const GOBSTYPE &t);

        /** @brief get type. */
        virtual GOBSTYPE type() const;

        /** @brief set attr. */
        virtual void gband(const t_gband &g);

        /** @brief get attr. */
        virtual t_gband gband() const;

        /** @brief set type (only! inherit). */
        int gobs(const GOBS &g);
        int gobs(const string &s);

        /** @brief get gobs enum. */
        GOBS gobs() const;

        /** @brief get 2char gobs (only C1/C2 and P1/P2 and L1/L2). */
        GOBS gobs2CH(GSYS gs) const;

        /** @brief get 3char gobs ( ). */
        GOBS gobs3CH() const;

        /** @brief change obs from 2 to 3. */
        void gobs2to3(GSYS gs);

        /** @brief override operator ==. */
        bool operator==(const t_gobs &g) const;

        /** @brief valid. */
        bool valid() const override;

        /** @brief get is_code. */
        bool is_code() const;

        /** @brief get is_phase. */
        bool is_phase() const;

        /** @brief get is_doppler. */
        bool is_doppler() const; 

    protected:
        GOBSTYPE _gtype; ///< gtype
    };

} // namespace

#endif // GOBS_H
