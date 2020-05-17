// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CIA_H
#define _CIA_H

#include "TOD.h"

// Action flags
#define CIACountA0     (1ULL << 0) // Decrements timer A
#define CIACountA1     (1ULL << 1)
#define CIACountA2     (1ULL << 2)
#define CIACountA3     (1ULL << 3)
#define CIACountB0     (1ULL << 4) // Decrements timer B
#define CIACountB1     (1ULL << 5)
#define CIACountB2     (1ULL << 6)
#define CIACountB3     (1ULL << 7)
#define CIALoadA0      (1ULL << 8) // Loads timer A
#define CIALoadA1      (1ULL << 9)
#define CIALoadA2      (1ULL << 10)
#define CIALoadB0      (1ULL << 11) // Loads timer B
#define CIALoadB1      (1ULL << 12)
#define CIALoadB2      (1ULL << 13)
#define CIAPB6Low0     (1ULL << 14) // Sets pin PB6 low
#define CIAPB6Low1     (1ULL << 15)
#define CIAPB7Low0     (1ULL << 16) // Sets pin PB7 low
#define CIAPB7Low1     (1ULL << 17)
#define CIASetInt0     (1ULL << 18) // Triggers an interrupt
#define CIASetInt1     (1ULL << 19)
#define CIAClearInt0   (1ULL << 20) // Releases the interrupt line
#define CIAOneShotA0   (1ULL << 21)
#define CIAOneShotB0   (1ULL << 22)
#define CIAReadIcr0    (1ULL << 23) // Indicates that ICR was read recently
#define CIAReadIcr1    (1ULL << 24)
#define CIAClearIcr0   (1ULL << 25) // Clears bit 8 in ICR register
#define CIAClearIcr1   (1ULL << 26)
#define CIAClearIcr2   (1ULL << 27)
#define CIAAckIcr0     (1ULL << 28) // Clears bit 0 - 7 in ICR register
#define CIAAckIcr1     (1ULL << 29)
#define CIASetIcr0     (1ULL << 30) // Sets bit 8 in ICR register
#define CIASetIcr1     (1ULL << 31)
#define CIATODInt0     (1ULL << 32) // Triggers an IRQ with TOD as source
#define CIASerInt0     (1ULL << 33) // Triggers an IRQ with serial reg as source
#define CIASerInt1     (1ULL << 34)
#define CIASerInt2     (1ULL << 35)
#define CIASerLoad0    (1ULL << 36) // Loads the serial shift register
#define CIASerLoad1    (1ULL << 37)
#define CIASerClk0     (1ULL << 38) // Clock signal driving the serial register
#define CIASerClk1     (1ULL << 39)
#define CIASerClk2     (1ULL << 40)
#define CIASerClk3     (1ULL << 41)

#define CIADelayMask ~((1ULL << 42) \
| CIACountA0 | CIACountB0 \
| CIALoadA0 | CIALoadB0 \
| CIAPB6Low0 | CIAPB7Low0 \
| CIASetInt0 | CIAClearInt0 \
| CIAOneShotA0 | CIAOneShotB0 \
| CIAReadIcr0 | CIAClearIcr0 \
| CIAAckIcr0 | CIASetIcr0 \
| CIATODInt0 | CIASerInt0 \
| CIASerLoad0 | CIASerClk0)


// Virtual complex interface adapter (CIA)
class CIA : public AmigaComponent {

    friend TOD;
    friend Amiga;

protected:

    // Identification number (0 = CIA A, 1 = CIA B)
    int nr;

    // Current configuration
    CIAConfig config;

    // Result of the latest inspection
    CIAInfo info;


    //
    // Sub components
    //

    TOD tod = TOD(this, amiga);


    //
    // Internal state
    //

public:
    
    // The CIA has been executed up to this clock cycle.
    Cycle clock;

protected:

    // Total number of skipped cycles (used by the debugger, only).
    Cycle idleCycles;
    
    // Timer A counter
    u16 counterA;
    
    // Timer B counter
    u16 counterB;
    
protected:
    
    // Timer A latch
    u16 latchA;
    
    // Timer B latch
    u16 latchB;

    
    //
    // Adapted from PC64Win by Wolfgang Lorenz
    //
    
    //
    // Control
    //
    
    // Action flags
    u64 delay;
    
    // New bits to feed into delay
    u64 feed;
    
    // Control register A
    u8 CRA;
    
    // Control register B
    u8 CRB;
    
    // Interrupt control register
    u8 icr;
    
