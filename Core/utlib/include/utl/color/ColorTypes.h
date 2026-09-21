// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

namespace utl {

/* Byte layout of a 32-bit pixel (see GpuColor<F> in Colors.h).
 *
 * A plain enum, unlike a core's own option enums: utlib doesn't use the
 * Reflectable-style enum/help/key machinery those need for their Option
 * systems (see e.g. vamiga::TexFormat / vamiga::TexFormatEnum in
 * HostTypes.h). A core that stores this as a persisted, UI-facing option
 * keeps its own Reflectable-paired enum with the same three values in the
 * same order, so a plain static_cast moves a value between the two.
 */
enum class TexelFormat
{
    ABGR,                       // AABBGGRR
    ARGB,                       // AARRGGBB
    RGBA                        // RRGGBBAA
};

}
