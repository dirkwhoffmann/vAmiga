// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"
#include "AppErrorTypes.h"

namespace vamiga {

class AppException : public utl::GenericException<i64>
{
public:

    AppException(i64 d, const string &s) : utl::GenericException<i64>(d, s) { }
    AppException(i64 d) : AppException(d, "") { }
    AppException(const string &s) : AppException(0, s) { }
    AppException() : AppException(0) { }

    i64 data() const { return payload; }
};

struct AppError : public Error {

    const char *errstr() const noexcept override {

        switch (payload) {
            default:
                return "AppError";
        }
    }

    AppError(Fault fault, const string &s);
    AppError(Fault fault, const char *s) : AppError(fault, string(s)) { };
    AppError(Fault fault, const fs::path &p) : AppError(fault, p.string()) { };
    AppError(Fault fault, std::integral auto v) : AppError(fault, std::to_string(v)) { };
    AppError(Fault fault) : AppError(fault, "") { }

    // Fault fault() const { return Fault(payload); }
};

}
