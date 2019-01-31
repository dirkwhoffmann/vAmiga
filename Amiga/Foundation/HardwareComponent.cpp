// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "HardwareComponent.h"
#include <algorithm>

HardwareComponent::~HardwareComponent()
{
    debug(3, "Terminated\n");

    if (snapshotItemsOld)
        delete [] snapshotItemsOld;
}

void
HardwareComponent::setAmiga(Amiga *amiga)
{
    assert(this->amiga == NULL);
    assert(amiga != NULL);
    
    this->amiga = amiga;
    
    // Set the reference in all sub components
    for (HardwareComponent *c : subComponents) {
        c->setAmiga(amiga);
    }
}

void
HardwareComponent::powerOn()
{
    if (!power) {
        
        // Power all sub components on
        for (HardwareComponent *c : subComponents) {
            c->powerOn();
        }
        
        // Power on this component
        debug(2, "Power on\n");
        power = true;
        _powerOn();
        
        // Watch out for unitialized state variables
           for (unsigned i = 0; snapshotItemsOld != NULL && snapshotItemsOld[i].data != NULL; i++) {
               assert(((uint8_t *)snapshotItemsOld[i].data)[0] != 42);
           }
    }
}

void
HardwareComponent::powerOff()
{
    if (power) {
        
         // Pause if needed
        pause();
        
        // Power off this component
        debug(2, "Power off\n");
        power = false;
        _powerOff();

        // Power all sub components off
        for (HardwareComponent *c : subComponents) {
            c->powerOff();
        }
    }
}

void
HardwareComponent::run()
{
    if (!running) {
        
        // Power on if needed
        powerOn();
            
        // Start all sub components
        for (HardwareComponent *c : subComponents) {
            c->run();
        }
        
        // Start this component
        debug(2, "Run\n");
        running = true;
        _run();
    }
}

void
HardwareComponent::pause()
{
    if (running) {
        
        // Pause this component
        debug(2, "Pause\n");
        running = false;
        _pause();

        // Pause all sub components
        for (HardwareComponent *c : subComponents) {
            c->pause();
        }
    }
}

void
HardwareComponent::reset()
{
    // Reset all sub components
    for (HardwareComponent *c : subComponents) {
        c->reset();
    }
    
    // Clear snapshot items marked with 'KEEP_ON_RESET'
    if (snapshotItemsOld != NULL)
        for (unsigned i = 0; snapshotItemsOld[i].data != NULL; i++)
            if (snapshotItemsOld[i].flags & CLEAR_ON_RESET)
                memset(snapshotItemsOld[i].data, 0, snapshotItemsOld[i].size);
    
    // Reset this component
    debug(2, "Powering down[%p]\n", this);
    _reset();
}

void
HardwareComponent::ping()
{
    // Ping all sub components
    for (HardwareComponent *c : subComponents) {
        c->ping();
    }
    
    // Ping this component
    debug(2, "Pinging[%p]\n", this);
    _ping();
}

void
HardwareComponent::dump()
{
    plainmsg("<%s>: Object at %p\n\n", getDescription(), this);
    _dump();
}

void
HardwareComponent::setWarp(bool value)
{
    if (warp != value) {
        
        warp = value;
        
        // Inform all sub components
        for (HardwareComponent *c : subComponents) {
            c->setWarp(value);
        }
        
        // Switch warp mode on or off in this component
        _setWarp(value);
    }
}

void
HardwareComponent::registerSubcomponents(vector<HardwareComponent *> components) {
    
    subComponents = components;
}

void
HardwareComponent::registerSnapshotItemsOld(SnapshotItem *items, unsigned length) {
    
    assert(items != NULL);
    assert(length % sizeof(SnapshotItem) == 0);
    
    unsigned i, numItems = length / sizeof(SnapshotItem);
    
    debug("Registering %d items\n", numItems);
    
    // Allocate new array on heap and copy array data
    snapshotItemsOld = new SnapshotItem[numItems];
    std::copy(items, items + numItems, &snapshotItemsOld[0]);
    
    // Initialize all snapshot items with a special bit pattern and determine
    // snapshot size. The bit pattern is used in powerOn() to detect
    // uninitialized variables.
    for (i = snapshotSize = 0; snapshotItemsOld[i].data != NULL; i++) {
        snapshotSize += snapshotItemsOld[i].size;
        uint8_t *data = (uint8_t *)snapshotItemsOld[i].data;
        for (size_t j = 0; j < snapshotItemsOld[i].size; j++) {
            data[j] = 42;
        }
    }
}

