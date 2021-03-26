// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HardwareComponent.h"

#include "AmigaObject.hpp"
#include "Serialization.hpp"
#include "Concurrency.hpp"
#include "Reflection.hpp"

#include <vector>
#include <iostream>
#include <iomanip>

//
// Reflection APIs
//

struct OptionEnum : util::Reflection<OptionEnum, Option> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < OPT_COUNT;
    }

    static const char *prefix() { return "OPT"; }
    static const char *key(Option value)
    {
        switch (value) {
                
            case OPT_AGNUS_REVISION:      return "AGNUS_REVISION";
            case OPT_SLOW_RAM_MIRROR:     return "SLOW_RAM_MIRROR";
                
            case OPT_DENISE_REVISION:     return "DENISE_REVISION";
                
            case OPT_RTC_MODEL:           return "RTC_MODEL";

            case OPT_CHIP_RAM:            return "CHIP_RAM";
            case OPT_SLOW_RAM:            return "SLOW_RAM";
            case OPT_FAST_RAM:            return "FAST_RAM";
            case OPT_EXT_START:           return "EXT_START";
            case OPT_SLOW_RAM_DELAY:      return "SLOW_RAM_DELAY";
            case OPT_BANKMAP:             return "BANKMAP";
            case OPT_UNMAPPING_TYPE:      return "UNMAPPING_TYPE";
            case OPT_RAM_INIT_PATTERN:    return "RAM_INIT_PATTERN";
                
            case OPT_DRIVE_CONNECT:       return "DRIVE_CONNECT";
            case OPT_DRIVE_SPEED:         return "DRIVE_SPEED";
            case OPT_LOCK_DSKSYNC:        return "LOCK_DSKSYNC";
            case OPT_AUTO_DSKSYNC:        return "AUTO_DSKSYNC";

            case OPT_DRIVE_TYPE:          return "DRIVE_TYPE";
            case OPT_EMULATE_MECHANICS:   return "EMULATE_MECHANICS";
            case OPT_DRIVE_PAN:           return "DRIVE_PAN";
            case OPT_STEP_VOLUME:         return "STEP_VOLUME";
            case OPT_POLL_VOLUME:         return "POLL_VOLUME";
            case OPT_INSERT_VOLUME:       return "INSERT_VOLUME";
            case OPT_EJECT_VOLUME:        return "EJECT_VOLUME";
            case OPT_DEFAULT_FILESYSTEM:  return "DEFAULT_FILESYSTEM";
            case OPT_DEFAULT_BOOTBLOCK:   return "DEFAULT_BOOTBLOCK";
                
            case OPT_SERIAL_DEVICE:       return "SERIAL_DEVICE";
 
            case OPT_HIDDEN_SPRITES:      return "HIDDEN_SPRITES";
            case OPT_HIDDEN_LAYERS:       return "HIDDEN_LAYERS";
            case OPT_HIDDEN_LAYER_ALPHA:  return "HIDDEN_LAYER_ALPHA";
            case OPT_CLX_SPR_SPR:         return "CLX_SPR_SPR";
            case OPT_CLX_SPR_PLF:         return "CLX_SPR_PLF";
            case OPT_CLX_PLF_PLF:         return "CLX_PLF_PLF";
                    
            case OPT_BLITTER_ACCURACY:    return "BLITTER_ACCURACY";
                
            case OPT_CIA_REVISION:        return "CIA_REVISION";
            case OPT_TODBUG:              return "TODBUG";
            case OPT_ECLOCK_SYNCING:      return "ECLOCK_SYNCING";
                
            case OPT_ACCURATE_KEYBOARD:   return "ACCURATE_KEYBOARD";

            case OPT_PULLUP_RESISTORS:    return "PULLUP_RESISTORS";
            case OPT_MOUSE_VELOCITY:      return "MOUSE_VELOCITY";

            case OPT_SAMPLING_METHOD:     return "SAMPLING_METHOD";
            case OPT_FILTER_TYPE:         return "FILTER_TYPE";
            case OPT_FILTER_ALWAYS_ON:    return "FILTER_ALWAYS_ON";
            case OPT_AUDPAN:              return "AUDPAN";
            case OPT_AUDVOL:              return "AUDVOL";
            case OPT_AUDVOLL:             return "AUDVOLL";
            case OPT_AUDVOLR:             return "AUDVOLR";
                
            case OPT_COUNT:               return "???";
        }
        return "???";
    }
};

struct EmulatorStateEnum : util::Reflection<EmulatorStateEnum, EmulatorState> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < EMULATOR_STATE_COUNT;
    }

    static const char *prefix() { return "EMULATOR_STATE"; }
    static const char *key(EmulatorState value)
    {
        switch (value) {
                
            case EMULATOR_STATE_OFF:      return "OFF";
            case EMULATOR_STATE_PAUSED:   return "PAUSED";
            case EMULATOR_STATE_RUNNING:  return "RUNNING";
            case EMULATOR_STATE_COUNT:    return "???";
        }
        return "???";
    }
};

