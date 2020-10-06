// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
AudioStream::clearRingbuffer()
{
    // Wipe out the ringbuffer
    clear(SamplePair {0, 0});
    
    // Put the write pointer ahead of the read pointer
    alignWritePtr();
}

void
AudioStream::copy(float *left, float *right, size_t n,
                  i32 &volume, i32 targetVolume, i32 volumeDelta)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);

    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (size_t i = 0; i < n; i++) {
            
            SamplePair pair = read();
            *left++ = pair.left * scale;
            *right++ = pair.right * scale;
        }

    } else {
        
        for (size_t i = 0; i < n; i++) {
                            
            if (volume < targetVolume) {
                volume += MIN(volumeDelta, targetVolume - volume);
            } else {
                volume -= MIN(volumeDelta, volume - targetVolume);
            }

            float scale = volume / 10000.0f;

            SamplePair pair = read();
            *left++ = pair.left * scale;
            *right++ = pair.right * scale;
        }
    }
}

void
AudioStream::copy(float *left, float *right, size_t n,
                  i32 &volume, i32 targetVolume, i32 volumeDelta,
                  AudioFilter &filterL, AudioFilter &filterR)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);
    
    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (size_t i = 0; i < n; i++) {
            
            SamplePair pair = read();
            *left++ = filterL.apply(pair.left) * scale;
            *right++ = filterR.apply(pair.right) * scale;
        }
        
    } else {
        
        for (size_t i = 0; i < n; i++) {
            
            if (volume < targetVolume) {
                volume += MIN(volumeDelta, targetVolume - volume);
            } else {
                volume -= MIN(volumeDelta, volume - targetVolume);
            }
            
            float scale = volume / 10000.0f;
            
            SamplePair pair = read();
            *left++ = filterL.apply(pair.left) * scale;
            *right++ = filterR.apply(pair.right) * scale;
        }
    }
}

void
AudioStream::copyMono(float *buffer, size_t n,
                      i32 &volume, i32 targetVolume, i32 volumeDelta)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);

    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (size_t i = 0; i < n; i++) {
            
            SamplePair pair = read();
            *buffer++ = (pair.left + pair.right) * scale;
        }

    } else {
        
        for (size_t i = 0; i < n; i++) {
                            
            if (volume < targetVolume) {
                volume += MIN(volumeDelta, targetVolume - volume);
            } else {
                volume -= MIN(volumeDelta, volume - targetVolume);
            }

            float scale = volume / 10000.0f;

            SamplePair pair = read();
            *buffer++ = (pair.left + pair.right) * scale;
        }
    }
}

void
AudioStream::copyMono(float *buffer, size_t n,
                      i32 &volume, i32 targetVolume, i32 volumeDelta,
                      AudioFilter &filterL, AudioFilter &filterR)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);
    
    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (size_t i = 0; i < n; i++) {
            
            SamplePair pair = read();
            *buffer++ = (filterL.apply(pair.left) + filterR.apply(pair.right)) * scale;
        }
        
    } else {
        
        for (size_t i = 0; i < n; i++) {
            
            if (volume < targetVolume) {
                volume += MIN(volumeDelta, targetVolume - volume);
            } else {
                volume -= MIN(volumeDelta, volume - targetVolume);
            }
            
            float scale = volume / 10000.0f;
            
            SamplePair pair = read();
            *buffer++ = (filterL.apply(pair.left) + filterR.apply(pair.right)) * scale;
        }
    }
}
