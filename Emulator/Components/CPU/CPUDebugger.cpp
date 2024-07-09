// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CPUDebugger.h"
#include "Emulator.h"
#include "Moira.h"

namespace vamiga {

std::optional<GuardInfo>
GuardsWrapper::guardNr(long nr) const
{
    if (auto *g = guards.guardNr(nr); g) {
        return GuardInfo {.addr = g->addr, .enabled = g->enabled, .ignore = g->ignore };
    }

    return { };
}

std::optional<GuardInfo>
GuardsWrapper::guardAt(u32 addr) const
{
    if (auto *g = guards.guardAt(addr); g) {
        return GuardInfo {.addr = g->addr, .enabled = g->enabled, .ignore = g->ignore };
    }

    return { };
}

void
GuardsWrapper::setAt(u32 target, isize ignores)
{
    if (guards.isSetAt(target)) throw Error(ERROR_GUARD_ALREADY_SET, target);
    guards.setAt(target, ignores);
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::moveTo(isize nr, u32 newTarget)
{
    if (!guards.guardNr(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.replace(nr, newTarget);
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::ignore(long nr, long count)
{
    if (!guards.guardNr(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.ignore(nr, count);
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::remove(isize nr)
{
    if (!guards.isSet(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.remove(nr);
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::removeAt(u32 target)
{
    if (!guards.isSetAt(target)) throw Error(ERROR_GUARD_NOT_FOUND, target);
    guards.removeAt(target);
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::removeAll()
{
    guards.removeAll();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::enable(isize nr)
{
    if (!guards.isSet(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.enable(nr);
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::enableAt(u32 target)
{
    if (!guards.isSetAt(target)) throw Error(ERROR_GUARD_NOT_FOUND, target);
    guards.enableAt(target);
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::enableAll()
{
    guards.enableAll();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::disable(isize nr)
{
    if (!guards.isSet(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.disable(nr);
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::disableAt(u32 target)
{
    if (!guards.isSetAt(target)) throw Error(ERROR_GUARD_NOT_FOUND, target);
    guards.disableAt(target);
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::disableAll()
{
    guards.disableAll();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
GuardsWrapper::toggle(isize nr)
{
    guards.isEnabled(nr) ? disable(nr) : enable(nr);
}

}
