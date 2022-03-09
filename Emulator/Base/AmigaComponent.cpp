// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AmigaComponent.h"
#include "Checksum.h"
#include "Serialization.h"

void
AmigaComponent::initialize()
{    
    for (AmigaComponent *c : subComponents) { c->initialize(); }
    _initialize();
}

void
AmigaComponent::reset(bool hard)
{
    for (AmigaComponent *c : subComponents) { c->reset(hard); }
    _reset(hard);
}

void
AmigaComponent::inspect() const
{
    for (AmigaComponent *c : subComponents) { c->inspect(); }
    _inspect();
}

isize
AmigaComponent::size()
{
    isize result = _size();
    
    // Add 8 bytes for the checksum
    result += 8;
    
    for (AmigaComponent *c : subComponents) { result += c->size(); }
    return result;
}

u64
AmigaComponent::checksum()
{
    u64 result = _checksum();
    
    // Compute checksums for all subcomponents
    for (AmigaComponent *c : subComponents) {
        result = util::fnvIt64(result, c->checksum());
    }
    
    return result;
}

isize
AmigaComponent::load(const u8 *buffer)
{
    assert(!isRunning());
    
    const u8 *ptr = buffer;
    
    // Call the delegate
    ptr += willLoadFromBuffer(ptr);

    // Load internal state of all subcomponents
    for (AmigaComponent *c : subComponents) {
        ptr += c->load(ptr);
    }

    // Load the checksum for this component
    auto hash = util::read64(ptr);

    // Load internal state of this component
    ptr += _load(ptr);

    // Call the delegate
    ptr += didLoadFromBuffer(ptr);
    isize result = (isize)(ptr - buffer);

    // Check integrity
    if (hash != _checksum() || FORCE_SNAP_CORRUPTED) {        
        throw VAError(ERROR_SNAP_CORRUPTED);
    }
    
    debug(SNP_DEBUG, "Loaded %ld bytes (expected %ld)\n", result, size());
    return result;
}

void
AmigaComponent::didLoad()
{
    assert(!isRunning());
        
    for (AmigaComponent *c : subComponents) {
        c->didLoad();
    }

    _didLoad();
}

isize
AmigaComponent::save(u8 *buffer)
{
    u8 *ptr = buffer;
    
    // Call delegation method
    ptr += willSaveToBuffer(ptr);
    
    // Save internal state of all subcomponents
    for (AmigaComponent *c : subComponents) {
        ptr += c->save(ptr);
    }

    // Save the checksum for this component
    util::write64(ptr, _checksum());
    
    // Save the internal state of this component
    ptr += _save(ptr);

    // Call delegation method
    ptr += didSaveToBuffer(ptr);
    isize result = (isize)(ptr - buffer);
    
    debug(SNP_DEBUG, "Saved %ld bytes (expected %ld)\n", result, size());
    assert(result == size());

    return result;
}

void
AmigaComponent::didSave()
{        
    for (AmigaComponent *c : subComponents) {
        c->didSave();
    }

    _didSave();
}

void
AmigaComponent::isReady() const
{
    for (auto c : subComponents) { c->isReady(); }
    _isReady();
}

void
AmigaComponent::powerOn()
{
    for (auto c : subComponents) { c->powerOn(); }
    _powerOn();
}
    
void
AmigaComponent::powerOff()
{
    for (auto c : subComponents) { c->powerOff(); }
    _powerOff();
}

void
AmigaComponent::run()
{
    for (auto c : subComponents) { c->run(); }
    _run();
}

void
AmigaComponent::pause()
{
    for (auto c : subComponents) { c->pause(); }
    _pause();
}

void
AmigaComponent::halt()
{
    for (auto c : subComponents) { c->halt(); }
    _halt();
}

void
AmigaComponent::warpOn()
{
    for (auto c : subComponents) { c->warpOn(); }
    _warpOn();
}

void
AmigaComponent::warpOff()
{
    for (auto c : subComponents) { c->warpOff(); }
    _warpOff();
}

void
AmigaComponent::debugOn()
{    
    for (auto c : subComponents) { c->debugOn(); }
    _debugOn();
}

void
AmigaComponent::debugOff()
{
    for (auto c : subComponents) { c->debugOff(); }
    _debugOff();
}
