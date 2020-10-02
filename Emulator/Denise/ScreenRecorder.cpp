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
    ffmpegInstalled = getSizeOfFile(ffmpegPath) > 0;

    msg("%s:%s installed\n", ffmpegPath, ffmpegInstalled ? "" : " not");
}

void
ScreenRecorder::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

bool
ScreenRecorder::isRecording()
{
    bool result = false;
    synchronized { result = ffmpeg != NULL; }
    return result;
}
    
int
ScreenRecorder::startRecording(int x1, int y1, int x2, int y2,
                               long bitRate,
                               long videoCodec,
                               long audioCodec)
{
    if (isRecording()) return 0;
    int error = 0;

    synchronized {

        char cmd[256];

        cutout.x1 = x1;
        cutout.x2 = x2;
        cutout.y1 = y1;
        cutout.y2 = y2;
                
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
                " -b:v %ldk"       // Bitrate
                // " -aspect 256:117"
                " -bsf:v \"h264_metadata=sample_aspect_ratio=117/256\""
                " -an %s",        // Output file
                
                ffmpegPath,
                "rawvideo",
                "rgba",
                x2 - x1,
                y2 - y1,
                50,
                "high444",
                3,
                bitRate,
                "/tmp/amiga.mp4"); // TODO: READ FROM config
        
        // Launch FFmpeg
        msg("Executing %s\n", cmd);
        if (!(ffmpeg = popen(cmd, "w"))) {
            error = 1;
        }
    }
    
    if (error == 0) {
        messageQueue.put(MSG_RECORDING_STARTED);
    } else {
        msg("Failed to launch FFmpeg\n");
    }
    
    return error;
}

void
ScreenRecorder::stopRecording()
{
    if (!isRecording()) return;

    synchronized {
        
        pclose(ffmpeg);
        ffmpeg = NULL;
    }
    
    messageQueue.put(MSG_RECORDING_STOPPED);
}

void
ScreenRecorder::vsyncHandler()
{
    if (!isRecording()) return;
    assert(ffmpeg != NULL);

    synchronized {
                
        static int frameCounter = 0;
        ScreenBuffer buffer = denise.pixelEngine.getStableBuffer();
        
        int width = cutout.x2 - cutout.x1;
        int height = cutout.y2 - cutout.y1;
        int offset = cutout.x1 + HBLANK_MIN * 4;
        
        // Experimental
        for (int y = 0, i = 0; y < height; y++) {
            for (int x = 0; x < width; x++, i++) {
                pixels[i] = buffer.data[(cutout.y1 + y) * HPIXELS + x + offset];
            }
        }
        
        fwrite(pixels, sizeof(u32), width * height, ffmpeg);
        frameCounter++;
        
        if (frameCounter == 1000) {
            plaindebug("Recording finished\n");
            pclose(ffmpeg);
            ffmpeg = NULL;
        }
    }
}

const char *ScreenRecorder::ffmpegPath = "/usr/local/bin/ffmpeg";
bool ScreenRecorder::ffmpegInstalled = false;
