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

struct Error : public util::Exception
{
    Error(ErrorCode code, const string &s);
    Error(ErrorCode code, const char *s) : Error(code, string(s)) { };
    Error(ErrorCode code, const std::filesystem::path &path) : Error(code, path.string()) { };
    Error(ErrorCode code, long v) : Error(code, std::to_string(v)) { };
    Error(ErrorCode code) : Error(code, "") { }
    
    const char *what() const throw() override;
};

}
