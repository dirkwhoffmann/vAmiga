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
// Basic memory buffer I/O
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
    DESERIALIZE8(signed char)
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
    DESERIALIZE64(FilterType)
    DESERIALIZE64(SerialPortDevice)
    DESERIALIZE64(DriveType)
    DESERIALIZE32(DriveState)
    DESERIALIZE32(KeyboardState)

    SerReader& operator&(Event &v)
    {
        *this & v.triggerCycle & v.id & v.data;
        return *this;
    }

    template <class T, size_t N>
    SerReader& operator&(T (&v)[N])
    {
        for(size_t i = 0; i < N; ++i) {
            *this & v[i];
        }
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
    SERIALIZE8(signed char)
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
    SERIALIZE64(FilterType)
    SERIALIZE64(SerialPortDevice)
    SERIALIZE64(DriveType)
    SERIALIZE32(DriveState)
    SERIALIZE32(KeyboardState)

    SerWriter& operator&(Event &v)
    {
        *this & v.triggerCycle & v.id & v.data;
        return *this;
    }

    template <class T, size_t N>
    SerWriter& operator&(T (&v)[N])
    {
        for(size_t i = 0; i < N; ++i) {
            *this & v[i];
        }
        return *this;
    }
};


//
// Resetter
//

#define RESET(type) \
SerResetter& operator&(type& v) \
{ \
v = (type)0; \
return *this; \
}

class SerResetter
{
public:

    SerResetter()
    {
    }

    RESET(bool)
    RESET(char)
    RESET(signed char)
    RESET(unsigned char)
    RESET(short)
    RESET(unsigned short)
    RESET(int)
    RESET(unsigned int)
    RESET(long)
    RESET(unsigned long)
    RESET(long long)
    RESET(unsigned long long)
    RESET(float)
    RESET(double)
    RESET(AmigaModel)
    RESET(MemorySource)
    RESET(EventID)
    RESET(SprDMAState)
    RESET(FilterType)
    RESET(SerialPortDevice)
    RESET(DriveType)
    RESET(DriveState)
    RESET(KeyboardState)

    SerResetter& operator&(Event &v)
    {
        *this & v.triggerCycle & v.id & v.data;
        return *this;
    }

    template <class T, size_t N>
    SerResetter& operator&(T (&v)[N])
    {
        for(size_t i = 0; i < N; ++i) {
            *this & v[i];
        }
        return *this;
    }
};

#endif
