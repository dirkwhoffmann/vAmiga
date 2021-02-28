// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "GenericTypes.h"
#include "AmigaConstants.h"
#include "Aliases.h"
#include "Debug.h"


//
// Optimizing code
//

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)


//
// Sanitizer macros
//

#if defined(__clang__)

// Definition for clang
#define NO_SANITIZE(x) __attribute__((no_sanitize(x)))

#else

// Definition for gcc
#define NO_SANITIZE(x)

#endif
