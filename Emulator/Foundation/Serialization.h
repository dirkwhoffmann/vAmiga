// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaTypes.h"
#include "Beam.h"
#include "Buffers.h"
#include "DDF.h"
#include "Event.h"
#include "Frame.h"
#include "TimeDelayed.h"
#include "Sampler.h"
#include "AudioStream.h"

#include <arpa/inet.h>

//
// Basic memory buffer I/O
//

inline u8 read8(const u8 *& buf)
{
    u8 result = R8BE(buf);
    buf += 1;
    return result;
}

inline u16 read16(const u8 *& buf)
{
    u16 result = R16BE(buf);
    buf += 2;
    return result;
}

inline u32 read32(const u8 *& buf)
{
    u32 result = R32BE(buf);
    buf += 4;
    return result;
}

inline u64 read64(const u8 *& buf)
{
    u32 hi = read32(buf);
    u32 lo = read32(buf);
    return ((u64)hi << 32) | lo;
}

inline double readDouble(const u8 *& buf)
{
    double result = 0;
    for (isize i = 0; i < 8; i++) ((u8 *)&result)[i] = read8(buf);
    return result;
}
 
inline void write8(u8 *& buf, u8 value)
{
    W8BE(buf, value);
    buf += 1;
}

inline void write16(u8 *& buf, u16 value)
{
    W16BE(buf, value);
    buf += 2;
}

inline void write32(u8 *& buf, u32 value)
{
    W32BE(buf, value);
    buf += 4;
}

inline void write64(u8 *& buf, u64 value)
{
    write32(buf, (u32)(value >> 32));
    write32(buf, (u32)(value));
}

inline void writeDouble(u8 *& buf, double value)
{
    for (isize i = 0; i < 8; i++) write8(buf, ((u8 *)&value)[i]);
}


//
// Counter (determines the state size)
//

#define COUNT(type,size) \
auto& operator&(type& v) \
{ \
count += size; \
return *this; \
}

#define COUNT8(type) static_assert(sizeof(type) == 1); COUNT(type,1)
#define COUNT16(type) static_assert(sizeof(type) == 2); COUNT(type,2)
#define COUNT64(type) static_assert(sizeof(type) <= 8); COUNT(type,8)
#define COUNTD(type) static_assert(sizeof(type) <= 8); COUNT(type,8)

#define STRUCT(type) \
auto& operator&(type& v) \
{ \
v.applyToItems(*this); \
return *this; \
}

#define __ ,

class SerCounter
{
public:

    isize count;

    SerCounter() { count = 0; }

    COUNT8(const bool)
    COUNT8(const char)
    COUNT8(const signed char)
    COUNT8(const unsigned char)
    COUNT16(const short)
    COUNT16(const unsigned short)
    COUNT64(const int)
    COUNT64(const unsigned int)
    COUNT64(const long)
    COUNT64(const unsigned long)
    COUNT64(const long long)
    COUNT64(const unsigned long long)
    COUNTD(const float)
    COUNTD(const double)
    
    COUNT64(const MemorySource)
    COUNT64(const EventID)
    COUNT8(const BusOwner)
    COUNT64(const DDFState)
    COUNT64(const SprDMAState)
    COUNT64(const SamplingMethod)
    COUNT64(const FilterType)
    COUNT64(const SerialPortDevice)
    COUNT64(const KeyboardState)
    COUNT64(const DriveType)
    COUNT64(const DriveState)
    COUNT64(const RTCRevision)
    COUNT64(const DiskDiameter)
    COUNT64(const DiskDensity)
    COUNT64(const CIARevision)
    COUNT64(const AgnusRevision)
    COUNT64(const DeniseRevision)

    STRUCT(Beam)
    STRUCT(DDF<true>)
    STRUCT(DDF<false>)
    STRUCT(Event)
    STRUCT(Frame)
    STRUCT(RegChange)
    template <class T, isize capacity> STRUCT(RingBuffer<T __ capacity>)
    template <class T, isize capacity> STRUCT(SortedRingBuffer<T __ capacity>)
    template <class T, int delay> STRUCT(TimeDelayed<T __ delay>)

    template <class T, isize N>
    SerCounter& operator&(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this & v[i];
        }
        return *this;
    }
};


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
#define DESERIALIZE64(type) static_assert(sizeof(type) <= 8); DESERIALIZE(type,read64)
#define DESERIALIZED(type) static_assert(sizeof(type) <= 8); DESERIALIZE(type,readDouble)

class SerReader
{
public:

    const u8 *ptr;

    SerReader(const u8 *p) : ptr(p)
    {
    }

    DESERIALIZE8(bool)
    DESERIALIZE8(char)
    DESERIALIZE8(signed char)
    DESERIALIZE8(unsigned char)
    DESERIALIZE16(short)
    DESERIALIZE16(unsigned short)
    DESERIALIZE64(int)
    DESERIALIZE64(unsigned int)
    DESERIALIZE64(long)
    DESERIALIZE64(unsigned long)
    DESERIALIZE64(long long)
    DESERIALIZE64(unsigned long long)
    DESERIALIZED(float)
    DESERIALIZED(double)
    
