// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Reflection.h"

//
// Structures
//

//! The current emulator configuration
typedef struct
{
    //! Enable auto-snapshots
    bool snapshots;

    //! Delay between two auto-snapshots in seconds
    isize snapshotDelay;
}
EmulatorConfig;
