// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace utl::types {

// Signed integers
typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;
typedef signed long        isize;

// Unsigned integers
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef unsigned long      usize;

namespace fs = std::filesystem;

using std::string;
using std::optional;
using std::unordered_map;
using std::vector;

//
// Converting data types
//

// Signed alternative for the sizeof keyword
template <typename T>
constexpr isize isizeof(const T&) noexcept {
    return static_cast<isize>(sizeof(T));
}

template <typename T>
constexpr isize isizeof() noexcept {
    return static_cast<isize>(sizeof(T));
}

}
