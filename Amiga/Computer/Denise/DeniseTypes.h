// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DENISE_T_INC
#define _DENISE_T_INC

typedef struct
{
    uint16_t pos;
    uint16_t ctl;
    uint16_t ptr;
    int16_t hstrt;
    int16_t vstrt;
    int16_t vstop;
    bool attach;
}
SpriteInfo;

typedef struct
{
    uint16_t bplcon0;
    uint16_t bplcon1;
    uint16_t bplcon2;
    uint16_t bpldat[6];
    uint32_t color[32];
    
    SpriteInfo sprite[8];
}
DeniseInfo;

#endif
