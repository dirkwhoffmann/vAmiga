/*!
 * @header      C64.h
 * @author      Dirk W. Hoffmann, www.dirkwhoffmann.de
 * @copyright   Dirk W. Hoffmann. All rights reserved.
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _C64_INC
#define _C64_INC


// Data types and constants
#include "C64_types.h"

// General
#include "MessageQueue.h"

// Loading and saving

// Sub components
#include "ControlPort.h"
#include "Memory.h"
#include "C64Memory.h"
#include "VIC.h"
#include "TOD.h"
#include "CIA.h"

// Cartridges



/*! @class    A complete virtual Commodore 64
 *  @brief    This class is the most prominent one of all. To run the emulator,
 *            it is sufficient to create a single object of this type. All
 *            sub-components are created automatically. The public API gives
 *            you control over the emulator's behaviour such as running and
 *            pausing the emulation. Please note that most sub-components
 *            have their own public API. E.g., to query information from VICII,
 *            you need to invoke a public method on c64->vic.
 */
class C64 : public VirtualComponent {
    
    public:
    
    //
    // Hardware components
    //
    
    //! @brief    The C64's virtual memory (ROM, RAM, and color RAM)
    C64Memory mem;
    
    
    
    //! @brief    The C64's Video Interface Controller
    VIC vic;
    
    //! @brief    The C64's first Complex Interface Adapter
    CIA1 cia1;
    
    //! @brief    The C64's second Complex Interface Adapter
    CIA2 cia2;
    
    //! @brief    The C64's first control port
    ControlPort port1 = ControlPort(1);
    
    //! @brief    The C64's second control port
    ControlPort port2 = ControlPort(2);
    
    
    
    
    //
    // Frame, rasterline, and rasterline cycle information
    //
    
    //! @brief    The total number of frames drawn since power up
    uint64_t frame;
    
    //! @brief    The currently drawn rasterline
    /*! @details  The first rasterline is numbered 0. The number of rasterlines
     *            drawn in a single frame depends on the selected VICII model.
     *  @see      VIC::getRasterlinesPerFrame()
     */
    uint16_t rasterLine;
    
    /*! @brief    The currently executed rasterline cycle
     *  @details  The first rasterline cycle is numbered 1. The number of cycles
     *            executed in a single rasterline depends on the selected
     *            VICII model.
     *  @see      VIC::getCyclesPerRasterline()
     */
    uint8_t rasterCycle;

    /*! @brief    Current CPU frequency
     *  @details  This value is set in setClockFrequency()
     */
    uint32_t frequency;

    /*! @brief    Duration of a CPU cycle in 1/10 nano seconds
     *  @details  This value is set in setClockFrequency()
     */
    uint64_t durationOfOneCycle;
    
    //! @brief    VICII function table.
    /*! @details  Stores a pointer to the VICII method that is executed
     *            in a certain rasterline cycle.
     *  @note     vicfunc[0] is a stub. It is never called, because the first
     *            rasterline cycle is numbered 1.
     */
    void (VIC::*vicfunc[66])(void);
    
    
    //
    // Execution thread
    //
    
    //! @brief    An invocation counter for implementing suspend() / resume()
    unsigned suspendCounter = 0;
    
    /*! @brief    The emulators execution thread
     *  @details  The thread is created when the emulator is started and
     *            destroyed when the emulator is halted.
     */
    pthread_t p;
    
    private:
    
    /*! @brief    System timer information
     *  @details  Used to put the emulation thread to sleep for the proper
     *            amount of time.
     */
    mach_timebase_info_data_t timebase;
    
    /*! @brief    Wake-up time of the synchronization timer in nanoseconds
     *  @details  This value is recomputed each time the emulator thread is
     *            put to sleep.
     */
    uint64_t nanoTargetTime;
    
    /*! @brief    Indicates if c64 is currently running at maximum speed
     *            (with timing synchronization disabled)
     */
    bool warp;
    
    //! @brief    Indicates that we should always run as possible.
    bool alwaysWarp;
    
