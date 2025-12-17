// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystem.h"

namespace vamiga {

FSBlockType
FileSystem::typeOf(Block nr) const noexcept
{
    return storage.getType(nr);
}

FSItemType
FileSystem::typeOf(Block nr, isize pos) const noexcept
{
    return storage.read(nr) ? storage[nr].itemType(pos) : FSItemType::UNUSED;
}

FSFormat
FileSystem::predictDOS(BlockView &dev) noexcept
{
    if (auto *blk = dev.readBlock(0)) {

        if (strncmp((const char *)blk->ptr, "DOS", 3) == 0 || blk->ptr[3] <= 7) {
            return FSFormat(blk->ptr[3]);
        }
    }
    return FSFormat::NODOS;
}

FSBlockType
FileSystem::predictType(FSDescriptor &layout, Block nr, const u8 *buf) noexcept
{
    if (buf) {

        // Is it a boot block?
        if (nr == 0 || nr == 1) return FSBlockType::BOOT;

        // Is it a bitmap block?
        if (std::find(layout.bmBlocks.begin(), layout.bmBlocks.end(), nr) != layout.bmBlocks.end())
            return FSBlockType::BITMAP;

        // Is it a bitmap extension block?
        if (std::find(layout.bmExtBlocks.begin(), layout.bmExtBlocks.end(), nr) != layout.bmExtBlocks.end())
            return FSBlockType::BITMAP_EXT;

        // For all other blocks, check the type and subtype fields
        u32 type = FSBlock::read32(buf);
        u32 subtype = FSBlock::read32(buf + layout.bsize - 4);

        if (type == 2  && subtype == 1)       return FSBlockType::ROOT;
        if (type == 2  && subtype == 2)       return FSBlockType::USERDIR;
        if (type == 2  && subtype == (u32)-3) return FSBlockType::FILEHEADER;
        if (type == 16 && subtype == (u32)-3) return FSBlockType::FILELIST;

        // Check if this block is a data block
        if (isOFSVolumeType(layout.dos)) {
            if (type == 8) return FSBlockType::DATA_OFS;
        } else {
            for (isize i = 0; i < layout.bsize; i++) if (buf[i]) return FSBlockType::DATA_FFS;
        }
    }

    return FSBlockType::EMPTY;
}

FSBlockType
FileSystem::predictType(Block nr, const u8 *buf) const noexcept
{
    if (buf) {

        // Is it a boot block?
        if (nr == 0 || nr == 1) return FSBlockType::BOOT;

        // Is it a bitmap block?
        if (std::find(bmBlocks.begin(), bmBlocks.end(), nr) != bmBlocks.end())
            return FSBlockType::BITMAP;

        // Is it a bitmap extension block?
        if (std::find(bmExtBlocks.begin(), bmExtBlocks.end(), nr) != bmExtBlocks.end())
            return FSBlockType::BITMAP_EXT;

        // For all other blocks, check the type and subtype fields
        u32 type = FSBlock::read32(buf);
        u32 subtype = FSBlock::read32(buf + traits.bsize - 4);

        if (type == 2  && subtype == 1)       return FSBlockType::ROOT;
        if (type == 2  && subtype == 2)       return FSBlockType::USERDIR;
        if (type == 2  && subtype == (u32)-3) return FSBlockType::FILEHEADER;
        if (type == 16 && subtype == (u32)-3) return FSBlockType::FILELIST;

        // Check if this block is a data block
        if (traits.ofs()) {
            if (type == 8) return FSBlockType::DATA_OFS;
        } else {
            for (isize i = 0; i < traits.bsize; i++) if (buf[i]) return FSBlockType::DATA_FFS;
        }
    }
    
    return FSBlockType::EMPTY;
}

FSBlock *
FileSystem::read(Block nr) noexcept
{
    return storage.read(nr);
}

FSBlock *
FileSystem::read(Block nr, FSBlockType type) noexcept
{
    return storage.read(nr, type);
}

FSBlock *
FileSystem::read(Block nr, std::vector<FSBlockType> types) noexcept
{
    return storage.read(nr, types);
}

const FSBlock *
FileSystem::read(Block nr) const noexcept
{
    return storage.read(nr);
}

const FSBlock *
FileSystem::read(Block nr, FSBlockType type) const noexcept
{
    return storage.read(nr, type);
}

const FSBlock *
FileSystem::read(Block nr, std::vector<FSBlockType> types) const noexcept
{
    return storage.read(nr, types);
}

FSBlock &
FileSystem::at(Block nr)
{
    return storage.at(nr);
}

FSBlock &
FileSystem::at(Block nr, FSBlockType type)
{
    return storage.at(nr, type);
}

FSBlock &
FileSystem::at(Block nr, std::vector<FSBlockType> types)
{
    return storage.at(nr, types);
}

const FSBlock &
FileSystem::at(Block nr) const
{
    return storage.at(nr);
}

const FSBlock &
FileSystem::at(Block nr, FSBlockType type) const
{
    return storage.at(nr, type);
}

const FSBlock &
FileSystem::at(Block nr, std::vector<FSBlockType> types) const
{
    return storage.at(nr, types);
}

void
FileSystem::flush()
{
    storage.flush();
}

FSBlock &
FileSystem::operator[](size_t nr)
{
    return storage[nr];
}

const FSBlock &
FileSystem::operator[](size_t nr) const
{
    return storage[nr];
}

}
