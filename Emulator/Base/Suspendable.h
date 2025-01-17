// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <cstdio>

namespace vamiga {

class Suspendable {

public:

    Suspendable() { }
    virtual ~Suspendable() = default;

    // Informs about the current state
    virtual bool isSuspended() const = 0;
    
    // Suspends the thread
    virtual void suspend() const = 0;

    // Resumes the thread
    virtual void resume() const = 0;
};

struct AutoResume {

    Suspendable *s;
    AutoResume(Suspendable *s) : s(s) { s->suspend(); }
    ~AutoResume() { s->resume(); }
};

#define SUSPENDED AutoResume _ar(this);

}
