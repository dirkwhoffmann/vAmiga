// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "GuardList.h"
#include "Emulator.h"

namespace vamiga {

std::optional<GuardInfo>
GuardList::guardNr(long nr) const
{
    if (auto *g = guards.guardNr(nr); g) {
        return GuardInfo {.addr = g->addr, .enabled = g->enabled, .ignore = g->ignore };
    }

    return { };
}

std::optional<GuardInfo>
GuardList::guardAt(u32 addr) const
{
    if (auto *g = guards.guardAt(addr); g) {
        return GuardInfo {.addr = g->addr, .enabled = g->enabled, .ignore = g->ignore };
    }

    return { };
}

std::optional<GuardInfo> 
GuardList::hit() const
{
    if (auto g = guards.hit; g) {
        return GuardInfo {.addr = g->addr, .enabled = g->enabled, .ignore = g->ignore };
    }

    return { };
}

void
GuardList::setAt(u32 target, isize ignores)
{
    if (guards.isSetAt(target)) throw CoreError(Fault::GUARD_ALREADY_SET, target);
    guards.setAt(target, ignores);
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::moveTo(isize nr, u32 newTarget)
{
    if (!guards.guardNr(nr)) throw CoreError(Fault::GUARD_NOT_FOUND, nr);
    guards.replace(nr, newTarget);
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::ignore(long nr, long count)
{
    if (!guards.guardNr(nr)) throw CoreError(Fault::GUARD_NOT_FOUND, nr);
    guards.ignore(nr, count);
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::remove(isize nr)
{
    if (!guards.isSet(nr)) throw CoreError(Fault::GUARD_NOT_FOUND, nr);
    guards.remove(nr);
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::removeAt(u32 target)
{
    if (!guards.isSetAt(target)) throw CoreError(Fault::GUARD_NOT_FOUND, target);
    guards.removeAt(target);
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::removeAll()
{
    guards.removeAll();
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::enable(isize nr)
{
    if (!guards.isSet(nr)) throw CoreError(Fault::GUARD_NOT_FOUND, nr);
    guards.enable(nr);
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::enableAt(u32 target)
{
    if (!guards.isSetAt(target)) throw CoreError(Fault::GUARD_NOT_FOUND, target);
    guards.enableAt(target);
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::enableAll()
{
    guards.enableAll();
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::disable(isize nr)
{
    if (!guards.isSet(nr)) throw CoreError(Fault::GUARD_NOT_FOUND, nr);
    guards.disable(nr);
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::disableAt(u32 target)
{
    if (!guards.isSetAt(target)) throw CoreError(Fault::GUARD_NOT_FOUND, target);
    guards.disableAt(target);
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::disableAll()
{
    guards.disableAll();
    update();
    emu.main.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::toggle(isize nr)
{
    guards.isEnabled(nr) ? disable(nr) : enable(nr);
}

void 
GuardList::update() {

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
