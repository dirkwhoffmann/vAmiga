// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Snapshot.h"
#include "Amiga.h"
#include "IOUtils.h"

void
Thumbnail::take(Amiga &amiga, isize dx, isize dy)
{
    u32 *source = (u32 *)amiga.denise.pixelEngine.getStableBuffer().ptr;
    u32 *target = screen;
    
    isize xStart = 4 * HBLANK_MAX + 1, xEnd = HPIXELS + 4 * HBLANK_MIN;
    isize yStart = VBLANK_CNT, yEnd = VPIXELS - 2;
    
    width  = (i32)((xEnd - xStart) / dx);
    height = (i32)((yEnd - yStart) / dy);
    
    source += xStart + yStart * HPIXELS;
    
    for (isize y = 0; y < height; y++) {
        for (isize x = 0; x < width; x++) {
            target[x] = source[x * dx];
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
    if constexpr (FORCE_SNAP_TOO_OLD) throw VAError(ERROR_SNAP_TOO_OLD);
    if constexpr (FORCE_SNAP_TOO_NEW) throw VAError(ERROR_SNAP_TOO_NEW);
    if constexpr (FORCE_SNAP_IS_BETA) throw VAError(ERROR_SNAP_IS_BETA);

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
