/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Serializable.h"

namespace vamiga {

struct RgbColor;
struct YuvColor;
struct AmigaColor;
struct GpuColor;

struct RgbColor {

    double r;
    double g;
    double b;

    RgbColor() : r(0), g(0), b(0) {}
    RgbColor(double rv, double gv, double bv) : r(rv), g(gv), b(bv) {}
    RgbColor(u8 rv, u8 gv, u8 bv) : r(rv / 255.0), g(gv / 255.0), b(bv / 255.0) {}
    RgbColor(u32 rgba) : RgbColor(u8(rgba >> 24), u8(rgba >> 16), u8(rgba >> 8)) {}
    RgbColor(const YuvColor &c);
    RgbColor(const AmigaColor &c);
    RgbColor(const GpuColor &c);

    static const RgbColor black;
    static const RgbColor white;
    static const RgbColor red;
    static const RgbColor green;
    static const RgbColor blue;
    static const RgbColor yellow;
    static const RgbColor magenta;
    static const RgbColor cyan;

    bool operator==(const RgbColor &rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }

    RgbColor mix(RgbColor additive, double weight) const;
    RgbColor tint(double weight) const { return mix(white, weight); }
    RgbColor shade(double weight) const { return mix(black, weight); }
};

struct YuvColor {

    double y;
    double u;
    double v;

    YuvColor() : y(0), u(0), v(0) { }
    YuvColor(double yv, double uv, double vv) : y(yv), u(uv), v(vv) { }
    YuvColor(u8 yv, u8 uv, u8 vv) : y(yv / 255.0), u(uv / 255.0), v(vv / 255.0) { }
    YuvColor(const RgbColor &c);
    YuvColor(const AmigaColor &c) : YuvColor(RgbColor(c)) { }
    YuvColor(const GpuColor &c) : YuvColor(RgbColor(c)) { }

    static const YuvColor black;
    static const YuvColor white;
    static const YuvColor red;
    static const YuvColor green;
    static const YuvColor blue;
    static const YuvColor yellow;
    static const YuvColor magenta;
    static const YuvColor cyan;

    bool operator==(const YuvColor &rhs) const {
        return y == rhs.y && u == rhs.u && v == rhs.v;
    }
};

//
// Amiga color (native Amiga RGB format)
//

struct AmigaColor : SerializableStruct
{
    u8 r;
    u8 g;
    u8 b;

    // template <class W> void operator<<(W& worker) { worker << r << g << b; }

    AmigaColor() : r(0), g(0), b(0) {}
    AmigaColor(u8 rv, u8 gv, u8 bv) : r(rv & 0xF), g(gv & 0xF), b(bv & 0xF) {}
    AmigaColor(u16 rgb) : AmigaColor(u8(rgb >> 8), u8(rgb >> 4), u8(rgb)) {}
    AmigaColor(const RgbColor &c);
    AmigaColor(const YuvColor &c) : AmigaColor(RgbColor(c)) { }
    AmigaColor(const GpuColor &c);

    //
    // Methods from Serializable
    //

public:

    template <class T>
    void serialize(T& worker)
    {
        worker

        << r << g << b;

    } STRUCT_SERIALIZERS(serialize);



    u16 rawValue() const { return u16(r << 8 | g << 4 | b); }

    static const AmigaColor black;
    static const AmigaColor white;
    static const AmigaColor red;
    static const AmigaColor green;
    static const AmigaColor blue;
    static const AmigaColor yellow;
    static const AmigaColor magenta;
    static const AmigaColor cyan;

    bool operator==(const AmigaColor &rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }

    AmigaColor ehb() const;
    AmigaColor shr() const;
    AmigaColor mix(const AmigaColor &c) const;
};

//
// GPU color (native GPU RGBA format)
//

struct GpuColor {

    u32 rawValue;

    GpuColor() : rawValue(0) {}
    GpuColor(u32 v) : rawValue(v) {}
    GpuColor(u64 v) : rawValue(u32(v)) {}
    GpuColor(const RgbColor &c);
    GpuColor(const AmigaColor &c);
    GpuColor(u8 r, u8 g, u8 b);

    u8 r() const { return u8(rawValue       & 0xFF); }
    u8 g() const { return u8(rawValue >> 8  & 0xFF); }
    u8 b() const { return u8(rawValue >> 16 & 0xFF); }
    u8 a() const { return u8(rawValue >> 24 & 0xFF); }

    static const GpuColor black;
    static const GpuColor white;
    static const GpuColor red;
    static const GpuColor green;
    static const GpuColor blue;
    static const GpuColor yellow;
    static const GpuColor magenta;
    static const GpuColor cyan;

    bool operator==(const GpuColor &rhs) const {
        return rawValue == rhs.rawValue;
    }

    GpuColor mix(const RgbColor &color, double weight) const;
    GpuColor tint(double weight) const { return mix(RgbColor::white, weight); }
    GpuColor shade(double weight) const { return mix(RgbColor::black, weight); }
};

}
