// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ZorroBoard.h"

namespace vamiga {

class RamExpansion : public ZorroBoard {

    //
    // Constructing
    //
    
public:
    
    using ZorroBoard::ZorroBoard;
    
    RamExpansion& operator= (const RamExpansion& other) {

        CLONE(baseAddr)
        CLONE(state)

        return *this;
    }


    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from CoreComponent
    //
    
private:
        
    template <class T>
    void serialize(T& worker)
    {
        if (isSoftResetter(worker)) return;

        worker

        << baseAddr
        << state;

    } SERIALIZERS(serialize, override);

    void _didReset(bool hard) override;
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


    //
    // Methods from ZorroBoard
    //

    virtual bool pluggedIn() const override;
    virtual isize pages() const override;
    virtual u8 type() const override             { return ERT_ZORROII | ERTF_MEMLIST; }
    virtual u8 product() const override          { return 0x67; }
    virtual u8 flags() const override            { return 0x80; }
    virtual u16 manufacturer() const override    { return 0x07B9; }
    virtual u32 serialNumber() const override    { return 27182; }
    virtual u16 initDiagVec() const override     { return 0x0; }
    virtual string vendorName() const override   { return "VAMIGA"; }
    virtual string productName() const override  { return "RAM Expansion board"; }
    virtual string revisionName() const override { return "1.0"; }

    void updateMemSrcTables() override;
};

}
