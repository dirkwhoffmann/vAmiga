// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "GuardList.h"
#include "Emulator.h"

namespace vamiga {

std::optional<GuardListInfo>
MoiraGuardList::guardNr(long nr) const
{
    if (auto *g = guards.guardNr(nr); g) {
        return GuardListInfo {.addr = g->addr, .enabled = g->enabled, .ignore = g->ignore };
    }

    return { };
}

std::optional<GuardListInfo>
MoiraGuardList::guardAt(u32 addr) const
{
    if (auto *g = guards.guardAt(addr); g) {
        return GuardListInfo {.addr = g->addr, .enabled = g->enabled, .ignore = g->ignore };
    }

    return { };
}

void
MoiraGuardList::setAt(u32 target, isize ignores)
{
    if (guards.isSetAt(target)) throw Error(ERROR_GUARD_ALREADY_SET, target);
    guards.setAt(target, ignores);
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::moveTo(isize nr, u32 newTarget)
{
    if (!guards.guardNr(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.replace(nr, newTarget);
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::ignore(long nr, long count)
{
    if (!guards.guardNr(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.ignore(nr, count);
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::remove(isize nr)
{
    if (!guards.isSet(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.remove(nr);
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::removeAt(u32 target)
{
    if (!guards.isSetAt(target)) throw Error(ERROR_GUARD_NOT_FOUND, target);
    guards.removeAt(target);
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::removeAll()
{
    guards.removeAll();
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::enable(isize nr)
{
    if (!guards.isSet(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.enable(nr);
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::enableAt(u32 target)
{
    if (!guards.isSetAt(target)) throw Error(ERROR_GUARD_NOT_FOUND, target);
    guards.enableAt(target);
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::enableAll()
{
    guards.enableAll();
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::disable(isize nr)
{
    if (!guards.isSet(nr)) throw Error(ERROR_GUARD_NOT_FOUND, nr);
    guards.disable(nr);
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::disableAt(u32 target)
{
    if (!guards.isSetAt(target)) throw Error(ERROR_GUARD_NOT_FOUND, target);
    guards.disableAt(target);
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::disableAll()
{
    guards.disableAll();
    update();
    emu.main.msgQueue.put(MSG_GUARD_UPDATED);
}

void
MoiraGuardList::toggle(isize nr)
{
    guards.isEnabled(nr) ? disable(nr) : enable(nr);
}

void 
MoiraGuardList::update() {

    needsCheck = false;
    for (isize i = 0; i < guards.elements(); i++) {

        if (guards.isEnabled(i)) {

            needsCheck = true;
            break;
        }
    }

    setNeedsCheck(needsCheck);
}

}
