// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CIA.h"
#include "Amiga.h"
#include "Agnus.h"
#include "ControlPort.h"
#include "DiskController.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MsgQueue.h"
#include "Paula.h"
#include "SerialPort.h"

namespace vamiga {

CIA::CIA(int n, Amiga& ref) : SubComponent(ref), nr(n)
{    
    subComponents = std::vector<CoreComponent *> { &tod };
}

void
CIA::_initialize()
{
    CoreComponent::_initialize();

    pa = 0xFF;
    pb = 0xFF;
}

void
CIA::_reset(bool hard)
{
    if (!hard) wakeUp();

    RESET_SNAPSHOT_ITEMS(hard)
    
    cnt = true;
    irq = 1;
    
    counterA = 0xFFFF;
    counterB = 0xFFFF;
    latchA = 0xFFFF;
    latchB = 0xFFFF;
    
    // UAE initializes CRB with 4 (which I think is wrong)
    if constexpr (MIMIC_UAE) crb = 0x4;

    updatePA();
    updatePB();
    
    // Update the memory layout because the OVL bit may have changed
    mem.updateMemSrcTables();
}

void
CIA::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.defaults;

    std::vector <Option> options = {
        
        OPT_CIA_REVISION,
        OPT_TODBUG,
        OPT_ECLOCK_SYNCING
    };
    
    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }
}

i64
CIA::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_CIA_REVISION:   return config.revision;
        case OPT_TODBUG:         return config.todBug;
        case OPT_ECLOCK_SYNCING: return config.eClockSyncing;

        default:
            fatalError;
    }
}

void
CIA::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_CIA_REVISION:
            
            if (!CIARevisionEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, CIARevisionEnum::keyList());
            }
            
            config.revision = (CIARevision)value;
            return;

        case OPT_TODBUG:

            config.todBug = value;
            return;
            
        case OPT_ECLOCK_SYNCING:
            
            config.eClockSyncing = value;
            return;
            
        default:
            fatalError;
    }
}

void
CIA::_inspect() const
{
    {   SYNCHRONIZED
        
        info.portA.port = computePA();
        info.portA.reg = pra;
        info.portA.dir = ddra;
        
        info.portB.port = computePB();
        info.portB.reg = prb;
        info.portB.dir = ddrb;
        
        info.timerA.count = LO_HI(spypeek(0x04), spypeek(0x05));
        info.timerA.latch = latchA;
        info.timerA.running = (delay & CIACountA3);
        info.timerA.toggle = cra & 0x04;
        info.timerA.pbout = cra & 0x02;
        info.timerA.oneShot = cra & 0x08;
        
        info.timerB.count = LO_HI(spypeek(0x06), spypeek(0x07));
        info.timerB.latch = latchB;
        info.timerB.running = (delay & CIACountB3);
        info.timerB.toggle = crb & 0x04;
        info.timerB.pbout = crb & 0x02;
        info.timerB.oneShot = crb & 0x08;
        
        info.sdr = sdr;
        info.ssr = ssr;
        info.icr = icr;
        info.imr = imr;
        info.irq = irq;
        
        info.tod = tod.info;
        info.todIrqEnable = imr & 0x04;
        
        info.idleSince = idleSince();
        info.idleTotal = idleTotal();
        info.idlePercentage = clock ? (double)idleCycles / (double)clock : 100.0;
    }
}

