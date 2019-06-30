// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _RTC_INC
#define _RTC_INC

#include "HardwareComponent.h"

class RTC : public HardwareComponent {
 
    private:
    
    /* The currently stored time
     * The RTC stores the time in form of the time difference relative to the
     * the current time that is provided by the host machine. I.e.:
     *
     *     Time of the real-time clock = Time of the host machine + time
     *
     * By default, this variable is 0 which means that the Amiga's real-time
     * clock is identical to the one in the host machine.
     */
    time_t timeDiff;
    
    // The 16 RTC 4-bit registers
    uint8_t reg[16];
    
    // The last call to function getTime()
    Cycle lastCall;

    // The last time we've queried the host system's real-time clock
    Cycle lastMeasure;

    // The result of our last query
    time_t lastMeasuredValue;
    
    
    //
    // Constructing and destructing
    //
    
    public:
    
    RTC();
    
    
    //
    // Methods from HardwareComponent
    //
    
    private:
    
    void _powerOn() override;
    void _reset() override;
    void _dump() override;
    
    
    //
    // Accessing the stored time
    //
    
    // Returns the current value of the real-time clock
    time_t getTime();
    
    // Sets the current value of the real-time clock
    void setTime(time_t t);
    
    
    //
    // Accessing register
    //
    
    public:
    
    // Reads one of the 16 RTC registers
    uint8_t peek(unsigned nr);
    
    // Writes one of the 16 RTC registers
    void poke(unsigned nr, uint8_t value);
    
    private:
    
    /* Converts the register value to the internally stored time-stamp.
     * This function has to be called *before* a RTC register is *read*.
     */
    void time2registers();
    
    /* Converts the internally stored time-stamp to register values.
     * This function has to be called *after* a RTC register is *written*.
     */
    void registers2time();
};

#endif
