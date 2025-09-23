// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
extension MyController {

    func installAros() {

        installAros(crc32: vamiga.CRC32_AROS_20250219)
    }

    func installAros(crc32: UInt32) {

        switch crc32 {

        case vamiga.CRC32_AROS_54705:       // Taken from UAE
            installAros(rom: "aros-svn54705-rom", ext: "aros-svn54705-ext")

        case vamiga.CRC32_AROS_55696:       // Taken from SAE
            installAros(rom: "aros-svn55696-rom", ext: "aros-svn55696-ext")

        case vamiga.CRC32_AROS_20250219:    // 2025 version
            installAros(rom: "aros-20250219-rom", ext: "aros-20250219-ext")

        default:
            fatalError()
        }
    }

    func installAros(rom: String, ext: String) {

        guard let config = config, let emu = emu else { return }

        // Install both Roms
        install(rom: rom)
        install(ext: ext)

        // Configure the location of the exansion Rom
        config.extStart = 0xE0

        // Make sure the machine has enough Ram to run Aros
        let chip = emu.get(.MEM_CHIP_RAM)
        let slow = emu.get(.MEM_SLOW_RAM)
        let fast = emu.get(.MEM_FAST_RAM)
        if chip + slow + fast < 1024*1024 { config.slowRam = 512 }
    }

    func install(rom: String) {

        let data = NSDataAsset(name: rom)!.data
        try? emu?.mem.loadRom(buffer: data)
    }

    func install(ext: String) {

        let data = NSDataAsset(name: ext)!.data
        try? emu?.mem.loadExt(buffer: data)
    }
}
