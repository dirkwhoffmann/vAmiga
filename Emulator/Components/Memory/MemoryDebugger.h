// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MemoryTypes.h"
#include "SubComponent.h"

namespace vamiga {

class MemoryDebugger final : public SubComponent
{
    Descriptions descriptions = {{

        .name           = "MemoryDebugger",
        .description    = "Memory Debugger",
        .shell          = ""
    }};

    ConfigOptions options = {

    };

    
    //
    // Methods
    //

public:

    using SubComponent::SubComponent;

    MemoryDebugger& operator= (const Host& other) {

        return *this;
    }


    //
    // Methods from Serializable
    //

public:

    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream& os) const override { };


    //
    // Configuring
    //

public:

    const ConfigOptions &getOptions() const override { return options; }

};

}
