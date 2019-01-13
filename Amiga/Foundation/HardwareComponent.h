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

// Forward declarations
class Amiga;


/* Base class for all hardware components
 * This class defines the base functionality of all hardware components.
 * it comprises functions for powering up and down, resetting, suspending and
 * resuming, as well as functions for loading and saving snapshots.
 */
class HardwareComponent : public AmigaObject {
    
protected:
    
    /* Type and behavior of a snapshot item
     * The reset flags indicate whether the snapshot item should be set to 0
     * automatically during a reset. The format flags are important when big
     * chunks of data are specified. They are needed loadBuffer and saveBuffer
     * to correctly converting little endian to big endian format.
     */
    enum {
        KEEP_ON_RESET  = 0x00, //! Don't touch item during a reset
        CLEAR_ON_RESET = 0x10, //! Reset to zero during a reset
        
        BYTE_ARRAY     = 0x01, //! Data chunk is an array of bytes
        WORD_ARRAY     = 0x02, //! Data chunk is an array of words
        DWORD_ARRAY    = 0x04, //! Data chunk is an array of double words
        QWORD_ARRAY    = 0x08  //! Data chunk is an array of quad words
    };
    
    /* Fingerprint of a snapshot item
     */
    typedef struct {
        
        void *data;
        size_t size;
        uint8_t flags;
        
    } SnapshotItem;
    
public:
    
    /* Reference to the Amiga top-level object.
     * Because nearly all hardware components need to interact very closely
     * with each other, we keep a reference to the top-level object in each
     * component. Hence, each component can access every other component every
     * time.
     */
    Amiga *amiga = NULL;
    
protected:
    
    // Indicates if the component is powered up or down.
    bool power = false;
    
    // Sub components of this component
    HardwareComponent **subComponents = NULL;
    
    // List of snapshot items of this component
    SnapshotItem *snapshotItems = NULL;
    
    // Snapshot size on disk (in bytes)
    unsigned snapshotSize = 0;
    
public:
    
    virtual ~HardwareComponent();
    
    
    //
    // Initializing the component
    //
    
    /* Assign top-level C64 object.
     * The provided reference is propagated automatically to all sub components.
     * This functions is called in the constructor of the Amiga class and never
     * called again afterwards.
     */
    virtual void setAmiga(Amiga *amiga);
    
    // Returns true if the component is powered on
    bool isPoweredOn() { return power; }

    // Returns true if the component is powered off
    bool isPoweredOff() { return !power; }

    /* Emulates a power on event on the virtual Amiga.
     * By default, each component powers on its sub components.
     */
    void powerOn();

    // Subclass specific powerOn behaviour
    virtual void _powerOn() { _reset(); }
    
    /* Emulates a power off event on the virtual Amiga.
     * By default, each component powers off its sub components.
     */
    void powerOff();

    // Subclass specific powerOff behaviour
    virtual void _powerOff() { };
    
    // Calls powerOn() or powerOff(), depending on the current state.
    void powerOnOrOff() { isPoweredOn() ? powerOff() : powerOn(); }
    
    /* Emulates a reset event on the virtual Amiga.
     * By default, each component resets its sub components.
     */
    void reset();
    
    // Subclass specific reset behaviour
    virtual void _reset() { };
    
    /* Asks the component to inform the GUI about its current state.
     * The GUI invokes this function when it needs to update all of its visual
     * elements. This happens, e.g., when a snapshot file was loaded.
     */
    void ping();
    
    // Subclass specific ping behaviour
    virtual void _ping() { };
    
    // Dumps some debug information about the internal state to the console.
    void dump();
    
    // Subclass specific dump behaviour
    virtual void _dump() { };
    
       
    //
    // Registering snapshot items and sub components
    //
    
    /* Registers all subcomponents of this component.
     * Subcomponents are usually registered in the constructor of a component.
     *   - subComponents  points to the first element of a HardwareComponent* array.
     *                    The end of the array is marked by a NULL pointer.
     *   - length         Size of the array in bytes.
     */
    void registerSubcomponents(HardwareComponent **subcomponents, unsigned length);
    
    /* Registers all snapshot items for this component.
     * Snaphshot items are usually registered in the constructor of a component.
     *   - item           points to the first element of a SnapshotItem* array.
     *                    The end of the array is marked by a NULL pointer.
     *   - length         Size of the array in bytes.
     */
    void registerSnapshotItems(SnapshotItem *items, unsigned length);
    
    
public:
    
    //
    // Loading and saving snapshots
    //
    
    // Returns the size of the internal state in bytes.
    virtual size_t stateSize();
    
    /* Loads the internal state from a memory buffer.
     */
    void loadFromBuffer(uint8_t **buffer);
    
    /* Delegation methods called inside loadFromBuffer()
     * A component can override this method to add custom behavior if not all
     * elements can be processed by the default implementation.
     */
    virtual void  willLoadFromBuffer(uint8_t **buffer) { };
    virtual void  didLoadFromBuffer(uint8_t **buffer) { };
    
    /* Saves the internal state to a memory buffer.
     */
    void saveToBuffer(uint8_t **buffer);
    
    /* Delegation methods called inside saveToBuffer()
     * A component can override this method to add custom behavior if not all
     * elements can be processed by the default implementation.
     */
    virtual void  willSaveToBuffer(uint8_t **buffer) { };
    virtual void  didSaveToBuffer(uint8_t **buffer) { };
};

#endif
