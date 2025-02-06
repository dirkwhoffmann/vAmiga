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

struct VAException : public util::Exception
{
    VAException(VAError code, const string &s);
    VAException(VAError code, const char *s) : VAException(code, string(s)) { };
    VAException(VAError code, const std::filesystem::path &path) : VAException(code, path.string()) { };
    VAException(VAError code, long v) : VAException(code, std::to_string(v)) { };
    VAException(VAError code) : VAException(code, "") { }
    
    const char *what() const throw() override;
};

}