/* This class defines the base functionality of all hardware components. It
 * comprises functions for initializing, configuring, and serializing the
 * emulator, as well as functions for powering up and down, running and
 * pausing. Furthermore, a 'synchronized' macro is provided to prevent mutual
 * execution of certain code components.
 */

#define synchronized \
for (util::AutoMutex _am(mutex); _am.active; _am.active = false)

namespace Dump {
enum Category : usize {
    
    Config    = 0b0000001,
    State     = 0b0000010,
    Registers = 0b0000100,
    Events    = 0b0001000,
    Checksums = 0b0010000,
    Dma       = 0b0100000,
    BankMap   = 0b1000000
};
}

class HardwareComponent : public AmigaObject {
    
public:
    
    // Sub components
    std::vector<HardwareComponent *> subComponents;
    
protected:
    
    /* State model. The virtual hardware components can be in three different
     * states called 'Off', 'Paused', and 'Running':
     *
     *        Off: The Amiga is turned off
     *     Paused: The Amiga is turned on, but there is no emulator thread
     *    Running: The Amiga is turned on and the emulator thread running
     */
    EmulatorState state = EMULATOR_STATE_OFF;
    
    /* Indicates if the emulator should be executed in warp mode. To speed up
     * emulation (e.g., during disk accesses), the virtual hardware may be put
     * into warp mode. In this mode, the emulation thread is no longer paused
     * to match the target frequency and runs as fast as possible.
     */
    bool warpMode = false;
    
    /* Indicates if the emulator should be executed in debug mode. Debug mode
     * is enabled when the GUI debugger is opend and disabled when the GUI
     * debugger is closed. In debug mode, several time-consuming tasks are
     * performed that are usually left out. E.g., the CPU checks for
     * breakpoints and records the executed instruction in it's trace buffer.
     */
    bool debugMode = false;
            
    /* Mutex for implementing the 'synchronized' macro. The macro can be used
     * to prevent multiple threads to enter the same code block. It mimics the
     * behaviour of the well known Java construct 'synchronized(this) { }'.
     */
    util::ReentrantMutex mutex;

        
    //
    // Initializing
    //
    
public:
    
    virtual ~HardwareComponent();
    
    /* Initializes the component and it's subcomponents. The initialization
     * procedure is initiated exactly once, in the constructor of the Amiga
     * class. Some subcomponents implement the delegation method _initialize()
     * to finalize their initialization, e.g., by setting up referecens that
     * did not exist when they were constructed.
     */
    void initialize();
    virtual void _initialize() { };
    
    /* Resets the component and it's subcomponents. Two reset modes are
     * distinguished:
     *
     *     hard: A hard reset restores the initial state. It resets the Amiga
     *           from an emulator point of view.
     *
     *     soft: A soft reset emulates a reset inside the virtual Amiga. It is
     *           used to emulate the RESET instruction of the CPU.
     */
    void reset(bool hard);
    virtual void _reset(bool hard) = 0;
    
    
    //
    // Configuring
    //
    
    /* Configures the component and it's subcomponents. This function
     * distributes a configuration request to all subcomponents by calling
     * setConfigItem(). The function returns true iff the current configuration
     * has changed.
     */
    bool configure(Option option, long value) throws;
    bool configure(Option option, long id, long value) throws;
    
    /* Requests the change of a single configuration item. Each sub-component
     * checks if it is responsible for the requested configuration item. If
     * yes, it changes the internal state. If no, it ignores the request.
     * The function returns true iff the current configuration has changed.
     */
    virtual bool setConfigItem(Option option, long value) throws { return false; }
    virtual bool setConfigItem(Option option, long id, long value) throws { return false; }
    
        
    //
    // Analyzing
    //
    
    /* Collects information about the component and it's subcomponents. Many
     * components contain an info variable of a class specific type (e.g.,
     * CPUInfo, MemoryInfo, ...). These variables contain the information shown
     * in the GUI's inspector window and are updated by calling this function.
     * Note: Because this function accesses the internal emulator state with
     * many non-atomic operations, it must not be called on a running emulator.
     * To carry out inspections while the emulator is running, set up an
     * inspection target via Amiga::setInspectionTarget().
     */
    void inspect();
    virtual void _inspect() { }
    
    /* Base method for building the class specific getInfo() methods. When the
     * emulator is running, the result of the most recent inspection is
     * returned. If the emulator isn't running, the function first updates the
     * cached values in order to return up-to-date results.
     */
    template<class T> T getInfo(T &cachedValues) {
        
        if (!isRunning()) inspect();
        
        T result;
        synchronized { result = cachedValues; }
        return result;
    }
    
