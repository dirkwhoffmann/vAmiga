// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RTCTypes.h"
#include "SubComponent.h"

namespace vamiga {

class RTC final : public SubComponent {

    Descriptions descriptions = {{

        .type           = RTCClass,
        .name           = "RTC",
        .description    = "Real-time Clock",
        .shell          = "rtc"
    }};

    ConfigOptions options = {

        OPT_RTC_MODEL
    };
    
    // The current configuration
    RTCConfig config = {};

    /* The current time of the real-time clock. The RTC stores the time as a
     * difference to the time provided by the host machine. I.e.:
     *
     *     Time of the real-time clock = Time of the host machine + timeDiff
     *
     * By default, this variable is 0 which means that the Amiga's real-time
     * clock is identical to the one in the host machine.
     */
    i64 timeDiff;
    
    // The RTC registers
    u8 reg[4][16];
    
    // Time stamp of the last call to function getTime()
    Cycle lastCall;

    // Remembers the most recent query of the host machine's real-time clock
    Cycle lastMeasure;

    // The result of the most recent query
    i64 lastMeasuredValue;
    
    
    //
    // Methods
    //

public:
    
    using SubComponent::SubComponent;
    
    RTC& operator= (const RTC& other) {
        
        CLONE(timeDiff)
        CLONE_ARRAY(reg[0])
        CLONE_ARRAY(reg[1])
        CLONE_ARRAY(reg[2])
        CLONE_ARRAY(reg[3])
        CLONE(lastCall)
        CLONE(lastMeasure)
        CLONE(lastMeasuredValue)

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

        << timeDiff
        << reg
        << lastCall
        << lastMeasure
        << lastMeasuredValue;

        if (isResetter(worker)) return;

        worker

        << config.model;
    }

    void operator << (SerResetter &worker) override;
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

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from Configurable
    //

public:
    
    const RTCConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option option) const override;
    void checkOption(Option opt, i64 value) override;
    void setOption(Option option, i64 value) override;

    
    //
    // Accessing time
    //
    
    // Returns the current value of the real-time clock
    time_t getTime();
    
    // Sets the current value of the real-time clock
    void setTime(time_t t);
    
    
    //
    // Accessing registers
    //
    
public:
    
    // Updates all 16 RTC registers
    void update();
    
    // Reads one of the 16 RTC registers (call update() first)
    u8 peek(isize nr);

    // Returns the current value in the register cache
    u8 spypeek(isize nr) const;
    
    // Writes one of the 16 RTC registers
    void poke(isize nr, u8 value);

private:

    // Reads one of the three control registers
    u8 peekD() const { return reg[0][0xD]; };
    u8 peekE() const { return config.model == RTC_RICOH ? 0 : reg[0][0xE]; }
    u8 peekF() const { return config.model == RTC_RICOH ? 0 : reg[0][0xF]; }

    // Writes one of the three control registers
    void pokeD(u8 value) { reg[0][0xD] = value; }
    void pokeE(u8 value) { reg[0][0xE] = value; }
    void pokeF(u8 value) { reg[0][0xF] = value; }

    /* Returns the currently selected register bank. The Ricoh clock comprises
     * four register banks. A bank is selected by by bits 0 and 1 in control
     * register D. The OKI clock has a single bank, only.
     */
    isize bank() const { return config.model == RTC_RICOH ? (reg[0][0xD] & 0b11) : 0; }
    
    /* Converts the register value to the internally stored time-stamp. This
     * function has to be called *before* a RTC register is *read*.
     */
    void time2registers();
    void time2registersOki(tm *t);
    void time2registersRicoh(tm *t);

    /* Converts the internally stored time-stamp to register values. This
     * function has to be called *after* a RTC register is *written*.
     */
    void registers2time();
    void registers2timeOki(tm *t);
    void registers2timeRicoh(tm *t);
};

}
