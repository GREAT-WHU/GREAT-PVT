
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
 
-*/

#include <iostream>
#include <iomanip>

#include "gutils/gmutex.h"

namespace gnut
{

    t_gmutex::t_gmutex()
    {
#ifdef USE_OPENMP
        omp_init_lock(&_mutex);
        isLock = false;
#endif 
        isLock = false;
    }

    t_gmutex::t_gmutex(const t_gmutex &Other)
    {
#ifdef USE_OPENMP
        omp_init_lock(&this->_mutex);
        isLock = Other.isLock;
#endif
        isLock = Other.isLock;
    }

    t_gmutex::~t_gmutex()
    {
    }

    t_gmutex t_gmutex::operator=(const t_gmutex &Other)
    {
        return t_gmutex();
    }

    void t_gmutex::lock()
    {
#ifdef USE_OPENMP
        if (!isLock)
        {
            omp_set_lock(&_mutex);
            isLock = true;
        }
#else
        if (!isLock)
        {
            _mutex.lock();
            isLock = true;
        }
#endif 
    }

    void t_gmutex::unlock()
    {
#ifdef USE_OPENMP
        if (isLock)
        {
            omp_unset_lock(&_mutex);
            isLock = false;
        }
#else
        if (isLock)
        {
            _mutex.unlock();
            isLock = false;
        }
#endif
    }

} // namespace
