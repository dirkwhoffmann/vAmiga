// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "RegressionTester.h"
#include "Emulator.h"
#include "IOUtils.h"

#include <fstream>

namespace vamiga {

void
RegressionTester::prepare(ConfigScheme scheme, string rom, string ext)
{
    // Only proceed if the /tmp folder exisits
    if (!util::fileExists("/tmp")) throw CoreError(Fault::DIR_NOT_FOUND, "/tmp");

    // Check if we've got write permissions
    if (host.tmp() != "/tmp") throw CoreError(Fault::DIR_ACCESS_DENIED, "/tmp");
    
    // Initialize the emulator according to the specified scheme
    emulator.powerOff();
    emulator.set(scheme);

    // Load Kickstart Rom
    if (rom != "") amiga.mem.loadRom(rom.c_str());
    
    // Load Extension Rom (if provided)
    if (ext != "") amiga.mem.loadExt(ext.c_str());
    
    // Choose a color palette that stays stable across releases
    emulator.set(Opt::MON_PALETTE, (i64)Palette::RGB);
    
    // Choose a warp source that prevents the GUI from disabling warp mode
    constexpr isize warpSource = 1;
    
    // Run as fast as possible
    emulator.warpOn(warpSource);
}

void
RegressionTester::run(string adf)
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
RegressionTester::dumpTexture(Amiga &amiga, const string &filename)
{
    /* This function is used for automatic regression testing. It dumps the
     * visible portion of the texture into the /tmp directory and exits the
     * application. The regression test script picks up the texture and
     * compares it against a previously recorded reference image.
     */
    std::ofstream file;

    // Open an output stream
    file.open(("/tmp/" + filename + ".raw").c_str());
    
    // Dump texture
    dumpTexture(amiga, file);
    file.close();

    // Ask the GUI to quit
    msgQueue.put(Msg::ABORT, retValue);
}

void
RegressionTester::dumpTexture(Amiga &amiga, std::ostream& os)
{
    Texel grey2 = FrameBuffer::grey2;
    Texel grey4 = FrameBuffer::grey4;

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
