/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/util/mutex.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <cstring>

using namespace paludis;

#ifdef PALUDIS_ENABLE_THREADS

Mutex::Mutex() :
    _attr(new pthread_mutexattr_t),
    _mutex(new pthread_mutex_t)
{
    pthread_mutexattr_init(_attr);
    pthread_mutexattr_settype(_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(_mutex, _attr);
}

Mutex::~Mutex()
{
    int r(0);
    if (0 != ((r = pthread_mutex_destroy(_mutex))))
        throw InternalError(PALUDIS_HERE, "mutex destory failed: " + stringify(strerror(r)));
    pthread_mutexattr_destroy(_attr);

    delete _mutex;
    delete _attr;
}

pthread_mutex_t *
Mutex::posix_mutex()
{
    return _mutex;
}

Lock::Lock(Mutex & m) :
    _mutex(&m)
{
    int r(0);
    if (0 != ((r = pthread_mutex_lock(_mutex->posix_mutex()))))
        throw InternalError(PALUDIS_HERE, "mutex lock failed: " + stringify(strerror(r)));
}

Lock::~Lock()
{
    int r(0);
    if (0 != ((r = pthread_mutex_unlock(_mutex->posix_mutex()))))
        throw InternalError(PALUDIS_HERE, "mutex unlock failed: " + stringify(strerror(r)));
}

void
Lock::acquire_then_release_old(Mutex & m)
{
    int r(0);
    if (0 != ((r = pthread_mutex_lock(m.posix_mutex()))))
        throw InternalError(PALUDIS_HERE, "mutex lock failed: " + stringify(strerror(r)));
    if (0 != ((r = pthread_mutex_unlock(_mutex->posix_mutex()))))
        throw InternalError(PALUDIS_HERE, "mutex unlock failed: " + stringify(strerror(r)));
    _mutex = &m;
}

TryLock::TryLock(Mutex & m) :
    _mutex(&m)
{
    if (0 != pthread_mutex_trylock(_mutex->posix_mutex()))
        _mutex = 0;
}

TryLock::~TryLock()
{
    int r(0);
    if (_mutex)
        if (0 != ((r = pthread_mutex_unlock(_mutex->posix_mutex()))))
            throw InternalError(PALUDIS_HERE, "mutex unlock failed: " + stringify(strerror(r)));
}

bool
TryLock::operator() () const
{
    return _mutex;
}

#else

Mutex::Mutex()
{
}

Mutex::~Mutex()
{
}

Lock::Lock(Mutex &)
{
}

void
Lock::acquire_then_release_old(Mutex &)
{
}

Lock::~Lock()
{
}

TryLock::TryLock(Mutex &)
{
}

TryLock::~TryLock()
{
}

bool
TryLock::operator() () const
{
    return true;
}

#endif

