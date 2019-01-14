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
    uint8_t *kickstartRom = NULL;
    size_t kickstartRomSize = 0;
    
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
    bool kickstartIsWritable = false;
    
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
    bool allocateKickstartRom();
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
    
    /* Peeks a byte from memory.
     */
    uint8_t peek8(uint32_t addr);
    
    /* Peeks a byte from memory without causing side effects.
     */
    uint8_t spypeek8(uint32_t addr);
 
    /* Pokes a byte into memory.
     */
    void poke8(uint32_t addr, uint8_t value);
    
   
    

};

#endif
