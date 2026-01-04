// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CoreObjectTypes.h"
#include "CoreError.h"
#include "utl/abilities/Loggable.h"
#include "utl/abilities/Reportable.h"

namespace vamiga {

class CoreObject : public Loggable, Reportable {

    //
    // Initializing
    //
    
public:
    
    CoreObject() = default;
    virtual ~CoreObject() = default;
    
    // Returns the name for this component
    virtual const char *objectName() const = 0;
    
    // Returns a textual description for this component
    virtual const char *description() const { return ""; }
    
    // Called by loginfo() and logdebug() to produce a detailed debug output
    virtual string prefix(LogLevel, const std::source_location &) const override;

    
    //
    // Reporting state
    //

protected:

    const Report makeReport(isize category) const override { return {{ "Name", objectName() }}; }


    //
    // Dumping state (deprecated, will be replaced by 'Reportable' API)
    //

public:

    virtual void _dump(Category category, std::ostream &ss) const { }
    
    void dump(Category category, std::ostream &ss) const;
    void dump(Category category) const;
};

}
