// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SequencerTypes.h"
#include "Constants.h"
#include "ChangeRecorder.h"
#include "SubComponent.h"
#include "MemoryTypes.h"

/* A central element in the emulation of the Amiga is the accurate modeling of
 * the DMA timeslot allocation table (Fig. 6-9 im the HRM, 3rd revision). All
 * bitplane related events are managed in the BPL_SLOT. All disk, audio, and
 * sprite related events are managed in the DAS_SLOT.
 *
 * vAmiga utilizes two event tables for scheduling events in the DAS_SLOT and
 * BPL_SLOT. Assuming sprite DMA is enabled and Denise draws 6 bitplanes in
 * lores mode starting at 0x28, the tables look like this:
 *
 *     bplEvent[0x00] = EVENT_NONE   dasEvent[0x00] = EVENT_NONE
 *     bplEvent[0x01] = EVENT_NONE   dasEvent[0x01] = BUS_REFRESH
 *         ...                           ...
 *     bplEvent[0x28] = EVENT_NONE   dasEvent[0x28] = EVENT_NONE
 *     bplEvent[0x29] = BPL_L4       dasEvent[0x29] = DAS_S5_1
 *     bplEvent[0x2A] = BPL_L6       dasEvent[0x2A] = EVENT_NONE
 *     bplEvent[0x2B] = BPL_L2       dasEvent[0x2B] = DAS_S5_2
 *     bplEvent[0x2C] = EVENT_NONE   dasEvent[0x2C] = EVENT_NONE
 *     bplEvent[0x2D] = BPL_L3       dasEvent[0x2D] = DAS_S6_1
 *     bplEvent[0x2E] = BPL_L5       dasEvent[0x2E] = EVENT_NONE
 *     bplEvent[0x2F] = BPL_L1       dasEvent[0x2F] = DAS_S6_2
 *         ...                           ...
 *     bplEvent[0xE2] = EVENT_NONE   dasEvent[0xE2] = BUS_REFRESH
 *     bplEvent[0xE3] = EVENT_NONE   dasEvent[0xE3] = EVENT_NONE
*
 * All events in the BPL_SLOT can be superimposed by two drawing flags (bit 0
 * and bit 1) which trigger the transfer of the data registers into the shift
 * registers at the correct DMA cycle. Bit 0 controls the odd bitplanes and
 * bit 1 controls the even bitplanes.
 *
 * Each event table is accompanied by a jump table that points to the next
 * event. Given the example tables above, the jump tables would look like this:
 *
 *     nextBplEvent[0x00] = 0x29     nextDasEvent[0x00] = 0x01
 *     nextBplEvent[0x01] = 0x29     nextDasEvent[0x01] = 0x03
 *           ...                           ...
 *     nextBplEvent[0x28] = 0x29     nextDasEvent[0x28] = 0x29
 *     nextBplEvent[0x29] = 0x2A     nextDasEvent[0x29] = 0x2B
 *     nextBplEvent[0x2A] = 0x2B     nextDasEvent[0x2A] = 0x2B
 *     nextBplEvent[0x2B] = 0x2D     nextDasEvent[0x2B] = 0x2D
 *     nextBplEvent[0x2C] = 0x2D     nextDasEvent[0x2C] = 0x2D
 *     nextBplEvent[0x2D] = 0x2E     nextDasEvent[0x2D] = 0x2F
 *     nextBplEvent[0x2E] = 0x2F     nextDasEvent[0x2E] = 0x2F
 *     nextBplEvent[0x2F] = 0x31     nextDasEvent[0x2F] = 0x31
 *           ...                           ...
 *     nextBplEvent[0xE2] = 0xE2     nextDasEvent[0xE2] = 0xE3
 *     nextBplEvent[0xE3] = 0x00     nextDasEvent[0xE3] = 0x00
 *
 * Whenever one the DMA tables is modified, the corresponding jump table
 * has to be updated, too.
 *
 * To quickly setup the DAS event table, vAmiga utilizes a static lookup table.
 * Depending on the current DMA status, segments of this table are copied to
 * the event table.
 *
 *      Table: dasDMA[dmacon]
 *
 *             (Disk, Audio, and Sprite DMA events in a single rasterline)
 *
 *                 dmacon : Bits 0 .. 5 of register DMACON
 *
 * On a real Amiga, bitplane DMA is controlled by a small sub-circuit composed
 * of a small number of flip-flops. It is controlled by the following input
 * lines:
 *
 *     BMAPEN : Equals 1 iff bitplane DMA is enabled in DMACON.
 *   BPVSTART : Equals 1 iff the vertical counter matches DIWSTART.
 *    BPVSTOP : Equals 1 iff the vertical counter matches DIWSTOP.
 *        SVB : Equals 1 iff the last scanline has been reached.
 *        SHW : Equals 1 iff the horizontal counter matches the left HW stop.
 *        RHW : Equals 1 iff the horizontal counter matches the right HW stop.
 *   BPHSTART : Equals 1 iff the horizontal counter matches DDFSTART.
 *    BPHSTOP : Equals 1 iff the horizontal counter matches DDFSTOP.
 *
 * All signals except SVB, SHW, and RHW may change arbitrarily in the middle
 * of a scanline. To emulate the sequencer logic accurately, a very general
 * approach is taken. The emulator maintains a buffer called sigRecorder that
 * keeps tracks of all signal changes that occur in a the current rasterline.
 * The bitmap event table is constructed inside function computeBplEventTable
 * which basically replays all recorded signals and sets up the bitplanes
 * events accordingly. Because emulating the sequencer logic is a costly
 * operation, it is tried to postpone this task whenever possible. E.g., in
 * many cases it is sufficient to recalculate the bitplane event table at
 * the beginning of the next line.
 *
 * To keep track of pending tasks, so called action flags are utilized. They
 * are evaluated inside the hsync handler and trigger the following actions:
 *
 *  UPDATE_SIG_RECORDER : Forces the hsync handler to renew the contents
 *                        of the signal recorder suitable for the line to come.
 *     UPDATE_BPL_TABLE : Forces the hsync handler to update the bitplane
 *                        DMA event table.
 *     UPDATE_DAS_TABLE : Forces the hsync handler to update the disk,
 *                        audio, sprite DMA event table.
 */
