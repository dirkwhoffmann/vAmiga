// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus

#include "BasicTypes.h"
#include <functional>
#include <map>

/* The purpose of the Reflection interface is to make the internal names of
 * an enumeration type available inside the application. I.e., it provides
 * several functions for converting enum numbers to strings and vice versa.
 *
 * Two general types on enumerations are distinguished:
 *
 * - Standard enumerations
 *
 *   The enumeration members are numbered 0, 1, 2, etc. Each member of the
 *   enumeration is treated as a stand-alone option.
 *
 * - Bit field enumerations
 *
 *   The enumeration members are numbered 1, 2, 4, etc. Each member of the
 *   enumeration is treated as flag of a combined bit field.
 */
namespace util {

#define assert_enum(e,v) assert(e##Enum::isValid(v))

template <class T, typename E> struct Reflection {

    // Returns the key as a C string
    static const char *key(long value) {

        static string result;

        result = "";
        if constexpr (T::minVal == 1) {

            // This enum is a bit field
            for (isize i = T::minVal; i <= T::maxVal; i *= 2) {
                if (value & i) result += (result.empty() ? "" : " | ") + string(T::_key((E)i));
            }

        } else {
            
            // This enum is a standard enumeration
            result = string(T::_key((E)value));
        }

        return result.c_str();
    }

    // Returns the key without the section prefix (if any)
    // TODO: Integrate into key()
    static const char *plainkey(isize nr) {

        auto *p = T::key((E)nr);
        for (isize i = 0; p[i]; i++) if (p[i] == '.') return p + i + 1;
        return p;
    }
    
    // Collects all key / value pairs
    static std::map <string,long> pairs(std::function<bool(E)> filter = [](E){ return true; }) {

        std::map <string,long> result;

        for (isize i = T::minVal; i <= T::maxVal; i++) {
            if (T::isValid(i) && filter(E(i))) result.insert(std::make_pair(key(i), i));
        }

        return result;
    }

    // Returns a list in form of a colon seperated string
    static string keyList(std::function<bool(E)> filter = [](E){ return true; }, const string &delim = ", ") {

        string result;

        for (const auto &pair : pairs(filter)) {
            result += (result.empty() ? "" : delim) + pair.first;
        }

        return result;
    }

    // Convenience wrapper
    static string argList(std::function<bool(E)> filter = [](E){ return true; }) {

        return "{ " + keyList(filter, " | ") + " }";
    }
};

}

#endif
