/**
*
* @verbatim
    History
    2011-11-04  JD: created
    2012-09-04  JD: RINEX 3.02 implemented
    2012-09-27  JD: support of multi-GNSS (phase not converted to [m])
    2013-03-08  JD: filtering via general settings
    2014-05-03  PV: gobj getting information from rinexo header
    2014-11-19  JD: shared pointers
    2014-12-21  JD: advanced request filtering
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     rinexo3.h
* @brief    Purpose: Obs RINEX encoder/decoder
*    Todo:  header information completed
            encoders implementation
* @author   JD
* @version  1.0.0
* @date     2011-11-04
*
*/

#ifndef RINEXO3_H
#define RINEXO3_H
#include <string>
#include <vector>

#include "gall/gallobs.h"
#include "gall/gallobj.h"
#include "gutils/gtime.h"
#include "gutils/gtriple.h"
#include "gutils/gsys.h"
#include "gutils/gobs.h"
#include "gutils/gtypeconv.h"
#include "gcoders/gcoder.h"
#include "gcoders/rinexo2.h"
#include "gdata/grnxhdr.h"

using namespace std;

namespace gnut
{
    /**
    *@brief Class for t_rinexo3 derive from t_rinexo2
    */
    class LibGnut_LIBRARY_EXPORT t_rinexo3 : public t_rinexo2
    {
    public:
        /** @brief constructor set + version + sz. */
        t_rinexo3(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief constructor beg + end + set + version + sz. */
        t_rinexo3(t_gtime beg, t_gtime end, t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief default destructor. */
        virtual ~t_rinexo3(){};

        /** @brief decode head. */
        virtual int decode_head(char *buff, int sz, vector<string> &errmsg) = 0;

        /**
         * @brief 
         * 
         * @param buff 
         * @param sz 
         * @param cnt 
         * @param errmsg 
         * @return int 
         */
        virtual int decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg) = 0;

        /** @brief get epoch. */
        t_gtime get_epoch() { return _epoch; };

    protected:
        /** @brief decode head. */
        virtual int _decode_head();

        /**
         * @brief decode data
         * 
         * @return int 
         */
        virtual int _decode_data();

        /** @brief fill header information. */
        virtual int _check_head();

        /** @brief read epoch & number of satellites, return flag. */
        virtual int _read_epoch();

        /** @brief read single satellite observation types. */
        virtual int _read_obstypes(const string &sat, const string &sys);

        /** @brief fix band (BDS). */
        virtual int _fix_band(string sys, string &go);

        t_rnxhdr::t_obstypes _mapcyc;  ///< map of GOBS phase quater-cycle shifts
        t_rnxhdr::t_obstypes _glofrq;  ///< map of GLONASS slot/frequency
        t_rnxhdr::t_vobstypes _globia; ///< vec of GLONASS obs code-phase biases

    private:
    };

} // namespace

#endif
