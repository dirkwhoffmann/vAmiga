// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FileSystems/Amiga/PosixAdapter.h"

namespace retro::vault::amigafs {

PosixAdapter::PosixAdapter(FileSystem &fs) : fs(fs)
{

}

NodeMeta *
PosixAdapter::getMeta(BlockNr nr)
{
    auto it = meta.find(nr);
    return it == meta.end() ? nullptr : &it->second;
}

NodeMeta &
PosixAdapter::ensureMeta(BlockNr nr)
{
    auto [it, inserted] = meta.try_emplace(nr);
    return it->second;
}

NodeMeta &
PosixAdapter::ensureMeta(HandleRef ref)
{
    return ensureMeta(getHandle(ref).headerBlock);
}

FSPosixStat
PosixAdapter::stat() const noexcept
{
    return fs.stat();
}

FSPosixAttr
PosixAdapter::attr(const fs::path &path) const
{
    return fs.attr(fs.seek(path));
}

void
PosixAdapter::mkdir(const fs::path &path)
{
    auto parent = path.parent_path();
    auto name = path.filename();

    // Lookup destination directory
    auto node = fs.seek(parent);

    // Create directory
    auto udb = fs.mkdir(node, FSName(name));

    // Create meta info
    auto &info = ensureMeta(udb);
    info.linkCount = 1;
}

void
PosixAdapter::rmdir(const fs::path &path)
{
    // Lookup directory
    auto node = fs.seek(path);

    // Only empty directories can be removed
    require.emptyDirectory(node);

    if (auto *info = getMeta(node); info) {

        // Remove directory entry
        fs.unlink(node);

        // Decrement link count
        if (info->linkCount > 0) info->linkCount--;

        // Maybe delete
        tryReclaim(node);
    }
}

std::vector<string>
PosixAdapter::readDir(const fs::path &path) const
{
    std::vector<string> result;

    for (auto &it : fs.getItems(fs.seek(path))) {
        result.push_back(fs.fetch(it).cppName());
    }

    return result;
}

HandleRef
PosixAdapter::open(const fs::path &path, u32 flags)
{
    // Resolve path
    auto node = fs.seek(path);

    // Create a unique identifier
    auto ref = nextHandle++;

    // Create a new file handle
    handles[ref] = Handle {

        .id = ref,
        .headerBlock = node,
        .offset = 0,
        .flags = flags
    };
    auto &handle = handles[ref];
    auto &info = ensureMeta(node);
    info.openHandles.insert(ref);

    // Evaluate flags
    if ((flags & O_TRUNC) && (flags & (O_WRONLY | O_RDWR))) {
        fs.resize(node, 0);
    }
    if (flags & O_APPEND) {
        handle.offset = lseek(ref, 0, SEEK_END);
    }

    return ref;
}

void
PosixAdapter::close(HandleRef ref)
{
    // Lookup handle
    auto &handle = getHandle(ref);
    auto header = handle.headerBlock;

    // Remove from metadata
    auto &info = ensureMeta(header);
    info.openHandles.erase(ref);

    // Remove from global handle table
    handles.erase(ref);

    // Attempt deletion after all references are gone
    tryReclaim(header);
}

void
PosixAdapter::unlink(const fs::path &path)
{
    auto node = fs.seek(path);

    if (auto *info = getMeta(node); info) {

        // Remove directory entry
        fs.unlink(node);

        // Decrement link count
        if (info->linkCount > 0) info->linkCount--;

        // Maybe delete
        tryReclaim(node);
    }
}

void
PosixAdapter::tryReclaim(BlockNr node)
{
    if (auto *info = getMeta(node); info) {

        if (info->linkCount == 0 && info->openCount() == 0) {

            // Delete file
            fs.reclaim(node);

            // Trash meta data
            meta.erase(node);
        }
    }
}

Handle &
PosixAdapter::getHandle(HandleRef ref)
{
    auto it = handles.find(ref);

    if (it == handles.end()) {
        throw FSError(FSError::FS_INVALID_HANDLE, std::to_string(ref));
    }

    return it->second;
}

BlockNr
PosixAdapter::ensureFile(const fs::path &path)
{
    auto node = fs.seek(path);
    require.file(node);
    return node;
}

BlockNr
PosixAdapter::ensureFileOrDirectory(const fs::path &path)
{
    auto node = fs.seek(path);
    require.fileOrDirectory(node);
    return node;
}

BlockNr
PosixAdapter::ensureDirectory(const fs::path &path)
{
    auto node = fs.seek(path);
    require.directory(node);
    return node;
}

void
PosixAdapter::create(const fs::path &path)
{
    auto parent = path.parent_path();
    auto name   = path.filename();

    // Lookup destination directory
    auto node = fs.seek(parent);

    // Create file
    auto fhb = fs.createFile(node, FSName(name));

    // Create meta info
    auto &info = ensureMeta(fhb);
    info.linkCount = 1;
}

isize
PosixAdapter::lseek(HandleRef ref, isize offset, u16 whence)
{
    auto &handle  = getHandle(ref);
    auto &node    = fs.fetch(handle.headerBlock);
    auto fileSize = isize(node.getFileSize());

    isize newOffset;

    switch (whence) {

        case SEEK_SET:  newOffset = offset; break;
        case SEEK_CUR:  newOffset = handle.offset + offset; break;
        case SEEK_END:  newOffset = fileSize + offset; break;

        default:
            throw FSError(FSError::FS_UNKNOWN); // TODO: Throw, e.g., FS_INVALID_FLAG
    }

    // Ensure that the offset is not negative
    newOffset = std::max(newOffset, (isize)0);

    // Update the file handle and return
    handle.offset = newOffset;
    return newOffset;
}

void
PosixAdapter::move(const fs::path &oldPath, const fs::path &newPath)
{
    auto newDir  = newPath.parent_path();
    auto newName = newPath.filename();
    auto src     = fs.seek(oldPath);
    auto dst     = fs.seek(newDir);

    fs.move(src, dst, FSName(newName));
}

void
PosixAdapter::chmod(const fs::path &path, mode_t mode)
{
    auto &node = fs.fetch(ensureFile(path)).mutate();

    u32 prot = node.getProtectionBits();

    if (mode & S_IRUSR) prot &= ~0x01; else prot |= 0x01;
    if (mode & S_IWUSR) prot &= ~0x02; else prot |= 0x02;
    if (mode & S_IXUSR) prot &= ~0x04; else prot |= 0x04;

    node.setProtectionBits(prot);
}

void
PosixAdapter::resize(const fs::path &path, isize size)
{
    fs.resize(ensureFile(path), size);
}

isize
PosixAdapter::read(HandleRef ref, std::span<u8> buffer)
{
    auto &handle = getHandle(ref);
    auto &node   = fs.fetch(handle.headerBlock);
    auto &meta   = ensureMeta(node.nr);

    // Cache the file if necessary
    if (meta.cache.empty()) { node.extractData(meta.cache); }

    // Check for EOF
    if (handle.offset >= meta.cache.size) return 0;

    // Compute the number of bytes to read
    auto count = std::min(meta.cache.size - handle.offset, (isize)buffer.size());

    // Copy the requested range
    std::memcpy(buffer.data(), meta.cache.ptr + handle.offset, count);

    // Advance the handle offset
    handle.offset += count;

    return count;
}

isize
PosixAdapter::write(HandleRef ref, std::span<const u8> buffer)
{
    auto &handle = getHandle(ref);
//    auto &fhb    = fs.fetch(handle.headerBlock);
    auto &meta   = ensureMeta(handle.headerBlock);

    // Cache the file if necessary
    if (meta.cache.empty()) { fs.fetch(handle.headerBlock).extractData(meta.cache); }

    // Determine the new file size
    auto newSize = std::max(meta.cache.size, handle.offset + (isize)buffer.size());

    // Resize the cached file if necessary (pad with 0)
    meta.cache.resize(newSize, 0);

    // Compute the number of bytes to write
    auto count = buffer.size();

    // Update data
    std::memcpy(meta.cache.ptr + handle.offset, buffer.data(), count);

    // Write back
    fs.replace(handle.headerBlock, meta.cache);

    return count;
}

}
