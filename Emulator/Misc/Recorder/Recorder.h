// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Buffer.h"
#include "Chrono.h"
#include "FFmpeg.h"
#include "AudioPort.h"
#include "NamedPipe.h"

namespace vamiga {

using util::Buffer;

class Recorder final : public SubComponent {

    Descriptions descriptions = {{

        .type           = RecorderClass,
        .name           = "Recorder",
        .description    = "Video Recorder",
        .shell          = ""

    }};

    ConfigOptions options = {

    };

    //
    // Subcomponents
    //
    
    // Custom audio port for synthesizing the audio track
    AudioPort audioPort = AudioPort(amiga, 1);


    //
    // Handles
    //
    
    // FFmpeg instances
    FFmpeg videoFFmpeg;
    FFmpeg audioFFmpeg;

    // Video and audio pipes
    NamedPipe videoPipe;
    NamedPipe audioPipe;

    
    //
    // Recording status
    //
    
    // All possible recorder states
    enum class State { wait, prepare, record, finalize, abort };

    // The current recorder state
    State state = State::wait;
    
    // Audio has been recorded up to this cycle
    Cycle audioClock = 0;

    
    //
    // Recording parameters
    //
    
    // Frame rate, Bit rate, Sample rate
    isize frameRate = 0;
    isize bitRate = 0;
    isize sampleRate = 0;

    // Sound samples per frame
    isize samplesPerFrame = 0;

    // The texture cutout that is going to be recorded
    struct { isize x1; isize y1; isize x2; isize y2; } cutout;

    // Time stamps
    util::Time recStart;
    util::Time recStop;

    // Temporary storage for audio and video data
    Buffer<u32> videoData;
    Buffer<float> audioData;

    
    //
    // Initializing
    //
    
public:
    
    Recorder(Amiga& ref);
    
    Recorder& operator= (const Recorder& other) {

        CLONE(audioPort)
        CLONE(audioClock)

        return *this;
    }


    //
    // Methods from Serializable
    //

public:

    template <class T>
    void serialize(T& worker)
    {
        if (isHardResetter(worker)) {

            worker << audioClock;
        }

    } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream& os) const override;
    void _initialize() override;


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


    //
    // Querying locations and flags
    //
    
    // Returns the paths to the two named input pipes
    string videoPipePath();
    string audioPipePath();

    // Return the paths to the two temporary output files
    string videoStreamPath();
    string audioStreamPath();

    //Returns the log level passed to FFmpeg
    const string loglevel() { return REC_DEBUG ? "verbose" : "warning"; }

    
    //
    // Querying recording parameters
    //

public:
    
    util::Time getDuration() const;
    isize getFrameRate() const { return frameRate; }
    isize getBitRate() const { return bitRate; }
    isize getSampleRate() const { return sampleRate; }

    
    //
    // Starting and stopping a video capture
    //
    
public:

    // Checks whether the screen is currently recorded
    bool isRecording() const { return state != State::wait; }

    // Starts the screen recorder
    void startRecording(isize x1, isize y1, isize x2, isize y2,
                        isize bitRate,
                        isize aspectX, isize aspectY) throws;
    
    // Stops the screen recorder
    void stopRecording();

    // Exports the recorded video
    bool exportAs(const std::filesystem::path &path);

    
    //
    // Recording a video stream
    //
   
public:

    // Records a single frame
    void vsyncHandler(Cycle target);
    
private:
    
    void prepare();
    void record(Cycle target);
    void recordVideo(Cycle target);
    void recordAudio(Cycle target);
    void finalize();
    void abort();
};

}
