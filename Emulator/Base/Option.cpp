// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Option.h"
#include "VAmigaTypes.h"
#include "Error.h"
#include "IOUtils.h"

namespace vamiga {

std::unique_ptr<OptionParser>
OptionParser::create(Option opt, i64 arg)
{
    auto enumParser = [&]<class T, typename E>() { return std::unique_ptr<EnumParser<T,E>>(new EnumParser<T,E>(opt, arg)); };
    auto boolParser = [&]() { return std::unique_ptr<BoolParser>(new BoolParser(opt, arg)); };
    auto numParser  = [&](string unit = "") { return std::unique_ptr<NumParser>(new NumParser(opt, arg, unit)); };
    auto hexParser  = [&](string unit = "") { return std::unique_ptr<HexParser>(new HexParser(opt, arg, unit)); };

    switch (opt) {

        case Option::HOST_SAMPLE_RATE:          return numParser(" Hz");
        case Option::HOST_REFRESH_RATE:         return numParser(" fps");
        case Option::HOST_FRAMEBUF_WIDTH:       return numParser(" pixels");
        case Option::HOST_FRAMEBUF_HEIGHT:      return numParser(" pixels");

        case Option::AMIGA_VIDEO_FORMAT:        return enumParser.template operator()<TVEnum,TV>();
        case Option::AMIGA_WARP_MODE:           return enumParser.template operator()<WarpEnum,Warp>();
        case Option::AMIGA_WARP_BOOT:           return numParser(" sec");
        case Option::AMIGA_VSYNC:               return boolParser();
        case Option::AMIGA_SPEED_BOOST:         return numParser("%");
        case Option::AMIGA_RUN_AHEAD:           return numParser(" frames");

        case Option::AMIGA_SNAP_AUTO:           return boolParser();
        case Option::AMIGA_SNAP_DELAY:          return numParser(" sec");
        case Option::AMIGA_SNAP_COMPRESS:       return boolParser();

        case Option::AGNUS_REVISION:            return enumParser.template operator()<AgnusRevisionEnum,AgnusRevision>();
        case Option::AGNUS_PTR_DROPS:           return boolParser();

        case Option::DENISE_REVISION:           return enumParser.template operator()<DeniseRevEnum,DeniseRev>();
        case Option::DENISE_VIEWPORT_TRACKING:  return boolParser();
        case Option::DENISE_FRAME_SKIPPING:     return boolParser();
        case Option::DENISE_HIDDEN_BITPLANES:   return numParser();
        case Option::DENISE_HIDDEN_SPRITES:     return numParser();
        case Option::DENISE_HIDDEN_LAYERS:      return numParser();
        case Option::DENISE_HIDDEN_LAYER_ALPHA: return numParser();
        case Option::DENISE_CLX_SPR_SPR:        return boolParser();
        case Option::DENISE_CLX_SPR_PLF:        return boolParser();
        case Option::DENISE_CLX_PLF_PLF:        return boolParser();

        case Option::MON_PALETTE:               return enumParser.template operator()<PaletteEnum,Palette>();
        case Option::MON_BRIGHTNESS:            return numParser("%");
        case Option::MON_CONTRAST:              return numParser("%");
        case Option::MON_SATURATION:            return numParser("%");

        case Option::DMA_DEBUG_ENABLE:          return boolParser();
        case Option::DMA_DEBUG_MODE:            return enumParser.template operator()<DmaDisplayModeEnum,DmaDisplayMode>();
        case Option::DMA_DEBUG_OPACITY:         return numParser("%");
        case Option::DMA_DEBUG_CHANNEL0:        return boolParser();
        case Option::DMA_DEBUG_CHANNEL1:        return boolParser();
        case Option::DMA_DEBUG_CHANNEL2:        return boolParser();
        case Option::DMA_DEBUG_CHANNEL3:        return boolParser();
        case Option::DMA_DEBUG_CHANNEL4:        return boolParser();
        case Option::DMA_DEBUG_CHANNEL5:        return boolParser();
        case Option::DMA_DEBUG_CHANNEL6:        return boolParser();
        case Option::DMA_DEBUG_CHANNEL7:        return boolParser();
        case Option::DMA_DEBUG_COLOR0:          return numParser();
        case Option::DMA_DEBUG_COLOR1:          return numParser();
        case Option::DMA_DEBUG_COLOR2:          return numParser();
        case Option::DMA_DEBUG_COLOR3:          return numParser();
        case Option::DMA_DEBUG_COLOR4:          return numParser();
        case Option::DMA_DEBUG_COLOR5:          return numParser();
        case Option::DMA_DEBUG_COLOR6:          return numParser();
        case Option::DMA_DEBUG_COLOR7:          return numParser();

        case Option::LA_PROBE0:                 return enumParser.template operator()<ProbeEnum,Probe>();
        case Option::LA_PROBE1:                 return enumParser.template operator()<ProbeEnum,Probe>();
        case Option::LA_PROBE2:                 return enumParser.template operator()<ProbeEnum,Probe>();
        case Option::LA_PROBE3:                 return enumParser.template operator()<ProbeEnum,Probe>();
        case Option::LA_ADDR0:                  return hexParser();
        case Option::LA_ADDR1:                  return hexParser();
        case Option::LA_ADDR2:                  return hexParser();
        case Option::LA_ADDR3:                  return hexParser();

        case Option::VID_WHITE_NOISE:           return boolParser();
            
        case Option::CPU_REVISION:              return enumParser.template operator()<CPURevEnum,CPURev>();
        case Option::CPU_DASM_REVISION:         return enumParser.template operator()<DasmRevEnum,DasmRev>();
        case Option::CPU_DASM_SYNTAX:           return enumParser.template operator()<DasmSyntaxEnum,DasmSyntax>();
        case Option::CPU_DASM_NUMBERS:          return enumParser.template operator()<DasmNumbersEnum,DasmNumbers>();
        case Option::CPU_OVERCLOCKING:          return numParser("x");
        case Option::CPU_RESET_VAL:             return numParser();

        case Option::RTC_MODEL:                 return enumParser.template operator()<RTCRevisionEnum,RTCRevision>();

        case Option::MEM_CHIP_RAM:              return numParser();
        case Option::MEM_SLOW_RAM:              return numParser();
        case Option::MEM_FAST_RAM:              return numParser();
        case Option::MEM_EXT_START:             return numParser();
        case Option::MEM_SAVE_ROMS:             return boolParser();
        case Option::MEM_SLOW_RAM_DELAY:        return boolParser();
        case Option::MEM_SLOW_RAM_MIRROR:       return boolParser();
        case Option::MEM_BANKMAP:               return enumParser.template operator()<BankMapEnum,BankMap>();
        case Option::MEM_UNMAPPING_TYPE:        return enumParser.template operator()<UnmappedMemoryEnum,UnmappedMemory>();
        case Option::MEM_RAM_INIT_PATTERN:      return enumParser.template operator()<RamInitPatternEnum,RamInitPattern>();

        case Option::DC_SPEED:                  return numParser();
        case Option::DC_LOCK_DSKSYNC:           return boolParser();
        case Option::DC_AUTO_DSKSYNC:           return boolParser();

        case Option::DRIVE_CONNECT:             return boolParser();
        case Option::DRIVE_TYPE:                return enumParser.template operator()<FloppyDriveTypeEnum,FloppyDriveType>();
        case Option::DRIVE_MECHANICS:           return enumParser.template operator()<DriveMechanicsEnum,DriveMechanics>();
        case Option::DRIVE_RPM:                 return numParser();
        case Option::DRIVE_SWAP_DELAY:          return numParser();
        case Option::DRIVE_PAN:                 return numParser();
        case Option::DRIVE_STEP_VOLUME:         return numParser("%");
        case Option::DRIVE_POLL_VOLUME:         return numParser("%");
        case Option::DRIVE_INSERT_VOLUME:       return numParser("%");
        case Option::DRIVE_EJECT_VOLUME:        return numParser("%");

        case Option::HDC_CONNECT:               return boolParser();

        case Option::HDR_TYPE:                  return enumParser.template operator()<HardDriveTypeEnum,HardDriveType>();
        case Option::HDR_WRITE_THROUGH:         return boolParser();
        case Option::HDR_PAN:                   return numParser();
        case Option::HDR_STEP_VOLUME:           return numParser("%");

        case Option::SER_DEVICE:                return enumParser.template operator()<SerialPortDeviceEnum,SerialPortDevice>();
        case Option::SER_VERBOSE:               return boolParser();

        case Option::BLITTER_ACCURACY:          return numParser();

        case Option::CIA_REVISION:              return enumParser.template operator()<CIARevEnum,CIARev>();
        case Option::CIA_TODBUG:                return boolParser();
        case Option::CIA_ECLOCK_SYNCING:        return boolParser();
        case Option::CIA_IDLE_SLEEP:            return boolParser();

        case Option::KBD_ACCURACY:              return boolParser();

        case Option::MOUSE_PULLUP_RESISTORS:    return boolParser();
        case Option::MOUSE_SHAKE_DETECTION:     return boolParser();
        case Option::MOUSE_VELOCITY:            return numParser();

        case Option::JOY_AUTOFIRE:              return boolParser();
        case Option::JOY_AUTOFIRE_BURSTS:       return boolParser();
        case Option::JOY_AUTOFIRE_BULLETS:      return numParser();
        case Option::JOY_AUTOFIRE_DELAY:        return numParser();

        case Option::AUD_SAMPLING_METHOD:       return enumParser.template operator()<SamplingMethodEnum,SamplingMethod>();
        case Option::AUD_FILTER_TYPE:           return enumParser.template operator()<FilterTypeEnum,FilterType>();
        case Option::AUD_PAN0:                  return numParser();
        case Option::AUD_PAN1:                  return numParser();
        case Option::AUD_PAN2:                  return numParser();
        case Option::AUD_PAN3:                  return numParser();
        case Option::AUD_VOL0:                  return numParser("%");
        case Option::AUD_VOL1:                  return numParser("%");
        case Option::AUD_VOL2:                  return numParser("%");
        case Option::AUD_VOL3:                  return numParser("%");
        case Option::AUD_VOLL:                  return numParser("%");
        case Option::AUD_VOLR:                  return numParser("%");
        case Option::AUD_FASTPATH:              return boolParser();

        case Option::DIAG_BOARD:                return boolParser();

        case Option::SRV_PORT:                  return numParser();
        case Option::SRV_PROTOCOL:              return enumParser.template operator()<ServerProtocolEnum,ServerProtocol>();
        case Option::SRV_AUTORUN:               return boolParser();
        case Option::SRV_VERBOSE:               return boolParser();

        default:
            fatalError;
    }
}

i64
OptionParser::parse(Option opt, const string &arg)
{
    return create(opt)->parse(arg);
}

std::vector<std::pair<string, long>>
OptionParser::pairs(Option opt)
{
    return create(opt)->pairs();
}

string
OptionParser::asPlainString(Option opt, i64 arg)
{
    return create(opt, arg)->asPlainString();
}

string
OptionParser::asString(Option opt, i64 arg)
{
    return create(opt, arg)->asString();
}

string
OptionParser::keyList(Option opt)
{
    return create(opt)->keyList();
}

string
OptionParser::argList(Option opt)
{
    return create(opt)->argList();
}

string
OptionParser::help(Option opt, isize item)
{
    return create(opt)->help(item);
}

string
HexParser::asPlainString()
{
    std::stringstream ss;
    ss << std::hex << "0x" << std::setw(4) << std::setfill('0') << arg;

    return ss.str();
}

}
