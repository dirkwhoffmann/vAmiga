// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

PaulaAudio::PaulaAudio(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("AudioUnit");
    
    subComponents = vector<HardwareComponent *> {

        &channel0,
        &channel1,
        &channel2,
        &channel3,
        &muxer
    };
}

void
PaulaAudio::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)    
}

void
PaulaAudio::_inspect()
{
    synchronized {
        
        info.channel[0] = channel0.getInfo();
        info.channel[1] = channel1.getInfo();
        info.channel[2] = channel2.getInfo();
        info.channel[3] = channel3.getInfo();
    }
}

void
PaulaAudio::executeUntil(Cycle target)
{
    muxer.synthesize(clock, target);
    clock = target;
}

void
PaulaAudio::readMonoSamples(float *buffer, size_t n)
{
    muxer.copyMono(buffer, n);
}

void
PaulaAudio::readStereoSamples(float *left, float *right, size_t n)
{
    muxer.copy(left, right, n);
}
