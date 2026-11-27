// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "RegressionTester.h"
#include "Emulator.h"
#include "utl/io.h"
#include <fstream>

namespace vamiga {

void
RegressionTester::prepare(RegressionScheme scheme, const fs::path &rom, const fs::path &ext)
{
    // Only proceed if the /tmp folder exisits
    if (!utl::fileExists("/tmp")) throw IOError(IOError::DIR_NOT_FOUND, "/tmp");

    // Check if we've got write permissions
    if (host.tmp() != "/tmp") throw IOError(IOError::DIR_ACCESS_DENIED, "/tmp");

    // Initialize the emulator according to the specified scheme
    emulator.powerOff();
    apply(scheme);

    // Load Kickstart Rom
    if (rom != "") amiga.mem.loadRom(rom);
    
    // Load Extension Rom (if provided)
    if (ext != "") amiga.mem.loadExt(ext);
    
    // Choose a color palette that stays stable across releases
    emulator.set(Opt::MON_PALETTE, (i64)Palette::COLOR);
    
    // Choose a warp source that prevents the GUI from disabling warp mode
    constexpr isize warpSource = 1;
    
    // Run as fast as possible
    emulator.warpOn(warpSource);
}

void
RegressionTester::apply(RegressionScheme scheme)
{
    RegressionSchemeEnum::validate(scheme);

    emulator.revertToDefaultConfig();

    switch(scheme) {
            
        case RegressionScheme::A1000_OCS_1MB:
            
            amiga.set(Opt::CPU_REVISION, (i64)CPURev::CPU_68000);
            amiga.set(Opt::CPU_OVERCLOCKING, 0);
            amiga.set(Opt::AGNUS_REVISION, (i64)AgnusRevision::OCS_OLD);
            amiga.set(Opt::DENISE_REVISION, (i64)DeniseRev::OCS);
            amiga.set(Opt::AMIGA_VIDEO_FORMAT, (i64)TV::PAL);
            amiga.set(Opt::MEM_BUS_WIDTH, 16);
            amiga.set(Opt::MEM_CHIP_RAM, 512);
            amiga.set(Opt::MEM_SLOW_RAM, 512);
            amiga.set(Opt::MEM_BANKMAP, (i64)BankMap::A1000);
            break;
            
        case RegressionScheme::A500_OCS_1MB:
            
            amiga.set(Opt::CPU_REVISION, (i64)CPURev::CPU_68000);
            amiga.set(Opt::CPU_OVERCLOCKING, 0);
            amiga.set(Opt::AGNUS_REVISION, (i64)AgnusRevision::OCS);
            amiga.set(Opt::DENISE_REVISION, (i64)DeniseRev::OCS);
            amiga.set(Opt::AMIGA_VIDEO_FORMAT, (i64)TV::PAL);
            amiga.set(Opt::MEM_BUS_WIDTH, 16);
            amiga.set(Opt::MEM_CHIP_RAM, 512);
            amiga.set(Opt::MEM_SLOW_RAM, 512);
            amiga.set(Opt::MEM_BANKMAP, (i64)BankMap::A500);
            break;
            
        case RegressionScheme::A500_ECS_1MB:
            
            amiga.set(Opt::CPU_REVISION, (i64)CPURev::CPU_68000);
            amiga.set(Opt::CPU_OVERCLOCKING, 0);
            amiga.set(Opt::AGNUS_REVISION, (i64)AgnusRevision::ECS_1MB);
            amiga.set(Opt::DENISE_REVISION, (i64)DeniseRev::OCS);
            amiga.set(Opt::AMIGA_VIDEO_FORMAT, (i64)TV::PAL);
            amiga.set(Opt::MEM_BUS_WIDTH, 16);
            amiga.set(Opt::MEM_CHIP_RAM, 512);
            amiga.set(Opt::MEM_SLOW_RAM, 512);
            amiga.set(Opt::MEM_BANKMAP, (i64)BankMap::A500);
            break;
            
        case RegressionScheme::A500_PLUS_1MB:
            
            amiga.set(Opt::CPU_REVISION, (i64)CPURev::CPU_68000);
            amiga.set(Opt::CPU_OVERCLOCKING, 0);
            amiga.set(Opt::AGNUS_REVISION, (i64)AgnusRevision::ECS_2MB);
            amiga.set(Opt::DENISE_REVISION, (i64)DeniseRev::ECS);
            amiga.set(Opt::AMIGA_VIDEO_FORMAT, (i64)TV::PAL);
            amiga.set(Opt::MEM_BUS_WIDTH, 16);
            amiga.set(Opt::MEM_CHIP_RAM, 512);
            amiga.set(Opt::MEM_SLOW_RAM, 512);
            amiga.set(Opt::MEM_BANKMAP, (i64)BankMap::A500);
            break;
 
        case RegressionScheme::A1200_2MB:
            
            amiga.set(Opt::CPU_REVISION, (i64)CPURev::CPU_68EC020);
            amiga.set(Opt::CPU_OVERCLOCKING, 2);
            amiga.set(Opt::AGNUS_REVISION, (i64)AgnusRevision::AGA);
            amiga.set(Opt::DENISE_REVISION, (i64)DeniseRev::AGA);
            amiga.set(Opt::AMIGA_VIDEO_FORMAT, (i64)TV::PAL);
            amiga.set(Opt::MEM_BUS_WIDTH, 32);
            amiga.set(Opt::MEM_CHIP_RAM, 2048);
            amiga.set(Opt::MEM_SLOW_RAM, 0);
            amiga.set(Opt::MEM_BANKMAP, (i64)BankMap::A1200);
            break;
            
        default:
            fatalError;
    }

    /* Pin the peripherals the reference images were recorded with. They used
     * to be the default settings, which have changed in the meantime.
     */
    amiga.set(Opt::DRIVE_CONNECT, true, { 0 });
    amiga.set(Opt::DRIVE_CONNECT, false, { 1, 2, 3 });
    amiga.set(Opt::HDC_CONNECT, true, { 0 });
    amiga.set(Opt::HDC_CONNECT, false, { 1, 2, 3 });
}

void
RegressionTester::run(const fs::path &adf)
{
    // Insert the test disk
    df0.swapDisk(adf);

    // Run the emulator
    emulator.powerOn();
    emulator.run();
}

void
RegressionTester::dumpTexture(Amiga &amiga)
{
    dumpTexture(amiga, dumpTexturePath);
}

void
RegressionTester::dumpTexture(Amiga &amiga, const fs::path &path)
{
    /* This function is used for automatic regression testing. It dumps the
     * visible portion of the texture into the /tmp directory and exits the
     * application. The regression test script picks up the texture and
     * compares it against a previously recorded reference image.
     */
    std::ofstream file;

    // Open an output stream
    file.open((fs::path("/tmp") / path).replace_extension(".raw"));
        
    // Dump texture
    dumpTexture(amiga, file);
    file.close();

    // Ask the GUI to quit
    msgQueue.put(Msg::ABORT, retValue);
}

void
RegressionTester::dumpTexture(Amiga &amiga, std::ostream &os)
{
    Texel grey2 = Texture::grey2;
    Texel grey4 = Texture::grey4;

    auto checkerboard = [&](isize y, isize x) {
        return ((y >> 3) & 1) == ((x >> 3) & 1) ? (char *)&grey2 : (char *)&grey4;
    };
    
    Texel *ptr = amiga.denise.pixelEngine.stablePtr() - 4 * HBLANK_MIN;
    char *cptr;
    
    for (isize y = Y1; y < Y2; y++) {
        
        for (isize x = X1; x < X2; x++) {
            
            if (y >= y1 && y < y2 && x >= x1 && x < x2) {
                cptr = (char *)(ptr + y * HPIXELS + x);
            } else {
                cptr = checkerboard(y, x);
            }
            
            os.write(cptr + 0, 1);
            os.write(cptr + 1, 1);
            os.write(cptr + 2, 1);
        }
    }
}

void
RegressionTester::setErrorCode(u8 value)
{
    retValue = value;
}

}
