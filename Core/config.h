// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "debug.h"

//
// Release settings
//

// Version number
static constexpr int VER_MAJOR      = 4;
static constexpr int VER_MINOR      = 4;
static constexpr int VER_SUBMINOR   = 0;
static constexpr int VER_BETA       = 7;

// Snapshot version number
static constexpr int SNP_MAJOR      = 4;
static constexpr int SNP_MINOR      = 4;
static constexpr int SNP_SUBMINOR   = 0;
static constexpr int SNP_BETA       = 7;


//
// Video settings
//

/* Texels per pixel. Set to 1 to create a texture in hires resolution where
 * every hires pixel is represented by a single texel). Set to 2 to generate a
 * texture in super-hires resolution where every hires pixel is represented by
 * two texels.
 */
#define TPP 1


//
// Execution settings
//

static constexpr int DIAG_BOARD       = 0; // Plug in the diagnose board
static constexpr int ALLOW_ALL_ROMS   = 1; // Disable the magic bytes check


//
// Debug settings
//

static constexpr bool betaRelease = VER_BETA != 0;

#ifdef NDEBUG
static constexpr bool releaseBuild = 1;
static constexpr bool debugBuild = 0;
// typedef const long debugflag;
#else
static constexpr bool releaseBuild = 0;
static constexpr bool debugBuild = 1;
// typedef long debugflag;
#endif

#ifdef __APPLE__
static constexpr bool macOS = 1;
#else
static constexpr bool macOS = 0;
#endif

#ifdef __EMSCRIPTEN__
static constexpr bool wasmBuild = 1;
#else
static constexpr bool wasmBuild = 0;
#endif

#ifdef VAMIGA_DOS
static constexpr bool vAmigaDOS = 1;
#else
static constexpr bool vAmigaDOS = 0;
#endif
