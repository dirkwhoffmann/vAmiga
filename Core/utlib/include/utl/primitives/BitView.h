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

template <typename T>
class BaseBitView {

    static_assert(std::is_same_v<T, u8> || std::is_same_v<T, const u8>);

    std::span<T> sp{};   // underlying bytes
    isize len = 0;       // number of bits

public:

    constexpr BaseBitView() = default;

    constexpr BaseBitView(T* data, isize bitCount)
    : sp(data, (bitCount + 7) / 8), len(bitCount)
    {
        assert(len >= 0);
        assert(isize(sp.size()) * 8 >= len);
    }

    constexpr BaseBitView(std::span<T> bytes, isize bitCount)
    : sp(bytes), len(bitCount)
    {
        assert(len >= 0);
        assert(isize(sp.size()) * 8 >= len);
    }

    // Allows const-view from mutable-view
    constexpr BaseBitView(const BaseBitView<u8>& other)
        requires std::is_const_v<T>
    : sp(other.bytes()), len(other.size())
    {}

    // Provides a byte-level view
    constexpr auto byteView() const
    {
        // if constexpr (std::is_const_v<decltype(*this)>) {
        if constexpr (std::is_const_v<T>) {
            return ByteView(sp);
        } else {
            return MutableByteView(sp);
        }
    }

    // Reads a single bit
    constexpr bool operator[](isize i) const
    {
        assert(i >= 0 && i < len);
        return (sp[i >> 3] >> (7 - (i & 7))) & 1;
    }

    constexpr u8 getByte(isize bitIndex) const
    {
        assert(len > 0);

        // Fast path for a byte-aligned and byte-sized buffer
        if (((bitIndex & 7) == 0) && ((len & 7) == 0)) {

            const isize byteCount = len >> 3;
            isize byteIndex = (bitIndex >> 3) % byteCount;
            if (byteIndex < 0) byteIndex += byteCount;

            return sp[byteIndex];
        }

        // Normalize start bit
        isize pos = bitIndex % len;
        if (pos < 0) pos += len;

        u8 value = 0;

        for (int b = 0; b < 8; ++b) {

            isize i = pos + b;
            if (i >= len) i -= len;

            value <<= 1;
            value |= (sp[i >> 3] >> (7 - (i & 7))) & 1;
        }

        return value;
    }

    // Writes a single bit
    constexpr void set(isize bitIndex, bool value)
    requires (!std::is_const_v<T>)
    {
        // Clamp position
        isize i = bitIndex % len;
        if (i < 0) i += len;

        // assert(i >= 0 && i < len);

        auto& byte = sp[i >> 3];
        auto  mask = u8(1 << (7 - (i & 7)));
        value ? byte |= mask : byte &= ~mask;
    }

    // Writes a byte
    constexpr void setByte(isize bitIndex, u8 value)
    {
        assert(len > 0);

        // Fast path for a byte-aligned and byte-sized buffer
        if (((bitIndex & 7) == 0) && ((len & 7) == 0)) {

            const isize byteCount = len >> 3;
            isize byteIndex = (bitIndex >> 3) % byteCount;
            if (byteIndex < 0) byteIndex += byteCount;

            sp[byteIndex] = value;
            return;
        }

        // Normalize start bit
        isize pos = bitIndex % len;
        if (pos < 0) pos += len;

        for (int b = 0; b < 8; ++b) {

            isize i = pos + b;
            if (i >= len) i -= len;

            u8& byte = sp[i >> 3];
            const u8 mask = u8(1 << (7 - (i & 7)));

            if (value & (1 << (7 - b)))
                byte |= mask;
            else
                byte &= ~mask;
        }
    }

    constexpr void setBytes(isize bitIndex, const std::vector<u8> &values)
    {
        for (auto &value : values) {
            setByte(bitIndex, value); bitIndex += 8;
        }
    }
    
    constexpr isize size()  const { return len; }
    constexpr bool  empty() const { return len == 0; }
    constexpr std::span<T> bytes() const { return sp; }
    constexpr T* data() const { return sp.data();}
    
    // -----------------------------------------------------------------
    // Iterator (read-only bit iterator — by value, like std::vector<bool>)
    // -----------------------------------------------------------------

    class iterator {

        const BaseBitView* view_;
        isize pos_;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = bool;
        using difference_type   = isize;
        using pointer           = void;
        using reference         = bool;

        constexpr iterator(const BaseBitView* v, isize p)
        : view_(v), pos_(p)
        {
            assert(view_);
        }

        constexpr bool operator*() const { return (*view_)[pos_]; }

        constexpr iterator& operator++() { ++pos_; return *this; }
        constexpr iterator& operator--() { --pos_; return *this; }

        constexpr bool operator[](difference_type n) const {
            return *(*this + n);
        }

        friend constexpr iterator operator+(iterator it, difference_type n) {
            it.pos_ += n; return it;
        }
        friend constexpr iterator operator-(iterator it, difference_type n) {
            it.pos_ -= n; return it;
        }
        friend constexpr difference_type operator-(iterator a, iterator b) {
            assert(a.view_ == b.view_); return a.pos_ - b.pos_;
        }
        friend constexpr bool operator==(iterator a, iterator b) {
            return a.view_ == b.view_ && a.pos_ == b.pos_;
        }
    };

    constexpr iterator begin() const { return iterator(this, 0); }
    constexpr iterator end()   const { return iterator(this, len); }

    // -----------------------------------------------------------------
    // Cyclic iterator
    // -----------------------------------------------------------------

    class cyclic_iterator {

        const BaseBitView* view_;
        isize pos_;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = bool;
        using difference_type   = isize;
        using pointer           = void;
        using reference         = bool;

        constexpr cyclic_iterator(const BaseBitView* v, isize p = 0)
        : view_(v), pos_(p)
        {
            assert(view_);
            assert(!view_->empty());
        }

        constexpr bool operator*() const
        {
            isize n = view_->size();
            isize i = pos_ % n;
            if (i < 0) i += n;
            return (*view_)[i];
        }

        constexpr cyclic_iterator& operator++() { ++pos_; return *this; }
        constexpr cyclic_iterator& operator--() { --pos_; return *this; }

        constexpr bool operator[](difference_type n) const {
            return *(*this + n);
        }

        constexpr isize offset() const { return pos_; }

        friend constexpr cyclic_iterator operator+(cyclic_iterator it, difference_type n) {
            it.pos_ += n; return it;
        }
    };

    constexpr cyclic_iterator cyclic_begin(isize pos = 0) const {
        return cyclic_iterator(this, pos);
    }
};

using BitView        = BaseBitView<const u8>;
using MutableBitView = BaseBitView<u8>;

}