void
CIA::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        os << tab("Revision");
        os << CIARevisionEnum::key(config.revision) << std::endl;
        os << tab("Emulate TOD bug");
        os << bol(config.todBug) << std::endl;
        os << tab("Sync with E-clock");
        os << bol(config.eClockSyncing) << std::endl;
    }

    if (category == Category::Registers) {
        
        os << tab("Counter A") << hex(counterA) << std::endl;
        os << tab("Latch A") << hex(latchA) << std::endl;
        os << tab("Data register A") << hex(pra) << std::endl;
        os << tab("Data port direction A") << hex(ddra) << std::endl;
        os << tab("Data port A") << hex(pa) << std::endl;
        os << tab("Control register A") << hex(cra) << std::endl;
        os << std::endl;
        os << tab("Counter B") << hex(counterB) << std::endl;
        os << tab("Latch B") << hex(latchB) << std::endl;
        os << tab("Data register B") << hex(prb) << std::endl;
        os << tab("Data port direction B") << hex(ddrb) << std::endl;
        os << tab("Data port B") << hex(pb) << std::endl;
        os << tab("Control register B") << hex(crb) << std::endl;
        os << std::endl;
        os << tab("Interrupt control reg") << hex(icr) << std::endl;
        os << tab("Interrupt mask reg") << hex(imr) << std::endl;
        os << std::endl;
        os << tab("SDR") << hex(sdr) << std::endl;
        os << tab("SSR") << hex(ssr) << std::endl;
        os << tab("serCounter") << dec(serCounter) << std::endl;
    }

    if (category == Category::State) {

        os << tab("Clock") << dec(clock) << std::endl;
        os << tab("Sleeping") << bol(sleeping) << std::endl;
        os << tab("Tiredness") << (isize)tiredness << std::endl;
        os << tab("Sleep cycle") << dec(sleepCycle) << std::endl;
        os << tab("Wakeup cycle") << dec(wakeUpCycle) << std::endl;
        os << tab("CNT") << bol(cnt) << std::endl;
        os << tab("INT") << bol(irq) << std::endl;
    }

    if (category == Category::Tod) {
        
        tod.dump(Category::State, os);
    }
}

void
CIA::emulateRisingEdgeOnFlagPin()
{
    wakeUp();
}

void
CIA::emulateFallingEdgeOnFlagPin()
{
    wakeUp();

    icr |= 0x10;
    
    if (imr & 0x10) {
        triggerFlagPinIrq(&delay);
    }
}

void
CIA::emulateRisingEdgeOnCntPin()
{
    trace(CIASER_DEBUG, "emulateRisingEdgeOnCntPin\n");
    
    wakeUp();
    cnt = 1;
    
    // Timer A
    if ((cra & 0x21) == 0x21) delay |= CIACountA1;
    
    // Timer B
    if ((crb & 0x61) == 0x21) delay |= CIACountB1;
    
    // Serial register
    if (!(cra & 0x40) /* input mode */ ) {
        
        // debug("rising CNT: serCounter %d\n", serCounter);
        if (serCounter == 0) serCounter = 8;
        trace(CIASER_DEBUG, "Clocking in bit %d [%d]\n", sp, serCounter);
        
        // Shift in a bit from the SP line
        ssr = (u8)(ssr << 1) | (u8)sp;
        
        // Perform special action if a byte is complete
        if (--serCounter == 0) {
            
            // Load the data register (SDR) with the shift register (SSR)
            trace(CIASER_DEBUG, "Loading %x into sdr\n", sdr);
            delay |= CIASsrToSdr0; // sdr = ssr;
            
            // Trigger interrupt
            delay |= CIASerInt0;
            // debug(KBD_DEBUG, "Received serial byte: %02x\n", sdr);
        }
    }
}

void
CIA::emulateFallingEdgeOnCntPin()
{
    trace(CIASER_DEBUG, "emulateFallingEdgeOnCntPin\n");

    wakeUp();
    cnt = 0;
}

void
CIA::reloadTimerA(u64 *delay)
{
    counterA = latchA;
    
    // Make sure the timer waits for one cycle before it continues to count
    *delay &= ~CIACountA2;
}

void
CIA::reloadTimerB(u64 *delay)
{
    counterB = latchB;
    
    // Make sure the timer waits for one cycle before it continues to count
    *delay &= ~CIACountB2;
}

