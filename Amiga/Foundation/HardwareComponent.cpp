// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

HardwareComponent::HardwareComponent()
{
    pthread_mutex_init(&lock, NULL);
}

HardwareComponent::~HardwareComponent()
{
    pthread_mutex_destroy(&lock);
    debug(3, "Terminated\n");
}

void
HardwareComponent::prefix()
{
    fprintf(stderr, "[%lld] (%3d,%3d) ",
            _amiga->agnus.frame, _amiga->agnus.vpos, _amiga->agnus.hpos);

    fprintf(stderr, " %06X: ", _amiga->cpu.getPC());

    uint16_t dmacon = _amiga->agnus.dmacon;
    bool dmaen = dmacon & DMAEN;
    fprintf(stderr, "%c%c%c%c ",
            (dmacon & BPLEN) ? (dmaen ? 'P' : 'p') : '-',
            (dmacon & COPEN) ? (dmaen ? 'C' : 'c') : '-',
            (dmacon & BLTEN) ? (dmaen ? 'B' : 'b') : '-',
            (dmacon & DSKEN) ? (dmaen ? 'D' : 'd') : '-');

    fprintf(stderr, "%04X %04X: ", _paula->intena, _paula->intreq);

    if (getDescription())
        fprintf(stderr, "%s: ", getDescription());
}

void
HardwareComponent::initialize(Amiga *amiga)
{
    assert(this->_amiga == NULL);
    assert(amiga != NULL);

    this->_amiga   = amiga;
    this->_handler = &amiga->agnus.eventHandler;
    this->_mem     = &amiga->mem;
    this->_ciaA    = &amiga->ciaA;
    this->_ciaB    = &amiga->ciaB;
    this->_paula   = &amiga->paula;
    this->_zorro   = &amiga->zorro;

    // Initialize all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->initialize(amiga);
    }
    
    // Call the delegation method
    _initialize();
}

void
HardwareComponent::powerOn()
{
    if (!power) {
        
        // Power all subcomponents on
        for (HardwareComponent *c : subComponents) {
            c->powerOn();
        }
        
        // Zero out all snapshot items that are not marked as persistent.
        for (SnapshotItem i : snapshotItems) {
            if ((i.flags & PERSISTANT) == 0) memset(i.data, 0, i.size);
        }
        
        // Power this component on
        debug(2, "Powering on\n");
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
        debug(2, "Powering off\n");
        power = false;
        _powerOff();

        // Power all subcomponents off
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
            
        // Start all subcomponents
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

        // Pause all subcomponents
        for (HardwareComponent *c : subComponents) {
            c->pause();
        }
    }
}

void
HardwareComponent::reset()
{
    // Reset all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->reset();
    }
    
    // Reset this component
    debug(2, "Reset [%p]\n", this);
    _reset();
}

void
HardwareComponent::ping()
{
    // Ping all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->ping();
    }
    
    // Ping this component
    debug(2, "Ping [%p]\n", this);
    _ping();
}

void
HardwareComponent::inspect()
{
    // Inspect all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->inspect();
    }
    
    // Inspect this component
    _inspect();
}

void
HardwareComponent::dump()
{
    plainmsg("%s (memory location: %p)\n\n", getDescription(), this);
    _dump();
}

void
HardwareComponent::setWarp(bool value)
{
    if (warp != value) {
        
        warp = value;
        
        // Enable or disable warp mode for all subcomponents
        for (HardwareComponent *c : subComponents) {
            c->setWarp(value);
        }
        
        // Enable or disable warp mode for this component
        _setWarp(value);
    }
}

void
HardwareComponent::registerSubcomponents(vector<HardwareComponent *> components) {
    
    subComponents = components;
}

void
HardwareComponent::registerSnapshotItems(vector<SnapshotItem> items) {
    
    snapshotItems = items;
    snapshotSize = 0;
    
    for (SnapshotItem item : snapshotItems) {
        
        debug(3, "Registering item at address %p", item.data);
        
        // Calculate snapshot size
        snapshotSize += item.size;
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
    
    // Load internal state of all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->loadFromBuffer(buffer);
    }
    
    // Load internal state of this component
    for (SnapshotItem i : snapshotItems) {
        
        switch (i.flags & 0x0F) {
                
            case 0: // Auto detect
                
                switch (i.size) {
                    case 1:  *i.data8  = read8(buffer);  break;
                    case 2:  *i.data16 = read16(buffer); break;
                    case 4:  *i.data32 = read32(buffer); break;
                    case 8:  *i.data64 = read64(buffer); break;
                    default: readBlock(buffer, i.data8, i.size);
                }
                break;
            
            case 1: // Byte array
            
                readBlock(buffer, i.data8, i.size);
                break;
                
            case 2: // Word array
                
                readBlock16(buffer, i.data16, i.size);
                break;
                
            case 4: // Double word array
                
                readBlock32(buffer, i.data32, i.size);
                break;
                
            case 8: // Quad word array
                
                readBlock64(buffer, i.data64, i.size);
                break;
                
            default:
                assert(false);
        }
    }
    
    // Call delegation method
    didLoadFromBuffer(buffer);
    
    // Verify that the number of processed bytes matches the state size
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
    
    // Save internal state of all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->saveToBuffer(buffer);
    }
    
    // Save internal state of this component
    for (SnapshotItem i : snapshotItems) {
        
        switch (i.flags & 0x0F) {
                
            case 0: // Auto detect
                
                switch (i.size) {
                    case 1:  write8(buffer, *i.data8); break;
                    case 2:  write16(buffer, *i.data16); break;
                    case 4:  write32(buffer, *i.data32); break;
                    case 8:  write64(buffer, *i.data64); break;
                    default: writeBlock(buffer, i.data8, i.size);
                }
                break;
                
            case 1: // Byte array
                
                writeBlock(buffer, i.data8, i.size);
                break;
                
            case 2: // Word array
                
                writeBlock16(buffer, i.data16, i.size);
                break;
                
            case 4: // Double word array
                
                writeBlock32(buffer, i.data32, i.size);
                break;
                
            case 8: // Quad word array
                
                writeBlock64(buffer, i.data64, i.size);
                break;
                
            default:
                assert(false);
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
