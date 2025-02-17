// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "Macros.h"
#include "IOUtils.h"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <fstream>
#include <iomanip>

namespace vamiga::util {

fs::path
makeUniquePath(const fs::path &path)
{
    auto location = path.root_path() / path.relative_path();
    auto name = path.stem().string();
    auto extension = path.extension();

    for (isize nr = 2;; nr++) {

        auto index = std::to_string(nr);
        fs::path result = location / fs::path(name + index) / extension;

        if (!util::fileExists(result)) return result;
    }

    unreachable;
}

isize
getSizeOfFile(const fs::path &path)
{
    struct stat fileProperties;

    if (stat(path.string().c_str(), &fileProperties) != 0)
        return -1;

    return (isize)fileProperties.st_size;
}

bool
fileExists(const fs::path &path)
{
    return getSizeOfFile(path) >= 0;
}

bool
isDirectory(const fs::path &path)
{
    try {
        
        const auto &entry = fs::directory_entry(path);
        return entry.is_directory();
    
    } catch (...) {
        
        return false;
    }
}

bool
createDirectory(const fs::path &path)
{
    try {
        
        return fs::create_directory(path);
    
    } catch (...) {
        
        return false;
    }
}

isize
numDirectoryItems(const fs::path &path)
{
    isize result = 0;
    
    try {
        
        for (const auto &entry : fs::directory_iterator(path)) {
            
            const auto &name = entry.path().filename().string();
            if (name[0] != '.') result++;
        }
        
    } catch (...) { }
    
    return result;
}

std::vector<fs::path>
files(const fs::path &path, const string &suffix)
{
    std::vector <string> suffixes;
    if (suffix != "") suffixes.push_back(suffix);

    return files(path, suffixes);
}

std::vector<fs::path>
files(const fs::path &path, std::vector <string> &suffixes)
{
    std::vector<fs::path> result;

    try {
        
        for (const auto &entry : fs::directory_iterator(path)) {

            const auto &name = entry.path().filename();
            auto suffix = name.extension().string();

            if (std::find(suffixes.begin(), suffixes.end(), suffix) != suffixes.end()) {
                result.push_back(name);
            }
        }
        
    } catch (...) { }
    
    return result;
}

bool
matchingFileHeader(const fs::path &path, const u8 *header, isize len, isize offset)
{
    std::ifstream file(path, std::ios::binary);
    return file.is_open() ? matchingStreamHeader(file, header, len, offset) : false;
}

bool
matchingFileHeader(const fs::path &path, const string &header, isize offset)
{
    std::ifstream file(path, std::ios::binary);
    return file.is_open() ? matchingStreamHeader(file, header, offset) : false;
}

bool
matchingStreamHeader(std::istream &is, const u8 *header, isize len, isize offset)
{
    assert(header != nullptr);
    
    is.seekg(offset, std::ios::beg);
    
    for (isize i = 0; i < len; i++) {
        
        if (is.get() != (int)header[i]) {
            is.seekg(0, std::ios::beg);
            return false;
        }
    }
    is.seekg(0, std::ios::beg);
    return true;
}

bool
matchingStreamHeader(std::istream &is, const string &header, isize offset)
{
    return matchingStreamHeader(is, (u8 *)header.c_str(), (isize)header.length(), offset);
}

bool
matchingBufferHeader(const u8 *buffer, const u8 *header, isize len, isize offset)
{
    assert(buffer != nullptr);
    assert(header != nullptr);
    
    for (isize i = 0; i < len; i++) {
        if (buffer[offset + i] != header[i])
            return false;
    }
    return true;
}

bool 
matchingBufferHeader(const u8 *buf, isize blen, const string &header, isize offset)
{
    assert(buf != nullptr);

    if (isize length = isize(header.length()); length <= blen) {

        for (usize i = 0; i < header.length(); i++) {
            if (buf[offset + i] != header[i])
                return false;
        }
    }
    return true;
}

bool 
matchingBufferHeader(const u8 *buf, const string &header, isize offset)
{
    auto blen = std::numeric_limits<isize>::max();
    return matchingBufferHeader(buf, blen, header, offset);
}

std::ostream &
dec::operator()(std::ostream &os) const
{
    os << std::dec << value;
    return os;
};

std::ostream &
hex::operator()(std::ostream &os) const
{
    os << std::hex << "0x" << std::setw(int(digits)) << std::setfill('0') << value;
    return os;
};

std::ostream &
bin::operator()(std::ostream &os) const
{
    os << "%";

    for (isize i = 7; i >= 0; i--) {

        if ((digits == 64 && i < 8) ||
            (digits == 32 && i < 4) ||
            (digits == 16 && i < 2) ||
            (digits == 8  && i < 1)  ) {

            std::bitset<8> x(GET_BYTE(value, i));
            os << x << (i ? "." : "");
        }
    }
    return os;
};

std::ostream &
flt::operator()(std::ostream &os) const
{
    os << value;
    return os;
};

std::ostream &
tab::operator()(std::ostream &os) const {
    os << std::setw(int(pads)) << std::right << std::setfill(' ') << str;
    os << (str.empty() ? "   " : " : ");
    return os;
}

std::ostream &
bol::operator()(std::ostream &os) const {
    os << (value ? str1 : str2);
    return os;
}

std::ostream &
str::operator()(std::ostream &os) const
{
    auto c = [&](isize pos) {

        auto byte = GET_BYTE(value, pos);
        return std::isprint(byte) ? string{(char)byte} : string{'.'};
    };

    if (characters >= 8) os << c(7) << c(6) << c(5) << c(4);
    if (characters >= 4) os << c(3) << c(2);
    if (characters >= 2) os << c(1);
    if (characters >= 1) os << c(0);

    return os;
};

const string &bol::yes = "yes";
const string &bol::no = "no";

}
