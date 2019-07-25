// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DISK_CONTROLLER_INC
#define _DISK_CONTROLLER_INC

#include "HardwareComponent.h"

class Drive;

class DiskController : public HardwareComponent {

    // Quick-access references
    class Memory *mem; 
    class Agnus *agnus;
    class Paula *paula; 
    
    //
    // Configuration items
    //
    
    /* Connection status of all four drives.
     * Note: connected[0] is always true, because the internal drive cannot be
     * disconnected.
     */
    bool connected[4] = { true, false, false, false };
    
    // Indicates if a FIFO buffer should be emulated.
    bool fifoBuffering = true;
    
    
    //
    // Bookkeeping
    //
    
    // Information shown in the GUI inspector panel
    DiskControllerInfo info;
    
    // Quick-references to the disk drives
    Drive *df[4] = { NULL, NULL, NULL, NULL };
    
    // The currently selected drive (-1 if no drive is selected)
    int8_t selected = -1;
    
    // The number of words transferred during a single DMA cycle.
    int32_t acceleration = 1;
    
    // The current drive state (off, read, or write)
    DriveState state;
    
    // Set to true if the currently read disk word matches the sync word.
    // NOT USED AT THE MOMENT
    bool syncFlag = false;
    
    
    //
    // Data buffers
    //
    
    // The latest incoming byte (value shows up in DSKBYTER)
    uint8_t incoming;
    
    // Timestamp of the latest write to variable 'incoming'
    Cycle incomingCycle;
    
    /* The drive controller's FIFO buffer
     * On each DSK_ROTATE event, a byte is read from the selected drive and
     * put into this buffer. Each Disk DMA operation will read two bytes from
     * the buffer and store them at the desired location.
     */
    uint64_t fifo;
    
    // Number of bytes stored in the FIFO buffer
    uint8_t fifoCount;
    
    
    //
    // Registers
    //
    
    // Disk DMA block length
    uint16_t dsklen;
    
    // Disk SYNC word
    uint16_t dsksync;
    
    // A copy of the PRB register of CIA B
    uint8_t prb;
    
    
    //
    // Debugging
    //
    
    // For debugging, an FNV-32 checksum is computed for each DMA operation.
    uint32_t checksum = fnv_1a_init32();
    uint64_t checkcnt = 0;

    
    //
    // Constructing and destructing
    //
    
public:
    
    DiskController();
    

    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & connected
        & fifoBuffering;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & selected
        & acceleration
        & state
        & syncFlag
        & incoming
        & incomingCycle
        & fifo
        & fifoCount
        & dsklen
        & prb
        & checksum
        & checkcnt;
    }

    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _inspect() override;
    void _dump() override;
    size_t _load(uint8_t *buffer) override;
    size_t _save(uint8_t *buffer) override;

    
    //
    // Accesing the internal state
    //
    
public:
    
    // Returns the latest internal state recorded by inspect().
    DiskControllerInfo getInfo();

    // Indicates if the motor of the specified drive is switched on.
    bool spinning(unsigned driveNr);

    // Indicates if the motor of at least one drive is switched on.
    bool spinning();
    
    // Returns the current drive state
    DriveState getState() { return state; }
    
private:
    
    void setState(DriveState state);
    
    
    //
    // Getter and setter
    //
    
public:
    
    // Connection status
    bool isConnected(int df) { assert(df < 4); return connected[df]; }
    void setConnected(int df, bool value);
    
    void connect(int df) { setConnected(df, true); }
    void disconnect(int df) { setConnected(df, false); }
    void toggleConnected(int df) { setConnected(df, !isConnected(df)); }
    
    // FIFO emulation
    bool getFifoBuffering() { return fifoBuffering; }
    void setFifoBuffering(bool value) { fifoBuffering = value; }
    
    // Returns the currently selected drive or NULL if no drive is selected.
    Drive *getSelectedDrive();

    
    //
    // Accessing registers
    //
    
public:
    
    // OCR register 0x008 (r)
    uint16_t peekDSKDATR();
    
    // OCR register 0x024 (w)
    void pokeDSKLEN(uint16_t value);
    
    // OCR register 0x026 (w)
    void pokeDSKDAT(uint16_t value);
    
    // OCR register 0x01A (r)
    uint16_t peekDSKBYTR();
    
    // OCR register 0x07E (w)
    void pokeDSKSYNC(uint16_t value);
    
    // Read handler for the PRA register of CIA A
    uint8_t driveStatusFlags();
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(uint8_t oldValue, uint8_t newValue);
    
    
    //
    // Processing events
    //
    
public:
    
    // Serves an event in the disk controller slot.
    void serveDiskEvent();
    
    
    //
    // Working with the FIFO buffer
    //
    
private:
    
    // Informs about the current FIFO fill state
    bool fifoIsEmpty() { return fifoCount == 0; }
    bool fifoIsFull() { return fifoCount == 6; }
    bool fifoHasWord() { return fifoCount >= 2; }
    bool fifoCanStoreWord() { return fifoCount <= 4; }

    // Clears the FIFO buffer.
    void clearFifo();
    
    // Reads or writes a byte from or to the FIFO
    uint8_t readFifo();
    void writeFifo(uint8_t byte);

    // Reads a word from the FIFO buffer. DEPRECATED
    uint16_t readFifo16();
    
    // Returns true if the next word to read matches the specified value
    bool compareFifo(uint16_t word);

    /* Emulates a data transfert between the selected drive and the FIFO buffer.
     * This function is executed periodically in serveDiskEvent().
     * The exact operation is dependent of the current DMA state. If DMA is
     * off, no action is taken. If a read mode is active, the FIFO is filled
     * with data from the drive. If a write mode is active, data from the FIFO
     * is written to the drive head.
     */
    void executeFifo();
         
    
    //
    // Performing DMA
    //

public:

    /* The emulator supports three disk DMA modes at the moment:
     *
     *     1. Standard DMA mode    (most accurate)
     *     2. Simple DMA mode
     *     3. Turbo DMA mode       (least accurate)
     *
     * In standard DMA mode, performDMA() is invoked three times per raster
     * line, in each of the three DMA slots. Communication with the drive is
     * decoupled by a FIFO buffer. Data is never read directly from or written
     * to the drive. It is always exchanged via the FIFO. Data transfer
     * between the FIFO and the drive takes place in serveDiskEvent(), which is
     * called periodically by the event handler.
     *
     * In simple DMA mode, performDMA() is called three times per raster
     * line, just like in standard mode. The FIFO phase is skipped. I.e., data
     * is read from or written to the drive immediately when a DMA transfer
     * takes place.
     *
     * Turbo DMA mode is applied when the drive is configured as a turbo drive.
     * With these drives, data is transferred immediately when the DSKLEN
     * register is written. This mode is the least compatible. Neither does it
     * uses the rasterline DMA slots, nor does it use a FIFO buffer.
     */
  
    // 1. Standard DMA mode
    void performDMA();
    void performDMARead(Drive *drive);
    void performDMAWrite(Drive *drive);
 
    // 2. Simple DMA mode
    void performSimpleDMA();
    void performSimpleDMARead(Drive *drive);
    void performSimpleDMAWrite(Drive *drive);

    // 3. Turbo DMA mode
    void performTurboDMA(Drive *d);
    void performTurboRead(Drive *drive);
    void performTurboWrite(Drive *drive);
};

#endif
