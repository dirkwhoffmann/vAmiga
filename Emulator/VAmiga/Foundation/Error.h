// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ErrorTypes.h"
#include "Exception.h"
#include <filesystem>

namespace vamiga {

struct CoreError : public util::CoreException
{
    CoreError(Fault code, const string &s);
    CoreError(Fault code, const char *s) : CoreError(code, string(s)) { };
    CoreError(Fault code, const fs::path &path) : CoreError(code, path.string()) { };
    CoreError(Fault code, long v) : CoreError(code, std::to_string(v)) { };
    CoreError(Fault code) : CoreError(code, "") { }
    
    const char *what() const throw() override;
    Fault fault() const { return Fault(data); }
};

}
