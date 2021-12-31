// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Recorder.h"
#include "IOUtils.h"
#include "Denise.h"
#include "MsgQueue.h"
#include "Paula.h"

Recorder::Recorder(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<AmigaComponent *> {
        
        &muxer
    };
}

void
Recorder::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) audioClock = 0;
}

void
Recorder::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    os << tab("FFmpeg path") << FFmpeg::ffmpegPath() << std::endl;
    os << tab("Installed") << bol(FFmpeg::available()) << std::endl;
    os << tab("Recording") << bol(isRecording()) << std::endl;
}
    
util::Time
Recorder::getDuration() const
{
    return (isRecording() ? util::Time::now() : recStop) - recStart;
}

bool
Recorder::startRecording(int x1, int y1, int x2, int y2,
                               long bitRate,
                               long aspectX,
                               long aspectY)
{
    synchronized {
                
        debug(REC_DEBUG, "startRecording(%d,%d,%d,%d,%ld,%ld,%ld)\n",
              x1, y1, x2, y2, bitRate, aspectX, aspectY);

        if (isRecording()) return false;

        // Create pipes
        debug(REC_DEBUG, "Creating pipes...\n");
        
        if (!videoPipe.create(videoPipePath())) {
            
            warn("Failed to create the video encoder pipe.\n");
            return false;
        }
        if (!audioPipe.create(audioPipePath())) {
         
            warn("Failed to create the video encoder pipe.\n");
            return false;
        }
                
        debug(REC_DEBUG, "Pipes created\n");
        dump();
        
        debug(REC_DEBUG, "startRecording(%d,%d,%d,%d,%ld,%ld,%ld)\n",
              x1, y1, x2, y2, bitRate, aspectX, aspectY);
        
        if (isRecording()) return false;
        
        // Make sure the screen dimensions are even
        if ((x2 - x1) % 2) x2--;
        if ((y2 - y1) % 2) y2--;
        
        // Remember the cutout
        cutout.x1 = x1;
        cutout.x2 = x2;
        cutout.y1 = y1;
        cutout.y2 = y2;
        debug(REC_DEBUG, "Recorded area: (%d,%d) - (%d,%d)\n", x1, y1, x2, y2);
        
        // Set the bit rate, frame rate, and sample rate
        this->bitRate = bitRate;
        frameRate = 50;
        sampleRate = 44100;
        samplesPerFrame = sampleRate / frameRate;
        
        // Create temporary buffers
        debug(REC_DEBUG, "Creating buffers...\n");

        if (videoData) delete [] videoData;
        if (audioData) delete [] audioData;
        videoData = new u32[(x2 - x1) * (y2 - y1)];
        audioData = new float[2 * samplesPerFrame];
        
        // Create pipes
        /*
        debug(REC_DEBUG, "Creating pipes...\n");
        
        unlink(videoPipePath().c_str());
        unlink(audioPipePath().c_str());
        if (mkfifo(videoPipePath().c_str(), 0666) == -1) return false;
        if (mkfifo(audioPipePath().c_str(), 0666) == -1) return false;
        */

        //
        // Assemble the command line arguments for the video encoder
        //
        
        debug(REC_DEBUG, "Assembling command line arguments\n");
        
        // Console interactions
        string cmd1 = " -nostdin";
        
        // Verbosity
        cmd1 += " -loglevel " + loglevel();
        
        // Input stream format
        cmd1 += " -f:v rawvideo -pixel_format rgba";
        
        // Frame rate
        cmd1 += " -r " + std::to_string(frameRate);
        
        // Frame size (width x height)
        cmd1 += " -s:v " + std::to_string(x2 - x1) + "x" + std::to_string(y2 - y1);
        
        // Input source (named pipe)
        cmd1 += " -i " + videoPipePath();
        
        // Output stream format
        cmd1 += " -f mp4 -pix_fmt yuv420p";
        
        // Bit rate
        cmd1 += " -b:v " + std::to_string(bitRate) + "k";
        
        // Aspect ratio
        cmd1 += " -bsf:v ";
        cmd1 += "\"h264_metadata=sample_aspect_ratio=";
        cmd1 += std::to_string(aspectX) + "/" + std::to_string(2*aspectY) + "\"";
        
        // Output file
        cmd1 += " -y " + videoStreamPath();
        
        
        //
        // Assemble the command line arguments for the audio encoder
        //
        
        // Console interactions
        string cmd2 = " -nostdin";
        
        // Verbosity
        cmd2 += " -loglevel " + loglevel();
        
        // Audio format and number of channels
        cmd2 += " -f:a f32le -ac 2";
        
        // Sampling rate
        cmd2 += " -sample_rate " + std::to_string(sampleRate);
        
        // Input source (named pipe)
        cmd2 += " -i " + audioPipePath();
        
        // Output stream format
        cmd2 += " -f mp4";
        
        // Output file
        cmd2 += " -y " + audioStreamPath();
        
        //
        // Launch FFmpeg instances
        //
        
        assert(!videoFFmpeg.isRunning());
        assert(!audioFFmpeg.isRunning());
        
        // Launch the video encoder
        debug(REC_DEBUG, "\nLaunching video encoder with options:\n");
        debug(REC_DEBUG, "%s\n", cmd1.c_str());

        if (!videoFFmpeg.launch(cmd1)) {
            
            warn("Failed to launch the FFmpeg video encoder.\n");
            return false;
        }

        // Launch the audio encoder
        debug(REC_DEBUG, "\nLaunching audio encoder with options:\n");
        debug(REC_DEBUG, "%s\n", cmd2.c_str());
        
        if (!audioFFmpeg.launch(cmd2)) {
            
            warn("Failed to launch the FFmpeg audio encoder.\n");
            return false;
        }

        // Open the video pipe
        debug(REC_DEBUG, "Opening video pipe\n");
        
        if (!videoPipe.open()) {

            warn("Failed to launch the video pipe\n");
            return false;
        }
        
        // Open the audio pipe
        debug(REC_DEBUG, "Opening audio pipe\n");

        if (!audioPipe.open()) {
            
            warn("Failed to launch the audio pipe\n");
            return false;
        }
        
        debug(REC_DEBUG, "Success\n");
        state = State::prepare;
    }

    return true;
}

