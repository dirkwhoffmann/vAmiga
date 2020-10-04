// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _PAULA_AUDIO_TYPES_H
#define _PAULA_AUDIO_TYPES_H

//
// Enumerations
//

typedef VA_ENUM(long, SamplingMethod)
{
    SMP_NONE,
    SMP_NEAREST,
    SMP_LINEAR
};

static inline bool isSamplingMethod(long value) { return value >= 0 && value <= SMP_LINEAR; }

typedef VA_ENUM(long, FilterType)
{
    FILT_NONE,
    FILT_BUTTERWORTH,
    FILT_COUNT
};

static inline bool isFilterType(long value) { return value >= 0 && value < FILT_COUNT; }


//
// Structures
//

typedef struct
{
    // The target frequency in Hz
    double sampleRate;

    // Selects how the audio buffer is resampled to match the target frequency
    SamplingMethod samplingMethod;

    // The selected audio filter
    FilterType filterType;

    // If set to true, the Amiga can't deactivate the filter
    bool filterAlwaysOn;

    // Input channel volumes and pan settings
    double vol[4];
    double pan[4];
    
    // Output channel volumes
    double volL;
    double volR;
}
AudioConfig;

typedef struct
{
    i8 state;
    bool dma;
    u16 audlenLatch;
    u16 audlen;
    u16 audperLatch;
    i32 audper;
    u16 audvolLatch;
    u16 audvol;
    u16 auddat;
}
AudioChannelInfo;

typedef struct
{
    AudioChannelInfo channel[4];
}
AudioInfo;

typedef struct
{
    long bufferUnderflows;
    long bufferOverflows;
}
AudioStats;

typedef struct
{
    float left;
    float right;
}
SamplePair;

#endif
