// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "Option.h"
#include "VAmigaTypes.h"
#include "Error.h"
#include "IOUtils.h"

namespace vamiga {

std::unique_ptr<OptionParser>
OptionParser::create(Opt opt, i64 arg)
{
    auto enumParser = [&]<class T, typename E>() { return std::unique_ptr<EnumParser<T,E>>(new EnumParser<T,E>(opt, arg)); };
    auto boolParser = [&]() { return std::unique_ptr<BoolParser>(new BoolParser(opt, arg)); };
    auto numParser  = [&](string unit = "") { return std::unique_ptr<NumParser>(new NumParser(opt, arg, unit)); };
    auto hexParser  = [&](string unit = "") { return std::unique_ptr<HexParser>(new HexParser(opt, arg, unit)); };

    switch (opt) {

        case Opt::HOST_SAMPLE_RATE:          return numParser(" Hz");
        case Opt::HOST_REFRESH_RATE:         return numParser(" fps");
        case Opt::HOST_FRAMEBUF_WIDTH:       return numParser(" pixels");
        case Opt::HOST_FRAMEBUF_HEIGHT:      return numParser(" pixels");

        case Opt::AMIGA_VIDEO_FORMAT:        return enumParser.template operator()<TVEnum,TV>();
        case Opt::AMIGA_WARP_MODE:           return enumParser.template operator()<WarpEnum,Warp>();
        case Opt::AMIGA_WARP_BOOT:           return numParser(" sec");
        case Opt::AMIGA_VSYNC:               return boolParser();
        case Opt::AMIGA_SPEED_BOOST:         return numParser("%");
        case Opt::AMIGA_RUN_AHEAD:           return numParser(" frames");
        case Opt::AMIGA_SNAP_AUTO:           return boolParser();
        case Opt::AMIGA_SNAP_DELAY:          return numParser(" sec");
        case Opt::AMIGA_SNAP_COMPRESSOR:     return enumParser.template operator()<CompressorEnum,Compressor>();
        case Opt::AMIGA_WS_COMPRESSION:      return boolParser();

        case Opt::AGNUS_REVISION:            return enumParser.template operator()<AgnusRevisionEnum,AgnusRevision>();
        case Opt::AGNUS_PTR_DROPS:           return boolParser();

        case Opt::DENISE_REVISION:           return enumParser.template operator()<DeniseRevEnum,DeniseRev>();
        case Opt::DENISE_VIEWPORT_TRACKING:  return boolParser();
        case Opt::DENISE_FRAME_SKIPPING:     return boolParser();
        case Opt::DENISE_HIDDEN_BITPLANES:   return numParser();
        case Opt::DENISE_HIDDEN_SPRITES:     return numParser();
        case Opt::DENISE_HIDDEN_LAYERS:      return numParser();
        case Opt::DENISE_HIDDEN_LAYER_ALPHA: return numParser();
        case Opt::DENISE_CLX_SPR_SPR:        return boolParser();
        case Opt::DENISE_CLX_SPR_PLF:        return boolParser();
        case Opt::DENISE_CLX_PLF_PLF:        return boolParser();

        case Opt::MON_PALETTE:               return enumParser.template operator()<PaletteEnum,Palette>();
        case Opt::MON_BRIGHTNESS:            return numParser("%");
        case Opt::MON_CONTRAST:              return numParser("%");
        case Opt::MON_SATURATION:            return numParser("%");

        case Opt::DMA_DEBUG_ENABLE:          return boolParser();
        case Opt::DMA_DEBUG_MODE:            return enumParser.template operator()<DmaDisplayModeEnum,DmaDisplayMode>();
        case Opt::DMA_DEBUG_OPACITY:         return numParser("%");
        case Opt::DMA_DEBUG_CHANNEL0:        return boolParser();
        case Opt::DMA_DEBUG_CHANNEL1:        return boolParser();
        case Opt::DMA_DEBUG_CHANNEL2:        return boolParser();
        case Opt::DMA_DEBUG_CHANNEL3:        return boolParser();
        case Opt::DMA_DEBUG_CHANNEL4:        return boolParser();
        case Opt::DMA_DEBUG_CHANNEL5:        return boolParser();
        case Opt::DMA_DEBUG_CHANNEL6:        return boolParser();
        case Opt::DMA_DEBUG_CHANNEL7:        return boolParser();
        case Opt::DMA_DEBUG_COLOR0:          return numParser();
        case Opt::DMA_DEBUG_COLOR1:          return numParser();
        case Opt::DMA_DEBUG_COLOR2:          return numParser();
        case Opt::DMA_DEBUG_COLOR3:          return numParser();
        case Opt::DMA_DEBUG_COLOR4:          return numParser();
        case Opt::DMA_DEBUG_COLOR5:          return numParser();
        case Opt::DMA_DEBUG_COLOR6:          return numParser();
        case Opt::DMA_DEBUG_COLOR7:          return numParser();

        case Opt::LA_PROBE0:                 return enumParser.template operator()<ProbeEnum,Probe>();
        case Opt::LA_PROBE1:                 return enumParser.template operator()<ProbeEnum,Probe>();
        case Opt::LA_PROBE2:                 return enumParser.template operator()<ProbeEnum,Probe>();
        case Opt::LA_PROBE3:                 return enumParser.template operator()<ProbeEnum,Probe>();
        case Opt::LA_ADDR0:                  return hexParser();
        case Opt::LA_ADDR1:                  return hexParser();
        case Opt::LA_ADDR2:                  return hexParser();
        case Opt::LA_ADDR3:                  return hexParser();

        case Opt::VID_WHITE_NOISE:           return boolParser();
            
        case Opt::CPU_REVISION:              return enumParser.template operator()<CPURevEnum,CPURev>();
        case Opt::CPU_DASM_REVISION:         return enumParser.template operator()<DasmRevEnum,DasmRev>();
        case Opt::CPU_DASM_SYNTAX:           return enumParser.template operator()<DasmSyntaxEnum,DasmSyntax>();
        case Opt::CPU_DASM_NUMBERS:          return enumParser.template operator()<DasmNumbersEnum,DasmNumbers>();
        case Opt::CPU_OVERCLOCKING:          return numParser("x");
        case Opt::CPU_RESET_VAL:             return numParser();

        case Opt::RTC_MODEL:                 return enumParser.template operator()<RTCRevisionEnum,RTCRevision>();

        case Opt::MEM_CHIP_RAM:              return numParser();
        case Opt::MEM_SLOW_RAM:              return numParser();
        case Opt::MEM_FAST_RAM:              return numParser();
        case Opt::MEM_EXT_START:             return numParser();
        case Opt::MEM_SAVE_ROMS:             return boolParser();
        case Opt::MEM_SLOW_RAM_DELAY:        return boolParser();
        case Opt::MEM_SLOW_RAM_MIRROR:       return boolParser();
        case Opt::MEM_BANKMAP:               return enumParser.template operator()<BankMapEnum,BankMap>();
        case Opt::MEM_UNMAPPING_TYPE:        return enumParser.template operator()<UnmappedMemoryEnum,UnmappedMemory>();
        case Opt::MEM_RAM_INIT_PATTERN:      return enumParser.template operator()<RamInitPatternEnum,RamInitPattern>();

        case Opt::DC_SPEED:                  return numParser();
        case Opt::DC_LOCK_DSKSYNC:           return boolParser();
        case Opt::DC_AUTO_DSKSYNC:           return boolParser();

        case Opt::DRIVE_CONNECT:             return boolParser();
        case Opt::DRIVE_TYPE:                return enumParser.template operator()<FloppyDriveTypeEnum,FloppyDriveType>();
        case Opt::DRIVE_MECHANICS:           return enumParser.template operator()<DriveMechanicsEnum,DriveMechanics>();
        case Opt::DRIVE_RPM:                 return numParser();
        case Opt::DRIVE_SWAP_DELAY:          return numParser();
        case Opt::DRIVE_PAN:                 return numParser();
        case Opt::DRIVE_STEP_VOLUME:         return numParser("%");
        case Opt::DRIVE_POLL_VOLUME:         return numParser("%");
        case Opt::DRIVE_INSERT_VOLUME:       return numParser("%");
        case Opt::DRIVE_EJECT_VOLUME:        return numParser("%");

        case Opt::HDC_CONNECT:               return boolParser();

        case Opt::HDR_TYPE:                  return enumParser.template operator()<HardDriveTypeEnum,HardDriveType>();
        case Opt::HDR_PAN:                   return numParser();
        case Opt::HDR_STEP_VOLUME:           return numParser("%");

        case Opt::SER_DEVICE:                return enumParser.template operator()<SerialPortDeviceEnum,SerialPortDevice>();
        case Opt::SER_VERBOSE:               return boolParser();

        case Opt::BLITTER_ACCURACY:          return numParser();

        case Opt::CIA_REVISION:              return enumParser.template operator()<CIARevEnum,CIARev>();
        case Opt::CIA_TODBUG:                return boolParser();
        case Opt::CIA_ECLOCK_SYNCING:        return boolParser();
        case Opt::CIA_IDLE_SLEEP:            return boolParser();

        case Opt::KBD_ACCURACY:              return boolParser();

        case Opt::MOUSE_PULLUP_RESISTORS:    return boolParser();
        case Opt::MOUSE_SHAKE_DETECTION:     return boolParser();
        case Opt::MOUSE_VELOCITY:            return numParser();

        case Opt::JOY_AUTOFIRE:              return boolParser();
        case Opt::JOY_AUTOFIRE_BURSTS:       return boolParser();
        case Opt::JOY_AUTOFIRE_BULLETS:      return numParser();
        case Opt::JOY_AUTOFIRE_DELAY:        return numParser();

        case Opt::AUD_PAN0:                  return numParser();
        case Opt::AUD_PAN1:                  return numParser();
        case Opt::AUD_PAN2:                  return numParser();
        case Opt::AUD_PAN3:                  return numParser();
        case Opt::AUD_VOL0:                  return numParser("%");
        case Opt::AUD_VOL1:                  return numParser("%");
        case Opt::AUD_VOL2:                  return numParser("%");
        case Opt::AUD_VOL3:                  return numParser("%");
        case Opt::AUD_VOLL:                  return numParser("%");
        case Opt::AUD_VOLR:                  return numParser("%");
        case Opt::AUD_FILTER_TYPE:           return enumParser.template operator()<FilterTypeEnum,FilterType>();
        case Opt::AUD_BUFFER_SIZE:           return numParser(" samples");
        case Opt::AUD_SAMPLING_METHOD:       return enumParser.template operator()<SamplingMethodEnum,SamplingMethod>();
        case Opt::AUD_ASR:                   return boolParser();
        case Opt::AUD_FASTPATH:              return boolParser();

        case Opt::DIAG_BOARD:                return boolParser();

        case Opt::SRV_PORT:                  return numParser();
        case Opt::SRV_PROTOCOL:              return enumParser.template operator()<ServerProtocolEnum,ServerProtocol>();
        case Opt::SRV_AUTORUN:               return boolParser();
        case Opt::SRV_VERBOSE:               return boolParser();

        default:
            fatalError;
    }
}

i64
OptionParser::parse(Opt opt, const string &arg)
{
    return create(opt)->parse(arg);
}

std::vector<std::pair<string, long>>
OptionParser::pairs(Opt opt)
{
    return create(opt)->pairs();
}

string
OptionParser::asPlainString(Opt opt, i64 arg)
{
    return create(opt, arg)->asPlainString();
}

string
OptionParser::asString(Opt opt, i64 arg)
{
    return create(opt, arg)->asString();
}

string
OptionParser::keyList(Opt opt)
{
    return create(opt)->keyList();
}

string
OptionParser::argList(Opt opt)
{
    return create(opt)->argList();
}

string
OptionParser::help(Opt opt, isize item)
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
