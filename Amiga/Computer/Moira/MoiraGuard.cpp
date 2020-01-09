// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Moira.h"
#include <string.h>
#include <stdio.h>

namespace moira {

bool
Guard::eval(u32 addr)
{
    if (this->addr == addr && this->enabled) {
        if (++hits > skip) {
            hits = 0;
            return true;
        }
    }
    return false;
}

Guard *
GuardCollection::guardWithNr(long nr)
{
    return nr < count ? &guards[nr] : NULL;
}

Guard *
GuardCollection::guardAtAddr(u32 addr)
{
    for (int i = 0; i < count; i++) {
        if (guards[i].addr == addr) return &guards[i];
    }

    return NULL;
}

bool
GuardCollection::hasGuardAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL;
}

bool
GuardCollection::hasDisabledGuardAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL && !guard->enabled;
}

bool
GuardCollection::hasConditionalGuardAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL && guard->skip != 0;
}

void
GuardCollection::add(u32 addr, long skip)
{
    if (hasGuardAt(addr)) return;

    if (count >= capacity) {

        Guard *newguards = new Guard[2 * capacity];
        for (long i = 0; i < capacity; i++) newguards[i] = guards[i];
        delete [] guards;
        guards = newguards;
        capacity *= 2;
        printf("New capacity = %d\n", capacity);
    }

    guards[count].addr = addr;
    guards[count].enabled = true;
    guards[count].hits = 0;
    guards[count].skip = skip;
    count++;
}

void
GuardCollection::remove(long nr)
{
    if (nr < count) removeAt(guards[nr].addr);
}

void
GuardCollection::removeAt(uint32_t addr)
{
    for (int i = 0; i < count; i++) {

        if (guards[i].addr == addr) {

            for (int j = i; j + 1 < count; j++) guards[j] = guards[j + 1];
            count--;
            break;
        }
    }
}

bool
GuardCollection::isEnabled(long nr)
{
    return nr < count ? guards[nr].enabled : false;
}

void
GuardCollection::setEnable(long nr, bool val)
{
    if (nr < count) guards[nr].enabled = val;
}

void
GuardCollection::setEnableAt(uint32_t addr, bool value)
{
    Guard *guard = guardAtAddr(addr);
    if (guard) guard->enabled = value;
}

bool
GuardCollection::eval(u32 addr)
{
    for (int i = 0; i < count; i++)
        if (guards[i].eval(addr)) return true;

    return false;
}

Observer::Observer()
{
    // REMOVE ASAP
    watchpoints.add(0xDFF180);
}

bool
Observer::breakpointMatches(u32 addr)
{
    // Check if a soft breakpoint has been reached
    if (addr == softStop || softStop == UINT64_MAX) {

        // Soft breakpoints are deleted when reached
        softStop = UINT64_MAX - 1;
        return true;
    }

    return breakpoints.eval(addr);
}

bool
Observer::watchpointMatches(u32 addr)
{
    return watchpoints.eval(addr);
}

}
