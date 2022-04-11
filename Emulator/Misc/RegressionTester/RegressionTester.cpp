// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RegressionTester.h"
#include "Amiga.h"
#include "IOUtils.h"

#include <fstream>

void
RegressionTester::prepare(ConfigScheme scheme, string rom, string ext)
{
    // Only proceed if the /tmp folder exisits
    if (!util::fileExists("/tmp")) throw VAError(ERROR_DIR_NOT_FOUND, "/tmp");

    // Check if we've got write permissions
    if (amiga.tmp() != "/tmp") throw VAError(ERROR_DIR_ACCESS_DENIED, "/tmp");
    
    // Initialize the emulator according to the specified scheme
    amiga.revertToFactorySettings();
    amiga.configure(scheme);

    // Load Kickstart Rom
    amiga.mem.loadRom(rom.c_str());
    
    // Load Extension Rom (if provided)
    if (ext != "") amiga.mem.loadExt(ext.c_str());
    
    // Choose a warp source that prevents the GUI from disabling warp mode
    constexpr isize warpSource = 1;
    
    // Run as fast as possible
    amiga.warpOn(warpSource);    
}

void
RegressionTester::run(string adf)
{
    // Insert the test disk
    df0.swapDisk(adf);
 
    // Run the emulator
    amiga.powerOn();
    amiga.run();
}

void
RegressionTester::dumpTexture(Amiga &amiga)
{
    dumpTexture(amiga, dumpTexturePath);
}

void
RegressionTester::dumpTexture(Amiga &amiga, const string &filename)
{
    /* This function is used for automatic regression testing. It generates a
     * TIFF image of the current emulator texture in the /tmp directory and
     * exits the application. The regression testing script will pick up the
     * texture and compare it against a previously recorded reference image.
     */
    std::ofstream file;
        
    // Assemble the target file names
    string rawFile = "/tmp/" + filename + ".raw";
    string tiffFile = "/tmp/" + filename + ".tiff";

    // Open an output stream
    file.open(rawFile.c_str());
    
    // Dump texture
    dumpTexture(amiga, file);
    file.close();
    
    // Convert raw data into a TIFF file
    string cmd = "/usr/local/bin/raw2tiff";
    cmd += " -p rgb -b 3";
    cmd += " -w " + std::to_string(x2 - x1);
    cmd += " -l " + std::to_string(y2 - y1);
    cmd += " " + rawFile + " " + tiffFile;
    
    if (system(cmd.c_str()) == -1) {
        warn("Error executing %s\n", cmd.c_str());
    }
    
    // Ask the GUI to quit
    msgQueue.put(MSG_ABORT, retValue);
}

void
RegressionTester::dumpTexture(Amiga &amiga, std::ostream& os)
{
    {   SUSPENDED
        
        auto &buffer = amiga.denise.pixelEngine.getStableBuffer();
        
        for (isize y = y1; y < y2; y++) {
            
            for (isize x = x1; x < x2; x++) {
                
                char *cptr = (char *)(buffer.ptr + y * HPIXELS + x);
                os.write(cptr + 0, 1);
                os.write(cptr + 1, 1);
                os.write(cptr + 2, 1);
            }
        }
    }
}

void
RegressionTester::setErrorCode(u8 value)
{
    retValue = value;
}