    /* Prints debug information about the current configuration. The additional
     * 'flags' parameter is a bit field which can be used to limit the displayed
     * information to certain categories.
     */
    void dump(Dump::Category category, std::ostream& ss) const;
    virtual void _dump(Dump::Category category, std::ostream& ss) const { };

    void dump(Dump::Category category) const;
    void dump(std::ostream& ss) const;
    void dump() const;

    
    //
    // Serializing
    //
    
    // Returns the size of the internal state in bytes
    isize size();
    virtual isize _size() = 0;
    
    // Loads the internal state from a memory buffer
    isize load(const u8 *buffer);
    virtual isize _load(const u8 *buffer) = 0;
    
    // Saves the internal state to a memory buffer
    isize save(u8 *buffer);
    virtual isize _save(u8 *buffer) = 0;
    
    /* Delegation methods called inside load() or save(). Some components
     * override these methods to add custom behavior if not all elements can be
     * processed by the default implementation.
     */
    virtual isize willLoadFromBuffer(const u8 *buffer) { return 0; }
    virtual isize didLoadFromBuffer(const u8 *buffer) { return 0; }
    virtual isize willSaveToBuffer(u8 *buffer) const {return 0; }
    virtual isize didSaveToBuffer(u8 *buffer) const { return 0; }
    
    
    //
    // Controlling
    //
    
public:
    
    /* State model. At any time, a component is in one of three states:
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
     *
     * Additional component flags: warp (on / off), debug (on / off)
     */
    
    bool isPoweredOff() const { return state == EMULATOR_STATE_OFF; }
    bool isPoweredOn() const { return state != EMULATOR_STATE_OFF; }
    bool isPaused() const { return state == EMULATOR_STATE_PAUSED; }
    bool isRunning() const { return state == EMULATOR_STATE_RUNNING; }
    
protected:
    
    /* powerOn() powers the component on.
     *
     * current   | next      | action
     * ------------------------------------------------------------------------
     * off       | paused    | _powerOn() on each subcomponent
     * paused    | paused    | none
     * running   | running   | none
     */
    void powerOn();
    virtual void _powerOn() { }
    
    /* powerOff() powers the component off.
     *
     * current   | next      | action
     * ------------------------------------------------------------------------
     * off       | off       | none
     * paused    | off       | _powerOff() on each subcomponent
     * running   | off       | pause(), _powerOff() on each subcomponent
     */
    void powerOff();
    virtual void _powerOff() { }
    
    /* run() puts the component in 'running' state.
     *
     * current   | next      | action
     * ------------------------------------------------------------------------
     * off       | running   | powerOn(), _run() on each subcomponent
     * paused    | running   | _run() on each subcomponent
     * running   | running   | none
     */
    void run();
    virtual void _run() { }
    
    /* pause() puts the component in 'paused' state.
     *
     * current   | next      | action
     * ------------------------------------------------------------------------
     * off       | off       | none
     * paused    | paused    | none
     * running   | paused    | _pause() on each subcomponent
     */
    void pause();
    virtual void _pause() { };
        
    // Switches warp mode on or off
    void setWarp(bool enable);
    virtual void _setWarp(bool enable) { };
    
    // Switches debug mode on or off
    void setDebug(bool enable);
    virtual void _setDebug(bool enable) { };
};

//
// Standard implementations of _reset, _load, and _save
//

#define COMPUTE_SNAPSHOT_SIZE \
util::SerCounter counter; \
applyToPersistentItems(counter); \
applyToHardResetItems(counter); \
applyToResetItems(counter); \
return counter.count;

#define RESET_SNAPSHOT_ITEMS(hard) \
{ \
util::SerResetter resetter; \
if (hard) applyToHardResetItems(resetter); \
applyToResetItems(resetter); \
debug(SNP_DEBUG, "Resetted (%s)\n", hard ? "hard" : "soft"); \
}

#define LOAD_SNAPSHOT_ITEMS \
{ \
util::SerReader reader(buffer); \
applyToPersistentItems(reader); \
applyToHardResetItems(reader); \
applyToResetItems(reader); \
debug(SNP_DEBUG, "Recreated from %zu bytes\n", reader.ptr - buffer); \
return (isize)(reader.ptr - buffer); \
}

#define SAVE_SNAPSHOT_ITEMS \
{ \
util::SerWriter writer(buffer); \
applyToPersistentItems(writer); \
applyToHardResetItems(writer); \
applyToResetItems(writer); \
debug(SNP_DEBUG, "Serialized to %zu bytes\n", writer.ptr - buffer); \
return (isize)(writer.ptr - buffer); \
}
