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
#include "utl/wrappers.h"
#include "utl/storage/RingBuffer.h"

namespace vamiga {

class LogicAnalyzer final : public SubComponent {
    
    Descriptions descriptions = {{

        .type           = Class::LogicAnalyzer,
        .name           = "LogicAnalyzer",
        .description    = "Logic Analyzer",
        .shell          = "logicanalyzer"
    }};

    Options options = {

        Opt::LA_CONNECT,
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

public:

    // Result of the latest inspection
    utl::Backed<LogicAnalyzerInfo> info;

private:

    // Recorded signal trace
    RingBuffer<LogicAnalyzerSample, 512> trace;
    
    
    //
    // Constructing
    //
    
public:
    
    LogicAnalyzer(Amiga& ref);

    LogicAnalyzer& operator= (const LogicAnalyzer& other) {

        return *this;
    }


    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream &os) const override;

    
    //
    // Methods from CoreComponent
    //
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }
    void _pause() override;

    
    //
    // Analyzing
    //

public:

    LogicAnalyzerInfo cacheInfo() const;

    
    //
    // Methods from Configurable
    //

public:

    const LogicAnalyzerConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;

    
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
    
private:

    // Records all signal values belonging to the current DMA cycle
    void recordCurrent(LogicAnalyzerSample &sample);
    
    // Records all signal values belonging to the previous DMA cycle
    void recordDelayed(LogicAnalyzerSample &sample);

    // Enable or disables the logic analyzer based on the current config
    void checkEnable();
    
    
    //
    // Accessing
    //
    
public:
    
    // Returns the number of samples held in the signal trace
    isize traceCount() const { return trace.count(); }

    /* Returns a sample from the signal trace. Sample 0 is the most recently
     * recorded one, sample 1 the one before, and so on.
     */
    LogicAnalyzerSample traceSample(isize nr) const {

        auto count = trace.count();
        return nr >= 0 && nr < count ? trace.current(count - 1 - nr) : LogicAnalyzerSample { };
    }
};

}

