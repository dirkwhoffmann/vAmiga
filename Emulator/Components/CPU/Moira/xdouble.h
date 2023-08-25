// -----------------------------------------------------------------------------
// double-double - A lightweight double-double arithmetic library written in C++
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//         THIS IS A BETA-VERSION. THE CODE IS NOT PRODUCTION-READY
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Double-double arithmetic increases the precision of floating point operations
// by representing numbers as an unevaluated sum of two floating point numbers
// of lower precision. This results in a floating point format with almost
// twice the precision of the base format without sacrificing speed too much.
//
// This library is a lightweight implementation of double-double arithetics
// written in C++20.
//
// Highlights:
//
//   - Header-only library.
//
//   - Interchangable base types due to a template-based design.
//
// References:
//
//   - Hida, Y., Li, X. S., Bailey, D. H. (2000), "Quad-Double Arithmetic:
//     Algorithms, Implementation, and Application", Technical Report LBNL-46996
//
//   - double-double Python library by Juraj Sukop
//     https://github.com/sukop/doubledouble/tree/master
//
//   - Fork of QD (2.3.17) by scibuilder
//     https://github.com/scibuilder/QD
//
//   - David H. Bailey: High-Precision Software Directory
//     https://www.davidhbailey.com/dhbsoftware
// -----------------------------------------------------------------------------

#pragma once

#include <cfenv>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <climits>
#include <string>
#ifdef DBL_DEBUG
#include "gmpxx.h"
#endif

