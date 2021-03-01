// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaPublicTypes.h"

#include <exception>
#include <string>


//
// VAError
//

struct VAError : public std::exception
{
    ErrorCode errorCode;
    
    VAError(ErrorCode code) : errorCode(code) { }
    
    const char *what() const throw() override;
};


//
// ParseError
//

struct ParseError : public std::exception {

    string token;
    string expected;
    
    ParseError(const string &t) : token(t) { }
    ParseError(const string &t, const string &e) : token(t), expected(e) { }

    const char *what() const throw() override { return token.c_str(); }
};

struct ParseEnumError : public ParseError { using ParseError::ParseError; };
    

//
// ConfigError
//

struct ConfigError : public std::exception
{
    string description;
    
    ConfigError(const string &s) : description(s) { }
    
    const char *what() const throw() override {
        return  description.c_str();
    }
};

struct ConfigArgError : ConfigError {
    ConfigArgError(const string &s) : ConfigError(s) { };
};

struct ConfigFileReadError : ConfigError {
    ConfigFileReadError(const string &s) : ConfigError(s) { };
};

struct ConfigLockedError : ConfigError {
    ConfigLockedError() : ConfigError("") { };
};

struct ConfigUnsupportedError : ConfigError {
    ConfigUnsupportedError() : ConfigError("") { };
};
