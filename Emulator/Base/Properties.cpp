// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Properties.h"
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

Properties::Properties()
{
    setDefaultValue(OPT_AGNUS_REVISION, AGNUS_ECS_1MB);
    setDefaultValue(OPT_SLOW_RAM_MIRROR, true);
    setDefaultValue(OPT_DENISE_REVISION, DENISE_OCS);
    setDefaultValue(OPT_VIEWPORT_TRACKING, true);
    setDefaultValue(OPT_PALETTE, PALETTE_COLOR);
    setDefaultValue(OPT_BRIGHTNESS, 50);
    setDefaultValue(OPT_CONTRAST, 100);
    setDefaultValue(OPT_SATURATION, 50);
    setDefaultValue(OPT_DMA_DEBUG_ENABLE, false);
    setDefaultValue(OPT_DMA_DEBUG_MODE, DMA_DISPLAY_MODE_FG_LAYER);
    setDefaultValue(OPT_DMA_DEBUG_OPACITY, 50);
    setDefaultValue(OPT_REG_RESET_VAL, 0);
    setDefaultValue(OPT_RTC_MODEL, RTC_OKI);
    setDefaultValue(OPT_CHIP_RAM, 512);
    setDefaultValue(OPT_SLOW_RAM, 512);
    setDefaultValue(OPT_FAST_RAM, 0);
    setDefaultValue(OPT_EXT_START, 0xE0);
    setDefaultValue(OPT_SAVE_ROMS, true);
    setDefaultValue(OPT_SLOW_RAM_DELAY, true);
    setDefaultValue(OPT_BANKMAP, BANK_MAP_A500);
    setDefaultValue(OPT_UNMAPPING_TYPE, RAM_INIT_ALL_ZEROES);
    setDefaultValue(OPT_RAM_INIT_PATTERN, UNMAPPED_FLOATING);
    setDefaultValue(OPT_DRIVE_CONNECT, 0, true);
    setDefaultValue(OPT_DRIVE_CONNECT, { 1, 2, 3 }, false);
    setDefaultValue(OPT_DRIVE_SPEED, 1);
    setDefaultValue(OPT_LOCK_DSKSYNC, false);
    setDefaultValue(OPT_AUTO_DSKSYNC, false);
    setDefaultValue(OPT_DRIVE_TYPE, { 0, 1, 2, 3 }, DRIVE_DD_35);
    setDefaultValue(OPT_EMULATE_MECHANICS, { 0, 1, 2, 3 }, true);
    setDefaultValue(OPT_START_DELAY, { 0, 1, 2, 3 }, MSEC(380));
    setDefaultValue(OPT_STOP_DELAY, { 0, 1, 2, 3 }, MSEC(80));
    setDefaultValue(OPT_STEP_DELAY, { 0, 1, 2, 3 }, USEC(8000));
    setDefaultValue(OPT_DISK_SWAP_DELAY, { 0, 1, 2, 3 }, SEC(1.8));
    setDefaultValue(OPT_DRIVE_PAN, { 0, 1 }, 100);
    setDefaultValue(OPT_DRIVE_PAN, { 2, 3 }, -100);
    setDefaultValue(OPT_STEP_VOLUME, { 0, 1, 2, 3 }, 128);
    setDefaultValue(OPT_POLL_VOLUME, { 0, 1, 2, 3 }, 128);
    setDefaultValue(OPT_INSERT_VOLUME, { 0, 1, 2, 3 }, 128);
    setDefaultValue(OPT_EJECT_VOLUME, { 0, 1, 2, 3 }, 128);
    setDefaultValue(OPT_HDC_CONNECT, 0, true);
    setDefaultValue(OPT_HDC_CONNECT, { 1, 2, 3 }, true);
    setDefaultValue(OPT_HDR_TYPE, HDR_GENERIC);
    setDefaultValue(OPT_HDR_PAN, { 0, 1 }, 100);
    setDefaultValue(OPT_HDR_PAN, { 2, 3 }, -100);
    setDefaultValue(OPT_HDR_STEP_VOLUME, { 0, 1, 2, 3 }, 128);
    setDefaultValue(OPT_SERIAL_DEVICE, SPD_NONE);
    setDefaultValue(OPT_HIDDEN_BITPLANES, 0);
    setDefaultValue(OPT_HIDDEN_SPRITES, 0);
    setDefaultValue(OPT_HIDDEN_LAYERS, 0);
    setDefaultValue(OPT_HIDDEN_LAYER_ALPHA, 128);
    setDefaultValue(OPT_CLX_SPR_SPR, true);
    setDefaultValue(OPT_CLX_SPR_PLF, true);
    setDefaultValue(OPT_CLX_PLF_PLF, true);
    setDefaultValue(OPT_BLITTER_ACCURACY, 2);
    setDefaultValue(OPT_CIA_REVISION, CIA_MOS_8520_DIP);
    setDefaultValue(OPT_TODBUG, true);
    setDefaultValue(OPT_ECLOCK_SYNCING, true);
    setDefaultValue(OPT_ACCURATE_KEYBOARD, true);
    setDefaultValue(OPT_PULLUP_RESISTORS, true);
    setDefaultValue(OPT_SHAKE_DETECTION, true);
    setDefaultValue(OPT_MOUSE_VELOCITY, 100);
    setDefaultValue(OPT_AUTOFIRE, false);
    setDefaultValue(OPT_AUTOFIRE_BULLETS, -3);
    setDefaultValue(OPT_AUTOFIRE_DELAY, 125);
    setDefaultValue(OPT_SAMPLING_METHOD, SMP_NONE);
    setDefaultValue(OPT_FILTER_TYPE, FILTER_BUTTERWORTH);
    setDefaultValue(OPT_FILTER_ALWAYS_ON, false);
    setDefaultValue(OPT_AUDPAN, { 0, 3 }, 50);
    setDefaultValue(OPT_AUDPAN, { 1, 2 }, 350);
    setDefaultValue(OPT_AUDVOL, { 0, 1, 2, 3 }, 100);
    setDefaultValue(OPT_AUDVOLL, 50);
    setDefaultValue(OPT_AUDVOLR, 50);
    setDefaultValue(OPT_DIAG_BOARD, false);
    setDefaultValue(OPT_SRV_PORT, SERVER_SER, 8080);
    setDefaultValue(OPT_SRV_PROTOCOL, SERVER_SER, SRVPROT_DEFAULT);
    setDefaultValue(OPT_SRV_AUTORUN, SERVER_SER, true);
    setDefaultValue(OPT_SRV_VERBOSE, SERVER_SER, true);
    setDefaultValue(OPT_SRV_PORT, SERVER_RSH, 8081);
    setDefaultValue(OPT_SRV_PROTOCOL, SERVER_RSH, SRVPROT_DEFAULT);
    setDefaultValue(OPT_SRV_AUTORUN, SERVER_RSH, false);
    setDefaultValue(OPT_SRV_VERBOSE, SERVER_RSH, true);
    setDefaultValue(OPT_SRV_PORT, SERVER_GDB, 8082);
    setDefaultValue(OPT_SRV_PROTOCOL, SERVER_GDB, SRVPROT_DEFAULT);
    setDefaultValue(OPT_SRV_AUTORUN, SERVER_GDB, true);
    setDefaultValue(OPT_SRV_VERBOSE, SERVER_GDB, true);
}