    /*! @brief    Indicates that we should run as fast as possible at least
     *            during disk operations.
     */
    bool warpLoad;
    
    
    //
    // Operation modes
    //
    
    /*! @brief    Indicates whether C64 is running in ultimax mode.
     *  @details  Ultimax mode can be enabled by external cartridges by pulling
     *            game line low and keeping exrom line high. In ultimax mode,
     *            most of the C64's RAM and ROM is invisible.
     */
    bool ultimax;
    
    
    //
    // Message queue
    //
    
    /*! @brief    Message queue.
     *  @details  Used to communicate with the graphical user interface. The
     *            GUI registers a listener and a callback function to retrieve
     *            messages.
     */
    MessageQueue queue;
    
    
    //
    // Snapshot storage
    //
    
    private:

    //! @brief    Indicates if snapshots should be taken automatically.
    bool takeAutoSnapshots = true;
    
    /*! @brief    Time in seconds between two auto-saved snapshots
     *  @note     This value only takes effect if takeAutoSnapshots equals true.
     */
    long autoSnapshotInterval = 3;
    
    //! @brief    Maximum number of stored snapshots
    static const size_t MAX_SNAPSHOTS = 32;
    
    
    
    //
    //! @functiongroup Constructing and destructing
    //
    
    public:
    
    //! @brief    Standard constructor
    C64();
    
    //! @brief    Stabdard destructor
    ~C64();
    
    //
    //! @functiongroup Methods from VirtualComponent
    //
    
    void reset();
    void ping();
    void setClockFrequency(uint32_t frequency);
    void suspend();
    void resume();
    void dump();
    
 
    //
    //! @functiongroup Configuring the emulator
    //
    
    /*! @brief    Returns the emulated C64 model
     *  @return   C64_CUSTOM, if the selected sub-components do not match any
     *            of the supported C64 models.
     */
    C64Model getModel();
    
    /*! @brief    Sets the currently emulated C64 model
     *  @param    m is any C64Model other than C64_CUSTOM.
     *  @note     It it safe to call this function on a running emulator.
     */
    void setModel(C64Model m);
    
    //! @brief    Updates the VIC function table
    /*! @details  This function is invoked by VIC::setModel(), only.
     */
    void updateVicFunctionTable();
    
    
    //
    //! @functiongroup Accessing the message queue
    //
    
    //! @brief    Registers a listener callback function
    void addListener(const void *sender, void(*func)(const void *, int, long) ) {
        queue.addListener(sender, func);
    }
    
    //! @brief    Removes a listener callback function
    void removeListener(const void *sender) {
        queue.removeListener(sender);
    }
    
    //! @brief    Gets a notification message from message queue
    Message getMessage() { return queue.getMessage(); }
    
    //! @brief    Feeds a notification message into message queue
    void putMessage(MessageType msg, uint64_t data = 0) { queue.putMessage(msg, data); }
    
    
    //
    //! @functiongroup Running the emulator
    //
    
    /*! @brief    Cold starts the virtual C64.
     *  @details  The emulator and all of its sub components are reset and
     *            the execution thread is started.
     *  @note     It it safe to call this function on a running emulator.
     */
    void powerUp();
    
    /*! @brief    Starts the execution thread.
     *  @details  This method launches the execution thread and is usually
     *            called after emulation was stopped by a call to halt() or by
     *            reaching a breakpoint. Calling this functions has no effect,
     *            if the emulator is currently running.
     */
    void run();
    
    /*! @brief    Stops the emulation execution thread.
     *  @details  The execution thread is canceled, but the internal state
     *            remains intact. Emulation can be continued by a call to run().
     *            Calling this functions has no effect, if the emulator is
     *            not running.
     */
    void halt();
        
    /*! @brief    The tread exit function.
     *  @details  This method is invoked automatically when the emulator thread
     *            terminates.
     */
    void threadCleanup();
    
    //! @brief    Returns true iff the virtual C64 is able to run.
    /*! @details  The emulator needs all four Roms to run. Hence, this method
     *            returns true if and only if all four Roms are installed.
     *  @see      loadRom()
     */
    bool isRunnable();
    
