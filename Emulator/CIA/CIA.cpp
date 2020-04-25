// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

CIA::CIA(int n, Amiga& ref) : nr(n), AmigaComponent(ref)
{
	setDescription("CIA");

    subComponents = vector<HardwareComponent *> { &tod };

    config.type = CIA_8520_DIP;
    config.todBug = true;
    
    PA = 0xFF;
    PB = 0xFF;
}

void
CIA::_powerOn()
{

}

void
CIA::_run()
{

}

void
CIA::_reset()
{
    RESET_SNAPSHOT_ITEMS

    CNT = true;
    INT = 1;

    counterA = 0xFFFF;
    counterB = 0xFFFF;
    latchA = 0xFFFF;
    latchB = 0xFFFF;

    CRA = 0x4; // seen in SAE
    CRB = 0x4; // seen in SAE

    updatePA();
    updatePB();

    // The OVL bit influences the memory layout. Hence, we need to update it.
    mem.updateMemSrcTable();
}

void
CIA::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);

    updatePA();
    info.portA.port = PA;
    info.portA.reg = PRA;
    info.portA.dir = DDRA;

    updatePB();
    info.portB.port = PB;
    info.portB.reg = PRB;
    info.portB.dir = DDRB;
    
    info.timerA.count = LO_HI(spypeek(0x04), spypeek(0x05));
    info.timerA.latch = latchA;
    info.timerA.running = (delay & CIACountA3);
    info.timerA.toggle = CRA & 0x04;
    info.timerA.pbout = CRA & 0x02;
    info.timerA.oneShot = CRA & 0x08;
    
    info.timerB.count = LO_HI(spypeek(0x06), spypeek(0x07));
    info.timerB.latch = latchB;
    info.timerB.running = (delay & CIACountB3);
    info.timerB.toggle = CRB & 0x04;
    info.timerB.pbout = CRB & 0x02;
    info.timerB.oneShot = CRB & 0x08;
    
    info.sdr = SDR;
    
    info.icr = icr;
    info.imr = imr;
    info.intLine = INT;
    
    info.cnt = tod.info;
    info.cntIntEnable = imr & 0x04;
    
    info.idleCycles = idle();
    info.idlePercentage = clock ? (double)idleCycles / (double)clock : 100.0;
    
    pthread_mutex_unlock(&lock);
}

void
CIA::emulateRisingEdgeOnFlagPin()
{
}

void
CIA::emulateFallingEdgeOnFlagPin()
{
    wakeUp();

    icr |= 0x10;
    
    if (imr & 0x10) {
        triggerFlagPinIrq();
    }
}

void
CIA::emulateRisingEdgeOnCntPin()
{
    wakeUp();

    // Timer A
    if ((CRA & 0x21) == 0x21) delay |= CIACountA1;

    // Timer B
    if ((CRB & 0x61) == 0x21) delay |= CIACountB1;
}

void
CIA::emulateFallingEdgeOnCntPin()
{
}

void
CIA::triggerTimerIrq()
{
    debug(CIA_DEBUG, "triggerTimerIrq()\n");
    delay |= (delay & CIAReadIcr0) ? CIASetInt0 : CIASetInt1;
    delay |= (delay & CIAReadIcr0) ? CIASetIcr0 : CIASetIcr1;
}

void
CIA::triggerTodIrq()
{
    debug(CIA_DEBUG, "triggerTodIrq()\n");
    delay |= CIASetInt0;
    delay |= CIASetIcr0;
}

void
CIA::triggerFlagPinIrq()
{
    debug(CIA_DEBUG, "triggerFlagPinIrq()\n");
    delay |= CIASetInt0;
    delay |= CIASetIcr0;
}

void
CIA::triggerSerialIrq()
{
    debug(CIA_DEBUG, "triggerSerialIrq()\n");
    delay |= CIASetInt0;
    delay |= CIASetIcr0;
}

