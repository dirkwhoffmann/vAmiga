// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _AMIGA_ENUMS_H
#define _AMIGA_ENUMS_H

// Replacement for VA_ENUM which is only available on macOS
#define VA_ENUM(_type, _name) \
enum __attribute__((enum_extensibility(open))) _name : _type _name; \
enum _name : _type

/*
// Basic replacement for CF_ENUM and CF_OPTIONS for source trees that lack CoreFoundation.

#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif
#ifndef __has_extension
#define __has_extension(x) 0
#endif

#if __has_attribute(enum_extensibility)
#define __VA_ENUM_ATTRIBUTES __attribute__((enum_extensibility(open)))
#define __VA_CLOSED_ENUM_ATTRIBUTES __attribute__((enum_extensibility(closed)))
#define __VA_OPTIONS_ATTRIBUTES __attribute__((flag_enum,enum_extensibility(open)))
#else
#define __VA_ENUM_ATTRIBUTES
#define __VA_CLOSED_ENUM_ATTRIBUTES
#define __VA_OPTIONS_ATTRIBUTES
#endif

#define __VA_ENUM_GET_MACRO(_1, _2, NAME, ...) NAME
#if (__cplusplus && __cplusplus >= 201103L && (__has_extension(cxx_strong_enums) || __has_feature(objc_fixed_enum))) || (!__cplusplus && __has_feature(objc_fixed_enum))
#define __VA_NAMED_ENUM(_type, _name)     enum __VA_ENUM_ATTRIBUTES _name : _type _name; enum _name : _type
#define __VA_ANON_ENUM(_type)             enum __VA_ENUM_ATTRIBUTES : _type
#define VA_CLOSED_ENUM(_type, _name)      enum __VA_CLOSED_ENUM_ATTRIBUTES _name : _type _name; enum _name : _type
#if (__cplusplus)
#define VA_OPTIONS(_type, _name) _type _name; enum __VA_OPTIONS_ATTRIBUTES : _type
#else
#define VA_OPTIONS(_type, _name) enum __VA_OPTIONS_ATTRIBUTES _name : _type _name; enum _name : _type
#endif
#else
#define __VA_NAMED_ENUM(_type, _name) _type _name; enum
#define __VA_ANON_ENUM(_type) enum
#define VA_CLOSED_ENUM(_type, _name) _type _name; enum
#define VA_OPTIONS(_type, _name) _type _name; enum
#endif

#define VA_ENUM(...) __VA_ENUM_GET_MACRO(__VA_ARGS__, __VA_NAMED_ENUM, __VA_ANON_ENUM, )(__VA_ARGS__)

#endif
*/

#endif
