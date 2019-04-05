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

class VIC;
class IEC;
class Keyboard;
class Joystick;

// Adapted from PC64WIN
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
#define CIATODInt0     (1ULL << 32) // Triggers an interrupt with TOD as source
#define CIASerInt0     (1ULL << 33) // Triggers an interrupt with serial register as source
#define CIASerInt1     (1ULL << 34)
#define CIASerInt2     (1ULL << 35)
#define CIASerLoad0    (1ULL << 36) // Loads the serial shift register
#define CIASerLoad1    (1ULL << 37)
#define CIASerClk0     (1ULL << 38) // Clock signal driving the serial register
#define CIASerClk1     (1ULL << 39)
#define CIASerClk2     (1ULL << 40)
#define CIASerClk3     (1ULL << 41)

#define DelayMask ~((1ULL << 42) | CIACountA0 | CIACountB0 | CIALoadA0 | CIALoadB0 | CIAPB6Low0 | CIAPB7Low0 | CIASetInt0 | CIAClearInt0 | CIAOneShotA0 | CIAOneShotB0 | CIAReadIcr0 | CIAClearIcr0 | CIAAckIcr0 | CIASetIcr0 | CIATODInt0 | CIASerInt0 | CIASerLoad0 | CIASerClk0)


// Virtual complex interface adapter (CIA)
class CIA : public HardwareComponent {
    
    friend TOD;
    friend Amiga;
    
    private:
    
    // Information shown in the GUI inspector panel
    CIAInfo info;
    
    public:
    
    // The CIA has been executed up to this clock cycle.
    Cycle clock;

    protected:
    
    // Identification (0 = CIA A, 1 = CIA B)
    int nr;
    
    // Total number of skipped cycles (used by the debugger, only).
    Cycle idleCycles;
    
    // Timer A counter
    uint16_t counterA;
    
    // Timer B counter
    uint16_t counterB;
    
    protected:
    
    // Timer A latch
    uint16_t latchA;
    
    // Timer B latch
    uint16_t latchB;
    
    // 24-bit counter
    TOD tod = TOD(this);
    
    
    //
    // Adapted from PC64Win by Wolfgang Lorenz
    //
    
    //
    // Control
    //
    
    // Performs delay by shifting left at each clock
    uint64_t delay;
    
    // New bits to feed into dwDelay
    uint64_t feed;
    
    // Control register A
    uint8_t CRA;
    
    // Control register B
    uint8_t CRB;
    
    // Interrupt control register
    uint8_t icr;
    
    // ICR bits that need to deleted when CIAAckIcr1 hits
    uint8_t icrAck;
    
    // Interrupt mask register
    uint8_t imr;
    
    protected:
    
    // Bit mask for PB outputs: 0 = port register, 1 = timer
    uint8_t PB67TimerMode;
    
    // PB outputs bits 6 and 7 in timer mode
    uint8_t PB67TimerOut;
    
    // PB outputs bits 6 and 7 in toggle mode
    uint8_t PB67Toggle;
    
    
    //
    // Port registers
    //
    
    protected:
    
    // Peripheral data register A
    uint8_t PRA;
    
    // Peripheral data register B
    uint8_t PRB;
    
    // Data directon register A (0 = input, 1 = output)
    uint8_t DDRA;
    
    // Data directon register B (0 = input, 1 = output)
    uint8_t DDRB;
    
    // Peripheral port A (pins PA0 to PA7)
    uint8_t PA;
    
    // Peripheral port A (pins PB0 to PB7)
    uint8_t PB;
    
    
    //
    // Shift register logic
    //
    
    protected:
    
    /* Serial data register
     * http://unusedino.de/ec64/technical/misc/cia6526/serial.html
     * "The serial port is a buffered, 8-bit synchronous shift register system.
     *  A control bit selects input or output mode. In input mode, data on the SP pin
     *  is shifted into the shift register on the rising edge of the signal applied
     *  to the CNT pin. After 8 CNT pulses, the data in the shift register is dumped
     *  into the Serial Data Register and an interrupt is generated. In the output
     *  mode, TIMER A is used for the baud rate generator. Data is shifted out on the
     *  SP pin at 1/2 the underflow rate of TIMER A. [...] Transmission will start
     *  following a write to the Serial Data Register (provided TIMER A is running
     *  and in continuous mode). The clock signal derived from TIMER A appears as an
     *  output on the CNT pin. The data in the Serial Data Register will be loaded
     *  into the shift register then shift out to the SP pin when a CNT pulse occurs.
     *  Data shifted out becomes valid on the falling edge of CNT and remains valid
     *  until the next falling edge. After 8 CNT pulses, an interrupt is generated to
     *  indicate more data can be sent. If the Serial Data Register was loaded with
     *  new information prior to this interrupt, the new data will automatically be
     *  loaded into the shift register and transmission will continue. If the
     *  microprocessor stays one byte ahead of the shift register, transmission will
     *  be continuous. If no further data is to be transmitted, after the 8th CNT
     *  pulse, CNT will return high and SP will remain at the level of the last data
     *  bit transmitted. SDR data is shifted out MSB first and serial input data
     *  should also appear in this format.
     */
    uint8_t SDR;
    
