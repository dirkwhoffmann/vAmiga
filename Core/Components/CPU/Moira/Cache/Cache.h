// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "../MoiraTypes.h"

namespace MOIRA_NAMESPACE {

/* On-chip caches
 *
 * The 68020 has an instruction cache of 64 longwords. The 68030 has an
 * instruction cache and a data cache of 256 bytes each, organized as 16 lines
 * of four longwords. The caches of the 68040 models are not emulated. They are
 * physically tagged, 4-way set-associative and support copyback mode, which is
 * a different design.
 *
 * Organization
 *
 * The caches are direct-mapped. An address selects a line by its index bits and
 * a longword inside the line by its longword bits. The tag contains the upper
 * 24 address bits and the supervisor bit, which keeps user and supervisor code
 * from aliasing the same line:
 *
 *   tag = xxxxxxxx xxxxxxxx xxxxxxxx -------S     ( (addr & ~0xFF) | S )
 *
 * Two layouts exist. They differ in the line size only:
 *
 *   68020:     64 lines of 1 longword   index = addr[7:2]
 *   68030:      16 lines of 4 longwords  index = addr[7:4], longword = addr[3:2]
 *
 * Each longword has its own valid bit, so a line can be partially filled, e.g.,
 * after a miss that is served by a single longword transfer. Allocating a
 * longword in a line with a different tag invalidates the line first.
 *
 * Both layouts store 64 longwords. Because the longword number is addr[7:2] in
 * either case, the data and valid arrays are shared and only the tag index and
 * the grouping of valid bits into lines differ.
 *
 * Logical addressing
 *
 * Tags are logical addresses. A hit is served without involving the MMU, which
 * means that it can neither fault nor update the translation tables. Only a
 * miss translates the address, which also reveals the cache-inhibit attribute
 * of the page. Inhibited accesses bypass the caches.
 *
 * Cache control
 *
 * The Cache Control Register (CACR) controls the caches:
 *
 *   68020                          68030
 *   Bit 0 (E)  Enable              Bit 0 (EI)   Enable instruction cache
 *   Bit 1 (F)  Freeze              Bit 1 (FI)   Freeze instruction cache
 *   Bit 2 (CE) Clear entry         Bit 2 (CEI)  Clear instruction cache entry
 *   Bit 3 (C)  Clear               Bit 3 (CI)   Clear instruction cache
 *                                  Bit 4 (IBE)  Enable instruction burst fills
 *                                  Bit 8 (ED)   Enable data cache
 *                                  Bit 9 (FD)   Freeze data cache
 *                                  Bit 10 (CED) Clear data cache entry
 *                                  Bit 11 (CD)  Clear data cache
 *                                  Bit 12 (DBE) Enable data burst fills
 *                                  Bit 13 (WA)  Write allocate
 *
 * Clearing the enable bit bypasses the cache entirely. Lookups never hit and
 * fills never happen. This has to be checked on every lookup, not just on fill.
 * Freezing stops new entries from being allocated, but entries that are already
 * valid keep being served as hits (and are updated by writes). The clear-entry
 * bits invalidate the longword selected by the CAAR register.
 *
 * Misses and burst fills
 *
 * A miss reads the missing longword. If burst fills are enabled, the cache is
 * not frozen and the port is 32 bits wide, the remaining longwords of the
 * line are fetched right behind it (in wrap-around order). A burst fill is
 * only started if the missed longword is the line's only valid entry. A narrower
 * port needs two or four smaller transactions per longword and doesn't burst.
 *
 * Data cache
 *
 * The data cache is write-through. Every write reaches the bus. A write hit
 * updates the cached bytes. With write allocate (WA), an aligned longword write
 * allocates the entry. Writes never allocate in an inhibited page.
 *
 * Instruction cache latch
 *
 * Moira additionally keeps a one-entry "latch" (CacheLatch) in front of the
 * instruction cache. It records the longword last fetched, keyed by its base
 * address; a hit on the latch needs no cache lookup and no bus cycle at all,
 * modeling the CPU re-reading the same longword for the high and low
 * instruction word of a single fetch, or for consecutive fetches that
 * happen to land in the same longword.
 */
class Cache {

    // Maximum number of cached longwords
    static constexpr int capacity = 64;

protected:

    // Supported layouts
    enum class Layout { M68020, M68030 };

    // The CPU this cache belongs to
    class Moira &moira;

    // Cached longwords, indexed by addr[7:2]
    u32 data[capacity] = { };

private:

    // Tags, one per line
    u32 tags[capacity] = { };

    // Valid bits, one per longword
    bool valid[capacity] = { };

    // Selected layout
    Layout layout = Layout::M68020;


    //
    // Constructing
    //

public:

    Cache(Moira &ref) : moira(ref) { }
    virtual ~Cache() = default;

    // Selects a layout and invalidates all entries
    void reset(Layout layout);

    // Selects the layout of a CPU model and invalidates all entries
    void reset(CPUModel model);


    //
    // Querying the cache state
    //

public:

    // Returns layout properties
    Layout getLayout() const { return layout; }
    int numLongs() const { return layout == Layout::M68030 ? 4 : 1; }
    int numLines() const { return capacity / numLongs(); }

    // Indicates whether this cache is part of the emulated CPU (change via setEmulated)
    bool emulated = false;