void
CIA::triggerTimerIrq(u64 *delay)
{
    trace(CIA_DEBUG, "triggerTimerIrq()\n");
    *delay |= (*delay & CIAReadIcr0) ? CIASetInt0 : CIASetInt1;
    *delay |= (*delay & CIAReadIcr0) ? CIASetIcr0 : CIASetIcr1;
}

void
CIA::triggerTodIrq(u64 *delay)
{
    trace(CIA_DEBUG, "triggerTodIrq()\n");
    *delay |= CIASetInt0;
    *delay |= CIASetIcr0;
}

void
CIA::triggerFlagPinIrq(u64 *delay)
{
    trace(CIA_DEBUG, "triggerFlagPinIrq()\n");
    *delay |= CIASetInt0;
    *delay |= CIASetIcr0;
}

void
CIA::triggerSerialIrq(u64 *delay)
{
    trace(CIA_DEBUG, "triggerSerialIrq()\n");
    *delay |= CIASetInt0;
    *delay |= CIASetIcr0;
}

void
CIA::todInterrupt()
{
    wakeUp();
    delay |= CIATODInt0;
}

void
CIA::executeOneCycle()
{
    clock += CIA_CYCLES(1);

    // Make a local copy for speed
    u64 delay = this->delay;
    
    u64 oldDelay = delay;
    u64 oldFeed  = feed;
    
    //
    // Layout of timer (A and B)
    //

    // Source: "A Software Model of the CIA6526" by Wolfgang Lorenz
    //
    //                           Phi2            Phi2                  Phi2
    //                            |               |                     |
    // timerA      -----    ------v------   ------v------     ----------v---------
    // input  ---->| & |--->| dwDelay & |-X-| dwDelay & |---->| decrement counter|
    //         --->|   |    |  CountA2  | | |  CountA3  |     |        (1)       |
    //         |   -----    ------------- | -------------     |                  |
    // -----------------          ^ Clr   |                   |                  |
    // | bCRA & 0x01   | Clr (3)  |       | ------------------| new counter = 0? |
    // | timer A start |<----     |       | |                 |                  |
    // -----------------    |     |       v v                 |                  |
    //                    -----   |      -----                |      timer A     |
    //                    | & |   |      | & |                |  16 bit counter  |
    //                    |   |   |      |   |                |     and latch    |
    //                    -----   |      -----                |                  |
    //                     ^ ^    |        |(2)               |                  |
    //                     | |    ---------|-------------     |                  |
    //                     | |             |            |     |                  |
    // timer A             | |             |    -----   |     |                  |
    // output  <-----------|-X-------------X--->|>=1|---X---->| load from latch  |
    //                     |                --->|   |         |        (4)       |
    //                    -----             |   -----         --------------------
    //                    |>=1|             |
    //                    |   |             |       Phi2
    //                    -----             |        |
    //                     ^ ^              |  ------v------    ----------------
    //                     | |              ---| dwDelay & |<---| bcRA & 0x10  |
    //                     | ----------------  |  LoadA1   |    | force load   |
    //                     |       Phi2     |  -------------    ----------------
    //                     |        |       |                            ^ Clr
    // -----------------   |  ------v------ |                            |
    // | bCRA & 0x08   |   |  | dwDelay & | |                           Phi2
    // | one shot      |---X->| oneShotA0 |--
    // -----------------      -------------

    //
    // Timer A
    //
    
    // (1) : Decrement counter
    if (delay & CIACountA3) counterA--;

    // (2) : Check underflow condition
    bool timerAOutput = (counterA == 0 && (delay & CIACountA2));

    if (timerAOutput) {

        icrAck &= ~0x01;
        
        // (3) Stop timer in one shot mode
        if ((delay | feed) & CIAOneShotA0) {
            
            cra &= ~0x01;
            delay &= ~(CIACountA2 | CIACountA1 | CIACountA0);
            feed &= ~CIACountA0;
        }

        // Timer A output to timer B in cascade mode
        if ((crb & 0x61) == 0x41 || ((crb & 0x61) == 0x61 && cnt)) {
            
            delay |= CIACountB1;
        }
        
        // Reload counter immediately
        delay |= CIALoadA1;
    }
    
    // (4) : Load counter
    if (delay & CIALoadA1) reloadTimerA(&delay);

    //
    // Timer B
    //
    
    // (1) : Decrement counter
    if (delay & CIACountB3) counterB--;

    // (2) : Check underflow condition
    bool timerBOutput = (counterB == 0 && (delay & CIACountB2));

    if (timerBOutput) {

        icrAck &= ~0x02;
        
        // (3) : Stop timer in one shot mode
        if ((delay | feed) & CIAOneShotB0) {
            
            crb &= ~0x01;
            delay &= ~(CIACountB2 | CIACountB1 | CIACountB0);
            feed &= ~CIACountB0;
        }
        delay |= CIALoadB1;
    }

    // (4) : Load counter
    if (delay & CIALoadB1) reloadTimerB(&delay);

    //
    // Serial register
    //
    
    if (delay & CIASsrToSdr3) {
        sdr = ssr;
    }
    
    // Generate clock signal
    if (timerAOutput && (cra & 0x40)) {
        
        // output mode
        if (serCounter) {
            
            // Toggle serial clock signal
            feed ^= CIASerClk0;
            
        } else if (delay & CIASdrToSsr1) {
            
            // Load the shift register (SSR) with the data register (SDR)
            ssr = sdr;
            delay &= ~(CIASdrToSsr1 | CIASdrToSsr0);
            feed &= ~CIASdrToSsr0;
            serCounter = 8;
            
            // Toggle serial clock signal
            feed ^= CIASerClk0;
        }
    }
    
    // Run shift register with generated clock signal
    if (serCounter && (cra & 0x40)) {
        
        // Output mode
        if ((delay & (CIASerClk2 | CIASerClk1)) == CIASerClk1) {
            
            // Positive edge
            if (serCounter == 1) delay |= CIASerInt0;
        }
        else if ((delay & (CIASerClk2 | CIASerClk1)) == CIASerClk2) {
            
            // Negative edge
            serCounter--;
        }
    }

    //
    // Timer output to PB6 (timer A) and PB7 (timer B)
    //

    // Source: "A Software Model of the CIA6526" by Wolfgang Lorenz
    //
    //                     (7)            -----------------
    //         -------------------------->| bCRA & 0x04   |
    //         |                          | timer mode    |  ----------------
    //         |                          | 0x00: pulse   |->| 0x02 (timer) |
    // timerA  | Flip ---------------     |       (7)     |  |              |
    // output -X----->| bPB67Toggle |---->| 0x04: toggle  |  | bCRA & 0x02  |
    //            (5) |  ^ 0x40     |     |       (8)     |  | output mode  |-> PB6 out
    //                ---------------     -----------------  |     (6)      |
    //                       ^ Set        -----------------  | 0x00 (port)  |
    //                       |            | port B bit 6  |->|              |
    // ----------------- 0->1|            |    output     |  ----------------
    // | bCRA & 0x01   |------            -----------------
    // | timer A start |
    // -----------------

    //
    // Timer A output to PB6
    //
    
    if (timerAOutput) {

        // (5) : Toggle underflow counter bit
        pb67Toggle ^= 0x40;

        // (6)
        if (cra & 0x02) {

            if ((cra & 0x04) == 0) {
                
                // (7) : Set PB6 high for one clock cycle
                pb67TimerOut |= 0x40;
                delay |= CIAPB6Low0;
                delay &= ~CIAPB6Low1;
                
            } else {
                
                // (8) : Toggle PB6
                pb67TimerOut ^= 0x40;
            }
        }
    }

    //
    // Timer B output to PB7
    //
    
    if (timerBOutput) {

        // (5) : Toggle underflow counter bit
        pb67Toggle ^= 0x80;

        // (6)
        if (crb & 0x02) {

            if ((crb & 0x04) == 0) {
                
                // (7) : Set PB7 high for one clock cycle
                pb67TimerOut |= 0x80;
                delay |= CIAPB7Low0;
                delay &= ~CIAPB7Low1;
                
            } else {
                
                // (8) : Toggle PB7
                pb67TimerOut ^= 0x80;
            }
        }
    }

    // Set PB67 back to low
    if (delay & CIAPB6Low1) { pb67TimerOut &= ~0x40; }
    if (delay & CIAPB7Low1) { pb67TimerOut &= ~0x80; }


    //
    // Interrupt logic
    //

    // Source: "A Software Model of the CIA6526" by Wolfgang Lorenz
    //
    //                      ----------
    //                      | bIMR & |----
    //                      |  0x01  |   |    -----
    //                      ----------   ---->| & |----
    // timerA       (9) Set ----------   ---->|   |   |
    // output  ------------>| bICR & |   |    -----   |
    //           ---------->|  0x01  |----            |  -----
    //           |      Clr ----------                -->|>=1|---
    //           |          ----------                -->|   |  |
    //           |          | bIMR & |----            |  -----  |
    //           |          |  0x02  |   |    -----   |         |
    //           |          ----------   ---->| & |----         |
    // timerB    | (10) Set ----------   ---->|   |             |
    // output  --|--------->| bICR & |   |    -----             |
    //           X--------->|  0x01  |----                      |
    //           |      Clr ----------       	                |
    // read      |                                              |
    // ICR ------X---------------X-------------------           |
    //                           | (12)             |           |
    //                           v Clr              v Clr       |
    //           ------      ----------      ----------------   | (11)
    // Int    <--| -1 |<-----| bICR & |<-----|   dwDelay &  |<---
    // ouptput   |    |      |  0x80  | Set  |  Interrupt1  |
    // (14)      ------      ---------- (13) -------^--------
    //                                              |
    //                                             Phi2
    
    if (timerAOutput) { icr |= 0x01; } // (9)
    if (timerBOutput) { icr |= 0x02; } // (10)
    
    // (11) : Check for timer interrupt
    if ((timerAOutput && (imr & 0x01)) || (timerBOutput && (imr & 0x02))) {
        triggerTimerIrq(&delay);
    }

    // Check for TOD interrupt
    if (delay & CIATODInt0) {
        icr |= 0x04;
        if (imr & 0x04) {
            triggerTodIrq(&delay);
        }
    }
    
    // Check for Serial interrupt
    if (delay & CIASerInt2) {
        icr |= 0x08;
        if (imr & 0x08) {
            triggerSerialIrq(&delay);
        }
    }
    
    if (delay & (CIAClearIcr1 | CIAAckIcr1 | CIASetIcr1 | CIASetInt1 | CIAClearInt0)) {
        
        if (delay & CIAClearIcr1) { // (12)
            icr &= 0x7F;
        }
        if (delay & CIAAckIcr1) {
            icr &= ~icrAck;
        }
        if (delay & CIASetIcr1) { // (13)
            icr |= 0x80;
        }
        if (delay & CIASetInt1) { // (14)
            irq = 0;
            pullDownInterruptLine();
        }
        if (delay & CIAClearInt0) { // (14)
            irq = 1;
            releaseInterruptLine();
        }
    }

    // Move delay flags left and feed in new bits
    delay = ((delay << 1) & CIADelayMask) | feed;
    
    // Get tired if nothing has happened in this cycle
    if (oldDelay == delay && oldFeed == feed) tiredness++; else tiredness = 0;
    
    // Write back local copy
    this->delay = delay;

    // Sleep if threshold is reached
    if (tiredness > 8 && !CIA_ON_STEROIDS) {
        sleep();
        scheduleWakeUp();
    } else {
        scheduleNextExecution();
    }
}

