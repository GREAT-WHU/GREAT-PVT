/**
 * @file         gposdata.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        pos data structure for storing pos data
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GPOSDATA_H
#define GPOSDATA_H

#include "gdata/gdata.h"
#include <Eigen/Eigen>
#include "gexport/ExportLibGREAT.h"
#include <newmat/newmat.h>
using namespace gnut;

namespace gsins
{
    /**
    *@brief t_gposdata Class for storing the pos data
    *
    * t_gposdata is used for loosely coupled integration.
    * The data contains information about position,velocity,their std and so on.
    * The gcoder t_gposdata corresponding to the gcoder t_posfile.
    */
    class LibGREAT_LIBRARY_EXPORT t_gposdata : public t_gdata
    {
    public:
        /** @brief default constructor. */
        t_gposdata();

        t_gposdata(t_spdlog spdlog);
        /** @brief default destructor. */
        ~t_gposdata();

        /**
        * @struct data_pos
        * @brief describe pos data information
        */
        struct data_pos
        {
            double t;             ///< time
            Eigen::Vector3d pos;  ///< position
            Eigen::Vector3d vn;   ///<
            Eigen::Vector3d Rpos; ///<
            Eigen::Vector3d Rvn;  ///<
            double PDOP;          ///< the value of PDOP
            int nSat;             ///< the number of satellite
            bool amb_state;       ///< the state of ambiguity
            double sigma0_2;      ///< the value of sigma0^2 (added by tyx for cps)

            /** @brief override operator. */
            bool operator<(const data_pos &dp) const
            { 
                return (t < dp.t);
            }
            bool operator<(const double &d) const
            { 
                return (t < d);
            }
            bool operator>(const double &d) const
            { 
                return (t > d);
            }
            void operator=(const double &d)
            { 
                t = d;
                pos = Eigen::Vector3d::Zero();
                vn = Eigen::Vector3d::Zero();
            }
        };

        struct data_dd
        {
            Matrix A;
            SymmetricMatrix P;
            ColumnVector l;
            map<vector<string>, vector<Eigen::Vector3d>> sat;
            map<vector<string>, vector<Eigen::Vector3d>> ant;
        };

        struct rtk_pos
        {
            double t;                  ///< time
            Eigen::Vector3d baseline;  ///< baseline
            Eigen::Vector3d Rbaseline; ///< baseline rms
            Eigen::Vector3d xbase;     ///< base station coordinate
            double baseline_len;       ///< baseline length
            double PDOP;               ///< the value of PDOP
            int nSat;                  ///< the number of satellite
            bool amb_state;            ///< the state of ambiguity
            double sigma0_2;           ///< the value of sigma0^2

            /** @brief override operator. */
            bool operator<(const rtk_pos &dp) const
            { 
                return (t < dp.t);
            }
            bool operator<(const double &d) const
            { 
                return (t < d);
            }
            bool operator>(const rtk_pos &dp) const
            { 
                return (t > dp.t);
            }
            bool operator>(const double &d) const
            { 
                return (t > d);
            }
            bool operator==(const rtk_pos &dp)
            { 
                return t == dp.t;
            }
            bool operator==(const double &d)
            { 
                return t == d;
            }
        };

    private:
        vector<data_pos> _vecpos; /// latitude and longitude are saved by radian
        set<data_pos> _setpos;    /// latitude and longitude are saved by radian
        int _ptr;
    };

}

#endif