/**
*
* @verbatim
    History
    2011-04-20  JD: created
@endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file     sp3.h
* @brief    Purpose: sp3 data storing
* @author   JD
* @version  1.0.0
* @date     2011-04-20
*
*/

#ifndef SP3_H
#define SP3_H

#include <vector>
#include "gexport/ExportLibGnut.h"
#include "gcoders/gcoder.h"
#include "gutils/gtime.h"
#include "gall/gallprec.h"

using namespace std;

namespace gnut
{
    /**
    *@brief Class for t_sp3 derive from t_gdata
    */
    class LibGnut_LIBRARY_EXPORT t_sp3 : public t_gcoder
    {

    public:
        /** @brief constructor set + version + sz. */
        explicit t_sp3(t_gsetbase *s, string version = "", int sz = DEFAULT_BUFFER_SIZE);

        /** @brief default destructor. */
        virtual ~t_sp3(){};

        /** @brief decode head. */
        virtual int decode_head(char *buff, int sz, vector<string> &errmsg);

        /**
         * @brief 
         * 
         * @param buff 
         * @param sz 
         * @param cnt 
         * @param errmsg 
         * @return int 
         */
        virtual int decode_data(char *buff, int sz, int &cnt, vector<string> &errmsg);


    protected:
    private:
        t_gtime _start;           ///< start time
        t_gtime _lastepo;         ///< last time
        long _orbintv;            ///< orb interval [sec]
        int _nepochs;             ///< number of epoch
        int _nrecmax;             ///< max number of reciver
        int _nrecord;             ///< number of reciver
        string _orbrefs;          ///< orb reference
        string _orbtype;          ///< orb type
        string _agency;           ///< agency
        vector<string> _prn;      ///< satellite prn
        vector<string> _sat_sp3;  ///< satellite name
        vector<int> _acc;         ///< acc
        vector<string> _timesys;  ///< time of system
        vector<int> _accbase;     ///< acc base
        int _maxsats;             ///< max number of satellite
        string _sattype;          ///< satellite type
        string _data_type;        ///< data type
        t_map_prn _mapsp3;        ///< map sp3
        t_map_dat _sp3data;       ///< sp3 data
    };

} // namespace

#endif
