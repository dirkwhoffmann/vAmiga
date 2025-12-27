// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once


#include "utl/primitives/ByteView.h"

namespace utl {

class BitView {

    std::span<const u8> span{}; // Data
    isize length = 0;           // Length in bits

public:

    constexpr BitView() = default;
    constexpr BitView(const u8* data, isize bitCount) {

        span     = std::span(data, (bitCount + 7) / 8);
        length = bitCount;

        assert(length >= 0);
        assert(isize(span.size()) * 8 >= length);
    }

    constexpr BitView(std::span<const u8> bytes, isize bitCount) {

        span     = bytes;
        length = bitCount;

        assert(length >= 0);
        assert(isize(span.size()) * 8 >= length);
    }

    constexpr BitView(const ByteView& view) {

        span     = view.bytes();
        length = view.size() * 8;

        assert(length >= 0);
        assert(isize(span.size()) * 8 >= length);
    }

    constexpr bool operator[](isize i) const {

        assert(i >= 0 && i < length);
        return (span[i >> 3] >> (7 - (i & 7))) & 1;
    }

    constexpr isize size() const { return length; }
    constexpr bool empty() const { return length == 0; }
    constexpr std::span<const u8> bytes() const { return span; }

    class iterator {

        const BitView* view_;
        isize pos_;

    public:

        using iterator_category = std::random_access_iterator_tag;
        using value_type        = bool;
        using difference_type   = isize;
        using pointer           = void;     // Proxy-free
        using reference         = bool;     // Returned by value

        constexpr iterator(const BitView* view, isize pos) : view_(view), pos_(pos) {

            assert(view_);
            assert(!view_->empty());
        }

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

    class cyclic_iterator {

        const BitView* view_;
        isize pos_;

    public:

        using iterator_category = std::random_access_iterator_tag;
        using value_type        = bool;
        using difference_type   = isize;
        using pointer           = void;     // Proxy-free
        using reference         = bool;     // Returned by value

        constexpr cyclic_iterator(const BitView* view, isize pos = 0) : view_(view), pos_(pos) {

            assert(view_);
            assert(!view_->empty());
        }

        // Dereference (cyclic)
        constexpr reference operator*() const
        {
            const isize n = view_->size();
            isize i = pos_ % n;
            if (i < 0) i += n;
            return (*view_)[i];
        }

        // Increment / Decrement
        constexpr cyclic_iterator& operator++() { ++pos_; return *this; }
        constexpr cyclic_iterator& operator--() { --pos_; return *this; }

        // Random access
        constexpr reference operator[](difference_type n) const { return *(*this + n); }

    private:

        friend constexpr cyclic_iterator operator+(const cyclic_iterator& it, difference_type n) {
            return cyclic_iterator(it.view_, it.pos_ + n);
        }
        friend constexpr cyclic_iterator operator-(const cyclic_iterator& it, difference_type n) {
            return cyclic_iterator(it.view_, it.pos_ - n);
        }
        friend constexpr difference_type operator-(const cyclic_iterator& lhs, const cyclic_iterator& rhs) {
            assert(lhs.view_ == rhs.view_); return lhs.pos_ - rhs.pos_;
        }
        friend constexpr bool operator==(const cyclic_iterator& lhs, const cyclic_iterator& rhs) {
            return lhs.view_ == rhs.view_ && lhs.pos_ == rhs.pos_;
        }
        friend constexpr bool operator!=(const cyclic_iterator& lhs, const cyclic_iterator& rhs) {
            return !(lhs == rhs);
        }
        friend constexpr bool operator<(const cyclic_iterator& lhs, const cyclic_iterator& rhs) {
            assert(lhs.view_ == rhs.view_); return lhs.pos_ < rhs.pos_;
        }
        friend constexpr bool operator<=(const cyclic_iterator& lhs, const cyclic_iterator& rhs) {
            return !(rhs < lhs);
        }
        friend constexpr bool operator>(const cyclic_iterator& lhs, const cyclic_iterator& rhs) {
            return rhs < lhs;
        }
        friend constexpr bool operator>=(const cyclic_iterator& lhs, const cyclic_iterator& rhs) {
            return !(lhs < rhs);
        }
    };
};

}