    DESERIALIZE64(MemorySource)
    DESERIALIZE64(EventID)
    DESERIALIZE8(BusOwner)
    DESERIALIZE64(DDFState)
    DESERIALIZE64(SprDMAState)
    DESERIALIZE64(SamplingMethod)
    DESERIALIZE64(FilterType)
    DESERIALIZE64(SerialPortDevice)
    DESERIALIZE64(KeyboardState)
    DESERIALIZE64(DriveType)
    DESERIALIZE64(DriveState)
    DESERIALIZE64(RTCRevision)
    DESERIALIZE64(DiskDiameter)
    DESERIALIZE64(DiskDensity)
    DESERIALIZE64(CIARevision)
    DESERIALIZE64(AgnusRevision)
    DESERIALIZE64(DeniseRevision)

    STRUCT(Beam)
    STRUCT(DDF<true>)
    STRUCT(DDF<false>)
    STRUCT(Event)
    STRUCT(Frame)
    STRUCT(RegChange)
    template <class T, isize capacity> STRUCT(RingBuffer<T __ capacity>)
    template <class T, isize capacity> STRUCT(SortedRingBuffer<T __ capacity>)
    template <class T, int delay> STRUCT(TimeDelayed<T __ delay>)

    template <class T, isize N>
    SerReader& operator&(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this & v[i];
        }
        return *this;
    }

    void copy(void *dst, isize n)
    {
        memcpy(dst, (void *)ptr, n);
        ptr += n;
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

#define SERIALIZE8(type)  static_assert(sizeof(type) == 1); SERIALIZE(type,write8,u8)
#define SERIALIZE16(type) static_assert(sizeof(type) == 2); SERIALIZE(type,write16,u16)
#define SERIALIZE64(type) static_assert(sizeof(type) <= 8); SERIALIZE(type,write64,u64)
#define SERIALIZED(type) static_assert(sizeof(type) <= 8); SERIALIZE(type,writeDouble,double)

class SerWriter
{
public:

    u8 *ptr;

    SerWriter(u8 *p) : ptr(p)
    {
    }

    SERIALIZE8(const bool)
    SERIALIZE8(const char)
    SERIALIZE8(const signed char)
    SERIALIZE8(const unsigned char)
    SERIALIZE16(const short)
    SERIALIZE16(const unsigned short)
    SERIALIZE64(const int)
    SERIALIZE64(const unsigned int)
    SERIALIZE64(const long)
    SERIALIZE64(const unsigned long)
    SERIALIZE64(const long long)
    SERIALIZE64(const unsigned long long)
    SERIALIZED(const float)
    SERIALIZED(const double)
    
    SERIALIZE64(const MemorySource)
    SERIALIZE64(const EventID)
    SERIALIZE8(const BusOwner)
    SERIALIZE64(const DDFState)
    SERIALIZE64(const SprDMAState)
    SERIALIZE64(const SamplingMethod)
    SERIALIZE64(const FilterType)
    SERIALIZE64(const SerialPortDevice)
    SERIALIZE64(const KeyboardState)
    SERIALIZE64(const DriveType)
    SERIALIZE64(const DriveState)
    SERIALIZE64(const RTCRevision)
    SERIALIZE64(const DiskDiameter)
    SERIALIZE64(const DiskDensity)
    SERIALIZE64(const CIARevision)
    SERIALIZE64(const AgnusRevision)
    SERIALIZE64(const DeniseRevision)

    STRUCT(Beam)
    STRUCT(DDF<true>)
    STRUCT(DDF<false>)
    STRUCT(Event)
    STRUCT(Frame)
    STRUCT(RegChange)
    template <class T, isize capacity> STRUCT(RingBuffer<T __ capacity>)
    template <class T, isize capacity> STRUCT(SortedRingBuffer<T __ capacity>)
    template <class T, int delay> STRUCT(TimeDelayed<T __ delay>)

    template <class T, isize N>
    SerWriter& operator&(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this & v[i];
        }
        return *this;
    }

    void copy(const void *src, isize n)
    {
        memcpy((void *)ptr, src, n);
        ptr += n;
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
    
    RESET(MemorySource)
    RESET(EventID)
    RESET(BusOwner)
    RESET(DDFState)
    RESET(SprDMAState)
    RESET(SamplingMethod)
    RESET(FilterType)
    RESET(SerialPortDevice)
    RESET(KeyboardState)
    RESET(DriveType)
    RESET(DriveState)
    RESET(RTCRevision)

    STRUCT(Beam)
    STRUCT(DDF<true>)
    STRUCT(DDF<false>)
    STRUCT(Event)
    STRUCT(Frame)
    STRUCT(RegChange)
    template <class T, isize capacity> STRUCT(RingBuffer<T __ capacity>)
    template <class T, isize capacity> STRUCT(SortedRingBuffer<T __ capacity>)
    template <class T, int delay> STRUCT(TimeDelayed<T __ delay>)

    template <class T, isize N>
    SerResetter& operator&(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this & v[i];
        }
        return *this;
    }
};
