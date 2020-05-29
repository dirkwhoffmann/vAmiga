// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGACOMPONENT_INC
#define _AMIGACOMPONENT_INC

#include "AmigaObject.h"

typedef enum
{
    EMU_OFF,
    EMU_PAUSED,
    EMU_RUNNING
}
EmulatorState;

/* Base class for all hardware components
 * This class defines the base functionality of all hardware components.
 * it comprises functions for powering up and down, resetting, suspending and
 * resuming, as well as functions for loading and saving snapshots.
 */
class HardwareComponent : public AmigaObject {

public:

    // The sub components of this component
    vector<HardwareComponent *> subComponents;

protected:
    
    /* Access lock for shared variables
     * This lock is used to control the read and write operations for all
     * variables that are accessed by both the emulator thread and the GUI.
     */
    pthread_mutex_t lock;

    /* State model
     * The virtual hardware components can be in three different states
     * called 'Off', 'Paused', and 'Running'.
     *
     *        Off: The Amiga is turned off
     *     Paused: The Amiga is turned on, but there is no emulator thread
     *    Running: The Amiga is turned on and the emulator thread running
     */
    EmulatorState state = EMU_OFF;
    
    // Indicates if this component should run in warp mode
    bool warp = false;


    //
    // Constructing and destroying
    //

public:
    
    HardwareComponent();
    virtual ~HardwareComponent();
    
    
    //
    // Initializing the component
    //
    
public:
    
    /* Initializes the component and it's sub-component.
     * This function is called exactly once, in the constructor of the Amiga
     * class. It's main purpose is to initialize the quick-reference pointers
     * contained in class HardwareComponent. Some components implement the
     * delegation method _initialize() to finalize their initialization, e.g.,
     * by setting up referecens that do not exist at the time they are
     * constructed.
     */
    void initialize();
    virtual void _initialize() { };
    
    /* There are several functions for querying and changing state:
     *
     *          -----------------------------------------------
     *         |                     run()                     |
     *         |                                               V
     *     ---------   powerOn()   ---------     run()     ---------
     *    |   Off   |------------>| Paused  |------------>| Running |
     *    |         |<------------|         |<------------|         |
     *     ---------   powerOff()  ---------    pause()    ---------
     *         ^                                               |
     *         |                   powerOff()                  |
     *          -----------------------------------------------
     *
     *     isPoweredOff()         isPaused()          isRunning()
     * |-------------------||-------------------||-------------------|
     *                      |----------------------------------------|
     *                                     isPoweredOn()
     */
    
    bool isPoweredOff() { return state == EMU_OFF; }
    bool isPoweredOn() { return state != EMU_OFF; }
    bool isPaused() { return state == EMU_PAUSED; }
    bool isRunning() { return state == EMU_RUNNING; }
    
protected:
    
    /* powerOn() powers the component on.
     *
     * current   | next      | action
     * -------------------------------------------------------------------------
     * off       | paused    | _powerOn() on each subcomponent
     * paused    | paused    | none
     * running   | running   | none
     */
    virtual void powerOn();
    virtual void _powerOn() { }
    
    /* powerOff() powers the component off.
     *
     * current   | next      | action
     * -------------------------------------------------------------------------
     * off       | off       | none
     * paused    | off       | _powerOff() on each subcomponent
     * running   | off       | pause(), _powerOff() on each subcomponent
     */
    virtual void powerOff();
    virtual void _powerOff() { }
    
    /* run() puts the component in 'running' state.
     *
     * current   | next      | action
     * -------------------------------------------------------------------------
     * off       | running   | powerOn(), _run() on each subcomponent
     * paused    | running   | _run() on each subcomponent
     * running   | running   | none
     */
    virtual void run();
    virtual void _run() { }
    
    /* pause() puts the component in 'paused' state.
     *
     * current   | next      | action
     * -------------------------------------------------------------------------
     * off       | off       | none
     * paused    | paused    | none
     * running   | paused    | _pause() on each subcomponent
     */
    virtual void pause();
    virtual void _pause() { };
    
public:
    
