// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

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

}

// Make types visible in subspaces
namespace utl::abilities   { using namespace utl::types; }
namespace utl::animation   { using namespace utl::types; }
namespace utl::chrono      { using namespace utl::types; }
namespace utl::concurrency { using namespace utl::types; }
namespace utl::storage     { using namespace utl::types; }
namespace utl::support     { using namespace utl::types; }
