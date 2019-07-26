// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ZORRO_MANAGER_INC
#define _ZORRO_MANAGER_INC

/* Additional information:
 *
 *   Fast Ram emulation (Zorro II) is based on:
 *   github.com/PR77/A500_ACCEL_RAM_IDE-Rev-1/blob/master/Logic/RAM/A500_RAM.v
 */

// Manager for plugged in Zorro II devices
class ZorroManager : public HardwareComponent {

    // Quick-access references
    class Memory *mem;

    // The value returned when peeking into the auto-config space.
    uint8_t autoConfData;
    
    // The current configuration state (0 = unconfigured).
    uint8_t fastRamConf;
    
    // Base address of the Fast Ram (value is provided by Kickstart).
    uint32_t fastRamBaseAddr;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    ZorroManager();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    size_t _load(uint8_t *buffer) override { return 0; }
    size_t _save(uint8_t *buffer) override { return 0; }

    
public:
    
    //
    // Getter and setter
    //
    
    
    //
    // Emulating Fast Ram
    //
    
    uint8_t peekFastRamDevice(uint32_t addr);
    void pokeFastRamDevice(uint32_t addr, uint8_t value);
};

#endif
