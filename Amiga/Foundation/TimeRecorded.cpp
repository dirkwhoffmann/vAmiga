// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "TimeRecorded.h"

template <class T>
TimeRecorded<T>::TimeRecorded()
{
    clear();
}
template TimeRecorded<uint16_t>::TimeRecorded();

template <class T>
size_t TimeRecorded<T>::stateSize()
{
    return capacity * sizeof(uint64_t) + sizeof(lastWrite);
}
template size_t TimeRecorded<uint16_t>::stateSize();

template <class T>
void TimeRecorded<T>::loadFromBuffer(uint8_t **buffer)
{
    uint8_t *old = *buffer;

    for (unsigned i = 0; i < capacity; i++) {
        pipeline[i] = (T)read64(buffer);
    }
    lastWrite = (Cycle)read64(buffer);

    assert(*buffer - old == stateSize());
}
template void TimeRecorded<uint16_t>::loadFromBuffer(uint8_t **);

template <class T>
void TimeRecorded<T>::saveToBuffer(uint8_t **buffer)
{
    uint8_t *old = *buffer;

    for (unsigned i = 0; i < capacity; i++) {
        write64(buffer, (uint64_t)pipeline[i]);
    }
    write64(buffer, (uint64_t)lastWrite);

    assert(*buffer - old == stateSize());
}
template void TimeRecorded<uint16_t>::saveToBuffer(uint8_t **);
