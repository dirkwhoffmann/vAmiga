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
    qSupported,
    qSymbol,
    qOffset,
    qTStatus,
    qTfV,
    qTfP,
    qfThreadInfo,
    qsThreadInfo,
    qAttached,
    qC
};

typedef struct
{
    isize port;
    bool verbose;
}
RemoteServerConfig;
