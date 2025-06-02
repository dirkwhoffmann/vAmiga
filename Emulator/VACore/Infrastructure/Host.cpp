// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "Host.h"
#include "Emulator.h"
#include "IOUtils.h"
#include "StringUtils.h"

#include <unordered_set>

namespace vamiga {

i64
Host::getOption(Opt option) const
{
    switch (option) {

        case Opt::HOST_REFRESH_RATE:     return i64(config.refreshRate);
        case Opt::HOST_SAMPLE_RATE:      return i64(config.sampleRate);
        case Opt::HOST_FRAMEBUF_WIDTH:   return i64(config.frameBufferWidth);
        case Opt::HOST_FRAMEBUF_HEIGHT:  return i64(config.frameBufferHeight);

        default:
            fatalError;
    }
}

void
Host::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::HOST_REFRESH_RATE:
        case Opt::HOST_SAMPLE_RATE:
        case Opt::HOST_FRAMEBUF_WIDTH:
        case Opt::HOST_FRAMEBUF_HEIGHT:
            
            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
Host::setOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::HOST_REFRESH_RATE:

            config.refreshRate = isize(value);
            return;

        case Opt::HOST_SAMPLE_RATE:

            config.sampleRate = isize(value);
            audioPort.setSampleRate(double(value));
            return;

        case Opt::HOST_FRAMEBUF_WIDTH:

            config.frameBufferWidth = isize(value);
            return;

        case Opt::HOST_FRAMEBUF_HEIGHT:

            config.frameBufferHeight = isize(value);
            return;

        default:
            fatalError;
    }
}

void
Host::_dump(Category category, std::ostream &os) const
{
    using namespace util;

    if (category == Category::Config) {

        os << tab("Search path");
        os << searchPath << std::endl;

        dumpConfig(os);
    }
}

fs::path
Host::sanitize(const string &filename)
{
    auto toUtf8 = [&](u8 c) {
        
        if (c < 0x80) {
            return string(1, c);
        } else {
            return string(1, char( 0xC0 | (c >> 6))) + char(0x80 | (c & 0x3F));
        }
    };
    
    /*
    auto toOctal = [&](u8 c) {
        
        std::ostringstream oss;
        oss << std::oct << std::setw(3) << std::setfill('0') << int(c);
        return oss.str();
    };
    */
    
    auto rectify = [&](u8 c) {

        // Check the standard characters first
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c < 'Z') || (c >= '0' && c < '9')) return string(1, c);
        
        // Check for the lower ASCII range
        if (c < 23) return toUtf8(c);
                
        // Check for the upper ASCII range
        if (c > 127) return toUtf8(c);
        
        // Check for invalid characters
        switch (c) {

            case '<':
            case '>':
            case ':':
            case '"':
            case '\\':
            case '/':
            case '|':
            case '?':
            case '*':
                return toUtf8(c);
                
            default:
                
                // Accept everything else
                return string(1, c);
        }
    };

    auto isReserved = [&](const string& name) {
        
        static const std::unordered_set<std::string> reserved {
            "CON", "PRN", "AUX", "NUL",
            "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
            "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
        };
        
        return reserved.count(util::uppercased(name)) > 0;
    };

    std::string result;
    
    // Convert characters one by one
    for (char c : filename) result += rectify((u8)c);
    
    // Strip trailing dots or spaces
    result = util::rtrim(result, ".");

    // Avoid reserved Windows names
    if (isReserved(result)) result = "__" + result;

    if (filename != result) {
        printf("sanitize: %s -> %s\n", filename.c_str(), result.c_str());
    }
    
    return fs::path(result);
}

string
Host::unsanitize(const fs::path &filename)
{
    const auto &s = filename.string();
    const auto len = isize(s.length());
    
    auto isUtf8 = [&](isize i) {
        
        if (i + 2 >= len) return false;
        return u8(s[i]) >= 0xC0 && (u8(s[i + 1]) & 0xC0) == 0x80;
    };
    
    auto fromUtf8 = [&](isize i) {
        
        return (u8)((((u8)s[i] & 0x1F) << 6) | ((u8)s[i + 1] & 0x3F));
    };
    
    auto isReserved = [&]() {
        
        static const std::unordered_set<std::string> reserved {
            "CON", "PRN", "AUX", "NUL",
            "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
            "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
        };
        
        if (s.rfind("__", 0) != 0) return false;
        return reserved.count(util::uppercased(s.substr(2))) > 0;
    };

    std::string result;

    if (isReserved()) {

        // Restore the original reserved word
        result = s.substr(2);
        
    } else {
        
        // Convert characters one by one
        for (isize i = 0; i < len; i++) {
            
            if (isUtf8(i)) {
                result += (char)fromUtf8(i); i += 1;
            } else {
                result += s[i];
            }
        }
    }
    
    if (filename.string() != result) {
        printf("unsanitize: %s -> %s\n", filename.string().c_str(), result.c_str());
    }
    
    return result;
}

void
Host::setSearchPath(const fs::path &path)
{
    SYNCHRONIZED
    
    searchPath = path;
}

fs::path
Host::makeAbsolute(const fs::path &path) const
{
    SYNCHRONIZED
    
    return path.is_absolute() ? path : searchPath / path;
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
        std::ofstream logfile(base / "vAmiga.log");

        // If /tmp is not accessible, use a different directory
        if (!logfile.is_open()) {

            base = fs::temp_directory_path();
            logfile.open(base / "vAmiga.log");

            if (!logfile.is_open()) {

                throw AppError(Fault::DIR_NOT_FOUND);
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
