// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RamseyTypes.h"
#include "SubComponent.h"

namespace vamiga {

/* Ramsey is the DRAM controller of the A3000 and the A4000. It manages the
 * motherboard RAM which is located at the top of the 16 MB window
 * $07000000 - $07FFFFFF. The CPU accesses its registers in the $DE0000 bank:
 *
 *     $DE0003 : Control register
 *     $DE0043 : Revision register
 *
 * Bits of the control register:
 *
 *     0 : Page mode enable
 *     1 : Burst mode enable
 *     2 : Wrap (burst wrap-around)
 *     3 : DRAM size (0 = 256K x 4, 1 = 1M x 4)
 *     4 : Skip (static column) mode
 *   5-6 : Refresh rate
 *     7 : Test mode
 *
 * The control bits only affect timing which is not emulated. The register is
 * emulated as a plain storage location.
 */
class Ramsey final : public SubComponent {

    Descriptions descriptions = {{

        .type           = Class::Ramsey,
        .name           = "Ramsey",
        .description    = "Memory Controller",
        .shell          = "ramsey"
    }};

    Options options = {

        Opt::RAMSEY_REVISION
    };

    // The current configuration
    RamseyConfig config = {};

    // The control register
    u8 control = 0;


    //
    // Methods
    //

public:

    using SubComponent::SubComponent;

    Ramsey& operator= (const Ramsey& other) {

        CLONE(control)
        CLONE(config)

        return *this;
    }


    //
    // Methods from Serializable
    //

private:

    template <class T>
    void serialize(T& worker)
    {
        if (isSoftResetter(worker)) return;

        worker

        << control;

        if (isResetter(worker)) return;

        worker

        << config.revision;
    }

    void operator << (SerResetter &worker) override { serialize(worker); }
    void operator << (SerChecker &worker) override { serialize(worker); }
    void operator << (SerCounter &worker) override { serialize(worker); }
    void operator << (SerReader &worker) override { serialize(worker); }
    void operator << (SerWriter &worker) override { serialize(worker); }


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

    const RamseyConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt option, i64 value) override;
    void setOption(Opt option, i64 value) override;


    //
    // Analyzing
    //

public:

    // Checks whether a Ramsey chip is installed
    bool isPresent() const { return config.revision != RamseyRevision::NONE; }

    // Returns the value of the revision register
    u8 revisionId() const;


    //
    // Accessing registers
    //

public:

    // Reads a register (addr is an address inside the $DE0000 bank)
    u8 peek8(u32 addr) const { return spypeek8(addr); }
    u8 spypeek8(u32 addr) const;

    // Writes a register (addr is an address inside the $DE0000 bank)
    void poke8(u32 addr, u8 value);
};

}
