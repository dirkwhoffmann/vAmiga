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

enum class GdbCmd
{
    CtrlC,
    Supported,
    Symbol,
    Offset,
    TStatus,
    TfV,
    TfP,
    fThreadInfo,
    sThreadInfo,
    Attached,
    C
};