u8
CIA::peek(u16 addr)
{
	u8 result;

    if (addr == 0 || addr == 1) {
        // debug(DSKREG_DEBUG, "Peek($%X) DDRA = $%X DDRB = $%X\n", addr, DDRA, DDRB);
    }
    debug(CIAREG_DEBUG, "Peek($%X)\n", addr);

    wakeUp();

    assert(addr <= 0x000F);
	switch(addr) {
            
        case 0x00: // CIA_DATA_PORT_A

            updatePA();
            //plaindebug("%s Peek %d (hex: %02X) = %d (DDRA = %X)\n",
            //           getDescription(), addr, addr, PA, DDRA);
            debug(DSKREG_DEBUG, "  PA = %X\n", PA);
            return PA;

        case 0x01: // CIA_DATA_PORT_B

            updatePB();
            // plaindebug("%s Peek %d (hex: %02X) = %d (DDRB = %X)\n",
            //            getDescription(), addr, addr, PB, DDRB);
            // debug(DSKREG_DEBUG, "  PB = %X\n", PB);
            return PB;

        case 0x02: // CIA_DATA_DIRECTION_A

			result = DDRA;
            // debug(DSKREG_DEBUG, "read DDRA = %X\n", DDRA);
			break;

        case 0x03: // CIA_DATA_DIRECTION_B

			result = DDRB;
            // debug(DSKREG_DEBUG, "read DDRB = %X\n", DDRB);
			break;
			
        case 0x04: // CIA_TIMER_A_LOW
			
            result = LO_BYTE(counterA);
			break;
			
        case 0x05: // CIA_TIMER_A_HIGH
            result = HI_BYTE(counterA);
			break;
			
        case 0x06: // CIA_TIMER_B_LOW

            result = LO_BYTE(counterB);
			break;
			
        case 0x07: // CIA_TIMER_B_HIGH

            result = HI_BYTE(counterB);
			break;
			
        case 0x08: // EVENT_0_7

			result = tod.getCounterLo();
            tod.defreeze();
			break;
		
        case 0x09: // EVENT_8_15

			result = tod.getCounterMid();
            // if (nr == 1) debug("EVENT_8_15 = %d (frozen = %d)\n", result, tod.frozen);
			break;
			
        case 0x0A: // EVENT_16_23

            if (!(CRB & 0x80)) tod.freeze();
			result = tod.getCounterHi();
			break;
			
        case 0x0B: // UNUSED

            result = 0;
			break;
			
        case 0x0C: // CIA_SERIAL_DATA_REGISTER
			
			result = SDR;
			break;
			
        case 0x0D: // CIA_INTERRUPT_CONTROL

            // Set upper bit if an IRQ is being triggered
            if ((delay & CIASetInt1) && (icr & 0x1F)) {
                icr |= 0x80;
            }
            
            // Remember result
            result = icr;
            
            // Release interrupt request
            if (INT == 0) {
                delay |= CIAClearInt0;
            }
            
            // Discard pending interrupts
            delay &= ~(CIASetInt0 | CIASetInt1);
        
            // Schedule the ICR bits to be cleared
            delay |= CIAClearIcr0; // Uppermost bit
            delay |= CIAAckIcr0;   // Other bits
            icrAck = result;

            // Remember the read access
            delay |= CIAReadIcr0;
			break;

        case 0x0E: // CIA_CONTROL_REG_A

			result = (u8)(CRA & ~0x90); // Bit 4 and 7 always read as 0
			break;
			
        case 0x0F: // CIA_CONTROL_REG_B
			
			result = (u8)(CRB & ~0x10); // Bit 4 always reads as 0
			break;
			
		default:
			result = 0;
			panic("Unknown CIA address %04X\n", addr);
			break;
	}
	
    // debug("Peek %d (hex: %02X) = %d\n", addr, addr, result);
    
	return result;
}

u8
CIA::spypeek(u16 addr)
{
    bool running;

    assert(addr <= 0x000F);
    switch(addr) {
          
        case 0x00: // CIA_DATA_PORT_A
            return PA;
            
        case 0x01: // CIA_DATA_PORT_B
            return PB;
            
        case 0x02: // CIA_DATA_DIRECTION_A
            return DDRA;
            
        case 0x03: // CIA_DATA_DIRECTION_B
            return DDRB;
            
        case 0x04: // CIA_TIMER_A_LOW
            running = delay & CIACountA3;
            return LO_BYTE(counterA - (running ? (u16)idle() : 0));
            
        case 0x05: // CIA_TIMER_A_HIGH
            running = delay & CIACountA3;
            return HI_BYTE(counterA - (running ? (u16)idle() : 0));
            
        case 0x06: // CIA_TIMER_B_LOW
            running = delay & CIACountB3;
            return LO_BYTE(counterB - (running ? (u16)idle() : 0));
            
        case 0x07: // CIA_TIMER_B_HIGH
            running = delay & CIACountB3;
            return HI_BYTE(counterB - (running ? (u16)idle() : 0));
            
        case 0x08: // CIA_EVENT_0_7
            return tod.getCounterLo();
            
        case 0x09: // CIA_EVENT_8_15
            return tod.getCounterMid();
            
        case 0x0A: // CIA_EVENT_16_23
            return tod.getCounterHi();
            
        case 0x0B: // UNUSED
            return 0;
            
        case 0x0C: // CIA_SERIAL_DATA_REGISTER
            return SDR;
            
        case 0x0D: // CIA_INTERRUPT_CONTROL
            return icr;
            
        case 0x0E: // CIA_CONTROL_REG_A
            return CRA & ~0x10;
            
        case 0x0F: // CIA_CONTROL_REG_B
            return CRB & ~0x10;
            
        default:
            assert(0);
            return 0;
    }
}

