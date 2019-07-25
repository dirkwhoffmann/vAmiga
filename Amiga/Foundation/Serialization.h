// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _SERIALIZATION_INC
#define _SERIALIZATION_INC

//
// Basic I/O
//

inline uint8_t read8(uint8_t *& buffer)
{
    uint8_t result = *buffer;
    buffer += 1;
    return result;
}

inline uint16_t read16(uint8_t *& buffer)
{
    uint16_t result = ntohs(*((uint16_t *)buffer));
    buffer += 2;
    return result;
}

inline uint32_t read32(uint8_t *& buffer)
{
    uint32_t result = ntohl(*((uint32_t *)buffer));
    buffer += 4;
    return result;
}

inline uint64_t read64(uint8_t *& buffer)
{
    uint32_t hi = read32(buffer);
    uint32_t lo = read32(buffer);
    return ((uint64_t)hi << 32) | lo;
}

inline void write8(uint8_t *& buffer, uint8_t value)
{
    *buffer = value;
    buffer += 1;
}

inline void write16(uint8_t *& buffer, uint16_t value)
{
    *((uint16_t *)buffer) = htons(value);
    buffer += 2;
}

inline void write32(uint8_t *& buffer, uint32_t value)
{
    *((uint32_t *)buffer) = htonl(value);
    buffer += 4;
}

inline void write64(uint8_t *& buffer, uint64_t value)
{
    write32(buffer, (uint32_t)(value >> 32));
    write32(buffer, (uint32_t)(value));
}

//
// Deserializing a hardware component
//

template <class T>
size_t deserializeFromBuffer(T& component, uint8_t *buffer)
{
    printf("deserializeFromBuffer\n");

    uint8_t *ptr = buffer;

    // Call delegation method
    ptr += component.willLoadFromBuffer(ptr);

    // Load internal state of all subcomponents
    for (HardwareComponent *c : component.subComponents) {
        ptr += c->loadFromBuffer(ptr);
    }

    // Load internal state of this component
    ptr += component._loadFromBuffer(ptr);

    // Call delegation method
    ptr += component.didLoadFromBuffer(ptr);

    // Verify that the number of processed bytes matches the state size
    if (ptr - buffer != component.stateSize()) {
        printf("loadFromBuffer: Snapshot size is wrong. Got %ld, expected %zu.",
              ptr - buffer, component.stateSize());
        assert(false);
    }

    return ptr - buffer;
}


//
// Serialize a hardware component
//

template <class T>
size_t serializeToBuffer(T& component, uint8_t *buffer)
{
    printf("serializeToBuffer\n");

    uint8_t *ptr = buffer;

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


//
// Reader (Deserializer)
//

#define DESERIALIZE(type,function) \
SerReader& operator&(type& v) \
{ \
v = (type)function(ptr); \
return *this; \
}

#define DESERIALIZE8(type)  static_assert(sizeof(type) == 1); DESERIALIZE(type,read8)
#define DESERIALIZE16(type) static_assert(sizeof(type) == 2); DESERIALIZE(type,read16)
#define DESERIALIZE32(type) static_assert(sizeof(type) == 4); DESERIALIZE(type,read32)
#define DESERIALIZE64(type) static_assert(sizeof(type) == 8); DESERIALIZE(type,read64)

class SerReader
{
public:

    uint8_t *ptr;

    SerReader(uint8_t *p) : ptr(p)
    {
    }

    DESERIALIZE8(bool)
    DESERIALIZE8(char)
    DESERIALIZE8(unsigned char)
    DESERIALIZE16(short)
    DESERIALIZE16(unsigned short)
    DESERIALIZE32(int)
    DESERIALIZE32(unsigned int)
    DESERIALIZE64(long)
    DESERIALIZE64(unsigned long)
    DESERIALIZE64(long long)
    DESERIALIZE64(unsigned long long)
    DESERIALIZE32(float)
    DESERIALIZE64(double)
    DESERIALIZE64(AmigaModel)
    DESERIALIZE32(MemorySource)
    DESERIALIZE64(EventID)
    DESERIALIZE32(SprDMAState)

    SerReader& operator&(Event &v)
    {
        *this & v.triggerCycle & v.id & v.data;
        return *this;
    }

    template <class T, size_t N>
    SerReader& operator&(T (&v)[N])
    {
        for(size_t i = 0; i < N; ++i)
            *this & v[i];
        return *this;
    }
};


//
// Writer (Serializer)
//

#define SERIALIZE(type,function,cast) \
SerWriter& operator&(type& v) \
{ \
function(ptr, (cast)v); \
return *this; \
}

#define SERIALIZE8(type)  static_assert(sizeof(type) == 1); SERIALIZE(type,write8,uint8_t)
#define SERIALIZE16(type) static_assert(sizeof(type) == 2); SERIALIZE(type,write16,uint16_t)
#define SERIALIZE32(type) static_assert(sizeof(type) == 4); SERIALIZE(type,write32,uint32_t)
#define SERIALIZE64(type) static_assert(sizeof(type) == 8); SERIALIZE(type,write64,uint64_t)

class SerWriter
{
public:

    uint8_t *ptr;

    SerWriter(uint8_t *p) : ptr(p)
    {
    }

    SERIALIZE8(bool)
    SERIALIZE8(char)
    SERIALIZE8(unsigned char)
    SERIALIZE16(short)
    SERIALIZE16(unsigned short)
    SERIALIZE32(int)
    SERIALIZE32(unsigned int)
    SERIALIZE64(long)
    SERIALIZE64(unsigned long)
    SERIALIZE64(long long)
    SERIALIZE64(unsigned long long)
    SERIALIZE32(float)
    SERIALIZE64(double)
    SERIALIZE64(AmigaModel)
    SERIALIZE32(MemorySource)
    SERIALIZE64(EventID)
    SERIALIZE32(SprDMAState)

    SerWriter& operator&(Event &v)
    {
        *this & v.triggerCycle & v.id & v.data;
        return *this;
    }

    template <class T, size_t N>
    SerWriter& operator&(T (&v)[N])
    {
        // uint32_t len = N;
        // *this & len;
        for(size_t i = 0; i < N; ++i)
            *this & v[i];
        return *this;
    }
};


class SerWalker
{

public:

    template <class T>
    SerWalker& operator&(T &v)
    {
        v.applyToSubComponents(*this);
        printf("Serial Walker\n");
        return *this;
    }

};

#endif
