// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include <pthread.h>

class Mutex
{
    pthread_mutex_t mutex;

public:
    
    Mutex();
    ~Mutex();
    
    int lock();
    int unlock();
};

class RecursiveMutex
{
    pthread_mutex_t mutex;

public:
    
    RecursiveMutex();
    ~RecursiveMutex();
    
    int lock();
    int unlock();
};

class AutoMutex
{
    RecursiveMutex &mutex;

public:

    bool active = true;

    AutoMutex(RecursiveMutex &ref) : mutex(ref) { mutex.lock(); }
    ~AutoMutex() { mutex.unlock(); }
};