void
CIA::poke(u16 addr, u8 value)
{
    if (addr == 0 || addr == 1) {
        // debug(DSKREG_DEBUG, "Poke($%X,$%X) DDRA = $%X DDRB = $%X\n", addr, value, DDRA, DDRB);
    }
    debug(CIAREG_DEBUG, "Poke($%X,$%X) (%d,%d)\n", addr, value, addr, value);
    
    wakeUp();
    
	switch(addr) {
		
        case 0x00: // CIA_DATA_PORT_A

            // plaindebug("%s poke(0, %X)\n", getDescription(), value);
            pokePA(value);
            return;
            
        case 0x01: // CIA_DATA_PORT_B
            
            // if (this == &amiga->ciaB) plaindebug("%s poke(1, %X)\n", getDescription(), value);
            PRB = value;
            updatePB();
            return;
            
        case 0x02: // CIA_DATA_DIRECTION_A
        
            // plaindebug("%s poke(DDRA, %X)\n", getDescription(), value);
            // debug(DSKREG_DEBUG, " DDRA = %X\n", DDRA);
            pokeDDRA(value);
            // DDRA = value;
            // updatePA();
            return;
            
        case 0x03: // CIA_DATA_DIRECTION_B
        
            // plaindebug("%s poke(DDRB, %X)\n", getDescription(), value);
            // debug(DSKREG_DEBUG, " DDRB = %X\n", DDRB);
            DDRB = value;
            updatePB();
            return;
            
        case 0x04: // CIA_TIMER_A_LOW
			
            latchA = (latchA & 0xFF00) | value;
			if (delay & CIALoadA2) {
                counterA = (counterA & 0xFF00) | value;
			}
			return;
			
        case 0x05: // CIA_TIMER_A_HIGH
			
            latchA = (latchA & 0x00FF) | (value << 8);
            if (delay & CIALoadA2) {
                counterA = (counterA & 0x00FF) | (value << 8);
            }
            
			// Load counter if timer is stopped
			if (!(CRA & 0x01)) {
				delay |= CIALoadA0;
			}
            
            /* MOS 8520 only feature:
             * "In one-shot mode, a write to timer-high (register 5 for timer A,
             *  register 7 for Timer B) will transfer the timer latch to the
             *  counter and initiate counting regardless of the start bit." [HRM]
             */
            if (CRA & 0x08) {
                if (!(CRA & 0x01)) {
                    PB67Toggle |= 0x40;
                }
                if (!(CRA & 0x20)) {
                    delay |= CIACountA1 | CIALoadA0 | CIACountA0;
                    feed |= CIACountA0;
                }
                CRA |= 0x01;
            }
            
			return;
			
        case 0x06: // CIA_TIMER_B_LOW

            latchB = (latchB & 0xFF00) | value;
			if (delay & CIALoadB2) {
                counterB = (counterB & 0xFF00) | value;
			}			
			return;
			
        case 0x07: // CIA_TIMER_B_HIGH
			
            latchB = (latchB & 0x00FF) | (value << 8);
            if (delay & CIALoadB2) {
                counterB = (counterB & 0x00FF) | (value << 8);
            }
            
			// Load counter if timer is stopped
			if ((CRB & 0x01) == 0) {
				delay |= CIALoadB0;
			}
            
            /* MOS 8520 only feature:
             * "In one-shot mode, a write to timer-high (register 5 for timer A,
             *  register 7 for Timer B) will transfer the timer latch to the
             *  counter and initiate counting regardless of the start bit." [HRM]
             */
            if (CRB & 0x08) {
                if (!(CRB & 0x01)) {
                    PB67Toggle |= 0x80;
                }
                if (!(CRB & 0x60)) {
                    delay |= CIACountB1 | CIALoadB0 | CIACountB0;
                    feed |= CIACountB0;
                }
                CRB |= 0x01;
            }
            
			return;
			
        case 0x08: // CIA_EVENT_0_7
            
			if (CRB & 0x80) {
				tod.setAlarmLo(value);
			} else { 
				tod.setCounterLo(value);
                tod.cont();
			}
			return;
			
        case 0x09: // CIA_EVENT_8_15
            
            if (CRB & 0x80) {
				tod.setAlarmMid(value);
            } else {
				tod.setCounterMid(value);
            }
			return;
			
        case 0x0A: // CIA_EVENT_16_23
            
            if (CRB & 0x80) {
				tod.setAlarmHi(value);
            } else {
				tod.setCounterHi(value);
                tod.stop();
            }
			return;
			
        case 0x0B: // UNUSED
        
			return;
			
        case 0x0C: // CIA_DATA_REGISTER
            
            SDR = value;
            delay |= CIASerLoad0;
            feed |= CIASerLoad0;
            // delay &= ~SerLoad1;
			return;
			
        case 0x0D: // CIA_INTERRUPT_CONTROL
			
			// Bit 7 means set (1) or clear (0) the other bits
			if ((value & 0x80) != 0) {
				imr |= (value & 0x1F);
			} else {
				imr &= ~(value & 0x1F);
			}
            // debug("imr = %d (hex: %X) icr = %d (hex: %X) INT = %d\n", imr, imr, icr, icr, INT);
            
            // Raise an interrupt in the next cycle if conditions match
            if ((imr & icr & 0x1F) && INT && !(delay & CIAReadIcr1)) {
                delay |= (CIASetInt1 | CIASetIcr1);
            }
			return;
			
        case 0x0E: // CIA_CONTROL_REG_A
		
            // -------0 : Stop timer
            // -------1 : Start timer
            if (value & 0x01) {
                delay |= CIACountA1 | CIACountA0;
                feed |= CIACountA0;
                if (!(CRA & 0x01))
                    PB67Toggle |= 0x40; // Toggle is high on start
            } else {
                delay &= ~(CIACountA1 | CIACountA0);
                feed &= ~CIACountA0;
            }
            
            // ------0- : Don't indicate timer underflow on port B
            // ------1- : Indicate timer underflow on port B bit 6
            if (value & 0x02) {
                PB67TimerMode |= 0x40;
                if (!(value & 0x04)) {
                    if ((delay & CIAPB7Low1) == 0) {
                        PB67TimerOut &= ~0x40;
                    } else {
                        PB67TimerOut |= 0x40;
                    }
                } else {
                    PB67TimerOut = (PB67TimerOut & ~0x40) | (PB67Toggle & 0x40);
                }
            } else {
                PB67TimerMode &= ~0x40;
            }
            
            // -----0-- : Upon timer underflow, invert port B bit 6
            // -----1-- : Upon timer underflow, generate a positive edge
            //            on port B bit 6 for one cycle

            // ----0--- : Timer restarts upon underflow
            // ----1--- : Timer stops upon underflow (One shot mode)
            if (value & 0x08) {
                feed |= CIAOneShotA0;
            } else {
                feed &= ~CIAOneShotA0;
            }
            
            // ---0---- : Nothing to do
            // ---1---- : Load start value into timer
            if (value & 0x10) {
                delay |= CIALoadA0;
            }

            // --0----- : Timer counts system cycles
            // --1----- : Timer counts positive edges on CNT pin
            if (value & 0x20) {
                delay &= ~(CIACountA1 | CIACountA0);
                feed &= ~CIACountA0;
            }
    
            // -0------ : Serial shift register in input mode (read)
            // -1------ : Serial shift register in output mode (write)
            if (isCIAA() && ((CRA & 0x40) ^ (value & 0x40))) {
                keyboard.setSPLine(!(value & 0x40), clock);
            }

            if ((value ^ CRA) & 0x40)
            {
                // Serial direction changing
                delay &= ~(CIASerLoad0 | CIASerLoad1);
                feed &= ~CIASerLoad0;
                serCounter = 0;
            
                delay &= ~(CIASerClk0 | CIASerClk1 | CIASerClk2);
                feed &= ~CIASerClk0;
            }
            
            updatePB(); // Because PB67timerMode and PB6TimerOut may have changed
			CRA = value;
			
			return;
			
        case 0x0F: // CIA_CONTROL_REG_B
		{
            // -------0 : Stop timer
            // -------1 : Start timer
            if (value & 0x01) {
                delay |= CIACountB1 | CIACountB0;
                feed |= CIACountB0;
                if (!(CRB & 0x01))
                    PB67Toggle |= 0x80; // Toggle is high on start
            } else {
                delay &= ~(CIACountB1 | CIACountB0);
                feed &= ~CIACountB0;
            }
            
            // ------0- : Don't indicate timer underflow on port B
            // ------1- : Indicate timer underflow on port B bit 7
            if (value & 0x02) {
                PB67TimerMode |= 0x80;
                if ((value & 0x04) == 0) {
                    if ((delay & CIAPB7Low1) == 0) {
                        PB67TimerOut &= ~0x80;
                    } else {
                        PB67TimerOut |= 0x80;
                    }
                } else {
                    PB67TimerOut = (PB67TimerOut & ~0x80) | (PB67Toggle & 0x80);
                }
            } else {
                PB67TimerMode &= ~0x80;
            }
            
            // -----0-- : Upon timer underflow, invert port B bit 7
            // -----1-- : Upon timer underflow, generate a positive edge
            //            on port B bit 7 for one cycle
            
            // ----0--- : Timer restarts upon underflow
            // ----1--- : Timer stops upon underflow (One shot mode)
            if (value & 0x08) {
                feed |= CIAOneShotB0;
            } else {
                feed &= ~CIAOneShotB0;
            }
            
            // ---0---- : Nothing to do
            // ---1---- : Load start value into timer
            if (value & 0x10) {
                delay |= CIALoadB0;
            }
            
            // -00----- : Timer counts system cycles
            // -01----- : Timer counts positive edges on CNT pin
            // -10----- : Timer counts underflows of timer A
            // -11----- : Timer counts underflows of timer A occurring along with a
            //            positive edge on CNT pin
            if (value & 0x60) {
                delay &= ~(CIACountB1 | CIACountB0);
                feed &= ~CIACountB0;
            }
            
            // 0------- : Writing into TOD registers sets TOD
            // 1------- : Writing into TOD registers sets alarm time
            
            updatePB(); // Because PB67timerMode and PB6TimerOut may have changed
			CRB = value;
			
			return;			
		}
			
		default:
			panic("PANIC: Unknown CIA address (poke) %04X\n", addr);
	}	
}

