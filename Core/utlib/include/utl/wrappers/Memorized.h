// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/types.h"
#include <functional>

namespace utl::wrappers {

template <typename T> class Memorized {

    std::optional<T> value;
    std::function<T()> getter;

public:

    explicit Memorized(std::function<T()> g = {}) : getter(g) {}

    void bind(std::function<T()> g) { getter = g; value.reset(); }

    const T &current() const {

        value = getter();
        return *value;
    }

    const T &cached() const {

        if (!value) value = getter();
        return *value;
    }
};

}

