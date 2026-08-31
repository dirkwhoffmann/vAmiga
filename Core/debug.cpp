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
#include <type_traits>

#ifndef NDEBUG

namespace vamiga {

/* The flag descriptor tables, generated from the X-macro lists in debug.h.
 * Each entry wraps its flag in a pair of captureless lambdas, so that flags
 * of different types can be read and written through a common interface.
 *
 * A debug flag declares its type, and the 'boolean' field is derived from it
 * rather than restated, so the two cannot drift apart. RetroShell offers a
 * 'true'/'false' setter for the boolean ones and a numeric argument for all
 * others (see DebuggerConsole).
 */

#define LOG_FLAG_ENTRY(name, dflt, help) \
    { #name, help, false, \
      []() -> long { return (long)name; }, \
      [](long value) { name = value; } },

#define DEBUG_FLAG_ENTRY(type, name, dflt, help) \
    { #name, help, std::is_same_v<type, bool>, \
      []() -> long { return (long)name; }, \
      [](long value) { name = (type)value; } },

const std::vector<FlagInfo> logFlags = { VA_LOG_FLAGS(LOG_FLAG_ENTRY) };
const std::vector<FlagInfo> debugFlags = { VA_DEBUG_FLAGS(DEBUG_FLAG_ENTRY) };

#undef LOG_FLAG_ENTRY
#undef DEBUG_FLAG_ENTRY

}

#endif
