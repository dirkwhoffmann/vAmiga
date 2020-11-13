// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CONCURRENCY_H
#define _CONCURRENCY_H

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

#endif
