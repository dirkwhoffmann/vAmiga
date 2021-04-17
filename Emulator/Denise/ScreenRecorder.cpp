// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ScreenRecorder.h"
#include "IO.h"
#include "Denise.h"
#include "MsgQueue.h"
#include "Paula.h"

ScreenRecorder::ScreenRecorder(Amiga& ref) : AmigaComponent(ref)
{
    subComponents = std::vector<HardwareComponent *> {
        
        &muxer
    };
}

bool
ScreenRecorder::hasFFmpeg() const
{
    return util::getSizeOfFile(ffmpegPath()) > 0;
}

void
ScreenRecorder::_initialize()
{
        
}

void
ScreenRecorder::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

void
ScreenRecorder::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    os << tab("ffmpeg path") << ffmpegPath() << std::endl;
    os << tab("Installed") << bol(hasFFmpeg()) << std::endl;
    os << tab("Video pipe") << bol(videoPipe != -1) << std::endl;
    os << tab("Audio pipe") << bol(audioPipe != -1) << std::endl;
}
    
bool
ScreenRecorder::startRecording(int x1, int y1, int x2, int y2,
                               long bitRate,
                               long aspectX,
                               long aspectY)
{
    if (isRecording()) return false;

    // Create pipes
    debug(REC_DEBUG, "Creating pipes...\n");

    unlink(videoPipePath().c_str());
    unlink(audioPipePath().c_str());
    if (mkfifo(videoPipePath().c_str(), 0666) == -1) return false;
    if (mkfifo(audioPipePath().c_str(), 0666) == -1) return false;
        
    debug(REC_DEBUG, "Pipes created\n");
    dump();
    
    synchronized {
        
        // Make sure the screen dimensions are even
        if ((x2 - x1) % 2) x2--;
        if ((y2 - y1) % 2) y2--;
        cutout.x1 = x1;
        cutout.x2 = x2;
        cutout.y1 = y1;
        cutout.y2 = y2;
        debug(REC_DEBUG, "Recorded area: (%d,%d) - (%d,%d)\n", x1, y1, x2, y2);
        
        //
        // Assemble the command line arguments for the video encoder
        //
                
        // Path to the FFmpeg executable
        // ptr += sprintf(ptr, "%s -nostdin", ffmpegPath());
        string cmd1 = ffmpegPath() + " -nostdin";
        
        // Verbosity
        // ptr += sprintf(ptr, " -loglevel %s", loglevel());
        cmd1 += " -loglevel " + loglevel();

        // Input stream format
        // ptr += sprintf(ptr, " -f:v rawvideo -pixel_format rgba");
        cmd1 += " -f:v rawvideo -pixel_format rgba";

        // Frame rate
        // ptr += sprintf(ptr, " -r %d", frameRate);
        cmd1 += " -r " + std::to_string(frameRate);

        // Frame size (width x height)
        // ptr += sprintf(ptr, " -s:v %dx%d", x2 - x1, y2 - y1);
        cmd1 += " -s:v " + std::to_string(x2 - x1) + "x" + std::to_string(y2 - y1);

        // Input source (named pipe)
        // ptr += sprintf(ptr, " -i %s", videoPipePath());
        cmd1 += " -i " + videoPipePath();

        // Output stream format
        // ptr += sprintf(ptr, " -f mp4 -pix_fmt yuv420p");
        cmd1 += " -f mp4 -pix_fmt yuv420p";

        // Bit rate
        // ptr += sprintf(ptr, " -b:v %ldk", bitRate);
        cmd1 += " -b:v " + std::to_string(bitRate) + "k";

        // Aspect ratio
        // ptr += sprintf(ptr, " -bsf:v ");
        // ptr += sprintf(ptr, "\"h264_metadata=sample_aspect_ratio=");
        // ptr += sprintf(ptr, "%ld/%ld\"", aspectX, 2*aspectY);
        cmd1 += " -bsf:v ";
        cmd1 += "\"h264_metadata=sample_aspect_ratio=";
        cmd1 += std::to_string(aspectX) + "/" + std::to_string(2*aspectY) + "\"";

        // Output file
        // ptr += sprintf(ptr, " -y %s", videoStreamPath());
        cmd1 += " -y " + videoStreamPath();

        
        //
        // Assemble the command line arguments for the audio encoder
        //
                
        // Path to the FFmpeg executable
        // ptr += sprintf(ptr, "%s -nostdin", ffmpegPath());
        string cmd2 = ffmpegPath() + " -nostdin";

        // Verbosity
        // ptr += sprintf(ptr, " -loglevel %s", loglevel());
        cmd2 += " -loglevel " + loglevel();

        // Audio format and number of channels
        // ptr += sprintf(ptr, " -f:a f32le -ac 2");
        cmd2 += " -f:a f32le -ac 2";

        // Sampling rate
        // ptr += sprintf(ptr, " -sample_rate %d", sampleRate);
        cmd2 += " -sample_rate " + std::to_string(sampleRate);

        // Input source (named pipe)
        // ptr += sprintf(ptr, " -i %s", audioPipePath());
        cmd2 += " -i " + audioPipePath();

        // Output stream format
        // ptr += sprintf(ptr, " -f mp4");
        cmd2 += " -f mp4";

        // Output file
        // ptr += sprintf(ptr, " -y %s", audioStreamPath());
        cmd2 += " -y " + audioStreamPath();

        //
        // Launch FFmpeg instances
        //
        
        assert(videoFFmpeg == nullptr);
        assert(audioFFmpeg == nullptr);
        
        msg("\nStarting video encoder with options:\n%s\n", cmd1.c_str());
        videoFFmpeg = popen(cmd1.c_str(), "w");
        msg(videoFFmpeg ? "Success\n" : "Failed to launch\n");
        
        msg("\nStarting audio encoder with options:\n%s\n", cmd2.c_str());
        audioFFmpeg = popen(cmd2.c_str(), "w");
        msg(audioFFmpeg ? "Success\n" : "Failed to launch\n");
        
        // Open pipes
        videoPipe = open(videoPipePath().c_str(), O_WRONLY);
        audioPipe = open(audioPipePath().c_str(), O_WRONLY);
        debug(REC_DEBUG, "Pipes are open");
        
        recording = videoFFmpeg && audioFFmpeg && videoPipe != -1 && audioPipe != -1;
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
    debug(REC_DEBUG, "stopRecording()\n");
    
    if (!isRecording()) return;
    
    synchronized {
        recording = false;
        recordCounter++;
        audioClock = 0;
    }

    // Close pipes
    close(videoPipe);
    close(audioPipe);
    videoPipe = -1;
    audioPipe = -1;
     
    // Shut down encoders
    pclose(videoFFmpeg);
    pclose(audioFFmpeg);
    videoFFmpeg = nullptr;
    audioFFmpeg = nullptr;
    
    debug(REC_DEBUG, "Recording has stopped\n");
    messageQueue.put(MSG_RECORDING_STOPPED);
}

bool
ScreenRecorder::exportAs(const string &path)
{
    if (isRecording()) return false;
    
    //
    // Assemble the command line arguments for the video encoder
    //
    
    // char cmd[512]; char *ptr = cmd;
    string cmd;
    
    // Path to the FFmpeg executable
    // ptr += sprintf(ptr, "%s", ffmpegPath());
    cmd += ffmpegPath();

    // Verbosity
    // ptr += sprintf(ptr, " -loglevel %s", loglevel());
    cmd += " -loglevel " + loglevel();

    // Input streams
    // ptr += sprintf(ptr, " -i %s", videoStreamPath());
    // ptr += sprintf(ptr, " -i %s", audioStreamPath());
    cmd += " -i " + videoStreamPath();
    cmd += " -i " + audioStreamPath();

    // Don't reencode
    // ptr += sprintf(ptr, " -c:v copy -c:a copy");
    cmd += " -c:v copy -c:a copy";

    // Output file
    // ptr += sprintf(ptr, " -y %s", path);
    cmd += " -y " + path;

    //
    // Launch FFmpeg
    //
    
    msg("\nMerging video and audio stream with options:\n%s\n", cmd.c_str());
    /*
    FILE *ffmpeg = popen(cmd, "w");

    if (!ffmpeg) {
        msg("Failed to launch\n");
        return false;
    }
    
    // Wait for FFmpeg to finish
    fclose(ffmpeg);
    */
    if (system(cmd.c_str()) == -1) {
        warn("Failed: %s\n", cmd.c_str());
    }
    
    msg("Done\n");
    return true;
}

void
ScreenRecorder::vsyncHandler(Cycle target)
{
    if (!isRecording()) return;
    
    // debug("vsyncHandler\n");
    assert(videoFFmpeg != nullptr);
    assert(audioFFmpeg != nullptr);
    
    synchronized {
        
        //
        // Video
        //
        
        ScreenBuffer buffer = denise.pixelEngine.getStableBuffer();
        
        isize width = sizeof(u32) * (cutout.x2 - cutout.x1);
        isize height = cutout.y2 - cutout.y1;
        isize offset = cutout.y1 * HPIXELS + cutout.x1 + HBLANK_MIN * 4;
        u8 *data = new u8[width * height];
        u8 *src = (u8 *)(buffer.data + offset);
        u8 *dst = data;
        for (isize y = 0; y < height; y++, src += 4 * HPIXELS, dst += width) {
            memcpy(dst, src, width);
        }
        
        // Feed the video pipe
        assert(videoPipe != -1);
        (void)write(videoPipe, data, width * height);
        
        //
        // Audio
        //
        
        // Clone Paula's muxer contents
        muxer.sampler[0]->clone(*paula.muxer.sampler[0]);
        muxer.sampler[1]->clone(*paula.muxer.sampler[1]);
        muxer.sampler[2]->clone(*paula.muxer.sampler[2]);
        muxer.sampler[3]->clone(*paula.muxer.sampler[3]);
        assert(muxer.sampler[0]->r == paula.muxer.sampler[0]->r);
        assert(muxer.sampler[0]->w == paula.muxer.sampler[0]->w);
        
        // If this is the first frame to record, adjust the audio clock
        if (audioClock == 0) audioClock = target-1;

        // Synthesize audio samples
        muxer.synthesize(audioClock, target, samplesPerFrame);
        audioClock = target;
        
        // Copy samples to buffer
        float *samples = new float[2 * samplesPerFrame];
        muxer.copy(samples, samplesPerFrame);
        
        // Feed the audio pipe
        assert(audioPipe != -1);
        (void)write(audioPipe, (u8 *)samples, 2 * sizeof(float) * samplesPerFrame);
    }
}