void
CIA::sleep()
{
    // Don't call this method on a sleeping CIA
    assert(!sleeping);
    
    // Determine maximum possible sleep cycle based on timer counts
    assert(IS_CIA_CYCLE(clock));
    Cycle sleepA = clock + CIA_CYCLES((counterA > 2) ? (counterA - 1) : 0);
    Cycle sleepB = clock + CIA_CYCLES((counterB > 2) ? (counterB - 1) : 0);
    
    // CIAs with stopped timers can sleep forever
    if (!(feed & CIACountA0)) sleepA = INT64_MAX;
    if (!(feed & CIACountB0)) sleepB = INT64_MAX;
    
    // ZZzzz
    sleepCycle = clock;
    wakeUpCycle = std::min(sleepA, sleepB);;
    sleeping = true;
    tiredness = 0;
}

void
CIA::wakeUp()
{
    if (!sleeping) return;
    sleeping = false;
    
    Cycle targetCycle = CIA_CYCLES(AS_CIA_CYCLES(agnus.clock));
    wakeUp(targetCycle);
}

void
CIA::wakeUp(Cycle targetCycle)
{
    assert(clock == sleepCycle);

    // Calculate the number of missed cycles
    Cycle missedCycles = targetCycle - sleepCycle;
    assert(missedCycles % CIA_CYCLES(1) == 0);
    
    // Make up for missed cycles
    if (missedCycles > 0) {
        
        if (feed & CIACountA0) {
            assert(counterA >= AS_CIA_CYCLES(missedCycles));
            counterA -= (u16)AS_CIA_CYCLES(missedCycles);
        }
        if (feed & CIACountB0) {
            assert(counterB >= AS_CIA_CYCLES(missedCycles));
            counterB -= (u16)AS_CIA_CYCLES(missedCycles);
        }
        
        idleCycles += missedCycles;
        clock = targetCycle;
    }
    
    // Schedule the next execution event
    scheduleNextExecution();
}

