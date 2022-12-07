// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CIATypes.h"
#include "SubComponent.h"
#include "AgnusTypes.h"
#include "TOD.h"

namespace vamiga {

constexpr u64 CIACountA0 =   (1ULL << 0); // Decrements timer A
constexpr u64 CIACountA1 =   (1ULL << 1);
constexpr u64 CIACountA2 =   (1ULL << 2);
constexpr u64 CIACountA3 =   (1ULL << 3);
constexpr u64 CIACountB0 =   (1ULL << 4); // Decrements timer B
constexpr u64 CIACountB1 =   (1ULL << 5);
constexpr u64 CIACountB2 =   (1ULL << 6);
constexpr u64 CIACountB3 =   (1ULL << 7);
constexpr u64 CIALoadA0 =    (1ULL << 8); // Loads timer A
constexpr u64 CIALoadA1 =    (1ULL << 9);
constexpr u64 CIALoadA2 =    (1ULL << 10);
constexpr u64 CIALoadB0 =    (1ULL << 11); // Loads timer B
constexpr u64 CIALoadB1 =    (1ULL << 12);
constexpr u64 CIALoadB2 =    (1ULL << 13);
constexpr u64 CIAPB6Low0 =   (1ULL << 14); // Sets pin PB6 low
constexpr u64 CIAPB6Low1 =   (1ULL << 15);
constexpr u64 CIAPB7Low0 =   (1ULL << 16); // Sets pin PB7 low
constexpr u64 CIAPB7Low1 =   (1ULL << 17);
constexpr u64 CIASetInt0 =   (1ULL << 18); // Triggers an interrupt
constexpr u64 CIASetInt1 =   (1ULL << 19);
constexpr u64 CIAClearInt0 = (1ULL << 20); // Releases the interrupt line
constexpr u64 CIAOneShotA0 = (1ULL << 21);
constexpr u64 CIAOneShotB0 = (1ULL << 22);
constexpr u64 CIAReadIcr0 =  (1ULL << 23); // Indicates that ICR was read recently
constexpr u64 CIAReadIcr1 =  (1ULL << 24);
constexpr u64 CIAClearIcr0 = (1ULL << 25); // Clears bit 8 in ICR register
constexpr u64 CIAClearIcr1 = (1ULL << 26);
constexpr u64 CIAClearIcr2 = (1ULL << 27);
constexpr u64 CIAAckIcr0 =   (1ULL << 28); // Clears bit 0 - 7 in ICR register
constexpr u64 CIAAckIcr1 =   (1ULL << 29);
constexpr u64 CIASetIcr0 =   (1ULL << 30); // Sets bit 8 in ICR register
constexpr u64 CIASetIcr1 =   (1ULL << 31);
constexpr u64 CIATODInt0 =   (1ULL << 32); // Triggers an IRQ with TOD as source
constexpr u64 CIASerInt0 =   (1ULL << 33); // Triggers an IRQ with serial reg as source
constexpr u64 CIASerInt1 =   (1ULL << 34);
constexpr u64 CIASerInt2 =   (1ULL << 35);
constexpr u64 CIASdrToSsr0 = (1ULL << 36); // Move serial data reg to serial shift reg
constexpr u64 CIASdrToSsr1 = (1ULL << 37);
constexpr u64 CIASsrToSdr0 = (1ULL << 38); // Move serial shift reg to serial data reg
constexpr u64 CIASsrToSdr1 = (1ULL << 39);
constexpr u64 CIASsrToSdr2 = (1ULL << 40);
constexpr u64 CIASsrToSdr3 = (1ULL << 41);
constexpr u64 CIASerClk0 =   (1ULL << 42); // Clock signal driving the serial register
constexpr u64 CIASerClk1 =   (1ULL << 43);
constexpr u64 CIASerClk2 =   (1ULL << 44);
constexpr u64 CIASerClk3 =   (1ULL << 45);
constexpr u64 CIALast =      (1ULL << 46);

constexpr u64 CIADelayMask = ~CIALast
& ~CIACountA0 & ~CIACountB0 & ~CIALoadA0 & ~CIALoadB0 & ~CIAPB6Low0
& ~CIAPB7Low0 & ~CIASetInt0 & ~CIAClearInt0 & ~CIAOneShotA0 & ~CIAOneShotB0
& ~CIAReadIcr0 & ~CIAClearIcr0 & ~CIAAckIcr0 & ~CIASetIcr0 & ~CIATODInt0
& ~CIASerInt0 & ~CIASdrToSsr0 & ~CIASsrToSdr0 & ~CIASerClk0;

class CIA : public SubComponent {
    
