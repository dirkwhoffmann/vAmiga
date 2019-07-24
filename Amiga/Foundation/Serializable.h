// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _SERIALIZABLE_INC
#define _SERIALIZABLE_INC

template <class T>
size_t serializeToBuffer(const T& component, uint8_t *buffer) {

    printf("serializeToBuffer\n");
    return component.saveToBuffer(buffer);

    uint8_t *ptr = buffer;

    printf("    Serializing internal state ...\n");

    // Call delegation method
    ptr += component.willSaveToBuffer(ptr);

    // Save internal state of all subcomponents
    for (HardwareComponent *c : component.subComponents) {
        ptr += c->saveToBuffer(ptr);
    }

    // Save internal state of this component
    ptr += component._saveToBuffer(ptr);

    // Call delegation method
    ptr += component.didSaveToBuffer(ptr);

    // Verify that the number of written bytes matches the state size
    if (ptr - buffer != component.stateSize()) {
        printf("saveToBuffer: Snapshot size is wrong. Got %ld, expected %zu.",
              ptr - buffer, component.stateSize());
        assert(false);
    }

    return ptr - buffer;

}

class SerWriter
{
public:

    uint8_t *ptr;

    SerWriter(uint8_t *p) : ptr(p)
    {
    }

    SerWriter& operator&(uint8_t& v)
    {
        printf("SerWriter uint8_t: %u\n", v);
        ptr += sizeof(v);
        return *this;
    }

    SerWriter& operator&(uint16_t& v)
    {
        printf("SerWriter uint16_t: %u\n", v);
        ptr += sizeof(v);
        return *this;
    }

    SerWriter& operator&(uint32_t& v)
    {
        printf("SerWriter uint32_t: %u\n", v);
        ptr += sizeof(v);
        return *this;
    }

    SerWriter& operator&(uint64_t& v)
    {
        printf("SerWriter uint64_t: %llu\n", v);
        ptr += sizeof(v);
        return *this;
    }

    SerWriter& operator&(int64_t& v)
    {
        printf("SerWriter int64_t: %lld\n", v);
        ptr += sizeof(v);
        return *this;
    }

    SerWriter& operator&(bool& v)
    {
        printf("SerWriter bool: %d\n", v);
        ptr += sizeof(v);
        return *this;
    }

    SerWriter& operator&(AmigaModel& v)
    {
        printf("SerWriter AmigaModel: %ld\n", v);
        ptr += sizeof(v);
        return *this;
    }

};

#endif

