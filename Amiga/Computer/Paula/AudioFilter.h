// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AUDIO_FILTER_INC
#define _AUDIO_FILTER_INC

#include "HardwareComponent.h"

class AudioFilter : public HardwareComponent {
    
    // The currently set filter type
    FilterType type;
    
    // Coefficients of the butterworth filter
    double a1, a2, b0, b1, b2;
    
    // The butterworth filter pipeline
    double x1, x2, y1, y2;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    AudioFilter();


    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & type;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }
    
    
    //
    // Methods from HardwareComponent
    //

private:

    size_t _load(uint8_t *buffer) override;
    size_t _save(uint8_t *buffer) override;


    //
    // Configuring the device
    //
    
public:
    
    // Filter type
    FilterType getFilterType() { return type; }
    void setFilterType(FilterType type);

    // Sample rate
    void setSampleRate(double sampleRate);

    
    //
    // Using the device
    //

    // Initializes the filter pipeline with zero elements
    void clear();

    // Inserts a sample into the filter pipeline
    float apply(float sample);
    
};
    
#endif
