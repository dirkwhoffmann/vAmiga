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

class RamExpansion : public ZorroBoard {
            
    //
    // Constructing
    //
    
public:
    
    using ZorroBoard::ZorroBoard;
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "RamExpansion"; }
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {

    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << state
            << baseAddr;
        }
    }
    
    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Methods from ZorroBoard
    //
        
    virtual bool pluggedIn() const override;
    virtual isize pages() const override;
    virtual u8 type() const override             { return ERT_ZORROII | ERTF_MEMLIST; }
    virtual u8 product() const override          { return 0x67; }
    virtual u8 flags() const override            { return 0x80; }
    virtual u16 manufacturer() const override    { return 0x07B9; }
    virtual u32 serialNumber() const override    { return 2718281; }
    virtual u16 initDiagVec() const override     { return 0x0; }
    virtual string vendorName() const override   { return "VAMIGA"; }
    virtual string productName() const override  { return "RAM Expansion board"; }
    virtual string revisionName() const override { return "1.0"; }

    void updateMemSrcTables() override;
};
