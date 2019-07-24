// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _TOD_INC
#define _TOD_INC

#include "HardwareComponent.h"

class CIA;

/* 24-bit counter
 * Each CIA chip contains a 24-bit counter with an alarm. When the alarm value
 * is reached, an interrupt is initiated.
 */
class TOD : public HardwareComponent {
    
    friend CIA;
    
    private:
    
    // Information shown in the GUI inspector panel
    CounterInfo info;
    
    private:
    
    // Reference to the connected CIA
    CIA *cia;
    
    // The 24 bit counter
    Counter24 tod;
    
    // The counter latch
    Counter24 latch;
    
    // Alarm value
    Counter24 alarm;
    
    /* Indicates if the TOD registers are frozen
     * The CIA chip freezes the registers when the counter's high byte (bits
     * 16 - 23) is read and reactivates them, when the low byte (bits 0 - 7)
     * is read. Although the values stay constant, the internal clock continues
     * to run.
     */
    bool frozen;
    
    /* Indicates if the TOD clock is halted.
     * The CIA chip stops the TOD clock when the counter's high byte (bits
     * 16 - 23) is written and restarts it, when the low byte (bits 0 - 7) is
     * written.
     */
    bool stopped;
    
    /* Indicates if tod time matches the alarm value
     * This value is read in checkForInterrupt() for edge detection.
     */
    bool matching;
    
    public:
    
    //
    // Creating and destructing
    //
    
    TOD(CIA *cia);


    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & tod.value
        & latch.value
        & alarm.value
        & frozen
        & stopped
        & matching;
    }
    
    
    //
    // Methods from HardwareComponent
    //
    
    void _powerOn() override;
    void _inspect() override;
    void _dump() override;

public:

    size_t _loadFromBuffer(uint8_t *buffer) override;
    size_t _saveToBuffer(uint8_t *buffer) override;

    
    //
    // Reading the internal state
    //
    
    public:
    
    // Returns the latest internal state recorded by inspect()
    CounterInfo getInfo();
    
    
    //
    // Running the component
    //
    
    private:
    
    // Freezes the counter.
    void freeze() { if (!frozen) { latch.value = tod.value; frozen = true; } }
    
    // Unfreezes the counter.
    void defreeze() { frozen = false; }
    
    // Stops the counter.
    void stop() { stopped = true; }
    
    // Starts the counter.
    void cont() { stopped = false; }
    
    // Returns the counter's high byte (bits 16 - 23).
    uint8_t getCounterHi() { return frozen ? latch.hi : tod.hi; }
    
    // Returns the counter's intermediate byte (bits 8 - 15).
    uint8_t getCounterMid() { return frozen ? latch.mid : tod.mid; }
    
    // Returns the counter's low byte (bits 0 - 7).
    uint8_t getCounterLo() { return frozen ? latch.lo : tod.lo; }
    
    //! Returns the alarm value's high byte (bits 16 - 23).
    uint8_t getAlarmHi() { return alarm.hi; }
    
    // Returns the alarm value's intermediate byte (bits 8 - 15).
    uint8_t getAlarmMid() { return alarm.mid; }
    
    // Returns the alarm value's low byte (bits 0 - 7).
    uint8_t getAlarmLo() { return alarm.lo; }
    
    // Sets the counter's high byte (bits 16 - 23).
    void setCounterHi(uint8_t value) { tod.hi = value; checkForInterrupt(); }
    
    // Sets the counter's intermediate byte (bits 8 - 15).
    void setCounterMid(uint8_t value) { tod.mid = value; checkForInterrupt(); }
    
    //! Sets the counter's low byte (bits 0 - 7).
    void setCounterLo(uint8_t value) { tod.lo = value; checkForInterrupt(); }
    
    // Sets the alarm value's high byte (bits 16 - 23).
    void setAlarmHi(uint8_t value) { alarm.hi = value; checkForInterrupt(); }
    
    // Sets the alarm value's intermediate byte (bits 8 - 15).
    void setAlarmMid(uint8_t value) { alarm.mid = value; checkForInterrupt(); }
    
    // Sets the alarm value's low byte (bits 0 - 7).
    void setAlarmLo(uint8_t value) { alarm.lo = value; checkForInterrupt(); }
    
    // Increment the counter
    void increment();
    
    /* Updates variable 'matching'
     * If a positive edge occurs, the connected CIA is requested to trigger
     * an interrupt.
     */
    void checkForInterrupt();
};

#endif


