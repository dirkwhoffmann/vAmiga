// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RTCTypes.h"
#include "SubComponent.h"

class RTC : public SubComponent {

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
    // Constructing
    //
    
public:
    
    using SubComponent::SubComponent;
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "RTC"; }
    void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.model;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << timeDiff
            << reg
            << lastCall
            << lastMeasure
            << lastMeasuredValue;
        }
    }
    
    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Configuring
    //
    
public:
    
    static RTCConfig getDefaultConfig();
    const RTCConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    
    bool isPresent() const { return config.model != RTC_NONE; }

    
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
