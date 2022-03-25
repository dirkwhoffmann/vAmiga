// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "OSDescriptors.h"
#include "MemUtils.h"
#include "IOUtils.h"
#include "Error.h"

string
HunkTypeEnum::key(u32 type) {
    
    switch (type) {
            
        case HUNK_UNIT:     return string("UNIT");
        case HUNK_CODE:     return string("CODE");
        case HUNK_NAME:     return string("NAME");
        case HUNK_DATA:     return string("DATA");
        case HUNK_BSS:      return string("BSS");
        case HUNK_RELOC32:  return string("RELOC32");
        case HUNK_RELOC16:  return string("RELOC16");
        case HUNK_RELOC8:   return string("RELOC8");
        case HUNK_EXT:      return string("EXT");
        case HUNK_SYMBOL:   return string("SYMBOL");
        case HUNK_DEBUG:    return string("DEBUG");
        case HUNK_END:      return string("END");
        case HUNK_HEADER:   return string("HEADER");
        case HUNK_OVERLAY:  return string("OVERLAY");
        case HUNK_BREAK:    return string("BREAK");
        case HUNK_DREL32:   return string("DREL32");
        case HUNK_DREL16:   return string("DREL16");
        case HUNK_DREL8:    return string("DREL8");
        case HUNK_LIB:      return string("LIB");
        case HUNK_INDEX:    return string("INDEX");
            
        default:
            return std::to_string(type);
    }
};

string
MemFlagsEnum::key(u32 value) {
            
    string result;
    
    auto add = [&](const char *str) {
        
        if (!result.empty()) result += " | ";
        result += string(str);
    };
    
    if (value & MEMF_PUBLIC) add("PUBLIC");
    if (value & MEMF_CHIP)   add("CHIP");
    if (value & MEMF_FAST)   add("FAST");
    
    return result;
};

void
HunkDescriptor::dump() const
{
    dump(std::cout);
}

void
HunkDescriptor::dump(std::ostream& os) const
{
    using namespace util;

    os << std::setw(10) << std::left << std::setfill(' ') << HunkTypeEnum::key(type);
    os << std::setw(10) << std::left << offset;
    os << std::setw(10) << std::left << size;
    os << std::setw(10) << std::left << memSize;
    os << std::setw(10) << std::left << std::setfill(' ') << MemFlagsEnum::key(memFlags);
    os << std::endl;
}

void
ExeDescriptor::init(const u8 *buf, isize len)
{
    assert(buf);

    isize offset = 0;
    
    auto read = [&]() {
        
        if (offset + 4 > len) throw VAError(ERROR_HUNK_CORRUPTED);
        auto result = R32BE_ALIGNED(buf + offset);
        offset += 4;
        return result;
    };

    auto cookie = read();
    
    // Check magic cookie
    if (cookie != HUNK_HEADER) throw VAError(ERROR_HUNK_BAD_COOKIE);
    
    // Skip strings
    for (auto count = read(); count != 0; count = read()) {
                        
        for (isize i = 0; i < count; i++) (void)read();
    }

    // Read block count
    auto sections = read();
    if (sections == 0) throw VAError(ERROR_HUNK_NO_SECTIONS);
    
    // Read hunk range
    auto first = read();
    if (first != 0) throw VAError(ERROR_HUNK_UNSUPPORTED);
    auto last = read();
    if (last != sections - 1) throw VAError(ERROR_HUNK_UNSUPPORTED);

    // Read hunk sizes
    for (isize i = 0; i < sections; i++) {

        auto value = read();
        auto memSize = (value & 0x3FFFFFFF) << 2;
        auto memFlags = (value & 0xC0000000) >> 29;
        if (memFlags == (MEMF_CHIP | MEMF_FAST)) {
            memFlags = read() & ~(1 << 30);
        }
        memFlags = memFlags | MEMF_PUBLIC;

        hunks.push_back( HunkDescriptor { .memSize = memSize, .memFlags = memFlags } );
    }
    
    // Scan hunks
    for (isize i = 0; i < sections; i++) {

        // Store offset and read type
        hunks[i].offset = u32(offset);
        hunks[i].type = read() & 0x3FFFFFFF;
        
        printf("type = %d\n", hunks[i].type);
        
        switch (hunks[i].type) {
                
            case HUNK_NAME:
            case HUNK_CODE:
            case HUNK_DATA:
                
                hunks[i].size = 4 * read();
                offset += hunks[i].size;
                break;

            case HUNK_BSS:
                
                hunks[i].size = 4 * read();
                offset += 4;
                break;
                
            case HUNK_RELOC32:

                for (auto count = read(); count; count = read()) {

                    (void)read();
                    printf("count = %d\n", count);
                    hunks[i].size += 4 * count;
                    offset += 4 * count;
                }
                printf("RELOC32 size = %d\n", hunks[i].size);
                break;
                
            case HUNK_EXT:
            case HUNK_SYMBOL:

                for (auto count = read(); count; count = read()) {
                    
                    hunks[i].size += 4 * count;
                    offset += 4 * count + 4;
                }
                break;
                
            case HUNK_DEBUG:
                
                hunks[i].size = 4 * read();
                offset += hunks[i].size;
                break;
                
            case HUNK_END:

                hunks[i].size = 0;
                break;
                
            case HUNK_HEADER:
                
                // There cannot be a second header section
                throw VAError(ERROR_HUNK_CORRUPTED);
                break;
                
            case HUNK_OVERLAY:

                hunks[i].size = 4 * read();
                offset += hunks[i].size;
                break;

            case HUNK_BREAK:

                hunks[i].size = 0;
                break;

            default:
                
                throw VAError(ERROR_HUNK_UNSUPPORTED, HunkTypeEnum::key(hunks[i].type));
        }
    }
}

void
ExeDescriptor::dump() const
{
    dump(std::cout);
}

void
ExeDescriptor::dump(std::ostream& os) const
{
    using namespace util;

    os << "Nr  Type      Offset    Length    Mem size  Mem flags" << std::endl;

    for (usize i = 0; i < hunks.size(); i++) {

        os << std::setw(2) << std::right << std::setfill('0') << i << "  ";
        hunks[i].dump(os);
    }
}