void
Properties::_dump(Category category, std::ostream& os) const
{
    for (const auto &it: defaults) {
        
        const string key = it.first;
        os << util::tab(key);

        if (values.contains(key)) {
            os << values.at(key) << std::endl;
        } else {
            os << defaults.at(key) << " (Default)" << std::endl;
        }
    }
}

void
Properties::load(const fs::path &path)
{
    auto fs = std::ifstream(path, std::ifstream::binary);
    
    if (!fs.is_open()) {
        throw VAError(ERROR_FILE_NOT_FOUND);
    }
 
    load(fs);
}

void
Properties::load(std::ifstream &stream)
{
    std::stringstream ss;
    ss << stream.rdbuf();
    
    load(ss);
}

void
Properties::load(std::stringstream &stream)
{
    isize line = 0;
    string input;
    string section;
    
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
            
            // Convert to lower case
            section = util::lowercased(section);
            continue;
        }
        
        // Check if this line is a key-value pair
        if (auto pos = input.find("="); pos != std::string::npos) {
            
            auto key = input.substr(0, pos);
            auto value = input.substr(pos + 1, std::string::npos);
            
            // Remove white spaces
            util::trim(key);
            util::trim(value);
            
            // Convert to lower case
            key = util::lowercased(key);
            
            // Add the key-value pair
            values[section.empty() ? key : section + "." + key] = value;
            continue;
        }
        
        throw VAError(ERROR_SYNTAX, line);
    }
}

