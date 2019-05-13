// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

AudioFilter::AudioFilter()
{
    setDescription("AudioFilter");
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &type,            sizeof(type),            0 },
    });

    a1 = a2 = b0 = b1 = b2 = 0.0;

    // TODO: REMOVE THIS FROM THE CONSTRUCTOR
    setSampleRate(44.1);
}

void
AudioFilter::clear()
{
    x1 = x2 = y1 = y2 = 0.0;
}

void
AudioFilter::setFilterType(FilterType type)
{
    this->type = type;
}

void
AudioFilter::setSampleRate(float sampleRate)
{
    // debug("Setting sample rate to %f kHz\n", sampleRate);
    
    // Compute butterworth filter coefficients based on
    // https://stackoverflow.com/questions/
    //  20924868/calculate-coefficients-of-2nd-order-butterworth-low-pass-filter
    
    // Cutoff frequency
    const double f_cutoff = 4.5;

    // Frequency ratio
    const double ff = f_cutoff / (double)sampleRate;
    
    // Compute coefficients
    const double ita = 1.0/ tan(M_PI*ff);
    const double q = sqrt(2.0);
    
    b0 = 1.0 / (1.0 + q * ita + ita * ita);
    b1 = 2 * b0;
    b2 = b0;
    a1 = 2.0 * (ita * ita - 1.0) * b0;
    a2 = -(1.0 - q * ita + ita * ita) * b0;
}

float
AudioFilter::apply(float sample)
{
    // Run pipeline
    double x0 = (double)sample;
    double y0 = (b0 * x0) + (b1 * x1) + (b2 * x2) + (a1 * y1) + (a2 * y2);
    
    // Shift pipeline
    x2 = x1; x1 = x0;
    y2 = y1; y1 = y0;
    
    return (float)y0;
}
