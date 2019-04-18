// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This include file contains all type definitions related to Denise.
// It conforms to standard ANSI-C in order to be compatible with Swift.

#ifndef _AGNUS_T_INC
#define _AGNUS_T_INC

typedef enum
{
    SPR_DMA_IDLE,
    SPR_DMA_DATA, 
    
    SPR_FETCH_CONFIG, // DEPRECATED
    SPR_WAIT_VSTART, // DEPRECATED
    SPR_FETCH_DATA // DEPRECATED
}
SprDMAState;

#endif