    //! @brief    Returns true if the emulator is running.
    bool isRunning();
    
    //! @brief    Returns true if the emulator is not running.
    bool isHalted();
    
    /*! @brief    Executes a single instruction.
     *  @details  This method implements the debugger's 'step' action.
     */
    void step();
    
    /*! @brief    Executes until the instruction is reached
     *  @details  This method implements the debugger's 'step over' action.
     */
    void stepOver();
    
    /*! @brief    Executes until the end of the current rasterline is reached.
     *  @details  This method can be called even if a certain portion of the
     *            current rasterline has already been processed.
     */
    bool executeOneLine();
    
    /*! @brief    Executes until the end of the current frame is reached.
     *  @details  This method can be called even if a certain portion of the
     *            current frame has already been processed.
     */
    bool executeOneFrame();
    
    private:
    
    //! @brief    Executes a single CPU cycle
    bool executeOneCycle();
    
    //! @brief    Work horse for executeOneCycle()
    bool _executeOneCycle();
    
    //! @brief    Invoked before executing the first cycle of a rasterline
    void beginRasterLine();
    
    //! @brief    Invoked after executing the last cycle of a rasterline
    void endRasterLine();
    
    //! @brief    Invoked after executing the last rasterline of a frame
    void endFrame();
    
    
    //
    //! @functiongroup Managing the execution thread
    //
    
    private:
    
    //! @brief    Converts kernel time to nanoseconds.
    uint64_t abs_to_nanos(uint64_t abs) { return abs * timebase.numer / timebase.denom; }
    
    //! @brief    Converts nanoseconds to kernel time.
    uint64_t nanos_to_abs(uint64_t nanos) { return nanos * timebase.denom / timebase.numer; }
    
    public:
    
    //! @brief    Updates variable warp and returns the new value.
    /*! @details  As a side effect, messages are sent to the GUI if the
     *            variable has changed its value.
     */
    bool getWarp();
    
    //! @brief    Returns if the emulator should always run full speed.
    bool getAlwaysWarp() { return alwaysWarp; }
    
    //! @brief    Setter for alwaysWarp
    void setAlwaysWarp(bool b);
    
    //! @brief    Returns if warp mode should be activated during disk access.
    bool getWarpLoad() { return warpLoad; }
    
    //! @brief    Setter for warpLoad
    void setWarpLoad(bool b);
    
    /*! @brief    Restarts the synchronization timer.
     *  @details  The function is invoked at launch time to initialize the timer
     *            and reinvoked when the synchronization timer gets out of sync.
     */
    void restartTimer();
    
    private:
    
    /*! @brief    Puts the emulation the thread to sleep for a while.
     *  @details  This function is called inside endFrame(). It makes the
     *            emulation thread wait until nanoTargetTime has been reached.
     *            Before returning, nanoTargetTime is assigned with a new target
     *            value.
     */
    void synchronizeTiming();
    
 
    //
    //! @functiongroup Handling snapshots
    //
    
 

    //
    //! @functiongroup Handling Roms
    //
    
    //! @brief    Loads a ROM image into memory
    bool loadRom(const char *filename);

    
    //
    //! @functiongroup Flashing files
    //
    
 
    //
    //! @functiongroup Set and query ultimax mode
    //
    
    public:
    
    //! @brief    Returns the ultimax flag
    bool getUltimax() { return ultimax; }
    
    /*! @brief    Setter for ultimax
     *  @details  This method is called in function updatePeekPokeLookupTables()
     *            if a certain game / exrom line combination is provided.
     */
    void setUltimax(bool b) { ultimax = b; }
    
    
    //
    //! @functiongroup Debugging
    //
    
    /*! @brief    Returns true if the executable was compiled for development
     *  @details  In release mode, assertion checking should be switched off
     */
    inline bool developmentMode() {
#ifndef NDEBUG
        return true;
#endif
        return false;
    }
};

#endif

