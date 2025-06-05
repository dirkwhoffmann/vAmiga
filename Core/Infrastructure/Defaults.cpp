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
    setFallback(Opt::HOST_REFRESH_RATE,          60);
    setFallback(Opt::HOST_SAMPLE_RATE,           0);
    setFallback(Opt::HOST_FRAMEBUF_WIDTH,        0);
    setFallback(Opt::HOST_FRAMEBUF_HEIGHT,       0);

    setFallback(Opt::AMIGA_VIDEO_FORMAT,         (i64)TV::PAL);
    setFallback(Opt::AMIGA_WARP_BOOT,            0);
    setFallback(Opt::AMIGA_WARP_MODE,            (i64)Warp::NEVER);
    setFallback(Opt::AMIGA_VSYNC,                false);
    setFallback(Opt::AMIGA_SPEED_BOOST,          100);
    setFallback(Opt::AMIGA_RUN_AHEAD,            0);

    setFallback(Opt::AMIGA_SNAP_AUTO,            false);
    setFallback(Opt::AMIGA_SNAP_DELAY,           10);
    setFallback(Opt::AMIGA_SNAP_COMPRESSOR,      (i64)Compressor::GZIP);
    setFallback(Opt::AMIGA_WS_COMPRESSION,       true);

    setFallback(Opt::AGNUS_REVISION,             (i64)AgnusRevision::ECS_1MB);
    setFallback(Opt::AGNUS_PTR_DROPS,            true);

    setFallback(Opt::DENISE_REVISION,            (i64)DeniseRev::OCS);
    setFallback(Opt::DENISE_VIEWPORT_TRACKING,   true);
    setFallback(Opt::DENISE_FRAME_SKIPPING,      16);

    setFallback(Opt::MON_PALETTE,                (i64)Palette::COLOR);
    setFallback(Opt::MON_BRIGHTNESS,             50);
    setFallback(Opt::MON_CONTRAST,               100);
    setFallback(Opt::MON_SATURATION,             50);
    setFallback(Opt::MON_CENTER,                 (i64)Center::AUTO);
    setFallback(Opt::MON_HCENTER,                600);
    setFallback(Opt::MON_VCENTER,                470);
    setFallback(Opt::MON_ZOOM,                   (i64)Zoom::WIDE);
    setFallback(Opt::MON_HZOOM,                  1000);
    setFallback(Opt::MON_VZOOM,                  270);
    setFallback(Opt::MON_ENHANCER,               (i64)Upscaler::NONE);
    setFallback(Opt::MON_UPSCALER,               (i64)Upscaler::NONE);
    setFallback(Opt::MON_BLUR,                   true);
    setFallback(Opt::MON_BLUR_RADIUS,            0);
    setFallback(Opt::MON_BLOOM,                  false);
    setFallback(Opt::MON_BLOOM_RADIUS,           200);
    setFallback(Opt::MON_BLOOM_BRIGHTNESS,       200);
    setFallback(Opt::MON_BLOOM_WEIGHT,           100);
    setFallback(Opt::MON_DOTMASK,                (i64)Dotmask::NONE);
    setFallback(Opt::MON_DOTMASK_BRIGHTNESS,     550);
    setFallback(Opt::MON_SCANLINES,              (i64)Scanlines::NONE);
    setFallback(Opt::MON_SCANLINE_BRIGHTNESS,    550);
    setFallback(Opt::MON_SCANLINE_WEIGHT,        110);
    setFallback(Opt::MON_DISALIGNMENT,           0);
    setFallback(Opt::MON_DISALIGNMENT_H,         250);
    setFallback(Opt::MON_DISALIGNMENT_V,         250);
    setFallback(Opt::MON_FLICKER,                true);
    setFallback(Opt::MON_FLICKER_WEIGHT,         250);

    setFallback(Opt::DMA_DEBUG_ENABLE,           false);
    setFallback(Opt::DMA_DEBUG_MODE,             (i64)DmaDisplayMode::FG_LAYER);
    setFallback(Opt::DMA_DEBUG_OPACITY,          50);
    setFallback(Opt::DMA_DEBUG_CHANNEL0,         true);
    setFallback(Opt::DMA_DEBUG_CHANNEL1,         true);
    setFallback(Opt::DMA_DEBUG_CHANNEL2,         true);
    setFallback(Opt::DMA_DEBUG_CHANNEL3,         true);
    setFallback(Opt::DMA_DEBUG_CHANNEL4,         true);
    setFallback(Opt::DMA_DEBUG_CHANNEL5,         true);
    setFallback(Opt::DMA_DEBUG_CHANNEL6,         false);
    setFallback(Opt::DMA_DEBUG_CHANNEL7,         true);
    setFallback(Opt::DMA_DEBUG_COLOR0,           0xFFFF0000);
    setFallback(Opt::DMA_DEBUG_COLOR1,           0xFFCC0000);
    setFallback(Opt::DMA_DEBUG_COLOR2,           0x00FF0000);
    setFallback(Opt::DMA_DEBUG_COLOR3,           0xFF00FF00);
    setFallback(Opt::DMA_DEBUG_COLOR4,           0x0088FF00);
    setFallback(Opt::DMA_DEBUG_COLOR5,           0x00FFFF00);
    setFallback(Opt::DMA_DEBUG_COLOR6,           0xFFFFFF00);
    setFallback(Opt::DMA_DEBUG_COLOR7,           0xFF000000);

    setFallback(Opt::LA_PROBE0,                  (i64)Probe::NONE);
    setFallback(Opt::LA_PROBE1,                  (i64)Probe::NONE);
    setFallback(Opt::LA_PROBE2,                  (i64)Probe::NONE);
    setFallback(Opt::LA_PROBE3,                  (i64)Probe::NONE);
    setFallback(Opt::LA_ADDR0,                   0);
    setFallback(Opt::LA_ADDR1,                   0);
    setFallback(Opt::LA_ADDR2,                   0);
    setFallback(Opt::LA_ADDR3,                   0);

    setFallback(Opt::VID_WHITE_NOISE,            true);

    setFallback(Opt::CPU_REVISION,               (i64)CPURev::CPU_68000);
    setFallback(Opt::CPU_DASM_REVISION,          (i64)CPURev::CPU_68000);
    setFallback(Opt::CPU_DASM_SYNTAX,            (i64)DasmSyntax::MOIRA);
    setFallback(Opt::CPU_DASM_NUMBERS,           (i64)DasmNumbers::HEX);
    setFallback(Opt::CPU_OVERCLOCKING,           0);
    setFallback(Opt::CPU_RESET_VAL,              0);

    setFallback(Opt::RTC_MODEL,                  (i64)RTCRevision::OKI);

    setFallback(Opt::MEM_CHIP_RAM,               512);
    setFallback(Opt::MEM_SLOW_RAM,               512);
    setFallback(Opt::MEM_FAST_RAM,               0);
    setFallback(Opt::MEM_EXT_START,              0xE0);
    setFallback(Opt::MEM_SAVE_ROMS,              true);
    setFallback(Opt::MEM_SLOW_RAM_DELAY,         true);
    setFallback(Opt::MEM_SLOW_RAM_MIRROR,        true);
    setFallback(Opt::MEM_BANKMAP,                (i64)BankMap::A500);
    setFallback(Opt::MEM_UNMAPPING_TYPE,         (i64)RamInitPattern::ALL_ZEROES);
    setFallback(Opt::MEM_RAM_INIT_PATTERN,       (i64)UnmappedMemory::FLOATING);

    setFallback(Opt::DC_SPEED,                   1);
    setFallback(Opt::DC_LOCK_DSKSYNC,            false);
    setFallback(Opt::DC_AUTO_DSKSYNC,            false);

    setFallback(Opt::DRIVE_CONNECT,              true,                   { 0 });
    setFallback(Opt::DRIVE_CONNECT,              false,                  { 1, 2, 3 });
    setFallback(Opt::DRIVE_TYPE,                 (i64)FloppyDriveType::DD_35, { 0, 1, 2, 3 });
    setFallback(Opt::DRIVE_MECHANICS,            (i64)DriveMechanics::A1010, { 0, 1, 2, 3 });
    setFallback(Opt::DRIVE_RPM,                  300,                    { 0, 1, 2, 3 });
    setFallback(Opt::DRIVE_SWAP_DELAY,           SEC(1.8),               { 0, 1, 2, 3 });
    setFallback(Opt::DRIVE_PAN,                  100,                    { 0, 2 });
    setFallback(Opt::DRIVE_PAN,                  300,                    { 1, 3 });
    setFallback(Opt::DRIVE_STEP_VOLUME,          50,                     { 0, 1, 2, 3 });
    setFallback(Opt::DRIVE_POLL_VOLUME,          0,                      { 0, 1, 2, 3 });
    setFallback(Opt::DRIVE_INSERT_VOLUME,        50,                     { 0, 1, 2, 3 });
    setFallback(Opt::DRIVE_EJECT_VOLUME,         50,                     { 0, 1, 2, 3 });
    setFallback(Opt::HDC_CONNECT,                true,                   { 0 });
    setFallback(Opt::HDC_CONNECT,                false,                  { 1, 2, 3 });
    setFallback(Opt::HDR_TYPE,                   (i64)HardDriveType::GENERIC, { 0, 1, 2, 3 });
    setFallback(Opt::HDR_PAN,                    300,                    { 0, 2 });
    setFallback(Opt::HDR_PAN,                    100,                    { 1, 3 });
    setFallback(Opt::HDR_STEP_VOLUME,            50,                     { 0, 1, 2, 3 });

    setFallback(Opt::SER_DEVICE,                 (i64)SerialPortDevice::NONE);
    setFallback(Opt::SER_VERBOSE,                0);

    setFallback(Opt::DENISE_HIDDEN_BITPLANES,    0);
    setFallback(Opt::DENISE_HIDDEN_SPRITES,      0);
    setFallback(Opt::DENISE_HIDDEN_LAYERS,       0);
    setFallback(Opt::DENISE_HIDDEN_LAYER_ALPHA,  128);
    setFallback(Opt::DENISE_CLX_SPR_SPR,         false);
    setFallback(Opt::DENISE_CLX_SPR_PLF,         false);
    setFallback(Opt::DENISE_CLX_PLF_PLF,         false);

    setFallback(Opt::BLITTER_ACCURACY,           2);

    setFallback(Opt::CIA_REVISION,               (i64)CIARev::MOS_8520_DIP, { 0, 1} );
    setFallback(Opt::CIA_TODBUG,                 true,                   { 0, 1} );
    setFallback(Opt::CIA_ECLOCK_SYNCING,         true,                   { 0, 1} );
    setFallback(Opt::CIA_IDLE_SLEEP,             true,                   { 0, 1} );

    setFallback(Opt::KBD_ACCURACY,               true);

    setFallback(Opt::MOUSE_PULLUP_RESISTORS,     true,                   { 0, 1} );
    setFallback(Opt::MOUSE_SHAKE_DETECTION,      true,                   { 0, 1} );
    setFallback(Opt::MOUSE_VELOCITY,             100,                    { 0, 1} );

    setFallback(Opt::JOY_AUTOFIRE,               false,                  { 0, 1} );
    setFallback(Opt::JOY_AUTOFIRE_BURSTS,        false,                  { 0, 1} );
    setFallback(Opt::JOY_AUTOFIRE_BULLETS,       3,                      { 0, 1} );
    setFallback(Opt::JOY_AUTOFIRE_DELAY,         5,                      { 0, 1} );

    setFallback(Opt::AUD_PAN0,                   50);
    setFallback(Opt::AUD_PAN1,                   350);
    setFallback(Opt::AUD_PAN2,                   350);
    setFallback(Opt::AUD_PAN3,                   50);
    setFallback(Opt::AUD_VOL0,                   100);
    setFallback(Opt::AUD_VOL1,                   100);
    setFallback(Opt::AUD_VOL2,                   100);
    setFallback(Opt::AUD_VOL3,                   100);
    setFallback(Opt::AUD_VOLL,                   50);
    setFallback(Opt::AUD_VOLR,                   50);
    setFallback(Opt::AUD_FILTER_TYPE,            (i64)FilterType::A500);
    setFallback(Opt::AUD_BUFFER_SIZE,            (i64)4096);
    setFallback(Opt::AUD_SAMPLING_METHOD,        (i64)SamplingMethod::NONE);
    setFallback(Opt::AUD_ASR,                    true);
    setFallback(Opt::AUD_FASTPATH,               true);

    setFallback(Opt::DIAG_BOARD,                 false);

    setFallback(Opt::SRV_PORT,                   8080,                   { (i64)ServerType::SER });
    setFallback(Opt::SRV_PROTOCOL,               (i64)ServerProtocol::DEFAULT, { (i64)ServerType::SER });
    setFallback(Opt::SRV_AUTORUN,                false,                  { (i64)ServerType::SER });
    setFallback(Opt::SRV_VERBOSE,                true,                   { (i64)ServerType::SER });
    setFallback(Opt::SRV_PORT,                   8081,                   { (i64)ServerType::RSH });
    setFallback(Opt::SRV_PROTOCOL,               (i64)ServerProtocol::DEFAULT, { (i64)ServerType::RSH });
    setFallback(Opt::SRV_AUTORUN,                false,                  { (i64)ServerType::RSH });
    setFallback(Opt::SRV_VERBOSE,                true,                   { (i64)ServerType::RSH });
    setFallback(Opt::SRV_PORT,                   8082,                   { (i64)ServerType::PROM });
    setFallback(Opt::SRV_PROTOCOL,               (i64)ServerProtocol::DEFAULT, { (i64)ServerType::PROM });
    setFallback(Opt::SRV_AUTORUN,                false,                  { (i64)ServerType::PROM });
    setFallback(Opt::SRV_VERBOSE,                true,                   { (i64)ServerType::PROM });
    setFallback(Opt::SRV_PORT,                   8083,                   { (i64)ServerType::GDB });
    setFallback(Opt::SRV_PROTOCOL,               (i64)ServerProtocol::DEFAULT, { (i64)ServerType::GDB });
    setFallback(Opt::SRV_AUTORUN,                false,                  { (i64)ServerType::GDB });
    setFallback(Opt::SRV_VERBOSE,                true,                   { (i64)ServerType::GDB });

    setFallback("ROM_PATH",                     "");
    setFallback("EXT_PATH",                     "");
    setFallback("HD0_PATH",                     "");
    setFallback("HD1_PATH",                     "");
    setFallback("HD2_PATH",                     "");
    setFallback("HD3_PATH",                     "");
}

void
Defaults::_dump(Category category, std::ostream &os) const
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
        throw AppError(Fault::FILE_NOT_FOUND);
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

                    warn("Ignoring invalid key %s = %s\n", key.c_str(), value.c_str());
                    skipped++;
                    continue;
                }

                // Add the key-value pair
                values[key] = value;
                accepted++;
                continue;
            }
            
            throw AppError(Fault::SYNTAX, line);
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
        throw AppError(Fault::FILE_CANT_WRITE);
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

    throw AppError(Fault::INVALID_KEY, key);
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
    if (fallbacks.contains(key)) return fallbacks.at(key);

    throw AppError(Fault::INVALID_KEY, key);
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

        if (!fallbacks.contains(key)) {

            warn("Invalid key: %s\n", key.c_str());
            assert(false);
            throw AppError(Fault::INVALID_KEY, key);
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
        fallbacks[key] = value;
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

        if (!fallbacks.contains(key)) {

            warn("Invalid key: %s\n", key.c_str());
            assert(false);
            throw AppError(Fault::INVALID_KEY, key);
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
