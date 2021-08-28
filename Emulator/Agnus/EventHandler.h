// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Aliases.h"

//
// Scheduling specific events
//

// Schedules the next BPL event relative to a given DMA cycle
void scheduleNextBplEvent(isize hpos);

// Schedules the next BPL event relative to the currently emulated DMA cycle
void scheduleNextBplEvent() { scheduleNextBplEvent(pos.h); }

// Schedules the earliest BPL event that occurs at or after the given DMA cycle
void scheduleBplEventForCycle(isize hpos);

// Updates the scheduled BPL event according to the current event table
void updateBplEvent() { scheduleBplEventForCycle(pos.h); }

// Schedules the next DAS event relative to a given DMA cycle
void scheduleNextDasEvent(isize hpos);

// Schedules the next DAS event relative to the currently emulated DMA cycle
void scheduleNextDasEvent() { scheduleNextDasEvent(pos.h); }

// Schedules the earliest DAS event that occurs at or after the given DMA cycle
void scheduleDasEventForCycle(isize hpos);

// Updates the scheduled DAS event according to the current event table
void updateDasEvent() { scheduleDasEventForCycle(pos.h); }

// Schedules the next register change event
void scheduleNextREGEvent();


//
// Processing events
//

private:

// Event handlers for specific slots
template <int nr> void serviceCIAEvent();
void serviceREGEvent(Cycle until);
void serviceBPLEvent();
template <int nr> void serviceBPLEventHires();
template <int nr> void serviceBPLEventLores();
void serviceBPLEventLores();
void serviceDASEvent();
void serviceRASEvent();

public:

void serviceINSEvent();
