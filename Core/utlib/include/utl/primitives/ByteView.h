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

template<typename T>
class BaseByteView : public Hashable, public Dumpable {

    static_assert(std::is_same_v<T, u8> || std::is_same_v<T, const u8>);

    std::span<T> span{};

public:

    constexpr BaseByteView() = default;
    constexpr BaseByteView(T* data, isize size) {

        span = std::span(data, size_t(size));
    }

    constexpr BaseByteView(std::span<T> bytes) {

        span = bytes;
    }

    constexpr BaseByteView(const BaseByteView<u8>& other)
            requires std::is_const_v<T>
        : span(other.bytes()) {}

    constexpr T &operator[](isize i) const {

        assert(i >= 0 && i < isize(span.size()));
        return span[i];
    }

    constexpr operator std::span<const u8>() const {

        return span;
    }

    constexpr isize size() const { return (isize)span.size(); }
    constexpr bool empty() const { return span.empty(); }
    constexpr std::span<const u8> bytes() const { return span; }
    constexpr void clear(u8 value = 0) const requires (!std::is_const_v<T>) {
        for (auto &b : span) { b = value; }
    }

    //
    // Methods from Hashable
    //

    u64 hash(HashAlgorithm algorithm) const override {
        return Hashable::hash(span.data(), (isize)span.size(), algorithm);
    }


    //
    // Methods from Dumpable
    //

    Dumpable::DataProvider dataProvider() const override {
        return Dumpable::dataProvider(span);
    }


    //
    // Iterator
    //

    class iterator {

        const BaseByteView* view_;
        isize pos_;

    public:

        using iterator_category = std::random_access_iterator_tag;
        using value_type        = u8;
        using difference_type   = isize;
        using pointer           = void;
        using reference         = std::conditional_t<std::is_const_v<T>, const u8&, u8&>;

        constexpr iterator(const BaseByteView* view, isize pos) : view_(view), pos_(pos) {

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


    //
    // Cyclic iterator
    //

    class cyclic_iterator {

        const BaseByteView* view_;
        isize pos_;

    public:

        using iterator_category = std::random_access_iterator_tag;
        using value_type        = u8;
        using difference_type   = isize;
        using pointer           = void;
        using reference         = std::conditional_t<std::is_const_v<T>, const u8&, u8&>;

        constexpr cyclic_iterator(const BaseByteView* view, isize pos = 0) : view_(view), pos_(pos) {

            assert(view_);
            assert(!view_->empty());
        }

        // Dereference with wrap
        constexpr reference operator*() const
        {
            auto n = view_->size();
            auto i = pos_ % n;
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

    constexpr cyclic_iterator cyclic_begin(isize pos) const {
        return cyclic_iterator(this, pos);
    }
};

using ByteView        = BaseByteView<const u8>;
using MutableByteView = BaseByteView<u8>;

}
