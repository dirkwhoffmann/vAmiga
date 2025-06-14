// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Configurable.h"
#include "Option.h"
#include "Defaults.h"
#include "IOUtils.h"
#include <algorithm>

namespace vamiga {

using namespace util;

bool
Configurable::isValidOption(Opt opt) const
{
    for (auto &it: getOptions()) {
        if (it == opt) return true;
    }
    return false;
}

void
Configurable::checkOption(Opt opt, const string &value)
{
    checkOption(opt, OptionParser::parse(opt, value));
}

void
Configurable::checkOption(const string &opt, const string &value)
{
    checkOption(Opt(util::parseEnum<OptEnum>(opt)), value);
}

void
Configurable::setOption(Opt opt, const string &value) 
{
    setOption(opt, OptionParser::parse(opt, value));
}

void
Configurable::setOption(const string &opt, const string &value)
{
    setOption(Opt(util::parseEnum<OptEnum>(opt)), value);
}

void
Configurable::resetConfig(const Defaults &defaults, isize objid)
{
    for (auto &option : getOptions()) {
        
        try {
            setOption(option, defaults.get(option, objid));
        } catch (...) {
            setOption(option, defaults.getFallback(option, objid));
        }
    }
}

void
Configurable::dumpConfig(std::ostream &os) const
{
    using namespace util;

    for (auto &opt: getOptions()) {

        auto name = OptEnum::key(opt);
        auto help = OptEnum::help(opt);
        auto arg  = OptionParser::asString(opt, getOption(opt));
        auto arghelp = OptionParser::help(opt, getOption(opt));

        os << tab(name);
        os << std::setw(16) << std::left << std::setfill(' ') << arg;
        os << help;
        if (arghelp != "") os << " (" << arghelp << ")";
        os << std::endl;
    }
}

}
