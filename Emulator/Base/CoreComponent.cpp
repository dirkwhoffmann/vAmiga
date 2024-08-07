// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CoreComponent.h"
#include "Emulator.h"
#include "Defaults.h"
#include "Checksum.h"
#include "Option.h"

namespace vamiga {

bool
CoreComponent::operator== (CoreComponent &other)
{
    return checksum(true) == other.checksum(true);
}

const char *
CoreComponent::objectName() const
{
    assert(isize(getDescriptions().size()) > objid);
    return getDescriptions().at(objid).name;
}

const char *
CoreComponent::description() const
{
    assert(isize(getDescriptions().size()) > objid);
    return getDescriptions().at(objid).description;
}

const char *
CoreComponent::shellName() const
{
    assert(isize(getDescriptions().size()) > objid);
    return getDescriptions().at(objid).shell;
}

u64
CoreComponent::checksum(bool recursive)
{
    SerChecker checker;

    // Compute a checksum for the members of this component
    *this << checker;

    // Incoorporate subcomponents if requested
    if (recursive) for (auto &c : subComponents) checker << c->checksum(recursive);

    return checker.hash;
}

bool
CoreComponent::isInitialized() const
{
    return emulator.isInitialized();
}

bool
CoreComponent::isPoweredOff() const
{
    return emulator.isPoweredOff();
}

bool
CoreComponent::isPoweredOn() const
{
    return emulator.isPoweredOn();
}

bool
CoreComponent::isPaused() const
{
    return emulator.isPaused();
}

bool
CoreComponent::isRunning() const
{
    return emulator.isRunning();
}

bool
CoreComponent::isSuspended() const
{
    return emulator.isSuspended();
}

bool
CoreComponent::isHalted() const
{
    return emulator.isHalted();
}

void
CoreComponent::suspend()
{
    return emulator.suspend();
}

void
CoreComponent::resume()
{
    return emulator.resume();
}

void
CoreComponent::isReady() const
{
    for (auto c : subComponents) { c->isReady(); }
    _isReady();
}

void
CoreComponent::resetConfig()
{
    postorderWalk([this](CoreComponent *c) {
        c->Configurable::resetConfig(emulator.defaults, c->objid);
    });
}

Configurable *
CoreComponent::routeOption(Option opt, isize objid)
{
    if (this->objid == objid) {
        for (auto &o : getOptions()) if (o == opt) return this;
    }
    for (auto &c : subComponents) {
        if (auto result = c->routeOption(opt, objid); result) return result;
    }

    return nullptr;
}

/*
void
CoreComponent::routeOption(Option opt, std::vector<Configurable *> &result)
{
    for (auto &o : getOptions()) {
        if (o == opt) result.push_back(this);
    }
    for (auto &c : subComponents) {
        c->routeOption(opt, result);
    }
}
*/

isize
CoreComponent::size()
{
    SerCounter counter;
    *this << counter;
    isize result = counter.count;

    // Add 8 bytes for the checksum
    result += 8;
    
    for (CoreComponent *c : subComponents) { result += c->size(); }
    return result;
}

isize
CoreComponent::load(const u8 *buffer)
{
    assert(!isRunning());
    
    const u8 *ptr = buffer;

    // Load internal state of all subcomponents
    for (CoreComponent *c : subComponents) {
        ptr += c->load(ptr);
    }

    // Load the checksum for this component
    auto hash = read64(ptr);

    // Load internal state of this component
    SerReader reader(ptr);
    *this << reader;
    ptr = reader.ptr;

    // Check integrity
    if (hash != checksum(false) || FORCE_SNAP_CORRUPTED) {
        throw Error(ERROR_SNAP_CORRUPTED);
    }

    isize result = (isize)(ptr - buffer);
    debug(SNP_DEBUG, "Loaded %ld bytes (expected %ld)\n", result, size());
    return result;
}

isize
CoreComponent::save(u8 *buffer)
{
    u8 *ptr = buffer;

    // Save internal state of all subcomponents
    for (CoreComponent *c : subComponents) {
        ptr += c->save(ptr);
    }

    // Save the checksum for this component
    write64(ptr, checksum(false));

    // Save the internal state of this component
    SerWriter writer(ptr);
    *this << writer;
    ptr = writer.ptr;

    isize result = (isize)(ptr - buffer);
    debug(SNP_DEBUG, "Saved %ld bytes (expected %ld)\n", result, size());
    assert(result == size());
    return result;
}

std::vector<CoreComponent *> 
CoreComponent::collectComponents()
{
    std::vector<CoreComponent *> result;
    collectComponents(result);
    return result;
}

void
CoreComponent::collectComponents(std::vector<CoreComponent *> &result)
{
    result.push_back(this);
    for (auto &c : subComponents) c->collectComponents(result);
}

void
CoreComponent::preoderWalk(std::function<void(CoreComponent *)> func)
{
    func(this);
    for (auto &c : subComponents) c->preoderWalk(func);
}

void
CoreComponent::postorderWalk(std::function<void(CoreComponent *)> func)
{
    for (auto &c : subComponents) c->postorderWalk(func);
    func(this);
}

void
CoreComponent::diff(CoreComponent &other)
{
    auto num = subComponents.size();
    assert(num == other.subComponents.size());

    // Compare all subcomponents
    for (usize i = 0; i < num; i++) {
        subComponents[i]->diff(*other.subComponents[i]);
    }

    // Compare this component
    if (auto check1 = checksum(false), check2 = other.checksum(false); check1 != check2) {
        debug(true, "Checksum mismatch: %llx != %llx\n", check1, check2);
    }
}

void
CoreComponent::exportConfig(std::ostream& ss, bool diff) const
{
    bool first = true;

    for (auto &opt: getOptions()) {

        auto current = getOption(opt);
        auto fallback = getFallback(opt);

        if (!diff || current != fallback) {

            if (first) {

                ss << "# " << description() << std::endl << std::endl;
                first = false;
            }

            auto cmd = "try " + string(shellName());
            auto currentStr = OptionParser::asPlainString(opt, current);
            auto fallbackStr = OptionParser::asPlainString(opt, fallback);

            string line = cmd + " set " + OptionEnum::plainkey(opt) + " " + currentStr;
            string comment = diff ? fallbackStr : OptionEnum::help(opt);

            ss << std::setw(40) << std::left << line << " # " << comment << std::endl;
        }
    }

    if (!first) ss << std::endl;

    for (auto &sub: subComponents) {

        sub->exportConfig(ss, diff);
    }
}

}