namespace xdb {

using std::exp;
using std::frexp;
using std::ldexp;
using std::log;
using std::log10;
using std::modf;
using std::exp2;
using std::log2;

using std::pow;
using std::sqrt;

using std::ceil;
using std::floor;
using std::fmod;
using std::trunc;
using std::round;
using std::lround;
using std::llround;
using std::rint;
using std::lrint;
using std::llrint;
using std::nearbyint;

using std::copysign;
using std::nan;
using std::fdim;
using std::fmax;
using std::fmin;

using std::fabs;
using std::abs;
using std::fma;

using std::isfinite;
using std::isinf;
using std::isnan;
using std::isnormal;
using std::signbit;

using std::to_string;

template <class T> struct XDouble;

template <class T> XDouble<T> exp(const XDouble<T> &op);
template <class T> XDouble<T> frexp(const XDouble<T> &op, int *exp);
template <class T> XDouble<T> frexp10(const XDouble<T> &op, int *exp);
template <class T> XDouble<T> ldexp(const XDouble<T> &op, int exp);
template <class T> XDouble<T> ldexp10(const XDouble<T> &op, int exp);
template <class T> XDouble<T> log(const XDouble<T> &op);
template <class T> XDouble<T> log10(const XDouble<T> &op);
template <class T> XDouble<T> modf(const XDouble<T> &op, XDouble<T> *iptr);
template <class T> XDouble<T> exp2(const XDouble<T> &op);
template <class T> XDouble<T> log2(const XDouble<T> &op);

template <class T> XDouble<T> powd(const XDouble<T> &base, int exponent);
template <class T> XDouble<T> pow(const XDouble<T> &base, const XDouble<T> &exponent);
template <class T> XDouble<T> sqr(const XDouble<T> &x);
template <class T> XDouble<T> sqrt(const XDouble<T> &x);

template <class T> XDouble<T> ceil(const XDouble<T> &x);
template <class T> XDouble<T> ceil(const XDouble<T> &x, int fracdigits);
template <class T> XDouble<T> floor(const XDouble<T> &x);
template <class T> XDouble<T> floor(const XDouble<T> &x, int fracdigits);
template <class T> XDouble<T> fmod(const XDouble<T> &numer, const XDouble<T> &denom);
template <class T> XDouble<T> trunc(const XDouble<T> &x);
template <class T> XDouble<T> trunc(const XDouble<T> &x, int fracdigits);
template <class T> XDouble<T> round(const XDouble<T> &x);
template <class T> XDouble<T> round(const XDouble<T> &x, int fracdigits);
template <class T> XDouble<T> roundEven(const XDouble<T> &x);
template <class T> XDouble<T> roundEven(const XDouble<T> &x, int fracdigits);
template <class T> long lround(const XDouble<T> &x);
template <class T> long long llround(const XDouble<T> &x);
template <class T> XDouble<T> rint(const XDouble<T> &x);
template <class T> XDouble<T> rint(const XDouble<T> &x, int fracdigits);
template <class T> long lrint(const XDouble<T> &x);
template <class T> long long llrint(const XDouble<T> &x);
template <class T> XDouble<T> nearbyint(const XDouble<T> &x);
template <class T> XDouble<T> nearbyint(const XDouble<T> &x, int fracdigits);

template <class T>  XDouble<T> copysign(const XDouble<T> &x, const XDouble<T> &y);
template <class T> XDouble<T> fdim(const XDouble<T> &x, const XDouble<T> &y);
template <class T> XDouble<T> fmax(const XDouble<T> &x, const XDouble<T> &y);
template <class T> XDouble<T> fmin(const XDouble<T> &x, const XDouble<T> &y);

template <class T> XDouble<T> fabs(const XDouble<T> &x);
template <class T> XDouble<T> abs(const XDouble<T> &x);
template <class T> XDouble<T> fma(const XDouble<T> &x, const XDouble<T> &y, const XDouble<T> &z);

template <class T> bool isfinite(const XDouble<T> &x);
template <class T> bool isinf(const XDouble<T> &x);
template <class T> bool isposinf(const XDouble<T> &x);
template <class T> bool isminusinf(const XDouble<T> &x);
template <class T> bool isnan(const XDouble<T> &x);
template <class T> bool isposnan(const XDouble<T> &x);
template <class T> bool isnegnan(const XDouble<T> &x);
template <class T> bool isnormal(const XDouble<T> &x);
template <class T> bool signbit(const XDouble<T> &x);

template <class T> bool isinteger(const XDouble<T> &x);
template <class T> bool isoddinteger(const XDouble<T> &x);
template <class T> bool iseveninteger(const XDouble<T> &x);
template <class T> bool iszero(const XDouble<T> &x);
template <class T> bool isposzero(const XDouble<T> &x);
template <class T> bool isminzero(const XDouble<T> &x);
template <class T> bool isone(const XDouble<T> &x);
template <class T> bool ispositive(const XDouble<T> &x);
template <class T> bool isnegative(const XDouble<T> &x);

template <class T> std::string to_string(const XDouble<T> &x);
template <class T> std::string to_string(const XDouble<T> &x, int rdigits);
template <class T> std::string to_string(const XDouble<T> &x, int ldigits, int rdigits);

template <class T> struct XDouble {

    constexpr static bool useFma = true;

    // High and low parts of the represented value
    T h, l;


    //
    // Constructors
    //

    XDouble() noexcept : h(0), l(0) { }
    XDouble(T h, T l) noexcept : h(h), l(l) { }
    XDouble(long double h) noexcept : XDouble(T(h), T(0)) { }
    XDouble(const XDouble<T>& other) noexcept : h(other.h), l(other.l) { }
    XDouble(XDouble<T>&& other) noexcept : XDouble() { swap(*this, other); }

    XDouble(const std::string &s) noexcept : XDouble()
    {
        std::string ls = s;
        std::string rs = "";

        if (auto pos = s.find("."); pos != std::string::npos) {

            ls = s.substr(0, pos);
            rs = s.substr(pos + 1, std::string::npos);
        }

        *this = XDouble(ls, rs);
    }

    XDouble(const std::string &ls, const std::string &rs) noexcept : XDouble()
    {
        XDouble<T> lval, rval;

        // Check if the number is positive or negative
        bool neg = ls.empty() ? false : ls[0] == '-';

        // Parse the integral digits
        for (auto it = ls.begin() ; it != ls.end(); ++it) {
            if (*it >= '0' && *it <= '9') lval = lval * XDouble<T>(10) + XDouble<T>(*it - '0');
        }

        // Parse the fractional digits (in reverse order)
        for (auto it = rs.crbegin() ; it != rs.crend(); ++it) {
            if (*it >= '0' && *it <= '9') rval = (rval + XDouble<T>(*it - '0')) / XDouble<T>(10);
        }

        // Assemble the result
        *this = neg ? -(lval + rval) : (lval + rval);
    }

    friend void swap(XDouble<T>& first, XDouble<T>& second) noexcept
    {
        std::swap(first.h, second.h);
        std::swap(first.l, second.l);
    }


    //
    // Constants
    //

    static const XDouble<T> e;
    static const XDouble<T> log2e;
    static const XDouble<T> log10e;
    static const XDouble<T> pi;
    static const XDouble<T> inv_pi;
    static const XDouble<T> inv_sqrtpi;
    static const XDouble<T> ln2;
    static const XDouble<T> ln10;
    static const XDouble<T> sqrt2;
    static const XDouble<T> sqrt3;
    static const XDouble<T> inv_sqrt3;
    static const XDouble<T> egamma;
    static const XDouble<T> phi;

    static XDouble<T> nan() { return XDouble<T>(std::numeric_limits<double>::quiet_NaN()); }
    static XDouble<T> posnan() { return nan(); }
    static XDouble<T> negnan() { return -nan(); }

    static XDouble<T> inf() { return XDouble<T>(std::numeric_limits<double>::infinity()); }
    static XDouble<T> posinf() { return inf(); }
    static XDouble<T> neginf() { return -inf(); }


    static int digits()
    {
        if constexpr (std::is_same<T, float>::value) {
            return std::numeric_limits<float>::digits/ 2 + 1;
        } else if constexpr (std::is_same<T, double>::value) {
            return std::numeric_limits<double>::digits/ 2 + 1;
        } else if constexpr (std::is_same<T, long double>::value) {
            return std::numeric_limits<long double>::digits/ 2 + 1;
        } else {
            return T::digits() - 3;
        }
    }


    //
    // Assignment operators
    //

    XDouble<T>& operator=(XDouble<T> rhs)
    {
        swap(*this, rhs);
        return *this;
    }


    //
    // Cast operators
    //

    operator int() const { return to_int(); }
    operator long() const { return to_long(); }
    operator long long() const { return to_long_long(); }
    operator unsigned long long() const { return to_ulong_long(); }
    operator float() const { return to_float(); }
    operator double() const { return to_double(); }
    operator long double() const { return to_long_double(); }


    //
    // Conversion functions
    //

    int to_int() const
    {
        if (isfinite()) return static_cast<int>(to_long_double());
        return (int)h;
    }

    long to_long() const
    {
        if (isfinite()) return static_cast<long>(to_long_double());
        return (long)h;
    }

    long long to_long_long() const
    {
        if (isfinite()) return static_cast<long long>(to_long_double());
        return (long long)h;
    }

    long long to_ulong_long() const
    {
        printf("Casting %f to %lldu\n", to_double(), static_cast<unsigned long long>(to_long_double()));
        if (isfinite()) return static_cast<unsigned long long>(to_long_double());
        return (unsigned long long)h;
    }

    float to_float() const
    {
        return static_cast<float>(to_long_double());
    }

    double to_double() const
    {
        return static_cast<double>(to_long_double());
    }

    long double to_long_double() const
    {
        if (!isfinite() || iszero()) return (long double)h;
        return static_cast<long double>(h) + static_cast<long double>(l);
    }

#ifdef DBL_DEBUG
    mpf_class to_mpf() const
    {
        constexpr int precision = 256;

        if constexpr (std::is_same<T, float>::value) {
            return mpf_class(h, precision) + mpf_class(l, precision);
        } else if constexpr (std::is_same<T, double>::value) {
            return mpf_class(h, precision) + mpf_class(l, precision);
        } else {
            return h.to_mpf() + l.to_mpf();
        }
    }
#endif

    char to_character() const
    {
        auto digit = round().to_long() + '0';
        return digit >= '0' && digit <= '9' ? char(digit) : '?';
    }

    std::string to_string() const { return xdb::to_string(*this); }
    std::string to_string(int rdigits) const { return xdb::to_string(*this, rdigits); }
    std::string to_string(int ldigits, int rdigits) const { return xdb::to_string(*this, ldigits, rdigits); }


    //
    // Comparison operators
    //

    bool operator==(const XDouble<T>& rhs) const
    {
        if (isfinite() && rhs.isfinite()) {
            return l == rhs.l && h == rhs.h;
        } else {
            return h == rhs.h;
        }
    }
    bool operator==(double rhs) const { return *this == XDouble<T>(rhs); }
    bool operator==(long double rhs) const { return *this == XDouble<T>(rhs); }

    bool operator!=(const XDouble<T>& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator!=(double rhs) const { return *this != XDouble<T>(rhs); }
    bool operator!=(long double rhs) const { return *this != XDouble<T>(rhs); }

    bool operator<(const XDouble<T>& rhs) const
    {
        if (isfinite() && rhs.isfinite()) {
            return h < rhs.h || (h == rhs.h && l < rhs.l);
        } else {
            return h < rhs.h;
        }
    }
    bool operator<(double rhs) const { return *this < XDouble<T>(rhs); }
    bool operator<(long double rhs) const { return *this < XDouble<T>(rhs); }

    bool operator>(const XDouble<T>& rhs) const
    {
        if (isfinite() && rhs.isfinite()) {
            return h > rhs.h || (h == rhs.h && l > rhs.l);
        } else {
            return h > rhs.h;
        }
    }
    bool operator>(double rhs) const { return *this > XDouble<T>(rhs); }
    bool operator>(long double rhs) const { return *this > XDouble<T>(rhs); }

    bool operator<=(const XDouble<T>& rhs) const
    {
        if (isfinite() && rhs.isfinite()) {
            return !(*this > rhs);
        } else {
            return h <= rhs.h;
        }
    }
    bool operator<=(double rhs) const { return *this <= XDouble<T>(rhs); }
    bool operator<=(long double rhs) const { return *this <= XDouble<T>(rhs); }

    bool operator>=(const XDouble<T>& rhs) const
    {
        if (isfinite() && rhs.isfinite()) {
            return !(*this < rhs);
        } else {
            return h >= rhs.h;
        }
    }
    bool operator>=(double rhs) const { return *this >= XDouble<T>(rhs); }
    bool operator>=(long double rhs) const { return *this >= XDouble<T>(rhs); }


    //
    // Basic arithmetic functions
    //

    static XDouble<T> quickTwoSum(T a, T b) {

        T s = a + b;
        T err = b - (s - a);

        return XDouble<T>(s, err);
    }

    static XDouble<T> twoSum(T a, T b) {

        T s = a + b;
        T v = s - a;
        T err = (a - (s - v)) + (b - v);

        return XDouble<T>(s, err);
    }

    static XDouble<T> split(T a) {

        T t = (xdb::ldexp(T(1), digits()) + T(1)) * a;
        T h = t - (t - a);
        T l = a - h;

        return XDouble<T>(h, l);
    }

    static XDouble<T> twoProd(T a, T b) {

        if constexpr (useFma) {

            auto p =  a * b;
            auto err = xdb::fma(a, b, -p);

            return XDouble<T>(p, err);

        } else {

            auto p =  a * b;
            auto aa = split(a);
            auto bb = split(b);
            auto err = ((aa.h * bb.h - p) + aa.h * bb.l + aa.l * bb.h) + aa.l * bb.l;

            return XDouble<T>(p, err);
        }
    }

    XDouble<T> operator-() const
    {
        return XDouble<T>(-h, -l);
    }

    XDouble<T> &operator+=(const XDouble<T> &rhs)
    {
        if (isfinite() && rhs.isfinite()) {

            auto sum = twoSum(h, rhs.h);
            sum.l += l + rhs.l;
            *this = quickTwoSum(sum.h, sum.l);

        } else {

            *this = XDouble<T>(h + rhs.h);
        }

        return *this;
    }

    XDouble<T> operator+(const XDouble<T> &rhs) const
    {
        XDouble<T> result(*this);
        result += rhs;

        return result;
    }

    friend XDouble<T> operator+(double lhs, XDouble<T> rhs) { return XDouble<T>(lhs) + rhs; }
    friend XDouble<T> operator+(long double lhs, XDouble<T> rhs) { return XDouble<T>(lhs) + rhs; }
    friend XDouble<T> operator+(XDouble<T> lhs, double rhs) { return lhs + XDouble<T>(rhs); }
    friend XDouble<T> operator+(XDouble<T> lhs, long double rhs) { return lhs + XDouble<T>(rhs); }

    XDouble<T> &operator-=(const XDouble<T> &rhs)
    {
        if (isfinite() && rhs.isfinite()) {

            *this += -rhs;

        } else {

            *this = XDouble<T>(h - rhs.h);
        }
        return *this;
    }

    XDouble<T> operator-(const XDouble<T> &rhs) const
    {
        XDouble<T> result(*this);
        result -= rhs;

        return result;
    }

    friend XDouble<T> operator-(double lhs, XDouble<T> rhs) { return XDouble<T>(lhs) - rhs; }
    friend XDouble<T> operator-(long double lhs, XDouble<T> rhs) { return XDouble<T>(lhs) - rhs; }
    friend XDouble<T> operator-(XDouble<T> lhs, double rhs) { return lhs - XDouble<T>(rhs); }
    friend XDouble<T> operator-(XDouble<T> lhs, long double rhs) { return lhs - XDouble<T>(rhs); }

    XDouble<T> &operator*=(const XDouble<T> &rhs)
    {
        if (isfinite() && rhs.isfinite()) {

            auto val = twoProd(h, rhs.h);
            val.l += h * rhs.l + l * rhs.h;
            *this = quickTwoSum(val.h, val.l);

        } else {

            *this = XDouble<T>(h * rhs.h);
        }

        return *this;
    }

    XDouble<T> operator*(const XDouble<T> &rhs) const
    {
        XDouble<T> result(*this);
        result *= rhs;

        return result;
    }

    friend XDouble<T> operator*(double lhs, XDouble<T> rhs) { return XDouble<T>(lhs) * rhs; }
    friend XDouble<T> operator*(long double lhs, XDouble<T> rhs) { return XDouble<T>(lhs) * rhs; }
    friend XDouble<T> operator*(XDouble<T> lhs, double rhs) { return lhs * XDouble<T>(rhs); }
    friend XDouble<T> operator*(XDouble<T> lhs, long double rhs) { return lhs * XDouble<T>(rhs); }

    XDouble<T> &operator/=(const XDouble<T> &rhs)
    {
        if (isfinite() && !iszero() && rhs.isfinite() && !rhs.iszero()) {

            XDouble<T> r = *this;
            T q1 = r.h / rhs.h;

            r -= XDouble<T>(q1) * rhs;
            T q2 = r.h / rhs.h;

            r -= XDouble<T>(q2) * rhs;
            T q3 = r.h / rhs.h;

            *this = XDouble<T>(q1) + XDouble<T>(q2) + XDouble<T>(q3);

        } else {

            *this = XDouble<T>(h / rhs.h);
        }

        return *this;
    }

    XDouble<T> operator/(const XDouble<T> &rhs) const
    {
        XDouble<T> result(*this);
        result /= rhs;

        return result;
    }

    friend XDouble<T> operator/(double lhs, XDouble<T> rhs) { return XDouble<T>(lhs) / rhs; }
    friend XDouble<T> operator/(long double lhs, XDouble<T> rhs) { return XDouble<T>(lhs) / rhs; }
    friend XDouble<T> operator/(XDouble<T> lhs, double rhs) { return lhs / XDouble<T>(rhs); }
    friend XDouble<T> operator/(XDouble<T> lhs, long double rhs) { return lhs / XDouble<T>(rhs); }

    void operator++() { *this += 1.0; }
    void operator++(int) { *this += 1.0; }
    void operator--() { *this -= 1.0; }
    void operator--(int) { *this -= 1.0; }


    //
    // Trigonometric functions
    //

    // Not implemented, yet


    //
    // Exponential and logarithmic functions
    //

    XDouble<T> exp() const { return xdb::exp(*this); }
    XDouble<T> frexp(int *exp) const { return xdb::frexp(*this, exp); }
    XDouble<T> frexp10(int *exp) const { return xdb::frexp10(*this, exp); }
    XDouble<T> ldexp(int exp) const { return xdb::ldexp(*this, exp); }
    XDouble<T> ldexp10(int exp) const { return xdb::ldexp10(*this, exp); }
    XDouble<T> log() const { return xdb::log(*this); }
    XDouble<T> log10() const  { return xdb::log10(*this); }
    XDouble<T> modf(XDouble<T> *iptr) const { return xdb::modf(*this, iptr); }
    XDouble<T> exp2() const { return xdb::exp2(*this); }
    XDouble<T> log2() const { return xdb::log2(*this); }


    //
    // Power functions
    //

    XDouble<T> powd(int exponent) const { return xdb::powd(*this, exponent); }
    XDouble<T> pow(const XDouble<T> &exponent) const { return xdb::pow(*this, exponent); }
    XDouble<T> sqr() const { return xdb::sqr(*this); }
    XDouble<T> sqrt() const { return xdb::sqrt(*this); }


    //
    // Error and gamma functions
    //

    // Not implemented


    //
    // Rounding and remainder functions
    //

    XDouble<T> ceil() const { return xdb::ceil(*this); }
    XDouble<T> ceil(int fracdigits) const { return xdb::ceil(*this, fracdigits); }
    XDouble<T> floor() const { return xdb::floor(*this); }
    XDouble<T> floor(int fracdigits) const { return xdb::floor(*this, fracdigits); }
    XDouble<T> fmod(XDouble<T> denom) const { return xdb::fmod(*this, denom); }
    XDouble<T> trunc() const { return xdb::trunc(*this); }
    XDouble<T> trunc(int fracdigits) const { return xdb::trunc(*this, fracdigits); }
    XDouble<T> round() const { return xdb::round(*this); }
    XDouble<T> round(int fracdigits) const { return xdb::round(*this, fracdigits); }
    XDouble<T> roundEven() const { return xdb::roundEven(*this); }
    XDouble<T> roundEven(int fracdigits) const  { return xdb::roundEven(*this, fracdigits); }
    long lround() const { return xdb::lround(*this); }
    long long llround() const { return xdb::llround(*this); }
    XDouble<T> rint() const { return xdb::rint(*this); }
    XDouble<T> rint(int fracdigits) const { return xdb::rint(*this, fracdigits); }
    long lrint() const { return xdb::lrint(*this); }
    long long llrint() const { return xdb::llrint(*this); }
    XDouble<T> nearbyint() const { return xdb::nearbyint(*this); }
    XDouble<T> nearbyint(int fracdigits) const { return xdb::nearbyint(*this, fracdigits); }


    //
    // Floating-point manipulation functions
    //

    XDouble<T> copysign(const XDouble<T> &y) const { return xdb::copysign(*this, y); }


    //
    // Minimum, maximum, difference functions
    //

    XDouble<T> fdim(const XDouble<T> &y) const { return xdb::fdim(*this, y); }
    XDouble<T> fmax(const XDouble<T> &y) const { return xdb::fmax(*this, y); }
    XDouble<T> fmin(const XDouble<T> &y) const { return xdb::fmin(*this, y); }


    //
    // Other functions
    //

    XDouble<T> fabs() const { return xdb::fabs(*this); }
    XDouble<T> abs() const { return xdb::abs(*this); }


    //
    // Classification functions
    //

    bool isfinite() const { return xdb::isfinite(*this); }
    bool isinf() const { return xdb::isinf(*this); }
    bool isposinf() const { return xdb::isposinf(*this); }
    bool isneginf() const { return xdb::isminusinf(*this); }
    bool isnan() const { return xdb::isnan(*this); }
    bool isposnan() const { return xdb::isposnan(*this); }
    bool isnegnan() const { return xdb::isnegnan(*this); }
    bool isnormal() const { return xdb::isnormal(*this); }
    bool signbit() const { return xdb::signbit(*this); }

    bool isinteger() const { return xdb::isinteger(*this); }
    bool isoddinteger() const { return xdb::isoddinteger(*this); }
    bool iseveninteger() const { return xdb::iseveninteger(*this); }
    bool iszero() const { return xdb::iszero(*this); }
    bool isposzero() const { return xdb::isposzero(*this); }
    bool isnegzero() const { return xdb::isminzero(*this); }
    bool isone() const { return xdb::isone(*this); }
    bool ispositive() const { return xdb::ispositive(*this); }
    bool isnegative() const { return xdb::isnegative(*this); }
};


//
// Conversion functions
//

template <class T> inline std::string
to_string(const XDouble<T> &x) {
    return to_string(x, 128, 8);
}

template <class T> inline std::string
to_string(const XDouble<T> &x, int rdigits) {
    return to_string(x, 128, rdigits);
}

template <class T> inline std::string
to_string(const XDouble<T> &x, int ldigits, int rdigits)
{
    std::string result;

    // Catch NaN and infinity
    if (!x.isfinite()) return xdb::to_string(x.h);

    // Split number into its integral (l) and fractional (r) part
    XDouble<T> l; XDouble<T> r = x.abs().modf(&l);

    // Compute integral digits
    for (int i = 0; i < ldigits; i++) {

        l /= 10.0;
        XDouble<T> digit = l.modf(&l) * 10.0;
        result = digit.to_character() + result;
        if (l.abs() < 1.0) break;
    }

    // Create a decimal point if necessary
    if (rdigits) result += '.';

    // Compute fractional digits
    for (int i = 0; i < rdigits; i++) {

        r *= 10.0;
        XDouble<T> digit; r = r.modf(&digit);
        result = result + digit.to_character();
    }

    return (x.isnegative() ? "-" : "") + result;
}


//
// Exponential and logarithmic functions
//

template <class T> inline XDouble<T>
exp(const XDouble<T> &op)
{
    if (!op.isfinite()) return XDouble<T>(xdb::exp(op.h));

    auto n = xdb::round(op.h);
    auto w = op - XDouble<T>(n);

    auto u = (((((((((((w +
                        156.0) * w + 12012.0) * w +
                      600600.0) * w + 21621600.0) * w +
                    588107520.0) * w + 12350257920.0) * w +
                  201132771840.0) * w + 2514159648000.0) * w +
                23465490048000.0) * w + 154872234316800.0) * w +
              647647525324800.0) * w + 1295295050649600.0;

    auto v = (((((((((((w -
                        156.0) * w + 12012.0) * w -
                      600600.0) * w + 21621600.0) * w -
                    588107520.0) * w + 12350257920.0) * w -
                  201132771840.0) * w + 2514159648000.0) * w -
                23465490048000.0) * w + 154872234316800.0) * w -
              647647525324800.0) * w + 1295295050649600.0;

    return XDouble<T>::e.powd((int)n) * (u / v);
}

template <class T> inline XDouble<T>
frexp(const XDouble<T> &op, int *exp)
{
    if (!op.isfinite()) return XDouble<T>(xdb::frexp(op.h, exp));

    auto r = xdb::frexp(op.h, exp);
    auto e = xdb::ldexp(op.l, -(*exp));

    return XDouble<T>(r, e);
}

template <class T> inline XDouble<T>
frexp10(const XDouble<T> &op, int *exp)
{
    if (!op.isfinite()) return XDouble<T>(xdb::frexp(op.h, exp));

    *exp = op.iszero() ? 0 : 1 + (op.fabs().log10().floor().to_int());
    return op * XDouble<T>(10).pow(-(*exp));
}

template <class T> inline XDouble<T>
ldexp(const XDouble<T> &op, int exp)
{
    if (!op.isfinite()) return XDouble<T>(xdb::ldexp(op.h, exp));

    return XDouble<T>(xdb::ldexp(op.h, exp), xdb::ldexp(op.l, exp));
}

template <class T> inline XDouble<T>
ldexp10(const XDouble<T> &op, int exp)
{
    if (!op.isfinite()) return XDouble<T>(xdb::ldexp(op.h, exp));

    return op * XDouble<T>(10).pow(exp);
}

template <class T> inline XDouble<T>
log(const XDouble<T> &op)
{
    if (!op.isfinite() || op.isnegative() || op.iszero()) return XDouble<T>(xdb::log(op.h));

    auto r = XDouble<T>(xdb::log(op.h));
    auto u = r.exp();
    r -= 2.0 * (u - op) / (u + op);

    return r;
}

template <class T> inline XDouble<T>
log10(const XDouble<T> &op)
{
    if (!op.isfinite() || op.isnegative() || op.iszero()) return XDouble<T>(xdb::log10(op.h));

    return op.log() / XDouble<T>::ln10;
}

template <class T> inline XDouble<T>
modf(const XDouble<T> &op, XDouble<T> *iptr)
{
    auto nearby = op.trunc();
    auto result = (op.isinf() ? XDouble<T>(0.0) : op - nearby).copysign(op);

    *iptr = nearby;
    return result;
}

template <class T> inline XDouble<T>
exp2(const XDouble<T> &op)
{
    if (!op.isfinite()) return XDouble<T>(xdb::exp2(op.h));

    return (op * XDouble<T>::ln2).exp();
}

template <class T> inline XDouble<T>
log2(const XDouble<T> &op)
{
    if (!op.isfinite() || op.isnegative() || op.iszero()) return XDouble<T>(xdb::log2(op.h));

    return op.log() * XDouble<T>::log2e;
}


//
// Power functions
//

template <class T> inline XDouble<T>
powd(const XDouble<T> &base, int exponent)
{
    auto result = XDouble<T>(1.0);
    auto b = base;

    for (unsigned i = xdb::abs(exponent); i; i >>= 1) {

        if (i & 1) result *= b;
        b *= b;
    }

    if (exponent >= 0) {
        return result;
    } else if (result == 0.0) {
        return base.signbit() ? -XDouble<T>::inf() : XDouble<T>::inf();
    } else {
        return 1.0 / result;
    }
}

template <class T> inline XDouble<T>
pow(const XDouble<T> &base, const XDouble<T> &exponent)
{
    // Standard case
    if (base.isfinite() && !base.iszero() && !base.abs().isone()) {
        if (exponent.isfinite() && !exponent.iszero()) {

            if (isinteger(exponent)) {
                return powd(base, (int)exponent);
            } else {
                return (log(base) * exponent).exp();
            }
        }
    }

    // Special cases
    return XDouble<T>(std::pow((double)base, (double)exponent));
}

template <class T> inline XDouble<T>
sqr(const XDouble<T> &x)
{
    return x * x;
}

template <class T> inline XDouble<T>
sqrt(const XDouble<T> &x)
{
    if (x.iszero() || x.isnegative()) return XDouble<T>(xdb::sqrt(x.h));

    auto r = XDouble<T>(1.0 / xdb::sqrt(x.h));
    auto h = x * 0.5;

    r += (0.5 - h * r.sqr()) * r;
    r += (0.5 - h * r.sqr()) * r;
    r += (0.5 - h * r.sqr()) * r;

    return r * x;
}


//
// Error and gamma functions
//

// Not implemented


//
// Rounding and remainder functions
//

template <class T> inline XDouble<T>
ceil(const XDouble<T> &x)
{
    T hi = xdb::ceil(x.h);

    if (hi == x.h) {

        // Upper part is an integer
        T lo = xdb::ceil(x.l);
        return XDouble<T>::quickTwoSum(hi, lo);

    } else {

        return XDouble<T>(hi);
    }
}

template <class T> inline XDouble<T>
ceil(const XDouble<T> &x, int fracdigits)
{
    return x.ldexp10(fracdigits).ceil().ldexp10(-fracdigits);
}

template <class T> inline XDouble<T>
floor(const XDouble<T> &x)
{
    T hi = xdb::floor(x.h);

    if (hi == x.h) {

        // Upper part is an integer
        T lo = xdb::floor(x.l);
        return XDouble<T>::quickTwoSum(hi, lo);

    } else {

        return XDouble<T>(hi);
    }
}

template <class T> inline XDouble<T>
floor(const XDouble<T> &x, int fracdigits)
{
    return x.ldexp10(fracdigits).floor().ldexp10(-fracdigits);
}

template <class T> inline XDouble<T>
fmod(const XDouble<T> &numer, const XDouble<T> &denom)
{
    if (numer.isfinite()) {

        if (denom.isfinite() && !denom.iszero()) {

            XDouble<T> tquot = (numer / denom).trunc();
            return numer - tquot * denom;
        }
        if (denom.isinf()) {

            return numer;
        }
    }

    return XDouble<T>(std::fmod(numer.h, denom.h));
}

template <class T> inline XDouble<T>
trunc(const XDouble<T> &x)
{
    return xdb::isnegative(x) ? ceil(x) : floor(x);
}

template <class T> inline XDouble<T>
trunc(const XDouble<T> &x, int fracdigits)
{
    return x.ldexp10(fracdigits).trunc().ldexp10(-fracdigits);
}

template <class T> inline XDouble<T>
round(const XDouble<T> &x)
{
    if (x.isnegative()) {
        return (x - 0.5).ceil();
    } else {
        return (x + 0.5).floor();
    }
}

template <class T> inline XDouble<T>
round(const XDouble<T> &x, int fracdigits)
{
    return x.ldexp10(fracdigits).round().ldexp10(-fracdigits);
}

template <class T> inline XDouble<T>
roundEven(const XDouble<T> &x)
{
    auto m = x.fmod(2.0);

    if (x.isnegative()) {

        if (m >= -0.5) return x.ceil();
        if (m >  -1.0) return x.floor();
        if (m >  -1.5) return x.ceil();
        return x.floor();

    } else {

        if (m <= 0.5) return x.floor();
        if (m <  1.0) return x.ceil();
        if (m <  1.5) return x.floor();
        return x.ceil();
    }
}

template <class T> inline XDouble<T>
roundEven(const XDouble<T> &x, int fracdigits)
{
    return x.ldexp10(fracdigits).roundEven().ldexp10(-fracdigits);
}

template <class T> inline long
lround(const XDouble<T> &x)
{
    return x.isfinite() ? x.round().to_long() : lround(x.h);
}

template <class T> inline long long
llround(const XDouble<T> &x)
{
    return x.isfinite() ? x.round().to_long_long() : llround(x.h);
}

template <class T> inline XDouble<T>
rint(const XDouble<T> &x)
{
    auto result = xdb::nearbyint(x);

    // TODO:
    // The rint() functions do the same [as nearbyint], but will raise the
    // inexact exception (FE_INEXACT, checkable via fetestexcept(3)) when
    // the result differs in value from the argument.

    return result;
}

template <class T> inline XDouble<T>
rint(const XDouble<T> &x, int fracdigits)
{
    auto result = xdb::nearbyint(x, fracdigits);

    // TODO:
    // The rint() functions do the same [as nearbyint], but will raise the
    // inexact exception (FE_INEXACT, checkable via fetestexcept(3)) when
    // the result differs in value from the argument.

    return result;
}

template <class T> inline long
lrint(const XDouble<T> &x)
{
    return x.isfinite() ? x.rint().to_long() : lrint(x.h);
}

template <class T> inline long long
llrint(const XDouble<T> &x)
{
    return x.isfinite() ? x.rint().to_long_long() : llrint(x.h);
}

template <class T> inline XDouble<T>
nearbyint(const XDouble<T> &x)
{
    switch (fegetround()) {

        case FE_DOWNWARD:   return floor(x);
        case FE_TONEAREST:  return roundEven(x);
        case FE_TOWARDZERO: return trunc(x);
        default:            return ceil(x);
    }
}

template <class T> inline XDouble<T>
nearbyint(const XDouble<T> &x, int fracdigits)
{
    switch (fegetround()) {

        case FE_DOWNWARD:   return floor(x, fracdigits);
        case FE_TONEAREST:  return roundEven(x, fracdigits);
        case FE_TOWARDZERO: return trunc(x, fracdigits);
        default:            return ceil(x, fracdigits);
    }
}


//
// Floating-point manipulation functions
//

template <class T> inline XDouble<T>
copysign(const XDouble<T> &x, const XDouble<T> &y)
{
    if (xdb::signbit(x) == xdb::signbit(y)) return x;

    auto h = copysign(x.h, signbit(x.h) ? T(1.0) : T(-1.0));
    auto l = copysign(x.l, signbit(x.l) ? T(1.0) : T(-1.0));

    return XDouble<T>(T(h), T(l));
}


//
// Minimum, maximum, difference functions
//

template <class T> inline XDouble<T>
fdim(const XDouble<T> &x, const XDouble<T> &y)
{
    if (x.isfinite() && y.isfinite()) {
        return (x > y) ? x - y : XDouble<T>();
    }

    return xdb::fdim(x.h, y.h);
}

template <class T> inline XDouble<T>
fmax(const XDouble<T> &x, const XDouble<T> &y)
{
    if (x.isnan()) return y;
    if (y.isnan()) return x;

    return x > y ? x : y;
}

template <class T> inline XDouble<T>
fmin(const XDouble<T> &x,const XDouble<T> &y)
{
    if (x.isnan()) return y;
    if (y.isnan()) return x;

    return x < y ? x : y;
}


//
// Other functions
//

template <class T> inline XDouble<T>
fabs(const XDouble<T> &x)
{
    return xdb::isnegative(x) ? -x : x;
}

template <class T> inline XDouble<T>
abs(const XDouble<T> &x)
{
    return fabs(x);
}

template <class T> inline XDouble<T>
fma(const XDouble<T> &x, const XDouble<T> &y, const XDouble<T> &z)
{
    return x * y + z;
}


//
// Classification functions
//

template <class T> inline bool
isfinite(const XDouble<T> &x)
{
    return isfinite(x.h);
}

template <class T> inline bool
isinf(const XDouble<T> &x)
{
    return isinf(x.h);
}

template <class T> inline bool
isposinf(const XDouble<T> &x)
{
    return isinf(x) && ispositive(x);
}

template <class T> inline bool
isminusinf(const XDouble<T> &x)
{
    return isinf(x) && isnegative(x);
}

template <class T> inline bool
isnan(const XDouble<T> &x)
{
    return isnan(x.h);
}

template <class T> inline bool
isposnan(const XDouble<T> &x)
{
    return isnan(x) && ispositive(x);
}

template <class T> inline bool
isnegnan(const XDouble<T> &x)
{
    return isnan(x) && isnegative(x);
}

template <class T> inline bool
isnormal(const XDouble<T> &x)
{
    return isnormal(x.h);
}

template <class T> inline bool
signbit(const XDouble<T> &x)
{
    return x.h == 0.0 && x.l != 0.0 ? xdb::signbit(x.l) : xdb::signbit(x.h);
}

template <class T> inline bool
isinteger(const XDouble<T> &x)
{
    return x.isfinite() && x.floor() == x;
}

template <class T> inline bool
iseveninteger(const XDouble<T> &x)
{
    return x.isinteger() && x.fmod(2.0).abs() < 0.5;
}

template <class T> inline bool
isoddinteger(const XDouble<T> &x)
{
    return x.isinteger() && x.fmod(2.0).abs() > 0.5;
}

template <class T> inline bool
iszero(const XDouble<T> &x)
{
    return x.h == 0.0 && x.l == 0.0;
}

template <class T> inline bool
isposzero(const XDouble<T> &x)
{
    return iszero(x) && signbit(x) == 0;
}

template <class T> inline bool
isminzero(const XDouble<T> &x)
{
    return iszero(x) && signbit(x) == 1;
}

template <class T> inline bool
isone(const XDouble<T> &x)
{
    return x.h == 1.0 && x.l == 0.0;
}

template <class T> inline bool
ispositive(const XDouble<T> &x)
{
    return !signbit(x.h);
}

template <class T> inline bool
isnegative(const XDouble<T> &x)
{
    return signbit(x.h);
}

template <class T>
std::ostream& operator<<(std::ostream& os, const XDouble<T>& obj)
{
    os << "(" << obj.h;
    os << ",";
    os << obj.l;
    os << ")";

    return os;
}

template <class T> inline const XDouble<T>
XDouble<T>::e          ("2.71828182 84590452 35360287 47135266 24977572 47093699 95957496 69676277");
template <class T> inline const XDouble<T>
XDouble<T>::log2e      ("1.44269504 08889634 07359924 68100189 21374266 45954152 98593413 54494069");
template <class T> inline const XDouble<T>
XDouble<T>::log10e     ("0.43429448 19032518 27651128 91891660 50822943 97005803 66656611 44537831");
template <class T> inline const XDouble<T>
XDouble<T>::pi         ("3.14159265 35897932 38462643 38327950 28841971 69399375 10582097 49445923");
template <class T> inline const XDouble<T>
XDouble<T>::inv_pi     ("0.31830988 61837906 71537767 52674502 87240689 19291480 91289749 53346881");
template <class T> inline const XDouble<T>
XDouble<T>::inv_sqrtpi ("0.56418958 35477562 86948079 45156077 25858440 50629328 99885684 4085a722");
template <class T> inline const XDouble<T>
XDouble<T>::ln2        ("0.69314718 05599453 09417232 12145817 65680755 00134360 25525412 06800195");
template <class T> inline const XDouble<T>
XDouble<T>::ln10       ("2.30258509 29940456 84017991 45468436 42076011 01488628 77297603 33279009");
template <class T> inline const XDouble<T>
XDouble<T>::sqrt2      ("1.41421356 23730950 48801688 72420969 80785696 71875376 94807317 66797379");
template <class T> inline const XDouble<T>
XDouble<T>::sqrt3      ("1.73205080 75688772 93527446 34150587 23669428 05253810 38062805 58069794");
template <class T> inline const XDouble<T>
XDouble<T>::inv_sqrt3  ("0.57735026 91896257 64509148 78050195 74556476 01751270 12687601 86023264");
template <class T> inline const XDouble<T>
XDouble<T>::egamma     ("0.57721566 49015328 60606512 09008240 24310421 59335939 92359880 57672349");
template <class T> inline const XDouble<T>
XDouble<T>::phi        ("1.61803398 87498948 48204586 83436563 81177203 09179805 76286213 54486227");

}
