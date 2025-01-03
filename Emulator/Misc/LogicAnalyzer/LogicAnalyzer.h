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
#include "Constants.h"

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
        OPT_LA_PROBE4,
        OPT_LA_PROBE5,
        OPT_LA_ADDR0,
        OPT_LA_ADDR1,
        OPT_LA_ADDR2,
        OPT_LA_ADDR3,
        OPT_LA_ADDR4,
        OPT_LA_ADDR5
    };

    // The current configuration
    LogicAnalyzerConfig config = {};
    
    // Recorded signal traces
    isize record[6][HPOS_CNT];
    
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
    void _pause() override;

    
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
    // Recording
    //
    
    
public:
        
    // Records data for all configured channels
    void recordSignals();
    void recordSignals(isize hpos);
    
private:
    
    // Enable or disables the logic analyzer based on the current config
    void checkEnable();
    
    
    //
    // Accessing
    //
    
public:
    
    isize get(isize channel, isize nr) { return record[channel][nr]; }
    isize *get(isize channel) { return record[channel]; }
};

}

