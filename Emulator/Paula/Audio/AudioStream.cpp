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
AudioStream::copy(float *left, float *right, size_t n,
                  float &volume, float targetVolume, float volumeDelta)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(samples.count() < n);

    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (size_t i = 0; i < n; i++) {
            
            SamplePair pair = samples.read();
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

            SamplePair pair = samples.read();
            *left++ = pair.left * scale;
            *right++ = pair.right * scale;
        }
    }
}

void
AudioStream::copy(float *left, float *right, size_t n,
                  float &volume, float targetVolume, float volumeDelta,
                  AudioFilter &filterL, AudioFilter &filterR)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(samples.count() < n);
    
    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (size_t i = 0; i < n; i++) {
            
            SamplePair pair = samples.read();
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
            
            SamplePair pair = samples.read();
            *left++ = filterL.apply(pair.left) * scale;
            *right++ = filterR.apply(pair.right) * scale;
        }
    }
}

void
AudioStream::copyMono(float *buffer, size_t n,
                      float &volume, float targetVolume, float volumeDelta)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(samples.count() < n);

    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (size_t i = 0; i < n; i++) {
            
            SamplePair pair = samples.read();
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

            SamplePair pair = samples.read();
            *buffer++ = (pair.left + pair.right) * scale;
        }
    }
}

void
AudioStream::copyMono(float *buffer, size_t n,
                      float &volume, float targetVolume, float volumeDelta,
                      AudioFilter &filterL, AudioFilter &filterR)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(samples.count() < n);
    
    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (size_t i = 0; i < n; i++) {
            
            SamplePair pair = samples.read();
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
            
            SamplePair pair = samples.read();
            *buffer++ = (filterL.apply(pair.left) + filterR.apply(pair.right)) * scale;
        }
    }
}
