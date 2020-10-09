// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _SCREEN_RECORDER_H
#define _SCREEN_RECORDER_H

#include "AmigaComponent.h"
#include "Muxer.h"

/* BufferedPipe is wrapper around a classical POSIX pipe which is utilized to
 * feed video and audio data into FFmpeg. The class maintains a FIFO of data
 * chunks and a worker thread that gets active whenever the FIFO has at least
 * one element. The thread waits for the receiver to accept data and dumps
 * all buffered data chunks into the pipe until the FIFO is emptied.
 */
struct BufferedPipe: AmigaObject {
       
    // Name of this pipe
    const char *path = NULL;
    
private:

    // Pipe identifier
    int pipe = -1;
    
    // Data buffer
    struct DataChunk { u8 *data; size_t size; };
    std::queue<DataChunk> fifo;
    
    // Worker thread
    std::thread t;
    
    // Mutex used to block the thread while the FIFO is empty
    std::mutex m;
        
    // Indicates if the thread is running. Set to false to terminate
    bool running = false;
    
    // The worker thread's execution function
    void worker();

public:
        
    // Factory method
    static BufferedPipe *make(const char *path);
    
    // Writes a new data chunk into the FIFO
    void send(u8 *data, size_t size);
    
    // Tell the thread to terminate
    void cancel() { stopWorker(); }
    
    // Wait until the thread has terminated
    void join() { t.join(); }
    
private:
    
    // Starts or stops the worker thread
    void startWorker();
    void stopWorker();
};

class ScreenRecorder : public AmigaComponent {

    //
    // Sub components
    //
    
    // Audio muxer for synthesizing the audio track
    Muxer muxer = Muxer(amiga);

    
    // Path to the FFmpeg executable
    static const char *ffmpegPath;
    
    // Indicates whether FFmpeg is installed on this machine
    static bool ffmpegInstalled;

    // Audio sample frequency in the output stream
    static const int frameRate = 50;
    static const int sampleRate = 44100;
    static const int samplesPerFrame = sampleRate / frameRate;

    // File handle to access the FFmpeg encoder
    FILE *ffmpeg = NULL;
        
    // Video and audio pipe
    BufferedPipe *videoPipe = BufferedPipe::make("/tmp/videoPipe");
    BufferedPipe *audioPipe = BufferedPipe::make("/tmp/audioPipe");

    // Indicates if the recorder is active
    bool recording = false;
    
    // Time stamp recorded in the vSync handler
    Cycle audioClock = 0;
    
    // Path of the output file
    char *outfile = NULL;

    // The texture cutout that is going to be recorded
    struct { int x1; int y1; int x2; int y2; } cutout;

    // Bitrate passed to FFmpeg
    long bitRate = 0;
    
    // Pixel aspect ratio
    long aspectX;
    long aspectY;
            
    
    //
    // Initializing
    //
    
public:
    
    ScreenRecorder(Amiga& ref);
    bool isFFmpegInstalled() { return ffmpegInstalled; }
    
    void _reset(bool hard) override;

    
    //
    // Analyzing
    //

private:
    
    void _dump() override;
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Preparing to record a video stream
    //
    
public:
    
    // Sets the target file name
    bool setPath(const char *path);
    
    
    //
    // Starting and stopping a video stream
    //
    
public:
    
    // Checks whether the screen recorder is fully functional
    bool isReady();
    
    // Checks whether the screen is currently recorded
    bool isRecording();
    
    // Start or stop the screen recorder
    bool startRecording(int x1, int y1, int x2, int y2,
                        long bitRate,
                        long aspectX,
                        long aspectY);
    void stopRecording();

    
    //
    // Recording a video stream
    //

public:
    
    // Writes an audio sample to the temporary buffer
    // void addSample(float left, float right);
    
    // Records a single frame
    void vsyncHandler(Cycle target);
};

#endif
