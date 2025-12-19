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
#include "utl/io.h"
#include <fstream>

namespace vamiga {

using namespace utl;

bool
FSExporter::exportVolume(u8 *dst, isize size) const
{
    return exportBlocks(0, (Block)(fs.blocks() - 1), dst, size);
}

bool
FSExporter::exportVolume(u8 *dst, isize size, FSFault *err) const
{
    return exportBlocks(0, (Block)(fs.blocks() - 1), dst, size, err);
}

bool
FSExporter::exportBlock(Block nr, u8 *dst, isize size) const
{
    return exportBlocks(nr, nr, dst, size);
}

bool
FSExporter::exportBlock(Block nr, u8 *dst, isize size, FSFault *error) const
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
FSExporter::exportBlocks(Block first, Block last, u8 *dst, isize size) const
{
    FSFault error;
    bool result = exportBlocks(first, last, dst, size, &error);

    assert(result == (error == FSError::FS_OK));
    return result;
}

bool
FSExporter::exportBlocks(Block first, Block last, u8 *dst, isize size, FSFault *err) const
{
    assert(last < (Block)fs.blocks());
    assert(first <= last);
    assert(dst);

    isize count = last - first + 1;

    debug(FS_DEBUG, "Exporting %ld blocks (%d - %d)\n", count, first, last);

    // Only proceed if the (predicted) block size matches
    if (size % traits.bsize != 0) {
        if (err) *err = FSError::FS_WRONG_BSIZE;
        return false;
    }

    // Only proceed if the source buffer contains the right amount of data
    if (count * traits.bsize != size) {
        if (err) *err = FSError::FS_WRONG_CAPACITY;
        return false;
    }

    // Wipe out the target buffer
    std::memset(dst, 0, size);

    // Export all blocks
    for (auto &block: cache.keys(first, last)) {

        cache.tryFetch(block)->exportBlock(dst + (block - first) * traits.bsize, traits.bsize);
    }

    debug(FS_DEBUG, "Success\n");
    if (err) *err = FSError::FS_OK;
    return true;
}

void
FSExporter::exportBlock(Block nr, const fs::path &path) const
{
    exportBlocks(nr, nr, path);
}

void
FSExporter::exportBlocks(Block first, Block last, const fs::path &path) const
{
    std::ofstream stream(path, std::ios::binary);

    if (!stream.is_open()) {
        throw IOError(IOError::FILE_CANT_CREATE, path);
    }

    for (Block i = first; i <= last; i++) {

        auto *data = fs.fetch(i).data();
        stream.write((const char *)data, traits.bsize);
    }

    if (!stream) {
        throw IOError(IOError::FILE_CANT_WRITE, path);
    }
}

void
FSExporter::exportBlocks(const fs::path &path) const
{
    if (traits.blocks) {
        exportBlocks(0, Block(traits.blocks - 1), path);
    }
}

void
FSExporter::exportFiles(Block nr, const fs::path &path, bool recursive, bool contents) const
{
    auto *block = fs.tryFetch(nr);
    exportFiles(*block, path, recursive, contents);

}

void
FSExporter::exportFiles(const FSBlock &item, const fs::path &path, bool recursive, bool contents) const
{
    fs::path hostPath;

    if (item.isDirectory()) {

        hostPath = contents ? path : path / item.cppName();
        if (!fs::exists(hostPath)) fs::create_directories(hostPath);

    } else if (item.isFile())  {

        hostPath = fs::is_directory(path) ? path / item.cppName() : path;
    }

    debug(FS_DEBUG, "Exporting %s to %s\n", item.absName().c_str(), hostPath.string().c_str());
    FSTree tree(item, { .recursive = recursive });
    tree.save(hostPath, { .recursive = recursive });
}

void
FSExporter::exportFiles(const fs::path &path, bool recursive, bool contents) const
{
    exportFiles(fs.pwd(), path, recursive, contents);
}

}