CIACycle
CIA::idleSince() const
{
    return isAwake() ? 0 : AS_CIA_CYCLES(agnus.clock - sleepCycle);
}


//
// CIA A
//

void
CIAA::_powerOn()
{
    msgQueue.put(MSG_POWER_LED_DIM);
}

void
CIAA::_powerOff()
{
    msgQueue.put(MSG_POWER_LED_OFF);
}

void 
CIAA::pullDownInterruptLine()
{
    trace(CIA_DEBUG, "Pulling down IRQ line\n");
    paula.raiseIrq(INT_PORTS);
}

void 
CIAA::releaseInterruptLine()
{
    trace(CIA_DEBUG, "Releasing IRQ line\n");
}

//              -------
//     OVL <--- | PA0 |  Overlay Rom
//    /LED <--- | PA1 |  Power LED
//   /CHNG ---> | PA2 |  Floppy drive disk change signal
//   /WPRO ---> | PA3 |  Floppy drive write protection enabled
//    /TK0 ---> | PA4 |  Floppy drive track 0 indicator
//    /RDY ---> | PA5 |  Floppy drive ready
//   /FIR0 ---> | PA6 |  Port 0 fire button
//   /FIR1 ---> | PA7 |  Port 1 fire button
//              -------

void
CIAA::updatePA()
{
    u8 oldpa = pa;
    pa = computePA();
    
    if (oldpa ^ pa) {
        
        trace(DSKREG_DEBUG,
              "/FIR1: %d /FIR0: %d /RDY: %d /TK0: %d "
              "/WPRO: %d /CHNG: %d /LED: %d OVL: %d\n",
              !!(pa & 0x80), !!(pa & 0x40), !!(pa & 0x20), !!(pa & 0x10),
              !!(pa & 0x08), !!(pa & 0x04), !!(pa & 0x02), !!(pa & 0x01));
    }
    
    // Check the LED bit
    if ((oldpa ^ pa) & 0b00000010) {
        msgQueue.put((pa & 0b00000010) ? MSG_POWER_LED_DIM : MSG_POWER_LED_ON);
    }

    // Check the OVL bit which controls the Kickstart ROM overlay
    if ((oldpa ^ pa) & 0b00000001) {
        mem.updateMemSrcTables();
    }
}

