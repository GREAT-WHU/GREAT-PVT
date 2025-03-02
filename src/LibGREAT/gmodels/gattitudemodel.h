/**
 * @file         gattitudemodel.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        attitude model class
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef GSAT_ATTITUDE_H
#define GSAT_ATTITUDE_H

#include "gexport/ExportLibGREAT.h"
#include "gdata/gsatdata.h"
#include "gmodels/gephplan.h"

using namespace gnut;
using namespace std;

namespace great
{
    /**
     *@brief Class for satellite attitude model
     */
    class LibGREAT_LIBRARY_EXPORT t_gattitude_model
    {
    public:
        /** @brief Constructor    */
        t_gattitude_model(){};

        /** @brief default destructor. */
        virtual ~t_gattitude_model(){};

        // attitude modeling - public interface
        /** @brief get satellite attitude.
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        *return 
            @retval =0                success
            @retval =1                fail
        */
        int attitude_old(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k); // from RTKlib (to remove)

        /** @brief get satellite attitude.
        *
        *param[in] satdata            satellite data
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        *return
            @retval =0                success
            @retval =1                fail
        */
        int attitude(t_gsatdata &satdata, double yaw, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief satellite attitude model ( with input Xsat, Vsat, Xsun) used in OI
        *
        *param[in] antype            antenna type
        *param[in] prn                satellite prn
        *param[in] xsat                TODO
        *param[in] vsat                TODO
        *param[in] xsun                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        *return
            @retval =0                success
            @retval =1                fail
        */
        int attitude(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief get satellite attitude.
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        *return
            @retval =-1                fail
            @retval =0                fail
            @retval =1                success
        */
        int attitude(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

    protected:
        // From RTKlib - needs to be removed
        /** @brief yaw model of different satellite
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] xs                satellite position X
        *param[in] ys                satellite position Y
        *param[in] zs                satellite position Z
        *return
            @retval =0                fail
            @retval =1                success
        */
        int _yaw(t_gsatdata &satdata, string antype, Eigen::Vector3d &xs, Eigen::Vector3d &ys, Eigen::Vector3d &zs);

        // attitude niminal modeling
        /** @brief Yaw-steering mode attitude model
        *
        *param[in] satdata            satellite data
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _ysm(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief Yaw-steering mode attitude model
        *
        *param[in] prn                satellite prn
        *param[in] bata                TODO
        *param[in] mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _ysm(string prn, double bata, double mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief orbit normal mode (i toward the velocity)
        *
        *param[in] satdata            satellite data
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _onm(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief orbit normal mode (i toward the velocity)
        *
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _onm(ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief noon maneuver
        *
        *param[in] satdata            satellite data
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _noon_turn(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief midnight maneuver
        *
        *param[in] satdata            satellite data
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _midnight_turn(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief noon maneuver
        *
        *param[in] _prn                satellite prn
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] R                columnVector
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _noon_turn(string _prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // attitude for GPS Block IIA
        /** @brief Attitude modelling for GPS Block IIA
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GPSIIA(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief Attitude modelling for GPS Block IIA
        *
        *param[in] antype            antenna type
        *param[in] prn                satellite prn
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GPSIIA(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief     midnight maneuver for GPS Block IIA
        *
        *param[in] satdata            satellite data
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _midnight_turn_GPSIIA(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief     midnight maneuver for GPS Block IIA
        *
        *param[in] prn                satellite prn
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _midnight_turn_GPSIIA(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // attitude for GPS Block IIR
        /** @brief attitude for GPS Block IIR
        *
        *param[in] satdata            satellite data
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GPSIIR(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief attitude for GPS Block IIR
        *
        *param[in] prn                satellite prn
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GPSIIR(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // attitude for GPS Block IIR-M
        /** @brief attitude for GPS Block IIR-M
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GPSIIRM(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        // attitude for GPS Block IIF
        /** @brief attitude for GPS Block IIF
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GPSIIF(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief attitude for GPS Block IIF
        *
        *param[in] antype            antenna type
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GPSIIF(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief midnight maneuver for GPS Block IIF
        *
        *param[in] satdata            satellite data
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _midnight_turn_GPSIIF(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief midnight maneuver for GPS Block IIF
        *
        *param[in] prn                satellite prn
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _midnight_turn_GPSIIF(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        //atttude for GPS Block III
        /** @brief midnight maneuver for GPS Block III
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GPSIII(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief attitude for GPS Block III
        *
        *param[in] antype            antenna type
        *param[in] prn                satellite prn
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GPSIII(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // attitude for Galileo IOV
        /** @brief attitude for Galileo IOV
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GAL1(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief attitude for Galileo IOV
        *
        *param[in] antype            antenna type
        *param[in] prn                satellite prn
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GAL1(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief noon maneuver for Galileo IOV
        *
        *param[in] satdata            satellite data
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _noon_turn_GAL1(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief noon maneuver for Galileo IOV
        *
        *param[in] antype            antenna type
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _noon_turn_GAL1(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // attitude for Galileo FOC
        /** @brief attitude for Galileo FOC
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GAL2(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief attitude for Galileo FOC
        *
        *param[in] antype            antenna type
        *param[in] prn                satellite prn
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GAL2(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief noon maneuver for Galileo FOC
        *
        *param[in] satdata            satellite data
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _noon_turn_GAL2(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief noon maneuver for Galileo FOC
        *
        *param[in] antype            antenna type
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _noon_turn_GAL2(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // Continuous yaw steering attitude modes of BDS satellites
        /** @brief Continuous yaw steering attitude modes of BDS satellites in CAST
        *
        *param[in] satdata            satellite data
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _cys_cast(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief Continuous yaw steering attitude modes of BDS satellites in CAST
        *
        *param[in] prn                satellite prn
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _cys_cast(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief Continuous yaw steering attitude modes of BDS satellites in SECM
        *
        *param[in] satdata            satellite data
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _cys_secm(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief Continuous yaw steering attitude modes of BDS satellites in SECM
        *
        *param[in] prn                satellite prn
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _cys_secm(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // attitude for BeiDou
        /** @brief attitude for BeiDou
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_BDS(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief attitude for BeiDou
        *
        *param[in] antype            antenna type
        *param[in] prn                satellite prn
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_BDS(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // continuous yaw steering attitude modes of QZS-2 IGSO satellites, added by yqyuan
        /** @brief Continuous yaw steering attitude modes of of QZS-2 IGSO satellites
        *
        *param[in] satdata            satellite data
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _cys_qzs(t_gsatdata &satdata, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief Continuous yaw steering attitude modes of of QZS-2 IGSO satellites
        *
        *param[in] prn                satellite prn
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _cys_qzs(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // attitude for QZSS
        /** @brief attitude for QZSS
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_QZS(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief attitude for QZSS
        *
        *param[in] antype            antenna type
        *param[in] prn                satellite prn
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_QZS(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        // attitude for GLO
        /** @brief attitude for GLO
        *
        *param[in] satdata            satellite data
        *param[in] antype            antenna type
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GLO(t_gsatdata &satdata, string antype, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief attitude for QZSS
        *
        *param[in] antype            antenna type
        *param[in] prn                satellite prn
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _attitude_GLO(string antype, string prn, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief midnight maneuver for GLONASS-M
        *
        *param[in] satdata            satellite data
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _midnight_turn_GLOM(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief noon maneuver for GLONASS-M
        *
        *param[in] satdata            satellite data
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _noon_turn_GLOM(t_gsatdata &satdata, double R, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief midnight maneuver for GLONASS-M
        *
        *param[in] prn                satellite prn
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _midnight_turn_GLOM(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief midnight maneuver for GLONASS-M
        *
        *param[in] prn                satellite prn
        *param[in] _beta            TODO
        *param[in] _mi                TODO
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] R                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _noon_turn_GLOM(string prn, double _beta, double _mi, ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double R, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief Calculate satellite-fixed vectors from yaw angle
        *
        *param[in] satdata            satellite data
        *param[in] yaw                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _yaw2ijk(t_gsatdata &satdata, double &yaw, Eigen::Vector3d &i, Eigen::Vector3d &j, Eigen::Vector3d &k);

        /** @brief Calculate satellite-fixed vectors from yaw angle
        *
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        *param[in] yaw                TODO
        *param[in] i                columnVector1
        *param[in] j                columnVector2
        *param[in] k                columnVector3
        */
        void _yaw2ijk(ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun, double &yaw, ColumnVector &i, ColumnVector &j, ColumnVector &k);

        /** @brief Calculate orbit angle
        *
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        */
        double _orb_angle(ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun);

        /** @brief Calculate beta
        *
        *param[in] xsat                satellite position
        *param[in] vsat                satellite velocity
        *param[in] xsun                sun position
        */
        double _beta(ColumnVector &xsat, ColumnVector &vsat, ColumnVector &xsun);

        /** @brief Calculate the value of a with the sign of b
        *
        *param[in] a                parameter 1 
        *param[in] b                parameter 2
        */
        double sign(double a, double b);

        ATTITUDES _attitudes;           ///< satellite attitudes
        t_gephplan _ephplan;            ///< planetary ephemerises
        map<string, double> _last_beta; ///< last beta
        map<string, double> _last_yaw;  ///< last yaw
        map<string, t_gtime> _last_epo; ///< last epoch
    };
}

#endif