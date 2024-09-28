/**
*
* @verbatim
    History
      2011-11-04  JD: created
      2019-11-29  ZHJ: add encoder
*
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     rinexc.h
* @brief    Purpose: Clock RINEX encoder/decoder
*
* @author   JD
* @version  1.0.0
* @date     2011-11-04
*
*/

#ifndef RINEXC_H
#define RINEXC_H
#include <vector>

#include "gcoders/gcoder.h"
#include "gutils/gtime.h"
#include "gall/gallobj.h"
#include "gall/gallprec.h"

#define RINEXC_BUFFER_LEN 81

using namespace std;

namespace gnut
{
    /**
    *@brief Class for t_rinexc derive from t_gcoder
    */
    class LibGnut_LIBRARY_EXPORT t_rinexc : public t_gcoder
    {

    public:
        /** @brief constructor set + version + sz. */
        explicit t_rinexc(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief default destructor. */
        virtual ~t_rinexc(){};

        /** @brief decode head. */
        virtual int decode_head(char *buff, int sz, vector<string> &errmsg);

        /**
         * @brief decode data
         * 
         * @param buff 
         * @param sz 
         * @param cnt 
         * @param errmsg 
         * @return int 
         */
        virtual int decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg);

        /** @brief set/get gnss system. */
        void gnsssys(char s) { _gnsssys = s; }
        char gnsssys() { return _gnsssys; }

    protected:
        /** @brief meta struct. */
        struct t_meta
        {
            t_gtriple xyz;  ///< position
            string name;    ///< name
            string domes;   ///< domes
            string desc;    ///< desc
            string ant;     ///< ant
            string rec;     ///< reciver
            t_gtime begCLK; ///< begin time
            t_gtime endCLK; ///< end time
        };

        t_gtime _file_beg; ///< file first epoch
        t_gtime _file_end; ///< file last epoch
        t_gtime _file_run; ///< file created

        t_gallobj *_allobj;                                    ///< all object
        map<string, shared_ptr<t_gobj>> _mapsat;               ///< map satellite
        map<string, shared_ptr<t_gobj>> _maprec;               ///< map rec
        map<string, shared_ptr<t_gobj>>::const_iterator itOBJ; ///< itOBJ

        set<string> _sites;      ///< site list
        set<string> _satellites; ///< satellite list

    private:
        char _gnsssys; ///< gnss system
    };

} // namespace

#endif
