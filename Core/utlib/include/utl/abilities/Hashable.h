// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"

namespace utl {

class Hashable {

public:

    enum class HashAlgorithm { CRC16, CRC32, FNV32, FNV64 };

    //
    // Class methods
    //

    // Returns the FNV-1a seed value
    static consteval u32 fnvInit32() { return 0x811c9dc5; }
    static consteval u64 fnvInit64() { return 0xcbf29ce484222325; }

    // Performs a single iteration of the FNV-1a hash algorithm
    static u32 fnvIt32(u32 prv, u32 val);
    static u64 fnvIt64(u64 prv, u64 val);

    // Computes a checksum for a given buffer
    static u64 hash(const u8 *addr, isize size, HashAlgorithm algorithm);

    // Computes a FNV-1a checksum for a given buffer
    static u32 fnv32(const u8 *addr, isize size);
    static u64 fnv64(const u8 *addr, isize size);

    // Computes a CRC checksum for a given buffer
    static u16 crc16(const u8 *addr, isize size);
    static u32 crc32(const u8 *addr, isize size);

    //
    // Instance methods
    //

    virtual ~Hashable() = default;

    // Main entry point (provided by the subclass)
    virtual u64 hash(HashAlgorithm algorithm) = 0;

    // Wrapper functions
    u32 fnv32() { return (u32)hash(HashAlgorithm::FNV32); }
    u64 fnv64() { return (u64)hash(HashAlgorithm::FNV64); }

    // Computes a CRC checksum for a given buffer
    u16 crc16() { return (u16)hash(HashAlgorithm::CRC16); }
    u32 crc32() { return (u32)hash(HashAlgorithm::CRC32); }
};

}
