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

/// An entry of the address translation cache of the 68040
struct Atc040Entry {

    u32 logical;            ///< Logical page address
    u32 physical;           ///< Physical page address
    bool super;             ///< Supervisor space (FC2)
    bool valid;             ///<
    u32 status;             ///< Page attributes in MMUSR layout (G, U1, U0, S, CM, M, W, R)
    bool history;           ///< Set if the entry was used recently (guides the replacement policy)
};

/// Number of entries of each address translation cache (instruction and data)
static constexpr int ATC040_ENTRIES             = 64;

/// MMUSR bit: bus error
static constexpr u32 MMUSR040_BUS_ERROR         = 1 << 11;
/// MMUSR bit: global page
static constexpr u32 MMUSR040_GLOBAL            = 1 << 10;
/// MMUSR bit: user page attribute 1
static constexpr u32 MMUSR040_U1                = 1 << 9;
/// MMUSR bit: user page attribute 0
static constexpr u32 MMUSR040_U0                = 1 << 8;
/// MMUSR bit: supervisor only
static constexpr u32 MMUSR040_SUPERVISOR_ONLY   = 1 << 7;
/// MMUSR mask: cache mode
static constexpr u32 MMUSR040_CM                = 3 << 5;
/// MMUSR bit: page modified
static constexpr u32 MMUSR040_MODIFIED          = 1 << 4;
/// MMUSR bit: write protected
static constexpr u32 MMUSR040_WRITE_PROTECTED   = 1 << 2;
/// MMUSR bit: access matches a transparent translation register
static constexpr u32 MMUSR040_TRANSPARENT       = 1 << 1;
/// MMUSR bit: page is resident
static constexpr u32 MMUSR040_RESIDENT          = 1 << 0;

/// Mask of the page descriptor bits that are copied into the status register
static constexpr u32 MMUSR040_PAGE_BITS         = 0x7F5;

/// Translation control register bit: enable address translation
static constexpr u32 TC040_ENABLE               = 1 << 15;
/// Translation control register bit: 8 KB pages (4 KB if cleared)
static constexpr u32 TC040_PAGE_8K              = 1 << 14;

/** Memory management unit of the 68040
 *
 * This class is used through the MMU wrapper class (see MMU.h). The 68040
 * walks a fixed three-level table structure and owns two address translation
 * caches, one for instruction fetches and one for data accesses.
 */
class MMU040 {

    // The CPU this MMU belongs to
    class Moira &moira;

    // Set while peek() walks the tables. Descriptors are read with read16Dasm()
    // instead of the bus, and the U and M bits are not written back.
    mutable bool peeking = false;

public:

    /// User root pointer
    u32 urp = 0;
    /// Supervisor root pointer
    u32 srp = 0;

    /// Translation control register
    u32 tc = 0;

    /// Instruction transparent translation register 0
    u32 itt0 = 0;
    /// Instruction transparent translation register 1
    u32 itt1 = 0;
    /// Data transparent translation register 0
    u32 dtt0 = 0;
    /// Data transparent translation register 1
    u32 dtt1 = 0;

    /// MMU Status register
    u32 mmusr = 0;

    /// Address translation cache for instruction fetches
    Atc040Entry iatc[ATC040_ENTRIES] = { };
    /// Address translation cache for data accesses
    Atc040Entry datc[ATC040_ENTRIES] = { };


    //
    // Constructing
    //

public:

    MMU040(Moira &ref) : moira(ref) { reset(); }

    /// Initializes all registers and invalidates the ATCs
    void reset();


    //
    // Configuring
    //

public:

    /// Indicates whether address translation is active
    bool isEnabled() const { return tc & TC040_ENABLE; }

    /// Sets the translation control register
    void setTC(u32 value);
    /// Sets instruction transparent translation register 0
    void setITT0(u32 value);
    /// Sets instruction transparent translation register 1
    void setITT1(u32 value);
    /// Sets data transparent translation register 0
    void setDTT0(u32 value);
    /// Sets data transparent translation register 1
    void setDTT1(u32 value);


    //
    // Translating
    //

public:

    /// Returns the page size mask as defined in the translation control register
    u32 pageMask() const { return (tc & TC040_PAGE_8K) ? 0x1FFF : 0xFFF; }

    /// Checks whether an access matches one of the transparent translation registers
    bool testTT(u32 addr, u8 fc, bool write) const;

    /// Translates a logical address to a physical address. If 'ci' is provided,
    /// it is set if the access must bypass the on-chip caches.
    std::optional<u32> translate(u32 addr, u8 fc, bool write, bool *ci = nullptr);

    /// Translates an address like translate(), but without side effects: The ATC
    /// is not changed, the U and M bits are not set, and the descriptors are read
    /// with read16Dasm(). Returns std::nullopt if the access would cause a bus error.
    std::optional<u32> peek(u32 addr, u8 fc, bool write = false) const;

    /// Creates an ATC entry for a logical address and returns the MMUSR value (PTEST)
    u32 test(u32 addr, u8 fc, bool write);


    //
    // Managing the address translation caches
    //

public:

    /// Searches the ATC. Returns the line number of a matching entry.
    std::optional<int> atcFind(u32 addr, u8 fc, bool write);

    /// Creates an ATC entry and returns its line number
    int atcInsert(u32 addr, u8 fc, u32 physical, u32 status);

    /// Invalidates all ATC entries
    void atcFlush();
    /// Invalidates all ATC entries. Global entries are only affected if `global` is set.
    void atcFlush(bool global);
    /// Invalidates the ATC entries of a page. Global entries are only affected if `global` is set.
    void atcFlushPage(u32 addr, bool super, bool global);

private:

    // Returns the ATC that serves an access
    Atc040Entry *atc(u8 fc) { return (fc & 3) == 2 ? iatc : datc; }

    // Returns the first unused line, if any
    static std::optional<int> atcFindFree(Atc040Entry *atc);

    // Returns the first line without history bit, if any
    static std::optional<int> atcFindUnused(Atc040Entry *atc);

    // Marks an entry as recently used
    static void atcTouch(Atc040Entry *atc, int line);

    // Checks whether an access matches a single transparent translation register
    enum class TTResult { NO_MATCH, MATCH, NO_WRITE };
    static TTResult testTT(u32 ttx, u32 addr, bool super, bool write);

    // Walks the translation tables
    u32 walk(u32 addr, bool super, bool write, u32 &status);

    // Reads a descriptor (with read16Dasm() while peeking)
    u32 readDescriptor(u32 addr) const;
};

}
