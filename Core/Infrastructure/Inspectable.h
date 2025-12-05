// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <iostream>

namespace vamiga {

struct Void { };

/** Inspection interface
 *
 *  The purpose of the inspection interface is to provide functions for
 *  recording portions of the emulator's current state and returning them to
 *  the caller. All components record two different kinds of information:
 *  Infos and statistics. Infos comprise the values of important variables that
 *  are used internally by the component. Examples of statistical information
 *  are the average CIA activity or the current fill level of the audio buffer.
 */
template <typename T1, typename T2 = Void>
requires std::is_default_constructible_v<T1> && std::is_default_constructible_v<T2>
class Inspectable {

protected:

    mutable T1 info = { };
    mutable T2 stats = { };

public:

    Inspectable() = default;
    virtual ~Inspectable() = default;

    const T1 &getInfo() const {

        cacheInfo(info);
        return info;
    }

    const T1 &getCachedInfo() const {

        return info;
    }

    const T2 &getStats() const {

        cacheStats(stats);
        return stats;
    }

    const T2 &getCachedStats() const {

        return stats;
    }

    virtual void clearStats() {

        stats = T2{};
    }

    virtual void record() const {

        cacheInfo(info);
        cacheStats(stats);
    }

protected:

    virtual void cacheInfo(T1 &result) const { };
    virtual void cacheStats(T2 &result) const { };
};

}
