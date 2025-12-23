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
    return exportBlocks(0, (BlockNr)(fs.blocks() - 1), dst, size);
}

bool
FSExporter::exportVolume(u8 *dst, isize size, FSFault *err) const
{
    return exportBlocks(0, (BlockNr)(fs.blocks() - 1), dst, size, err);
}

bool
FSExporter::exportBlock(BlockNr nr, u8 *dst, isize size) const
{
    return exportBlocks(nr, nr, dst, size);
}

bool
FSExporter::exportBlock(BlockNr nr, u8 *dst, isize size, FSFault *error) const
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
FSExporter::exportBlocks(BlockNr first, BlockNr last, u8 *dst, isize size) const
{
    FSFault error;
    bool result = exportBlocks(first, last, dst, size, &error);

    assert(result == (error == FSError::FS_OK));
    return result;
}

bool
FSExporter::exportBlocks(BlockNr first, BlockNr last, u8 *dst, isize size, FSFault *err) const
{
    assert(last < (BlockNr)fs.blocks());
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
    for (BlockNr nr = first; nr <= last; nr++) {

        fs.fetch(nr).exportBlock(dst + (nr - first) * traits.bsize, traits.bsize);
    }

    debug(FS_DEBUG, "Success\n");
    if (err) *err = FSError::FS_OK;
    return true;
}

void
FSExporter::exportBlock(BlockNr nr, const fs::path &path) const
{
    exportBlocks(nr, nr, path);
}

void
FSExporter::exportBlocks(BlockNr first, BlockNr last, const fs::path &path) const
{
    std::ofstream stream(path, std::ios::binary);

    if (!stream.is_open()) {
        throw IOError(IOError::FILE_CANT_CREATE, path);
    }

    for (BlockNr i = first; i <= last; i++) {

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
        exportBlocks(0, BlockNr(traits.blocks - 1), path);
    }
}

void
FSExporter::exportFiles(BlockNr nr, const fs::path &path, bool recursive, bool contents) const
{
    auto &item = fs.fetch(nr);
    fs::path hostPath;

    if (item.isDirectory()) {

        hostPath = contents ? path : path / item.cppName();
        if (!fs::exists(hostPath)) fs::create_directories(hostPath);

    } else if (item.isFile())  {

        hostPath = fs::is_directory(path) ? path / item.cppName() : path;
    }

    debug(FS_DEBUG, "Exporting %s to %s\n", item.absName().c_str(), hostPath.string().c_str());

    auto newTree = fs.build(nr, { .depth = recursive ? MAX_ISIZE : 1 });
    save(newTree, hostPath, recursive);
}

void
FSExporter::exportFiles(const fs::path &path, bool recursive, bool contents) const
{
    exportFiles(fs.pwd(), path, recursive, contents);
}

void
FSExporter::save(const FSTree &tree, const fs::path &path, bool recursive) const
{
    auto &node = fs.fetch(tree.nr);

    if (node.isDirectory()) {

        if (fs::exists(path)) {

            if (!fs::is_directory(path)) {
                throw FSError(FSError::FS_NOT_A_DIRECTORY, path.string());
            }
            if (!fs::is_empty(path)) {
                throw FSError(FSError::FS_DIR_NOT_EMPTY, path.string());
            }

        } else {

            fs::create_directories(path);
        }
        saveDir(tree, path, recursive);
    }

    if (node.isFile()) {

        if (fs::exists(path)) {
            throw FSError(FSError::FS_EXISTS, path.string());
        }
        saveFile(tree, path, recursive);
    }
}

void
FSExporter::saveDir(const FSTree &tree, const fs::path &path, bool recursive) const
{
    // Save files
    for (auto &it : tree.children) {

        auto &node = fs.fetch(it.nr);
        if (!node.isFile()) continue;
        node.exportBlock(path / node.name().path());
    }

    if (!recursive) return;

    // Save directories
    for (auto &it : tree.children) {

        auto &node = fs.fetch(it.nr);
        if (!node.isDirectory()) continue;
        save(it, path / node.name().path(), recursive);
    }
}

void
FSExporter::saveFile(const FSTree &tree, const fs::path &path, bool recursive) const
{
    auto &node = fs.fetch(tree.nr);

    // Get data
    Buffer<u8> buffer; node.extractData(buffer);

    // Open file
    std::ofstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        throw IOError(IOError::FILE_CANT_CREATE, path);
    }

    // Write data
    stream.write((const char *)buffer.ptr, buffer.size);
    if (!stream) {
        throw IOError(IOError::FILE_CANT_WRITE, path);
    }
}

}
