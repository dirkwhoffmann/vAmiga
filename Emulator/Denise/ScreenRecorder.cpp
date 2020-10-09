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

/*
void
BufferedPipe::worker()
{
    // Open the pipe
    pipe = open(path, O_WRONLY);
    
    while (1) {
        
        // Check if the thread has been requested to terminate
        if (!running && fifo.empty()) break;
        
        // Wait until the FIFO has been filled with at least one element
        m.lock(); m.unlock();
        
        // Remove the oldest element
        DataChunk chunk;
        synchronized {
            chunk = fifo.front();
            fifo.pop();
            if (fifo.empty()) m.lock();
        }
        
        // Write data to the pipe
        write(pipe, chunk.data, chunk.size);
        delete(chunk.data);
    }
    
    // Close the pipe
    assert(running == false);
    close(pipe);
    plaindebug("Worker thread has stopped\n");
}
*/

Pipe *
Pipe::make(const char *path)
{
    assert(path != NULL);
    
    // Remove old pipe (if any)
    unlink(path);
    
    // Create a new pipe
    int id = mkfifo(path, 0666);
    if (id == -1) { return NULL; }
    
    // Create object
    Pipe *pipe = new Pipe();
    pipe->setDescription("BufferedPipe");
    pipe->path = path;
    
    return pipe;
}

void
Pipe::send(u8 *data, size_t size)
{
    // Open the pipe (blocking)
    if (pipe == -1) pipe = open(path, O_WRONLY);

    write(pipe, data, size);    
}

ScreenRecorder::ScreenRecorder(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("ScreenRecorder");

    subComponents = vector<HardwareComponent *> {
        
        &muxer
    };
}

bool
ScreenRecorder::hasFFmpeg()
{
    return getSizeOfFile(ffmpegPath()) > 0;
}

void
ScreenRecorder::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

