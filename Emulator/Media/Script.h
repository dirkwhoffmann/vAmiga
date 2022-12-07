// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.h"

namespace vamiga {

class Amiga;

class Script : public AmigaFile {
    
public:
    
    static bool isCompatible(const string &path);
    static bool isCompatible(std::istream &stream);
    
    bool isCompatiblePath(const string &path) const override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) const override { return isCompatible(stream); }

    
    //
    // Initializing
    //

public:

    Script(const string &path) throws { init(path); }
    Script(const u8 *buf, isize len) throws { init(buf, len); }

    const char *getDescription() const override { return "Script"; }

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    FileType type() const override { return FILETYPE_SCRIPT; }

    
    //
    // Processing
    //
    
    // Executes the script
    void execute(Amiga &amiga);
};

}
