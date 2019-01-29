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
    
    if (subComponents)
        delete [] subComponents;
    
    if (snapshotItems)
        delete [] snapshotItems;
}

void
HardwareComponent::setAmiga(Amiga *amiga)
{
    assert(this->amiga == NULL);
    assert(amiga != NULL);
    
    this->amiga = amiga;
    
    // Set the reference in all sub components
    if (subComponents != NULL)
        for (unsigned i = 0; subComponents[i] != NULL; i++)
            subComponents[i]->setAmiga(amiga);
}

void
HardwareComponent::powerOn()
{
    if (!power) {
        
        // Power all sub components on
        if (subComponents != NULL)
            for (unsigned i = 0; subComponents[i] != NULL; i++)
                subComponents[i]->powerOn();
        
        // Power on this component
        debug(2, "Power on\n");
        power = true;
        _powerOn();
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
        if (subComponents != NULL)
            for (unsigned i = 0; subComponents[i] != NULL; i++)
                subComponents[i]->powerOff();
    }
}

void
HardwareComponent::run()
{
    if (!running) {
        
        // Power on if needed
        powerOn();
            
        // Start all sub components
        if (subComponents != NULL)
            for (unsigned i = 0; subComponents[i] != NULL; i++)
                subComponents[i]->run();
        
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
        if (subComponents != NULL)
            for (unsigned i = 0; subComponents[i] != NULL; i++)
                subComponents[i]->pause();
    }
}

void
HardwareComponent::reset()
{
    // Reset all sub components
    if (subComponents != NULL)
        for (unsigned i = 0; subComponents[i] != NULL; i++)
            subComponents[i]->reset();
    
    // Clear snapshot items marked with 'KEEP_ON_RESET'
    if (snapshotItems != NULL)
        for (unsigned i = 0; snapshotItems[i].data != NULL; i++)
            if (snapshotItems[i].flags & CLEAR_ON_RESET)
                memset(snapshotItems[i].data, 0, snapshotItems[i].size);
    
    // Reset this component
    debug(2, "Powering down[%p]\n", this);
    _reset();
}

void
HardwareComponent::ping()
{
    // Ping all sub components
    if (subComponents != NULL)
        for (unsigned i = 0; subComponents[i] != NULL; i++)
            subComponents[i]->ping();
    
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
HardwareComponent::registerSubcomponents(HardwareComponent **components, unsigned length) {
    
    assert(components != NULL);
    assert(length % sizeof(HardwareComponent *) == 0);
    
    unsigned numItems = length / sizeof(HardwareComponent *);
    
    // Allocate new array on heap and copy array data
    subComponents = new HardwareComponent*[numItems];
    std::copy(components, components + numItems, &subComponents[0]);
}

void
HardwareComponent::registerSnapshotItems(SnapshotItem *items, unsigned length) {
    
    assert(items != NULL);
    assert(length % sizeof(SnapshotItem) == 0);
    
    unsigned i, numItems = length / sizeof(SnapshotItem);
    
    debug("Registering %d items\n", numItems);
    
    // Allocate new array on heap and copy array data
    snapshotItems = new SnapshotItem[numItems];
    std::copy(items, items + numItems, &snapshotItems[0]);
    
    // Determine size of snapshot on disk
    for (i = snapshotSize = 0; snapshotItems[i].data != NULL; i++)
        snapshotSize += snapshotItems[i].size;
}

size_t
HardwareComponent::stateSize()
{
    uint32_t result = snapshotSize;
    
    if (subComponents != NULL)
        for (unsigned i = 0; subComponents[i] != NULL; i++)
            result += subComponents[i]->stateSize();
    
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
    if (subComponents != NULL)
        for (unsigned i = 0; subComponents[i] != NULL; i++)
            subComponents[i]->loadFromBuffer(buffer);
    
    // Load own internal state
    void *data; size_t size; int flags;
    for (unsigned i = 0; snapshotItems != NULL && snapshotItems[i].data != NULL; i++) {
        
        data  = snapshotItems[i].data;
        flags = snapshotItems[i].flags & 0x0F;
        size  = snapshotItems[i].size;
        
        if (flags == 0) { // Auto detect size
            
            debug("Reading back to %p\n", data);
            
            switch (snapshotItems[i].size) {
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
    if (subComponents != NULL) {
        for (unsigned i = 0; subComponents[i] != NULL; i++)
            subComponents[i]->saveToBuffer(buffer);
    }
    
    // Save own internal state
    void *data; size_t size; int flags;
    for (unsigned i = 0; snapshotItems != NULL && snapshotItems[i].data != NULL; i++) {
        
        data  = snapshotItems[i].data;
        flags = snapshotItems[i].flags & 0x0F;
        size  = snapshotItems[i].size;
        
        if (flags == 0) { // Auto detect size
            
            switch (snapshotItems[i].size) {
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