static constexpr usize UPDATE_SIG_RECORDER  = 0b001;
static constexpr usize UPDATE_BPL_TABLE     = 0b010;
static constexpr usize UPDATE_DAS_TABLE     = 0b100;

class Sequencer : public SubComponent
{
    friend class Agnus;
    
    //
    // Event tables
    //
    
private:
    
    // Disk, audio, and sprites lookup table ([Bits 0 .. 5 of DMACON])
    static EventID dasDMA[64][HPOS_CNT];

    // Offset into the DAS lookup table
    u16 dmaDAS;

    // Current layout of a fetch unit
    EventID fetch[2][8];

public:

    // Currently scheduled events
    EventID bplEvent[HPOS_CNT];
    EventID dasEvent[HPOS_CNT];

    // Jump tables connecting the scheduled events
    u8 nextBplEvent[HPOS_CNT];
    u8 nextDasEvent[HPOS_CNT];
    
    
    //
    // Display Data Fetch (DDF)
    //

public:

    // The display data fetch registers
    u16 ddfstrt;
    u16 ddfstop;

    // The display logic state
    DDFState ddfInitial;
    DDFState ddf;

    
    //
    // Display Window (DIW)
    //

public:
    
    // Register values as written by pokeDIWSTRT/STOP()
    u16 diwstrt;
    u16 diwstop;

    // Extracted vertical display window coordinates
    isize vstrt;
    isize vstop;

    // Indicates if the current or previous line is a blank line
    bool lineIsBlank;
    bool lineWasBlank;


    //
    // Signal recorder
    //
    
public:
    
    // Signals controlling the bitplane display logic
    SigRecorder sigRecorder;

    
    //
    // Execution control
    //

private:
    
    // Action flags controlling the HSYNC handler
    usize hsyncActions;
    
    
    //
    // Initializing
    //
    
public:
    
    Sequencer(Amiga& ref);

private:
    
    void initDasEventTable();


    //
    // Methods from AmigaObject
    //

    private:

    const char *getDescription() const override { return "Sequencer"; }
    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from AmigaComponent
    //

    private:

    void _reset(bool hard) override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {

        worker
        
        << dmaDAS
        << fetch
        << bplEvent
        << dasEvent
        << nextBplEvent
        << nextDasEvent
                
        << ddfstrt
        << ddfstop
        >> ddfInitial
        >> ddf
        
        << diwstrt
        << diwstop
        << vstrt
        << vstop
        << lineIsBlank
        << lineWasBlank

        >> sigRecorder

        << hsyncActions;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Accessing registers (SequencerRegs.cpp)
    //

public:
    
    template <Accessor s> void pokeDDFSTRT(u16 value);
    void setDDFSTRT(u16 old, u16 value);
    void setDDFSTRT(u16 value) { setDDFSTRT(ddfstrt, value); }

    template <Accessor s> void pokeDDFSTOP(u16 value);
    void setDDFSTOP(u16 old, u16 value);
    void setDDFSTOP(u16 value) { setDDFSTOP(ddfstop, value); }

    void setDIWSTRT(u16 value);
    void setDIWSTOP(u16 value);

    
    //
    // Managing the bitplane time slot table (SequencerBpl.cpp)
    //
    
public:

    // Recomputes the BPL event table
    void computeBplEventTable(const SigRecorder &sr);

private:

    // Initializes the event table with default values
    void initBplEvents();

    // Predict signal changes for the current scanline
    void initSigRecorder();
    
    // Recomputes the BPL event table
    template <bool ecs> void computeBplEventTable(const SigRecorder &sr);
    template <bool ecs> void computeBplEventsSlow(const SigRecorder &sr, DDFState &state);
    template <bool ecs> void computeBplEventsFast(const SigRecorder &sr, DDFState &state);
    template <bool ecs> void computeBplEvents(isize strt, isize stop, DDFState &state);

    // Processes a signal change
    template <bool ecs> void processSignal(u16 signal, DDFState &state);
 
    // Updates the jump table for the bplEvent table
    void updateBplJumpTable(i16 end = HPOS_MAX);

    // Computes the layout of a single fetch unit
    void computeFetchUnit(u8 dmacon);
    template <u8 channels> void computeLoresFetchUnit();
    template <u8 channels> void computeHiresFetchUnit();
    
    
    //
    // Managing the disk, audio, sprite time slot table (SequencerDas.cpp)
    //

public:
    
    // Removes all events
    void initDasEvents();

    // Renews all events in the the DAS event table
    void updateDasEvents(u16 dmacon, isize pos = 0);

private:

    // Updates the jump table for the dasEvent table
    void updateDasJumpTable(i16 end = HPOS_MAX);
    
    
    //
    // Managing events
    //

private:
    
    void eolHandler();
    void eofHandler();
};
