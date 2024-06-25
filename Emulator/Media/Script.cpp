// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Script.h"
#include "Amiga.h"
#include "IOUtils.h"

#include <sstream>

namespace vamiga {

bool
Script::isCompatible(const string &path)
{
    auto suffix = util::uppercased(util::extractSuffix(path));
    return suffix == "INI";
}

bool
Script::isCompatible(std::istream &stream)
{
    return true;
}

void
Script::execute(class Amiga &amiga)
{
    string s((char *)data.ptr, data.size);
    try { amiga.retroShell.execScript(s); } catch (util::Exception &) { }
}

}
