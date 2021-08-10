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

namespace util {

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
ReentrantMutex::ReentrantMutex()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex, &attr);
}

ReentrantMutex::~ReentrantMutex()
{
    pthread_mutex_destroy(&mutex);
}

int
ReentrantMutex::lock()
{
    return pthread_mutex_lock(&mutex);
}

int
ReentrantMutex::unlock()
{
    return pthread_mutex_unlock(&mutex);
}

void
Wakeable::waitForWakeUp()
{
    std::unique_lock<std::mutex> lock(condMutex);
    condFlag = false;
    cond.wait_for(lock,
                  std::chrono::seconds(1000),
                  [this]() { return condFlag; } );
}

void
Wakeable::wakeUp()
{
    {
        std::lock_guard<std::mutex> lock(condMutex);
        condFlag = true;
    }
    cond.notify_one();
}

}
