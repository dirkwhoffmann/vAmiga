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
#include "Scheduler.h"
#include "MemoryTypes.h"

/* A central element in the emulation of the Amiga is the accurate modeling of
* the DMA timeslot allocation table (Fig. 6-9 im the HRM, 3rd revision). All
* bitplane related events are managed in the BPL_SLOT. All disk, audio, and
* sprite related events are managed in the DAS_SLOT.
*
* vAmiga utilizes two event tables to schedule events in the DAS_SLOT and
* BPL_SLOT. Assuming that sprite DMA is enabled and Denise draws 6 bitplanes
* in lores mode starting at 0x28, the tables look like this:
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
*     bplEvent[0xE2] = BPL_EOL      dasEvent[0xE2] = BUS_REFRESH
*
* The BPL_EOL event doesn't perform DMA. It concludes the current line.
*
* All events in the BPL_SLOT can be superimposed by two drawing flags (bit 0
* and bit 1) that trigger the transfer of the data registers into the shift
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
*     nextBplEvent[0xE2] = 0x00     nextDasEvent[0xE2] = 0x00
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
*/

/* Hsync handler action flags
 *
 *  UPDATE_SIG_RECORDER : Forces the hsync handler to renew the contents
 *                        of the signal recorder.
 *  UPDATE_BPL_TABLE    : Forces the hsync handler to update the bitplane
 *                        DMA event table.
 *  UPDATE_DAS_TABLE    : Forces the hsync handler to update the disk,
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

    /* This value is updated in the hsync handler with the lowest 6 bits of
     * dmacon if the master enable bit is 1 or set to 0 if the master enable
     * bit is 0. It is used as an offset into the DAS lookup tables.
     */
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
    
    /* The Amiga limits the visible screen area by an upper, a lower, a left,
     * and a right border. The border encloses an area called the Display
     * Window (DIW). The color of the pixels inside the display window depends
     * on the bitplane data. The pixels of the border area are always drawn in
     * the background color (which might change inside the border area).
     * The size of the display window is controlled by two registers called
     * DIWSTRT and DIWSTOP. They contain the vertical and horizontal positions
     * at which the window starts and stops. The resolution of vertical start
     * and stop is one scan line. The resolution of horizontal start and stop
     * is one low-resolution pixel.
     *
     * I haven't found detailed information about the how the DIW logic is
     * implemented in hardware inside Agnus. If you have such information,
     * please let me know. For the time being, I base my implementation on the
     * following assumptions:
     *
     * 1. Denise contains a single flipflop controlling the display window
     *    horizontally. The flop is cleared inside the border area and set
     *    inside the display area.
     * 2. When hpos matches the position in DIWSTRT, the flipflop is set.
     * 3. When hpos matches the position in DIWSTOP, the flipflop is reset.
     * 4. The smallest valid value for DIWSTRT is $02. If it is smaller, it is
     *    not recognised.
     * 5. The largest valid value for DIWSTOP is $(1)C7. If it is larger, it is
     *    not recognised.
     */

    // Register values as written by pokeDIWSTRT/STOP()
    u16 diwstrt;
    u16 diwstop;

    /* Extracted display window coordinates
     *
     * The coordinates are computed out of diwstrt and diwstop and set in
     * setDIWSTRT/STOP(). The following horizontal values are possible:
     *
     *    diwHstrt : $02  ... $FF   or -1
     *    diwHstop : $100 ... $1C7  or -1
     *
     * A -1 is assigned if DIWSTRT or DIWSTOP are written with values that
     * result in coordinates outside the valid range.
     */
    isize diwVstrt;
    isize diwVstop;
    isize diwVstrtInitial;
    isize diwVstopInitial;

    // Indicates if the current line is a blank line
    bool lineIsBlank;

    
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
    void _dump(dump::Category category, std::ostream& os) const override;


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
        << diwVstrt
        << diwVstop
        << diwVstrtInitial
        << diwVstopInitial
        << lineIsBlank

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
    template <bool ecs, u16 signal> void processSignal(DDFState &state);

    // Updates the jump table for the bplEvent table
    void updateBplJumpTable();

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
    void updateDasEvents(u16 dmacon);

private:

    // Updates the jump table for the dasEvent table
    void updateDasJumpTable(i16 end = HPOS_MAX);
    
    
    //
    // Managing events
    //

private:
    
    void hsyncHandler();
    void vsyncHandler();
};
