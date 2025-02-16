// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
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
    if (align != 1 && IS_ODD(addr)) throw CoreError(Fault::ADDR_UNALIGNED);

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
    if (sz != 1 && IS_ODD(addr)) throw CoreError(Fault::ADDR_UNALIGNED);
    
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
    if (sz != 1 && IS_ODD(addr)) throw CoreError(Fault::ADDR_UNALIGNED);
    
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
    if (!stream.is_open()) throw CoreError(Fault::FILE_NOT_FOUND, path);

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
    if (!stream.is_open()) throw CoreError(Fault::FILE_CANT_CREATE, path);

    save(stream, addr, count);
}

bool
MemoryDebugger::isReadable(Reg reg) const
{
    switch (reg) {

        case Reg::DMACONR:   case Reg::VPOSR:     case Reg::VHPOSR:    case Reg::DSKDATR:
        case Reg::JOY0DAT:   case Reg::JOY1DAT:   case Reg::CLXDAT:    case Reg::ADKCONR:
        case Reg::POT0DAT:   case Reg::POT1DAT:   case Reg::POTGOR:    case Reg::SERDATR:
        case Reg::DSKBYTR:   case Reg::INTENAR:   case Reg::INTREQR:

            return true;

        case Reg::DENISEID:

            return denise.isECS();

        default:

            return false;
    }
}

