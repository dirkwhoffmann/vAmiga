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
Guard::matches(u32 addr)
{
    if (this->addr == addr && this->enabled) {
        if (++hits > skip) {
            hits = 0;
            return true;
        }
    }
    return false;
}

template <int Capacity> Guard *
GuardCollection<Capacity>::guardWithNr(long nr)
{
    return nr < count ? &guards[nr] : NULL;
}

template <int Capacity> Guard *
GuardCollection<Capacity>::guardAtAddr(u32 addr)
{
    for (int i = 0; i < count; i++) {
        if (guards[i].addr == addr) return &guards[i];
    }

    return NULL;
}

template <int Capacity> bool
GuardCollection<Capacity>::hasGuardAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL;
}

template <int Capacity> bool
GuardCollection<Capacity>::hasDisabledGuardAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL && !guard->enabled;
}

template <int Capacity> bool
GuardCollection<Capacity>::hasConditionalGuardAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL && guard->skip != 0;
}

template <int Capacity> void
GuardCollection<Capacity>::setAt(u32 addr, long skip)
{
    if (!hasGuardAt(addr) && count + 1 < Capacity) {

        guards[count].addr = addr;
        guards[count].enabled = true;
        guards[count].hits = 0;
        guards[count].skip = skip;
        count++;
    }
}

template <int Capacity> void
GuardCollection<Capacity>::remove(long nr)
{
    if (nr < Capacity) removeAt(guards[nr].addr);
}

template <int Capacity> void
GuardCollection<Capacity>::removeAt(uint32_t addr)
{
    printf("count = %ld removeAt(%x)\n", count, addr);
    for (int i = 0; i < count; i++) {

        if (guards[i].addr == addr) {

            printf("Removing \n");
            for (int j = i; j + 1 < count; j++)
                guards[j] = guards[j + 1];
            count--;
            printf("count = %ld\n", count);
            break;
        }
    }
}

template <int Capacity> void
GuardCollection<Capacity>::setEnableAt(uint32_t addr, bool value)
{
    Guard *guard = guardAtAddr(addr);
    if (guard) guard->enabled = value;
}

template <int Capacity> bool
GuardCollection<Capacity>::matches(u32 addr)
{
    for (int i = 0; i < count; i++) {
        if (guards[i].matches(addr)) return true;
    }
    return false;
}

Observer::Observer()
{
    // REMOVE ASAP
    watchpoints.setAt(42);
    watchpoints.setAt(0xFCC8000);
    watchpoints.remove(0);
    watchpoints.removeAt(0xFCC8000);
    watchpoints.setAt(43);
    watchpoints.setAt(0xFCC8002);
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

    return breakpoints.matches(addr);
}

bool
Observer::watchpointMatches(u32 addr)
{
    return watchpoints.matches(addr);
}

}