    friend class TOD;
    
protected:

    // Identification number (0 = CIA A, 1 = CIA B)
    const int nr;

    // Current configuration
    CIAConfig config = {};

    // Result of the latest inspection
    mutable CIAInfo info = {};


    //
    // Sub components
    //

public:
    
    TOD tod = TOD(*this, amiga);


    //
    // Internals
    //
    
protected:
    
    // The CIA has been executed up to this master clock cycle
    Cycle clock;

    // Total number of skipped cycles (used by the debugger, only)
    Cycle idleCycles;

    // Action flags
    u64 delay;
    u64 feed;

    
    //
    // Timers
    //
    
protected:
    
    // Timer counters
    u16 counterA;
    u16 counterB;

    // Timer latches
    u16 latchA;
    u16 latchB;

    // Timer control registers
    u8 cra;
    u8 crb;

    
    //
    // Interrupts
    //
    
    // Interrupt mask register
    u8 imr;

    // Interrupt control register
    u8 icr;
    
    // ICR bits that need to deleted when CIAAckIcr1 hits
    u8 icrAck;

    
    //
    // Peripheral ports
    //
    
protected:
    
    // Data registers
    u8 pra;
    u8 prb;
    
    // Data directon registers
    u8 ddra;
    u8 ddrb;
    
    // Bit mask for PB outputs (0 = port register, 1 = timer)
    u8 pb67TimerMode;
    
    // PB output bits 6 and 7 in timer mode
    u8 pb67TimerOut;
    
    // PB output bits 6 and 7 in toggle mode
    u8 pb67Toggle;

    
    //
    // Port values (chip pins)
    //
    
    // Peripheral port pins
    u8 pa;
    u8 pb;

    // Serial port pins
    bool sp;
    bool cnt;
    
    // Interrupt request pin
    bool irq;
    
    
    //
    // Shift register
    //
    
protected:
    
    /* Serial data register
     * http://unusedino.de/ec64/technical/misc/cia6526/serial.html
     * "The serial port is a buffered, 8-bit synchronous shift register system.
     *  A control bit selects input or output mode. In input mode, data on the
     *  SP pin is shifted into the shift register on the rising edge of the
     *  signal applied to the CNT pin. After 8 CNT pulses, the data in the shift
     *  register is dumped into the Serial Data Register and an interrupt is
     *  generated. In the output mode, TIMER A is used for the baud rate
     *  generator. Data is shifted out on the SP pin at 1/2 the underflow rate
     *  of TIMER A. [...] Transmission will start following a write to the
     *  Serial Data Register (provided TIMER A is running and in continuous
     *  mode). The clock signal derived from TIMER A appears as an output on the
     *  CNT pin. The data in the Serial Data Register will be loaded into the
     *  shift register then shift out to the SP pin when a CNT pulse occurs.
     *  Data shifted out becomes valid on the falling edge of CNT and remains
     *  valid until the next falling edge. After 8 CNT pulses, an interrupt is
     *  generated to indicate more data can be sent. If the Serial Data Register
     *  was loaded with new information prior to this interrupt, the new data
     *  will automatically be loaded into the shift register and transmission
     *  will continue. If the microprocessor stays one byte ahead of the shift
     *  register, transmission will be continuous. If no further data is to be
     *  transmitted, after the 8th CNT pulse, CNT will return high and SP will
     *  remain at the level of the last data bit transmitted. SDR data is
     *  shifted out MSB first and serial input data should also appear in this
     *  format.
     */
    u8 sdr;