bool
MemoryDebugger::isWritable(Reg reg) const
{
    switch (reg) {

        case Reg::DSKPTH:    case Reg::DSKPTL:    case Reg::DSKLEN:    case Reg::DSKDAT:
        case Reg::REFPTR:    case Reg::VPOSW:     case Reg::VHPOSW:    case Reg::COPCON:
        case Reg::SERDAT:    case Reg::SERPER:    case Reg::POTGO:     case Reg::JOYTEST:
        case Reg::STREQU:    case Reg::STRVBL:    case Reg::STRHOR:    case Reg::STRLONG:
        case Reg::BLTCON0:   case Reg::BLTCON1:   case Reg::BLTAFWM:   case Reg::BLTALWM:
        case Reg::BLTCPTH:   case Reg::BLTCPTL:   case Reg::BLTBPTH:   case Reg::BLTBPTL:
        case Reg::BLTAPTH:   case Reg::BLTAPTL:   case Reg::BLTDPTH:   case Reg::BLTDPTL:
        case Reg::BLTSIZE:   case Reg::BLTCMOD:   case Reg::BLTBMOD:   case Reg::BLTAMOD:
        case Reg::BLTDMOD:   case Reg::BLTCDAT:   case Reg::BLTBDAT:   case Reg::BLTADAT:
        case Reg::DSKSYNC:   case Reg::COP1LCH:   case Reg::COP1LCL:   case Reg::COP2LCH:
        case Reg::COP2LCL:   case Reg::COPJMP1:   case Reg::COPJMP2:   case Reg::COPINS:
        case Reg::DIWSTRT:   case Reg::DIWSTOP:   case Reg::DDFSTRT:   case Reg::DDFSTOP:
        case Reg::DMACON:    case Reg::CLXCON:    case Reg::INTENA:    case Reg::INTREQ:
        case Reg::ADKCON:    case Reg::AUD0LCH:   case Reg::AUD0LCL:   case Reg::AUD0LEN:
        case Reg::AUD0PER:   case Reg::AUD0VOL:   case Reg::AUD0DAT:   case Reg::AUD1LCH:
        case Reg::AUD1LCL:   case Reg::AUD1LEN:   case Reg::AUD1PER:   case Reg::AUD1VOL:
        case Reg::AUD1DAT:   case Reg::AUD2LCH:   case Reg::AUD2LCL:   case Reg::AUD2LEN:
        case Reg::AUD2PER:   case Reg::AUD2VOL:   case Reg::AUD2DAT:   case Reg::AUD3LCH:
        case Reg::AUD3LCL:   case Reg::AUD3LEN:   case Reg::AUD3PER:   case Reg::AUD3VOL:
        case Reg::AUD3DAT:   case Reg::BPL1PTH:   case Reg::BPL1PTL:   case Reg::BPL2PTH:
        case Reg::BPL2PTL:   case Reg::BPL3PTH:   case Reg::BPL3PTL:   case Reg::BPL4PTH:
        case Reg::BPL4PTL:   case Reg::BPL5PTH:   case Reg::BPL5PTL:   case Reg::BPL6PTH:
        case Reg::BPL6PTL:   case Reg::BPLCON0:   case Reg::BPLCON1:   case Reg::BPLCON2:
        case Reg::BPL1MOD:   case Reg::BPL2MOD:   case Reg::BPL1DAT:   case Reg::BPL2DAT:
        case Reg::BPL3DAT:   case Reg::BPL4DAT:   case Reg::BPL5DAT:   case Reg::BPL6DAT:
        case Reg::SPR0PTH:   case Reg::SPR0PTL:   case Reg::SPR1PTH:   case Reg::SPR1PTL:
        case Reg::SPR2PTH:   case Reg::SPR2PTL:   case Reg::SPR3PTH:   case Reg::SPR3PTL:
        case Reg::SPR4PTH:   case Reg::SPR4PTL:   case Reg::SPR5PTH:   case Reg::SPR5PTL:
        case Reg::SPR6PTH:   case Reg::SPR6PTL:   case Reg::SPR7PTH:   case Reg::SPR7PTL:
        case Reg::SPR0POS:   case Reg::SPR0CTL:   case Reg::SPR0DATA:  case Reg::SPR0DATB:
        case Reg::SPR1POS:   case Reg::SPR1CTL:   case Reg::SPR1DATA:  case Reg::SPR1DATB:
        case Reg::SPR2POS:   case Reg::SPR2CTL:   case Reg::SPR2DATA:  case Reg::SPR2DATB:
        case Reg::SPR3POS:   case Reg::SPR3CTL:   case Reg::SPR3DATA:  case Reg::SPR3DATB:
        case Reg::SPR4POS:   case Reg::SPR4CTL:   case Reg::SPR4DATA:  case Reg::SPR4DATB:
        case Reg::SPR5POS:   case Reg::SPR5CTL:   case Reg::SPR5DATA:  case Reg::SPR5DATB:
        case Reg::SPR6POS:   case Reg::SPR6CTL:   case Reg::SPR6DATA:  case Reg::SPR6DATB:
        case Reg::SPR7POS:   case Reg::SPR7CTL:   case Reg::SPR7DATA:  case Reg::SPR7DATB:
        case Reg::COLOR00:   case Reg::COLOR01:   case Reg::COLOR02:   case Reg::COLOR03:
        case Reg::COLOR04:   case Reg::COLOR05:   case Reg::COLOR06:   case Reg::COLOR07:
        case Reg::COLOR08:   case Reg::COLOR09:   case Reg::COLOR10:   case Reg::COLOR11:
        case Reg::COLOR12:   case Reg::COLOR13:   case Reg::COLOR14:   case Reg::COLOR15:
        case Reg::COLOR16:   case Reg::COLOR17:   case Reg::COLOR18:   case Reg::COLOR19:
        case Reg::COLOR20:   case Reg::COLOR21:   case Reg::COLOR22:   case Reg::COLOR23:
        case Reg::COLOR24:   case Reg::COLOR25:   case Reg::COLOR26:   case Reg::COLOR27:
        case Reg::COLOR28:   case Reg::COLOR29:   case Reg::COLOR30:   case Reg::COLOR31:
        case Reg::NO_OP:

            return true;

        case Reg::BLTCON0L:  case Reg::BLTSIZV:   case Reg::BLTSIZH:   case Reg::SPRHDAT:
        case Reg::BPLCON3:   case Reg::HTOTAL:    case Reg::HSSTOP:    case Reg::HBSTRT:
        case Reg::HBSTOP:    case Reg::VTOTAL:    case Reg::VSSTOP:    case Reg::VBSTRT:
        case Reg::VBSTOP:    case Reg::BEAMCON0:  case Reg::HSSTRT:    case Reg::VSSTRT:
        case Reg::HCENTER:

            return agnus.isECS();

        case Reg::DENISEID:

            return denise.isECS();

        case Reg::DIWHIGH:

            return agnus.isECS() || denise.isECS();

        default:

            return false;
    }
}

const char *
MemoryDebugger::regName(u32 addr)
{
    return RegEnum::key(Reg((addr >> 1) & 0xFF));
}

bool
MemoryDebugger::isUnused(Reg reg) const
{
    return !isReadable(reg) && !isWritable(reg);
}

u16
MemoryDebugger::readCs(Reg reg) const
{
    if (isUnused(reg)) throw CoreError(Fault::REG_UNUSED, RegEnum::key(reg));
    if (isWritable(reg)) throw CoreError(Fault::REG_WRITE_ONLY, RegEnum::key(reg));

    return mem.peekCustom16(u32(reg) << 1);
}
void
MemoryDebugger::writeCs(Reg reg, u16 value)
{
    if (isUnused(reg)) throw CoreError(Fault::REG_UNUSED, RegEnum::key(reg));
    if (isReadable(reg)) throw CoreError(Fault::REG_READ_ONLY, RegEnum::key(reg));

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