    // ICR bits that need to deleted when CIAAckIcr1 hits
    u8 icrAck;
    
    // Interrupt mask register
    u8 imr;
    
protected:
    
    // Bit mask for PB outputs: 0 = port register, 1 = timer
    u8 PB67TimerMode;
    
    // PB output bits 6 and 7 in timer mode
    u8 PB67TimerOut;
    
    // PB output bits 6 and 7 in toggle mode
    u8 PB67Toggle;
    
    
    //
    // Port registers
    //
    
protected:
    
    // Peripheral data register A
    u8 PRA;
    
    // Peripheral data register B
    u8 PRB;
    
    // Data directon register A (0 = input, 1 = output)
    u8 DDRA;
    
    // Data directon register B (0 = input, 1 = output)
    u8 DDRB;
    
    // Peripheral port A (pins PA0 to PA7)
    u8 PA;
    
    // Peripheral port A (pins PB0 to PB7)
    u8 PB;
    
    
    //
    // Shift register logic
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
    u8 SDR;
    
    // Clock signal for driving the serial register
    bool serClk;
    
    /* Shift register counter
     * The counter is set to 8 when the shift register is loaded and decremented
     * when a bit is shifted out.
     */
    u8 serCounter;
    
    //
    // Chip interface (port pins)
    //
    
    bool SP;
    bool CNT;
    bool INT;
    
    
    //
    // Speeding up emulation (sleep logic)
    //
    
    /* Idle counter
     * When the CIA's state does not change during execution, this variable is
     * increased by one. If it exceeds a certain threshhold, the chip is put
     * into idle state via sleep().
     */
    u8 tiredness;
    
public:
    
    // Indicates if the CIA is currently idle
    bool sleeping;
    
    /* The last executed cycle before the chip went idle.
     * The variable is set in sleep()
     */
    Cycle sleepCycle;
    
    /* The wake up cycle  executed cycle before the chip went idle.
     * The variable is set in sleep()
     */
    Cycle wakeUpCycle;


    //
    // Constructing and serializing
    //

public:
    
    CIA(int n, Amiga& ref);

    bool isCIAA() { return nr == 0; }
    bool isCIAB() { return nr == 1; }

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker & config.type;
        worker & config.todBug;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & clock
        & idleCycles
        & counterA
        & counterB
        & latchA
        & latchB
        & delay
        & feed
        & CRA
        & CRB
        & icr
        & icrAck
        & imr
        & PB67TimerMode
        & PB67TimerOut
        & PB67Toggle
        & PRA
        & PRB
        & DDRA
        & DDRB
        & PA
        & PB
        & SDR
        & serClk
        & serCounter
        & SP
        & CNT
        & INT
        & tiredness
        & sleeping
        & sleepCycle
        & wakeUpCycle;
    }

    //
    // Configuring
    //

    CIAConfig getConfig() { return config; }

    bool getTodBug() { return config.todBug; }
    void setTodBug(bool value) { config.todBug = value; }

    
    //
    // Methods from HardwareComponent
    //

protected:

    void _powerOn() override;
    void _run() override;
    void _reset(bool hard) override;
    void _inspect() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

public:

    //
    // Configuring
    //

    // Returns the result of the most recent call to inspect()
    CIAInfo getInfo() { return HardwareComponent::getInfo(info); }

    
    //
    // Accessing properties
    //
    
    // Getter for peripheral port A
    u8 getPA() { return PA; }
    u8 getDDRA() { return DDRA; }
    
    // Getter for peripheral port B
    u8 getPB() { return PB; }
    u8 getDDRB() { return DDRB; }

    // Getter for the interrupt line
    bool irqPin() { return INT; }

    // Simulates a rising edge on the flag pin
    void emulateRisingEdgeOnFlagPin();
    
    // Simulates a falling edge on the flag pin
    void emulateFallingEdgeOnFlagPin();

    // Simulates a rising edge on the CNT pin
    void emulateRisingEdgeOnCntPin();

    // Simulates a falling edge on the CNT pin
    void emulateFallingEdgeOnCntPin();

    // Set the SP pin (serial port pin)
    void setSP(bool value) { SP = value; }
    