void
ScreenRecorder::_dump()
{
    msg("%s:%s installed\n", ffmpegPath(), hasFFmpeg() ? "" : " not");
    msg("Video pipe:%s created\n", videoPipe ? "" : " not");
    msg("Audio pipe:%s created\n", videoPipe ? "" : " not");
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

/*
bool
ScreenRecorder::isReady()
{
    return ffmpegInstalled && videoPipe != NULL && audioPipe != NULL;
}
*/

bool
ScreenRecorder::isRecording()
{
    return recording;
}
    
bool
ScreenRecorder::startRecording(int x1, int y1, int x2, int y2,
                               long bitRate,
                               long aspectX,
                               long aspectY)
{
    videoPipe = Pipe::make("/tmp/videoPipe");
    audioPipe = Pipe::make("/tmp/audioPipe");

    debug("videoPipe = %p\n", videoPipe);
    debug("audioPipe = %p\n", audioPipe);

    if (isRecording()) return false;
        
    dump();
    
    synchronized {

        // Make sure the screen dimensions are even
        if ((x2 - x1) % 2) x2--;
        if ((y2 - y1) % 2) y2--;
        cutout.x1 = x1;
        cutout.x2 = x2;
        cutout.y1 = y1;
        cutout.y2 = y2;
        plaindebug("Recorded area: (%d,%d) - (%d,%d)\n", x1, y1, x2, y2);
          
        //
        // Assemble the command line arguments for the video encoder
        //
        
        char cmd1[512]; char *ptr = cmd1;

        // Path to the FFmpeg executable
        ptr += sprintf(ptr, "%s -nostdin", ffmpegPath());

        // Input stream format
        ptr += sprintf(ptr, " -f:v rawvideo -pixel_format rgba");
        
        // Frame rate
        ptr += sprintf(ptr, " -r %d", frameRate);

        // Frame size (width x height)
        ptr += sprintf(ptr, " -s:v %dx%d", x2 - x1, y2 - y1);
        
        // Input source (named pipe)
        ptr += sprintf(ptr, " -i %s", videoPipe->path);

        // Output stream format
        ptr += sprintf(ptr, " -f mp4 -pix_fmt yuv420p");

        // Bit rate
        ptr += sprintf(ptr, " -b:v %ldk", bitRate);

        // Aspect ratio
        ptr += sprintf(ptr, " -bsf:v ");
        ptr += sprintf(ptr, "\"h264_metadata=sample_aspect_ratio=");
        ptr += sprintf(ptr, "%ld/%ld\"", aspectX, 2*aspectY);
        
        // Output file
        ptr += sprintf(ptr, " -y %s", "/tmp/video.mp4");

        // ptr += sprintf(ptr, " < %s", videoPipe->path);

        
        //
        // Assemble the command line arguments for the audio encoder
        //
        
        char cmd2[512]; ptr = cmd2;

        // Path to the FFmpeg executable
        ptr += sprintf(ptr, "%s -nostdin", ffmpegPath());
        
        // Audio format and number of channels
        ptr += sprintf(ptr, " -f:a f32le -channels 2");

        // Sampling rate
        ptr += sprintf(ptr, " -sample_rate %d", sampleRate);

        // Input source (named pipe)
        ptr += sprintf(ptr, " -i %s", audioPipe->path);
        
        // Output stream format
        ptr += sprintf(ptr, " -f mp4");

        // Output file
        ptr += sprintf(ptr, " -y %s", "/tmp/audio.mp4");
        
        //
        // Launch FFmpeg instances
        //
            
        assert(videoFFmpeg == NULL);
        assert(audioFFmpeg == NULL);

        msg("\nStarting video encoder with options:\n%s\n", cmd1);
        videoFFmpeg = popen(cmd1, "w");
        msg(videoFFmpeg ? "Success\n" : "Failed to launch\n");

        msg("\nStarting audio encoder with options:\n%s\n", cmd1);
        audioFFmpeg = popen(cmd2, "w");
        msg(audioFFmpeg ? "Success\n" : "Failed to launch\n");

        recording = videoFFmpeg != NULL && audioFFmpeg != NULL;
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
    plaindebug(REC_DEBUG, "stopRecording()\n");
    
    if (!isRecording()) return;
    
    synchronized { recording = false; }

    // Close pipes
    plaindebug(REC_DEBUG, "Stopping video pipe...\n");
    videoPipe->cancel();
    plaindebug(REC_DEBUG, "Stopping audio pipe...\n");
    audioPipe->cancel();

     
    // Shut down encoders
    plaindebug(REC_DEBUG, "Shutting down encoders...\n");
    pclose(videoFFmpeg);
    plaindebug(REC_DEBUG, "Video encoder shut down\n");
    pclose(audioFFmpeg);
    plaindebug(REC_DEBUG, "Audio encoder shut down\n");
    videoFFmpeg = NULL;
    audioFFmpeg = NULL;

    plaindebug(REC_DEBUG, "Recording has stopped\n");
    messageQueue.put(MSG_RECORDING_STOPPED);
}

void
ScreenRecorder::vsyncHandler(Cycle target)
{
    if (!isRecording()) return;
    
    // debug("vsyncHandler\n");
    assert(videoFFmpeg != NULL);
    assert(audioFFmpeg != NULL);

    synchronized {
        
        //
        // Video
        //
                        
        ScreenBuffer buffer = denise.pixelEngine.getStableBuffer();
        
        int width = sizeof(u32) * (cutout.x2 - cutout.x1);
        int height = cutout.y2 - cutout.y1;
        int offset = cutout.y1 * HPIXELS + cutout.x1 + HBLANK_MIN * 4;
        u8 *data = new u8[width * height];
        u8 *src = (u8 *)(buffer.data + offset);
        u8 *dst = data;
        for (int y = 0; y < height; y++, src += 4 * HPIXELS, dst += width) {
            memcpy(dst, src, width);
        }

        videoPipe->send(data, (size_t)(width * height));
        
        //
        // Audio
        //
        
        // Clone Paula's muxer contents
        muxer.sampler[0] = paula.muxer.sampler[0];
        muxer.sampler[1] = paula.muxer.sampler[1];
        muxer.sampler[2] = paula.muxer.sampler[2];
        muxer.sampler[3] = paula.muxer.sampler[3];
        assert(muxer.sampler[0].r == paula.muxer.sampler[0].r);
        assert(muxer.sampler[0].w == paula.muxer.sampler[0].w);

        // Synthesize audio samples for this frame
        if (audioClock == 0) audioClock = target-1;
        muxer.synthesize(audioClock, target, samplesPerFrame);
        audioClock = target;
        
        // Copy samples to buffer
        float *samples = new float[2 * samplesPerFrame];
        muxer.copyInterleaved(samples, samplesPerFrame);
                
        // Feed buffer contents into the audio pipe
        audioPipe->send((u8 *)samples, (size_t)(2 * sizeof(float) * samplesPerFrame));
    }
}
