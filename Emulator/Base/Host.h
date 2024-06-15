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
#include "IOUtils.h"

namespace vamiga {

/* This class stores some information about the host system. The values have
 * are set the GUI on start and updated on-the-fly when a value changes.
 */
class Host : public SubComponent {

    Descriptions descriptions = {{

        .name           = "Host",
        .description    = "Host Computer"
    }};

    ConfigOptions options = {

    };
    
    // Audio sample rate
    double sampleRate = 44100.0;

    // Video refresh rate of the host monitor
    double refreshRate = 60.0;

    // Framebuffer dimensions
    isize frameBufferWidth = 0;
    isize frameBufferHeight = 0;


    //
    // Initializing
    //

public:

    using SubComponent::SubComponent;


    //
    // Methods from CoreObject
    //

private:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }

private:

    void _reset(bool hard) override { };
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override { return 0; }
    isize _save(u8 *buffer) override { return 0; }


    //
    // Accessing properties
    //

public:

    double getSampleRate() const { return sampleRate; }
    void setSampleRate(double hz);

    double getHostRefreshRate() const { return refreshRate; }
    void setHostRefreshRate(double fps);

    std::pair<isize, isize> getFrameBufferSize() const;
    void setFrameBufferSize(std::pair<isize, isize> size);


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
