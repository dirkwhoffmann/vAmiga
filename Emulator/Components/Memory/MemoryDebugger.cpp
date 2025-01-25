// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "MemoryDebugger.h"
#include "Emulator.h"
#include "IOUtils.h"

namespace vamiga {

template <Accessor A> const char *
MemoryDebugger::ascDump(u32 addr, isize bytes) const
{
    assert(bytes < 256);

    static char str[256];

    for (isize i = 0; i < bytes; i += 2) {

        u16 word = mem.spypeek16 <A> ((u32)(addr + i));
        str[i] = isprint(HI_BYTE(word)) ? HI_BYTE(word) : '.';
        str[i+1] = isprint(LO_BYTE(word)) ? LO_BYTE(word) : '.';
    }
    str[bytes] = 0;
    return str;
}

template <Accessor A> const char *
MemoryDebugger::hexDump(u32 addr, isize bytes, isize sz) const
{
    assert(sz == 1 || bytes % 2 == 0);
    assert(bytes <= 64);

    static char str[256];
    char *p = str;

    for (isize i = 0; i < bytes; i++) {

        u8 byte = mem.spypeek8 <A> (u32(addr + i));

        u8 digit1 = HI_NIBBLE(byte);
        u8 digit2 = LO_NIBBLE(byte);

        *p++ = digit1 < 10 ? '0' + digit1 : 'A' + digit1 - 10;
        *p++ = digit2 < 10 ? '0' + digit2 : 'A' + digit2 - 10;

        if ((i + 1) % sz == 0) *p++ = ' ';
    }
    *p = 0;

    return str;
}

template <Accessor A> const char *
MemoryDebugger::memDump(u32 addr, isize bytes, isize sz) const
{
    assert(sz == 1 || bytes % 2 == 0);
    assert(bytes <= 32);

    static string str;
    str = string(hexDump<A>(addr, bytes, sz)) + "  " + string(ascDump<A>(addr, bytes));
    return str.c_str();

    /*
     static char str[256];

     strncpy(str, hexDump<A>(addr, bytes, sz), 126);
     strcat(str, "  ");
     strncat(str, ascDump<A>(addr, bytes), 126);

     return str;
     */
}

template <Accessor A> void
MemoryDebugger::ascDump(std::ostream& os, u32 addr, isize lines)
{
    for (isize i = 0; i < lines; i++, addr += 64) {

        os << std::setfill('0') << std::hex << std::right << std::setw(6) << isize(addr);
        os << ":  ";
        os << ascDump<A>(addr, 64);
        os << std::endl;
    }
    current = addr;
}

template <Accessor A> void
MemoryDebugger::hexDump(std::ostream& os, u32 addr, isize lines, isize sz)
{
    if (sz != 1) addr &= ~0x1;

    for (isize i = 0; i < lines; i++, addr += 16) {

        os << std::setfill('0') << std::hex << std::right << std::setw(6) << isize(addr);
        os << ":  ";
        os << hexDump<A>(addr, 16);
        os << std::endl;
    }
    current = addr;
}

template <Accessor A> void
MemoryDebugger::memDump(std::ostream& os, u32 addr, isize lines, isize sz)
{
    if (sz != 1) addr &= ~0x1;

    for (isize i = 0; i < lines; i++, addr += 16) {

        os << std::setfill('0') << std::hex << std::right << std::setw(6) << isize(addr);
        os << ":  ";
        os << hexDump<A>(addr, 16, sz);
        os << "  ";
        os << ascDump<A>(addr, 16);
        os << std::endl;
    }
    current = addr;
}

i64
MemoryDebugger::memSearch(const string &pattern, u32 addr, isize align)
{
    // Check alignment
    if (align != 1 && IS_ODD(addr)) throw Error(ErrorCode::ADDR_UNALIGNED);

    if (isize length = isize(pattern.length()); length > 0) {

        for (u32 i = addr; i < 0xFFFFFF; i += align) {

            for (isize j = 0;; j++) {

                // Get a byte from memory
                auto val = mem.spypeek8 <Accessor::CPU>(u32(i + j));

                // Stop searching if we find a mismatch
                if (val != u8(pattern[j])) break;

                // Return true if all values have matched
                if (j == length - 1) { current = i; return i; }
            }

            // Skip unmapped memory pages
            if (mem.isUnmapped(i)) { i = (i & 0xFFFF0000) + 0x010000; }
        }
    }

    return -1;
}

u32
MemoryDebugger::read(u32 addr, isize sz)
{
    u32 result;
    
    // Check alignment
    if (sz != 1 && IS_ODD(addr)) throw Error(ErrorCode::ADDR_UNALIGNED);
    
    switch (sz) {
            
        case 1: result = mem.spypeek8  <Accessor::CPU> (addr); break;
        case 2: result = mem.spypeek16 <Accessor::CPU> (addr); break;
        case 4: result = mem.spypeek32 <Accessor::CPU> (addr); break;
            
        default:
            fatalError;
    }
    
    current = u32(addr + sz);
    
    return result;
}

void
MemoryDebugger::write(u32 addr, u32 val, isize sz, isize repeats)
{
    // Check alignment
    if (sz != 1 && IS_ODD(addr)) throw Error(ErrorCode::ADDR_UNALIGNED);
    
    for (isize i = 0, a = addr; i < repeats && a <= 0xFFFFFF; i++, a += sz) {
        
        switch (sz) {
                
            case 1:
                mem.poke8  <Accessor::CPU> (u32(a), u8(val));
                break;
                
            case 2:
                mem.poke16 <Accessor::CPU> (u32(a), u16(val));
                break;
                
            case 4:
                mem.poke16 <Accessor::CPU> (u32(a), HI_WORD(val));
                mem.poke16 <Accessor::CPU> (u32(a + 2), LO_WORD(val));
                break;
                
            default:
                fatalError;
        }
    }
    
    current = u32(addr + sz * repeats);
}

void 
MemoryDebugger::load(std::istream& is, u32 addr)
{
    for (;; addr++) {

        auto val = is.get();
        if (val == EOF) return;

        mem.patch(addr, u8(val));
    }
}

void
MemoryDebugger::load(fs::path& path, u32 addr)
{
    std::ifstream stream(path, std::ifstream::binary);
    if (!stream.is_open()) throw Error(ErrorCode::FILE_NOT_FOUND, path);

    load(stream, addr);
}

void
MemoryDebugger::save(std::ostream& os, u32 addr, isize count)
{
    for (isize i = 0; i < count; i++) {

        auto val = mem.peek8 <Accessor::CPU> (u32(addr + i));
        os.put(val);
    }
}

void
MemoryDebugger::save(fs::path& path, u32 addr, isize count)
{
    std::ofstream stream(path, std::ifstream::binary);
    if (!stream.is_open()) throw Error(ErrorCode::FILE_CANT_CREATE, path);

    save(stream, addr, count);
}

bool
MemoryDebugger::isReadable(ChipsetReg reg) const
{
    switch (reg) {

        case ChipsetReg::DMACONR:   case ChipsetReg::VPOSR:     case ChipsetReg::VHPOSR:    case ChipsetReg::DSKDATR:
        case ChipsetReg::JOY0DAT:   case ChipsetReg::JOY1DAT:   case ChipsetReg::CLXDAT:    case ChipsetReg::ADKCONR:
        case ChipsetReg::POT0DAT:   case ChipsetReg::POT1DAT:   case ChipsetReg::POTGOR:    case ChipsetReg::SERDATR:
        case ChipsetReg::DSKBYTR:   case ChipsetReg::INTENAR:   case ChipsetReg::INTREQR:

            return true;

        case ChipsetReg::DENISEID:

            return denise.isECS();

        default:

            return false;
    }
}

bool
MemoryDebugger::isWritable(ChipsetReg reg) const
{
    switch (reg) {

        case ChipsetReg::DSKPTH:    case ChipsetReg::DSKPTL:    case ChipsetReg::DSKLEN:    case ChipsetReg::DSKDAT:
        case ChipsetReg::REFPTR:    case ChipsetReg::VPOSW:     case ChipsetReg::VHPOSW:    case ChipsetReg::COPCON:
        case ChipsetReg::SERDAT:    case ChipsetReg::SERPER:    case ChipsetReg::POTGO:     case ChipsetReg::JOYTEST:
        case ChipsetReg::STREQU:    case ChipsetReg::STRVBL:    case ChipsetReg::STRHOR:    case ChipsetReg::STRLONG:
        case ChipsetReg::BLTCON0:   case ChipsetReg::BLTCON1:   case ChipsetReg::BLTAFWM:   case ChipsetReg::BLTALWM:
        case ChipsetReg::BLTCPTH:   case ChipsetReg::BLTCPTL:   case ChipsetReg::BLTBPTH:   case ChipsetReg::BLTBPTL:
        case ChipsetReg::BLTAPTH:   case ChipsetReg::BLTAPTL:   case ChipsetReg::BLTDPTH:   case ChipsetReg::BLTDPTL:
        case ChipsetReg::BLTSIZE:   case ChipsetReg::BLTCMOD:   case ChipsetReg::BLTBMOD:   case ChipsetReg::BLTAMOD:
        case ChipsetReg::BLTDMOD:   case ChipsetReg::BLTCDAT:   case ChipsetReg::BLTBDAT:   case ChipsetReg::BLTADAT:
        case ChipsetReg::DSKSYNC:   case ChipsetReg::COP1LCH:   case ChipsetReg::COP1LCL:   case ChipsetReg::COP2LCH:
        case ChipsetReg::COP2LCL:   case ChipsetReg::COPJMP1:   case ChipsetReg::COPJMP2:   case ChipsetReg::COPINS:
        case ChipsetReg::DIWSTRT:   case ChipsetReg::DIWSTOP:   case ChipsetReg::DDFSTRT:   case ChipsetReg::DDFSTOP:
        case ChipsetReg::DMACON:    case ChipsetReg::CLXCON:    case ChipsetReg::INTENA:    case ChipsetReg::INTREQ:
        case ChipsetReg::ADKCON:    case ChipsetReg::AUD0LCH:   case ChipsetReg::AUD0LCL:   case ChipsetReg::AUD0LEN:
        case ChipsetReg::AUD0PER:   case ChipsetReg::AUD0VOL:   case ChipsetReg::AUD0DAT:   case ChipsetReg::AUD1LCH:
        case ChipsetReg::AUD1LCL:   case ChipsetReg::AUD1LEN:   case ChipsetReg::AUD1PER:   case ChipsetReg::AUD1VOL:
        case ChipsetReg::AUD1DAT:   case ChipsetReg::AUD2LCH:   case ChipsetReg::AUD2LCL:   case ChipsetReg::AUD2LEN:
        case ChipsetReg::AUD2PER:   case ChipsetReg::AUD2VOL:   case ChipsetReg::AUD2DAT:   case ChipsetReg::AUD3LCH:
        case ChipsetReg::AUD3LCL:   case ChipsetReg::AUD3LEN:   case ChipsetReg::AUD3PER:   case ChipsetReg::AUD3VOL:
        case ChipsetReg::AUD3DAT:   case ChipsetReg::BPL1PTH:   case ChipsetReg::BPL1PTL:   case ChipsetReg::BPL2PTH:
        case ChipsetReg::BPL2PTL:   case ChipsetReg::BPL3PTH:   case ChipsetReg::BPL3PTL:   case ChipsetReg::BPL4PTH:
        case ChipsetReg::BPL4PTL:   case ChipsetReg::BPL5PTH:   case ChipsetReg::BPL5PTL:   case ChipsetReg::BPL6PTH:
        case ChipsetReg::BPL6PTL:   case ChipsetReg::BPLCON0:   case ChipsetReg::BPLCON1:   case ChipsetReg::BPLCON2:
        case ChipsetReg::BPL1MOD:   case ChipsetReg::BPL2MOD:   case ChipsetReg::BPL1DAT:   case ChipsetReg::BPL2DAT:
        case ChipsetReg::BPL3DAT:   case ChipsetReg::BPL4DAT:   case ChipsetReg::BPL5DAT:   case ChipsetReg::BPL6DAT:
        case ChipsetReg::SPR0PTH:   case ChipsetReg::SPR0PTL:   case ChipsetReg::SPR1PTH:   case ChipsetReg::SPR1PTL:
        case ChipsetReg::SPR2PTH:   case ChipsetReg::SPR2PTL:   case ChipsetReg::SPR3PTH:   case ChipsetReg::SPR3PTL:
        case ChipsetReg::SPR4PTH:   case ChipsetReg::SPR4PTL:   case ChipsetReg::SPR5PTH:   case ChipsetReg::SPR5PTL:
        case ChipsetReg::SPR6PTH:   case ChipsetReg::SPR6PTL:   case ChipsetReg::SPR7PTH:   case ChipsetReg::SPR7PTL:
        case ChipsetReg::SPR0POS:   case ChipsetReg::SPR0CTL:   case ChipsetReg::SPR0DATA:  case ChipsetReg::SPR0DATB:
        case ChipsetReg::SPR1POS:   case ChipsetReg::SPR1CTL:   case ChipsetReg::SPR1DATA:  case ChipsetReg::SPR1DATB:
        case ChipsetReg::SPR2POS:   case ChipsetReg::SPR2CTL:   case ChipsetReg::SPR2DATA:  case ChipsetReg::SPR2DATB:
        case ChipsetReg::SPR3POS:   case ChipsetReg::SPR3CTL:   case ChipsetReg::SPR3DATA:  case ChipsetReg::SPR3DATB:
        case ChipsetReg::SPR4POS:   case ChipsetReg::SPR4CTL:   case ChipsetReg::SPR4DATA:  case ChipsetReg::SPR4DATB:
        case ChipsetReg::SPR5POS:   case ChipsetReg::SPR5CTL:   case ChipsetReg::SPR5DATA:  case ChipsetReg::SPR5DATB:
        case ChipsetReg::SPR6POS:   case ChipsetReg::SPR6CTL:   case ChipsetReg::SPR6DATA:  case ChipsetReg::SPR6DATB:
        case ChipsetReg::SPR7POS:   case ChipsetReg::SPR7CTL:   case ChipsetReg::SPR7DATA:  case ChipsetReg::SPR7DATB:
        case ChipsetReg::COLOR00:   case ChipsetReg::COLOR01:   case ChipsetReg::COLOR02:   case ChipsetReg::COLOR03:
        case ChipsetReg::COLOR04:   case ChipsetReg::COLOR05:   case ChipsetReg::COLOR06:   case ChipsetReg::COLOR07:
        case ChipsetReg::COLOR08:   case ChipsetReg::COLOR09:   case ChipsetReg::COLOR10:   case ChipsetReg::COLOR11:
        case ChipsetReg::COLOR12:   case ChipsetReg::COLOR13:   case ChipsetReg::COLOR14:   case ChipsetReg::COLOR15:
        case ChipsetReg::COLOR16:   case ChipsetReg::COLOR17:   case ChipsetReg::COLOR18:   case ChipsetReg::COLOR19:
        case ChipsetReg::COLOR20:   case ChipsetReg::COLOR21:   case ChipsetReg::COLOR22:   case ChipsetReg::COLOR23:
        case ChipsetReg::COLOR24:   case ChipsetReg::COLOR25:   case ChipsetReg::COLOR26:   case ChipsetReg::COLOR27:
        case ChipsetReg::COLOR28:   case ChipsetReg::COLOR29:   case ChipsetReg::COLOR30:   case ChipsetReg::COLOR31:
        case ChipsetReg::NO_OP:

            return true;

        case ChipsetReg::BLTCON0L:  case ChipsetReg::BLTSIZV:   case ChipsetReg::BLTSIZH:   case ChipsetReg::SPRHDAT:
        case ChipsetReg::BPLCON3:   case ChipsetReg::HTOTAL:    case ChipsetReg::HSSTOP:    case ChipsetReg::HBSTRT:
        case ChipsetReg::HBSTOP:    case ChipsetReg::VTOTAL:    case ChipsetReg::VSSTOP:    case ChipsetReg::VBSTRT:
        case ChipsetReg::VBSTOP:    case ChipsetReg::BEAMCON0:  case ChipsetReg::HSSTRT:    case ChipsetReg::VSSTRT:
        case ChipsetReg::HCENTER:

            return agnus.isECS();

        case ChipsetReg::DENISEID:

            return denise.isECS();

        case ChipsetReg::DIWHIGH:

            return agnus.isECS() || denise.isECS();

        default:

            return false;
    }
}

const char *
MemoryDebugger::regName(u32 addr)
{
    return ChipsetRegEnum::key(ChipsetReg((addr >> 1) & 0xFF));
}

bool
MemoryDebugger::isUnused(ChipsetReg reg) const
{
    return !isReadable(reg) && !isWritable(reg);
}

u16
MemoryDebugger::readCs(ChipsetReg reg) const
{
    if (isUnused(reg)) throw Error(ErrorCode::REG_UNUSED, ChipsetRegEnum::key(reg));
    if (isWritable(reg)) throw Error(ErrorCode::REG_WRITE_ONLY, ChipsetRegEnum::key(reg));

    return mem.peekCustom16(u32(reg) << 1);
}
void
MemoryDebugger::writeCs(ChipsetReg reg, u16 value)
{
    if (isUnused(reg)) throw Error(ErrorCode::REG_UNUSED, ChipsetRegEnum::key(reg));
    if (isReadable(reg)) throw Error(ErrorCode::REG_READ_ONLY, ChipsetRegEnum::key(reg));

    return mem.pokeCustom16<Accessor::CPU>(u32(reg) << 1, value);
}

void
MemoryDebugger::convertNumeric(std::ostream& os, u8 value) const
{
    using namespace util;

    os << std::setw(10) << std::right << std::setfill(' ') << dec(value) << " | ";
    os << hex(value) << " | ";
    os << bin(value) << " | ";
    os << str(value);
}

void
MemoryDebugger::convertNumeric(std::ostream& os, u16 value) const
{
    using namespace util;

    os << std::setw(10) << std::right << std::setfill(' ') << dec(value) << " | ";
    os << hex(value) << " | ";
    os << bin(value) << " | ";
    os << str(value);
}

void
MemoryDebugger::convertNumeric(std::ostream& os, u32 value) const
{
    using namespace util;

    os << std::setw(10) << std::right << std::setfill(' ') << dec(value) << " | ";
    os << hex(value) << " | ";
    os << bin(value) << " | ";
    os << str(value);
}

void
MemoryDebugger::convertNumeric(std::ostream& os, string s) const
{
    u8 bytes[4];

    bytes[0] = s.length() >= 4 ? (u8)s[s.length() - 4] : 0;
    bytes[1] = s.length() >= 3 ? (u8)s[s.length() - 3] : 0;
    bytes[2] = s.length() >= 2 ? (u8)s[s.length() - 2] : 0;
    bytes[3] = s.length() >= 1 ? (u8)s[s.length() - 1] : 0;

    convertNumeric(os, u32(HI_HI_LO_LO(bytes[0], bytes[1], bytes[2], bytes[3])));
}

template const char *MemoryDebugger::ascDump <Accessor::CPU> (u32, isize) const;
template const char *MemoryDebugger::ascDump <Accessor::AGNUS> (u32, isize) const;
template const char *MemoryDebugger::hexDump <Accessor::CPU> (u32, isize, isize) const;
template const char *MemoryDebugger::hexDump <Accessor::AGNUS> (u32, isize, isize) const;
template const char *MemoryDebugger::memDump <Accessor::CPU> (u32, isize, isize) const;
template const char *MemoryDebugger::memDump <Accessor::AGNUS> (u32, isize, isize) const;
template void MemoryDebugger::ascDump <Accessor::CPU> (std::ostream&, u32, isize);
template void MemoryDebugger::ascDump <Accessor::AGNUS> (std::ostream&, u32, isize);
template void MemoryDebugger::hexDump <Accessor::CPU> (std::ostream&, u32, isize, isize);
template void MemoryDebugger::hexDump <Accessor::AGNUS> (std::ostream&, u32, isize, isize);
template void MemoryDebugger::memDump <Accessor::CPU> (std::ostream&, u32, isize, isize);
template void MemoryDebugger::memDump <Accessor::AGNUS> (std::ostream&, u32, isize, isize);

}