size_t
HardwareComponent::stateSize()
{
    uint32_t result = snapshotSize;
    
    for (HardwareComponent *c : subComponents) {
        result += c->stateSize();
    }

    return result;
}

void
HardwareComponent::loadFromBuffer(uint8_t **buffer)
{
    uint8_t *old = *buffer;
    
    debug(3, "    Loading internal state ...\n");
    
    // Call delegation method
    willLoadFromBuffer(buffer);
    
    // Load internal state of all sub components
    for (HardwareComponent *c : subComponents) {
        c->loadFromBuffer(buffer);
    }
    
    // Load own internal state
    void *data; size_t size; int flags;
    for (unsigned i = 0; snapshotItemsOld != NULL && snapshotItemsOld[i].data != NULL; i++) {
        
        data  = snapshotItemsOld[i].data;
        flags = snapshotItemsOld[i].flags & 0x0F;
        size  = snapshotItemsOld[i].size;
        
        if (flags == 0) { // Auto detect size
            
            debug("Reading back to %p\n", data);
            
            switch (snapshotItemsOld[i].size) {
                case 1:  *(uint8_t *)data  = read8(buffer); break;
                case 2:  *(uint16_t *)data = read16(buffer); break;
                case 4:  *(uint32_t *)data = read32(buffer); break;
                case 8:  *(uint64_t *)data = read64(buffer); break;
                default: readBlock(buffer, (uint8_t *)data, size);
            }
            
        } else { // Format is specified manually
            
            switch (flags) {
                case BYTE_ARRAY: readBlock(buffer, (uint8_t *)data, size); break;
                case WORD_ARRAY: readBlock16(buffer, (uint16_t *)data, size); break;
                case DWORD_ARRAY: readBlock32(buffer, (uint32_t *)data, size); break;
                case QWORD_ARRAY: readBlock64(buffer, (uint64_t *)data, size); break;
                default: assert(0);
            }
        }
    }
    
    // Call delegation method
    didLoadFromBuffer(buffer);
    
    // Verify that the number of read bytes matches the state size
    if (*buffer - old != stateSize()) {
        panic("loadFromBuffer: Snapshot size is wrong. Got %d, expected %d.",
              *buffer - old, stateSize());
        assert(false);
    }
}

void
HardwareComponent::saveToBuffer(uint8_t **buffer)
{
    uint8_t *old = *buffer;
    
    debug(4, "    Saving internal state ...\n");
    
    // Call delegation method
    willSaveToBuffer(buffer);
    
    // Save internal state of all sub components
    for (HardwareComponent *c : subComponents) {
        c->saveToBuffer(buffer);
    }
    
    // Save own internal state
    void *data; size_t size; int flags;
    for (unsigned i = 0; snapshotItemsOld != NULL && snapshotItemsOld[i].data != NULL; i++) {
        
        data  = snapshotItemsOld[i].data;
        flags = snapshotItemsOld[i].flags & 0x0F;
        size  = snapshotItemsOld[i].size;
        
        if (flags == 0) { // Auto detect size
            
            switch (snapshotItemsOld[i].size) {
                case 1:  write8(buffer, *(uint8_t *)data); break;
                case 2:  write16(buffer, *(uint16_t *)data); break;
                case 4:  write32(buffer, *(uint32_t *)data); break;
                case 8:  write64(buffer, *(uint64_t *)data); break;
                default: writeBlock(buffer, (uint8_t *)data, size);
            }
            
        } else { // Format is specified manually
            
            switch (flags) {
                case BYTE_ARRAY: writeBlock(buffer, (uint8_t *)data, size); break;
                case WORD_ARRAY: writeBlock16(buffer, (uint16_t *)data, size); break;
                case DWORD_ARRAY: writeBlock32(buffer, (uint32_t *)data, size); break;
                case QWORD_ARRAY: writeBlock64(buffer, (uint64_t *)data, size); break;
                default: assert(0);
            }
        }
    }
    
    // Call delegation method
    didSaveToBuffer(buffer);
    
    // Verify that the number of written bytes matches the state size
    if (*buffer - old != stateSize()) {
        panic("saveToBuffer: Snapshot size is wrong. Got %d, expected %d.",
              *buffer - old, stateSize());
        assert(false);
    }
}
