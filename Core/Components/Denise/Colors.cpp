// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Colors.h"

namespace vamiga {

RgbColor::RgbColor(const AmigaColor &c)
{
    r = c.r / 15.0;
    g = c.g / 15.0;
    b = c.b / 15.0;
}

RgbColor::RgbColor(const GpuColor &c)
{
    r = (c.rawValue & 0xFF) / 255.0;
    g = ((c.rawValue >> 8) & 0xFF) / 255.0;
    b = ((c.rawValue >> 16) & 0xFF) / 255.0;
}

RgbColor::RgbColor(const YuvColor &c)
{
    r = c.y + 1.140 * c.v;
    g = c.y - 0.395 * c.u - 0.581 * c.v;
    b = c.y + 2.032 * c.u;
}

RgbColor
RgbColor::mix(RgbColor additive, double weight) const
{
    assert(additive.r <= 1.0);
    assert(additive.g <= 1.0);
    assert(additive.b <= 1.0);

    double newR = r + (additive.r - r) * weight;
    double newG = g + (additive.g - g) * weight;
    double newB = b + (additive.b - b) * weight;

    return RgbColor(newR, newG, newB);
}

const RgbColor RgbColor::black(0.0, 0.0, 0.0);
const RgbColor RgbColor::white(1.0, 1.0, 1.0);
const RgbColor RgbColor::red(1.0, 0.0, 0.0);
const RgbColor RgbColor::green(0.0, 1.0, 0.0);
const RgbColor RgbColor::blue(0.0, 0.0, 1.0);
const RgbColor RgbColor::yellow(1.0, 1.0, 0.0);
const RgbColor RgbColor::magenta(1.0, 0.0, 1.0);
const RgbColor RgbColor::cyan(0.0, 1.0, 1.0);

//
//
//

YuvColor::YuvColor(const RgbColor &c)
{
    y =  0.299 * c.r + 0.587 * c.g + 0.114 * c.b;
    u = -0.147 * c.r - 0.289 * c.g + 0.436 * c.b;
    v =  0.615 * c.r - 0.515 * c.g - 0.100 * c.b;
}

const YuvColor YuvColor::black(RgbColor::black);
const YuvColor YuvColor::white(RgbColor::white);
const YuvColor YuvColor::red(RgbColor::red);
const YuvColor YuvColor::green(RgbColor::green);
const YuvColor YuvColor::blue(RgbColor::blue);
const YuvColor YuvColor::yellow(RgbColor::yellow);
const YuvColor YuvColor::magenta(RgbColor::magenta);
const YuvColor YuvColor::cyan(RgbColor::cyan);

//
//
//

AmigaColor::AmigaColor(const GpuColor &c)
{
    r = u8(c.rawValue >> 4  & 0xF);
    g = u8(c.rawValue >> 12 & 0xF);
    b = u8(c.rawValue >> 20 & 0xF);
}

AmigaColor::AmigaColor(const struct RgbColor &c)
{
    r = u8(c.r * 0xF);
    g = u8(c.g * 0xF);
    b = u8(c.b * 0xF);
}

const AmigaColor AmigaColor::black(RgbColor::black);
const AmigaColor AmigaColor::white(RgbColor::white);
const AmigaColor AmigaColor::red(RgbColor::red);
const AmigaColor AmigaColor::green(RgbColor::green);
const AmigaColor AmigaColor::blue(RgbColor::blue);
const AmigaColor AmigaColor::yellow(RgbColor::yellow);
const AmigaColor AmigaColor::magenta(RgbColor::magenta);
const AmigaColor AmigaColor::cyan(RgbColor::cyan);

AmigaColor
AmigaColor::ehb() const
{
    return AmigaColor { u8(r / 2), u8(g / 2), u8(b / 2) };
}

AmigaColor
AmigaColor::shr() const
{
    return AmigaColor {

        u8((r & 0xC) | r >> 2),
        u8((g & 0xC) | g >> 2),
        u8((b & 0xC) | b >> 2)
    };
}

AmigaColor
AmigaColor::mix(const AmigaColor &c) const
{
    return AmigaColor { u8((r+c.r)/2), u8((g+c.g)/2), u8((b+c.b)/2) };
}


//
//
//

GpuColor::GpuColor(const AmigaColor &c)
{
    rawValue = u32(0xFF << 24 | c.b << 20 | c.g << 12 | c.r << 4);
}

GpuColor::GpuColor(const RgbColor &c)
{
    auto a = 255;
    auto r = u8(c.r * 255);
    auto g = u8(c.g * 255);
    auto b = u8(c.b * 255);

    rawValue = u32(a << 24 | b << 16 | g << 8 | r);
}

GpuColor::GpuColor(u8 r, u8 g, u8 b)
{
    auto a = 255;
    rawValue = u32(a << 24 | b << 16 | g << 8 | r);
}

const GpuColor GpuColor::black(RgbColor::black);
const GpuColor GpuColor::white(RgbColor::white);
const GpuColor GpuColor::red(RgbColor::red);
const GpuColor GpuColor::green(RgbColor::green);
const GpuColor GpuColor::blue(RgbColor::blue);
const GpuColor GpuColor::yellow(RgbColor::yellow);
const GpuColor GpuColor::magenta(RgbColor::magenta);
const GpuColor GpuColor::cyan(RgbColor::cyan);

GpuColor
GpuColor::mix(const RgbColor &color, double weight) const
{
    RgbColor mixedColor = RgbColor(*this).mix(color, weight);
    return GpuColor(mixedColor);
}

}
