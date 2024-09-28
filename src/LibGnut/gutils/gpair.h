
/**
* @verbatim
    History
    2012-05-11  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gpair.h
* @brief       Purpose: implements 2D coordinates representation (e.g. horizontal coordinates)
* @author      JD
* @version     1.0.0
* @date        2012-05-11
*
*/

#ifndef GPAIR_H
#define GPAIR_H

#include "gexport/ExportLibGnut.h"

#include <iostream>
#include <string.h>

#include "newmat/newmat.h"

using namespace std;

namespace gnut
{

    /** @brief class for t_gpair. */
    class LibGnut_LIBRARY_EXPORT t_gpair
    {

    public:
        /** @brief default constructor. */
        t_gpair();

        /** @brief constructor 1. */
        t_gpair(double x, double y);

        /** @brief constructor 2. */
        explicit t_gpair(double crd[2]);

        /** @brief constructor 3. */
        explicit t_gpair(const ColumnVector &crd);

        /** @brief default destructor. */
        virtual ~t_gpair();

        /** @brief override operator. */
        t_gpair &operator=(const t_gpair &other);      
        t_gpair operator+(const t_gpair &other) const; 
        bool operator==(const t_gpair &tr) const;      
        bool operator<(const t_gpair &tr) const;
        double &operator[](const size_t idx);      
        double operator[](const size_t idx) const; 
        friend ostream &operator<<(ostream &os, const t_gpair &x);

        /**@brief get single element */
        double crd(int idx) const;         

        /**@brief set single element */
        void set(int idx, double newValue);

        /**@brief set array by ColumnVector */
        void set(const ColumnVector &);    

        /**@brief set array by array */
        void set(double crd[2]);           

        /**@brief get array */
        double *crd_array();              

        /**@brief get ColumnVector */
        ColumnVector crd_cvect();        

        /**@brief get pair */
        t_gpair &crd_pair();               

        /**@brief get unit ColumnVector */
        ColumnVector unitary();         

        /**@brief true: zero elements, false: not zero elements */
        bool zero();                       

    protected:
    private:
        double _crd[2];   ///< Two-dimensional coordinates
    };

} // namespace

#endif
