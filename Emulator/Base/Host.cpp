// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Host.h"
#include "Emulator.h"
#include "IOUtils.h"

namespace vamiga {

i64
Host::getOption(Option option) const
{
    switch (option) {

        case OPT_HOST_REFRESH_RATE:     return i64(config.refreshRate);
        case OPT_HOST_SAMPLE_RATE:      return i64(config.sampleRate);
        case OPT_HOST_FRAMEBUF_WIDTH:   return i64(config.frameBufferWidth);
        case OPT_HOST_FRAMEBUF_HEIGHT:  return i64(config.frameBufferHeight);

        default:
            fatalError;
    }
}

void
Host::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_HOST_REFRESH_RATE:
        case OPT_HOST_SAMPLE_RATE:
        case OPT_HOST_FRAMEBUF_WIDTH:
        case OPT_HOST_FRAMEBUF_HEIGHT:
            
            return;

        default:
            throw(VAERROR_OPT_UNSUPPORTED);
    }
}

void
Host::setOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_HOST_REFRESH_RATE:

            config.refreshRate = isize(value);
            return;

        case OPT_HOST_SAMPLE_RATE:

            config.sampleRate = isize(value);
            audioPort.setSampleRate(double(value));
            return;

        case OPT_HOST_FRAMEBUF_WIDTH:

            config.frameBufferWidth = isize(value);
            return;

        case OPT_HOST_FRAMEBUF_HEIGHT:

            config.frameBufferHeight = isize(value);
            return;

        default:
            fatalError;
    }
}

void
Host::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {

        dumpConfig(os);
    }
}

fs::path
Host::tmp() const
{
    SYNCHRONIZED

    static fs::path base;

    if (base.empty()) {

        // Use /tmp as default folder for temporary files
        base = "/tmp";

        // Open a file to see if we have write permissions
        std::ofstream logfile(base / "virtualc64.log");

        // If /tmp is not accessible, use a different directory
        if (!logfile.is_open()) {

            base = fs::temp_directory_path();
            logfile.open(base / "vAmiga.log");

            if (!logfile.is_open()) {

                throw Error(VAERROR_DIR_NOT_FOUND);
            }
        }

        logfile.close();
        fs::remove(base / "vAmiga.log");
    }

    return base;
}

fs::path
Host::tmp(const string &name, bool unique) const
{
    auto base = tmp();
    auto result = base / name;

    // Make the file name unique if requested
    if (unique) result = fs::path(util::makeUniquePath(result));

    return result;
}

}
