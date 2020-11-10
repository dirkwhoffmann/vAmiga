// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSEmptyBlock::FSEmptyBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
}

FSEmptyBlock::~FSEmptyBlock()
{
}
