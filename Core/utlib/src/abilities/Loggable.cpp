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
        chns.push_back(LogChannelInfo { "NULL", 0, "Suppress output" });
        chns.push_back(LogChannelInfo { "STD",  1, "Standard messages" });
    }

    // Seek the channel
    for (LogChannel i = 0; i < LogChannel(chns.size()); ++i)
        if (chns[i].name == name) return i;

    // It it does not exist, assign a new channel number
    const LogChannel id = LogChannel(chns.size());

    // Add a new channel
    chns.push_back(LogChannelInfo{

        .verbosity       = level,
        .name        = std::move(name),
        .description = std::move(description)
    });

    return id;
}

void
Loggable::setLVerbosity(isize nr, isize verbosity)
{
    if (nr < size())
        channels()[nr].verbosity = verbosity;;
}

void
Loggable::setLVerbosity(string name, isize verbosity)
{
    for (auto c : channels())
        if (c.name == name) { c.verbosity = verbosity; return; }
}

void
Loggable::log(LogChannel c,
              LogLevel level,
              const std::source_location &loc,
              const char *fmt, ...) const
{
    auto v = c < LogChannel(channels().size()) ? channels()[c].verbosity : 0;
    if (v == 0) return;

    switch (level) {

        case LogLevel::Message:

            prefix(verbosity, loc);
            break;

        case LogLevel::Warning:

            prefix(verbosity, loc);
            fprintf(stderr, "WARNING: ");
            break;

        case LogLevel::Fatal:

            prefix(verbosity, loc);
            fprintf(stderr, " FATAL ERROR: ");
            assert(0);
            exit(1);
            break;

        case LogLevel::Debug:

            prefix(verbosity, loc);
            break;

        case LogLevel::Trace:

            tracePrefix(verbosity, loc);
            prefix(verbosity, loc);
            break;

        default:
            fatalError;
    }

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

}
