// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "../Moira.h"
#include "../MoiraMacros.h"

namespace MOIRA_NAMESPACE {

// Thrown to terminate a table walk
struct MmuWalkStop { };

// MMU context used while walking the translation tables
struct MmuContext {

    // The logical address, positioned such that the next table index sits in
    // the upper word. Each call to nextAddrBits() shifts the next index there.
    u64 addrBits;

    // The index sizes from the translation control register (TC). The nibbles
    // for the initial shift and the table indices are consumed one by one.
    u64 indexBits;

    // Total number of address bits consumed so far, computed out of the initial
    // shift and the table indices.
    u8  shiftCnt;

    // Permitted range of the next table index, derived from the limit field of
    // the root pointer or of the previous descriptor
    u32 lowerLimit;
    u32 upperLimit;

    // Protection bits accumulated over all descriptors visited so far
    bool wp;                // Write protect
    bool su;                // Supervisor only

    // Properties of the access
    bool super;             // Supervisor access (bit 2 of the function code)
    u8 fc;                  // Function code

    // Dry run (PTEST): Walk the tables without side effects. If 'level' is
    // non-zero, the walk ends at the descriptor with that number.
    bool ptest;
    u32 level;

    // Number of descriptors fetched so far and the address of the last one
    u32 depth;
    u32 descrAddr;

    // Status bits collected during the walk (the L and I bits of the MMUSR)
    u32 flags;

    // Indicates that the page has been modified (M bit of the page descriptor)
    bool modified;

    // Cache inhibit bit of the page descriptor
    bool ci;

    // Mask covering the page offset (derived from the page size in the TC register)
    u32 pageMask;

    // The letter ('A' to 'E') of the last table that is indexed by the logical address
    char lastTable;

    // The logical address that triggered the lookup
    u32 faultAddr;

    // Consumes the next index size and returns the corresponding address bits
    u32 nextAddrBits() {

        // Shift the new index into the upper word
        indexBits = (indexBits & 0xFFFFFFFF) << 4;

        // Get the upper word
        auto shift = indexBits >> 32;

        // Remember the total number of shifted bits
        shiftCnt += u8(shift);

        // Shift the new address bits into the upper word
        addrBits = (addrBits & 0xFFFFFFFF) << shift;

        // Return the upper word
        return u32(addrBits >> 32);
    }

    // Returns the next address bits without consuming them
    u32 peekAddrBits() const {

        MmuContext copy = *this;
        return copy.nextAddrBits();
    }
    
    // Returns the number of address bits that haven't been consumed yet
    u32 remainingAddrBits() const { return u32(addrBits) >> shiftCnt; }

    // Indicates a supervisor violation (supervisor-only descriptor, user access)
    bool violation() const { return su && !super; }

    // Returns the MMU status register value
    u32 status() const {

        return flags
        | (wp ? MMUSR_WRITE_PROTECTED : 0)
        | (violation() ? MMUSR_SUPERVISOR_ONLY : 0)
        | (modified ? MMUSR_MODIFIED : 0)
        | (depth & MMUSR_NUMBER_OF_LEVELS);
    }
    
