// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "../MoiraTypes.h"
#include <optional>

namespace MOIRA_NAMESPACE {

struct MmuContext;

enum class MmuReg {
    
    TT0,                    ///< Transparent translation register 0
    TT1,                    ///< Transparent translation register 1
    MMUSR,                  ///< MMU Status register
    CRP,                    ///< CPU Root pointer
    SRP,                    ///< Supervisor Root pointer
    TC                      ///< Translation control register
};

/// An entry of the address translation cache of the 68030
struct AtcEntry {

    u32 logical;            ///< Logical page address
    u32 physical;           ///< Physical page address
    u8  fc;                 ///< Function code
    bool valid;             ///<
    bool busError;          ///< Translation error bit
    bool wp;                ///< Write protection bit
    bool modified;          ///< Modification bit
    bool ci;                ///< Cache inhibit bit
    bool history;           ///< Set if the entry was used recently (guides the replacement policy)
};

/// Number of entries of the address translation cache
static constexpr int ATC_ENTRIES                = 22;

/// MMUSR bit: bus error
static constexpr u32 MMUSR_BUS_ERROR            = 1 << 15;
/// MMUSR bit: limit violation
static constexpr u32 MMUSR_LIMIT_VIOLATION      = 1 << 14;
/// MMUSR bit: supervisor only
static constexpr u32 MMUSR_SUPERVISOR_ONLY      = 1 << 13;
/// MMUSR bit: write protected
static constexpr u32 MMUSR_WRITE_PROTECTED      = 1 << 11;
/// MMUSR bit: invalid descriptor
static constexpr u32 MMUSR_INVALID              = 1 << 10;
/// MMUSR bit: page modified
static constexpr u32 MMUSR_MODIFIED             = 1 << 9;
/// MMUSR bit: access matches a transparent translation register
static constexpr u32 MMUSR_TRANSPARENT_ACCESS   = 1 << 6;
/// MMUSR mask: number of table levels that were walked
static constexpr u32 MMUSR_NUMBER_OF_LEVELS     = 0b111;

/** Memory management unit of the 68030
 *
 * This class is used through the MMU wrapper class (see MMU.h).
 */
class MMU030 {

    // The CPU this MMU belongs to
    class Moira &moira;

    // Set while peek() walks the tables. Descriptors are read with read16Dasm()
    // instead of the bus, and the U and M bits are not written back.
    mutable bool peeking = false;

public:

    /// CPU root pointer
    u64 crp = 0;
    /// Supervisor root pointer
    u64 srp = 0;
    
    /// Translation control register
    u32 tc = 0;
    
    /// Transparent translation register 0
    u32 tt0 = 0;
    /// Transparent translation register 1
    u32 tt1 = 0;
    
    /// MMU Status register
    u32 mmusr = 0;

    /// Address translation cache
    AtcEntry atc[ATC_ENTRIES] = { };


    //
    // Constructing
    //

public:

    MMU030(Moira &ref) : moira(ref) { reset(); }

    /// Initializes all registers and invalidates the ATC
    void reset();


    //
    // Configuring
    //

public:

    /// Indicates whether address translation is active
    bool isEnabled() const { return tc & 0x80000000; }

    /// Sets the translation control register
    void setTC(u32 value);
    /// Sets transparent translation register 0
    void setTT0(u32 value);
    /// Sets transparent translation register 1
    void setTT1(u32 value);


    //
    // Translating
    //

public:

    /// Returns the page size mask as defined in the translation control register
    u32 pageMask() const { return (1u << (tc >> 20 & 0xF)) - 1; }

    /// Returns the number of tables indexed by the logical address
    int numTables() const;

    /// Checks whether an access matches one of the transparent translation registers
    bool testTT(u32 addr, u8 fc, bool rw) const;

    /// Translates a logical address to a physical address. If 'ci' is provided,
    /// it is set if the access must bypass the on-chip caches.
    std::optional<u32> translate(u32 addr, u8 fc, bool write, bool *ci = nullptr);

    /// Translates an address like translate(), but without side effects: The ATC
    /// is not changed, the U and M bits are not set, and the descriptors are read
    /// with read16Dasm(). Returns std::nullopt if the access would cause a bus error.
    std::optional<u32> peek(u32 addr, u8 fc, bool write = false) const;

    /// Creates an ATC entry for a logical address (PLOAD)
    void load(u32 addr, u8 fc, bool write);

    /// Returns the MMUSR value and the address of the last descriptor (PTEST)
    u32 test(u32 addr, u8 fc, bool write, u32 level, u32 &descrAddr);


    //
    // Managing the address translation cache
    //

public:

    /// Searches the ATC. Returns the line number of a matching entry.
    std::optional<int> atcFind(u32 addr, u8 fc, bool write);

    /// Creates an ATC entry and returns its line number
    int atcInsert(u32 addr, u8 fc, u32 physical, u32 status, bool ci);

    /// Invalidates all ATC entries
    void atcFlush();
    /// Invalidates all ATC entries with a matching function code
    void atcFlush(u8 fcBase, u8 fcMask);
    /// Invalidates the ATC entries of a page
    void atcFlushPage(u32 addr);
    /// Invalidates the ATC entries of a page with a matching function code
    void atcFlushPage(u32 addr, u8 fcBase, u8 fcMask);

private:

    // Returns the first unused line, if any
    std::optional<int> atcFindFree();

    // Returns the first line without history bit, if any
    std::optional<int> atcFindUnused();

    // Marks an entry as recently used
    void atcTouch(int line);

    // Checks whether an access matches a single transparent translation register
    static bool testTT(u32 ttx, u32 addr, u8 fc, bool rw);

    // Walks the translation tables
    u32 walk(u32 addr, u8 fc, bool write, MmuContext &c);

    // Walks the translation tables without side effects (PTEST mode)
    u32 walk(u32 addr, u8 fc, bool write, MmuContext &c, bool ptest, u32 level);

    // Reads a descriptor (with read16Dasm() while peeking)
    u32 readDescriptor(u32 addr) const;

    // Helpers called inside the walk function
    u32 lookupShort(char table, u32 taddr, u32 offset, bool write, MmuContext &c);
    u32 lookupLong(char table, u32 taddr, u32 offset, bool write, MmuContext &c);
};

}
