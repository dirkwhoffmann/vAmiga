// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef VA_CONSTANTS_H
#define VA_CONSTANTS_H

// Clock rate of the master clock (PAL)
static const double masterClockFrequency = 28.37516;

// Clock rate of the Motorola 68000 CPU (7.09379 MHz)
static const double cpuClockFrequency = masterClockFrequency / 4.0;

// Frame rate (PAL)
// IS THIS 100% CORRECT? E.G., THE C64 DIFFERS SLIGHTLHY FROM 50
static const double frameRate = 50.0;

// Number of master clock cycles per frame
static const double masterCyclesPerFrame = (masterClockFrequency * 1000000.0) / frameRate;


// Horizontal screen parameters

//! @brief    Width of left VBLANK area in PAL mode
static const uint16_t PAL_LEFT_VBLANK = 76;

//! @brief    Width of left border in PAL mode
static const uint16_t PAL_LEFT_BORDER_WIDTH = 48;

//! @brief    Width of canvas area in PAL mode
static const uint16_t PAL_CANVAS_WIDTH = 320;

//! @brief    Width of right border in PAL mode
static const uint16_t PAL_RIGHT_BORDER_WIDTH = 37;

//! @brief    Width of right VBLANK area in PAL mode
static const uint16_t PAL_RIGHT_VBLANK = 23;

//! @brief    Total width of a rasterline (including VBLANK) in PAL mode
static const uint16_t PAL_WIDTH = 504; // 76 + 48 + 320 + 37 + 23


//! @brief    Number of viewable pixels per rasterline in PAL mode
// static const uint16_t PAL_VISIBLE_PIXELS = 403;


// Vertical screen parameters

//! @brief    Number of VBLANK lines at top in PAL mode
static const uint16_t PAL_UPPER_VBLANK = 16;

//! @brief    Heigt of upper boder in PAL mode
static const uint16_t PAL_UPPER_BORDER_HEIGHT = 35;

//! @brief    Height of canvas area in PAL mode
static const uint16_t PAL_CANVAS_HEIGHT = 200;

//! @brief    Lower border height in PAL mode
static const uint16_t PAL_LOWER_BORDER_HEIGHT = 49;

//! @brief    Number of VBLANK lines at bottom in PAL mode
static const uint16_t PAL_LOWER_VBLANK = 12;

//! @brief    Total height of a frame (including VBLANK) in PAL mode
static const uint16_t PAL_HEIGHT = 312; // 16 + 35 + 200 + 49 + 12

//! @brief    Number of drawn rasterlines per frame in PAL mode
static const uint16_t PAL_RASTERLINES = 284; // 35 + 200 + 49

//! @brief    Number of viewable rasterlines per frame in PAL mode
static const uint16_t PAL_VISIBLE_RASTERLINES = 284; // was 292

//! @brief    Number of drawn pixels per rasterline in PAL mode
static const uint16_t PAL_PIXELS = 405; // 48 + 320 + 37

//! @brief    Number of drawn pixels per rasterline in NTSC mode
static const uint16_t NTSC_PIXELS = 428; // 55 + 320 + 53


#endif 
