// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"

namespace va {

class RetroShell : public AmigaComponent {

    string t;
    
    
    //
    // Initializing
    //
    
public:
    
    using AmigaComponent::AmigaComponent;
    
    const char *getDescription() const override { return "RetroShell"; }

    void _reset(bool hard) override { }
    
    
    //
    // Serializing
    //

private:

    isize _size() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }

    
    //
    //
    //
    
    void pressKey(char c);
    const char *text();
};

}
