// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "utl/abilities/Loggable.h"

namespace utl::channel {

/* Every log call now decides whether to print via its own debug flag
 * (see debug.h), so a single always-on output channel is all that's left
 * to register. Channel 0 is reserved by convention as a disabled sink.
 */

LogChannel NULLDEV = Loggable::subscribe("NULLDEV", std::optional<LogLevel>(std::nullopt), "Message sink");
LogChannel STDERR  = Loggable::subscribe("STDERR",  std::optional<LogLevel>(LogLevel::LOG_DEBUG), "Standard error");

}
