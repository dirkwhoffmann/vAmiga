// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum class GdbCmd
{
    Attached,
    C,
    CtrlC,
    Offset,
    StartNoAckMode,
    sThreadInfo,
    Supported,
    Symbol,
    TfV,
    TfP,
    TStatus,
    fThreadInfo,
};

//
// Structures
//

typedef struct
{
    bool verbose;
}
GdbServerConfig;
