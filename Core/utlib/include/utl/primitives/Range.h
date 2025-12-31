// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"

namespace utl {

template <typename T>
struct Range {

    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,
                  "Range<T> requires an ordered scalar type");

    // Bounds in [lower; upper) format
    T lower{}, upper{};

    constexpr bool valid() const noexcept {
        return lower >= 0 && lower <= upper;
    }

    constexpr bool inside(isize min, isize max) const noexcept {
        return lower >= min && lower <= upper && upper <= max;
    }

    constexpr bool contains(T value) const noexcept {
        return value >= lower && value < upper;
    }

    constexpr T size() const noexcept {
        return upper - lower;
    }

    constexpr T translate(T offset) const {

        if (offset >= T{} && offset < size()) return lower + offset;
        throw Error(offset, "Range offset out of bounds");
    }
};

template <typename T>
struct ClosedRange {

    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,
                  "Range<T> requires an ordered scalar type");

    // Bounds in [lower; upper] format
    T lower{}, upper{};

    constexpr bool valid() const noexcept {
        return lower <= upper;
    }

    constexpr bool contains(T value) const noexcept {
        return value >= lower && value <= upper;
    }

    constexpr T size() const noexcept {
        return upper - lower + T{1};
    }

    constexpr T translate(T offset) const {

        if (offset >= T{} && offset <= size()) return lower + offset;
        throw Error(offset, "Range offset out of bounds");
    }
};

}
