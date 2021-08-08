// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Oscillator.h"
#include "Agnus.h"
#include "Chrono.h"

const double Oscillator::masterClockFrequency = 28.37516;
const double Oscillator::cpuClockFrequency = masterClockFrequency / 4.0;
const double Oscillator::dmaClockFrequency = masterClockFrequency / 8.0;

/*
Oscillator::Oscillator(Amiga& ref) : SubComponent(ref)
{

}
*/

const char *
Oscillator::getDescription() const
{
#ifdef __MACH__
    return "Oscillator (Mac)";
#else
    return "Oscillator (Generic)";
#endif
}

void
Oscillator::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {
        
    }
}
