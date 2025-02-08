// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AnyFile.h"

namespace vamiga {

class Workspace : public AnyFile {
    
public:
    
    static bool isCompatible(const std::filesystem::path &path);
    static bool isCompatible(const u8 *buf, isize len) { return false; }
    static bool isCompatible(const Buffer<u8> &buffer) { return false; }
    static bool isCompatible(std::istream &stream) { return false; }
    
    
    //
    // Initializing
    //
    
    Workspace(const std::filesystem::path &path) throws { init(path); }
    
private:
    
    void init(const std::filesystem::path &path) throws;
    
    
    //
    // Methods from CoreObject
    //
    
public:
    
    const char *objectName() const override { return "Workspace"; }
    
    
    //
    // Methods from AnyFile
    //
    
    FileType type() const override { return FileType::WORKSPACE; }
    bool isCompatiblePath(const std::filesystem::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }
    u64 fnv64() const override { return 0; }
};

}
