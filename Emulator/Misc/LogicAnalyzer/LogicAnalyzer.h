// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "LogicAnalyzerTypes.h"
#include "SubComponent.h"
// #include "Constants.h"

namespace vamiga {

class LogicAnalyzer final : public SubComponent {
    
    Descriptions descriptions = {{

        .type           = LogicAnalyzerClass,
        .name           = "LogicAnalyzer",
        .description    = "Logic Analyzer",
        .shell          = "logicanalyzer"
    }};

    ConfigOptions options = {

        OPT_LA_PROBE0,
        OPT_LA_PROBE1,
        OPT_LA_PROBE2,
        OPT_LA_PROBE3,
        OPT_LA_ADDR0,
        OPT_LA_ADDR1,
        OPT_LA_ADDR2,
        OPT_LA_ADDR3
    };

    // The current configuration
    LogicAnalyzerConfig config = {};
    
private:
    
    //
    // Constructing
    //
    
public:
    
    using SubComponent::SubComponent;
    
    LogicAnalyzer& operator= (const LogicAnalyzer& other) {

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
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:

    const LogicAnalyzerConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option option) const override;
    void checkOption(Option opt, i64 value) override;
    void setOption(Option option, i64 value) override;

    
    //
    // Serializing
    //
    
    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);
    void _didReset(bool hard) override;

    
    //
    // Servicing events
    //
    
public:
    
    // Services a probe event
    void servicePROEvent();
    
    // Schedules the next probe event
    void scheduleNextProEvent();
};

}

