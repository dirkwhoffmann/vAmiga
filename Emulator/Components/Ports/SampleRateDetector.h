// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Chrono.h"

namespace vamiga {

class SampleRateDetector final : public SubComponent {

    Descriptions descriptions = {{

        .type           = SampleRateDetectorClass,
        .name           = "SampleRateDetector",
        .description    = "Sample rate detector",
        .shell          = ""
    }};

    ConfigOptions options = {

    };

    // Number of requested audio samples
    isize count = 0;

    // Measured timespan between two requests
    util::Clock delta;

    // Ring buffer storing the latest measurements
    util::RingBuffer<double, 256> buffer;

    // Number of filtered out outliers at both ends
    const isize trash = 6;


    //
    // Methods
    //

public:

    using SubComponent::SubComponent;

    SampleRateDetector& operator= (const SampleRateDetector& other) {

        return *this;
    }


    //
    // Methods from CoreObject
    //

private:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from CoreComponent
    //

private:

    template <class T>
    void serialize(T& worker)
    {

    } SERIALIZERS(serialize);

    void _didReset(bool hard) override;

public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


    //
    // Running the device
    //

    // Informs the detector that sound samples have been requested
    void feed(isize samples);

    // Returns the current sample rate
    double sampleRate();

};

}
