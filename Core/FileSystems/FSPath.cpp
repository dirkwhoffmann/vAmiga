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

FSPath::FSPath(const FileSystem *fs, Block dir) : fs(fs), ref(dir)
{
    if (!fs->initialized()) throw AppError(Fault::FS_UNINITIALIZED);
    if (!ptr()) throw AppError(Fault::FS_INVALID_BLOCK_REF);
}

/*
FSPath::FSPath(const FSPath &path) : fs(path.fs), ref(path.ref)
{
    if (!fs->initialized()) throw AppError(Fault::FS_UNINITIALIZED);
    if (!ptr()) throw AppError(Fault::FS_INVALID_BLOCK_REF);
}
*/

FSPath::FSPath(const FileSystem *fs) : fs(fs), ref(0)
{
    if (!fs->initialized()) throw AppError(Fault::FS_UNINITIALIZED);
    if (!ptr()) throw AppError(Fault::FS_INVALID_BLOCK_REF);
}

FSPath::FSPath(const FileSystem *fs, FSBlock *dir) : FSPath(fs, dir->nr)
{

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
    *this = seek(name);
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
    return fs->blockType(ref) == FSBlockType::ROOT_BLOCK;
}

bool
FSPath::isFile() const
{
    return fs->blockType(ref) == FSBlockType::FILEHEADER_BLOCK;
}

bool
FSPath::isDirectory() const
{
    return fs->blockType(ref) == FSBlockType::USERDIR_BLOCK || isRoot();
}

bool
FSPath::isRegular() const
{
    return isFile() || isDirectory();
}

FSBlock *
FSPath::ptr() const
{
    return fs->blockPtr(ref);
}

FSName
FSPath::last() const
{
    return isRoot() ? "" : fs->blockPtr(ref)->getName();
}

string
FSPath::name() const
{
    string result;

    for (auto &node : refs()) {

        auto name = fs->blockPtr(node)->getName();
        result = result + "/" + name.cpp_str();
    }

    return result.empty() ? "/" : result;
}

