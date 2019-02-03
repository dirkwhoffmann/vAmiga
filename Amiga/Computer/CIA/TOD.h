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
    
    // Reference to the connected CIA
    CIA *cia;
    
    // The 24 bit counter
	TimeOfDay tod;

    // The counter latch
    TimeOfDay latch;

    // Alarm value
	TimeOfDay alarm;
	
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
    // Methods from HardwareComponent
    //

    void _powerOn() override;
	void _dump() override;

    
    //
    // Retrieving debug information
    //
 
    // Returns the current configuration.
    TODInfo getInfo();
    

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
    uint8_t getTodMinutes() { return frozen ? latch.minutes : tod.minutes; }

    // Returns the counter's intermediate byte (bits 8 - 15).
    uint8_t getTodSeconds() { return frozen ? latch.seconds : tod.seconds; }

    // Returns the counter's low byte (bits 0 - 7).
    uint8_t getTodTenth() { return frozen ? latch.tenth : tod.tenth; }

    //! Returns the alarm value's high byte (bits 16 - 23).
    uint8_t getAlarmMinutes() { return alarm.minutes; }

    // Returns the alarm value's intermediate byte (bits 8 - 15).
    uint8_t getAlarmSeconds() { return alarm.seconds; }

    // Returns the alarm value's low byte (bits 0 - 7).
    uint8_t getAlarmTenth() { return alarm.tenth; }
	
	// Sets the counter's high byte (bits 16 - 23).
    void setTodMinutes(uint8_t value) { tod.minutes = value; checkForInterrupt(); }
	
	// Sets the counter's intermediate byte (bits 8 - 15).
    void setTodSeconds(uint8_t value) { tod.seconds = value; checkForInterrupt(); }
	
	//! Sets the counter's low byte (bits 0 - 7).
	void setTodTenth(uint8_t value) { tod.tenth = value; checkForInterrupt(); }
	
	// Sets the alarm value's high byte (bits 16 - 23).
    void setAlarmMinutes(uint8_t value) { alarm.minutes = value; checkForInterrupt(); }
	
	// Sets the alarm value's intermediate byte (bits 8 - 15).
    void setAlarmSeconds(uint8_t value) { alarm.seconds = value; checkForInterrupt(); }
	
	// Sets the alarm value's low byte (bits 0 - 7).
    void setAlarmTenth(uint8_t value) { alarm.tenth = value; checkForInterrupt(); }
	
	// Increment the counter
	void increment();

    /* Updates variable 'matching'
     * If a positive edge occurs, the connected CIA is requested to trigger
     * an interrupt.
     */
    void checkForInterrupt();
};

#endif


