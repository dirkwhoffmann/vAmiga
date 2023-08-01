/// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include <cmath>

namespace vamiga::moira {

struct ExtendedDouble {

    long double mantissa;
    long exponent;


    //
    // Initializing
    //

    ExtendedDouble() : mantissa(0), exponent(0) { }
    ExtendedDouble(long double m, long e) : mantissa(m), exponent(e) { }
    ExtendedDouble(long double m);


    //
    // Printing
    //

    friend std::ostream& operator<<(std::ostream& os, const ExtendedDouble& d);


    //
    // Converting
    //

    inline double asDouble() const { return ldexp(mantissa, (int)exponent); }
    inline float asFloat() const { return (float)asDouble(); }


    //
    // Normalizing
    //

    inline void reduce() {

        int exp;
        mantissa = std::frexp(mantissa, &exp);
        exponent += exp;
    }

    bool isReduced() const {

        auto pos = std::abs(mantissa);
        return mantissa == 0.0 ? exponent == 0 : pos >= 0.5 && pos < 1.0;
    }


    //
    // Assigning
    //

    ExtendedDouble &operator=(const struct ExtendedDouble &other)
    {
        mantissa = other.mantissa;
        exponent = other.exponent;
        return *this;
    }


    //
    // Comparing
    //

    inline bool operator==(const ExtendedDouble &other) const;


    //
    // Calculating
    //

    inline ExtendedDouble &operator+=(const ExtendedDouble &other) {

        if (exponent == other.exponent) {
            mantissa += other.mantissa;
        } else if (exponent > other.exponent) {
            mantissa += other.mantissa / std::pow(2, exponent - other.exponent);
        } else {
            mantissa /= std::pow(2, other.exponent - exponent);
            exponent = other.exponent;
            mantissa += other.mantissa;
        }
        return *this;
    }

    inline ExtendedDouble &operator-=(const ExtendedDouble &other) {

        if (exponent == other.exponent) {
            mantissa -= other.mantissa;
        } else if (exponent > other.exponent) {
            mantissa -= other.mantissa / std::pow(2, exponent - other.exponent);
        } else {
            mantissa /= std::pow(2, other.exponent - exponent);
            exponent = other.exponent;
            mantissa -= other.mantissa;
        }
        return *this;
    }

    inline ExtendedDouble &operator*=(const ExtendedDouble &other) {

        mantissa *= other.mantissa;
        exponent += other.exponent;
        return *this;
    }

    inline ExtendedDouble &operator*=(double scale) {

        mantissa *= scale;
        return *this;
    }

    inline ExtendedDouble &operator/=(const ExtendedDouble &other) {

        mantissa /= other.mantissa;
        exponent -= other.exponent;
        return *this;
    }

    inline ExtendedDouble operator+(const ExtendedDouble &other) const {

        ExtendedDouble result = *this;
        result += other;
        return result;
    }

    inline ExtendedDouble operator-(const ExtendedDouble &other) const {

        ExtendedDouble result = *this;
        result -= other;
        return result;
    }

    inline ExtendedDouble operator*(const ExtendedDouble &other) const {

        ExtendedDouble result = *this;
        result *= other;
        return result;
    }

    inline ExtendedDouble operator*(double scale) const {

        ExtendedDouble result = *this;
        result *= scale;
        return result;
    }

    inline ExtendedDouble operator/(const ExtendedDouble &other) const {

        ExtendedDouble result = *this;
        result /= other;
        return result;
    }

    inline ExtendedDouble abs() const {

        ExtendedDouble result = *this;
        result.mantissa = std::fabs(result.mantissa);
        return result;
    }

    inline ExtendedDouble reciprocal() const {

        ExtendedDouble result = ExtendedDouble { 1.0 / mantissa, -exponent };
        return result;
    }

    inline ExtendedDouble log() const {

        ExtendedDouble result = ExtendedDouble(std::log(mantissa) + exponent * std::log(2));
        return result;
    }

    inline ExtendedDouble log2() const {

        ExtendedDouble result = ExtendedDouble(std::log2(mantissa) + exponent);
        return result;
    }

    inline ExtendedDouble log10() const {

        ExtendedDouble result = ExtendedDouble(std::log10(mantissa) + exponent * std::log10(2));
        return result;
    }

    static inline ExtendedDouble edpow(double base, long exp) {

        double newexp = exp * std::log2(base);

        ExtendedDouble result;
        result.exponent = long(std::floor(newexp));
        result.mantissa = std::powl(2, newexp - result.exponent);
        result.reduce();

        return result;
    }

    inline ExtendedDouble edexp(long *exp) {

        *exp = (mantissa == 0) ? 0 : 1 + (long)std::floor(abs().log10().asDouble());
        return *this * ExtendedDouble::edpow(10, -(*exp));
    }

    //
    // Comparing
    //

    bool operator<(ExtendedDouble &other);
    bool operator>(ExtendedDouble &other);
    bool operator<(double other);
    bool operator>(double other);
};

}
