// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "utl/abilities/Loggable.h"

namespace utl {

long Loggable::verbosity = 3;

std::vector<LogChannelInfo> &
Loggable::channels()
{
    static std::vector<LogChannelInfo> v;
    return v;
}

LogChannel
Loggable::subscribe(string name, isize level, string description)
{
    auto &chns = channels();

    // Register default channels if not present yet
    if (chns.empty()) {

        // Reserve some space to speed up further registrations
        chns.reserve(64);

        // Register the default channels
        chns.push_back(LogChannelInfo { "LOG",    1, "Messages" });
        chns.push_back(LogChannelInfo { "WARN",   1, "Warnings" });
        chns.push_back(LogChannelInfo { "ERROR",  1, "Errors" });
        chns.push_back(LogChannelInfo { "XFILES", 1, "Paranormal activity" });
    }

    // Seek the channel
    for (LogChannel i = 0; i < LogChannel(chns.size()); ++i)
        if (chns[i].name == name) return i;

    // It it does not exist, assign a new channel number
    const LogChannel id = LogChannel(chns.size());

    // Add a new channel
    chns.push_back(LogChannelInfo{

        .level       = level,
        .name        = std::move(name),
        .description = std::move(description)
    });

    return id;
}

void
Loggable::log(LogChannel c, const std::source_location &loc, const char *fmt, ...) const
{
    if (c && (c >= LogChannel(channels().size()) || channels()[c].level) && verbosity) {

        prefix(verbosity, loc);

        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
}

void
Loggable::traceLog(LogChannel c, const std::source_location &loc, const char *fmt, ...) const
{
    if (c && (c >= LogChannel(channels().size()) || channels()[c].level) && verbosity) {

        tracePrefix(verbosity, loc);
        prefix(verbosity, loc);

        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
}

}
