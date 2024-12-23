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
#include "RemoteServerTypes.h"

std::string generate_metrics();
int createPromServer();

namespace vamiga {

class PromServer final : public SubComponent {

    friend class RemoteManager;

    Descriptions descriptions = {{

        .name           = "PromServer",
        .description    = "Prometheus Server",
        .shell          = "prom"
    }};

    ConfigOptions options = {

    };

    // The current server state
    SrvState state = SRV_STATE_OFF;

public:

    using SubComponent::SubComponent;

    PromServer& operator= (const PromServer& other) {

        return *this;
    }


    //
    // Methods from CoreObject
    //

protected:

    void _dump(Category category, std::ostream& os) const override { };

public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from CoreComponent
    //

private:

    template <class T>
    void serialize(T& worker)
    {

    } SERIALIZERS(serialize);


    //
    // Methods from Configurable
    //

public:

    // const PromServerConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    // i64 getOption(Option option) const override;
    // void checkOption(Option opt, i64 value) override;
    // void setOption(Option option, i64 value) override;


    //
    // Experimental
    //

public:

    // Launch the remote server
    void start();

    // Shuts down the remote server
    void stop();

    // Used by the launch daemon to determine if actions should be taken
    bool shouldRun() { return true; }

    // Experimental
    string generate_metrics();
};

}