u8 CIAA::computePA() const
{
    u8 internal = portAinternal();
    u8 external = portAexternal();
    
    u8 result = (internal & ddra) | (external & ~ddra);

    // A connected device may force the output level to a specific value
    controlPort1.changePra(result);
    controlPort2.changePra(result);
    
    // PLCC CIAs always return the PRA contents for output bits
    if (config.revision == CIA_MOS_8520_PLCC)
        result = (result & ~ddra) | (pra & ddra);

    return result;
}

u8
CIAA::portAinternal() const
{
    return pra;
}

u8
CIAA::portAexternal() const
{
    u8 result;
    
    // Set drive status bits
    result = diskController.driveStatusFlags();

    // The OVL bit must be 1
    assert(result & 1);
    
    return result;
}

//                    -------
//  Centronics 0 <--> | PB0 |
//  Centronics 1 <--> | PB1 |
//  Centronics 2 <--> | PB2 |
//  Centronics 3 <--> | PB3 |
//  Centronics 4 <--> | PB4 |
//  Centronics 5 <--> | PB5 |
//  Centronics 6 <--> | PB6 |
//  Centronics 7 <--> | PB7 |
//                    -------

void
CIAA::updatePB()
{
    pb = computePB();
}

u8
CIAA::computePB() const
{
    u8 internal = portBinternal();
    u8 external = portBexternal();

    u8 result = (internal & ddrb) | (external & ~ddrb);

    // Check if timer A underflows show up on PB6
    if (GET_BIT(pb67TimerMode, 6))
        REPLACE_BIT(result, 6, pb67TimerOut & (1 << 6));
    
    // Check if timer B underflows show up on PB7
    if (GET_BIT(pb67TimerMode, 7))
        REPLACE_BIT(result, 7, pb67TimerOut & (1 << 7));

    // PLCC CIAs always return the PRB contents for output bits
    if (config.revision == CIA_MOS_8520_PLCC)
        result = (result & ~ddrb) | (prb & ddrb);

    return result;
}

