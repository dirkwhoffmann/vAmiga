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

class LogicAnalyzer final : public SubComponent, public Inspectable<LogicAnalyzerInfo> {
    
    Descriptions descriptions = {{

        .type           = Class::LogicAnalyzer,
        .name           = "LogicAnalyzer",
        .description    = "Logic Analyzer",
        .shell          = "logicanalyzer"
    }};

    ConfigOptions options = {

        Opt::LA_PROBE0,
        Opt::LA_PROBE1,
        Opt::LA_PROBE2,
        Opt::LA_PROBE3,
        Opt::LA_ADDR0,
        Opt::LA_ADDR1,
        Opt::LA_ADDR2,
        Opt::LA_ADDR3
    };

    // The current configuration
    LogicAnalyzerConfig config = {};
    
    // Recorded signal traces
    isize record[4][HPOS_CNT];
    
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
    // Methods from Inspectable
    //

public:

    void cacheInfo(LogicAnalyzerInfo &result) const override;
    
    
    //
    // Methods from Configurable
    //

public:

    const LogicAnalyzerConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;

    
    //
    // Serializing
    //
    
    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize, override);
    void _didReset(bool hard) override;

    
    //
    // Recording
    //
    
    
public:
        
    // Records data for all configured channels
    void recordSignals();
    
private:

    // Records all signal values belonging to the current DMA cycle
    void recordCurrent(isize hpos);

    // Records all signal values belonging to the previous DMA cycle
    void recordDelayed(isize hpos);

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

