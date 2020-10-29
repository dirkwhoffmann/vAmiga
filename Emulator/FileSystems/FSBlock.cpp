// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

u32
FSBlock::checksum(u8 *p)
{
    assert(p != nullptr);
    
    u32 result = 0;

    for (int i = 0; i < 512; i += 4, p += 4) {
        result += HI_HI_LO_LO(p[0], p[1], p[2], p[3]);
    }
    
    return ~result + 1;
}

void
FSBlock::write32(u8 *p, u32 value)
{
    p[0] = (value >> 24) & 0xFF;
    p[1] = (value >> 16) & 0xFF;
    p[2] = (value >>  8) & 0xFF;
    p[3] = (value >>  0) & 0xFF;
}

void
FSBlock::write(u8 *p)
{
    memset(p, 0, volume.bsize);
}

bool
FSBlock::assertNotNull(u32 ref, bool verbose)
{
    if (ref != 0) return true;
    
    if (verbose) fprintf(stderr, "Block reference is missing.\n");
    return false;
}

bool
FSBlock::assertInRange(u32 ref, bool verbose)
{
    if (volume.isBlockNumber(ref)) return true;

    if (verbose) fprintf(stderr, "Block reference %d is invalid\n", ref);
    return false;
}

bool
FSBlock::assertHasType(u32 ref, FSBlockType type, bool verbose)
{
    return assertHasType(ref, type, type, verbose);
}

bool
FSBlock::assertHasType(u32 ref, FSBlockType type1, FSBlockType type2, bool verbose)
{
    assert(isFSBlockType(type1));
    assert(isFSBlockType(type2));

    FSBlock *block = volume.block(ref);
    FSBlockType type = block ? block->type() : FS_EMPTY_BLOCK;
    
    if (!isFSBlockType(type)) {
        if (verbose) fprintf(stderr, "Block type %d is not a known type.\n", type);
        return false;
    }
    
    if (block && (type == type1 || type == type2)) return true;
    
    if (verbose && type1 == type2) {
        fprintf(stderr, "Block %d has type %s. Expected %s.\n",
                ref,
                fsBlockTypeName(type),
                fsBlockTypeName(type1));
    }
    
    if (verbose && type1 != type2) {
        fprintf(stderr, "Block %d has type %s. Expected %s or %s.\n",
                ref,
                fsBlockTypeName(type),
                fsBlockTypeName(type1),
                fsBlockTypeName(type2));
    }

    return false;
}

bool
FSBlock::assertSelfRef(u32 ref, bool verbose)
{
    if (ref == nr && volume.block(ref) == this) return true;

    if (ref != nr && verbose) {
        fprintf(stderr, "%d is not a self-reference.\n", ref);
    }
    
    if (volume.block(ref) != this && verbose) {
        fprintf(stderr, "Array element %d references an invalid block\n", ref);
    }
    
    return false;
}

bool
FSBlock::check(bool verbose)
{
    printf("FSBlock::check(%d)\n", verbose);
    return assertSelfRef(nr, verbose);
}