    // Terminates the walk
    [[noreturn]] void stop(u32 bits = 0) { flags |= bits; throw MmuWalkStop(); }
};

void
MMU030::reset()
{
    // Reset registers
    crp = srp = 0;
    tc = tt0 = tt1 = mmusr = 0;
    
    // Reset cache
    for (auto &e : atc) e = { };
}

void
MMU030::setTC(u32 value)
{
    bool oldE = tc & (1 << 31);
    tc = value;
    bool newE = tc & (1 << 31);

    if (isEnabled()) moira.flags |= State::MMU; else moira.flags &= ~State::MMU;

    if (!oldE && newE) moira.didEnableMMU();
    if (oldE && !newE) moira.didDisableMMU();
}

void
MMU030::setTT0(u32 value)
{
    tt0 = value;
}

void
MMU030::setTT1(u32 value)
{
    tt1 = value;
}

//
// Translation
//

int
MMU030::numTables() const
{
    // A table exists for each leading non-zero TIx field in the TC register
    for (int i = 0; i < 4; i++) {
        if ((tc >> (12 - 4 * i) & 0xF) == 0) return i;
    }
    return 4;
}

bool
MMU030::testTT(u32 addr, u8 fc, bool rw) const
{
    return testTT(tt0, addr, fc, rw) || testTT(tt1, addr, fc, rw);
}

bool
MMU030::testTT(u32 ttx, u32 addr, u8 fc, bool rw)
{
    u32 addrBase  = ________xxxxxxxx (ttx >> 24);
    u32 addrMask  = ________xxxxxxxx (ttx >> 16);
    auto e        = x_______________ (ttx);
    auto rwb      = ______x_________ (ttx);
    auto rwm      = _______x________ (ttx);
    auto fcBase   = _________xxx____ (ttx);
    auto fcMask   = _____________xxx (ttx);

    if (!e) {
        return false;
    }
    if ((addr >> 24 | addrMask) != (addrBase | addrMask)) {
        return false;
    }
    if ((fc | fcMask) != (fcBase | fcMask)) {
        return false;
    }
    if ((rw || rwm) != (rwb || rwm)) {
        return false;
    }
    return true;
}

std::optional<u32>
MMU030::translate(u32 addr, u8 fc, bool write, bool *ci)
{
    if (ci) *ci = false;

    // CPU space accesses are not translated and not cached
    if (fc == 7) { if (ci) *ci = true; return addr; }

    // Check if transparent translation applies (the TT registers have a CI bit, too)
    for (u32 ttx : { tt0, tt1 }) {
        if (testTT(ttx, addr, fc, !write)) { if (ci) *ci = ttx & (1 << 10); return addr; }
    }

    // Check the cache
    auto line = atcFind(addr, fc, write);

    // On a cache miss, walk the tables and update the cache
    if (!line) {

        MmuContext context;
        u32 physAddr = walk(addr, fc, write, context);
        line = atcInsert(addr, fc, physAddr, context.status(), context.ci);
    }
    
    auto &e = atc[*line];

    // Invalid descriptors and supervisor violations cause a bus error
    if (e.busError) return std::nullopt;

    // And so do write accesses to write-protected pages
    if (write && e.wp) return std::nullopt;

    if (ci) *ci = e.ci;
    return e.physical | (addr & pageMask());
}

std::optional<u32>
MMU030::peek(u32 addr, u8 fc, bool write) const
{
    // Without translation, logical and physical addresses are the same
    if (!isEnabled() || fc == 7) return addr;

    // Check if transparent translation applies
    for (u32 ttx : { tt0, tt1 }) {
        if (testTT(ttx, addr, fc, !write)) return addr;
    }

    // Check the cache (without touching the history bits)
    u32 page = addr & ~pageMask();

    for (auto &e : atc) {

        if (!e.valid || e.logical != page || e.fc != fc) continue;

        if (e.busError || (write && e.wp)) return std::nullopt;

        // A write to an unmodified page would walk the tables to set the M bit
        if (!write || e.modified) return e.physical | (addr & pageMask());
        break;
    }

    // Walk the tables in PTEST mode, which doesn't update the descriptors
    MmuContext context;
    u32 physAddr;

    peeking = true;
    try {
        physAddr = const_cast<MMU030 *>(this)->walk(addr, fc, write, context, true, 0);
    } catch (const BusError &) {
        peeking = false;
        return std::nullopt;
    }
    peeking = false;

    u32 status = context.status();
    if (status & (MMUSR_INVALID | MMUSR_SUPERVISOR_ONLY)) return std::nullopt;
    if (write && (status & MMUSR_WRITE_PROTECTED)) return std::nullopt;

    return (physAddr & ~pageMask()) | (addr & pageMask());
}

u32
MMU030::readDescriptor(u32 addr) const
{
    if (peeking) return u32(moira.read16Dasm(addr)) << 16 | moira.read16Dasm(addr + 2);
    return moira.read32(addr);
}

void
MMU030::load(u32 addr, u8 fc, bool write)
{
    atcFlushPage(addr);

    MmuContext context;
    u32 physAddr = walk(addr, fc, write, context, false, 0);
    atcInsert(addr, fc, physAddr, context.status(), context.ci);
}

u32
MMU030::test(u32 addr, u8 fc, bool write, u32 level, u32 &descrAddr)
{
    descrAddr = 0;

    // Level 0 searches the ATC
    if (level == 0) {

        // Check for transparent translation
        if (testTT(addr, fc, !write)) return MMUSR_TRANSPARENT_ACCESS;

        u32 page = addr & ~pageMask();
        for (auto &e : atc) {

            if (!e.valid || e.logical != page || e.fc != fc) continue;

            return (e.busError ? MMUSR_BUS_ERROR | MMUSR_INVALID : 0)
                 | (e.wp ? MMUSR_WRITE_PROTECTED : 0)
                 | (e.modified ? MMUSR_MODIFIED : 0);
        }
        return MMUSR_INVALID;
    }

    MmuContext context;
    (void)walk(addr, fc, false, context, true, level);

    descrAddr = context.descrAddr;
    return context.status();
}

//
// Address translation cache
//

void
MMU030::atcTouch(int line)
{
    // Flag the entry as recently used
    atc[line].history = true;

    // If all history bits are set, clear all but the new one
    for (auto &e : atc) if (!e.history) return;
    for (auto &e : atc) e.history = false;
    atc[line].history = true;
}

std::optional<int>
MMU030::atcFind(u32 addr, u8 fc, bool write)
{
    u32 page = addr & ~pageMask();

    for (int i = 0; i < ATC_ENTRIES; i++) {

        auto &e = atc[i];
        
        // Check if the entry is valid
        if (!e.valid) continue;
        
        // Check if the logica page and the fc pins match
        if (e.logical != page || e.fc != fc) continue;

        // A write access to a page that hasn't been marked as modified yet
        // needs a table walk to set the M bit. The entry is invalidated.
        // Note: An address is mapped by at most one entry, so the search ends here.
        if (!write || e.modified || e.wp || e.busError) {

            atcTouch(i);
            return i;
        }
        
        e.valid = false;
        break;
    }
    
    return std::nullopt;
}

std::optional<int>
MMU030::atcFindFree()
{
    for (int i = 0; i < ATC_ENTRIES; i++) if (!atc[i].valid) return i;
    return std::nullopt;
}

std::optional<int>
MMU030::atcFindUnused()
{
    for (int i = 0; i < ATC_ENTRIES; i++) if (!atc[i].history) return i;
    return std::nullopt;
}

int
MMU030::atcInsert(u32 addr, u8 fc, u32 physical, u32 status, bool ci)
{
    // Find the first free entry
    auto line = atcFindFree();

    // If there is none, use the first entry wit no history bit
    if (!line) line = atcFindUnused();

    // If there is none, fallback to the first entry
    if (!line) line = 0;

    // Mark the picked entry as used
    atcTouch(*line);

    auto &e = atc[*line];
    
    e.logical   = addr & ~pageMask();
    e.physical  = physical & ~pageMask();
    e.fc        = fc;
    e.valid     = true;
    e.busError  = status & (MMUSR_INVALID | MMUSR_SUPERVISOR_ONLY);
    e.wp        = !!(status & MMUSR_WRITE_PROTECTED);
    e.modified  = status & MMUSR_MODIFIED;
    e.ci        = ci;

    return *line;
}

void
MMU030::atcFlush()
{
    for (auto &e : atc) e.valid = false;
}

void
MMU030::atcFlush(u8 fcBase, u8 fcMask)
{
    for (auto &e : atc) if ((fcBase & fcMask) == (e.fc & fcMask)) e.valid = false;
}

void
MMU030::atcFlushPage(u32 addr)
{
    for (auto &e : atc) if (e.logical == (addr & ~pageMask())) e.valid = false;
}

void
MMU030::atcFlushPage(u32 addr, u8 fcBase, u8 fcMask)
{
    for (auto &e : atc) {
        if (e.logical == (addr & ~pageMask()) && (fcBase & fcMask) == (e.fc & fcMask)) e.valid = false;
    }
}

//
// Table walk
//

u32
MMU030::walk(u32 addr, u8 fc, bool write, MmuContext &c)
{
    return walk(addr, fc, write, c, false, 0);
}

u32
MMU030::walk(u32 addr, u8 fc, bool write, MmuContext &c, bool ptest, u32 level)
{
    bool fcl = tc & (1 << 24);

    c = MmuContext {

        .addrBits   = addr,
        .indexBits  = tc << 12,
        .super      = bool(fc & 4),
        .fc         = u8(fc & 7),
        .ptest      = ptest,
        .level      = level,
        .pageMask   = pageMask(),
        .lastTable  = char('A' + numTables() - 1 + (fcl ? 1 : 0)),
        .faultAddr  = addr
    };

    // Start with the SRP or CRP
    u64 rp = (c.super && (tc & 0x02000000)) ? srp : crp;

    // +------------------------------------------------+
    // | ROOT POINTER (CRP/SRP)                         |
    // +-----+--------+---------------------------------+
    // | Bit | Length | Contents                        |
    // +-----+--------+---------------------------------+
    // | 00  |   04   | reserved ( must be 0 )          |
    // | 04  |   28   | TableA Address (upper 28 bits)  |
    // | 32  |   02   | Descriptor Type                 |
    // | 34  |   14   | reserved ( must be 0 )          |
    // | 48  |   15   | Limit                           |
    // | 63  |   01   | L/U                             |
    // +----+---------+---------------------------------+

    u32 taddr = u32(rp >> 0 ) & 0xFFFFFFF0;
    u32 dt    = u32(rp >> 32) & 0x3;
    u32 limit = u32(rp >> 48) & 0x7FFF;

    // Evaluate the limit field
    if (rp & (1LL << 63)) {

        c.lowerLimit = limit;
        c.upperLimit = 0XFFFF;

    } else {

        c.lowerLimit = 0;
        c.upperLimit = limit;
    }

    // Apply the initial shift (ignore some bits)
    (void)c.nextAddrBits();

    try {

        // Check the descriptor type
        switch (dt) {

            case 1:
            {
                // Early termination. No translation table exists. The physical
                // address is the sum of the TableA address and the logical address
                // (without the ignored bits). The limit applies to the first index
                // unless function code lookup is enabled.

                if (!fcl) {

                    u32 index = c.peekAddrBits();
                    if (index < c.lowerLimit || index > c.upperLimit) {
                        c.stop(MMUSR_LIMIT_VIOLATION | MMUSR_INVALID);
                    }
                }
                return (u32(rp) & 0xFFFFFF00 & ~c.pageMask) + c.remainingAddrBits();
            }
            case 2:
            case 3:
            {
                u32 offset;

                if (fcl) {

                    // The function code is used as the first index (no limit check)
                    offset = c.fc;
                    c.lowerLimit = 0;
                    c.upperLimit = 0xFFFF;

                } else {

                    offset = c.nextAddrBits();
                }

                return dt == 2 ? lookupShort('A', taddr, offset, write, c)
                               : lookupLong('A', taddr, offset, write, c);
            }
            default:

                c.stop(MMUSR_INVALID);
        }

    } catch (const MmuWalkStop &) { }

    return 0;
}

u32
MMU030::lookupShort(char table, u32 taddr, u32 offset, bool write, MmuContext &c)
{
    u32 physAddr;

    // Check offset
    if (offset < c.lowerLimit || offset > c.upperLimit) {
        c.stop(MMUSR_LIMIT_VIOLATION | MMUSR_INVALID);
    }
    
    // Read table entry
    u32 descrAddr = taddr + 4 * offset;
    u32 descriptor = readDescriptor(descrAddr);
    c.depth++;
    c.descrAddr = descrAddr;

    // Extract descriptor type
    u32 dt = descriptor & 0x3;

    switch (dt) {

        case 0:
        {
            // +------------------------------------------------+
            // | SHORT FORMAT INVALID DESCRIPTOR                |
            // +-----+--------+---------------------------------+
            // | 00  |   02   | Descriptor Type (DT)            |
            // | 02  |   30   | Unused                          |
            // +-----+--------+---------------------------------+

            c.stop(MMUSR_INVALID);
        }
        case 1:
        {
            // +------------------------------------------------+
            // | SHORT FORMAT EARLY TERM. OR PAGE DESCRIPTOR    |
            // +-----+--------+---------------------------------+
            // | Bit | Length | Contents                        |
            // +-----+--------+---------------------------------+
            // | 00  |   02   | Descriptor Type (DT)            |
            // | 02  |   01   | Write Protect (WP)              |
            // | 03  |   01   | Used (U)                        |
            // | 04  |   01   | Modified (M)                    |
            // | 05  |   01   | Reserved                        |
            // | 06  |   01   | Cache Inhibit (CI)              |
            // | 07  |   01   | Reserved                        |
            // | 08  |   24   | Page Address                    |
            // +-----+--------+---------------------------------+

            // Record WP and CI
            c.wp |= !!(descriptor & 0x4);
            c.ci = !!(descriptor & 0x40);

            // Update the U bit and the M bit (the latter only if the page is writable)
            if (!c.ptest && !c.violation()) {

                u32 desc2 = descriptor | 0x8;
                if (write) { if (!c.wp) desc2 |= 0x10; }
                if (desc2 != descriptor) moira.write16(descrAddr + 2, (u16)desc2);
                descriptor = desc2;
            }
            if (descriptor & 0x10) c.modified = true;

            // The page offset consists of the unused index bits and the page offset
            physAddr = (descriptor & 0xFFFFFF00 & ~c.pageMask) + c.remainingAddrBits();
            break;
        }
        default:
        {
            // +------------------------------------------------+
            // | SHORT FORMAT TABLE DESCRIPTOR                  |
            // +-----+--------+---------------------------------+
            // | Bit | Length | Contents                        |
            // +-----+--------+---------------------------------+
            // | 00  |   02   | Descriptor Type (DT)            |
            // | 02  |   01   | Write Protect (WP)              |
            // | 03  |   01   | Used (U)                        |
            // | 04  |   28   | Table Address                   |
            // +-----+--------+---------------------------------+

            if (table > c.lastTable) {

                // An indirect descriptor must point to a page descriptor
                c.stop(MMUSR_INVALID);
            }

            if (table == c.lastTable) {

                // Indirect descriptor. The pointer is used as is. Neither
                // the protection bits nor the U bit are evaluated.
                if (c.level == c.depth) c.stop();

                c.lowerLimit = 0;
                c.upperLimit = 0xFFFF;
                taddr = descriptor & 0xFFFFFFFC;

                physAddr = dt == 2 ? lookupShort(table + 1, taddr, 0, write, c)
                                   : lookupLong(table + 1, taddr, 0, write, c);

            } else {

                // Record WP
                c.wp |= !!(descriptor & 0x4);

                // Update the U bit
                if (!c.ptest && !c.violation() && !(descriptor & 0x8)) moira.write16(descrAddr + 2, (u16)(descriptor | 0x8));

                // Stop here if PTEST asked for this level
                if (c.level == c.depth) c.stop();

                offset = c.nextAddrBits();
                taddr = descriptor & 0xFFFFFFF0;

                // Short descriptors don't carry a limit
                c.lowerLimit = 0;
                c.upperLimit = 0xFFFF;

                physAddr = dt == 2 ? lookupShort(table + 1, taddr, offset, write, c)
                                   : lookupLong(table + 1, taddr, offset, write, c);
            }
            break;
        }
    }

    return physAddr;
}

u32
MMU030::lookupLong(char table, u32 taddr, u32 offset, bool write, MmuContext &c)
{
    u32 physAddr;

    // Check offset
    if (offset < c.lowerLimit || offset > c.upperLimit) {
        c.stop(MMUSR_LIMIT_VIOLATION | MMUSR_INVALID);
    }

    // Read table entry
    u32 descrAddr = taddr + 8 * offset;
    u64 descriptor = u64(readDescriptor(descrAddr)) << 32 | readDescriptor(descrAddr + 4);
    c.depth++;
    c.descrAddr = descrAddr;

    // The upper word contains the flags, the lower word the address
    u32 flags = u32(descriptor >> 32);
    u32 addr  = u32(descriptor);

    // Extract descriptor type
    u32 dt = flags & 0x3;

    // +------------------------------------------------+
    // | LONG FORMAT DESCRIPTORS                        |
    // +-----+--------+---------------------------------+
    // | Bit | Length | Contents                        |
    // +-----+--------+---------------------------------+
    // | 00  |   02   | Descriptor Type (DT)            |
    // | 02  |   01   | Write Protect (WP)              |
    // | 03  |   01   | Used (U)                        |
    // | 04  |   01   | Modified (M)  (page only)       |
    // | 06  |   01   | Cache Inhibit (CI) (page only)  |
    // | 08  |   01   | Supervisor Only (S)             |
    // | 16  |   15   | Limit                           |
    // | 31  |   01   | L/U                             |
    // | 32  |   32   | Table or page address           |
    // +-----+--------+---------------------------------+

    if (dt == 0) c.stop(MMUSR_INVALID);

    if (dt > 1 && table > c.lastTable) {

        // An indirect descriptor must point to a page descriptor
        c.stop(MMUSR_INVALID);
    }

    if (dt > 1 && table == c.lastTable) {

        // Indirect descriptor. The pointer is used as is. Neither
        // the protection bits nor the U bit are evaluated.
        if (c.level == c.depth) c.stop();

        c.lowerLimit = 0;
        c.upperLimit = 0xFFFF;
        taddr = addr & 0xFFFFFFFC;

        physAddr = dt == 2 ? lookupShort(table + 1, taddr, 0, write, c)
                           : lookupLong(table + 1, taddr, 0, write, c);

    } else {

        // Record WP and S bit
        c.wp |= !!(flags & 0x4);
        c.su |= !!(flags & 0x100);

        if (dt == 1) {

            // Page descriptor (or early termination)
            c.ci = !!(flags & 0x40);

            if (!c.ptest && !c.violation()) {

                u32 flags2 = flags | 0x8;
                if (write) { if (!c.wp) flags2 |= 0x10; }
                if (flags2 != flags) moira.write16(descrAddr + 2, (u16)flags2);
                flags = flags2;
            }
            if (flags & 0x10) c.modified = true;

            // In case of early termination, the limit applies to the next index
            if (table < c.lastTable) {

                u32 index = c.peekAddrBits();
                u32 limit = flags >> 16 & 0x7FFF;
                if ((flags & 0x80000000) ? index < limit : index > limit) {
                    c.stop(MMUSR_LIMIT_VIOLATION | MMUSR_INVALID);
                }
            }

            physAddr = (addr & 0xFFFFFF00 & ~c.pageMask) + c.remainingAddrBits();

        } else {

            // Table descriptor. Update the U bit.
            if (!c.ptest && !c.violation() && !(flags & 0x8)) moira.write16(descrAddr + 2, (u16)(flags | 0x8));

            // Stop here if PTEST asked for this level
            if (c.level == c.depth) c.stop();

            offset = c.nextAddrBits();
            taddr = addr & 0xFFFFFFF0;

            // The limit applies to the next index
            u32 limit = flags >> 16 & 0x7FFF;
            if (flags & 0x80000000) { c.lowerLimit = limit; c.upperLimit = 0xFFFF; }
            else                    { c.lowerLimit = 0; c.upperLimit = limit; }

            physAddr = dt == 2 ? lookupShort(table + 1, taddr, offset, write, c)
                               : lookupLong(table + 1, taddr, offset, write, c);
        }
    }

    return physAddr;
}

}
