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

struct Pipe: AmigaObject {
       
    // Name of this pipe
    const char *path = NULL;
    
    // Pipe identifier
    int pipe = -1;
    
    // Factory method
    static Pipe *make(const char *path);
    
    // Writes a chunk of data
    void send(u8 *data, size_t size);
    
    // Closes the pipe
    void cancel() { close(pipe); }
    
private:
    
    // Starts or stops the worker thread
    void startWorker();
};

class ScreenRecorder : public AmigaComponent {

    //
    // Sub components
    //
    
    // Audio muxer for synthesizing the audio track
    Muxer muxer = Muxer(amiga);
    
    // Path to the FFmpeg executable
    static const char *ffmpegPath() { return "/usr/local/bin/ffmpeg"; }
    
    // Audio sample frequency in the output stream
    static const int frameRate = 50;
    static const int sampleRate = 44100;
    static const int samplesPerFrame = sampleRate / frameRate;

    // File handles to access FFmpeg
    FILE *videoFFmpeg = NULL;
    FILE *audioFFmpeg = NULL;

    // Video and audio pipe
    Pipe *videoPipe = NULL; // Pipe::make("/tmp/videoPipe");
    Pipe *audioPipe = NULL; // Pipe::make("/tmp/audioPipe");

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
    
    bool hasFFmpeg();
    
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
    // bool isReady();
    
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
