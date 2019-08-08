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
HardwareComponent::prefix() const
{
    fprintf(stderr, "[%lld] (%3d,%3d) ",
            amiga->agnus.frame, amiga->agnus.pos.v, amiga->agnus.pos.h);

    fprintf(stderr, " %06X ", amiga->cpu.getPC());

    uint16_t dmacon = amiga->agnus.dmacon;
    bool dmaen = dmacon & DMAEN;
    fprintf(stderr, "%c%c%c%c%c%c ",
            (dmacon & BPLEN) ? (dmaen ? 'B' : 'B') : '-',
            (dmacon & COPEN) ? (dmaen ? 'C' : 'c') : '-',
            (dmacon & BLTEN) ? (dmaen ? 'B' : 'b') : '-',
            (dmacon & SPREN) ? (dmaen ? 'S' : 's') : '-',
            (dmacon & DSKEN) ? (dmaen ? 'D' : 'd') : '-',
            (dmacon & AUDEN) ? (dmaen ? 'A' : 'a') : '-');

    fprintf(stderr, "%04X %04X ", amiga->paula.intena, amiga->paula.intreq);

    if (amiga->agnus.copper.servicing) {
        fprintf(stderr, "[%06X] ", amiga->agnus.copper.getCopPC());
    }

    if (getDescription())
        fprintf(stderr, "%s: ", getDescription());
}

void
HardwareComponent::initialize(Amiga *amiga)
{
    assert(this->amiga == NULL);
    assert(amiga != NULL);

    this->amiga = amiga;

    // Initialize all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->initialize(amiga);
    }

    // Initialize this component
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
        
        // Reset all non-persistant snapshot items
        _reset();

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
        debug(RUNLOOP_DEBUG, "Run\n");
        running = true;
        _run();
    }
}

void
HardwareComponent::pause()
{
    if (running) {
        
        // Pause this component
        debug(RUNLOOP_DEBUG, "Pause\n");
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
HardwareComponent::warpOn()
{
    if (warp) return;

    warp = true;

    // Enable warp mode for all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->warpOn();
    }

    // Enable warp mode for this component
    _warpOn();
}

void
HardwareComponent::warpOff()
{
    if (!warp) return;

    warp = false;

    // Disable warp mode for all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->warpOff();
    }

    // Disable warp mode for this component
    _warpOff();
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
HardwareComponent::load(uint8_t *buffer)
{
    uint8_t *ptr = buffer;

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
    assert(ptr - buffer == size());
    // panic("Loaded %d bytes (expected %d)\n", ptr - buffer, size());

    return ptr - buffer;
}

size_t
HardwareComponent::save(uint8_t *buffer)
{
    uint8_t *ptr = buffer;

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
    assert(ptr - buffer == size());
    // panic("Saved %d bytes (expected %d)\n", ptr - buffer, size());

    return ptr - buffer;
}