u8
CIAA::portBinternal() const
{
    return prb;
}

u8
CIAA::portBexternal() const
{
    return 0xFF;
}

void
CIAA::setKeyCode(u8 keyCode)
{
    trace(KBD_DEBUG, "setKeyCode: %x\n", keyCode);
    
    // Put the key code into the serial data register
    sdr = keyCode;
    
    // Trigger a serial data interrupt
    delay |= CIASerInt0;

    // Wake up the CIA
    wakeUp();
}

//
// CIA B
// 

void 
CIAB::pullDownInterruptLine()
{
    trace(CIA_DEBUG, "Pulling down IRQ line\n");
    paula.raiseIrq(INT_EXTER);
}

void 
CIAB::releaseInterruptLine()
{
    trace(CIA_DEBUG, "Releasing IRQ line\n");
}

//                                 -------
//      Parallel port: BUSY   ---> | PA0 |
//      Parallel Port: POUT   ---> | PA1 |
//  Parallel / Serial: SEL/RI ---> | PA2 |
//        Serial port: /DSR   ---> | PA3 |
//        Serial port: /CTS   ---> | PA4 |
//        Serial port: /CD    ---> | PA5 |
//        Serial port: /RTS   <--- | PA6 |
//        Serial port: /DTR   <--- | PA7 |
//                                 -------

u8
CIAB::portAinternal() const
{
    return pra;
}

u8
CIAB::portAexternal() const
{
    u8 result = 0xFF;

    // Parallel port
    // NOT IMPLEMENTED

    // Shared between parallel and serial port
    if (serialPort.getRI()) CLR_BIT(result, 2);

    // Serial port
    if (serialPort.getDSR()) CLR_BIT(result, 3);
    if (serialPort.getCTS()) CLR_BIT(result, 4);
    if (serialPort.getCD())  CLR_BIT(result, 5);
    if (serialPort.getRTS()) CLR_BIT(result, 6);
    if (serialPort.getDTR()) CLR_BIT(result, 7);

    return result;
}

