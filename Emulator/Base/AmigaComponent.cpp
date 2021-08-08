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

AmigaComponent::~AmigaComponent()
{
}

void
AmigaComponent::initialize()
{    
    // Initialize all subcomponents
    for (AmigaComponent *c : subComponents) {
        c->initialize();
    }

    // Initialize this component
    _initialize();
}

void
AmigaComponent::reset(bool hard)
{
    // Reset all subcomponents
    for (AmigaComponent *c : subComponents) {
        c->reset(hard);
    }
    
    // Reset this component
    debug(RUN_DEBUG, "Reset [%p]\n", this);
    _reset(hard);
}

void
AmigaComponent::configure(Option option, i64 value)
{    
    // Configure all subcomponents
    for (AmigaComponent *c : subComponents) {
        c->configure(option, value);
    }
    
    // Configure this component
    setConfigItem(option, value);
}

void
AmigaComponent::configure(Option option, long id, i64 value)
{
    // Configure all subcomponents
    for (AmigaComponent *c : subComponents) {
        c->configure(option, id, value);
    }
    
    // Configure this component
    setConfigItem(option, id, value);
}

void
AmigaComponent::inspect()
{
    // Inspect all subcomponents
    for (AmigaComponent *c : subComponents) {
        c->inspect();
    }
    
    // Inspect this component
    _inspect();
}

void AmigaComponent::dump(dump::Category category, std::ostream& ss) const
{
    _dump(category, ss);
}

void
AmigaComponent::dump(dump::Category category) const
{
    dump(category, std::cout);
}

void
AmigaComponent::dump(std::ostream& ss) const
{
    dump((dump::Category)(-1), ss);
}

void
AmigaComponent::dump() const
{
    dump((dump::Category)(-1));
}

isize
AmigaComponent::size()
{
    isize result = _size();

    for (AmigaComponent *c : subComponents) {
        result += c->size();
    }

    return result;
}

isize
AmigaComponent::load(const u8 *buffer)
{
    const u8 *ptr = buffer;
    isize result;
    
    // Call delegation method
    ptr += willLoadFromBuffer(ptr);

    // Load internal state of all subcomponents
    for (AmigaComponent *c : subComponents) {
        ptr += c->load(ptr);
    }

    // Load internal state of this component
    ptr += _load(ptr);

    // Call delegation method
    ptr += didLoadFromBuffer(ptr);
    result = (isize)(ptr - buffer);
    
    // Verify that the number of written bytes matches the snapshot size
    trace(SNP_DEBUG, "Loaded %zd bytes (expected %zd)\n", result, size());
    assert(result == size());

    return result;
}

isize
AmigaComponent::save(u8 *buffer)
{
    u8 *ptr = buffer;
    isize result;
    
    // Call delegation method
    ptr += willSaveToBuffer(ptr);

    // Save internal state of all subcomponents
    for (AmigaComponent *c : subComponents) {
        ptr += c->save(ptr);
    }

    // Save internal state of this component
    ptr += _save(ptr);

    // Call delegation method
    ptr += didSaveToBuffer(ptr);
    result = (isize)(ptr - buffer);
    
    // Verify that the number of written bytes matches the snapshot size
    trace(SNP_DEBUG, "Saved %zd bytes (expected %zd)\n", result, size());
    assert(result == size());

    return result;
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
