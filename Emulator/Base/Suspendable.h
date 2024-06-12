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

    // Suspends the thread
    virtual void suspend() = 0;

    // Resumes the thread
    virtual void resume() = 0;
};

struct AutoResume {

    Suspendable *s;
    AutoResume(Suspendable *s) : s(s) { s->suspend(); }
    ~AutoResume() { s->resume(); }
};

#define SUSPENDED AutoResume _ar(this);

}
