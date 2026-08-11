/*
*
* @verbatim
    History

    @endverbatim
*
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

*
* @file        gsetrec.h
* @brief       implements receiver object setting class
* @author      Jan Dousa
* @version     1.0.0
* @date        2012-10-23
*
*/

#ifndef GSETREC_H
#define GSETREC_H

#ifndef XMLKEY_REC
#define XMLKEY_REC "receiver"
#endif

#include <map>
#include <string>
#include <iostream>

#include "gdata/grec.h"
#include "gset/gsetbase.h"
#include "gutils/gtypeconv.h"
#include "gutils/gtriple.h"
#include "gmodels/ggpt.h"

#ifndef HSL_UNKNOWN
#define HSL_UNKNOWN -9999
#endif


namespace gnut
{

    class LibGnut_LIBRARY_EXPORT t_gsetrec : public virtual t_gsetbase
    {
    public:
        /** @brief constructor */
        t_gsetrec();

        /** @brief destructor */
        ~t_gsetrec();

        /** @brief settings check */
        void check();

        /** @brief settings help */
        void help();

        /** @brief get crd xyz */
        t_gtriple get_crd_xyz(std::string s);

        /** @brief get all objects IDs */
        std::set<std::string> objects();
        std::shared_ptr<t_grec> grec(std::string s);

        /**
         * @brief get the List of recevier names
         * @return set<string> : List of recevier names
         */
        virtual std::set<std::string> recs();
        std::set<std::string> all_rec();

    protected:
        /** @brief get crd xyz */
        t_gtriple _get_crd_xyz(std::string s);

        /** @brief get ecc neu */
        t_gtriple _get_ecc_neu(std::string s);

        /** @brief get crd blh */
        t_gtriple _get_crd_blh(std::string s);

        /** @brief Global Pressure Temperature model */
        t_gpt _ggpt;

        /** @brief get all objects IDs */
        std::set<std::string> _objects();

        std::string _rec;      // default receiver name
        std::string _id;       // receiver id
        std::string _name_rec; // receiver name
        t_gtime _beg;     // default begin time
        t_gtime _end;     // default end time
        double _X;        // receiver X-coordinate [m]
        double _Y;        // receiver Y-coordinate [m]
        double _Z;        // receiver Z-coordinate [m]
        bool _overwrite;

    private:
    };

} // namespace gnut

#endif
