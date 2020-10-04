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

BufferedPipe::BufferedPipe()
{
    debug("Creating thread...\n");
    m.lock();
    t = std::thread(&BufferedPipe::worker, this);
    debug("Done...\n");
}

BufferedPipe *
BufferedPipe::make(const char *path)
{
    assert(path != NULL);
    
    // Remove old pipe (if any)
    unlink(path);
    
    // Create pipe
    if (mkfifo(path, 0666) == -1) { return NULL; }
    
    // Create buffered pipe
    BufferedPipe *pipe = new BufferedPipe();
    pipe->setDescription("BufferedPipe");
    pipe->path = path;
    return pipe;
}

void
BufferedPipe::add(DataChunk chunk)
{
    synchronized {
        // plaindebug("Adding data chunk (%p, %ld bytes)\n", chunk.data, chunk.size);
        fifo.push(chunk);
        m.unlock();
    }
}

void
BufferedPipe::worker()
{
    DataChunk chunk;
    
    // Open the pipe
    assert(pipe == -1);
    plaindebug("Opening pipe...\n");
    pipe = open(path, O_WRONLY);
    assert(pipe != -1);
    plaindebug("Pipe has been opened\n");
    
    while (1) {
        
        // Wait until the FIFO has been filled with at least one element
        // plaindebug("Waiting at barrier...\n");
        m.lock();
        m.unlock();
        // plaindebug("Passed (%d elements pending)\n", fifo.size());
        assert(!fifo.empty());
        
        // Remove the oldest element
        synchronized {
            chunk = fifo.front();
            fifo.pop();
            if (fifo.empty()) m.lock();
        }
        
        // Write data to the pipe
        // debug("Writing data chunk (%p, %ld bytes)\n", chunk.data, chunk.size);
        write(pipe, chunk.data, chunk.size);
        delete(chunk.data);
    }
}

void
BufferedPipe::done()
{
    plaindebug("Closing pipe %s\n", path);
    close(pipe);
    pipe = -1;
}

//
// OLD
//

/*
OldBufferedPipe *
OldBufferedPipe::make(const char *path)
{
    assert(path != NULL);
    
    // Remove old pipe (if any)
    unlink(path);
    
    // Create pipe
    if (mkfifo(path, 0666) == -1) { return NULL; }
    
    // Create object
    OldBufferedPipe *pipe = new OldBufferedPipe();
    pipe->path = path;
    return pipe;
}

void
OldBufferedPipe::resize(long newCapacity)
{
    u8 *newBuffer = new u8[newCapacity];
    
    // Copy over the old contents if the buffer grows. Otherwise, clear it.
    if (newCapacity > capacity) {
        printf("Buffer grows from %ld to %ld\n", capacity, newCapacity);
        memcpy(newBuffer, buffer, used);
    } else {
        printf("Buffer shrinks from %ld to %ld\n", capacity, newCapacity);
        used = 0;
    }
    
    // Assign the new buffer and adjust the capacity
    delete[] buffer;
    buffer = newBuffer;
    capacity = newCapacity;
}
          
void
OldBufferedPipe::append(u8 *data, long size)
{
    // Resize the buffer if it is too small
    if (used + size > capacity) { resize(MAX(used + size, capacity * 2)); }

    // Copy the new data over
    for (long i = 0; i < size; i++) { buffer[used + i] = data[i]; }

    used += size;
    assert(used <= capacity);
}

void
OldBufferedPipe::flush()
{
    if (tryOpen()) {
        printf("Pipe is open\n");
        write(pipe, buffer, used);
        printf("Flushing %ld bytes to %s\n", used, path);
        used = 0;
    } else {
        printf("Can't open pipe %s yet\n", path);
    }
}

void
OldBufferedPipe::terminate()
{
    close(pipe);
    pipe = -1;
    printf("Closing pipe %s\n", path);
}

bool
OldBufferedPipe::tryOpen()
{
    // Only proceed if the pipe is not open already
    if (pipe != -1) return true;
    
    // Check if a reader is connected. We do this by opening the pipe in
    // non-blocking mode. If no reader is connected, -1 is returned. Note: If
    // we omit O_NONBLOCK here, open() would block if no reade is connected.
    if ((pipe = open(path, O_WRONLY|O_NONBLOCK)) == -1) return false;

    printf("(1)\n");
    // The pipe is now open in non-blocking mode which means that a call to
    // write() is also non-blocking. This is not what we want to have. In
    // order to let a call to write() block, we reopen the pipe.
    close(pipe);
    pipe = open(path, O_WRONLY);
    printf("(2)\n");

    return true;
}
*/

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
    if (!isReady() || isRecording()) return false;
    
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
        ptr += sprintf(ptr, " -i %s", videoPipe->getPath());

        //
        // Audio input stream settings
        //
        
        // Audio format and number of channels
        ptr += sprintf(ptr, " -f f32le -channels 2");

        // Sampling rate
        ptr += sprintf(ptr, " -sample_rate %d", sampleRate);

        // Audio input source (named pipe)
        ptr += sprintf(ptr, " -i %s", audioPipe->getPath());
        
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

int stop = 0;

void
ScreenRecorder::stopRecording()
{
    if (!isReady() || !isRecording()) return;
    
    stop = 1;
    msg("Stopping FFmpeg\n");
    videoPipe->done();
    audioPipe->done();
    pclose(ffmpeg);
    ffmpeg = NULL;
    
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
    if(stop) return;
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
        DataChunk chunk2 { audio, audioSize };
        audioPipe->add(chunk2);
        // audioPipe->append((u8 *)samples, 2 * sizeof(float) * samplesPerFrame);
        // audioPipe->flush();
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
        DataChunk chunk { data, (size_t)(width * height) };
        videoPipe->add(chunk);
    }
}

const char *ScreenRecorder::ffmpegPath = "/usr/local/bin/ffmpeg";
bool ScreenRecorder::ffmpegInstalled = false;
