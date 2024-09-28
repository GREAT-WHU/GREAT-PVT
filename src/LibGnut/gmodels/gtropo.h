
/**
*
* @verbatim
    History
    2011-01-10 /JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file       gtropo.h
* @brief      Purpose: implements troposphere model class
*.
* @author     JD
* @version    1.0.0
* @date       2011-01-10
*
*/

#ifndef GTROPO_H
#define GTROPO_H

#include "gutils/gtime.h"
#include "gutils/gtriple.h"
#include "gprod/gprodcrd.h"
#include "gmodels/ggpt.h"
#include "gexport/ExportLibGnut.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_gtropo. */
    class LibGnut_LIBRARY_EXPORT t_gtropo
    {
    public:
        //t_gtropo(string site);

        /** @brief default constructor. */
        t_gtropo();

        /** @brief default destructor. */
        virtual ~t_gtropo();

        /** @brief get ZHD. */
        virtual double getZHD(const t_gtriple &ell, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]

        /** @brief get ZWD. */
        virtual double getZWD(const t_gtriple &ell, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]

    protected:
        t_gpt _gpt; ///< gpt
    };

    /** @brief class for t_saast derive from t_gtropo. */
    class LibGnut_LIBRARY_EXPORT t_saast : public t_gtropo
    {
    public:
        /** @brief default constructor. */
        t_saast() {}

        /** @brief default destructor. */
        ~t_saast() {}

        /** @brief get STD/ZHD/ZWD. */
        virtual double getSTD(const double &ele, const double &hel);     // ! Radians: elevation
        virtual double getZHD(const t_gtriple &ell, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
        virtual double getZWD(const t_gtriple &ell, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
    };

    /** @brief class for t_davis derive from t_gtropo. */
    class LibGnut_LIBRARY_EXPORT t_davis : public t_gtropo
    {
    public:
        /** @brief default constructor. */
        t_davis() {}

        /** @brief default destructor. */
        ~t_davis() {}

        /** @brief get ZHD/ZWD. */
        virtual double getZHD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
        virtual double getZWD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
    };

    /** @brief class for t_hopf derive from t_gtropo. */
    class LibGnut_LIBRARY_EXPORT t_hopf : public t_gtropo
    {
    public:
        /** @brief default constructor. */
        t_hopf() {}

        /** @brief default destructor. */
        ~t_hopf() {}

        /** @brief get ZHD/ZWD. */
        virtual double getZHD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
        virtual double getZWD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
    };

    /** @brief class for t_baby derive from t_gtropo. */
    class LibGnut_LIBRARY_EXPORT t_baby : public t_gtropo
    {
    public:
        /** @brief default constructor. */
        t_baby() {}

        /** @brief default destructor. */
        ~t_baby() {}

        /** @brief get ZHD/ZWD. */
        virtual double getZHD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
        virtual double getZWD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
    };

    /** @brief class for t_chao derive from t_gtropo. */
    class LibGnut_LIBRARY_EXPORT t_chao : public t_gtropo
    {
    public:
        /** @brief default constructor. */
        t_chao() {}

        /** @brief default destructor. */
        ~t_chao() {}

        /** @brief get ZHD/ZWD. */
        virtual double getZHD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
        virtual double getZWD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
    };

    /** @brief class for t_ifad derive from t_gtropo. */
    class LibGnut_LIBRARY_EXPORT t_ifad : public t_gtropo
    {
    public:
        /** @brief default constructor. */
        t_ifad() {}

        /** @brief default destructor. */
        ~t_ifad() {}

        /** @brief get ZHD/ZWD. */
        virtual double getZHD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
        virtual double getZWD(const t_gtriple &ele, const t_gtime &epo); // ! Radians: Ell[0] and Ell[1]
    };

} // namespace

#endif
