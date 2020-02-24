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

#include "AmigaComponent.h"

/* 24-bit counter
 * Each CIA chip contains a 24-bit counter with an alarm. When the alarm value
 * is reached, an interrupt is initiated.
 */
class TOD : public AmigaComponent {
    
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
    
    TOD(CIA *cia, Amiga& ref);

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
    void _reset() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

public:

    // Returns the result of the most recent call to inspect()
    CounterInfo getInfo();


    //
    // Accessing properties
    //

    // Returns the counter's high byte (bits 16 - 23).
    u8 getCounterHi() { return frozen ? latch.hi : tod.hi; }

    // Returns the counter's intermediate byte (bits 8 - 15).
    u8 getCounterMid() { return frozen ? latch.mid : tod.mid; }

    // Returns the counter's low byte (bits 0 - 7).
    u8 getCounterLo() { return frozen ? latch.lo : tod.lo; }

    //! Returns the alarm value's high byte (bits 16 - 23).
    u8 getAlarmHi() { return alarm.hi; }

    // Returns the alarm value's intermediate byte (bits 8 - 15).
    u8 getAlarmMid() { return alarm.mid; }

    // Returns the alarm value's low byte (bits 0 - 7).
    u8 getAlarmLo() { return alarm.lo; }

    // Sets the counter's high byte (bits 16 - 23).
    void setCounterHi(u8 value) { tod.hi = value; checkForInterrupt(); }

    // Sets the counter's intermediate byte (bits 8 - 15).
    void setCounterMid(u8 value) { tod.mid = value; checkForInterrupt(); }

    //! Sets the counter's low byte (bits 0 - 7).
    void setCounterLo(u8 value) { tod.lo = value; checkForInterrupt(); }

    // Sets the alarm value's high byte (bits 16 - 23).
    void setAlarmHi(u8 value) { alarm.hi = value; checkForInterrupt(); }

    // Sets the alarm value's intermediate byte (bits 8 - 15).
    void setAlarmMid(u8 value) { alarm.mid = value; checkForInterrupt(); }

    // Sets the alarm value's low byte (bits 0 - 7).
    void setAlarmLo(u8 value) { alarm.lo = value; checkForInterrupt(); }


    //
    // Running the component
    //
    
private:
    
    // Freezes the counter
    void freeze() { if (!frozen) { latch.value = tod.value; frozen = true; } }
    
    // Unfreezes the counter
    void defreeze() { frozen = false; }
    
    // Stops the counter
    void stop() { stopped = true; }
    
    // Starts the counter
    void cont() { stopped = false; }

    // Increments the counter
    void increment();
    bool incLoNibble(u8 &counter);
    bool incHiNibble(u8 &counter);

    /* Updates variable 'matching'
     * If a positive edge occurs, the connected CIA is requested to trigger
     * an interrupt.
     */
    void checkForInterrupt();
};

#endif


