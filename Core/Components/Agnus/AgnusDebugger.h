// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Beamtraps.h"

namespace vamiga {

class AgnusDebugger final : public SubComponent {

    friend class Agnus;

    Descriptions descriptions = {{

        .type           = Class::AgnusDebugger,
        .name           = "AgnusDebugger",
        .description    = "Agnus Debugger",
        .shell          = ""
    }};

    Options options = {

    };

public:

    // Beamtraps
    Beamtraps beamtraps = Beamtraps(agnus);

    // Interrupts the emulator at the end of the current line or frame
    bool eolTrap = false;
    bool eofTrap = false;


    //
    // Initializing
    //

public:

    using SubComponent::SubComponent;

    AgnusDebugger& operator= (const AgnusDebugger& other) {

        return *this;
    }


    //
    // Methods from Serializable
    //

private:

    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from Configurable
    //

public:

    const Options &getOptions() const override { return options; }


    //
    // Running the debugger
    //

public:

    // Called by Agnus at the end of each scanline
    void eolHandler();

    // Called by Agnus at the end of each frame
    void eofHandler();
};

}