void
Recorder::stopRecording()
{
    debug(REC_DEBUG, "stopRecording()\n");

    synchronized {
        
        if (isRecording()) {
            state = State::finalize;
        }
    }
}

bool
Recorder::exportAs(const string &path)
{
    if (isRecording()) return false;
    
    //
    // Assemble the command line arguments for the video encoder
    //
        
    // Verbosity
    string cmd = "-loglevel " + loglevel();

    // Input streams
    cmd += " -i " + videoStreamPath();
    cmd += " -i " + audioStreamPath();

    // Don't reencode
    cmd += " -c:v copy -c:a copy";

    // Output file
    cmd += " -y " + path;

    //
    // Launch FFmpeg
    //
    
    debug(REC_DEBUG, "\nMerging streams with options:\n%s\n", cmd.c_str());

    FFmpeg merger;
    if (!merger.launch(cmd)) {
        warn("Failed to merge video and audio: %s\n", cmd.c_str());
    }
    merger.join();
    
    debug(REC_DEBUG, "Success\n");
    return true;
}

void
Recorder::vsyncHandler(Cycle target)
{
    // Quick-exit if the recorder is not active
    if (state == State::wait) return;
    
    synchronized {
        
        switch (state) {
                
            case State::wait: break;
            case State::prepare: prepare(); break;
            case State::record: record(target); break;
            case State::finalize: finalize(); break;
            case State::abort: abort(); break;
        }
    }
}

void
Recorder::prepare()
{
    state = State::record;
    audioClock = 0;
    recStart = util::Time::now();
    msgQueue.put(MSG_RECORDING_STARTED);
}

void
Recorder::record(Cycle target)
{
    assert(videoFFmpeg.isRunning());
    assert(audioFFmpeg.isRunning());
    assert(videoPipe.isOpen());
    assert(audioPipe.isOpen());
    
    recordVideo(target);
    recordAudio(target);
}

void
Recorder::recordVideo(Cycle target)
{
    ScreenBuffer buffer = denise.pixelEngine.getStableBuffer();
    
    isize width = sizeof(u32) * (cutout.x2 - cutout.x1);
    isize height = cutout.y2 - cutout.y1;
    isize offset = cutout.y1 * HPIXELS + cutout.x1 + HBLANK_MIN * 4;
    u8 *src = (u8 *)(buffer.data + offset);
    u8 *dst = (u8 *)videoData;
    
    for (isize y = 0; y < height; y++, src += 4 * HPIXELS, dst += width) {
        std::memcpy(dst, src, width);
    }
    
    // Feed the video pipe
    assert(videoPipe.isOpen());
    isize length = width * height;
    isize written = videoPipe.write((u8 *)videoData, length);
    
    if (written != length || FORCE_RECORDING_ERROR) {
        state = State::abort;
    }
}

void
Recorder::recordAudio(Cycle target)
{
    
    // Clone Paula's muxer contents
    muxer.sampler[0] = paula.muxer.sampler[0];
    muxer.sampler[1] = paula.muxer.sampler[1];
    muxer.sampler[2] = paula.muxer.sampler[2];
    muxer.sampler[3] = paula.muxer.sampler[3];
    assert(muxer.sampler[0].r == paula.muxer.sampler[0].r);
    assert(muxer.sampler[0].w == paula.muxer.sampler[0].w);
    
    // If this is the first frame to record, adjust the audio clock
    if (audioClock == 0) audioClock = target-1;
    
    // Synthesize audio samples
    muxer.synthesize(audioClock, target, samplesPerFrame);
    audioClock = target;
    
    // Copy samples to buffer
    muxer.copy(audioData, samplesPerFrame);
    
    // Feed the audio pipe
    assert(audioPipe.isOpen());
    isize length = 2 * sizeof(float) * samplesPerFrame;
    isize written = audioPipe.write((u8 *)audioData, length);
 
    if (written != length || FORCE_RECORDING_ERROR) {
        state = State::abort;
    }
}

void
Recorder::finalize()
{    
    // Close pipes
    videoPipe.close();
    audioPipe.close();
    
    // Wait for the decoders to terminate
    videoFFmpeg.join();
    audioFFmpeg.join();
    
    // Switch state and inform the GUI
    state = State::wait;
    recStop = util::Time::now();
    msgQueue.put(MSG_RECORDING_STOPPED);
}

void
Recorder::abort()
{
    finalize();
    msgQueue.put(MSG_RECORDING_ABORTED);
}
