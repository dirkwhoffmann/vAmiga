// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Defaults.h"
#include "Amiga.h"
#include "StringUtils.h"
#include "AgnusTypes.h"
#include "AmigaTypes.h"
#include "AudioFilterTypes.h"
#include "CIATypes.h"
#include "DeniseTypes.h"
#include "DmaDebuggerTypes.h"
#include "FloppyDriveTypes.h"
#include "HardDriveTypes.h"
#include "MemoryTypes.h"
#include "MuxerTypes.h"
#include "PixelEngineTypes.h"
#include "RTCTypes.h"
#include "SerialPortTypes.h"
#include "RemoteManagerTypes.h"
#include "RemoteServerTypes.h"

namespace vamiga {

Defaults::Defaults()
{
    setFallback(OPT_VIDEO_FORMAT, PAL);
    setFallback(OPT_WARP_BOOT, 0);
    setFallback(OPT_WARP_MODE, WARP_NEVER);
    setFallback(OPT_SYNC_MODE, SYNC_PULSED);
    setFallback(OPT_VSYNC, false);
    setFallback(OPT_TIME_LAPSE, 100);
    setFallback(OPT_TIME_SLICES, 1);
    setFallback(OPT_AGNUS_REVISION, AGNUS_ECS_1MB);
    setFallback(OPT_SLOW_RAM_MIRROR, true);
    setFallback(OPT_PTR_DROPS, true);
    setFallback(OPT_DENISE_REVISION, DENISE_OCS);
    setFallback(OPT_VIEWPORT_TRACKING, true);
    setFallback(OPT_FRAME_SKIPPING, 16);
    setFallback(OPT_PALETTE, PALETTE_COLOR);
    setFallback(OPT_BRIGHTNESS, 50);
    setFallback(OPT_CONTRAST, 100);
    setFallback(OPT_SATURATION, 50);
    setFallback(OPT_DMA_DEBUG_ENABLE, false);
    setFallback(OPT_DMA_DEBUG_MODE, DMA_DISPLAY_MODE_FG_LAYER);
    setFallback(OPT_DMA_DEBUG_OPACITY, 50);
    setFallback(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_COPPER, true);
    setFallback(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_BLITTER, true);
    setFallback(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_DISK, true);
    setFallback(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_AUDIO, true);
    setFallback(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_SPRITE, true);
    setFallback(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_BITPLANE, true);
    setFallback(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_CPU, false);
    setFallback(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_REFRESH, true);
    setFallback(OPT_DMA_DEBUG_COLOR, DMA_CHANNEL_COPPER, 0xFFFF0000);
    setFallback(OPT_DMA_DEBUG_COLOR, DMA_CHANNEL_BLITTER, 0xFFCC0000);
    setFallback(OPT_DMA_DEBUG_COLOR, DMA_CHANNEL_DISK, 0x00FF0000);
    setFallback(OPT_DMA_DEBUG_COLOR, DMA_CHANNEL_AUDIO, 0xFF00FF00);
    setFallback(OPT_DMA_DEBUG_COLOR, DMA_CHANNEL_SPRITE, 0x0088FF00);
    setFallback(OPT_DMA_DEBUG_COLOR, DMA_CHANNEL_BITPLANE, 0x00FFFF00);
    setFallback(OPT_DMA_DEBUG_COLOR, DMA_CHANNEL_CPU, 0xFFFFFF00);
    setFallback(OPT_DMA_DEBUG_COLOR, DMA_CHANNEL_REFRESH, 0xFF000000);
    setFallback(OPT_CPU_REVISION, CPU_68000);
    setFallback(OPT_CPU_DASM_REVISION, CPU_68000);
    setFallback(OPT_CPU_DASM_SYNTAX, DASM_SYNTAX_MOIRA);
    setFallback(OPT_CPU_OVERCLOCKING, 0);
    setFallback(OPT_CPU_RESET_VAL, 0);
    setFallback(OPT_RTC_MODEL, RTC_OKI);
    setFallback(OPT_CHIP_RAM, 512);
    setFallback(OPT_SLOW_RAM, 512);
    setFallback(OPT_FAST_RAM, 0);
    setFallback(OPT_EXT_START, 0xE0);
    setFallback(OPT_SAVE_ROMS, true);
    setFallback(OPT_SLOW_RAM_DELAY, true);
    setFallback(OPT_BANKMAP, BANK_MAP_A500);
    setFallback(OPT_UNMAPPING_TYPE, RAM_INIT_ALL_ZEROES);
    setFallback(OPT_RAM_INIT_PATTERN, UNMAPPED_FLOATING);
    setFallback(OPT_DRIVE_CONNECT, 0, true);
    setFallback(OPT_DRIVE_CONNECT, { 1, 2, 3 }, false);
    setFallback(OPT_DRIVE_SPEED, 1);
    setFallback(OPT_LOCK_DSKSYNC, false);
    setFallback(OPT_AUTO_DSKSYNC, false);
    setFallback(OPT_DRIVE_TYPE, { 0, 1, 2, 3 }, DRIVE_DD_35);
    setFallback(OPT_DRIVE_MECHANICS, { 0, 1, 2, 3 }, MECHANICS_A1010);
    setFallback(OPT_DRIVE_RPM, { 0, 1, 2, 3 }, 300);
    setFallback(OPT_DISK_SWAP_DELAY, { 0, 1, 2, 3 }, SEC(1.8));
    setFallback(OPT_DRIVE_PAN, { 0, 2 }, 100);
    setFallback(OPT_DRIVE_PAN, { 1, 3 }, 300);
    setFallback(OPT_STEP_VOLUME, { 0, 1, 2, 3 }, 50);
    setFallback(OPT_POLL_VOLUME, { 0, 1, 2, 3 }, 0);
    setFallback(OPT_INSERT_VOLUME, { 0, 1, 2, 3 }, 50);
    setFallback(OPT_EJECT_VOLUME, { 0, 1, 2, 3 }, 50);
    setFallback(OPT_HDC_CONNECT, 0, true);
    setFallback(OPT_HDC_CONNECT, { 1, 2, 3 }, false);
    setFallback(OPT_HDR_TYPE, { 0, 1, 2, 3 }, HDR_GENERIC);
    setFallback(OPT_HDR_PAN, { 0, 2 }, 300);
    setFallback(OPT_HDR_PAN, { 1, 3 }, 100);
    setFallback(OPT_HDR_STEP_VOLUME, { 0, 1, 2, 3 }, 50);
    setFallback(OPT_SER_DEVICE, SPD_NONE);
    setFallback(OPT_SER_VERBOSE, 0);
    setFallback(OPT_HIDDEN_BITPLANES, 0);
    setFallback(OPT_HIDDEN_SPRITES, 0);
    setFallback(OPT_HIDDEN_LAYERS, 0);
    setFallback(OPT_HIDDEN_LAYER_ALPHA, 128);
    setFallback(OPT_CLX_SPR_SPR, false);
    setFallback(OPT_CLX_SPR_PLF, false);
    setFallback(OPT_CLX_PLF_PLF, false);
    setFallback(OPT_BLITTER_ACCURACY, 2);
    setFallback(OPT_CIA_REVISION, CIA_MOS_8520_DIP);
    setFallback(OPT_TODBUG, true);
    setFallback(OPT_ECLOCK_SYNCING, true);
    setFallback(OPT_CIA_IDLE_SLEEP, true);
    setFallback(OPT_ACCURATE_KEYBOARD, true);
    setFallback(OPT_PULLUP_RESISTORS, true);
    setFallback(OPT_SHAKE_DETECTION, true);
    setFallback(OPT_MOUSE_VELOCITY, 100);
    setFallback(OPT_AUTOFIRE, false);
    setFallback(OPT_AUTOFIRE_BULLETS, -3);
    setFallback(OPT_AUTOFIRE_DELAY, 125);
    setFallback(OPT_SAMPLING_METHOD, SMP_NONE);
    setFallback(OPT_FILTER_TYPE, FILTER_A500);
    setFallback(OPT_AUDPAN, { 0, 3 }, 50);
    setFallback(OPT_AUDPAN, { 1, 2 }, 350);
    setFallback(OPT_AUDVOL, { 0, 1, 2, 3 }, 100);
    setFallback(OPT_AUDVOLL, 50);
    setFallback(OPT_AUDVOLR, 50);
    setFallback(OPT_AUD_FASTPATH, true);
    setFallback(OPT_DIAG_BOARD, false);
    setFallback(OPT_SRV_PORT, SERVER_SER, 8080);
    setFallback(OPT_SRV_PROTOCOL, SERVER_SER, SRVPROT_DEFAULT);
    setFallback(OPT_SRV_AUTORUN, SERVER_SER, true);
    setFallback(OPT_SRV_VERBOSE, SERVER_SER, true);
    setFallback(OPT_SRV_PORT, SERVER_RSH, 8081);
    setFallback(OPT_SRV_PROTOCOL, SERVER_RSH, SRVPROT_DEFAULT);
    setFallback(OPT_SRV_AUTORUN, SERVER_RSH, false);
    setFallback(OPT_SRV_VERBOSE, SERVER_RSH, true);
    setFallback(OPT_SRV_PORT, SERVER_GDB, 8082);
    setFallback(OPT_SRV_PROTOCOL, SERVER_GDB, SRVPROT_DEFAULT);
    setFallback(OPT_SRV_AUTORUN, SERVER_GDB, true);
    setFallback(OPT_SRV_VERBOSE, SERVER_GDB, true);

    setFallback("ROM_PATH", "");
    setFallback("EXT_PATH", "");
    setFallback("HD0_PATH", "");
    setFallback("HD1_PATH", "");
    setFallback("HD2_PATH", "");
    setFallback("HD3_PATH", "");
}

void
Defaults::_dump(Category category, std::ostream& os) const
{
    for (const auto &it: fallbacks) {
        
        const string key = it.first;

        if (values.contains(key)) {

            os << util::tab(key);
            os << values.at(key) << std::endl;
            
        } else {
            
            os << util::tab(key);
            os << fallbacks.at(key) << " (Default)" << std::endl;
        }
    }
}

void
Defaults::load(const fs::path &path)
{
    auto fs = std::ifstream(path, std::ifstream::binary);
    
    if (!fs.is_open()) {
        throw VAError(ERROR_FILE_NOT_FOUND);
    }
    
    debug(DEF_DEBUG, "Loading user defaults from %s...\n", path.string().c_str());
    load(fs);
}

void
Defaults::load(std::ifstream &stream)
{
    std::stringstream ss;
    ss << stream.rdbuf();
    
    load(ss);
}

void
Defaults::load(std::stringstream &stream)
{
    isize line = 0;
    isize accepted = 0;
    isize skipped = 0;
    string input;
    string section;
    
    {   SYNCHRONIZED
        
        debug(DEF_DEBUG, "Loading user defaults from string stream...\n");
        
        while(std::getline(stream, input)) {
            
            line++;
            
            // Remove white spaces
            util::trim(input);
            
            // Ignore empty lines
            if (input == "") continue;
            
            // Ignore comments
            if (input.substr(0,1) == "#") continue;
            
            // Check if this line contains a section marker
            if (input.front() == '[' && input.back() == ']') {
                
                // Extract the section name
                section = input.substr(1, input.size() - 2);
                continue;
            }
            
            // Check if this line is a key-value pair
            if (auto pos = input.find("="); pos != std::string::npos) {
                
                auto key = input.substr(0, pos);
                auto value = input.substr(pos + 1, std::string::npos);
                
                // Remove white spaces
                util::trim(key);
                util::trim(value);

                // Assemble the key
                auto delimiter = section.empty() ? "" : ".";
                key = section + delimiter + key;
                
                // Check if the key is a known key
                if (!fallbacks.contains(key)) {

                    warn("Ignoring invalid key %s\n", key.c_str());
                    skipped++;
                    continue;
                }
                
                // Add the key-value pair
                values[key] = value;
                accepted++;
                continue;
            }
            
            throw VAError(ERROR_SYNTAX, line);
        }

        if (accepted || skipped) {
            debug(DEF_DEBUG, "%ld keys accepted, %ld ignored\n", accepted, skipped);
        }
    }
}

void
Defaults::save(const fs::path &path)
{
    auto fs = std::ofstream(path, std::ofstream::binary);
    
    if (!fs.is_open()) {
        throw VAError(ERROR_FILE_CANT_WRITE);
    }
    
    save(fs);
}

void
Defaults::save(std::ofstream &stream)
{
    std::stringstream ss;
    save(ss);
    
    stream << ss.rdbuf();
}

void
Defaults::save(std::stringstream &stream)
{
    {   SYNCHRONIZED
        
        debug(DEF_DEBUG, "Saving user defaults...\n");

        std::map <string, std::map <string, string>> groups;

        // Write header
        stream << "# vAmiga " << Amiga::build() << std::endl;
        stream << "# dirkwhoffmann.github.io/vAmiga" << std::endl;
        stream << std::endl;
        
        // Iterate through all known keys
        for (const auto &it: fallbacks) {

            auto key = it.first;
            auto value = getString(key);

            // Check if the key belongs to a group
            if (auto pos = key.find('.'); pos == std::string::npos) {

                // Write ungrouped keys immediately
                stream << key << "=" << value << std::endl;
                
            } else {
                
                // Save the key temporarily
                auto prefix = key.substr(0, pos);
                auto suffix = key.substr(pos + 1, string::npos);
                groups[prefix][suffix] = value;
            }
        }

        // Write all groups
        for (const auto &[group, values]: groups) {

            stream << std::endl << "[" << group << "]" << std::endl;

            for (const auto &[key, value]: values) {

                stream << key << "=" << value << std::endl;
            }
        }
    }
}

string
Defaults::getString(const string &key)
{
    if (values.contains(key)) return values[key];
    if (fallbacks.contains(key)) return fallbacks[key];

    warn("Invalid key: %s\n", key.c_str());
    assert(false);
    throw VAError(ERROR_INVALID_KEY, key);
}

i64
Defaults::getInt(const string &key)
{
    auto value = getString(key);
    i64 result = 0;

    try {
        
        result = i64(std::stoll(value));
        debug(DEF_DEBUG, "get(%s) = %lld\n", key.c_str(), result);
        
    } catch (...) {

        warn("Can't parse value %s\n", key.c_str());
    }

    return result;
}

i64
Defaults::get(Option option)
{
    return getInt(string(OptionEnum::key(option)));
}

i64
Defaults::get(Option option, isize nr)
{
    return getInt(string(OptionEnum::key(option)) + std::to_string(nr));
}

string
Defaults::getFallback(const string &key)
{
    if (!fallbacks.contains(key)) {

        warn("Invalid key: %s\n", key.c_str());
        assert(false);
        throw VAError(ERROR_INVALID_KEY, key);
    }
    
    return fallbacks[key];
}

void
Defaults::setString(const string &key, const string &value)
{
    {   SYNCHRONIZED
        
        debug(DEF_DEBUG, "%s = %s\n", key.c_str(), value.c_str());

        if (!fallbacks.contains(key)) {

            warn("Invalid key: %s\n", key.c_str());
            assert(false);
            throw VAError(ERROR_INVALID_KEY, key);
        }
        
        values[key] = value;
    }
}

void
Defaults::set(Option option, i64 value)
{
    auto key = string(OptionEnum::key(option));
    auto val = std::to_string(value);
    
    setString(key, val);
}

void
Defaults::set(Option option, isize nr, i64 value)
{
    auto key = string(OptionEnum::key(option)) + std::to_string(nr);
    auto val = std::to_string(value);

    setString(key, val);
}

void
Defaults::set(Option option, std::vector <isize> nrs, i64 value)
{
    for (auto &nr : nrs) set(option, nr, value);
}

void
Defaults::setFallback(const string &key, const string &value)
{
    {   SYNCHRONIZED
        
        debug(DEF_DEBUG, "Fallback: %s = %s\n", key.c_str(), value.c_str());
        
        fallbacks[key] = value;
    }
}

void
Defaults::setFallback(Option option, const string &value)
{
    setFallback(string(OptionEnum::key(option)), value);
}

void
Defaults::setFallback(Option option, i64 value)
{
    setFallback(option, std::to_string(value));
}

void
Defaults::setFallback(Option option, isize nr, const string &value)
{
    setFallback(string(OptionEnum::key(option)) + std::to_string(nr), value);
}

void
Defaults::setFallback(Option option, isize nr, i64 value)
{
    setFallback(option, nr, std::to_string(value));
}

void
Defaults::setFallback(Option option, std::vector <isize> nrs, const string &value)
{
    for (auto &nr : nrs) setFallback(option, nr, value);
}

void
Defaults::setFallback(Option option, std::vector <isize> nrs, i64 value)
{
    setFallback(option, nrs, std::to_string(value));
}

void
Defaults::remove()
{
    SYNCHRONIZED values.clear();
}

void
Defaults::remove(const string &key)
{
    {   SYNCHRONIZED
        
        if (!fallbacks.contains(key)) {

            warn("Invalid key: %s\n", key.c_str());
            assert(false);
            throw VAError(ERROR_INVALID_KEY, key);
        }
        if (values.contains(key)) {
            values.erase(key);
        }
    }
}

void
Defaults::remove(Option option)
{
    remove(string(OptionEnum::key(option)));
}

void
Defaults::remove(Option option, isize nr)
{
    remove(string(OptionEnum::key(option)) + std::to_string(nr));
}

void
Defaults::remove(Option option, std::vector <isize> nrs)
{
    for (auto &nr : nrs) remove(option, nr);
}

}
