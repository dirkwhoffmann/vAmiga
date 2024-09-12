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
    auto enumParser = [&]<typename T>() { return std::unique_ptr<EnumParser<T>>(new EnumParser<T>(opt, arg)); };
    auto boolParser = [&]() { return std::unique_ptr<BoolParser>(new BoolParser(opt, arg)); };
    auto numParser  = [&](string unit = "") { return std::unique_ptr<NumParser>(new NumParser(opt, arg, unit)); };
    // auto hexParser  = [&](string unit = "") { return std::unique_ptr<HexParser>(new HexParser(opt, arg, unit)); };

    switch (opt) {

        case OPT_HOST_SAMPLE_RATE:          return numParser(" Hz");
        case OPT_HOST_REFRESH_RATE:         return numParser(" fps");
        case OPT_HOST_FRAMEBUF_WIDTH:       return numParser(" pixels");
        case OPT_HOST_FRAMEBUF_HEIGHT:      return numParser(" pixels");

        case OPT_AMIGA_VIDEO_FORMAT:        return enumParser.template operator()<VideoFormatEnum>();
        case OPT_AMIGA_WARP_MODE:           return enumParser.template operator()<WarpModeEnum>();
        case OPT_AMIGA_WARP_BOOT:           return numParser(" sec");
        case OPT_AMIGA_VSYNC:               return boolParser();
        case OPT_AMIGA_SPEED_BOOST:         return numParser("%");
        case OPT_AMIGA_RUN_AHEAD:           return numParser(" frames");

        case OPT_AMIGA_SNAP_AUTO:           return boolParser();
        case OPT_AMIGA_SNAP_DELAY:          return numParser(" sec");
        case OPT_AMIGA_SNAP_COMPRESS:       return boolParser();

        case OPT_AGNUS_REVISION:            return enumParser.template operator()<AgnusRevisionEnum>();
        case OPT_AGNUS_PTR_DROPS:           return boolParser();

        case OPT_DENISE_REVISION:           return enumParser.template operator()<DeniseRevisionEnum>();
        case OPT_DENISE_VIEWPORT_TRACKING:  return boolParser();
        case OPT_DENISE_FRAME_SKIPPING:     return boolParser();
        case OPT_DENISE_HIDDEN_BITPLANES:   return numParser();
        case OPT_DENISE_HIDDEN_SPRITES:     return numParser();
        case OPT_DENISE_HIDDEN_LAYERS:      return numParser();
        case OPT_DENISE_HIDDEN_LAYER_ALPHA: return numParser();
        case OPT_DENISE_CLX_SPR_SPR:        return boolParser();
        case OPT_DENISE_CLX_SPR_PLF:        return boolParser();
        case OPT_DENISE_CLX_PLF_PLF:        return boolParser();

        case OPT_MON_PALETTE:               return enumParser.template operator()<PaletteEnum>();
        case OPT_MON_BRIGHTNESS:            return numParser("%");
        case OPT_MON_CONTRAST:              return numParser("%");
        case OPT_MON_SATURATION:            return numParser("%");

        case OPT_DMA_DEBUG_ENABLE:          return boolParser();
        case OPT_DMA_DEBUG_MODE:            return enumParser.template operator()<DmaDisplayModeEnum>();
        case OPT_DMA_DEBUG_OPACITY:         return numParser("%");
        case OPT_DMA_DEBUG_CHANNEL0:        return boolParser();
        case OPT_DMA_DEBUG_CHANNEL1:        return boolParser();
        case OPT_DMA_DEBUG_CHANNEL2:        return boolParser();
        case OPT_DMA_DEBUG_CHANNEL3:        return boolParser();
        case OPT_DMA_DEBUG_CHANNEL4:        return boolParser();
        case OPT_DMA_DEBUG_CHANNEL5:        return boolParser();
        case OPT_DMA_DEBUG_CHANNEL6:        return boolParser();
        case OPT_DMA_DEBUG_CHANNEL7:        return boolParser();
        case OPT_DMA_DEBUG_COLOR0:          return numParser();
        case OPT_DMA_DEBUG_COLOR1:          return numParser();
        case OPT_DMA_DEBUG_COLOR2:          return numParser();
        case OPT_DMA_DEBUG_COLOR3:          return numParser();
        case OPT_DMA_DEBUG_COLOR4:          return numParser();
        case OPT_DMA_DEBUG_COLOR5:          return numParser();
        case OPT_DMA_DEBUG_COLOR6:          return numParser();
        case OPT_DMA_DEBUG_COLOR7:          return numParser();

        case OPT_VID_WHITE_NOISE:           return boolParser();
            
        case OPT_CPU_REVISION:              return enumParser.template operator()<CPURevisionEnum>();
        case OPT_CPU_DASM_REVISION:         return enumParser.template operator()<DasmRevisionEnum>();
        case OPT_CPU_DASM_SYNTAX:           return enumParser.template operator()<DasmSyntaxEnum>();
        case OPT_CPU_OVERCLOCKING:          return numParser("x");
        case OPT_CPU_RESET_VAL:             return numParser();

        case OPT_RTC_MODEL:                 return enumParser.template operator()<RTCRevisionEnum>();

        case OPT_MEM_CHIP_RAM:              return numParser();
        case OPT_MEM_SLOW_RAM:              return numParser();
        case OPT_MEM_FAST_RAM:              return numParser();
        case OPT_MEM_EXT_START:             return numParser();
        case OPT_MEM_SAVE_ROMS:             return boolParser();
        case OPT_MEM_SLOW_RAM_DELAY:        return boolParser();
        case OPT_MEM_SLOW_RAM_MIRROR:       return boolParser();
        case OPT_MEM_BANKMAP:               return enumParser.template operator()<BankMapEnum>();
        case OPT_MEM_UNMAPPING_TYPE:        return enumParser.template operator()<UnmappedMemoryEnum>();
        case OPT_MEM_RAM_INIT_PATTERN:      return enumParser.template operator()<RamInitPatternEnum>();

        case OPT_DC_SPEED:                  return numParser();
        case OPT_DC_LOCK_DSKSYNC:           return boolParser();
        case OPT_DC_AUTO_DSKSYNC:           return boolParser();

        case OPT_DRIVE_CONNECT:             return boolParser();
        case OPT_DRIVE_TYPE:                return enumParser.template operator()<FloppyDriveTypeEnum>();
        case OPT_DRIVE_MECHANICS:           return enumParser.template operator()<DriveMechanicsEnum>();
        case OPT_DRIVE_RPM:                 return numParser();
        case OPT_DRIVE_SWAP_DELAY:          return numParser();
        case OPT_DRIVE_PAN:                 return numParser();
        case OPT_DRIVE_STEP_VOLUME:         return numParser("%");
        case OPT_DRIVE_POLL_VOLUME:         return numParser("%");
        case OPT_DRIVE_INSERT_VOLUME:       return numParser("%");
        case OPT_DRIVE_EJECT_VOLUME:        return numParser("%");

        case OPT_HDC_CONNECT:               return boolParser();

        case OPT_HDR_TYPE:                  return enumParser.template operator()<HardDriveTypeEnum>();
        case OPT_HDR_WRITE_THROUGH:         return boolParser();
        case OPT_HDR_PAN:                   return numParser();
        case OPT_HDR_STEP_VOLUME:           return numParser("%");

        case OPT_SER_DEVICE:                return enumParser.template operator()<SerialPortDeviceEnum>();
        case OPT_SER_VERBOSE:               return boolParser();

        case OPT_BLITTER_ACCURACY:          return numParser();

        case OPT_CIA_REVISION:              return enumParser.template operator()<CIARevisionEnum>();
        case OPT_CIA_TODBUG:                return boolParser();
        case OPT_CIA_ECLOCK_SYNCING:        return boolParser();
        case OPT_CIA_IDLE_SLEEP:            return boolParser();

        case OPT_KBD_ACCURACY:              return boolParser();

        case OPT_MOUSE_PULLUP_RESISTORS:    return boolParser();
        case OPT_MOUSE_SHAKE_DETECTION:     return boolParser();
        case OPT_MOUSE_VELOCITY:            return numParser();

        case OPT_JOY_AUTOFIRE:              return boolParser();
        case OPT_JOY_AUTOFIRE_BURSTS:       return boolParser();
        case OPT_JOY_AUTOFIRE_BULLETS:      return numParser();
        case OPT_JOY_AUTOFIRE_DELAY:        return numParser();

        case OPT_AUD_SAMPLING_METHOD:       return enumParser.template operator()<SamplingMethodEnum>();
        case OPT_AUD_FILTER_TYPE:           return enumParser.template operator()<FilterTypeEnum>();
        case OPT_AUD_PAN0:                  return numParser();
        case OPT_AUD_PAN1:                  return numParser();
        case OPT_AUD_PAN2:                  return numParser();
        case OPT_AUD_PAN3:                  return numParser();
        case OPT_AUD_VOL0:                  return numParser("%");
        case OPT_AUD_VOL1:                  return numParser("%");
        case OPT_AUD_VOL2:                  return numParser("%");
        case OPT_AUD_VOL3:                  return numParser("%");
        case OPT_AUD_VOLL:                  return numParser("%");
        case OPT_AUD_VOLR:                  return numParser("%");
        case OPT_AUD_FASTPATH:              return boolParser();

        case OPT_DIAG_BOARD:                return boolParser();

        case OPT_SRV_PORT:                  return numParser();
        case OPT_SRV_PROTOCOL:              return enumParser.template operator()<ServerProtocolEnum>();
        case OPT_SRV_AUTORUN:               return boolParser();
        case OPT_SRV_VERBOSE:               return boolParser();

        default:
            fatalError;
    }
}

i64
OptionParser::parse(Option opt, const string &arg)
{
    return create(opt)->parse(arg);
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
HexParser::asPlainString()
{
    std::stringstream ss;
    ss << std::hex << "0x" << std::setw(4) << std::setfill('0') << arg;

    return ss.str();
}

}
