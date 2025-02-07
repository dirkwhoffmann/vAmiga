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

struct CoreException : public util::Exception
{
    CoreException(CoreError code, const string &s);
    CoreException(CoreError code, const char *s) : CoreException(code, string(s)) { };
    CoreException(CoreError code, const std::filesystem::path &path) : CoreException(code, path.string()) { };
    CoreException(CoreError code, long v) : CoreException(code, std::to_string(v)) { };
    CoreException(CoreError code) : CoreException(code, "") { }
    
    const char *what() const throw() override;
};

}
