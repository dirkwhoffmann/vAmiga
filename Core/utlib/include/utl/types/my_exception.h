// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <exception>
#include <string>
#include <any>

namespace utl {

class exception : public std::exception {

    std::any _payload;
    std::string _msg;

public:

    explicit exception(std::any payload = {}, std::string msg = "")
        : _payload(std::move(payload)), _msg(std::move(msg)) {}

    const char *what() const noexcept override {
        return _msg.c_str();
    }

    // Setters
    void set_msg(std::string value) {
        _msg = std::move(value);
    }
    template<class T> void set_payload(T&& value) {
        _payload = std::forward<T>(value);
    }

    // Return typed pointer to payload if type matches
    template<class T> const T* payload() const noexcept {
        return std::any_cast<T>(&_payload);
    }

    // Return payload as std::any
    const std::any& payload() const noexcept {
        return _payload;
    }
};

}
