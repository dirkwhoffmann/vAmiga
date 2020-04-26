// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _PAULA_AUDIO_T_INC
#define _PAULA_AUDIO_T_INC

//
// Enumerations
//

typedef enum : long
{
    SMP_NONE,
    SMP_NEAREST,
    SMP_LINEAR
}
SamplingMethod;

static inline bool isSamplingMethod(long value) { return value >= 0 && value <= SMP_LINEAR; }

typedef enum : long
{
    FILT_NONE,
    FILT_BUTTERWORTH,
    FILT_COUNT
}
FilterType;

static inline bool isFilterType(long value) { return value >= 0 && value < FILT_COUNT; }

typedef enum : long
{
    FILTACT_POWER_LED,
    FILTACT_NEVER,
    FILTACT_ALWAYS,
    FILTACT_COUNT
}
FilterActivation;

static inline bool isFilterActivation(long value) { return value >= 0 && value < FILTACT_COUNT; }

//
// Structures
//

typedef struct
{
    // The sample rate in Hz
    double sampleRate;

    // The sample interpolation method
    SamplingMethod samplingMethod;

    // Determines when the audio filter is active
    FilterActivation filterActivation;

    // Selected audio filter type
    FilterType filterType;
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

#endif
