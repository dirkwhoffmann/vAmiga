// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <limits>

namespace utl {

// Signed integers
typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;
typedef signed long        isize;

static constexpr isize MIN_I8    = std::numeric_limits<i8>::min();
static constexpr isize MIN_I16   = std::numeric_limits<i16>::min();
static constexpr isize MIN_I32   = std::numeric_limits<i32>::min();
static constexpr isize MIN_I64   = std::numeric_limits<i64>::min();
static constexpr isize MIN_ISIZE = std::numeric_limits<isize>::min();

static constexpr isize MAX_I8    = std::numeric_limits<i8>::max();
static constexpr isize MAX_I16   = std::numeric_limits<i16>::max();
static constexpr isize MAX_I32   = std::numeric_limits<i32>::max();
static constexpr isize MAX_I64   = std::numeric_limits<i64>::max();
static constexpr isize MAX_ISIZE = std::numeric_limits<isize>::max();

// Unsigned integers
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef unsigned long      usize;

static constexpr isize MIN_U8    = std::numeric_limits<u8>::min();
static constexpr isize MIN_U16   = std::numeric_limits<u16>::min();
static constexpr isize MIN_U32   = std::numeric_limits<u32>::min();
static constexpr isize MIN_U64   = std::numeric_limits<u64>::min();
static constexpr isize MIN_USIZE = std::numeric_limits<usize>::min();

static constexpr isize MAX_U8    = std::numeric_limits<u8>::max();
static constexpr isize MAX_U16   = std::numeric_limits<u16>::max();
static constexpr isize MAX_U32   = std::numeric_limits<u32>::max();
static constexpr isize MAX_U64   = std::numeric_limits<u64>::max();
static constexpr isize MAX_USIZE = std::numeric_limits<usize>::max();

}