    /* Resets the virtual Amiga. Two reset modes are distinguished:
     *
     *     hard: A hard reset restores the initial state of all components.
     *           It resets the Amiga from an emulator point of view.
     *
     *     soft: A soft reset emulates a reset inside the virtual Amiga. It
     *           is used to emulate the CPU's RESET instruction.
     */
    virtual void reset(bool hard);
    virtual void _reset(bool hard) = 0;
    void hardReset() { reset(true); }
    void softReset() { reset(false); }
    
    /* Asks the component to inform the GUI about its current state.
     * The GUI invokes this function when it needs to update all of its visual
     * elements. This happens, e.g., when a snapshot file was loaded.
     */
    void ping();
    virtual void _ping() { }
    
    /* Collects information about the component and it's sub-components.
     * Many components contains an info variable of a class specific type
     * (e.g., CPUInfo, MemoryInfo, ...). These variables contain the
     * information shown in the GUI's inspector window and are updated by
     * calling this function. The function is called automatically when the
     * emulator switches to pause state to keep the GUI inspector data up
     * to date.
     * Note: Because this function accesses the internal emulator state with
     * many non-atomic operations, it must not be called on a running emulator.
     * To query information while the emulator is running, set up an inspection
     * target via setInspectionTarget()
     */
    void inspect();
    virtual void _inspect() { }

    /* Base method for buildung the class specific getInfo() methods
     * If the emulator is running, the result of the most recent inspection is
     * returned. If the emulator is not running, the function first updates the
     * cached values in order to return up-to-date results.
     */
    template<class T> T getInfo(T &cachedValues) {
        
        T result;
        
        if (!isRunning()) _inspect();
        
        pthread_mutex_lock(&lock);
        result = cachedValues;
        pthread_mutex_unlock(&lock);
        
        return result;
    }
    
    // Dumps debug information about the current configuration to the console
    void dumpConfig();
    virtual void _dumpConfig() { }

    // Dumps debug information about the internal state to the console
    void dump();
    virtual void _dump() { }
    
    // Getter for warp mode
    bool getWarp() { return warp; }

    // Switches warp mode on
    virtual void warpOn();
    virtual void _warpOn() { }

    // Switches warp mode off
    virtual void warpOff();
    virtual void _warpOff() { }


    //
    // Loading and saving snapshots
    //

public:

    // Returns the size of the internal state in bytes.
    size_t size();
    virtual size_t _size() = 0;

    // Loads the internal state from a memory buffer.
    size_t load(u8 *buffer);
    virtual size_t _load(u8 *buffer) = 0;

    /* Delegation methods called inside load()
     * A component can override this method to add custom behavior if not all
     * elements can be processed by the default implementation.
     */
    virtual size_t willLoadFromBuffer(u8 *buffer) { return 0; }
    virtual size_t didLoadFromBuffer(u8 *buffer) { return 0; }
    
    // Saves the internal state to a memory buffer.
    size_t save(u8 *buffer);
    virtual size_t _save(u8 *buffer) = 0;

    /* Delegation methods called inside save()
     * A component can override this method to add custom behavior if not all
     * elements can be processed by the default implementation.
     */
    virtual size_t willSaveToBuffer(u8 *buffer) {return 0; }
    virtual size_t didSaveToBuffer(u8 *buffer) { return 0; }
};

//
// Standard implementations for _reset, _load, and _save
//

#define COMPUTE_SNAPSHOT_SIZE \
SerCounter counter; \
applyToPersistentItems(counter); \
applyToResetItems(counter); \
return counter.count;

#define RESET_SNAPSHOT_ITEMS \
SerResetter resetter; \
applyToResetItems(resetter); \
debug(SNP_DEBUG, "Resetted\n");

#define LOAD_SNAPSHOT_ITEMS \
SerReader reader(buffer); \
applyToPersistentItems(reader); \
applyToResetItems(reader); \
debug(SNP_DEBUG, "Recreated from %d bytes\n", reader.ptr - buffer); \
return reader.ptr - buffer;

#define SAVE_SNAPSHOT_ITEMS \
SerWriter writer(buffer); \
applyToPersistentItems(writer); \
applyToResetItems(writer); \
debug(SNP_DEBUG, "Serialized to %d bytes\n", writer.ptr - buffer); \
return writer.ptr - buffer;

#endif
