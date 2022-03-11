// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Scheduler.h"
#include "CIA.h"
#include "ControlPort.h"
#include "CPU.h"
#include "Agnus.h"
#include "Paula.h"
#include "HardDrive.h"
#include "Keyboard.h"
#include "FloppyDrive.h"
#include "IOUtils.h"
#include "RemoteManager.h"
#include <iomanip>

void
Scheduler::_reset(bool hard)
{
    
    RESET_SNAPSHOT_ITEMS(hard)
}

