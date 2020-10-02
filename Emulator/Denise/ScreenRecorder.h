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

    // Path to the FFmpef executable
    static const char *ffmpegPath;

    // Indicates whether FFmpeg is installed on this machine
    static bool ffmpegInstalled;
    
    // File handle to access the FFmpeg encoder
    FILE *ffmpeg = NULL;

    // The texture cutout that is going to be recorded
    struct { int x1; int y1; int x2; int y2; } cutout;

    // Bitrate passed to FFmpeg
    long bitRate = 0;
    
    // Selected audio and video codecs
    long audioCodec = 0;
    long videoCodec = 0;
    
    // Temporary buffer (experimental)
    u32 pixels[PIXELS];
    
    
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
    // Recording a video stream
    //
    
public:
    
    // Checks whether the screen is currently recorded
    bool isRecording();
    
    // Start or stop the screen recorder
    int startRecording(int x1, int y1, int x2, int y2,
                       long bitRate,
                       long audioCodec,
                       long videoCodec);
    void stopRecording();
    
    
    // Records a single frame
    void vsyncHandler();
};

#endif