void
CIA::incrementTOD()
{
    tod.increment();
}

void
CIA::todInterrupt()
{
    wakeUp();
    delay |= CIATODInt0;
}

void
CIA::_dump()
{
    _inspect();

    msg("                   Clock : %lld\n", clock);
    msg("                Sleeping : %s\n", sleeping ? "yes" : "no");
    msg("               Tiredness : %d\n", tiredness);
    msg(" Most recent sleep cycle : %lld\n", sleepCycle);
    msg("Most recent wakeup cycle : %lld\n", wakeUpCycle);
    msg("\n");
	msg("               Counter A : %04X\n", info.timerA.count);
    msg("                 Latch A : %04X\n", info.timerA.latch);
    msg("         Data register A : %02X\n", info.portA.reg);
    msg("   Data port direction A : %02X\n", info.portA.dir);
    msg("             Data port A : %02X\n", info.portA.port);
	msg("      Control register A : %02X\n", CRA);
	msg("\n");
	msg("               Counter B : %04X\n", info.timerB.count);
	msg("                 Latch B : %04X\n", info.timerB.latch);
    msg("         Data register B : %02X\n", info.portB.reg);
	msg("   Data port direction B : %02X\n", info.portB.dir);
    msg("             Data port B : %02X\n", info.portB.port);
	msg("      Control register B : %02X\n", CRB);
	msg("\n");
	msg("   Interrupt control reg : %02X\n", info.icr);
	msg("      Interrupt mask reg : %02X\n", info.imr);
	msg("\n");
    msg("                     SDR : %02X %02X\n", info.sdr, SDR);
    msg("                  serClk : %02X\n", serClk);
    msg("              serCounter : %02X\n", serCounter);
    msg("\n");
    msg("                     CNT : %d\n", CNT);
    msg("                     INT : %d\n", INT);
    msg("\n");

	tod.dump();
}