    // Serial shift register
    u8 ssr;
    
    /* Shift register counter
     * The counter is set to 8 when the shift register is loaded and decremented
     * when a bit is shifted out.
     */
    i8 serCounter;
    
    
    //
    // Sleep logic
    //

public:
    
    // Indicates if the CIA is currently idle
    bool sleeping;
    
    /* The last executed cycle before the chip went idle.
     * The variable is set in sleep()
     */
    Cycle sleepCycle;
    
    /* The first cycle to be executed after the chip went idle.
     * The variable is set in sleep()
     */
    Cycle wakeUpCycle;

protected:
    
    /* Idle counter. When the CIA's state does not change during execution,
     * this variable is increased by one. If it exceeds a certain threshhold,
     * the chip is put into idle state via sleep().
     */
    u8 tiredness;
    
    
    //
    // Initializing
    //

public:
    
    CIA(int n, Amiga& ref);

    bool isCIAA() const { return nr == 0; }
    bool isCIAB() const { return nr == 1; }

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //

    void _initialize() override;
    void _reset(bool hard) override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker
        
        << config.revision
        << config.todBug
        << config.eClockSyncing;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << clock
            << idleCycles
            << tiredness
            << sleeping
            << sleepCycle
            << wakeUpCycle;
        }

        worker
        
        << delay
        << feed
        << counterA
        << counterB
        << latchA
        << latchB
        << cra
        << crb
        << imr
        << icr
        << icrAck
        << pra
        << prb
        << ddra
        << ddrb
        << pb67TimerMode
        << pb67TimerOut
        << pb67Toggle
        << pa
        << pb
        << sp
        << cnt
        << irq
        << sdr
        << ssr
        << serCounter;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Configuring
    //
    
public:

    void resetConfig() override;
    const CIAConfig &getConfig() const { return config; }
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    
    
    //
    // Analyzing
    //
    
public:
    
    CIAInfo getInfo() const { return AmigaComponent::getInfo(info); }
    Cycle getClock() const { return clock; }
    
protected:
    
    void _inspect() const override;

    
    //
    // Accessing registers
    //
    
public:
    
    // Reads a value from a CIA register
    u8 peek(u16 addr);
    
    // Reads a value from a CIA register without causing side effects
    u8 spypeek(u16 addr) const;

    // Writes a value into a CIA register
    void poke(u16 addr, u8 value);
    
    
    //
    // Accessing the data ports
    //
    
public:
    
    // Returns the data registers (call updatePA() or updatePB() first)
    u8 getPA() const { return pa; }
    u8 getPB() const { return pb; }

private:

    // Returns the data direction register
    u8 getDDRA() const { return ddra; }
    u8 getDDRB() const { return ddrb; }

    // Updates variable pa with the value we currently see at port A
    virtual void updatePA() = 0;
    virtual u8 computePA() const = 0;

    // Returns the value driving port A from inside the chip
    virtual u8 portAinternal() const = 0;
    
    // Returns the value driving port A from outside the chip
    virtual u8 portAexternal() const = 0;
    
    // Updates variable pa with the value we currently see at port B
    virtual void updatePB() = 0;
    virtual u8 computePB() const = 0;
    
    // Values driving port B from inside the chip
    virtual u8 portBinternal() const = 0;
    
    // Values driving port B from outside the chip
    virtual u8 portBexternal() const = 0;
    
protected:
    
    // Action method for poking the PA or PB register
    virtual void pokePA(u8 value) { pra = value; updatePA(); }
    virtual void pokePB(u8 value) { prb = value; updatePB(); }

