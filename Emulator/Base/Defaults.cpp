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
#include "IOUtils.h"

namespace vamiga {

Defaults::Defaults()
{
    setFallback(OPT_HOST_REFRESH_RATE,          60);
    setFallback(OPT_HOST_SAMPLE_RATE,           0);
    setFallback(OPT_HOST_FRAMEBUF_WIDTH,        0);
    setFallback(OPT_HOST_FRAMEBUF_HEIGHT,       0);
    
    setFallback(OPT_AMIGA_VIDEO_FORMAT,         PAL);
    setFallback(OPT_AMIGA_WARP_BOOT,            0);
    setFallback(OPT_AMIGA_WARP_MODE,            WARP_NEVER);
    setFallback(OPT_AMIGA_VSYNC,                false);
    setFallback(OPT_AMIGA_SPEED_BOOST,          100);
    setFallback(OPT_AMIGA_RUN_AHEAD,            0);

    setFallback(OPT_AMIGA_SNAP_AUTO,            false);
    setFallback(OPT_AMIGA_SNAP_DELAY,           10);
    setFallback(OPT_AMIGA_SNAP_COMPRESS,        true);

    setFallback(OPT_AGNUS_REVISION,             AGNUS_ECS_1MB);
    setFallback(OPT_AGNUS_PTR_DROPS,            true);
    
    setFallback(OPT_DENISE_REVISION,            DENISE_OCS);
    setFallback(OPT_DENISE_VIEWPORT_TRACKING,   true);
    setFallback(OPT_DENISE_FRAME_SKIPPING,      16);

    setFallback(OPT_MON_PALETTE,                PALETTE_COLOR);
    setFallback(OPT_MON_BRIGHTNESS,             50);
    setFallback(OPT_MON_CONTRAST,               100);
    setFallback(OPT_MON_SATURATION,             50);

    setFallback(OPT_DMA_DEBUG_ENABLE,           false);
    setFallback(OPT_DMA_DEBUG_MODE,             DMA_DISPLAY_MODE_FG_LAYER);
    setFallback(OPT_DMA_DEBUG_OPACITY,          50);
    setFallback(OPT_DMA_DEBUG_CHANNEL0,         true);
    setFallback(OPT_DMA_DEBUG_CHANNEL1,         true);
    setFallback(OPT_DMA_DEBUG_CHANNEL2,         true);
    setFallback(OPT_DMA_DEBUG_CHANNEL3,         true);
    setFallback(OPT_DMA_DEBUG_CHANNEL4,         true);
    setFallback(OPT_DMA_DEBUG_CHANNEL5,         true);
    setFallback(OPT_DMA_DEBUG_CHANNEL6,         false);
    setFallback(OPT_DMA_DEBUG_CHANNEL7,         true);
    setFallback(OPT_DMA_DEBUG_COLOR0,           0xFFFF0000);
    setFallback(OPT_DMA_DEBUG_COLOR1,           0xFFCC0000);
    setFallback(OPT_DMA_DEBUG_COLOR2,           0x00FF0000);
    setFallback(OPT_DMA_DEBUG_COLOR3,           0xFF00FF00);
    setFallback(OPT_DMA_DEBUG_COLOR4,           0x0088FF00);
    setFallback(OPT_DMA_DEBUG_COLOR5,           0x00FFFF00);
    setFallback(OPT_DMA_DEBUG_COLOR6,           0xFFFFFF00);
    setFallback(OPT_DMA_DEBUG_COLOR7,           0xFF000000);

    setFallback(OPT_VID_WHITE_NOISE,            true);

    setFallback(OPT_CPU_REVISION,               CPU_68000);
    setFallback(OPT_CPU_DASM_REVISION,          CPU_68000);
    setFallback(OPT_CPU_DASM_SYNTAX,            DASM_SYNTAX_MOIRA);
    setFallback(OPT_CPU_OVERCLOCKING,           0);
    setFallback(OPT_CPU_RESET_VAL,              0);

    setFallback(OPT_RTC_MODEL,                  RTC_OKI);

    setFallback(OPT_MEM_CHIP_RAM,               512);
    setFallback(OPT_MEM_SLOW_RAM,               512);
    setFallback(OPT_MEM_FAST_RAM,               0);
    setFallback(OPT_MEM_EXT_START,              0xE0);
    setFallback(OPT_MEM_SAVE_ROMS,              true);
    setFallback(OPT_MEM_SLOW_RAM_DELAY,         true);
    setFallback(OPT_MEM_SLOW_RAM_MIRROR,        true);
    setFallback(OPT_MEM_BANKMAP,                BANK_MAP_A500);
    setFallback(OPT_MEM_UNMAPPING_TYPE,         RAM_INIT_ALL_ZEROES);
    setFallback(OPT_MEM_RAM_INIT_PATTERN,       UNMAPPED_FLOATING);

    setFallback(OPT_DC_SPEED,                   1);
    setFallback(OPT_DC_LOCK_DSKSYNC,            false);
    setFallback(OPT_DC_AUTO_DSKSYNC,            false);

    setFallback(OPT_DRIVE_CONNECT,              true,                   { 0 });
    setFallback(OPT_DRIVE_CONNECT,              false,                  { 1, 2, 3 });
    setFallback(OPT_DRIVE_TYPE,                 DRIVE_DD_35,            { 0, 1, 2, 3 });
    setFallback(OPT_DRIVE_MECHANICS,            MECHANICS_A1010,        { 0, 1, 2, 3 });
    setFallback(OPT_DRIVE_RPM,                  300,                    { 0, 1, 2, 3 });
    setFallback(OPT_DRIVE_SWAP_DELAY,           SEC(1.8),               { 0, 1, 2, 3 });
    setFallback(OPT_DRIVE_PAN,                  100,                    { 0, 2 });
    setFallback(OPT_DRIVE_PAN,                  300,                    { 1, 3 });
    setFallback(OPT_DRIVE_STEP_VOLUME,          50,                     { 0, 1, 2, 3 });
    setFallback(OPT_DRIVE_POLL_VOLUME,          0,                      { 0, 1, 2, 3 });
    setFallback(OPT_DRIVE_INSERT_VOLUME,        50,                     { 0, 1, 2, 3 });
    setFallback(OPT_DRIVE_EJECT_VOLUME,         50,                     { 0, 1, 2, 3 });
    setFallback(OPT_HDC_CONNECT,                true,                   { 0 });
    setFallback(OPT_HDC_CONNECT,                false,                  { 1, 2, 3 });
    setFallback(OPT_HDR_TYPE,                   HDR_GENERIC,            { 0, 1, 2, 3 });
    setFallback(OPT_HDR_WRITE_THROUGH,          false,                  { 0, 1, 2, 3 });
    setFallback(OPT_HDR_PAN,                    300,                    { 0, 2 });
    setFallback(OPT_HDR_PAN,                    100,                    { 1, 3 });
    setFallback(OPT_HDR_STEP_VOLUME,            50,                     { 0, 1, 2, 3 });

    setFallback(OPT_SER_DEVICE,                 SPD_NONE);
    setFallback(OPT_SER_VERBOSE,                0);

    setFallback(OPT_DENISE_HIDDEN_BITPLANES,    0);
    setFallback(OPT_DENISE_HIDDEN_SPRITES,      0);
    setFallback(OPT_DENISE_HIDDEN_LAYERS,       0);
    setFallback(OPT_DENISE_HIDDEN_LAYER_ALPHA,  128);
    setFallback(OPT_DENISE_CLX_SPR_SPR,         false);
    setFallback(OPT_DENISE_CLX_SPR_PLF,         false);
    setFallback(OPT_DENISE_CLX_PLF_PLF,         false);

    setFallback(OPT_BLITTER_ACCURACY,           2);

    setFallback(OPT_CIA_REVISION,               CIA_MOS_8520_DIP,       { 0, 1} );
    setFallback(OPT_CIA_TODBUG,                 true,                   { 0, 1} );
    setFallback(OPT_CIA_ECLOCK_SYNCING,         true,                   { 0, 1} );
    setFallback(OPT_CIA_IDLE_SLEEP,             true,                   { 0, 1} );

    setFallback(OPT_KBD_ACCURACY,               true);

    setFallback(OPT_MOUSE_PULLUP_RESISTORS,     true,                   { 0, 1} );
    setFallback(OPT_MOUSE_SHAKE_DETECTION,      true,                   { 0, 1} );
    setFallback(OPT_MOUSE_VELOCITY,             100,                    { 0, 1} );

    setFallback(OPT_JOY_AUTOFIRE,               false,                  { 0, 1} );
    setFallback(OPT_JOY_AUTOFIRE_BURSTS,        false,                  { 0, 1} );
    setFallback(OPT_JOY_AUTOFIRE_BULLETS,       3,                      { 0, 1} );
    setFallback(OPT_JOY_AUTOFIRE_DELAY,         5,                      { 0, 1} );
    setFallback(OPT_AUD_SAMPLING_METHOD,        SMP_NONE);
    setFallback(OPT_AUD_FILTER_TYPE,            FILTER_A500);
    setFallback(OPT_AUD_PAN0,                   50);
    setFallback(OPT_AUD_PAN1,                   350);
    setFallback(OPT_AUD_PAN2,                   350);
    setFallback(OPT_AUD_PAN3,                   50);
    setFallback(OPT_AUD_VOL0,                   100);
    setFallback(OPT_AUD_VOL1,                   100);
    setFallback(OPT_AUD_VOL2,                   100);
    setFallback(OPT_AUD_VOL3,                   100);
    setFallback(OPT_AUD_VOLL,                   50);
    setFallback(OPT_AUD_VOLR,                   50);
    setFallback(OPT_AUD_FASTPATH,               true);
    setFallback(OPT_DIAG_BOARD,                 false);

    setFallback(OPT_SRV_PORT,                   8080,                   { SERVER_SER });
    setFallback(OPT_SRV_PROTOCOL,               SRVPROT_DEFAULT,        { SERVER_SER });
    setFallback(OPT_SRV_AUTORUN,                false,                  { SERVER_SER });
    setFallback(OPT_SRV_VERBOSE,                true,                   { SERVER_SER });
    setFallback(OPT_SRV_PORT,                   8081,                   { SERVER_RSH });
    setFallback(OPT_SRV_PROTOCOL,               SRVPROT_DEFAULT,        { SERVER_RSH });
    setFallback(OPT_SRV_AUTORUN,                false,                  { SERVER_RSH });
    setFallback(OPT_SRV_VERBOSE,                true,                   { SERVER_RSH });
    setFallback(OPT_SRV_PORT,                   8082,                   { SERVER_GDB });
    setFallback(OPT_SRV_PROTOCOL,               SRVPROT_DEFAULT,        { SERVER_GDB });
    setFallback(OPT_SRV_AUTORUN,                false,                  { SERVER_GDB });
    setFallback(OPT_SRV_VERBOSE,                true,                   { SERVER_GDB });

    setFallback("ROM_PATH",                     "");
    setFallback("EXT_PATH",                     "");
    setFallback("HD0_PATH",                     "");
    setFallback("HD1_PATH",                     "");
    setFallback("HD2_PATH",                     "");
    setFallback("HD3_PATH",                     "");
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
        throw Error(VAERROR_FILE_NOT_FOUND);
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
            
            throw Error(VAERROR_SYNTAX, line);
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
        throw Error(VAERROR_FILE_CANT_WRITE);
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
            auto value = getRaw(key);

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
Defaults::getRaw(const string &key) const
{
    if (values.contains(key)) return values.at(key);
    if (fallbacks.contains(key)) return fallbacks.at(key);

    throw Error(VAERROR_INVALID_KEY, key);
}

i64
Defaults::get(const string &key) const
{
    auto value = getRaw(key);

    try {

        return i64(std::stoll(value));

    } catch (...) {

        warn("Can't parse value %s\n", key.c_str());
        return 0;
    }
}

i64
Defaults::get(Option option, isize nr) const
{
    try {

        return get(string(OptionEnum::rawkey(option)) + std::to_string(nr));

    } catch (...) {

        return get(string(OptionEnum::rawkey(option)));
    }
}

string
Defaults::getFallbackRaw(const string &key) const
{
    if (fallbacks.contains(key)) return fallbacks.at(key);

    throw Error(VAERROR_INVALID_KEY, key);
}

i64
Defaults::getFallback(const string &key) const
{
    auto value = getFallbackRaw(key);

    try {

        return i64(std::stoll(value));

    } catch (...) {

        warn("Can't parse value %s\n", key.c_str());
        return 0;
    }
}

i64
Defaults::getFallback(Option option, isize nr) const
{
    try {

        return getFallback(string(OptionEnum::rawkey(option)) + std::to_string(nr));

    } catch (...) {

        return getFallback(string(OptionEnum::rawkey(option)));
    }
}

void
Defaults::set(const string &key, const string &value)
{
    {   SYNCHRONIZED

        debug(DEF_DEBUG, "%s = %s\n", key.c_str(), value.c_str());

        if (!fallbacks.contains(key)) {

            warn("Invalid key: %s\n", key.c_str());
            assert(false);
            throw Error(VAERROR_INVALID_KEY, key);
        }

        values[key] = value;
    }
}

void
Defaults::set(Option option, const string &value)
{
    set(OptionEnum::rawkey(option), value);
}

void
Defaults::set(Option option, const string &value, std::vector <isize> objids)
{
    auto key = string(OptionEnum::rawkey(option));

    for (auto &nr : objids) {
        set(key + std::to_string(nr), value);
    }
}

void
Defaults::set(Option option, i64 value)
{
    set(option, std::to_string(value));
}

void
Defaults::set(Option option, i64 value, std::vector <isize> objids)
{
    set(option, std::to_string(value), objids);
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
    setFallback(OptionEnum::rawkey(option), value);
}

void
Defaults::setFallback(Option option, const string &value, std::vector <isize> objids)
{
    auto key = string(OptionEnum::rawkey(option));

    for (auto &nr : objids) {
        setFallback(key + std::to_string(nr), value);
    }
}

void
Defaults::setFallback(Option option, i64 value)
{
    setFallback(option, std::to_string(value));
}

void
Defaults::setFallback(Option option, i64 value, std::vector <isize> objids)
{
    setFallback(option, std::to_string(value), objids);
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
            throw Error(VAERROR_INVALID_KEY, key);
        }
        if (values.contains(key)) {
            values.erase(key);
        }
    }
}

void
Defaults::remove(Option option)
{
    remove(string(OptionEnum::rawkey(option)));
}

void
Defaults::remove(Option option, std::vector <isize> nrs)
{
    for (auto &nr : nrs) {
        remove(string(OptionEnum::rawkey(option)) + std::to_string(nr));
    }
}

}
