// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MonitorTypes.h"
#include "SubComponent.h"

namespace vamiga {

class Monitor final : public SubComponent {

    Descriptions descriptions = {
        {
            .type           = Class::Monitor,
            .name           = "Monitor",
            .description    = "Computer Monitor",
            .shell          = "monitor"
        }
    };

    Options options = {

        Opt::MON_PALETTE,
        Opt::MON_BRIGHTNESS,
        Opt::MON_CONTRAST,
        Opt::MON_SATURATION,
        Opt::MON_CENTER,
        Opt::MON_HCENTER,
        Opt::MON_VCENTER,
        Opt::MON_ZOOM,
        Opt::MON_HZOOM,
        Opt::MON_VZOOM,
        Opt::MON_ENHANCER,
        Opt::MON_UPSCALER,
        Opt::MON_BLUR,
        Opt::MON_BLUR_RADIUS,
        Opt::MON_BLOOM,
        Opt::MON_BLOOM_RADIUS,
        Opt::MON_BLOOM_BRIGHTNESS,
        Opt::MON_BLOOM_WEIGHT,
        Opt::MON_DOTMASK,
        Opt::MON_DOTMASK_BRIGHTNESS,
        Opt::MON_SCANLINES,
        Opt::MON_SCANLINE_BRIGHTNESS,
        Opt::MON_SCANLINE_WEIGHT,
        Opt::MON_DISALIGNMENT,
        Opt::MON_DISALIGNMENT_H,
        Opt::MON_DISALIGNMENT_V,
        Opt::MON_FLICKER,
        Opt::MON_FLICKER_WEIGHT
    };

    // Current configuration
    MonitorConfig config = { };


    //
    // Methods
    //

public:

    using SubComponent::SubComponent;

    Monitor& operator= (const Monitor& other) {

        CLONE(config)

        return *this;
    }

    //
    // Methods from Serializable
    //

public:

    template <class T> void serialize(T& worker) {

        if (isResetter(worker)) return;

        worker

        << config.palette
        << config.brightness
        << config.contrast
        << config.saturation

        << config.hCenter
        << config.vCenter
        << config.hZoom
        << config.vZoom

        << config.upscaler

        << config.blur
        << config.blurRadius

        << config.bloom
        << config.bloomRadius
        << config.bloomBrightness
        << config.bloomWeight

        << config.dotmask
        << config.dotMaskBrightness

        << config.scanlines
        << config.scanlineBrightness
        << config.scanlineWeight

        << config.disalignment
        << config.disalignmentH
        << config.disalignmentV;


    } SERIALIZERS(serialize);


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

    const MonitorConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt opt) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt opt, i64 value) override;


    //
    // Computing color values
    //

public:

    // TODO
};

}

