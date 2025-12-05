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
#include "ErrorTypes.h"

namespace vamiga {

class AppException : public utl::exception
{
public:

    AppException(i64 d, const string &s) : utl::exception(d, s) { }
    AppException(i64 d) : utl::exception(d) { }
    AppException(const string &s) : utl::exception(0, s) { }
    AppException() : utl::exception(0) { }

    i64 data() const { return *payload<i64>(); }
};

struct AppError : public utl::exception
{
public:

    AppError(Fault fault, const string &s);
    AppError(Fault fault, const char *s) : AppError(fault, string(s)) { };
    AppError(Fault fault, const fs::path &p) : AppError(fault, p.string()) { };
    AppError(Fault fault, std::integral auto v) : AppError(fault, std::to_string(v)) { };
    AppError(Fault fault) : AppError(fault, "") { }

    Fault fault() const { return *payload<Fault>(); }
};

}
