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
#include "HostTypes.h"
#include "IOUtils.h"

namespace vamiga {

/* This class stores some information about the host system. The values need to
 * be set by the GUI on start and updated on-the-fly when a value changes.
 */
class Host final : public SubComponent {

    Descriptions descriptions = {{

        .type           = HostClass,
        .name           = "Host",
        .description    = "Host Computer",
        .shell          = "host"

    }};

    ConfigOptions options = {

        OPT_HOST_REFRESH_RATE,
        OPT_HOST_SAMPLE_RATE,
        OPT_HOST_FRAMEBUF_WIDTH,
        OPT_HOST_FRAMEBUF_HEIGHT
    };

    // Current configuration
    HostConfig config = { };


    //
    // Initializing
    //

public:

    using SubComponent::SubComponent;

    Host& operator= (const Host& other) {

        CLONE(config)
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

    void _dump(Category category, std::ostream& os) const override;



    //
    // Methods from CoreObject
    //

public:

    const HostConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option opt) const override;
    void checkOption(Option opt, i64 value) override;
    void setOption(Option opt, i64 value) override;


    //
    // Working with temporary files and folders
    //

public:

    // Returns a path to a temporary folder
    fs::path tmp() const throws;

    // Assembles a path to a temporary file
    fs::path tmp(const string &name, bool unique = false) const throws;
};

}
