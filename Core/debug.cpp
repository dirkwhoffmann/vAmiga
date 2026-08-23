// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "debug.h"

#ifndef NDEBUG

namespace vamiga {

/* The flag descriptor tables, generated from the X-macro lists in debug.h.
 * Each entry wraps its flag in a pair of captureless lambdas, so that flags
 * of different types can be read and written through a common interface.
 */

#define LOG_FLAG_ENTRY(name, dflt, help) \
    { #name, help, false, \
      []() -> long { return (long)name; }, \
      [](long value) { name = LogLevel(value); } },

#define DEBUG_FLAG_ENTRY(name, dflt, help) \
    { #name, help, true, \
      []() -> long { return (long)name; }, \
      [](long value) { name = (bool)value; } },

const std::vector<FlagInfo> logFlags = { LOG_FLAGS(LOG_FLAG_ENTRY) };
const std::vector<FlagInfo> debugFlags = { DEBUG_FLAGS(DEBUG_FLAG_ENTRY) };

#undef LOG_FLAG_ENTRY
#undef DEBUG_FLAG_ENTRY

}

#endif
