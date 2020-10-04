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

BufferedPipe *
BufferedPipe::make(const char *path)
{
    assert(path != NULL);
    
    // Remove old pipe (if any)
    unlink(path);
    
    // Create a new pipe
    if (mkfifo(path, 0666) == -1) { return NULL; }
    
    // Create object
    BufferedPipe *pipe = new BufferedPipe();
    pipe->setDescription("BufferedPipe");
    pipe->path = path;

    return pipe;
}

void
BufferedPipe::send(u8 *data, size_t size)
{
    startWorker();
    
    // Push the data packet into the FIFO buffer
    synchronized {
        fifo.push(DataChunk { data, size });
        m.unlock();
    }
}

void
BufferedPipe::startWorker()
{
    synchronized {
        
        if (!running) {
            running = true;
            m.lock();
            t = std::thread(&BufferedPipe::worker, this);
            plaindebug("Worker thread started");
        }
    }
}

void
BufferedPipe::stopWorker()
{
    plaindebug("Stopping worker thread...\n");
    running = false;
}

ScreenRecorder::ScreenRecorder(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("ScreenRecorder");
    
    // Check if FFmpeg is installed on this machine
    ffmpegInstalled = getSizeOfFile(ffmpegPath) > 0;

    msg("%s:%s installed\n", ffmpegPath, ffmpegInstalled ? "" : " not");
    msg("Video pipe:%s created\n", videoPipe ? "" : " not");
    msg("Audio pipe:%s created\n", videoPipe ? "" : " not");
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
    return ffmpegInstalled && videoPipe != NULL && audioPipe != NULL;
}

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
    if (!isReady() || isRecording()) return false;
        
    synchronized {

        recording = true;

        // Make sure the screen dimensions are even
        if ((x2 - x1) % 2) x2--;
        if ((y2 - y1) % 2) y2--;
        cutout.x1 = x1;
        cutout.x2 = x2;
        cutout.y1 = y1;
        cutout.y2 = y2;
        plaindebug("Recorded area: (%d,%d) - (%d,%d)\n", x1, y1, x2, y2);
                
        // Assemble the command line arguments for FFmpeg
        char cmd[512]; char *ptr = cmd;

        // Path to the FFmpeg executable
        ptr += sprintf(ptr, "%s", ffmpegPath);

        //
        // Video input stream settings
        //

        // Format of the input stream
        ptr += sprintf(ptr, " -f rawvideo -pixel_format rgba");
        
        // Frame size (width x height)
        ptr += sprintf(ptr, " -s %dx%d", x2 - x1, y2 - y1);
        
        // Frame rate
        ptr += sprintf(ptr, " -r 50");

        // Video input source (named pipe)
        ptr += sprintf(ptr, " -i %s", videoPipe->path);

        //
        // Audio input stream settings
        //
        
        // Audio format and number of channels
        ptr += sprintf(ptr, " -f f32le -channels 2");

        // Sampling rate
        ptr += sprintf(ptr, " -sample_rate %d", sampleRate);

        // Audio input source (named pipe)
        ptr += sprintf(ptr, " -i %s", audioPipe->path);
        
        //
        // Output stream settings
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
        // Launch FFmpeg
        //
            
        msg("\nStarting FFmpeg with options:\n%s\n", cmd);

        assert(ffmpeg == NULL);
        ffmpeg = popen(cmd, "w");
        
        msg(ffmpeg ? "Success\n" : "Failed to launch\n");
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
    if (!isReady() || !isRecording()) return;
    
    recording = false;

    // Ask both buffered pipes to terminate
    plaindebug("Stopping pipes..\n");
    videoPipe->cancel();
    audioPipe->cancel();
    
    // Wait until both pipes have terminated
    plaindebug("Wating for pipes to stop...\n");
    videoPipe->join();
    audioPipe->join();
    
    // Shut down FFmpeg
    pclose(ffmpeg);
    ffmpeg = NULL;

    plaindebug("Recording has stopped\n");
    messageQueue.put(MSG_RECORDING_STOPPED);
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
        // Audio
        //
        
        int samplesPerFrame = sampleRate / 50;
        if (samplesCnt != samplesPerFrame) {
            // debug("Got %d audio samples, expected %d (%f)\n",
            //        samplesCnt, sampleRate / 50, audioUnit.getSampleRate());
        }
        size_t audioSize = (size_t)(2 * sizeof(float) * samplesPerFrame);
        u8 *audio = new u8[audioSize];
        memcpy(audio, (u8 *)samples, audioSize);
        // DataChunk chunk2 { audio, audioSize };
        audioPipe->send(audio, audioSize);
        samplesCnt = 0;

        
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
        // DataChunk chunk { data, (size_t)(width * height) };
        videoPipe->send(data, (size_t)(width * height));
    }
}

const char *ScreenRecorder::ffmpegPath = "/usr/local/bin/ffmpeg";
bool ScreenRecorder::ffmpegInstalled = false;
