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
Script::isCompatible(const std::filesystem::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    printf("Script::isCompatible suffix = %s\n", suffix.c_str());
    return suffix == ".INI";
}

bool
Script::isCompatible(const u8 *buf, isize len)
{
    return true;
}

bool
Script::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

void
Script::execute(class Amiga &amiga)
{
    string s((char *)data.ptr, data.size);
    amiga.retroShell.asyncExecScript(s);
}

}
