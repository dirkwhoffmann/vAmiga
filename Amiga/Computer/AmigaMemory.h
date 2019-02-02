// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_MEMORY_INC
#define _AMIGA_MEMORY_INC

#include "HardwareComponent.h"

class AmigaMemory : public HardwareComponent {
    
public:
    
    // Boot Rom (Amiga 1000 only)
    uint8_t *bootRom = NULL;
    size_t bootRomSize = 0;
    
    // Kickstart Rom and size
    uint8_t *kickRom = NULL;
    size_t kickRomSize = 0;
    
    // Chip Ram and size
    uint8_t *chipRam = NULL;
    size_t chipRamSize = 0;
    
    // Slow Ram and size
    uint8_t *slowRam = NULL;
    size_t slowRamSize = 0;
    
    // Fast Ram and size
    uint8_t *fastRam = NULL;
    size_t fastRamSize = 0;
    
    /* Indicates if the Kickstart Rom is writable
     * If an A500 or A2000 is emulated, this variable is always false. If an
     * A1000 is emulated, it is true on startup to emulate WOM (Write Once
     * Memory).
     */
    bool kickIsWritable = false;
    
    /* We divide the memory into banks of size 64KB.
     * The Amiga has 24 address lines. Hence, the accessible memory is divided
     * into 256 different banks. For each bank, this array indicates the
     * type of memory that is seen by the Amiga.
     */
    MemorySource memSrc[256];
    
    // Text storage used by ascii()
    char str[17];
    
public:
    
    //
    // Constructing and destructing
    //
    
public:
    
    AmigaMemory();
    ~AmigaMemory();
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    
    
    //
    // Allocating memory
    //
    
    bool allocateBootRom();
    bool allocateKickRom();
    bool allocateChipRam(size_t size);
    bool allocateSlowRam(size_t size);
    bool allocateFastRam(size_t size);

    // Deallocates all previously allocated memory
    void dealloc();
    
private:
    
    // Convenience wrapper around 'new'
    bool alloc(size_t size, uint8_t *&ptrref, size_t &sizeref);

    // Convenience wrapper around 'new'
    void dealloc(uint8_t *&ptrref, size_t &sizeref);
    
public:
    
    // Returns the memory source lookup table.
    MemorySource *getMemSrcTable() { return memSrc; }
    
    // Updates the memory source lookup table.
    void updateMemSrcTable();
    
    /* Returns the memory source for a given address.
     */
    MemorySource getMemSrc(uint32_t addr);
    
    /* Peeks a value from memory.
     */
    uint8_t peek8(uint32_t addr);
    uint16_t peek16(uint32_t addr);

    /* Peeks a byte from memory without causing side effects.
     */
    uint8_t spypeek8(uint32_t addr);
    uint16_t spypeek16(uint32_t addr);

    /* Returns 16 bytes of memory as an ASCII string.
     */
    const char *ascii(uint32_t addr);
    
    /* Pokes a byte into memory.
     */
    void poke8(uint32_t addr, uint8_t value);
    void poke16(uint32_t addr, uint16_t value);

   
    

};

#endif
