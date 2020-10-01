// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

ScreenRecorder::ScreenRecorder(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("ScreenRecorder");
    
    // Check if FFmpeg is installed on this machine
    ffmpegInstalled = getSizeOfFile("/usr/local/ffmpeg") > 0;

    msg("FFmpeg: %s\n", ffmpegInstalled ? "installed" : "not installed");
}

void
ScreenRecorder::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

int
ScreenRecorder::startRecording(int newX1, int newY1, int newX2, int newY2)
{
    const char *exec = "/usr/local/bin/ffmpeg";
    char cmd[256];
    
    // Only proceed if the screen is not currently recorded
    if (isRecording()) return 0;

    x1 = newX1;
    x2 = newX2;
    y1 = newY1;
    y2 = newY2;
    
    // REMOVE ASAP
    x1 = 0;
    x2 = 800;
    y1 = 0;
    y2 = 600;
    
    plaindebug("Recorded area: (%d,%d) - (%d,%d)\n", x1, y1, x2, y2);
    
    // Check if the output file can be written
    // TODO
    
    // Assemble the command line arguments for FFmpeg
    sprintf(cmd,
            " %s"             // Path to the FFmpeg executable
            " -y"
            " -f %s"          // Input file format
            " -pix_fmt %s"    // Pixel format
            " -s %dx%d"       // Width and height
            " -r %d"          // Frames per second
            " -i -"           // Read from stdin
            " -profile:v %s"
            " -level:v %d"    // Log verbosity level
            " -b:v %s"        // Bitrate
            " -an %s",        // Output file
            
            exec,
            "rawvideo",
            "rgba",
            x2 - x1,
            y2 - y1,
            50,
            "high444",
            3,
            "64k",           // TODO: READ FROM config
            "/tmp/amiga.mp4"); // TODO: READ FROM config
            
    // Launch FFmpeg
    msg("Executing %s\n", cmd);
    if (!(ffmpeg = popen(cmd, "w"))) {
        msg("Failed to launch FFmpeg\n");
        return 3;
    }
    
    messageQueue.put(MSG_RECORDING_STARTED);

    return 0;
}

void
ScreenRecorder::stopRecording()
{
    // Only proceed if the screen is currently recorded
    if (!isRecording()) return;
        
    // Shut down the screen recorder
    pclose(ffmpeg);
    ffmpeg = NULL;
    
    messageQueue.put(MSG_RECORDING_STOPPED);
}

void
ScreenRecorder::vsyncHandler()
{
    if (!isRecording()) return;
    assert(ffmpeg != NULL);
    
    static int frameCounter = 0;
    ScreenBuffer buffer = denise.pixelEngine.getStableBuffer();
    
    // Experimental
    for (int y = 0; y < 600; y++) {
        for (int x = 0; x < 800; x++) {
            pixels[y][x] = buffer.data[y * HPIXELS + x];
        }
    }
    
    fwrite(pixels, sizeof(u32), 800*600, ffmpeg);
    frameCounter++;
    
    if (frameCounter == 1000) {
        plaindebug("Recording finished\n");
        pclose(ffmpeg);
        ffmpeg = NULL;
    }
}
