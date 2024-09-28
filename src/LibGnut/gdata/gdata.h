/**
*
* @verbatim
    History
    2011-03-25  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gdata.h
* @brief       about GNSS data
*.
* @author      JD
* @version     1.0.0
* @date        2011-03-25
*
*/

#ifndef GDATA_H
#define GDATA_H

#include <sstream>
#include <iostream>

#include "gutils/gmutex.h"
#include "gio/gnote.h"
#include "gutils/gcommon.h"
#include "gio/grtlog.h"
using namespace std;

namespace gnut
{

    /**
    *@brief       basic class for data storing derive from t_gmoint
    */
    class LibGnut_LIBRARY_EXPORT t_gdata
    {

    public:
        explicit t_gdata();
        /** @brief default constructor. */
        explicit t_gdata(t_spdlog spdlog);

        /** @brief copy constructor. */
        explicit t_gdata(const t_gdata &data);

        /** @brief default destructor. */
        virtual ~t_gdata();

        /** @brief override operator =. */
        t_gdata &operator=(const t_gdata &data);

        /** @brief data type */
        enum ID_TYPE
        {
            NONE,        ///<   none
            OBJ,         ///<   object
            TRN,         ///<   transmitter
            REC,         ///<   receiver
            OBS,         ///<   obseravation base
            OBSGNSS,     ///<   gnss observations
            SATDATA,     ///<   gnss observations + satellite data
            EPH,         ///<   navigation base
            EPHGPS,      ///<   navigation
            EPHGLO,      ///<   navigation
            EPHGAL,      ///<   navigation
            EPHQZS,      ///<   navigation
            EPHBDS,      ///<   navigation
            EPHSBS,      ///<   navigation
            EPHIRN,      ///<   navigation
            EPHPREC,     ///<   sp3/clocks
            ALLNAV,      ///<   all navigation all
            ALLPREC,     ///<   all sp3 + rinexc
            ALLOBS,      ///<   all observations
            ALLOBJ,      ///<   all objects
            ALLPCV,      ///<   all PCV
            ALLOTL,      ///<   all OTL
            ALLOPL,      ///<   all ocean pole looad
            ALLPROD,     ///<   all PROD
            ALLBIAS,     ///<   all PROD
            ALLPOLEUT1,  ///<   poleut1
            POS,         ///<   XYZT position/time
            CLK,         ///<   clocks
            IONEX,       ///<   ionospheric delay from tec grid products (GIM)
            PCV,         ///<   PCV model
            OTL,         ///<   ocean loading model
            OPL,         ///<   oceanpoleload
            BIAS,        ///<   code & phase biases
            LCI_POS,     ///<   lci position
            UPD,         ///<   upd
            IFCB,        ///<   Inter-Frequency Clock Bias
            ALLDE,       ///<   all planeteph
            LEAPSECOND,  ///<   leap Second
            ALLPCVNEQ,   ///<   all PCV NEG
            RSSIMAP,     ///<   rssi map
            LAST         ///<   last
        };

        /** @brief group type */
        enum ID_GROUP
        {
            GRP_NONE,    ///<   none
            GRP_OBSERV,  ///<   observations
            GRP_EPHEM,   ///<   ephemerides
            GRP_PRODUCT, ///<   positions/products
            GRP_MODEL,   ///<   models
            GRP_OBJECT,  ///<   objects
            GRP_LAST     ///<   last
        };

        /** @brief set glog pointer */
        void spdlog(t_spdlog spdlog);

        /** @brief get glog pointer */
        t_spdlog spdlog() const { return _spdlog; }

        /** @brief set gnote pointer */
        void gnote(t_gallnote *n) { _gnote = n; }

        /** @brief get gnote pointer */
        t_gallnote *gnote() const { return _gnote; }

        /** @brief get data type */
        const ID_TYPE &id_type() const { return _type; }

        /** @brief get group type */
        const ID_GROUP &id_group() const { return _group; }

        /** @brief get group type in string format */
        string str_group() const;

        /** @brief convert data type to string */
        static string type2str(ID_TYPE type);

        /** @brief lock mutex */
        void lock() const { this->_gmutex.lock(); }

        /** @brief unlock mutex */
        void unlock() const { this->_gmutex.unlock(); }

    protected:
        /**
         * @brief data type
         * @param t 
         * @return int 
         */
        int id_type(const ID_TYPE &t); 

        /**
         * @brief group type
         * @param g 
         * @return int 
         */
        int id_group(const ID_GROUP &g);

        mutable t_gmutex _gmutex; ///< mutex
        t_gallnote *_gnote;       ///< gnote
        ID_TYPE _type;            ///< type_ID
        ID_GROUP _group;          ///< group_ID
        t_spdlog _spdlog;         ///< spdlog pointer
    private:
    };
}
#endif
