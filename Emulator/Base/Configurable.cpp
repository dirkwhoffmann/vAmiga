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
#include "Reflection.h"
#include "Defaults.h"
#include <algorithm>

namespace vamiga {

using namespace util;

const ConfigOptions Configurable::options = { };

bool
Configurable::isValidOption(Option opt) const
{
    for (auto &it: getOptions()) {
        if (it == opt) return true;
    }
    return false;
}

void
Configurable::checkOption(Option opt, const string &value)
{
    checkOption(opt, OptionParser::parse(opt, value));
}

void
Configurable::checkOption(const string &opt, const string &value)
{
    checkOption(Option(util::parseEnum<OptionEnum>(opt)), value);
}

void
Configurable::setOption(Option opt, const string &value)
{
    setOption(opt, OptionParser::parse(opt, value));
}

void
Configurable::setOption(const string &opt, const string &value)
{
    setOption(Option(util::parseEnum<OptionEnum>(opt)), value);
}

void
Configurable::resetConfig(const Defaults &defaults, isize objid)
{
    for (auto &option : getOptions()) {
        setOption(option, defaults.get(option, objid));
    }
}

void
Configurable::dumpConfig(std::ostream& os) const
{
    using namespace util;

    for (auto &opt: getOptions()) {

        auto name = OptionEnum::plainkey(opt);
        auto help = OptionEnum::help(opt);
        auto arg  = OptionParser::asString(opt, getOption(opt));

        os << tab(name);
        os << std::setw(16) << std::left << std::setfill(' ') << arg;
        os <<help << std::endl;
    }
}

string 
Configurable::keyList() 
{
    return OptionEnum::keyList([&](Option i) { return isValidOption(i); });
}

string
Configurable::argList() 
{
    return OptionEnum::argList([&](Option i) { return isValidOption(i); });
}

}