    // Indicates whether the cache is emulated and enabled in the CACR
    bool active = false;

    // Indicates whether the CACR freezes the cache (no new entries are allocated)
    bool frozen = false;

    // Selects whether this cache is part of the emulated CPU
    void setEmulated(bool value) { emulated = value; refresh(); }

    // Derives the active and frozen flags from the CACR (call whenever it changes)
    virtual void refresh() = 0;

    // Indicates whether a miss at a physical address is served by a burst fill.
    // This requires the 68030 layout, the burst bit in the CACR and a 32 bit port.
    virtual bool burstEnabled(u32 phys) const = 0;

    //
    // Mapping entries
    //
    
protected:
    
    // Maps an address to the its absolute position in the data array
    int slot(u32 addr) const { return int(addr >> 2) & 63; }

    // Maps an address to the corresponding cache line
    int index(u32 addr) const { return slot(addr) / numLongs(); }

    // Returns the position of the longword inside its line
    int longword(u32 addr) const { return slot(addr) & (numLongs() - 1); }

    // Returns the slot of the first longword in the line an address maps to
    int first(u32 addr) const { return slot(addr) & ~(numLongs() - 1); }

    // Computes the tag of an address
    u32 tag(u32 addr, bool supervisor) const { return (addr & 0xFFFFFF00) | u32(supervisor); }
    
    
    //
    // Accessing entries
    //
    
protected:
    
    // Checks if the longword of an address is valid and carries the matching tag
    bool hit(u32 addr, bool supervisor) const;

    // Stores a longword
    void insert(u32 addr, bool supervisor, u32 value);

    // Fetches the missing longwords of a line right behind the longword that has just been loaded
    void burstFill(u32 logical, u32 physical, bool supervisor);
    
    
    //
    // Invalidating entries
    //

public:

    // Invalidates all lines (CACR.CI, CACR.CD)
    virtual void flush();

    // Invalidates the longword that an address maps to (CACR.CEI, CACR.CED)
    void invalidate(u32 addr);

protected:

    // Invalidates all longwords of the line an address maps to
    void invalidateLine(u32 addr);

    // Returns the number of valid longwords in the line an address maps to
    int validCount(u32 addr) const;


    //
    // Saving and restoring the contents
    //

public:

    /* The functions below let the application clone a CPU or save its state
     * in a snapshot. They cover the cached longwords, the tags and the valid
     * bits. The layout is derived from the CPU model, and the active and
     * frozen flags are derived from CACR. Call refresh() after CACR has been
     * restored.
     */

    // Copies the contents of another cache
    void copyContents(const Cache &other);

    // Passes the contents to a worker that implements operator<< for integral types
    template <class W> void serializeContents(W &worker)
    {
        for (int i = 0; i < capacity; i++) worker << data[i] << tags[i] << valid[i];
    }


    //
    // Delegates
    //
    
public:

    // Informs the cache that CACR contains a new value
    virtual void cacrDidChange(u32 value) = 0;

    // Informs the cache that CAAR contains a new value
    virtual void caarDidChange(u32 value) = 0;

    
};


//
// Instruction cache (68020 and up)
//

class ICache final : public Cache {

    struct CacheLatch { u32 addr; u32 data; };
    
    // The longword that has been fetched last
    CacheLatch latch = { 0xffffffff, 0xffffffff };

public:

    using Cache::Cache;

    void refresh() override;
    bool burstEnabled(u32 phys) const override;

    void flush() override;
    void cacrDidChange(u32 value) override;
    void caarDidChange(u32 value) override;

    // Forgets the longword that has been fetched last
    void flushLatch() { latch = { 0xffffffff, 0xffffffff }; }

    // Without cache emulation, records the longword of an instruction fetch (for timing only).
    // Returns true if the fetch starts a new longword, i.e., if the CPU would fill the latch.
    bool trackFetch(u32 addr) { auto base = addr & ~3u; bool fill = latch.addr != base; latch.addr = base; return fill; }

    // Reads an instruction word via the latch and the cache
    u16 fetch(u32 addr, bool &busAccess, u32 &phys);

    // Copies the contents of another instruction cache, including the latch
    void copyContents(const ICache &other) { Cache::copyContents(other); latch = other.latch; }

    // Passes the contents to a worker, including the latch (see Cache::serializeContents())
    template <class W> void serializeContents(W &worker)
    {
        Cache::serializeContents(worker);
        worker << latch.addr << latch.data;
    }

private:

    /* Serves an instruction fetch that is not covered by the latch.
     * Returns true on a miss.
     */
    bool fill(u32 base, u32 &phys);
};


//
// Data cache (68030)
//

class DCache final : public Cache {

public:

    using Cache::Cache;

    void refresh() override;
    bool burstEnabled(u32 phys) const override;

    void cacrDidChange(u32 value) override;
    void caarDidChange(u32 value) override;

    /* Reads an aligned longword
     *
     * Returns false if the access has to bypass the cache, because the page
     * is cache-inhibited. May throw a bus error.
     */
    bool read(u32 addr, u8 fc, u32 &result);

    /* Reflects a write access (the cache is write-through, so the caller has
     * already written to the bus). The mask selects the bytes of the aligned
     * longword that have been written. The ci flag tells whether the page is
     * cache-inhibited.
     */
    void write(u32 addr, bool supervisor, u32 mask, u32 value, bool ci);
};

}
