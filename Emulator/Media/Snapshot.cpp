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
    isize xEnd = 4 * PAL::HPOS_CNT;
    isize yStart = amiga.agnus.isPAL() ? PAL::VBLANK_CNT : NTSC::VBLANK_CNT;
    isize yEnd = amiga.agnus.isPAL() ? PAL::VPOS_CNT_SF : NTSC::VPOS_CNT_SF;

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
Snapshot::isCompatible(const std::filesystem::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".VAMIGA" && util::matchingFileHeader(path, "VASNAP");
}

bool
Snapshot::isCompatible(const u8 *buf, isize len)
{
    if (len < isizeof(SnapshotHeader)) return false;
    return util::matchingBufferHeader(buf, "VASNAP");
}

bool
Snapshot::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
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
    header->rawSize = i32(data.size);
}

Snapshot::Snapshot(Amiga &amiga) : Snapshot(amiga.size())
{
    {   util::StopWatch(SNP_DEBUG, "Taking screenshot...");
        
        takeScreenshot(amiga);
    }
    {   util::StopWatch(SNP_DEBUG, "Saving state...");
        
        amiga.save(getData());
    }
}

void
Snapshot::finalizeRead()
{
    if (FORCE_SNAP_TOO_OLD) throw Error(ErrorCode::SNAP_TOO_OLD);
    if (FORCE_SNAP_TOO_NEW) throw Error(ErrorCode::SNAP_TOO_NEW);
    if (FORCE_SNAP_IS_BETA) throw Error(ErrorCode::SNAP_IS_BETA);

    if (isTooOld()) throw Error(ErrorCode::SNAP_TOO_OLD);
    if (isTooNew()) throw Error(ErrorCode::SNAP_TOO_NEW);
    if (isBeta() && !betaRelease) throw Error(ErrorCode::SNAP_IS_BETA);
}

std::pair <isize,isize>
Snapshot::previewImageSize() const
{
    return { getThumbnail().width, getThumbnail().height };
}

const u32 *
Snapshot::previewImageData() const
{
    return getThumbnail().screen;
}

time_t
Snapshot::timestamp() const
{
    return getThumbnail().timestamp;
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
    return getHeader()->beta != 0;
}

void
Snapshot::takeScreenshot(Amiga &amiga)
{
    ((SnapshotHeader *)data.ptr)->screenshot.take(amiga);
}

void
Snapshot::compress()
{
    if (!isCompressed()) {

        debug(SNP_DEBUG, "Compressing %ld bytes (hash: 0x%x)...", data.size, data.fnv32());

        {   auto watch = util::StopWatch(SNP_DEBUG, "");
            
            data.compress(2, sizeof(SnapshotHeader));
            getHeader()->compressed = true;
        }
        debug(SNP_DEBUG, "Compressed size: %ld bytes\n", data.size);
    }
}
void
Snapshot::uncompress()
{
    if (isCompressed()) {
        
        isize expectedSize = getHeader()->rawSize;
        
        debug(SNP_DEBUG, "Uncompressing %ld bytes...", data.size);
        
        {   auto watch = util::StopWatch(SNP_DEBUG, "");
            
            data.uncompress(2, sizeof(SnapshotHeader), expectedSize);
            getHeader()->compressed = false;
        }
        debug(SNP_DEBUG, "Uncompressed size: %ld bytes (hash: 0x%x)\n", data.size, data.fnv32());
        
        if (getHeader()->rawSize != expectedSize) {
         
            warn("Snaphot size: %ld. Expected: %ld\n", data.size, expectedSize);
            fatalError;
        }
    }
}

}
