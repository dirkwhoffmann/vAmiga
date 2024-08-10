// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "OptionTypes.h"
#include <vector>

namespace vamiga {

typedef std::vector<Option> ConfigOptions;

class Configurable 
{

public:

    virtual ~Configurable() = default;

    // Returns the available config options
    virtual const ConfigOptions &getOptions() const = 0;

    // Returns true iff a specific option is available
    bool isValidOption(Option opt) const;

    // Gets a config option
    virtual i64 getOption(Option opt) const { return 0; }

    // Gets the fallback for a config option
    virtual i64 getFallback(Option opt) const = 0;

    // Throws an exception of if the given option/value pair is invalid
    virtual void checkOption(Option opt, i64 value) { }
    void checkOption(Option opt, const string &value);
    void checkOption(const string &opt, const string &value);

    // Sets a config option
    virtual void setOption(Option opt, i64 value) { }
    void setOption(Option opt, const string &value);
    void setOption(const string &opt, const string &value);

    // Resets all config options
    void resetConfig(const class Defaults &defaults, isize objid = 0);

    // Dumps the current configuration
    void dumpConfig(std::ostream& os) const;
};

}