void
CIA::executeOneCycle()
{
    clock += CIA_CYCLES(1);
    
    // debug("Executing CIA: new clock = %lld\n", clock);
    
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

				
	// Timer A

	// Decrement counter

	if (delay & CIACountA3)
		counterA--; // (1)
	
	// Check underflow condition
	bool timerAOutput = (counterA == 0 && (delay & CIACountA2)); // (2)
	
	if (timerAOutput) {
        
        debug(CIA_DEBUG, "Timer A underflow\n");
        
        icrAck &= ~0x01;
        
		// Stop timer in one shot mode
		if ((delay | feed) & CIAOneShotA0) { // (3)
			CRA &= ~0x01;
			delay &= ~(CIACountA2 | CIACountA1 | CIACountA0);
			feed &= ~CIACountA0;
		}
		
		// Timer A output to timer B in cascade mode
		if ((CRB & 0x61) == 0x41 || ((CRB & 0x61) == 0x61 && CNT)) {
			delay |= CIACountB1;
		}
        
        // Reload counter immediately
		delay |= CIALoadA1;
	}
    
	// Load counter
	if (delay & CIALoadA1) // (4)
		reloadTimerA(); 
	
	// Timer B
	
	// Decrement counter
	if (delay & CIACountB3) {
		counterB--; // (1)
    } 

	// Check underflow condition
	bool timerBOutput = (counterB == 0 && (delay & CIACountB2)); // (2)
	
	if (timerBOutput) {
				
        // debug("Timer B underflow\n");

        icrAck &= ~0x02;
        
		// Stop timer in one shot mode
		if ((delay | feed) & CIAOneShotB0) { // (3)
			CRB &= ~0x01;
			delay &= ~(CIACountB2 | CIACountB1 | CIACountB0);
			feed &= ~CIACountB0;
		}
		delay |= CIALoadB1;
	}
	
	// Load counter
	if (delay & CIALoadB1) // (4)
		reloadTimerB();
		
    //
    // Serial register
    //
    
    // Generate clock signal
    if (timerAOutput && (CRA & 0x40) /* output mode */ ) {
        
        if (serCounter) {
            
            // Toggle serial clock signal
            feed ^= CIASerClk0;
            
        } else if (delay & CIASerLoad1) {
            
            // Load shift register
            delay &= ~(CIASerLoad1 | CIASerLoad0);
            feed &= ~CIASerLoad0;
            serCounter = 8;
            feed ^= CIASerClk0;
        }
    }
    
    // Run shift register with generated clock signal
    if (serCounter) {
        if ((delay & (CIASerClk2 | CIASerClk1)) == CIASerClk1) {      // Positive edge
            if (serCounter == 1) {
                delay |= CIASerInt0; // Trigger interrupt
            }
        }
        else if ((delay & (CIASerClk2 | CIASerClk1)) == CIASerClk2) { // Negative edge
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
	//                ---------------     -----------------  |              |
	//                       ^ Set        -----------------  | 0x00 (port)  |
	//                       |            | port B bit 6  |->|              |
	// ----------------- 0->1|            |    output     |  ----------------
	// | bCRA & 0x01   |------            -----------------
	// | timer A start |
	// -----------------

	// Timer A output to PB6
	
	if (timerAOutput) {
		
		PB67Toggle ^= 0x40; // (5) toggle underflow counter bit
		
		if (CRA & 0x02) { // (6)

			if ((CRA & 0x04) == 0) { 
				// (7) set PB6 high for one clock cycle
				PB67TimerOut |= 0x40;
				delay |= CIAPB6Low0;
				delay &= ~CIAPB6Low1;
			} else { 
				// (8) toggle PB6 (copy bit 6 from PB67Toggle)
				// PB67TimerOut = (PB67TimerOut & 0xBF) | (PB67Toggle & 0x40);
                PB67TimerOut ^= 0x40;
			}
		}
	}

	// Timer B output to PB7
	
	if (timerBOutput) {
		
		PB67Toggle ^= 0x80; // (5) toggle underflow counter bit
	
		if (CRB & 0x02) { // (6)
		
			if ((CRB & 0x04) == 0) {
				// (7) set PB7 high for one clock cycle
				PB67TimerOut |= 0x80;
				delay |= CIAPB7Low0;
				delay &= ~CIAPB7Low1;
			} else {
				// (8) toggle PB7 (copy bit 7 from PB67Toggle)
				// PB67TimerOut = (PB67TimerOut & 0x7F) | (PB67Toggle & 0x80);
                PB67TimerOut ^= 0x80;
			}
		}
	}
	
	// Set PB67 back to low
	if (delay & CIAPB6Low1)
		PB67TimerOut &= ~0x40;

	if (delay & CIAPB7Low1)
		PB67TimerOut &= ~0x80;

	
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
    
	if (timerAOutput) { // (9)
		icr |= 0x01;
	}

    if (timerBOutput) { // (10)
        icr |= 0x02;
    }
    
    // Check for timer interrupt
    if ((timerAOutput && (imr & 0x01)) || (timerBOutput && (imr & 0x02))) { // (11)
        triggerTimerIrq();
    }

    // Check for TOD interrupt
    if (delay & CIATODInt0) {
        icr |= 0x04;
        if (imr & 0x04) {
            triggerTodIrq();
        }
    }
    
    // Check for Serial interrupt
    if (delay & CIASerInt2) {
        icr |= 0x08;
        if (imr & 0x08) {
            triggerSerialIrq();
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
            INT = 0;
            pullDownInterruptLine();
        }
        if (delay & CIAClearInt0) { // (14)
            INT = 1;
            releaseInterruptLine();
        }
    }

	// Move delay flags left and feed in new bits
	delay = ((delay << 1) & CIADelayMask) | feed;
    
    // Go into idle state if possible
    if (oldDelay == delay && oldFeed == feed) tiredness++; else tiredness = 0;
  
#ifndef CIA_ON_STEROIDS
    // Sleep if threshold is reached
    if (tiredness > 8) {
        sleep();
        scheduleWakeUp();
        return;
    }
#endif
    
    scheduleNextExecution();
}

void
CIA::scheduleNextExecution()
{
    if (isCIAA()) {
        agnus.scheduleAbs<CIAA_SLOT>(clock + CIA_CYCLES(1), CIA_EXECUTE);
    } else {
        agnus.scheduleAbs<CIAB_SLOT>(clock + CIA_CYCLES(1), CIA_EXECUTE);
    }
}

void
CIA::scheduleWakeUp()
{
    if (isCIAA()) {
        agnus.scheduleAbs<CIAA_SLOT>(wakeUpCycle, CIA_WAKEUP);
    } else {
        agnus.scheduleAbs<CIAB_SLOT>(wakeUpCycle, CIA_WAKEUP);
    }
}

void
CIA::sleep()
{
    // Don't call this method on a sleeping CIA
    assert(!sleeping);
    
    // Determine maximum possible sleep cycle based on timer counts
    assert(clock % 40 == 0);
    Cycle sleepA = clock + CIA_CYCLES((counterA > 2) ? (counterA - 1) : 0);
    Cycle sleepB = clock + CIA_CYCLES((counterB > 2) ? (counterB - 1) : 0);
    
    // CIAs with stopped timers can sleep forever
    if (!(feed & CIACountA0)) sleepA = INT64_MAX;
    if (!(feed & CIACountB0)) sleepB = INT64_MAX;
    
    // ZZzzzz
    // debug("ZZzzzz: clock = %lld A = %d B = %d sleepA = %lld sleepB = %lld\n", clock, counterA, counterB, sleepA, sleepB);
    sleepCycle = clock;
    wakeUpCycle = MIN(sleepA, sleepB);
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
            counterA -= AS_CIA_CYCLES(missedCycles);
            // debug("Making up %d timer A cycles\n", AS_CIA_CYCLES(missedCycles));
        }
        if (feed & CIACountB0) {
            assert(counterB >= AS_CIA_CYCLES(missedCycles));
            counterB -= AS_CIA_CYCLES(missedCycles);
            // debug("Making up %d timer B cycles\n", AS_CIA_CYCLES(missedCycles));
        }
        
        idleCycles += missedCycles;
        clock = targetCycle;
    }
    
    // Schedule the next execution event
    scheduleNextExecution();
}