void
Properties::save(const fs::path &path)
{
    auto fs = std::ofstream(path, std::ofstream::binary);
    
    if (!fs.is_open()) {
        throw VAError(ERROR_FILE_CANT_WRITE);
    }
 
    save(fs);
}

void
Properties::save(std::ofstream &stream)
{
    std::stringstream ss;
    save(ss);
    
    stream << ss.rdbuf();
}

void
Properties::save(std::stringstream &stream)
{
    for (const auto &[key, value]: values) {
        
        stream << key << "=" << value << std::endl;
    }
}

string
Properties::getValue(const string &key)
{
    if (values.contains(key)) {
        return values[key];
    } else {
        return getDefaultValue(key);
    }
}

i64
Properties::getValue(Option key)
{
    auto name = string(OptionEnum::key(key));
    auto value = getValue(name);
    
    try { return std::stol(value); } catch (...) {

        warn("Can't parse value %s\n", name.c_str());
        return 0;
    }
}

i64
Properties::getValue(Option key, isize nr)
{
    auto name = string(OptionEnum::key(key)) + std::to_string(nr);
    auto value = getValue(name);
    
    try { return std::stol(value); } catch (...) {

        warn("Can't parse value %s\n", name.c_str());
        return 0;
    }
}

string
Properties::getDefaultValue(const string &key)
{
    if (!defaults.contains(key)) {
        throw VAError(ERROR_INVALID_KEY);
    }
    
    return defaults[key];
}

i64
Properties::getDefaultValue(Option key)
{
    auto name = string(OptionEnum::key(key));
    auto value = getValue(name);
    
    return std::stol(value);
}

i64
Properties::getDefaultValue(Option key, isize nr)
{
    auto name = string(OptionEnum::key(key)) + std::to_string(nr);
    auto value = getValue(name);
    
    return std::stol(value);
}

void
Properties::setValue(const string &key, const string &value)
{
    if (!defaults.contains(key)) {
        throw VAError(ERROR_INVALID_KEY);
    }
    
    values[key] = value;
}

void
Properties::setValue(Option key, i64 value)
{
    auto name = string(OptionEnum::key(key));
    values[name] = std::to_string(value);
}

void
Properties::setValue(Option key, isize nr, i64 value)
{
    auto name = string(OptionEnum::key(key)) + std::to_string(nr);
    values[name] = std::to_string(value);

}

void
Properties::setDefaultValue(const string &key, const string &value)
{
    defaults[key] = value;
}

void
Properties::setDefaultValue(Option key, i64 value)
{
    auto name = string(OptionEnum::key(key));
    setDefaultValue(name, std::to_string(value));
}

void
Properties::setDefaultValue(Option key, isize nr, i64 value)
{
    auto name = string(OptionEnum::key(key)) + std::to_string(nr);
    setDefaultValue(name, std::to_string(value));
}

void
Properties::setDefaultValue(Option key, std::vector <isize> nrs, i64 value)
{
    for (auto &nr : nrs) setDefaultValue(key, nr, value);
}

void
Properties::removeValue(const string &key)
{
    if (values.contains(key)) values.erase(key);
}

void
Properties::removeValue(Option key)
{
    auto name = string(OptionEnum::key(key));
    removeValue(name);
}

void
Properties::removeValue(Option key, isize nr)
{
    auto name = string(OptionEnum::key(key)) + std::to_string(nr);
    removeValue(name);
}

void
Properties::removeAll()
{
    values.clear();
}
