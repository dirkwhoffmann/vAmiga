// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "EventHandlerTypes.h"
#include "AmigaComponent.h"

class EventHandler : public AmigaComponent {

private:

    //
    // Initializing
    //

public:

    EventHandler(Amiga &ref);

    const char *getDescription() const override { return "EventHandler"; }

    void _reset(bool hard) override { }


    //
    // Configuring
    //

public:

    //
    // Analyzing
    //
    
public:

    // Returns the result of the most recent call to inspect()
    // EventInfo getEventInfo();
    // EventSlotInfo getEventSlotInfo(isize nr);

    
    //
    // Serializing
    //

private:

    isize _size() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }
    
};
