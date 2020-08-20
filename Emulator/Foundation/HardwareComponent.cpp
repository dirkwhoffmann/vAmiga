// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

HardwareComponent::~HardwareComponent()
{
    debug(RUN_DEBUG, "Terminated\n");
}

void
HardwareComponent::initialize()
{
    // Initialize all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->initialize();
    }

    // Initialize this component
    _initialize();
}

void
HardwareComponent::reset(bool hard)
{
    // Reset all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->reset(hard);
    }
    
    // Reset this component
    debug(RUN_DEBUG, "Reset [%p]\n", this);
    _reset(hard);
}

bool
HardwareComponent::configure(ConfigOption option, long value)
{
    // Configure all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->configure(option, value);
    }
    
    // Configure this component
    setConfigItem(option, value);

    return true;
}

bool
HardwareComponent::configure(unsigned dfn, ConfigOption option, long value)
{
    // Configure all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->configure(option, value);
    }
    
    // Configure this component
    setConfigItem(dfn, option, value);

    return true;
}

void
HardwareComponent::dumpConfig()
{
    msg("%s (memory location: %p)\n\n", getDescription(), this);
    _dumpConfig();
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
    msg("%s (memory location: %p)\n\n", getDescription(), this);
    _dump();
}

void
HardwareComponent::ping()
{
    // Ping all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->ping();
    }
    
    // Ping this component
    debug(RUN_DEBUG, "Ping [%p]\n", this);
    _ping();
}

size_t
HardwareComponent::size()
{
    size_t result = _size();

    for (HardwareComponent *c : subComponents) {
        result += c->size();
    }

    return result;
}

size_t
HardwareComponent::load(u8 *buffer)
{
    u8 *ptr = buffer;

    // debug("Loading internal state ...\n");

    // Call delegation method
    ptr += willLoadFromBuffer(ptr);

    // Load internal state of all subcomponents
    for (HardwareComponent *c : subComponents) {
        ptr += c->load(ptr);
    }

    // Load internal state of this component
    ptr += _load(ptr);

    // Call delegation method
    ptr += didLoadFromBuffer(ptr);

    // Verify that the number of written bytes matches the snapshot size
    debug(SNP_DEBUG, "Loaded %d bytes (expected %d)\n", ptr - buffer, size());
    assert(ptr - buffer == size());
    // panic("Loaded %d bytes (expected %d)\n", ptr - buffer, size());

    return ptr - buffer;
}

size_t
HardwareComponent::save(u8 *buffer)
{
    u8 *ptr = buffer;

    // Call delegation method
    ptr += willSaveToBuffer(ptr);

    // Save internal state of all subcomponents
    for (HardwareComponent *c : subComponents) {
        ptr += c->save(ptr);
    }

    // Save internal state of this component
    ptr += _save(ptr);

    // Call delegation method
    ptr += didSaveToBuffer(ptr);

    // Verify that the number of written bytes matches the snapshot size
    debug(SNP_DEBUG, "Saved %d bytes (expected %d)\n", ptr - buffer, size());
    assert(ptr - buffer == size());
    debug(SNP_DEBUG, "Checksum: %x\n", fnv_1a_64(buffer, ptr - buffer));
    // hexdump(buffer, MIN(ptr - buffer, 128));

    return ptr - buffer;
}

void
HardwareComponent::powerOn()
{
    if (isPoweredOff()) {

        assert(!isRunning());
        
        // Power all subcomponents on
        for (HardwareComponent *c : subComponents) {
            c->powerOn();
        }
        
        // Reset all non-persistant snapshot items
        _reset(true);

        // Power this component on
        debug(RUN_DEBUG, "Powering on\n");
        state = STATE_PAUSED;
        _powerOn();
    }
}

void
HardwareComponent::powerOff()
{
    if (isPoweredOn()) {
        
        // Pause if needed
        pause();
        
        // Power off this component
        debug(RUN_DEBUG, "Powering off\n");
        state = STATE_OFF;
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
    if (!isRunning()) {
        
        // Power on if needed
        powerOn();
            
        // Start all subcomponents
        for (HardwareComponent *c : subComponents) {
            c->run();
        }
        
        // Start this component
        debug(RUN_DEBUG, "Run\n");
        state = STATE_RUNNING;
        _run();
    }
}

void
HardwareComponent::pause()
{
    if (isRunning()) {
        
        // Pause this component
        debug(RUN_DEBUG, "Pause\n");
        state = STATE_PAUSED;
        _pause();

        // Pause all subcomponents
        for (HardwareComponent *c : subComponents) {
            c->pause();
        }
    }
}

void
HardwareComponent::setWarp(bool enable)
{
    if (warpMode == enable) return;
    
    warpMode = enable;

     // Enable or disable warp mode for all subcomponents
     for (HardwareComponent *c : subComponents) {
         c->setWarp(enable);
     }

     // Enable warp mode for this component
     _setWarp(enable);
}

void
HardwareComponent::setDebug(bool enable)
{
    if (debugMode == enable) return;
    
    debugMode = enable;

     // Enable or disable debug mode for all subcomponents
     for (HardwareComponent *c : subComponents) {
         c->setDebug(enable);
     }

     // Enable debug mode for this component
     _setDebug(enable);
}
