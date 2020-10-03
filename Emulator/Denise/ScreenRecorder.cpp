// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"
#include <fcntl.h>

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
                               long aspectX,
                               long aspectY)
{
    // Only proceed if the screen recorder is available
    if (!isReady()) return false;

    // Only proceed if the recorder doesn't run
    if (isRecording()) return false;
    
    synchronized {

        // Make sure the screen dimensions are even
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

        // Tell FFmpeg to read from a pipe
        ptr += sprintf(ptr, " -i %s", videoPipePath);

        //
        // Output stream parameters
        //

        // Format of the output stream
        // cmd += sprintf(cmd, " -f mp4 -vcodec libx264 -pix_fmt yuv420p");
        ptr += sprintf(ptr, " -f mp4 -pix_fmt yuv420p");

        // Bit rate
        ptr += sprintf(ptr, " -b:v %ldk", bitRate);

        // Aspect ratio
        ptr += sprintf(ptr, " -bsf:v ");
        ptr += sprintf(ptr, "\"h264_metadata=sample_aspect_ratio=");
        ptr += sprintf(ptr, "%ld/%ld\"", aspectX, 2*aspectY);

        // Overwrite output files without asking
        ptr += sprintf(ptr, " -y");
        
        // Output file
        ptr += sprintf(ptr, " %s", outfile);

        //
        // EXPERIMENTAL (AUDIO)
        //

        /*
        sprintf(cmd, "%s -y -f f32le -sample_rate %d -channels 2 -i %s %s",
                ffmpegPath, sampleRate, audioPipePath, outfile);
        */
        
        //
        // Launch FFmpeg
        //
        
        if (createsPipes() && startFFmpeg(cmd) && openPipes()) {
            msg("Success\n");
        } else {
            closePipes();
            stopFFmpeg();
            msg("Failed to launch FFmpeg\n");
        }
    }
    
    if (isRecording()) {
        messageQueue.put(MSG_RECORDING_STARTED);
        return true;
    }
    
    return false;
}

void
ScreenRecorder::stopRecording()
{
    if (!isRecording()) return;
    
    stopFFmpeg();
    messageQueue.put(MSG_RECORDING_STOPPED);
}

bool
ScreenRecorder::createsPipes()
{
    // Remove old pipes if they still exist
    unlink(audioPipePath);
    unlink(videoPipePath);
    
    // Create pipes
    if (mkfifo(videoPipePath, 0666) != -1) {
        if (mkfifo(audioPipePath, 0666) != -1) {
            msg("Input pipes successfully created\n");
            return true;
        }
    }
    
    // In case of an error, clean up the mess
    warn("Failed to create input pipes\n");
    unlink(audioPipePath);
    unlink(videoPipePath);
    return false;
}

bool
ScreenRecorder::openPipes()
{
    videoPipe = open(videoPipePath, O_WRONLY);
    // audioPipe = open(audioPipePath, O_WRONLY);
    
    if (videoPipe != -1) { // } && audioPipe != -1) {
        msg("Input pipes are open\n");
    } else {
        msg("Failed to open input pipes\n");
        closePipes();
    }

    return videoPipe != -1; // && audioPipe != -1;
}

void
ScreenRecorder::closePipes()
{
    if (videoPipe != 1) {
        close(videoPipe);
        videoPipe = -1;
        msg("Video pipe closed\n");
    }
    if (audioPipe != 1) {
        close(audioPipe);
        audioPipe = -1;
        msg("Audio pipe closed\n");
    }
}

bool
ScreenRecorder::startFFmpeg(const char *cmd)
{
    if (!ffmpeg) {
        
        msg("Starting FFmpeg with options:\n%s", cmd);
        ffmpeg = popen(cmd, "w");
    }

    return ffmpeg != NULL;
}

void
ScreenRecorder::stopFFmpeg()
{
    if (ffmpeg) {
        
        msg("Stopping FFmpeg\n");
        closePipes();
        pclose(ffmpeg);
        ffmpeg = NULL;
    }
}

void
ScreenRecorder::addSample(float left, float right)
{
    if (samplesCnt < sizeof(samples) / (2 * sizeof(float))) {
        samples[samplesCnt][0] = left * 4;
        samples[samplesCnt][1] = right * 4;
        samplesCnt++;
    }
}

void
ScreenRecorder::vsyncHandler()
{
    if (!isRecording()) return;
    assert(ffmpeg != NULL);

    synchronized {
        
        //
        // Video
        //
        
        ScreenBuffer buffer = denise.pixelEngine.getStableBuffer();
        
        int width = cutout.x2 - cutout.x1;
        int height = cutout.y2 - cutout.y1;
        int offset = cutout.x1 + HBLANK_MIN * 4;
        
        /* Experimental code. The pixels of the texture rect are first written
         * to a temporary pixel buffer. Afterwards, the buffer is handed over
         * to FFmpeg with a single write command. Another approach would be
         * to use a single write for each rasterline contained in the texture
         * rect. TODO: Figure out which variant is faster
         */
        for (int y = 0, i = 0; y < height; y++) {
            for (int x = 0; x < width; x++, i++) {
                pixels[i] = buffer.data[(cutout.y1 + y) * HPIXELS + x + offset];
            }
        }
        
        write(videoPipe, pixels, sizeof(u32) * width * height);
        // fwrite(pixels, sizeof(u32), width * height, ffmpeg);
        
        //
        // Audio
        //
        
        /*
        if (samplesCnt != sampleRate / 50) {
            debug("Got %d audio samples, expected %d (%f)\n",
                   samplesCnt, sampleRate / 50, audioUnit.getSampleRate());
        }
        write(audioPipe, samples, 2 * sizeof(float) * (sampleRate / 50));
        samplesCnt = 0;
        */
    }
}

const char *ScreenRecorder::ffmpegPath = "/usr/local/bin/ffmpeg";
const char *ScreenRecorder::audioPipePath = "/tmp/audioPipe";
const char *ScreenRecorder::videoPipePath = "/tmp/videoPipe";
bool ScreenRecorder::ffmpegInstalled = false;