fs::path
FSPath::getPath() const
{
    fs::path result;

    for (auto &node : refs()) {

        auto name = fs->blockPtr(node)->getName().path();
        result = result.empty() ? name : name / result;
    }
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
        if (!fs->hashableBlockPtr(block->nr)) break;

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
FSPath::parent() const
{
    return isRoot() ? *this : FSPath(fs, fs->blockPtr(ref)->getParentDirRef());
}

FSPath
FSPath::seek(const FSName &name) const
{
    std::set<Block> visited;
    FSBlock *cdb = ptr();

    // Check for special tokens
    if (name == "")   return *this;
    if (name == ".")  return *this;
    if (name == "..") return parent();
    if (name == "/")  return fs->rootDir();

    // Only proceed if a hash table is present
    if (cdb && cdb->hashTableSize() != 0) {

        // Compute the table position and read the item
        u32 hash = name.hashValue() % cdb->hashTableSize();
        u32 ref = cdb->getHashRef(hash);

        // Traverse the linked list until the item has been found
        while (ref && visited.find(ref) == visited.end())  {

            FSBlock *item = fs->hashableBlockPtr(ref);
            if (item == nullptr) break;

            if (item->isNamed(name)) return FSPath(fs, item);

            visited.insert(ref);
            ref = item->getNextHashRef();
        }
    }

    throw AppError(Fault::FS_NOT_FOUND, name.cpp_str());
}

FSPath
FSPath::seek(const FSString &name) const
{
    return seek(name.cpp_str());
}

FSPath
FSPath::seek(const std::vector<FSName> &name) const
{
    FSPath result = *this;
    for (auto &it : name) { result = result.seek(it); }
    return result;
}

FSPath
FSPath::seek(const std::vector<string> &name) const
{
    FSPath result = *this;
    for (auto &it : name) { result = result.seek(FSName(it)); }
    return result;
}

FSPath
FSPath::seek(const fs::path &name) const
{
    FSPath result = fs->rootDir();
    for (const auto &it : name) { result = result.seek(FSName(it)); }
    return result;
}

FSPath
FSPath::seek(const string &name) const
{
    return seek(util::split(name, '/'));
}

FSPath
FSPath::seek(const char *name) const
{
    return seek(string(name));
}

FSPath
FSPath::seekDir(const FSName &dir) const
{
    if (auto result = seek(dir); result.isDirectory()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_DIRECTORY, dir.cpp_str());
}

FSPath
FSPath::seekDir(const FSString &dir) const
{
    if (auto result = seek(dir); result.isDirectory()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_DIRECTORY, dir.cpp_str());
}

FSPath
FSPath::seekDir(const std::vector<FSName> &dir) const
{
    if (auto result = seek(dir); result.isDirectory()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_DIRECTORY);
}

FSPath
FSPath::seekDir(const std::vector<string> &dir) const
{
    if (auto result = seek(dir); result.isDirectory()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_DIRECTORY);
}

FSPath
FSPath::seekDir(const fs::path &dir) const
{
    if (auto result = seek(dir); result.isDirectory()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_DIRECTORY, dir.string());
}

FSPath
FSPath::seekDir(const string &dir) const
{
    if (auto result = seek(dir); result.isDirectory()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_DIRECTORY, dir);
}

FSPath
FSPath::seekDir(const char *dir) const
{
    if (auto result = seek(dir); result.isDirectory()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_DIRECTORY, string(dir));
}

FSPath
FSPath::seekFile(const FSName &file) const
{
    if (auto result = seek(file); result.isFile()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_FILE, file.cpp_str());
}

FSPath
FSPath::seekFile(const FSString &file) const
{
    if (auto result = seek(file); result.isFile()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_FILE, file.cpp_str());
}

FSPath
FSPath::seekFile(const std::vector<FSName> &file) const
{
    if (auto result = seek(file); result.isFile()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_FILE);
}

FSPath
FSPath::seekFile(const std::vector<string> &file) const
{
    if (auto result = seek(file); result.isFile()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_FILE);
}

FSPath
FSPath::seekFile(const fs::path &file) const
{
    if (auto result = seek(file); result.isFile()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_FILE, file.string());
}

FSPath
FSPath::seekFile(const string &file) const
{
    if (auto result = seek(file); result.isFile()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_FILE, file);
}

FSPath
FSPath::seekFile(const char *file) const
{
    if (auto result = seek(file); result.isFile()) {
        return result;
    }
    throw AppError(Fault::FS_NOT_A_FILE, string(file));
}

std::vector<FSPath>
FSPath::collect(const FSOpt &opt) const
{
    std::vector<FSPath> result;
    std::set<Block> visited;

    // Collect the blocks for all items in this directory
    std::stack<Block> remainingItems;
    fs->collectHashedRefs(ref, remainingItems, visited);

    // Move the collected items to the result list
    while (remainingItems.size() > 0) {

        auto it = FSPath(fs, remainingItems.top());
        if (opt.accept(it)) result.push_back(it);
        remainingItems.pop();

        // Add subdirectory items to the queue
        if (opt.recursive) fs->collectHashedRefs(it.ref, remainingItems, visited);
    }

    // Sort items
    if (opt.sort) { std::sort(result.begin(), result.end(), opt.sort); }

    return result;
}

std::vector<FSPath>
FSPath::collectDirs(const FSOpt &opt) const
{
    auto eraser = [](const FSPath &p) { return !p.isDirectory(); };

    auto result = collect(opt);
    result.erase(std::remove_if(result.begin(), result.end(), eraser), result.end());
    return result;
}

std::vector<FSPath>
FSPath::collectFiles(const FSOpt &opt) const
{
    auto eraser = [](const FSPath &p) { return !p.isFile(); };

    auto result = collect(opt);
    result.erase(std::remove_if(result.begin(), result.end(), eraser), result.end());
    return result;
}

std::ostream &operator<<(std::ostream &os, const FSPath &path) {

    os << path.name();
    return os;
}

}
