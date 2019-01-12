// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef AMIGA_TYPES_H
#define AMIGA_TYPES_H

#include "VAFile_types.h"

// DEPRECATED
#include "C64_types.h"

typedef enum {
    AMIGA_500,
    AMIGA_1000,
    AMIGA_2000
} AmigaModel;

inline bool isAmigaModel(AmigaModel model) {
    return model >= AMIGA_500 && model <= AMIGA_2000;
}

#endif
