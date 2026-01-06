// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/abilities/Reflectable.h"

namespace retro::vault::cbm {

using namespace utl;

using FSFault = long;

struct FSError : public Error {

    static constexpr long FS_OK                     = 0;
    static constexpr long FS_UNKNOWN                = 1;

    // General
    static constexpr long FS_OUT_OF_RANGE           = 2;
    static constexpr long FS_INVALID_PATH           = 3;
    static constexpr long FS_INVALID_REGEX          = 4;
    static constexpr long FS_NOT_A_DIRECTORY        = 5;
    static constexpr long FS_NOT_A_FILE             = 6;
    static constexpr long FS_NOT_A_FILE_OR_DIRECTORY= 7;
    static constexpr long FS_NOT_FOUND              = 8;
    static constexpr long FS_EXISTS                 = 9;
    static constexpr long FS_CANNOT_OPEN            = 10;
    static constexpr long FS_UNFORMATTED            = 11;
    static constexpr long FS_UNSUPPORTED            = 12;
    static constexpr long FS_READ_ONLY              = 13;
    static constexpr long FS_WRONG_BSIZE            = 14;
    static constexpr long FS_WRONG_CAPACITY         = 15;
    static constexpr long FS_WRONG_DOS_TYPE         = 16;
    static constexpr long FS_WRONG_BLOCK_TYPE       = 17;
    static constexpr long FS_HAS_CYCLES             = 18;
    static constexpr long FS_CORRUPTED              = 19;

    // Posix layer
    static constexpr long FS_INVALID_HANDLE         = 20;
    // Import
    static constexpr long FS_OUT_OF_SPACE           = 30;

    // Export
    static constexpr long FS_DIR_NOT_EMPTY          = 40;
    static constexpr long FS_CANNOT_CREATE_DIR      = 41;
    static constexpr long FS_CANNOT_CREATE_FILE     = 42;

    const char *errstr() const noexcept override {

        switch (payload) {

            case FS_OK:                         return "FS_OK";
            case FS_UNKNOWN:                    return "FS_UNKNOWN";

            case FS_OUT_OF_RANGE:               return "FS_OUT_OF_RANGE";
            case FS_INVALID_PATH:               return "FS_INVALID_PATH";
            case FS_INVALID_REGEX:              return "FS_INVALID_REGEX";
            case FS_NOT_A_DIRECTORY:            return "FS_NOT_A_DIRECTORY";
            case FS_NOT_A_FILE:                 return "FS_NOT_A_FILE";
            case FS_NOT_A_FILE_OR_DIRECTORY:    return "FS_NOT_A_FILE_OR_DIRECTORY";
            case FS_NOT_FOUND:                  return "FS_NOT_FOUND";
            case FS_EXISTS:                     return "FS_EXISTS";
            case FS_CANNOT_OPEN:                return "FS_CANNOT_OPEN";
            case FS_UNFORMATTED:                return "FS_UNFORMATTED";
            case FS_UNSUPPORTED:                return "FS_UNSUPPORTED";
            case FS_READ_ONLY:                  return "FS_READ_ONLY";
            case FS_WRONG_BSIZE:                return "FS_WRONG_BSIZE";
            case FS_WRONG_CAPACITY:             return "FS_WRONG_CAPACITY";
            case FS_WRONG_DOS_TYPE:             return "FS_WRONG_DOS_TYPE";
            case FS_WRONG_BLOCK_TYPE:           return "FS_WRONG_BLOCK_TYPE";
            case FS_HAS_CYCLES:                 return "FS_HAS_CYCLES";
            case FS_CORRUPTED:                  return "FS_CORRUPTED";

            case FS_INVALID_HANDLE:             return "FS_INVALID_HANDLE";

            case FS_OUT_OF_SPACE:               return "FS_OUT_OF_SPACE";

            case FS_DIR_NOT_EMPTY:              return "FS_DIR_NOT_EMPTY";
            case FS_CANNOT_CREATE_DIR:          return "FS_CANNOT_CREATE_DIR";
            case FS_CANNOT_CREATE_FILE:         return "FS_CANNOT_CREATE_FILE";

            default:
                return "UNKNOWN";
        }
    }

    explicit FSError(long fault, const std::string &msg = "");
    explicit FSError(long fault, const char *str) : FSError(fault, string(str)) { }
    explicit FSError(long fault, const fs::path &path) : FSError(fault, path.string()) { }
};

}
