// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Script.h"
#include "Amiga.h"
#include "IOUtils.h"

#include <sstream>

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
    string s((char *)data, size);
    try { amiga.retroShell.execScript(s); } catch (util::Exception &) { }
}