private:
    
    //
    // Interrupt control
    //
    
    /* Requests the CPU to interrupt.
     * This function is abstract and implemented differently by CIA1 and CIA2.
     * CIA 1 activates the IRQ line and CIA 2 the NMI line.
     */
    virtual void pullDownInterruptLine() = 0;
    
    /* Removes the interrupt requests.
     * This function is abstract and implemented differently by CIA1 and CIA2.
     * CIA 1 clears the IRQ line and CIA 2 the NMI line.
     */
    virtual void releaseInterruptLine() = 0;
    
    /* Load latched value into timer.
     * As a side effect, CountA2 is cleared. This causes the timer to wait
     * for one cycle before it continues to count.
     */
    void reloadTimerA() { counterA = latchA; delay &= ~CIACountA2; }
    
    /* Loads latched value into timer.
     * As a side effect, CountB2 is cleared. This causes the timer to wait for
     * one cycle before it continues to count.
     */
    void reloadTimerB() { counterB = latchB; delay &= ~CIACountB2; }
    
    // Triggers a timer interrupt
    void triggerTimerIrq();
    
    // Triggers a TOD interrupt
    void triggerTodIrq();

    // Triggers a flag pin interrupt
    void triggerFlagPinIrq();

    // Triggers a serial interrupt
    void triggerSerialIrq();
    
private:
    
    //
    // Port registers
    //
    
    // Values driving port A from inside the chip
    virtual u8 portAinternal() = 0;
    
    // Values driving port A from outside the chip
    virtual u8 portAexternal() = 0;
    
public:
    
    // Computes the values which we currently see at port A
    virtual void updatePA() = 0;
    
private:
    
    // Values driving port B from inside the chip
    virtual u8 portBinternal() = 0;
    
    // Values driving port B from outside the chip
    virtual u8 portBexternal() = 0;
    
    // Computes the values which we currently see at port B
    virtual void updatePB() = 0;
    
protected:
    
    // Action method for poking the PA register
    virtual void pokePA(u8 value) { PRA = value; updatePA(); }
    
    // Action method for poking the DDRA register
    virtual void pokeDDRA(u8 value) { DDRA = value; updatePA(); }
    
    
    //
    // Accessing the I/O address space
    //
    
public:
    
    // Peeks a value from a CIA register.
    u8 peek(u16 addr);
    
    // Peeks a value from a CIA register without causing side effects.
    u8 spypeek(u16 addr);
    
    // Pokes a value into a CIA register.
    void poke(u16 addr, u8 value);
    
    
    //
    // Running the device
    //
    
public:
    
    // Advances the 24-bit counter by one tick.
    void incrementTOD();
    
    // Executes the CIA for one CIA cycle.
    void executeOneCycle();
    
    // Schedules the next execution event
    void scheduleNextExecution();
    
    // Schedules the next wakeup event
    void scheduleWakeUp();
    
private:
    
    //
    // Handling interrupt requests
    //
    
    // Handles an interrupt request from TOD
    void todInterrupt();
    
    
    //
    // Speeding up emulation
    //
    
private:
    
    // Puts the CIA into idle state.
    void sleep();
    
public:
    
    // Emulates all previously skipped cycles.
    void wakeUp();
    void wakeUp(Cycle targetCycle);
    
    // Returns true if the CIA is in idle state.
    bool isSleeping() { return sleeping; }
    
    // Returns true if the CIA is awake.
    bool isAwake() { return !sleeping; }
    
    // Returns true if the CIA has been executed up to the master clock.
    // bool isUpToDate();
    
    // The CIA is idle since this number of cycles.
    CIACycle idle();
    
    // Total number of cycles the CIA was idle.
    CIACycle idleTotal() { return idleCycles; }
};


/* The Amiga's first virtual Complex Interface Adapter (CIA A)
 */
class CIAA : public CIA {
    
public:
    
    CIAA(Amiga& ref);
    void _powerOn() override;
    void _powerOff() override;
    void _dump() override;
    
private:

    void pullDownInterruptLine() override;
    void releaseInterruptLine() override;
    
    u8 portAinternal() override;
    u8 portAexternal() override;
    void updatePA() override;
    u8 portBinternal() override;
    u8 portBexternal() override;
    void updatePB() override;
    
public:

    // Indicates if the power LED is currently on or off
    bool powerLED() { return (PA & 0x2) == 0; }

    // Emulates the receiption of a keycode from the keyboard
    void setKeyCode(u8 keyCode);
};

/* The Amiga's first virtual Complex Interface Adapter (CIA B)
 */
class CIAB : public CIA {
    
public:
    
    CIAB(Amiga& ref);
    void _dump() override;
    
private:
        
    void pullDownInterruptLine() override;
    void releaseInterruptLine() override;
    
    u8 portAinternal() override;
    u8 portAexternal() override;
    void updatePA() override;
    u8 portBinternal() override;
    u8 portBexternal() override;
    void updatePB() override;
};

#endif
