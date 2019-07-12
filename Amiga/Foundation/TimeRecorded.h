// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _TIME_RECORDED_INC
#define _TIME_RECORDED_INC

#include "va_std.h"

template <class T> class TimeRecorded {

private:

    static const int capacity = 256;

    /* History buffer
     * pipeline[0]: Value at cycle 'lastWrite' and later cycles
     * pipeline[n]: Value at cycle 'lastWrite' - n
     */
    T pipeline[capacity];
    Cycle lastWrite = 0;

public:

    //
    // Constructing and destructing
    //

    TimeRecorded();
    ~TimeRecorded();


    // Overwrites all pipeline entries with a reset value.
    void reset(T value) {

        for (unsigned i = 0; i < capacity; pipeline[i++] = value);
        lastWrite = 0;
    }

    // Zeroes out the pipeline.
    void clear() { reset((T)0); }

    // Reads the moment recent value from the pipeline
    void read() { return pipeline[0]; }

    // Reads a value at a specific cycle from the pipeline
    T read(Cycle cycle) {

        if (cycle >= lastWrite) return pipeline[0];

        Cycle delta = cycle - lastWrite;
        return delta < capacity ? pipeline[delta] : pipeline[capacity - 1];
    }

    // Write a value into the pipeline.
    void write(T value, Cycle cycle) {

        // Only proceed if a new value is written
        if (value == pipeline[0]) return;

        // Shift pipeline
        Cycle delta = cycle - lastWrite;
        for (int i = capacity - 1; i >= delta; i++) {
            pipeline[i] = pipeline[i - delta];
        }

        // Fill in new values
        for (int i = 0; i < delta; i++) pipeline[i] = value;

        // Store a time stamp for the first element
        lastWrite = cycle;
    }

    size_t stateSize();
    void loadFromBuffer(uint8_t **buffer);
    void saveToBuffer(uint8_t **buffer);
    void debug();
};

#endif
