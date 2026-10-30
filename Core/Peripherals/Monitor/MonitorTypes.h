// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"

namespace vamiga {

//
// Enumerations
//

/// Monitor color palette.
enum class Palette
{
    COLOR,
    RGB,
    BLACK_WHITE,
    PAPER_WHITE,
    GREEN,
    AMBER,
    SEPIA
};

struct PaletteEnum : utl::Reflectable<PaletteEnum, Palette>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Palette::SEPIA);
    
    static const char *_key(Palette value)
    {
        switch (value) {
                
            case Palette::COLOR:        return "COLOR";
            case Palette::RGB:          return "RGB";
            case Palette::BLACK_WHITE:  return "BLACK_WHITE";
            case Palette::PAPER_WHITE:  return "PAPER_WHITE";
            case Palette::GREEN:        return "GREEN";
            case Palette::AMBER:        return "AMBER";
            case Palette::SEPIA:        return "SEPIA";
        }
        return "???";
    }
    static const char *help(Palette value)
    {
        switch (value) {
                
            case Palette::COLOR:        return "Color palette";
            case Palette::RGB:          return "RGB direct";
            case Palette::BLACK_WHITE:  return "Black and white palette";
            case Palette::PAPER_WHITE:  return "Paper white palette";
            case Palette::GREEN:        return "Green palette";
            case Palette::AMBER:        return "Amber palette";
            case Palette::SEPIA:        return "Sepia palette";
        }
        return "???";
    }
};

/// Zoom level of the visible area.
enum class Zoom
{
    CUSTOM,
    NARROW,
    WIDE,
    EXTREME
};

struct ZoomEnum : utl::Reflectable<ZoomEnum, Zoom>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Zoom::EXTREME);
    
    static const char *_key(Zoom value)
    {
        switch (value) {
                
            case Zoom::CUSTOM:          return "CUSTOM";
            case Zoom::NARROW:          return "NARROW";
            case Zoom::WIDE:            return "WIDE";
            case Zoom::EXTREME:         return "EXTREME";
        }
        return "???";
    }
    static const char *help(Zoom value)
    {
        switch (value) {
                
            case Zoom::CUSTOM:          return "Custom zoom factors";
            case Zoom::NARROW:          return "Narrow cutout";
            case Zoom::WIDE:            return "Wide cutout";
            case Zoom::EXTREME:         return "Extreme cutout";
        }
        return "???";
    }
};

/// Centering mode of the visible area.
enum class Center
{
    CUSTOM,
    AUTO,
};

struct CenterEnum : utl::Reflectable<CenterEnum, Center>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Center::AUTO);
    
    static const char *_key(Center value)
    {
        switch (value) {
                
            case Center::CUSTOM:        return "CUSTOM";
            case Center::AUTO:          return "AUTO";
        }
        return "???";
    }
    static const char *help(Center value)
    {
        switch (value) {
                
            case Center::CUSTOM:        return "Custom offsets";
            case Center::AUTO:          return "Automatic screen centering";
        }
        return "???";
    }
};

/// Upscaling algorithm of the video port.
enum class Upscaler : long
{
    NONE,                      ///< No upscaler
    EPX_2X,                    ///< EPX upscaler (2x)
    XBR_4X                     ///< XBR upscaler (4x)
};

struct UpscalerEnum : utl::Reflectable<UpscalerEnum, Upscaler> {

    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Upscaler::XBR_4X);

    static const char *_key(Upscaler value)
    {
        switch (value) {

            case Upscaler::NONE:     return "NONE";
            case Upscaler::EPX_2X:   return "EPX_2X";
            case Upscaler::XBR_4X:   return "XBR_4X";
        }
        return "???";
    }
    
    static const char *help(Upscaler value)
    {
        return "";
    }
};

/// Dot mask of the monitor effect.
enum class Dotmask : long
{
    NONE,                       ///< No dotmask
    BISECTED,                   ///< Dotmask pattern 1
    TRISECTED,                  ///< Dotmask pattern 2
    BISECTED_SHIFTED,           ///< Dotmask pattern 3
    TRISECTED_SHIFTED           ///< Dotmask pattern 4
};

struct DotmaskEnum : utl::Reflectable<DotmaskEnum, Dotmask> {

    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Dotmask::TRISECTED_SHIFTED);

    static const char *_key(Dotmask value)
    {
        switch (value) {

            case Dotmask::NONE:              return "NONE";
            case Dotmask::BISECTED:          return "BISECTED";
            case Dotmask::TRISECTED:         return "TRISECTED";
            case Dotmask::BISECTED_SHIFTED:  return "BISECTED_SHIFTED";
            case Dotmask::TRISECTED_SHIFTED: return "TRISECTED_SHIFTED";
        }
        return "???";
    }
    
    static const char *help(Dotmask value)
    {
        return "";
    }
};

/// Scanline effect of the monitor.
enum class Scanlines : long
{
    NONE,                     ///< No scanlines
    EMBEDDED,                 ///< Embed scanlines in the emulator texture
    SUPERIMPOSE               ///< Emulate scanlines in the fragment shader
};

struct ScanlinesEnum : utl::Reflectable<ScanlinesEnum, Scanlines> {

    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Scanlines::SUPERIMPOSE);

    static const char *_key(Scanlines value)
    {
        switch (value) {

            case Scanlines::NONE:            return "NONE";
            case Scanlines::EMBEDDED:        return "EMBEDDED";
            case Scanlines::SUPERIMPOSE:     return "SUPERIMPOSE";
        }
        return "???";
    }
    
    static const char *help(Scanlines value)
    {
        return "";
    }
};


//
// Structures
//

/// Configuration options of the monitor.
typedef struct
{
    /// Colors
    Palette palette;
    isize brightness;                       ///< Monitor brightness
    isize contrast;                         ///< Monitor contrast
    isize saturation;                       ///< Monitor saturation

    /// Geometry
    Center center;
    isize hCenter;                          ///< Horizontal center
    isize vCenter;                          ///< Vertical center
    Zoom zoom;                              ///< Zoom mode
    isize hZoom;                            ///< Horizontal zoom
    isize vZoom;                            ///< Vertical zoom

    /// Effects
    Upscaler enhancer;
    Upscaler upscaler;                      ///< Upscaler

    bool blur;                              ///< Blur
    isize blurRadius;                       ///< Blur radius

    bool bloom;                             ///< Bloom
    isize bloomRadius;                      ///< Bloom radius
    isize bloomBrightness;                  ///< Bloom brightness
    isize bloomWeight;                      ///< Bloom weight

    Dotmask dotmask;                        ///< Dotmask
    isize dotMaskBrightness;                ///< Dotmask brightness

    Scanlines scanlines;                    ///< Scanlines
    isize scanlineBrightness;               ///< Scanline brightness
    isize scanlineWeight;                   ///< Scanline weight

    bool disalignment;                      ///< Cathode ray disalignment
    isize disalignmentH;                    ///< Horizontal cathode ray shift
    isize disalignmentV;                    ///< Vertical cathode ray shift
    
    bool flickerFixer;                      ///< Interlace flicker fixer
    isize flickerJitter;                    ///< Interlace flicker jitter
}
MonitorConfig;

}
