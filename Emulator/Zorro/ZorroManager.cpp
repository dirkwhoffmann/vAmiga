// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ZorroManager.h"
#include "Amiga.h"
#include "Memory.h"

ZorroManager::ZorroManager(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<AmigaComponent *> {
        
        &ramExpansion,
    };
}

void
ZorroManager::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
            
    if (category & dump::State) {
    
        for (isize i = 0; i < slotCount; i++) {

            os << tab("Slot " + std::to_string(i));
            
            if (auto board = getBoard(i); board != nullptr) {
                os << slots[i]->productName() << std::endl;
            } else {
                os << "Empty"<< std::endl;
            }
        }
    }
}

ZorroBoard *
ZorroManager::getBoard(isize i) const
{
    // Return nullptr if there is no device
    if (usize(i) >= slotCount || !slots[i]) return nullptr;
    
    // Only return the device if it is plugged in
    return slots[i]->pluggedIn() ? slots[i] : nullptr;
}

u8
ZorroManager::peek8(u32 addr)
{
    return mappedInDevice(addr)->peek8(addr);
}

u16
ZorroManager::peek16(u32 addr)
{
    return mappedInDevice(addr)->peek16(addr);
}

u8
ZorroManager::spypeek8(u32 addr) const
{
    return mappedInDevice(addr)->spypeek8(addr);
}

u16
ZorroManager::spypeek16(u32 addr) const
{
    return mappedInDevice(addr)->spypeek16(addr);
}

void
ZorroManager::poke8(u32 addr, u8 value)
{
    return mappedInDevice(addr)->poke8(addr, value);
}

void
ZorroManager::poke16(u32 addr, u16 value)
{
    return mappedInDevice(addr)->poke16(addr, value);
}

u8
ZorroManager::peekACF(u32 addr) const
{
    for (isize i = 0; slots[i]; i++) {

        if (slots[i]->state == STATE_AUTOCONF) {
            
            return slots[i]->peekACF8(addr);
        }
    }
    return 0xFF;
}

void
ZorroManager::pokeACF(u32 addr, u8 value)
{
    for (isize i = 0; slots[i]; i++) {

        if (slots[i]->state == STATE_AUTOCONF) {
            
            slots[i]->pokeACF8(addr, value);
            return;
        }
    }
}

void
ZorroManager::updateMemSrcTables()
{
    for (isize i = 0; slots[i]; i++) {
        
        slots[i]->updateMemSrcTables();
    }
}

ZorroBoard *
ZorroManager::mappedInDevice(u32 addr) const
{
    for (isize i = 0; slots[i]; i++) {
        if (slots[i]->mappedIn(addr)) return slots[i];
    }
    fatalError;
}
