// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
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
Snapshot::isCompatible(const fs::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".VASNAP" && util::matchingFileHeader(path, "VASNAP");
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

Snapshot::Snapshot(Amiga &amiga, Compressor compressor) : Snapshot(amiga)
{
    compress(compressor);
}

void
Snapshot::finalizeRead()
{
    if (FORCE_SNAP_TOO_OLD) throw CoreError(Fault::SNAP_TOO_OLD);
    if (FORCE_SNAP_TOO_NEW) throw CoreError(Fault::SNAP_TOO_NEW);
    if (FORCE_SNAP_IS_BETA) throw CoreError(Fault::SNAP_IS_BETA);

    if (isTooOld()) throw CoreError(Fault::SNAP_TOO_OLD);
    if (isTooNew()) throw CoreError(Fault::SNAP_TOO_NEW);
    if (isBeta() && !betaRelease) throw CoreError(Fault::SNAP_IS_BETA);
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
Snapshot::compress(Compressor compressor)
{
    debug(SNP_DEBUG, "compress(%s)\n", CompressorEnum::key(compressor));

    if (!isCompressed()) {

        debug(SNP_DEBUG, "Compressing %ld bytes (hash: 0x%x)...", data.size, data.fnv32());

        {   auto watch = util::StopWatch(SNP_DEBUG, "");
            
            switch (compressor) {
                    
                case Compressor::NONE:  break;
                case Compressor::GZIP:  data.gzip(sizeof(SnapshotHeader)); break;
                case Compressor::LZ4:   data.lz4 (sizeof(SnapshotHeader)); break;
                case Compressor::RLE2:  data.rle2(sizeof(SnapshotHeader)); break;
                case Compressor::RLE3:  data.rle3(sizeof(SnapshotHeader)); break;
            }
            
            getHeader()->compressor = u8(compressor);
        }
        debug(SNP_DEBUG, "Compressed size: %ld bytes\n", data.size);
    }
}
void
Snapshot::uncompress()
{
    debug(SNP_DEBUG, "uncompress(%s)\n", CompressorEnum::key(compressor()));

    if (isCompressed()) {
        
        isize expectedSize = getHeader()->rawSize;
        
        debug(SNP_DEBUG, "Uncompressing %ld bytes...", data.size);
        
        {   auto watch = util::StopWatch(SNP_DEBUG, "");
        
            switch (compressor()) {
                    
                case Compressor::NONE:  break;
                case Compressor::GZIP:  data.gunzip(sizeof(SnapshotHeader), expectedSize); break;
                case Compressor::LZ4:   data.unlz4 (sizeof(SnapshotHeader), expectedSize); break;
                case Compressor::RLE2:  data.unrle2(sizeof(SnapshotHeader), expectedSize); break;
                case Compressor::RLE3:  data.unrle3(sizeof(SnapshotHeader), expectedSize); break;
            }
            
            getHeader()->compressor = u8(Compressor::NONE);
        }
        debug(SNP_DEBUG, "Uncompressed size: %ld bytes (hash: 0x%x)\n", data.size, data.fnv32());
        
        if (getHeader()->rawSize != expectedSize) {
         
            warn("Snaphot size: %ld. Expected: %ld\n", data.size, expectedSize);
            fatalError;
        }
    }
}

}
