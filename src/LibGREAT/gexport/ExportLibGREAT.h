/**
 * @file         ExportLibGREAT.h
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        for the use of Lib Base.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#ifndef EXPORT_LibGREAT_H
#define EXPORT_LibGREAT_H

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
#if defined(GREAT_LibGREAT_LIBRARY)
#define LibGREAT_LIBRARY_EXPORT __declspec(dllexport)
#else
#define LibGREAT_LIBRARY_EXPORT __declspec(dllimport)
#endif
#else
#define LibGREAT_LIBRARY_EXPORT
#endif

#ifdef _MSC_VER
#if (_MSC_VER >= 1300)
#define __STL_MEMBER_TEMPLATES
#endif
#endif

#include <string>
#include <vector>
using namespace std;

#ifndef SIZE_INT
#define SIZE_INT sizeof(int)
#endif

#ifndef SIZE_DBL
#define SIZE_DBL sizeof(double)
#endif // !SIZE_DBL

#include "gio/grtlog.h"
#endif //EXPORT_LibBase_H