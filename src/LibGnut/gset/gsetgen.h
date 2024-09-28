/**
*
* @verbatim
    History

    @endverbatim
*
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)

*
* @file        gsetgen.h
* @brief       implements common general settings
* @author      Jan Dousa
* @version     1.0.0
* @date        2012-10-23
*
*/

#ifndef GSETGEN_H
#define GSETGEN_H

#include "gexport/ExportLibGnut.h"
#include <set>
#include <string>
#include <iostream>
#include <vector>

#include "gset/gsetbase.h"
#include "gio/grtlog.h"
#include "gutils/gsys.h"
#include "gutils/gtime.h"
#include "gutils/gtypeconv.h"

using namespace std;

namespace gnut
{

    class LibGnut_LIBRARY_EXPORT t_gobj;

    class LibGnut_LIBRARY_EXPORT t_gsetgen : public virtual t_gsetbase
    {
    public:
        /**
         * @brief default constructor, distinguish GNSS/nonGNSS app
         * @param[in] gnss : gnss process or not, default is true
         */
        t_gsetgen(bool gnss = true);

        /**@brief destructor */
        ~t_gsetgen();
        
        /**@brief settings check */
        void check() override;

        /**@brief settings help */
        void help() override;

        /**
         * @brief get the beg time of process
         * @return t_gtime : the beg time of process
         */
        t_gtime beg(bool conv = true);

        /**
         * @brief get the end time of process
         * @return t_gtime : the end time of process
         */
        t_gtime end(bool conv = true);

        /**
         * @brief get the sampling time of process
         * @return double : the sampling time of process
         */
        double sampling();

        /**
         * @brief get the default sampling time of process
         * @return double : the default sampling time of process
         *  @retval DEF_SAMPLING default sampling 
         */
        double sampling_default() const;

        /**
         * @brief get the decimals scale of sampling time in process
         * @return int : the decimals scale of sampling time in process
         */
        int sampling_scalefc() { return (int)pow(10, _dec); }

        /**
         * @brief get the decimals for sampling interval (for high-rate) in process
         * @return int : decimals for sampling interval (for high-rate) in process
         */
        const int &sampling_decimal() { return _dec; }

        /**
         * @brief get the List of system names
         * @return set<string> : List of system names
         */
        virtual set<string> sys();
        virtual void sys(string str);

        /**
         * @brief get the List of sites
         * @return set<string> : List of sites
         */
        set<string> recs();
        set<string> rec_all();

        /**
         * @brief get the List of base/rover sites
         * @return set<string> : List of base/rover sites
         */
        virtual vector<string> list_base();
        virtual vector<string> list_rover();

        /**
         * @brief get name of estimator
         * @return string : estimator name
         */
        virtual string estimator();

        /**
        * @brief add for remove unused satellites
        * @return set<string> : satellites which will be removed
        */
        virtual set<string> sat_rm();
        virtual void sat_rm(string str);

    protected:
        bool _gnss;  ///< gnss or not
        string _sys; ///< sys name
        int _dec;    ///< sampling

    private:
    };

} // namespace

#endif