void
CIAB::updatePA()
{
    u8 oldPA = pa;
    pa = computePA();

    // Drive serial pins if they are configured as output
    if (GET_BIT(ddra, 6)) serialPort.setRTS(!GET_BIT(pra, 6));
    if (GET_BIT(ddra, 7)) serialPort.setDTR(!GET_BIT(pra, 7));
    
    /* Inside the Amiga, PA0 and PA1 of CIAB are wired to the SP pin and the
     * CNT pin, respectively. If the shift register is run in input mode,
     * a positive edge on the CNT pin will transfer the value on the SP pin
     * into the shift register. To shift in the correct value, we need to set
     * the SP pin first and emulate the edge on the CNT pin afterwards.
     */
    if (ddra & 1) { setSP(pa & 1); } else { setSP(1); }
    if (!(oldPA & 2) &&  (pa & 2)) emulateRisingEdgeOnCntPin();
    if ( (oldPA & 2) && !(pa & 2)) emulateFallingEdgeOnCntPin();
}

u8
CIAB::computePA() const
{
    u8 internal = portAinternal();
    u8 external = portAexternal();

    u8 result = (internal & ddra) | (external & ~ddra);
    
    // PLCC CIAs always return the PRA contents for output bits
    if (config.revision == CIA_MOS_8520_PLCC)
        result = (result & ~ddra) | (pra & ddra);
    
    return result;
}

//            -------
//  /STEP <-- | PB0 |   (Floppy drive step heads)
//    DIR <-- | PB1 |   (Floppy drive head direction)
//  /SIDE <-- | PB2 |   (Floppy drive side select)
//  /SEL0 <-- | PB3 |   (Floppy drive select df0)
//  /SEL1 <-- | PB4 |   (Floppy drive select df1)
//  /SEL2 <-- | PB5 |   (Floppy drive select df2)
//  /SEL3 <-- | PB6 |   (Floppy drive select df3)
//   _MTR <-- | PB7 |   (Floppy drive motor on)
//            -------

u8
CIAB::portBinternal() const
{
    u8 result = prb;
    
    // Check if timer A underflow shows up on PB6
    if (GET_BIT(pb67TimerMode, 6))
        REPLACE_BIT(result, 6, pb67TimerOut & (1 << 6));

    // Check if timer B underflows show up on PB7
    if (GET_BIT(pb67TimerMode, 7))
        REPLACE_BIT(result, 7, pb67TimerOut & (1 << 7));

    return result;
}

u8
CIAB::portBexternal() const
{
    return 0xFF;
}

void
CIAB::updatePB()
{
    u8 oldPB = pb;
    pb = computePB();

    // Notify the disk controller about the changed bits
    if (oldPB ^ pb) {
        
        trace(DSKREG_DEBUG,
              "MTR: %d SEL3: %d SEL2: %d SEL1: %d "
              "SEL0: %d SIDE: %d DIR: %d STEP: %d\n",
              !!(pb & 0x80), !!(pb & 0x40), !!(pb & 0x20), !!(pb & 0x10),
              !!(pb & 0x08), !!(pb & 0x04), !!(pb & 0x02), !!(pb & 0x01));

        diskController.PRBdidChange(oldPB, pb);
    }
}

u8
CIAB::computePB() const
{
    u8 internal = portBinternal();
    u8 external = portBexternal();

    u8 result = (internal & ddrb) | (external & ~ddrb);
    
    // PLCC CIAs always return the PRB contents for output bits
    if (config.revision == CIA_MOS_8520_PLCC)
        result = (result & ~ddrb) | (prb & ddrb);
    
    return result;
}

}
