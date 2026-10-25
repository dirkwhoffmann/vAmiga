// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "GuardList.h"
#include "Emulator.h"

namespace vamiga {

std::optional<GuardInfo>
GuardList::guardNr(isize nr) const
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
    if (guards.isSetAt(target)) throw CoreError(CoreError::GUARD_ALREADY_SET, target);
    guards.setAt(target, ignores);
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::moveTo(isize nr, u32 newTarget)
{
    if (!guards.guardNr(nr)) throw CoreError(CoreError::GUARD_NOT_FOUND, nr);
    guards.replace(nr, newTarget);
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::ignore(isize nr, isize count)
{
    if (!guards.guardNr(nr)) throw CoreError(CoreError::GUARD_NOT_FOUND, nr);
    guards.ignore(nr, count);
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::remove(isize nr)
{
    if (!guards.isSet(nr)) throw CoreError(CoreError::GUARD_NOT_FOUND, nr);
    guards.remove(nr);
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::removeAt(u32 target)
{
    if (!guards.isSetAt(target)) throw CoreError(CoreError::GUARD_NOT_FOUND, target);
    guards.removeAt(target);
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::removeAll()
{
    guards.removeAll();
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::enable(isize nr)
{
    if (!guards.isSet(nr)) throw CoreError(CoreError::GUARD_NOT_FOUND, nr);
    guards.enable(nr);
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::enableAt(u32 target)
{
    if (!guards.isSetAt(target)) throw CoreError(CoreError::GUARD_NOT_FOUND, target);
    guards.enableAt(target);
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::enableAll()
{
    guards.enableAll();
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::disable(isize nr)
{
    if (!guards.isSet(nr)) throw CoreError(CoreError::GUARD_NOT_FOUND, nr);
    guards.disable(nr);
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::disableAt(u32 target)
{
    if (!guards.isSetAt(target)) throw CoreError(CoreError::GUARD_NOT_FOUND, target);
    guards.disableAt(target);
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
}

void
GuardList::disableAll()
{
    guards.disableAll();
    update();
    amiga.msgQueue.put(Msg::GUARD_UPDATED);
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


void
GuardList::processCommand(const Command &cmd)
{
    auto &g = cmd.guard;

    switch (cmd.type) {

        case Cmd::GUARD_SET_AT:      setAt(g.addr, g.ignores); break;
        case Cmd::GUARD_REMOVE_NR:   remove(g.nr); break;
        case Cmd::GUARD_MOVE_NR:     moveTo(g.nr, g.addr); break;
        case Cmd::GUARD_IGNORE_NR:   ignore(g.nr, g.ignores); break;
        case Cmd::GUARD_REMOVE_AT:   removeAt(g.addr); break;
        case Cmd::GUARD_REMOVE_ALL:  removeAll(); break;
        case Cmd::GUARD_ENABLE_NR:   enable(g.nr); break;
        case Cmd::GUARD_ENABLE_AT:   enableAt(g.addr); break;
        case Cmd::GUARD_ENABLE_ALL:  enableAll(); break;
        case Cmd::GUARD_DISABLE_NR:  disable(g.nr); break;
        case Cmd::GUARD_DISABLE_AT:  disableAt(g.addr); break;
        case Cmd::GUARD_DISABLE_ALL: disableAll(); break;
        case Cmd::GUARD_TOGGLE_NR:   toggle(g.nr); break;

        default:
            fatalError;
    }
}

}