    // Clock signal for driving the serial register
    bool serClk;
    
    /* Shift register counter
     * The counter is set to 8 when the shift register is loaded and decremented
     * when a bit is shifted out.
     */
    uint8_t serCounter;
    
    //
    // Chip interface (port pins)
    //
    
    // Serial clock or input timer clock or timer gate
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
    uint8_t tiredness;
    
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
    
    public:
    
    CIA();
    ~CIA();
    
    //
    // Methods from HardwareComponent
    //
    
    void _powerOn() override;
    void _powerOff() override;
    void _inspect() override;
    void _dump() override;
    
    // Dump trace line
    void dumpTrace();
    
    
    //
    // Reading the internal state
    //
    
    public:
    
    // Returns the latest internal state recorded by inspect()
    CIAInfo getInfo();
    
    
    //
    // Accessing device properties
    //
    
    // Getter for peripheral port A
    uint8_t getPA() { return PA; }
    uint8_t getDDRA() { return DDRA; }
    
    // Getter for peripheral port B
    uint8_t getPB() { return PB; }
    uint8_t getDDRB() { return DDRB; }
    
    // Simulates a rising edge on the flag pin
    void triggerRisingEdgeOnFlagPin();
    
    // Simulates a falling edge on the flag pin
    void triggerFallingEdgeOnFlagPin();
    
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
    
    /* Triggers a timer interrupt
     * Invoked inside executeOneCycle() if IRQ conditions are met.
     */
    void triggerTimerIrq();
    
    /* Triggers a TOD interrupt
     * Invoked inside executeOneCycle() if IRQ conditions are met.
     */
    void triggerTodIrq();
    
    /* Triggers a serial interrupt
     * Invoked inside executeOneCycle() if IRQ conditions are met.
     */
    void triggerSerialIrq();
    
    private:
    
    //
    // Port registers
    //
    
    // Values driving port A from inside the chip
    virtual uint8_t portAinternal() = 0;
    
    // Values driving port A from outside the chip
    virtual uint8_t portAexternal() = 0;
    
    public:
    
    // Computes the values which we currently see at port A
    virtual void updatePA() = 0;
    
    private:
    
    // Values driving port B from inside the chip
    virtual uint8_t portBinternal() = 0;
    
    // Values driving port B from outside the chip
    virtual uint8_t portBexternal() = 0;
    
    // Computes the values which we currently see at port B
    virtual void updatePB() = 0;
    
    protected:
    
    // Action method for poking the PA register
    virtual void pokePA(uint8_t value) { PRA = value; updatePA(); }
    
    // Action method for poking the DDRA register
    virtual void pokeDDRA(uint8_t value) { DDRA = value; updatePA(); }
    
    
    //
    // Accessing the I/O address space
    //
    
    public:
    
    // Peeks a value from a CIA register.
    uint8_t peek(uint16_t addr);
    
    // Peeks a value from a CIA register without causing side effects.
    uint8_t spypeek(uint16_t addr);
    
    // Pokes a value into a CIA register.
    void poke(uint16_t addr, uint8_t value);
    
    
    //
    // Running the device
    //
    
    public:
    
    // Advances the 24-bit counter by one tick.
    void incrementTOD();
    
    // Executes the CIA for one CIA cycle.
    void executeOneCycle();
    
    // Schedules the next execution event
    virtual void scheduleNextExecution() = 0;
    
    // Schedules the next wakeup event
    virtual void scheduleWakeUp() = 0;
    
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
    bool isUpToDate();
    
    // The CIA is idle since this number of cycles.
    CIACycle idle();
    
    // Total number of cycles the CIA was idle.
    CIACycle idleTotal() { return idleCycles; }
};


/* The Amiga's first virtual Complex Interface Adapter (CIA A)
 */
class CIAA : public CIA {
    
    public:
    
    CIAA();
    void _dump() override;
    
    private:
    
    void scheduleNextExecution() override;
    void scheduleWakeUp() override;
    
    void pullDownInterruptLine() override;
    void releaseInterruptLine() override;
    
    uint8_t portAinternal() override;
    uint8_t portAexternal() override;
    void updatePA() override;
    uint8_t portBinternal() override;
    uint8_t portBexternal() override;
    void updatePB() override;
    
    public:
    
    // Emulates the receiption of a keycode from the keyboard
    void setKeyCode(uint8_t keyCode);
};

/* The Amiga's first virtual Complex Interface Adapter (CIA B)
 */
class CIAB : public CIA {
    
    public:
    
    CIAB();
    void _reset() override;
    void _dump() override;
    
    private:
    
    void scheduleNextExecution() override;
    void scheduleWakeUp() override;
    
    void pullDownInterruptLine() override;
    void releaseInterruptLine() override;
    
    uint8_t portAinternal() override;
    uint8_t portAexternal() override;
    void updatePA() override;
    uint8_t portBinternal() override;
    uint8_t portBexternal() override;
    void updatePB() override;
};

#endif
