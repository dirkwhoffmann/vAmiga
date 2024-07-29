// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiagBoardTypes.h"
#include "ZorroBoard.h"
#include "Memory.h"

namespace vamiga {

class DiagBoard : public ZorroBoard {

    // Current configuration
    DiagBoardConfig config = {};
    
    // Rom code
    Buffer<u8> rom;

    // Transmitted pointers
    u32 pointer1 = 0;
    u32 pointer2 = 0;

    // List of tracked tasks
    std::vector <u32> tasks;
    
    // Names of tasks to catch
    std::vector <string> targets;
    
    
    //
    // Initializing
    //
    
public:
    
    using ZorroBoard::ZorroBoard;
    DiagBoard& operator= (const DiagBoard& other) { return *this; }


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

    } SERIALIZERS(serialize);

    void _didReset(bool hard) override;

    
    //
    // Methods from Configurable
    //

public:

    const DiagBoardConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option option) const override;
    void checkOption(Option opt, i64 value) override;
    void setOption(Option option, i64 value) override;
    
    
    //
    // Methods from ZorroBoard
    //
    
public:
    
    virtual bool pluggedIn() const override;
    virtual isize pages() const override         { return 1; }
    virtual u8 type() const override             { return ERT_ZORROII | ERTF_DIAGVALID; }
    virtual u8 product() const override          { return 0x77; }
    virtual u8 flags() const override            { return 0x00; }
    virtual u16 manufacturer() const override    { return 0x0539; }
    virtual u32 serialNumber() const override    { return 16180; }
    virtual u16 initDiagVec() const override     { return 0x40; }
    virtual string vendorName() const override   { return "RASTEC"; }
    virtual string productName() const override  { return "Diag Board"; }
    virtual string revisionName() const override { return "0.1"; }

private:
    
    void updateMemSrcTables() override;
    

    //
    // Accessing the board
    //

public:

    u8 peek8(u32 addr) override;
    u16 peek16(u32 addr) override;
    u8 spypeek8(u32 addr) const override;
    u16 spypeek16(u32 addr) const override;
    void poke8(u32 addr, u8 value) override;
    void poke16(u32 addr, u16 value) override;

private:
    
    void processInit(u32 ptr1);
    void processAddTask(u32 ptr1);
    void processRemTask(u32 ptr1);
    void processLoadSeg(u32 ptr1, u32 ptr2, bool bstr);

    
    //
    // Using the board
    //
    
public:
    
    // Pauses emulation when the specified task launches
    void catchTask(const string &name);
};

}
