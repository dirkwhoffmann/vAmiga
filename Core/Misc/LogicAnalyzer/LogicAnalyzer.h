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

    /* Recorded signal trace, in two copies.
     *
     * 'trace' is the working buffer. It belongs to the emulator thread, which
     * appends to it every DMA cycle, and nothing else may touch it.
     *
     * 'stable' is what the GUI reads. The emulator thread copies the working
     * buffer over at the end of each frame, under 'mutex'. Locking the working
     * buffer itself was the alternative and a poor trade: it would take a lock
     * three and a half million times a second to hand data to a reader that
     * wants it sixty times a second, and it would let a GUI read block the
     * emulator thread mid-cycle. Here the lock is taken once per frame and is
     * never held for longer than the copy.
     *
     * The copy also happens whenever the emulator pauses (see _pause()) and
     * after any change that edits the working buffer, so a paused machine
     * shows what was actually recorded rather than the state at the last
     * frame boundary. While running, the frame rate makes the difference
     * invisible.
     */
    RingBuffer<LogicAnalyzerSample, 512> trace;
    RingBuffer<LogicAnalyzerSample, 512> stable;
    
    
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

    // Publishes the working buffer to the GUI-visible one (end of frame)
    void eofHandler();
    
private:

    // Records all signal values belonging to the current DMA cycle
    void recordCurrent(LogicAnalyzerSample &sample);
    
    // Records all signal values belonging to the previous DMA cycle
    void recordDelayed(LogicAnalyzerSample &sample);

    // Enable or disables the logic analyzer based on the current config
    void checkEnable();

    // Copies the working buffer into the GUI-visible one
    void publish();
    
    
    //
    // Accessing
    //
    
public:
    
    /* Both accessors read the GUI-visible buffer, not the working one, and
     * hold the lock the copy is made under. They are meant to be called from
     * the GUI thread.
     */

    // Returns the number of samples held in the signal trace
    isize traceCount() const {

        {   SYNCHRONIZED

            return stable.count();
        }
    }

    /* Returns a sample from the signal trace. Sample 0 is the most recently
     * recorded one, sample 1 the one before, and so on.
     */
    LogicAnalyzerSample traceSample(isize nr) const {

        {   SYNCHRONIZED

            auto count = stable.count();
            return nr >= 0 && nr < count ? stable.current(count - 1 - nr) : LogicAnalyzerSample { };
        }
    }
};

}

