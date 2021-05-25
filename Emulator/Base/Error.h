// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ErrorTypes.h"
#include "Exception.h"

//
// VAError
//

struct VAError : public util::Exception
{
    VAError(ErrorCode code) : Exception((i64)code) { }
    VAError(ErrorCode code, const string &s) : Exception((i64)code, s) { }
    
    const char *what() const throw() override;
};
