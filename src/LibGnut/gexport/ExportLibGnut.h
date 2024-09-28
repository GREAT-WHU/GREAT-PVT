/**
*
* @verbatim
    History
    -1.0 jdhuang  2019-01-24  creat the file.

@endverbatim
* Copyright (c) 2018, Wuhan University. All rights reserved.
*
* @file       EXPORT_LibProcPos_H
* @brief      for the use of Lib Mat.
*
* @author     Jiande Huang , Wuhan University
* @version    1.0.0
* @date       2019-10-24
*
*/

#ifndef EXPORT_LibGnut_H
#define EXPORT_LibGnut_H

#if defined(_MSC_VER)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)
#pragma warning(disable : 4512)
#pragma warning(disable : 4267)
#pragma warning(disable : 4702)
#pragma warning(disable : 4511)
#pragma warning(disable : 4996)
#endif

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BCPLUSPLUS__) || defined(__MWERKS__)
#if defined(GREAT_LibGnut_LIBRARY)
#define LibGnut_LIBRARY_EXPORT __declspec(dllexport)
#else
#define LibGnut_LIBRARY_EXPORT __declspec(dllimport)
#endif
#else
#define LibGnut_LIBRARY_EXPORT
#endif

#ifdef _MSC_VER
#if (_MSC_VER >= 1300)
#define __STL_MEMBER_TEMPLATES
#endif
#endif

#include <string>
using namespace std;
#endif //EXPORT_LibGnut_H
