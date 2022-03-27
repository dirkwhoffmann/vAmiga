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
#include "AmigaObject.h"
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

std::optional <isize>
HunkDescriptor::seek(u32 type)
{
    for (auto &section : sections) {
        
        if (section.type == type) return isize(section.offset);
    }
    return { };
}

void
HunkDescriptor::dump(Category category) const
{
    dump(category, std::cout);
}

void
HunkDescriptor::dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Sections) {
        
        for (usize s = 0; s < sections.size(); s++) {
            
            // auto offset = "Offset " + std::to_string(sections[s].offset);
            auto offset = "+" + std::to_string(sections[s].offset);
            auto type = HunkTypeEnum::key(sections[s].type);

            os << tab("Section " + std::to_string(s));
            os << std::setw(13) << std::left << std::setfill(' ') << type;
            os << "  " << offset << std::endl;
        }
    }
}

void
ProgramUnitDescriptor::init(const u8 *buf, isize len)
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
                        
        for (usize i = 0; i < count; i++) (void)read();
    }

    // Read block count and range
    auto numHunks = isize(read());
    if (numHunks == 0) throw VAError(ERROR_HUNK_NO_SECTIONS);
    auto first = isize(read());
    if (first != 0) throw VAError(ERROR_HUNK_UNSUPPORTED);
    auto last = isize(read());
    if (last != numHunks - 1) throw VAError(ERROR_HUNK_UNSUPPORTED);

    // Read hunk sizes
    for (isize i = 0; i < numHunks; i++) {

        auto value = read();
        auto memSize = (value & 0x3FFFFFFF) << 2;
        auto memFlags = (value & 0xC0000000) >> 29;
        if (memFlags == (MEMF_CHIP | MEMF_FAST)) {
            memFlags = read() & ~(1 << 30);
        }
        memFlags = memFlags | MEMF_PUBLIC;

        hunks.push_back( HunkDescriptor { .memSize = memSize, .memFlags = memFlags } );
    }
        
    // Scan sections (s) of all hunks (h)
    for (isize h = 0, s = 0; h < numHunks; s++) {
        
        // Read type
        auto type = read() & 0x3FFFFFFF;

        // Continue with the next hunk if an END block is found
        // if (type == HUNK_END) { h++; continue; }
        
        // Add a new section
        hunks[h].sections.push_back(SectionDescriptor());
        auto &section = hunks[h].sections.back();
        section.type = type;
        section.offset = u32(offset) - 4;
        
        switch (type) {
                
            case HUNK_NAME:
            case HUNK_CODE:
            case HUNK_DATA:
                
                section.size = 4 * read();
                offset += section.size;
                break;

            case HUNK_BSS:
                
                section.size = 4 * read();
                break;
                
            case HUNK_RELOC32:

                for (auto count = read(); count; count = read()) {

                    (void)read();
                    section.size += 4 * count;
                    offset += 4 * count;
                }
                break;
                
            case HUNK_EXT:
            case HUNK_SYMBOL:

                for (auto count = read(); count; count = read()) {
                    
                    section.size += 4 * count;
                    offset += 4 * count + 4;
                }
                break;
                
            case HUNK_DEBUG:
                
                section.size = 4 * read();
                offset += section.size;
                break;
                
            case HUNK_END:

                section.size = 0;
                h++; continue;
                // break;
                
            case HUNK_HEADER:
                
                // There cannot be a second header section
                throw VAError(ERROR_HUNK_CORRUPTED);
                break;
                
            case HUNK_OVERLAY:

                section.size = 4 * read();
                offset += section.size;
                break;

            case HUNK_BREAK:

                section.size = 0;
                break;

            default:
                
                throw VAError(ERROR_HUNK_UNSUPPORTED, HunkTypeEnum::key(type));
        }
    }
}

void
ProgramUnitDescriptor::init(const Buffer<u8> &buf)
{
    init(buf.ptr, buf.size);
}

std::optional <isize>
ProgramUnitDescriptor::seek(u32 type)
{
    for (auto &hunk : hunks) {
        
        if (auto result = hunk.seek(type); result) return *result;
    }
    return { };
}

void
ProgramUnitDescriptor::dump(Category category) const
{
    dump(category, std::cout);
}

void
ProgramUnitDescriptor::dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Hunks || category == Category::Sections) {
        
        for (usize h = 0; h < hunks.size(); h++) {
            
            auto bytes = std::to_string(hunks[h].memSize) + " Bytes";
            
            os << tab("Hunk " + std::to_string(h));
            os << std::setw(13) << std::left << std::setfill(' ') << bytes;
            os << "  " << MemFlagsEnum::key(hunks[h].memFlags) << std::endl;
            
            if (category == Category::Sections) {
                
                os << std::endl;
                hunks[h].dump(Category::Sections);
                os << std::endl;
            }
        }
    }
}
