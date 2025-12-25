// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/abilities/Dumpable.h"
#include <span>
#include <cstdint>
#include <cassert>

namespace utl {

class ByteView : public Dumpable {

    std::span<const u8> span{};

public:

    constexpr ByteView() = default;
    constexpr ByteView(const u8* data, isize size) {

        span = std::span(data, size_t(size));
    }

    constexpr ByteView(std::span<const u8> bytes) {

        span = bytes;
    }

    constexpr const u8 &operator[](isize i) const {

        assert(i >= 0 && i < isize(span.size()));
        return span[i];
    }

    constexpr operator std::span<const u8>() const {

        return span;
    }

    constexpr isize size() const { return (isize)span.size(); }
    constexpr bool empty() const { return span.empty(); }
    constexpr std::span<const u8> bytes() const { return span; }

    /*
    void dump(std::ostream &os, DumpOpt opt) override {

        dump(os, opt, 0, size());
    };

    void dump(std::ostream &os, DumpOpt opt, isize offset, isize length) override {

        offset = std::clamp(offset, isize(0), size());
        length = std::clamp(length, isize(0), size() - offset);

        Dumpable::dump(os, opt, span.subspan(offset, length));
    };
    */

    class iterator {

        const ByteView* view_;
        isize pos_;

    public:

        using iterator_category = std::random_access_iterator_tag;
        using value_type        = u8;
        using difference_type   = isize;
        using pointer           = void;     // Proxy-free
        using reference         = const u8; // Returned by value

        constexpr iterator(const ByteView* view, isize pos) : view_(view), pos_(pos) {}

        // Dereference
        constexpr reference operator*() const { return (*view_)[pos_]; }

        // Increment / Decrement
        constexpr iterator& operator++() { ++pos_; return *this; }
        constexpr iterator& operator--() { --pos_; return *this; }

        // Random access
        constexpr reference operator[](difference_type n) const { return *(*this + n); }

    private:

        friend constexpr iterator operator+(const iterator& it, difference_type n) {
            return iterator(it.view_, it.pos_ + n);
        }
        friend constexpr iterator operator+(difference_type n, const iterator& it) {
            return iterator(it.view_, it.pos_ + n);
        }
        friend constexpr iterator operator-(const iterator& it, difference_type n) {
            return iterator(it.view_, it.pos_ - n);
        }
        friend constexpr difference_type operator-(const iterator& lhs, const iterator& rhs) {
            assert(lhs.view_ == rhs.view_); return lhs.pos_ - rhs.pos_;
        }
        friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) {
            return lhs.view_ == rhs.view_ && lhs.pos_ == rhs.pos_;
        }
        friend constexpr bool operator!=(const iterator& lhs, const iterator& rhs) {
            return !(lhs == rhs);
        }
        friend constexpr bool operator<(const iterator& lhs, const iterator& rhs) {
            assert(lhs.view_ == rhs.view_); return lhs.pos_ < rhs.pos_;
        }
        friend constexpr bool operator<=(const iterator& lhs, const iterator& rhs) {
            return !(rhs < lhs);
        }
        friend constexpr bool operator>(const iterator& lhs, const iterator& rhs) {
            return rhs < lhs;
        }
        friend constexpr bool operator>=(const iterator& lhs, const iterator& rhs) {
            return !(lhs < rhs);
        }
    };

    constexpr iterator begin() const { return iterator(this, 0); }
    constexpr iterator end()   const { return iterator(this, size()); }
};

}
