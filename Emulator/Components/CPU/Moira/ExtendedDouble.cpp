/// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "ExtendedDouble.h"
#include <iostream>

namespace vamiga::moira {

ExtendedDouble::ExtendedDouble(long double m)
{
    mantissa = m;
    exponent = 0;
    reduce();
}

std::ostream& operator<<(std::ostream& os, const ExtendedDouble& d)
{
    os << d.mantissa << "*2^" << d.exponent;
    return os;
}

bool
ExtendedDouble::operator==(const ExtendedDouble &other) const
{
    assert(isReduced());
    assert(other.isReduced());

    return mantissa == other.mantissa && exponent == other.exponent;
}

bool
ExtendedDouble::operator<(ExtendedDouble &other)
{
    assert(isReduced());
    assert(other.isReduced());

    if (mantissa == 0.0 && other.mantissa < 0.0) {
        return false;
    } if (mantissa == 0.0 && other.mantissa > 0.0) {
        return true;
    } else if (other.mantissa == 0.0 && mantissa < 0.0) {
        return true;
    } else if (other.mantissa == 0.0 && mantissa > 0.0) {
        return false;
    } else if (exponent < other.exponent) {
        return true;
    } else if (exponent > other.exponent) {
        return false;
    } else if (mantissa == other.mantissa) {
        return false;
    } else if (mantissa < other.mantissa) {
        return true;
    } else {
        return false;
    }
}

bool
ExtendedDouble::operator>(ExtendedDouble &other)
{
    return other < *this;
}

bool
ExtendedDouble::operator<(double other)
{
    assert(isReduced());

    ExtendedDouble tmp(other);
    tmp.reduce();
    return *this < tmp;
}

bool
ExtendedDouble::operator>(double other)
{
    assert(isReduced());

    ExtendedDouble tmp(other);
    tmp.reduce();
    return *this > tmp;
}

}
