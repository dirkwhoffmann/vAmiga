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

#include "utl/common.h"
#include "utl/abilities/Reflectable.h"
#include "utl/abilities/Compressible.h"
#include <sys/types.h>
#include <stdint.h>

//
// Integers
//

namespace vamiga {

using utl::i8;
using utl::i16;
using utl::i32;
using utl::i64;
using utl::isize;
using utl::u8;
using utl::u16;
using utl::u32;
using utl::u64;
using utl::usize;

// Types that appear in the signatures of the API
using utl::Error;
using utl::Compressor;

/*
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
*/

}


//
// Pointers
//

#include <memory>
namespace vamiga { using std::unique_ptr, std::make_unique; }

//
// Strings
//

#include <string>
#include <cstring>
namespace vamiga { using std::string; }


//
// Data structures
//

#include <vector>
namespace vamiga { using std::vector; }


//
// Optionals
//

#include <optional>
namespace vamiga { using std::optional; }


//
// Filesystem
//

#include <filesystem>
namespace vamiga { namespace fs = std::filesystem; }