    // Action method for poking the DDRA or DDRB register
    virtual void pokeDDRA(u8 value) { ddra = value; updatePA(); }
    virtual void pokeDDRB(u8 value) { ddrb = value; updatePB(); }

    
    //
    // Accessing port pins
    //
    
public:
    
    // Getter for the interrupt line
    bool getIrq() const { return irq; }

    // Simulates an edge edge on the flag pin
    void emulateRisingEdgeOnFlagPin();
    void emulateFallingEdgeOnFlagPin();

    // Simulates an edge on the CNT pin
    void emulateRisingEdgeOnCntPin();
    void emulateFallingEdgeOnCntPin();

    // Sets the serial port pin
    void setSP(bool value) { sp = value; }
    
    
    //
    // Handling interrupts
    //

public:
    
    // Handles an interrupt request from TOD
    void todInterrupt();

private:

    // Requests the CPU to interrupt
    virtual void pullDownInterruptLine() = 0;
    
    // Removes the interrupt requests
    virtual void releaseInterruptLine() = 0;
    
    // Loads a latched value into timer
    void reloadTimerA(u64 *delay);
    void reloadTimerB(u64 *delay);
    
    // Triggers an interrupt (invoked inside executeOneCycle())
    void triggerTimerIrq(u64 *delay);
    void triggerTodIrq(u64 *delay);
    void triggerFlagPinIrq(u64 *delay);
    void triggerSerialIrq(u64 *delay);
    
    
    //
    // Handling events
    //
    
public:
    
    // Services an event in the CIA slot
    void serviceEvent(EventID id);
    
    // Schedules the next execution event
    void scheduleNextExecution();
    
    // Schedules the next wakeup event
    void scheduleWakeUp();

    
    //
    // Executing
    //
    
public:

    // Executes the CIA for one CIA cycle
    void executeOneCycle();
    
    
    //
    // Speeding up (sleep logic)
    //
    
private:
    
    // Puts the CIA into idle state
    void sleep();
    
public:
    
    // Emulates all previously skipped cycles
    void wakeUp();
    void wakeUp(Cycle targetCycle);
    
    // Returns true if the CIA is in idle state or not
    bool isSleeping() const { return sleeping; }
    bool isAwake() const { return !sleeping; }

    // Returns the number of cycles the CIA is idle since
    CIACycle idleSince() const;
    
    // Retruns the total number of cycles the CIA was idle
    CIACycle idleTotal() const { return idleCycles; }
};


//
// CIAA
//

class CIAA : public CIA {
    
public:
    
    CIAA(Amiga& ref) : CIA(0, ref) { };
    
private:

    const char *getDescription() const override { return "CIAA"; }
    
    void _powerOn() override;
    void _powerOff() override;
    
    void pullDownInterruptLine() override;
    void releaseInterruptLine() override;
    
    u8 portAinternal() const override;
    u8 portAexternal() const override;
    void updatePA() override;
    u8 computePA() const override;

    u8 portBinternal() const override;
    u8 portBexternal() const override;
    void updatePB() override;
    u8 computePB() const override;
    
public:

    // Indicates if the power LED is currently on or off
    bool powerLED() const { return (pa & 0x2) == 0; }

    // Emulates the reception of a keycode from the keyboard
    void setKeyCode(u8 keyCode);
};


//
// CIAB
//

class CIAB : public CIA {
    
public:
    
    CIAB(Amiga& ref) : CIA(1, ref) { };
    
private:

    const char *getDescription() const override { return "CIAB"; }

    void pullDownInterruptLine() override;
    void releaseInterruptLine() override;
    
    u8 portAinternal() const override;
    u8 portAexternal() const override;
    void updatePA() override;
    u8 computePA() const override;

    u8 portBinternal() const override;
    u8 portBexternal() const override;
    void updatePB() override;
    u8 computePB() const override;
};

}
