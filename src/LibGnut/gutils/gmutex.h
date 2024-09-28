
/**
*
* @verbatim
    History
    2013-08-14  JD: created

  @endverbatim
* Copyright (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
*
* @file        gmutex.h
* @brief       Purpose: mutual exclusion
* @author      JD
* @version     1.0.0
* @date        2013-08-14
*
*/

#ifndef MUTEX_H
#define MUTEX_H

#include "gexport/ExportLibGnut.h"

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifdef USE_OPENMP
#include <omp.h>
#endif

#include <thread>
#include <mutex>

namespace gnut
{
    /** @brief class for t_gmutex. */
    class LibGnut_LIBRARY_EXPORT t_gmutex
    {
    public:
        /** @brief default constructor. */
        t_gmutex();

        /** @brief copy constructor. */
        t_gmutex(const t_gmutex &Other);

        /** @brief default destructor. */
        ~t_gmutex();

        /** @brief override operator =. */
        t_gmutex operator=(const t_gmutex &Other);

        /** @brief lock. */
        void lock();

        /** @brief unlock. */
        void unlock();

        bool isLock = false;

    protected:
#ifdef USE_OPENMP
        omp_lock_t _mutex;
#else
        mutex _mutex;

#endif 
    };
} // namespace

#endif // MUTEX_H
