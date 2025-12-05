// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSError.h"

namespace vamiga {

FSError::FSError(FSFault code, const string &s) : utl::GenericException<FSFault>(code)
{
    switch (code)
    {
        case FSFault::FS_UNINITIALIZED:
            set_msg("No file system present.");
            break;

        case FSFault::FS_INVALID_PATH:
            set_msg("Invalid path: " + s);
            break;

        case FSFault::FS_INVALID_REGEX:
            set_msg("Invalid search pattern: " + s);
            break;

        case FSFault::FS_NOT_A_DIRECTORY:
            set_msg(s.empty() ? "Not a directory." : s + " is not a directory.");
            break;

        case FSFault::FS_NOT_A_FILE:
            set_msg(s.empty() ? "Not a file." : s + " is not a file.");
            break;

        case FSFault::FS_NOT_A_FILE_OR_DIRECTORY:
            set_msg(s.empty() ? "Not a file or directory." : s + " is not a file or directory.");
            break;

        case FSFault::FS_NOT_FOUND:
            set_msg(s.empty() ? "Not found." : s + " not found.");
            break;

        case FSFault::FS_EXISTS:
            set_msg(s.empty() ? "Item already exists." : s + " already exists.");
            break;

        case FSFault::FS_CANNOT_OPEN:
            set_msg("Cannot open file" + (s.empty() ? "" : " " + s) + ".");
            break;

        case FSFault::FS_UNFORMATTED:
            set_msg("Unformatted device.");
            break;

        case FSFault::FS_UNSUPPORTED:
            set_msg("Unsupported file system.");
            break;

        case FSFault::FS_READ_ONLY:
            set_msg("Read-only file system.");
            break;

        case FSFault::FS_WRONG_BSIZE:
            set_msg("Invalid block size.");
            break;

        case FSFault::FS_WRONG_DOS_TYPE:
            set_msg("Wrong DOS type.");
            break;

        case FSFault::FS_WRONG_CAPACITY:
            set_msg("Wrong file system capacity.");
            break;

        case FSFault::FS_HAS_CYCLES:
            set_msg("Cyclic reference chain detected.");
            break;

        case FSFault::FS_CORRUPTED:
            set_msg("Corrupted file system.");
            break;

        case FSFault::FS_OUT_OF_SPACE:
            set_msg("File system capacity exceeded.");
            break;

        case FSFault::FS_DIR_NOT_EMPTY:
            set_msg("Directory is not empty.");
            break;

        case FSFault::FS_CANNOT_CREATE_DIR:
            set_msg("Unable to create directory.");
            break;

        case FSFault::FS_CANNOT_CREATE_FILE:
            set_msg("Unable to create file.");
            break;

        default:
            set_msg(string("Error code ") + std::to_string((i64)fault()) +
            " (" + FSFaultEnum::key(fault()) + ").");
            break;
    }
}

}
