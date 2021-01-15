// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "EventHandlerPublicTypes.h"
#include "Reflection.h"

/*
struct EventSlotEnum : Reflection<EventSlotEnum, EventSlot> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < SLOT_COUNT;
    }

    static const char *prefix() { return "SLOT"; }
    static const char *key(Palette value)
    {
        switch (value) {
                
            case PALETTE_COLOR:        return "COLOR";
            case PALETTE_BLACK_WHITE:  return "BLACK_WHITE";
            case PALETTE_PAPER_WHITE:  return "PAPER_WHITE";
            case PALETTE_GREEN:        return "GREEN";
            case PALETTE_AMBER:        return "AMBER";
            case PALETTE_SEPIA:        return "SEPIA";
            case PALETTE_COUNT:        return "???";
        }
        return "???";
    }
};
*/
