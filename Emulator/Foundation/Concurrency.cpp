// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Concurrency.h"

Mutex::Mutex()
{
    pthread_mutex_init(&mutex, nullptr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mutex);
}

int
Mutex::lock()
{
    return pthread_mutex_lock(&mutex);
}

int
Mutex::unlock()
{
    return pthread_mutex_unlock(&mutex);
}
RecursiveMutex::RecursiveMutex()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex, &attr);
}

RecursiveMutex::~RecursiveMutex()
{
    pthread_mutex_destroy(&mutex);
}

int
RecursiveMutex::lock()
{
    return pthread_mutex_lock(&mutex);
}

int
RecursiveMutex::unlock()
{
    return pthread_mutex_unlock(&mutex);
}
