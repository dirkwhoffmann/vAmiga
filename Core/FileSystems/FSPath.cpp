// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSPath.h"
#include "FileSystem.h"
#include <algorithm>

namespace vamiga {

FSPath::FSPath(const FSPath &path) : fs(path.fs), ref(path.ref)
{

}

FSPath::FSPath(const FileSystem &fs) : fs(fs), ref(0)
{

}

FSPath::FSPath(const FileSystem &fs, Block dir) : fs(fs), ref(dir)
{
    selfcheck();
}

FSPath::FSPath(const FileSystem &fs, FSBlock *dir) : FSPath(fs, dir->nr)
{

}

/*
FSPath::FSPath(const FileSystem &fs, const string &path) : FSPath(seek(path))
{

}

FSPath::FSPath(const FileSystem &fs, const fs::path &path) : FSPath(seek(path))
{

}
*/

void
FSPath::selfcheck() const
{
    // Check if the block number is in the valid range
    if (!ptr()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);

    // Check the block type
    if (!isRoot() && !isFile() && !isDirectory()) throw AppError(Fault::FS_INVALID_BLOCK_TYPE);
}

FSPath&
FSPath::operator=(const FSPath &path)
{
    ref = path.ref;
    return *this;
}

FSPath&
FSPath::operator/=(const FSName &name)
{
    *this = cd(name);
    return *this;
}

FSPath
FSPath::operator/(const FSName &name) const
{
    FSPath result = *this;
    result /= name;
    return result;
}

bool
FSPath::isRoot() const
{
    return fs.blockType(ref) == FSBlockType::ROOT_BLOCK;
}

bool
FSPath::isFile() const
{
    return fs.blockType(ref) == FSBlockType::FILEHEADER_BLOCK;
}

bool
FSPath::isDirectory() const
{
    return isRoot() || fs.blockType(ref) == FSBlockType::USERDIR_BLOCK;
}

FSBlock *
FSPath::ptr() const
{
    return fs.blockPtr(ref);
}

FSName
FSPath::last() const
{
    return isRoot() ? "" : fs.blockPtr(ref)->getName();
}

/*
FSName
FSPath::name() const
{
    return fs.blockPtr(ref)->getName();
}
*/

string
FSPath::name() const
{
    string result;

    for (auto &node : refs()) {

        auto name = fs.blockPtr(node)->getName();
        result = result + "/" + name.cpp_str();
    }

    return result.empty() ? "/" : result;
}

fs::path
FSPath::getPath() const
{
    fs::path result;

    for (auto &node : refs()) {

        auto name = fs.blockPtr(node)->getName().path();
        result = result.empty() ? name : name / result;
    }
    /*
    std::set<Block> visited;

    auto block = fs.blockPtr(ref);

    while (block) {

        // Break the loop if this block has an invalid type
        if (!fs.hashableBlockPtr(block->nr)) break;

        // Break the loop if this block was visited before
        if (visited.find(block->nr) != visited.end()) break;

        // Add the block to the set of visited blocks
        visited.insert(block->nr);

        // Expand the path
        auto name = block->getName().path();
        result = result.empty() ? name : name / result;

        // Continue with the parent block
        block = block->getParentDirBlock();
    }
    */

    return result;
}

std::vector<Block>
FSPath::refs() const
{
    std::vector<Block> result;
    std::set<Block> visited;
    
    auto block = ptr();

    while (block) {

        // Break the loop if this block has an invalid type
        if (!fs.hashableBlockPtr(block->nr)) break;

        // Break the loop if this block was visited before
        if (visited.contains(block->nr)) break;

        // Add the block
        result.push_back(block->nr);
        visited.insert(block->nr);

        // Continue with the parent block
        block = block->getParentDirBlock();
    }

    std::reverse(result.begin(), result.end());
    return result;
}

string
FSPath::getProtectionBitString() const
{
    auto bits = ptr()->getProtectionBits();

    // From dos/dos.h (AmigaDOS)
    constexpr isize FIBB_SCRIPT  = 6; // program is a script (execute) file
    constexpr isize FIBB_PURE    = 5; // program is reentrant and rexecutable
    constexpr isize FIBB_ARCHIVE = 4; // cleared whenever file is changed
    constexpr isize FIBB_READ    = 3; // ignored by old filesystem
    constexpr isize FIBB_WRITE   = 2; // ignored by old filesystem
    constexpr isize FIBB_EXECUTE = 1; // ignored by system, used by Shell
    constexpr isize FIBB_DELETE  = 0; // prevent file from being deleted

    string result;
    result += (bits & (1 << 7))            ? "h" : "-";
    result += (bits & (1 << FIBB_SCRIPT))  ? "s" : "-";
    result += (bits & (1 << FIBB_PURE))    ? "p" : "-";
    result += (bits & (1 << FIBB_ARCHIVE)) ? "a" : "-";
    result += (bits & (1 << FIBB_READ))    ? "-" : "r";
    result += (bits & (1 << FIBB_WRITE))   ? "-" : "w";
    result += (bits & (1 << FIBB_EXECUTE)) ? "-" : "e";
    result += (bits & (1 << FIBB_DELETE))  ? "-" : "d";

    return result;
}

FSPath
FSPath::seek(const FSName &name) const
{
    std::set<Block> visited;
    FSBlock *cdb = ptr();

    // Only proceed if a hash table is present
    if (cdb && cdb->hashTableSize() != 0) {

        // Compute the table position and read the item
        u32 hash = name.hashValue() % cdb->hashTableSize();
        u32 ref = cdb->getHashRef(hash);

        // Traverse the linked list until the item has been found
        while (ref && visited.find(ref) == visited.end())  {

            FSBlock *item = fs.hashableBlockPtr(ref);
            if (item == nullptr) break;

            if (item->isNamed(name)) return FSPath(fs, item);

            visited.insert(ref);
            ref = item->getNextHashRef();
        }
    }

    throw AppError(Fault::FS_NOT_FOUND);
}

FSPath
FSPath::seekDir(const FSName &name) const
{
    auto result = seek(name);
    if (!result.isDirectory()) throw AppError(Fault::FS_NOT_A_DIRECTORY);
    return result;
}

FSPath
FSPath::seekFile(const FSName &name) const
{
    auto result = seek(name);
    if (!result.isFile()) throw AppError(Fault::FS_NOT_A_FILE);
    return result;
}

FSPath
FSPath::seek(const std::vector<FSName> &path) const
{
    FSPath result = *this;
    for (auto &item : path) { result = result.seek(item); }
    return result;
}

FSPath
FSPath::seekDir(const std::vector<FSName> &path) const
{
    auto result = seek(path);
    if (!result.isDirectory()) throw AppError(Fault::FS_NOT_A_DIRECTORY);
    return result;
}

FSPath
FSPath::seekFile(const std::vector<FSName> &path) const
{
    auto result = seek(path);
    if (!result.isFile()) throw AppError(Fault::FS_NOT_A_FILE);
    return result;
}

FSPath
FSPath::seek(const std::vector<string> &path) const
{
    FSPath result = *this;
    for (auto &item : path) { result = result.seek(FSName(item)); }
    return result;
}

FSPath
FSPath::seekDir(const std::vector<string> &path) const
{
    auto result = seek(path);
    if (!result.isDirectory()) throw AppError(Fault::FS_NOT_A_DIRECTORY);
    return result;
}

FSPath
FSPath::seekFile(const std::vector<string> &path) const
{
    auto result = seek(path);
    if (!result.isFile()) throw AppError(Fault::FS_NOT_A_FILE);
    return result;
}

FSPath
FSPath::seek(const fs::path &path) const
{
    FSPath result = fs.rootDir();

    for (const auto& part : path) {

        if (part == path.filename()) {
            result = result.seekFile(FSName(part));
        } else {
            result = result.seekDir(FSName(part));
        }
    }
    return result;
}

FSPath
FSPath::seekDir(const fs::path &path) const
{
    if (FSPath result = seek(path); result.isDirectory()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_FOUND);
}

FSPath
FSPath::seekFile(const fs::path &path) const
{
    if (FSPath result = seek(path); result.isFile()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_FOUND);
}

FSPath
FSPath::seek(const string &path) const
{
    return seek(util::split(path, '/'));
}

FSPath
FSPath::seekDir(const string &path) const
{
    return seekDir(util::split(path, '/'));
}

FSPath
FSPath::seekFile(const string &path) const
{
    return seekFile(util::split(path, '/'));
}

FSPath
FSPath::cd(FSName name)
{
    return seek(name);
}

FSPath
FSPath::cd(const std::vector<FSName> &names)
{
    FSPath result = *this;
    for (const auto& name : names) result = cd(name);
    return result;
}

FSPath
FSPath::cd(const std::vector<string> &names)
{
    FSPath result = *this;
    for (const auto& name : names) result = cd(FSName(name));
    return result;
}

FSPath
FSPath::cd(const string &path)
{
    if (path == "")   return *this;
    if (path == ".")  return *this;
    if (path == "..") return parent();
    if (path == "/")  return fs.rootDir();

    FSPath result = *this;
    string p = path;

    // Start from the root if the path starts with '/'
    if (p[0] == '/') { result = fs.rootDir(); p.erase(0, 1); }

    auto parts = util::split(p, '/');
    for (const auto& part : parts) result = cd(FSName(part));

    return result;
}

/*
FSPath
FSPath::cd(const fs::path &path)
{
    if (!fs::is_directory(path)) throw AppError(Fault::FS_INVALID_PATH);

    FSPath result(fs);
    for (const auto& part : path) result = cd(FSName(part));

    return result;
}
*/

FSPath
FSPath::parent()
{
    return isRoot() ? *this : FSPath(fs, fs.blockPtr(ref)->getParentDirRef());
}

std::vector<FSPath>
FSPath::collect(bool recursive, bool sort) const
{
    // Collect all blocks
    std::vector<Block> blocks; fs.collect(*this, blocks, recursive);

    // Convert to paths
    std::vector<FSPath> paths; for (auto &it : blocks) paths.push_back(FSPath(fs, it));

    // Sort items
    if (sort) {

        std::sort(paths.begin(), paths.end(), [&](const FSPath &a, const FSPath &b) {

            if ( a.isDirectory() && !b.isDirectory()) return true;
            if (!a.isDirectory() &&  b.isDirectory()) return false;
            return a.last() < b.last();
        });
    }

    return paths;
}

std::ostream &operator<<(std::ostream &os, const FSPath &path) {

    os << path.name();
    return os;
}

}
