// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Snapshot.h"
#include "Amiga.h"
#include "IOUtils.h"

namespace vamiga {

void
Thumbnail::take(Amiga &amiga, isize dx, isize dy)
{
    isize xStart = 4 * HBLANK_CNT;
    isize xEnd = 4 * HPOS_CNT_PAL;
    isize yStart = VBLANK_CNT;
    isize yEnd = amiga.agnus.isPAL() ? VPOS_CNT_PAL_SF : VPOS_CNT_NTSC_SF;

    width  = (i32)((xEnd - xStart) / dx);
    height = (i32)((yEnd - yStart) / dy);

    auto *target = screen;
    auto *source = amiga.denise.pixelEngine.stablePtr();
    source += xStart + yStart * HPIXELS;

    for (isize y = 0; y < height; y++) {
        for (isize x = 0; x < width; x++) {
            target[x] = u32(source[x * dx]);
        }
        source += dy * HPIXELS;
        target += width;
    }

    timestamp = time(nullptr);
}

bool
Snapshot::isCompatible(const string &path)
{
    return true;
}

bool
Snapshot::isCompatible(std::istream &stream)
{
    const u8 magicBytes[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    if (util::streamLength(stream) < 0x15) return false;
    return util::matchingStreamHeader(stream, magicBytes, sizeof(magicBytes));
}

Snapshot::Snapshot(isize capacity)
{
    u8 signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    data.init(capacity + sizeof(SnapshotHeader));
    
    SnapshotHeader *header = (SnapshotHeader *)data.ptr;
    
    for (isize i = 0; i < isizeof(signature); i++)
        header->magic[i] = signature[i];
    header->major = SNP_MAJOR;
    header->minor = SNP_MINOR;
    header->subminor = SNP_SUBMINOR;
    header->beta = SNP_BETA;
}

Snapshot::Snapshot(Amiga &amiga) : Snapshot(amiga.size())
{
    takeScreenshot(amiga);
    amiga.save(getData());
}

void
Snapshot::finalizeRead()
{
    if (FORCE_SNAP_TOO_OLD) throw VAError(ERROR_SNAP_TOO_OLD);
    if (FORCE_SNAP_TOO_NEW) throw VAError(ERROR_SNAP_TOO_NEW);
    if (FORCE_SNAP_IS_BETA) throw VAError(ERROR_SNAP_IS_BETA);

    if (isTooOld()) throw VAError(ERROR_SNAP_TOO_OLD);
    if (isTooNew()) throw VAError(ERROR_SNAP_TOO_NEW);
    if (isBeta() && !betaRelease) throw VAError(ERROR_SNAP_IS_BETA);
}

bool
Snapshot::isTooOld() const
{
    auto header = getHeader();
    
    if (header->major < SNP_MAJOR) return true;
    if (header->major > SNP_MAJOR) return false;
    if (header->minor < SNP_MINOR) return true;
    if (header->minor > SNP_MINOR) return false;
    
    return header->subminor < SNP_SUBMINOR;
}

bool
Snapshot::isTooNew() const
{
    auto header = getHeader();
    
    if (header->major > SNP_MAJOR) return true;
    if (header->major < SNP_MAJOR) return false;
    if (header->minor > SNP_MINOR) return true;
    if (header->minor < SNP_MINOR) return false;

    return header->subminor > SNP_SUBMINOR;
}

bool
Snapshot::isBeta() const
{
    auto header = getHeader();

    return header->beta != 0;
}

void
Snapshot::takeScreenshot(Amiga &amiga)
{
    ((SnapshotHeader *)data.ptr)->screenshot.take(amiga);
}

}
