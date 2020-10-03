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

class ScreenRecorder : public AmigaComponent {

    // Path to the FFmpeg executable
    static const char *ffmpegPath;

    // Path to the pipes where FFmpeg reads the input from
    static const char *audioPipePath;
    static const char *videoPipePath;
    
    // Indicates whether FFmpeg is installed on this machine
    static bool ffmpegInstalled;

    // Audio sample frequency in the output stream
    static const int sampleRate = 48000;
    
    // File handle to access the FFmpeg encoder
    FILE *ffmpeg = NULL;

    // File handles to the data pipes
    int audioPipe = -1;
    int videoPipe = -1;

    // Path of the output file
    char *outfile = NULL;

    // The texture cutout that is going to be recorded
    struct { int x1; int y1; int x2; int y2; } cutout;

    // Bitrate passed to FFmpeg
    long bitRate = 0;
    
    // Pixel aspect ratio
    long aspectX;
    long aspectY;
    
    // Temporary pixel storage
    u32 pixels[1024*320];
    
    // Temporary aUdio storage
    float samples[sampleRate + 100][2];
    int samplesCnt = 0;
    
    
    //
    // Initializing
    //
    
public:
    
    ScreenRecorder(Amiga& ref);
    bool isFFmpegInstalled() { return ffmpegInstalled; }
    
    void _reset(bool hard) override;

    
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
    
private:
    
    // Creates the video and audio pipe (FFmpeg reads from them)
    bool createsPipes();
    
    // Opens or closes the video and audio pipe
    bool openPipes();
    void closePipes();

    // Start or stops FFmpeg
    bool startFFmpeg(const char *cmd);
    void stopFFmpeg();
        
    
    //
    // Recording a video stream
    //

public:
    
    // Writes an audio sample to the temporary buffer
    void addSample(float left, float right);
    
    // Records a single frame
    void vsyncHandler();
};

#endif
