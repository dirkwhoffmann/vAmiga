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
ScreenRecorder::setPath(const char *path)
{
    if (path == NULL) return false;
    
    // Check if we have write permissions for this file
    FILE *file = fopen(path, "w");
    
    if (file) {
        
        plaindebug("New screen capture target: %s\n", path);
        fclose(file);
        if (outfile) delete(outfile);
        outfile = strdup(path);
        return true;
    }

    warn("Failed to open file %s\n", path);
    return false;
}

bool
ScreenRecorder::isReady()
{
    return ffmpegInstalled; 
}

bool
ScreenRecorder::isRecording()
{
    bool result = false;
    synchronized { result = ffmpeg != NULL; }
    return result;
}
    
bool
ScreenRecorder::startRecording(int x1, int y1, int x2, int y2,
                               long bitRate,
                               long videoCodec,
                               long audioCodec)
{
    // Only proceed if the recorder doesn't running
    if (isRecording()) return false;
    
    // Only proceed if the screen recorder is configured
    if (!isReady()) return false;
    
    int aspectX = 117;
    int aspectY = 256;

    synchronized {

        // Make sure the width and height of the texture cutout are even numbers
        if ((x2 - x1) % 2) x2--;
        if ((y2 - y1) % 2) y2--;

        cutout.x1 = x1;
        cutout.x2 = x2;
        cutout.y1 = y1;
        cutout.y2 = y2;
                
        plaindebug("Recorded area: (%d,%d) - (%d,%d)\n", x1, y1, x2, y2);
        
        // Assemble the command line arguments for FFmpeg
        char cmd[256]; char *ptr = cmd;

        // Path to the FFmpeg executable
        ptr += sprintf(ptr, " %s", ffmpegPath);

        //
        // Input stream parameters
        //

        // Format of the input stream
        ptr += sprintf(ptr, " -f rawvideo -pixel_format rgba");
        
        // Frame size (width x height)
        ptr += sprintf(ptr, " -s %dx%d", x2 - x1, y2 - y1);
        
        // Frame rate
        ptr += sprintf(ptr, " -r 50");

        // Tell FFmpeg to read from stdin
        ptr += sprintf(ptr, " -i -");

        //
        // Output stream parameters
        //

        // Format of the output stream
        // cmd += sprintf(cmd, " -f mp4 -vcodec libx264 -pix_fmt yuv420p");
        ptr += sprintf(ptr, " -f mp4 -pix_fmt yuv420p");

        // Bit rate
        ptr += sprintf(ptr, " -b:v %ld", bitRate);

        // Aspect ratio
        ptr += sprintf(ptr, " -bsf:v ");
        ptr += sprintf(ptr, "\"h264_metadata=sample_aspect_ratio=");
        ptr += sprintf(ptr, "%d/%d\"", aspectX, aspectY);

        // Overwrite output files without asking
        ptr += sprintf(ptr, " -y");
        
        // Output file
        ptr += sprintf(ptr, " %s", outfile);
        msg("%s\n", cmd);

        // Launch FFmpeg
        ffmpeg = popen(cmd, "w");
    }
    
    if (ffmpeg) {
        messageQueue.put(MSG_RECORDING_STARTED);
        return true;
    } else {
        msg("Failed to launch FFmpeg\n");
        return false;
    }
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
