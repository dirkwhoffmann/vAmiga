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
#include "utl/io.h"
#include "utl/support.h"

namespace vamiga {

Defaults::Defaults()
{

}

Defaults &
Defaults::fallbacks()
{
    static Defaults defaults;

    if (!defaults.values.empty()) return defaults;

    /*
    auto setKeyVal = [&](const string &key, const string &value) {
        defaults.values[key] = value;
    };
    */
    auto registerDefault = [&](Opt option, u64 value) {
        defaults.values[OptEnum::fullKey(option)] = std::to_string(value);
    };

    auto registerDefaults = [&](Opt option, u64 value, std::vector <isize> objids) {
        auto key = string(OptEnum::fullKey(option));
        for (auto &nr : objids)
            defaults.values[key + std::to_string(nr)] = std::to_string(value);
    };

    registerDefault(Opt::HOST_REFRESH_RATE,          60);
    registerDefault(Opt::HOST_SAMPLE_RATE,           0);
    registerDefault(Opt::HOST_FRAMEBUF_WIDTH,        0);
    registerDefault(Opt::HOST_FRAMEBUF_HEIGHT,       0);

    registerDefault(Opt::AMIGA_VIDEO_FORMAT,         (i64)TV::PAL);
    registerDefault(Opt::AMIGA_WARP_BOOT,            0);
    registerDefault(Opt::AMIGA_WARP_MODE,            (i64)Warp::NEVER);
    registerDefault(Opt::AMIGA_VSYNC,                false);
    registerDefault(Opt::AMIGA_SPEED_BOOST,          100);
    registerDefault(Opt::AMIGA_RUN_AHEAD,            0);

    registerDefault(Opt::AMIGA_WS_COMPRESSION,       true);

    registerDefault(Opt::AGNUS_REVISION,             (i64)AgnusRevision::ECS_1MB);
    registerDefault(Opt::AGNUS_PTR_DROPS,            true);

    registerDefault(Opt::DENISE_REVISION,            (i64)DeniseRev::OCS);
    registerDefault(Opt::DENISE_VIEWPORT_TRACKING,   true);
    registerDefault(Opt::DENISE_FRAME_SKIPPING,      16);

    registerDefault(Opt::MON_PALETTE,                (i64)Palette::COLOR);
    registerDefault(Opt::MON_BRIGHTNESS,             50);
    registerDefault(Opt::MON_CONTRAST,               100);
    registerDefault(Opt::MON_SATURATION,             50);
    registerDefault(Opt::MON_CENTER,                 (i64)Center::AUTO);
    registerDefault(Opt::MON_HCENTER,                600);
    registerDefault(Opt::MON_VCENTER,                470);
    registerDefault(Opt::MON_ZOOM,                   (i64)Zoom::WIDE);
    registerDefault(Opt::MON_HZOOM,                  1000);
    registerDefault(Opt::MON_VZOOM,                  270);
    registerDefault(Opt::MON_ENHANCER,               (i64)Upscaler::NONE);
    registerDefault(Opt::MON_UPSCALER,               (i64)Upscaler::NONE);
    registerDefault(Opt::MON_BLUR,                   true);
    registerDefault(Opt::MON_BLUR_RADIUS,            0);
    registerDefault(Opt::MON_BLOOM,                  false);
    registerDefault(Opt::MON_BLOOM_RADIUS,           200);
    registerDefault(Opt::MON_BLOOM_BRIGHTNESS,       200);
    registerDefault(Opt::MON_BLOOM_WEIGHT,           100);
    registerDefault(Opt::MON_DOTMASK,                (i64)Dotmask::NONE);
    registerDefault(Opt::MON_DOTMASK_BRIGHTNESS,     550);
    registerDefault(Opt::MON_SCANLINES,              (i64)Scanlines::NONE);
    registerDefault(Opt::MON_SCANLINE_BRIGHTNESS,    550);
    registerDefault(Opt::MON_SCANLINE_WEIGHT,        110);
    registerDefault(Opt::MON_DISALIGNMENT,           0);
    registerDefault(Opt::MON_DISALIGNMENT_H,         250);
    registerDefault(Opt::MON_DISALIGNMENT_V,         250);
    registerDefault(Opt::MON_FLICKER,                true);
    registerDefault(Opt::MON_FLICKER_WEIGHT,         250);

    registerDefault(Opt::DMA_DEBUG_ENABLE,           false);
    registerDefault(Opt::DMA_DEBUG_MODE,             (i64)DmaDisplayMode::FG_LAYER);
    registerDefault(Opt::DMA_DEBUG_OPACITY,          50);
    registerDefault(Opt::DMA_DEBUG_CHANNEL0,         true);
    registerDefault(Opt::DMA_DEBUG_CHANNEL1,         true);
    registerDefault(Opt::DMA_DEBUG_CHANNEL2,         true);
    registerDefault(Opt::DMA_DEBUG_CHANNEL3,         true);
    registerDefault(Opt::DMA_DEBUG_CHANNEL4,         true);
    registerDefault(Opt::DMA_DEBUG_CHANNEL5,         true);
    registerDefault(Opt::DMA_DEBUG_CHANNEL6,         false);
    registerDefault(Opt::DMA_DEBUG_CHANNEL7,         true);
    registerDefault(Opt::DMA_DEBUG_COLOR0,           0xFFFF0000);
    registerDefault(Opt::DMA_DEBUG_COLOR1,           0xFFCC0000);
    registerDefault(Opt::DMA_DEBUG_COLOR2,           0x00FF0000);
    registerDefault(Opt::DMA_DEBUG_COLOR3,           0xFF00FF00);
    registerDefault(Opt::DMA_DEBUG_COLOR4,           0x0088FF00);
    registerDefault(Opt::DMA_DEBUG_COLOR5,           0x00FFFF00);
    registerDefault(Opt::DMA_DEBUG_COLOR6,           0xFFFFFF00);
    registerDefault(Opt::DMA_DEBUG_COLOR7,           0xFF000000);

    registerDefault(Opt::LA_PROBE0,                  (i64)Probe::NONE);
    registerDefault(Opt::LA_PROBE1,                  (i64)Probe::NONE);
    registerDefault(Opt::LA_PROBE2,                  (i64)Probe::NONE);
    registerDefault(Opt::LA_PROBE3,                  (i64)Probe::NONE);
    registerDefault(Opt::LA_ADDR0,                   0);
    registerDefault(Opt::LA_ADDR1,                   0);
    registerDefault(Opt::LA_ADDR2,                   0);
    registerDefault(Opt::LA_ADDR3,                   0);

    registerDefault(Opt::VID_WHITE_NOISE,            true);

    registerDefault(Opt::CPU_REVISION,               (i64)CPURev::CPU_68000);
    registerDefault(Opt::CPU_DASM_REVISION,          (i64)CPURev::CPU_68000);
    registerDefault(Opt::CPU_DASM_SYNTAX,            (i64)DasmSyntax::MOIRA);
    registerDefault(Opt::CPU_DASM_NUMBERS,           (i64)DasmNumbers::HEX);
    registerDefault(Opt::CPU_OVERCLOCKING,           0);
    registerDefault(Opt::CPU_RESET_VAL,              0);

    registerDefault(Opt::RTC_MODEL,                  (i64)RTCRevision::OKI);

    registerDefault(Opt::MEM_CHIP_RAM,               512);
    registerDefault(Opt::MEM_SLOW_RAM,               512);
    registerDefault(Opt::MEM_FAST_RAM,               0);
    registerDefault(Opt::MEM_EXT_START,              0xE0);
    registerDefault(Opt::MEM_SAVE_ROMS,              true);
    registerDefault(Opt::MEM_SLOW_RAM_DELAY,         true);
    registerDefault(Opt::MEM_SLOW_RAM_MIRROR,        true);
    registerDefault(Opt::MEM_BANKMAP,                (i64)BankMap::A500);
    registerDefault(Opt::MEM_UNMAPPING_TYPE,         (i64)RamInitPattern::ALL_ZEROES);
    registerDefault(Opt::MEM_RAM_INIT_PATTERN,       (i64)UnmappedMemory::FLOATING);

    registerDefault(Opt::DC_SPEED,                   1);
    registerDefault(Opt::DC_LOCK_DSKSYNC,            false);
    registerDefault(Opt::DC_AUTO_DSKSYNC,            false);

    registerDefaults(Opt::DRIVE_CONNECT,              true,                   { 0 });
    registerDefaults(Opt::DRIVE_CONNECT,              false,                  { 1, 2, 3 });
    registerDefaults(Opt::DRIVE_TYPE,                 (i64)FloppyDriveType::DD_35, { 0, 1, 2, 3 });
    registerDefaults(Opt::DRIVE_MECHANICS,            (i64)DriveMechanics::A1010, { 0, 1, 2, 3 });
    registerDefaults(Opt::DRIVE_RPM,                  300,                    { 0, 1, 2, 3 });
    registerDefaults(Opt::DRIVE_SWAP_DELAY,           SEC(1.8),               { 0, 1, 2, 3 });
    registerDefaults(Opt::DRIVE_PAN,                  100,                    { 0, 2 });
    registerDefaults(Opt::DRIVE_PAN,                  300,                    { 1, 3 });
    registerDefaults(Opt::DRIVE_STEP_VOLUME,          50,                     { 0, 1, 2, 3 });
    registerDefaults(Opt::DRIVE_POLL_VOLUME,          0,                      { 0, 1, 2, 3 });
    registerDefaults(Opt::DRIVE_INSERT_VOLUME,        50,                     { 0, 1, 2, 3 });
    registerDefaults(Opt::DRIVE_EJECT_VOLUME,         50,                     { 0, 1, 2, 3 });
    registerDefaults(Opt::HDC_CONNECT,                true,                   { 0 });
    registerDefaults(Opt::HDC_CONNECT,                false,                  { 1, 2, 3 });
    registerDefaults(Opt::HDR_TYPE,                   (i64)HardDriveType::GENERIC, { 0, 1, 2, 3 });
    registerDefaults(Opt::HDR_PAN,                    300,                    { 0, 2 });
    registerDefaults(Opt::HDR_PAN,                    100,                    { 1, 3 });
    registerDefaults(Opt::HDR_STEP_VOLUME,            50,                     { 0, 1, 2, 3 });

    registerDefault(Opt::SER_DEVICE,                 (i64)SerialPortDevice::NONE);
    registerDefault(Opt::SER_VERBOSE,                0);

    registerDefault(Opt::DENISE_HIDDEN_BITPLANES,    0);
    registerDefault(Opt::DENISE_HIDDEN_SPRITES,      0);
    registerDefault(Opt::DENISE_HIDDEN_LAYERS,       0);
    registerDefault(Opt::DENISE_HIDDEN_LAYER_ALPHA,  128);
    registerDefault(Opt::DENISE_CLX_SPR_SPR,         false);
    registerDefault(Opt::DENISE_CLX_SPR_PLF,         false);
    registerDefault(Opt::DENISE_CLX_PLF_PLF,         false);

    registerDefault(Opt::BLITTER_ACCURACY,           2);

    registerDefaults(Opt::CIA_REVISION,               (i64)CIARev::MOS_8520_DIP, { 0, 1} );
    registerDefaults(Opt::CIA_TODBUG,                 true,                   { 0, 1} );
    registerDefaults(Opt::CIA_ECLOCK_SYNCING,         true,                   { 0, 1} );
    registerDefaults(Opt::CIA_IDLE_SLEEP,             true,                   { 0, 1} );

    registerDefault(Opt::KBD_ACCURACY,               true);

    registerDefaults(Opt::MOUSE_PULLUP_RESISTORS,     true,                   { 0, 1} );
    registerDefaults(Opt::MOUSE_SHAKE_DETECTION,      true,                   { 0, 1} );
    registerDefaults(Opt::MOUSE_VELOCITY,             100,                    { 0, 1} );

    registerDefaults(Opt::JOY_AUTOFIRE,               false,                  { 0, 1} );
    registerDefaults(Opt::JOY_AUTOFIRE_BURSTS,        false,                  { 0, 1} );
    registerDefaults(Opt::JOY_AUTOFIRE_BULLETS,       3,                      { 0, 1} );
    registerDefaults(Opt::JOY_AUTOFIRE_DELAY,         5,                      { 0, 1} );

    registerDefault(Opt::AUD_PAN0,                   50);
    registerDefault(Opt::AUD_PAN1,                   350);
    registerDefault(Opt::AUD_PAN2,                   350);
    registerDefault(Opt::AUD_PAN3,                   50);
    registerDefault(Opt::AUD_VOL0,                   100);
    registerDefault(Opt::AUD_VOL1,                   100);
    registerDefault(Opt::AUD_VOL2,                   100);
    registerDefault(Opt::AUD_VOL3,                   100);
    registerDefault(Opt::AUD_VOLL,                   50);
    registerDefault(Opt::AUD_VOLR,                   50);
    registerDefault(Opt::AUD_FILTER_TYPE,            (i64)FilterType::A500);
    registerDefault(Opt::AUD_BUFFER_SIZE,            4096);
    registerDefault(Opt::AUD_SAMPLING_METHOD,        (i64)SamplingMethod::NONE);
    registerDefault(Opt::AUD_ASR,                    true);
    registerDefault(Opt::AUD_FASTPATH,               true);

    registerDefault(Opt::DIAG_BOARD,                 false);

    registerDefaults(Opt::SRV_ENABLE,                 false,                  { (i64)ServerType::RSH });
    registerDefaults(Opt::SRV_PORT,                   8081,                   { (i64)ServerType::RSH });
    registerDefaults(Opt::SRV_PROTOCOL,               (i64)ServerProtocol::DEFAULT, { (i64)ServerType::RSH });
    registerDefaults(Opt::SRV_VERBOSE,                true,                   { (i64)ServerType::RSH });

    registerDefaults(Opt::SRV_ENABLE,                 false,                  { (i64)ServerType::RPC });
    registerDefaults(Opt::SRV_PORT,                   8082,                   { (i64)ServerType::RPC });
    registerDefaults(Opt::SRV_PROTOCOL,               (i64)ServerProtocol::DEFAULT, { (i64)ServerType::RPC });
    registerDefaults(Opt::SRV_VERBOSE,                true,                   { (i64)ServerType::RPC });

    registerDefaults(Opt::SRV_ENABLE,                 false,                  { (i64)ServerType::GDB });
    registerDefaults(Opt::SRV_PORT,                   8083,                   { (i64)ServerType::GDB });
    registerDefaults(Opt::SRV_PROTOCOL,               (i64)ServerProtocol::DEFAULT, { (i64)ServerType::GDB });
    registerDefaults(Opt::SRV_VERBOSE,                true,                   { (i64)ServerType::GDB });

    registerDefaults(Opt::SRV_ENABLE,                 false,                  { (i64)ServerType::PROM });
    registerDefaults(Opt::SRV_PORT,                   8084,                   { (i64)ServerType::PROM });
    registerDefaults(Opt::SRV_PROTOCOL,               (i64)ServerProtocol::DEFAULT, { (i64)ServerType::PROM });
    registerDefaults(Opt::SRV_VERBOSE,                true,                   { (i64)ServerType::PROM });

    registerDefaults(Opt::SRV_ENABLE,                 false,                  { (i64)ServerType::SER });
    registerDefaults(Opt::SRV_PORT,                   8085,                   { (i64)ServerType::SER });
    registerDefaults(Opt::SRV_PROTOCOL,               (i64)ServerProtocol::DEFAULT, { (i64)ServerType::SER });
    registerDefaults(Opt::SRV_VERBOSE,                true,                   { (i64)ServerType::SER });

    defaults.values["ROM_PATH"] = "";
    defaults.values["EXT_PATH"] = "";
    defaults.values["HD0_PATH"] = "";
    defaults.values["HD1_PATH"] = "";
    defaults.values["HD2_PATH"] = "";
    defaults.values["HD3_PATH"] = "";

    return defaults;
}

void
Defaults::_dump(Category category, std::ostream &os) const
{
    using namespace utl;

    for (const auto &it: fallbacks().values) {

        const string key = it.first;

        if (values.contains(key)) {

            os << tab(key);
            os << values.at(key) << std::endl;

        } else {

            os << tab(key);
            os << fallbacks().values.at(key) << " (Default)" << std::endl;
        }
    }
}

void
Defaults::load(const fs::path &path)
{
    auto fs = std::ifstream(path, std::ifstream::binary);

    if (!fs.is_open()) {
        throw IOError(IOError::FILE_NOT_FOUND);
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
    {   SYNCHRONIZED

        isize line = 0;
        isize accepted = 0;
        isize skipped = 0;
        string input;
        string section;

        debug(DEF_DEBUG, "Loading user defaults from string stream...\n");

        while(std::getline(stream, input)) {

            line++;

            // Remove white spaces
            utl::trim(input);

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
                utl::trim(key);
                utl::trim(value);

                // Assemble the key
                auto delimiter = section.empty() ? "" : ".";
                key = section + delimiter + key;

                // Check if the key is a known key
                if (!fallbacks().values.contains(key)) {

                    warn("Ignoring invalid key %s = %s\n", key.c_str(), value.c_str());
                    skipped++;
                    continue;
                }

                // Add the key-value pair
                values[key] = value;
                accepted++;
                continue;
            }
            
            throw CoreError(CoreError::SYNTAX, line);
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
        throw IOError(IOError::FILE_CANT_WRITE);
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
        for (const auto &it: fallbacks().values) {

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
    if (fallbacks().values.contains(key)) return fallbacks().values.at(key);

    throw CoreError(CoreError::INVALID_KEY, key);
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
Defaults::get(Opt option, isize nr) const
{
    try {

        return get(string(OptEnum::fullKey(option)) + std::to_string(nr));

    } catch (...) {

        return get(string(OptEnum::fullKey(option)));
    }
}

string
Defaults::getFallbackRaw(const string &key) const
{
    if (fallbacks().values.contains(key)) return fallbacks().values.at(key);

    throw CoreError(CoreError::INVALID_KEY, key);
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
Defaults::getFallback(Opt option, isize nr) const
{
    try {

        return getFallback(string(OptEnum::fullKey(option)) + std::to_string(nr));

    } catch (...) {

        return getFallback(string(OptEnum::fullKey(option)));
    }
}

void
Defaults::set(const string &key, const string &value)
{
    {   SYNCHRONIZED

        debug(DEF_DEBUG, "%s = %s\n", key.c_str(), value.c_str());

        if (!fallbacks().values.contains(key)) {

            warn("Invalid key: %s\n", key.c_str());
            assert(false);
            throw CoreError(CoreError::INVALID_KEY, key);
        }

        values[key] = value;
    }
}

void
Defaults::set(Opt option, const string &value)
{
    set(OptEnum::fullKey(option), value);
}

void
Defaults::set(Opt option, const string &value, std::vector <isize> objids)
{
    auto key = string(OptEnum::fullKey(option));

    for (auto &nr : objids) {
        set(key + std::to_string(nr), value);
    }
}

void
Defaults::set(Opt option, i64 value)
{
    set(option, std::to_string(value));
}

void
Defaults::set(Opt option, i64 value, std::vector <isize> objids)
{
    set(option, std::to_string(value), objids);
}

void
Defaults::setFallback(const string &key, const string &value)
{
    {   SYNCHRONIZED

        debug(DEF_DEBUG, "Fallback: %s = %s\n", key.c_str(), value.c_str());
        fallbacks().values[key] = value;
    }
}

void
Defaults::setFallback(Opt option, const string &value)
{
    setFallback(OptEnum::fullKey(option), value);
}

void
Defaults::setFallback(Opt option, const string &value, std::vector <isize> objids)
{
    auto key = string(OptEnum::fullKey(option));

    for (auto &nr : objids) {
        setFallback(key + std::to_string(nr), value);
    }
}

void
Defaults::setFallback(Opt option, i64 value)
{
    setFallback(option, std::to_string(value));
}

void
Defaults::setFallback(Opt option, i64 value, std::vector <isize> objids)
{
    setFallback(option, std::to_string(value), objids);
}

void
Defaults::remove()
{
    {   SYNCHRONIZED

        values.clear();
    }
}

void
Defaults::remove(const string &key)
{
    {   SYNCHRONIZED

        if (!fallbacks().values.contains(key)) {

            warn("Invalid key: %s\n", key.c_str());
            assert(false);
            throw CoreError(CoreError::INVALID_KEY, key);
        }
        if (values.contains(key)) {
            values.erase(key);
        }
    }
}

void
Defaults::remove(Opt option)
{
    remove(string(OptEnum::fullKey(option)));
}

void
Defaults::remove(Opt option, std::vector <isize> nrs)
{
    for (auto &nr : nrs) {
        remove(string(OptEnum::fullKey(option)) + std::to_string(nr));
    }
}

}
