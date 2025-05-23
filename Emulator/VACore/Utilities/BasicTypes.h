// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <sys/types.h>
#include <stdint.h>


//
// Integers
//

namespace vamiga {

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
