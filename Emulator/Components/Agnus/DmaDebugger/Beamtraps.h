// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "GuardList.h"

#pragma once

namespace vamiga {

class Beamtraps : public GuardList {

    class Agnus &agnus;

public:

    Beamtraps(Agnus& ref);
    virtual ~Beamtraps() { }

    void setNeedsCheck(bool value) override;

    void serviceEvent();
    void scheduleNextEvent();
};

}