CIACycle
CIA::idle()
{
    return isAwake() ? 0 : AS_CIA_CYCLES(agnus.clock - sleepCycle);
}


// -----------------------------------------------------------------------------
// Complex Interface Adapter A
// -----------------------------------------------------------------------------

CIAA::CIAA(Amiga& ref) : CIA(0, ref)
{
    setDescription("CIAA");
}

void 
CIAA::_dump()
{
    CIA::_dump();
}

void
CIAA::_powerOn()
{
    CIA::_powerOn();
    amiga.putMessage(MSG_POWER_LED_DIM);
}

void
CIAA::_powerOff()
{
    amiga.putMessage(MSG_POWER_LED_OFF);
}

void 
CIAA::pullDownInterruptLine()
{
    debug(CIA_DEBUG, "Pulling down IRQ line\n");
    paula.raiseIrq(INT_PORTS);
    // paula->setINTREQ(0x8000 | (1 << 3));
}

void 
CIAA::releaseInterruptLine()
{
    debug(CIA_DEBUG, "Releasing IRQ line\n");
    paula.checkInterrupt();
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

u8
CIAA::portAinternal()
{
    return PRA;
}

u8
CIAA::portAexternal()
{
    u8 result;
    
    // Set drive status bits
    result = diskController.driveStatusFlags();

    // The OVL bit must be 1
    assert(result & 1);
    
    return result;
}

void
CIAA::updatePA()
{
    u8 internal = portAinternal();
    u8 external = portAexternal();

    u8 oldPA = PA;
    PA = (internal & DDRA) | (external & ~DDRA);

    // A connected device may force the output level to a specific value
    controlPort1.changePra(PA);
    controlPort2.changePra(PA);

    // PLCC CIAs always return the PRA contents for output bits
    // We ignore PLCC emulation until the A600 is supported
    // if (config.type == CIA_8520_PLCC) PA = (PA & ~DDRA) | (PRA & DDRA);

    // Check the LED bit
    if ((oldPA ^ PA) & 0b00000010) {
        amiga.putMessage((PA & 0b00000010) ? MSG_POWER_LED_DIM : MSG_POWER_LED_ON);
    }

    // Check the OVL bot (Kickstart overlay)
    if ((oldPA ^ PA) & 0b00000001) {
        mem.updateMemSrcTable();
    }
    
    /*
    if (oldPA ^ PA) {
        debug("## PA changed: /FIR1: %d /FIR0: %d /RDY: %d /TK0: %d /WPRO: %d /CHNG: %d /LED: %d OVL: %d\n",
              !!(PA & 0x80), !!(PA & 0x40), !!(PA & 0x20), !!(PA & 0x10),
              !!(PA & 0x08), !!(PA & 0x04), !!(PA & 0x02), !!(PA & 0x01));
    }
    */
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

u8
CIAA::portBinternal()
{
    return PRB;
}

u8
CIAA::portBexternal()
{
    return 0xFF;
}

void
CIAA::updatePB()
{
    u8 internal = portBinternal();
    u8 external = portBexternal();

    PB = (internal & DDRB) | (external & ~DDRB);

    // Check if timer A underflows show up on PB6
    if (GET_BIT(PB67TimerMode, 6))
        REPLACE_BIT(PB, 6, PB67TimerOut & (1 << 6));
    
    // Check if timer B underflows show up on PB7
    if (GET_BIT(PB67TimerMode, 7))
        REPLACE_BIT(PB, 7, PB67TimerOut & (1 << 7));

    // PLCC CIAs always return the PRB contents for output bits
    // We ignore PLCC emulation until the A600 is supported
    // if (config.type == CIA_8520_PLCC) PB = (PB & ~DDRB) | (PRB & DDRB);
}

void
CIAA::setKeyCode(u8 keyCode)
{
    debug(CIA_DEBUG, "setKeyCode: %X\n", keyCode);
    
    // Put the key code into the serial data register
    SDR = keyCode;
    
    // Trigger a serial data interrupt
    delay |= CIASerInt0;

    // Wake up the CIA
    wakeUp();
}

// -----------------------------------------------------------------------------
// Complex Interface Adapter B
// -----------------------------------------------------------------------------

CIAB::CIAB(Amiga& ref) : CIA(1, ref)
{
    setDescription("CIAB");
}

void 
CIAB::_dump()
{
    CIA::_dump();
}

void 
CIAB::pullDownInterruptLine()
{
    debug(CIA_DEBUG, "Pulling down IRQ line\n");
    paula.raiseIrq(INT_EXTER);
}

void 
CIAB::releaseInterruptLine()
{
    debug(CIA_DEBUG, "Releasing IRQ line\n");
    paula.checkInterrupt();
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
CIAB::portAinternal()
{
    return PRA;
}

u8
CIAB::portAexternal()
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
    // debug(CIA_DEBUG, "updatePA()\n");

    u8 internal = portAinternal();
    u8 external = portAexternal();

    u8 oldPA = PA;
    PA = (internal & DDRA) | (external & ~DDRA);

    // PLCC CIAs always return the PRA contents for output bits
    // We ignore PLCC emulation until the A600 is supported
    // if (config.type == CIA_8520_PLCC) PA = (PA & ~DDRA) | (PRA & DDRA);

    // PA1 is connected to the CNT pin
    if (!(oldPA & 2) &&  (PA & 2)) emulateRisingEdgeOnCntPin();
    if ( (oldPA & 2) && !(PA & 2)) emulateFallingEdgeOnCntPin();
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
CIAB::portBinternal()
{
    u8 result = PRB;
    
    // Check if timer A underflow shows up on PB6
    if (GET_BIT(PB67TimerMode, 6))
        REPLACE_BIT(result, 6, PB67TimerOut & (1 << 6));

    // Check if timer B underflows show up on PB7
    if (GET_BIT(PB67TimerMode, 7))
        REPLACE_BIT(result, 7, PB67TimerOut & (1 << 7));

    return result;
}

u8
CIAB::portBexternal()
{
    return 0xFF;
}

void
CIAB::updatePB()
{
    u8 internal = portBinternal();
    u8 external = portBexternal();

    u8 oldPB = PB;
    PB = (internal & DDRB) | (external & ~DDRB);

    // PLCC CIAs always return the PRB contents for output bits
    // We ignore PLCC emulation until the A600 is supported
    // if (config.type == CIA_8520_PLCC) PB = (PB & ~DDRB) | (PRB & DDRB);

    // Notify the disk controller about the changed bits
    if (oldPB ^ PB) {
        /*
        debug("PB changed: MTR: %d SEL3: %d SEL2: %d SEL1: %d SEL0: %d SIDE: %d DIR: %d STEP: %d\n",
              !!(PB & 0x80), !!(PB & 0x40), !!(PB & 0x20), !!(PB & 0x10),
              !!(PB & 0x08), !!(PB & 0x04), !!(PB & 0x02), !!(PB & 0x01));
        */
        diskController.PRBdidChange(oldPB, PB);
    }
}
