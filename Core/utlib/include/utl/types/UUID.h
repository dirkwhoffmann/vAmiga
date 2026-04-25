// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"

namespace utl {

struct UUID {

    u64 high = 0;
    u64 low  = 0;

    static UUID v4();
    static UUID fromString(const string& str);
    string toString() const;

    UUID(const u64 high = 0, const u64 low = 0) : high(high), low(low) {}

    bool operator==(const UUID&) const = default;
    auto operator<=>(const UUID&) const = default;
};

}
