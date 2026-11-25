// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "../Moira.h"
#include "../MoiraMacros.h"

namespace MOIRA_NAMESPACE {

void
MMU040::reset()
{
    // Reset registers
    urp = srp = 0;
    tc = itt0 = itt1 = dtt0 = dtt1 = mmusr = 0;

    // Reset caches
    for (auto &e : iatc) e = { };
    for (auto &e : datc) e = { };
}

void
MMU040::setTC(u32 value)
{
    bool oldE = tc & TC040_ENABLE;
    tc = value & (TC040_ENABLE | TC040_PAGE_8K);
    bool newE = tc & TC040_ENABLE;

    // Changing the page size invalidates all translations
    atcFlush();

    if (isEnabled()) moira.flags |= State::MMU; else moira.flags &= ~State::MMU;

    if (!oldE && newE) moira.didEnableMMU();
    if (oldE && !newE) moira.didDisableMMU();
}

void
MMU040::setITT0(u32 value)
{
    itt0 = value & 0xFFFFE364;
}

void
MMU040::setITT1(u32 value)
{
    itt1 = value & 0xFFFFE364;
}

void
MMU040::setDTT0(u32 value)
{
    dtt0 = value & 0xFFFFE364;
}

void
MMU040::setDTT1(u32 value)
{
    dtt1 = value & 0xFFFFE364;
}

//
// Translation
//

bool
MMU040::testTT(u32 addr, u8 fc, bool write) const
{
    bool super = fc & 4;

    // Instruction accesses use the ITT registers, all others the DTT registers
    bool data = (fc & 3) != 2;
    u32 tt0 = data ? dtt0 : itt0;
    u32 tt1 = data ? dtt1 : itt1;

    return testTT(tt0, addr, super, write) != TTResult::NO_MATCH ||
           testTT(tt1, addr, super, write) != TTResult::NO_MATCH;
}

MMU040::TTResult
MMU040::testTT(u32 ttx, u32 addr, bool super, bool write)
{
    u32 addrBase  = ________xxxxxxxx (ttx >> 24);
    u32 addrMask  = ________xxxxxxxx (ttx >> 16);
    auto e        = x_______________ (ttx);
    auto sIgnore  = ttx & (1 << 14);
    auto sSuper   = ttx & (1 << 13);
    auto w        = ttx & (1 << 2);

    if (!e) {
        return TTResult::NO_MATCH;
    }
    if ((addr >> 24 | addrMask) != (addrBase | addrMask)) {
        return TTResult::NO_MATCH;
    }
    if (!sIgnore && super != bool(sSuper)) {
        return TTResult::NO_MATCH;
    }
    return (write && w) ? TTResult::NO_WRITE : TTResult::MATCH;
}

std::optional<u32>
MMU040::translate(u32 addr, u8 fc, bool write, bool *ci)
{
    if (ci) *ci = false;

    // CPU space accesses are not translated and not cached
    if (fc == 7) { if (ci) *ci = true; return addr; }

    bool super = fc & 4;
    bool data  = (fc & 3) != 2;

    // Check if transparent translation applies (the TT registers have a CM field, too)
    for (u32 ttx : { data ? dtt0 : itt0, data ? dtt1 : itt1 }) {

        switch (testTT(ttx, addr, super, write)) {

            case TTResult::MATCH:    if (ci) *ci = ttx & 0x40; return addr;
            case TTResult::NO_WRITE: return std::nullopt;
            default:                 break;
        }
    }

    // Check the cache
    auto line = atcFind(addr, fc, write);

    // On a cache miss, walk the tables and update the cache
    if (!line) {

        u32 status;
        u32 physAddr = walk(addr, super, write, status);
        line = atcInsert(addr, fc, physAddr, status);
    }

    auto &e = atc(fc)[*line];

    // Invalid descriptors cause a bus error
    if (!(e.status & MMUSR040_RESIDENT)) return std::nullopt;

    // And so do write accesses to write-protected pages
    if (write && (e.status & MMUSR040_WRITE_PROTECTED)) return std::nullopt;

    // And so do user accesses to supervisor-only pages
    if (!super && (e.status & MMUSR040_SUPERVISOR_ONLY)) return std::nullopt;

    if (ci) *ci = e.status & 0x40;
    return e.physical | (addr & pageMask());
}

std::optional<u32>
MMU040::peek(u32 addr, u8 fc, bool write) const
{
    // Without translation, logical and physical addresses are the same
    if (!isEnabled() || fc == 7) return addr;

    bool super = fc & 4;
    bool data  = (fc & 3) != 2;

    // Check if transparent translation applies
    for (u32 ttx : { data ? dtt0 : itt0, data ? dtt1 : itt1 }) {

        switch (testTT(ttx, addr, super, write)) {

            case TTResult::MATCH:    return addr;
            case TTResult::NO_WRITE: return std::nullopt;
            default:                 break;
        }
    }

    // Checks whether an access to a page with the given status bits succeeds
    auto allowed = [&](u32 status) {

        if (!(status & MMUSR040_RESIDENT)) return false;
        if (write && (status & MMUSR040_WRITE_PROTECTED)) return false;
        if (!super && (status & MMUSR040_SUPERVISOR_ONLY)) return false;
        return true;
    };

    // Check the cache (without touching the history bits)
    u32 page = addr & ~pageMask();
    const Atc040Entry *cache = data ? datc : iatc;

    for (int i = 0; i < ATC040_ENTRIES; i++) {

        auto &e = cache[i];
        if (!e.valid || e.logical != page || e.super != super) continue;

        if (!allowed(e.status)) return std::nullopt;

        // A write to an unmodified page would walk the tables to set the M bit
        if (!write || (e.status & MMUSR040_MODIFIED)) return e.physical | (addr & pageMask());
        break;
    }

    // Walk the tables without updating the descriptors
    u32 status, physAddr;

    peeking = true;
    try {
        physAddr = const_cast<MMU040 *>(this)->walk(addr, super, write, status);
    } catch (const BusError &) {
        peeking = false;
        return std::nullopt;
    }
    peeking = false;

    if (!allowed(status)) return std::nullopt;
    return physAddr | (addr & pageMask());
}

u32
MMU040::readDescriptor(u32 addr) const
{
    if (peeking) return u32(moira.read16Dasm(addr)) << 16 | moira.read16Dasm(addr + 2);
    return moira.read32(addr);
}

u32
MMU040::test(u32 addr, u8 fc, bool write)
{
    bool super = fc & 4;

    // The page is looked up from scratch
    atcFlushPage(addr, super, true);

    // Check for transparent translation
    bool data = (fc & 3) != 2;
    for (u32 ttx : { data ? dtt0 : itt0, data ? dtt1 : itt1 }) {

        switch (testTT(ttx, addr, super, write)) {

            case TTResult::MATCH:    return MMUSR040_TRANSPARENT | MMUSR040_RESIDENT;
            case TTResult::NO_WRITE: return MMUSR040_BUS_ERROR;
            default:                 break;
        }
    }

    // Walk the tables and create an ATC entry
    u32 status;
    u32 physAddr = walk(addr, super, write, status);
    atcInsert(addr, fc, physAddr, status);

    return (status & MMUSR040_RESIDENT) ? (physAddr & ~pageMask()) | status : 0;
}

//
// Address translation cache
//

void
MMU040::atcTouch(Atc040Entry *atc, int line)
{
    // Flag the entry as recently used
    atc[line].history = true;

    // If all history bits are set, clear all but the new one
    for (int i = 0; i < ATC040_ENTRIES; i++) if (!atc[i].history) return;
    for (int i = 0; i < ATC040_ENTRIES; i++) atc[i].history = false;
    atc[line].history = true;
}

std::optional<int>
MMU040::atcFind(u32 addr, u8 fc, bool write)
{
    u32 page = addr & ~pageMask();
    bool super = fc & 4;
    auto *cache = atc(fc);

    for (int i = 0; i < ATC040_ENTRIES; i++) {

        auto &e = cache[i];

        // Check if the entry is valid
        if (!e.valid) continue;

        // Check if the logical page and the supervisor bit match
        if (e.logical != page || e.super != super) continue;

        // A write access to a page that hasn't been marked as modified yet
        // needs a table walk to set the M bit. The entry is invalidated.
        // Note: An address is mapped by at most one entry, so the search ends here.
        if (!write || (e.status & MMUSR040_MODIFIED) || !(e.status & MMUSR040_RESIDENT) ||
            (e.status & MMUSR040_WRITE_PROTECTED) ||
            (!super && (e.status & MMUSR040_SUPERVISOR_ONLY))) {

            atcTouch(cache, i);
            return i;
        }

        e.valid = false;
        break;
    }

    return std::nullopt;
}

std::optional<int>
MMU040::atcFindFree(Atc040Entry *atc)
{
    for (int i = 0; i < ATC040_ENTRIES; i++) if (!atc[i].valid) return i;
    return std::nullopt;
}

std::optional<int>
MMU040::atcFindUnused(Atc040Entry *atc)
{
    for (int i = 0; i < ATC040_ENTRIES; i++) if (!atc[i].history) return i;
    return std::nullopt;
}

int
MMU040::atcInsert(u32 addr, u8 fc, u32 physical, u32 status)
{
    auto *cache = atc(fc);

    // Find the first free entry
    auto line = atcFindFree(cache);

    // If there is none, use the first entry with no history bit
    if (!line) line = atcFindUnused(cache);

    // If there is none, fallback to the first entry
    if (!line) line = 0;

    // Mark the picked entry as used
    atcTouch(cache, *line);

    auto &e = cache[*line];

    e.logical  = addr & ~pageMask();
    e.physical = physical & ~pageMask();
    e.super    = fc & 4;
    e.valid    = true;
    e.status   = status;

    return *line;
}

void
MMU040::atcFlush()
{
    for (auto &e : iatc) e.valid = false;
    for (auto &e : datc) e.valid = false;
}

void
MMU040::atcFlush(bool global)
{
    for (auto &e : iatc) if (global || !(e.status & MMUSR040_GLOBAL)) e.valid = false;
    for (auto &e : datc) if (global || !(e.status & MMUSR040_GLOBAL)) e.valid = false;
}

void
MMU040::atcFlushPage(u32 addr, bool super, bool global)
{
    u32 page = addr & ~pageMask();

    for (auto *cache : { iatc, datc }) {

        for (int i = 0; i < ATC040_ENTRIES; i++) {

            auto &e = cache[i];
            if (e.logical == page && e.super == super && (global || !(e.status & MMUSR040_GLOBAL))) {
                e.valid = false;
            }
        }
    }
}

//
// Table walk
//

u32
MMU040::walk(u32 addr, bool super, bool write, u32 &status)
{
    status = 0;

    /* The 68040 uses a fixed three-level structure. The root table has 128
     * entries, the pointer tables have 128 entries, and the page tables have
     * 64 (4K pages) or 32 (8K pages) entries.
     *
     * +------------------------------------------------+
     * | TABLE DESCRIPTOR                               |
     * +-----+--------+---------------------------------+
     * | Bit | Length | Contents                        |
     * +-----+--------+---------------------------------+
     * | 00  |   02   | Upper Level Descriptor Type     |
     * | 02  |   01   | Write Protect (W)               |
     * | 03  |   01   | Used (U)                        |
     * | 04  |   28   | Table Address                   |
     * +-----+--------+---------------------------------+
     *
     * +------------------------------------------------+
     * | PAGE DESCRIPTOR                                |
     * +-----+--------+---------------------------------+
     * | Bit | Length | Contents                        |
     * +-----+--------+---------------------------------+
     * | 00  |   02   | Page Descriptor Type (PDT)      |
     * | 02  |   01   | Write Protect (W)               |
     * | 03  |   01   | Used (U)                        |
     * | 04  |   01   | Modified (M)                    |
     * | 05  |   02   | Cache Mode (CM)                 |
     * | 07  |   01   | Supervisor Only (S)             |
     * | 08  |   02   | User Page Attributes (U0, U1)   |
     * | 10  |   01   | Global (G)                      |
     * | 11  |   01   | Reserved (4K) / UR (8K)         |
     * | 12  |   20   | Page Address                    |
     * +-----+--------+---------------------------------+
     */

    bool wp = false;
    bool page8k = tc & TC040_PAGE_8K;

    // Writes the lower word of a descriptor if it carries new status bits
    auto update = [&](u32 descrAddr, u32 descriptor, u32 bits) {

        if ((descriptor & bits) == bits) return descriptor;
        descriptor |= bits;
        if (!peeking) moira.write16(descrAddr + 2, u16(descriptor));
        return descriptor;
    };

    // Root table
    u32 rp = super ? srp : urp;
    u32 descrAddr = (rp & 0xFFFFFE00) | (addr >> 23 & 0x1FC);
    u32 descriptor = readDescriptor(descrAddr);
    if (!(descriptor & 2)) return 0;

    wp |= (descriptor & MMUSR040_WRITE_PROTECTED) != 0;
    descriptor = update(descrAddr, descriptor, 0x8);

    // Pointer table
    descrAddr = (descriptor & 0xFFFFFE00) | (addr >> 16 & 0x1FC);
    descriptor = readDescriptor(descrAddr);
    if (!(descriptor & 2)) return 0;

    wp |= (descriptor & MMUSR040_WRITE_PROTECTED) != 0;
    descriptor = update(descrAddr, descriptor, 0x8);

    // Page table
    descrAddr = page8k ? (descriptor & 0xFFFFFF80) + (addr >> 11 & 0x7C)
                       : (descriptor & 0xFFFFFF00) + (addr >> 10 & 0xFC);
    descriptor = readDescriptor(descrAddr);

    // Indirect descriptor. The pointer is followed once.
    if ((descriptor & 3) == 2) {

        descrAddr = descriptor & ~3u;
        descriptor = readDescriptor(descrAddr);
    }

    // Page fault (also caused by a second indirection)
    if (!(descriptor & 1)) return 0;

    wp |= (descriptor & MMUSR040_WRITE_PROTECTED) != 0;

    // Update the U bit and the M bit (the latter only if the page is writable)
    bool protect = wp || ((descriptor & MMUSR040_SUPERVISOR_ONLY) && !super);
    descriptor = update(descrAddr, descriptor, (write && !protect) ? 0x18 : 0x8);

    // Write protection is inherited from the upper levels
    if (wp) descriptor |= MMUSR040_WRITE_PROTECTED;

    status = descriptor & MMUSR040_PAGE_BITS;
    return descriptor & ~(page8k ? 0x1FFFu : 0xFFFu);
}

}
