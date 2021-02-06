// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaTypes.h"
#include <exception>
#include <string>

struct VAError : public std::exception
{
    ErrorCode errorCode;
    
    VAError(ErrorCode code) : errorCode(code) { }
    
    const char *what() const throw() override {
        return  ErrorCodeEnum::key(errorCode);
    }
};

struct ConfigError : public std::exception
{
    std::string description;
    
    ConfigError(const std::string &s) : description(s) { }
    
    const char *what() const throw() override {
        return  description.c_str();
    }
};

struct ConfigArgError : ConfigError {
    ConfigArgError(const std::string &s) : ConfigError(s) { }; 
};

struct ConfigLockedError : ConfigError {
    ConfigLockedError() : ConfigError("") { };
};
