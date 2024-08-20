// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HdControllerTypes.h"
#include "ZorroBoard.h"
#include "HDFFile.h"
#include "RomFileTypes.h"

namespace vamiga {

class HdController : public ZorroBoard, public Inspectable<HdcInfo, HdcStats> {

    Descriptions descriptions = {
        {
            .type           = HdControllerClass,
            .name           = "HdController0",
            .description    = "Hard Drive Controller 0",
            .shell          = ""
        },
        {
            .type           = HdControllerClass,
            .name           = "HdController1",
            .description    = "Hard Drive Controller 1",
            .shell          = ""
        },
        {
            .type           = HdControllerClass,
            .name           = "HdController2",
            .description    = "Hard Drive Controller 2",
            .shell          = ""
        },
        {
            .type           = HdControllerClass,
            .name           = "HdController3",
            .description    = "Hard Drive Controller 3",
            .shell          = ""
        }
    };

    ConfigOptions options = {

        OPT_HDC_CONNECT
    };

    // The hard drive this controller is connected to
    HardDrive &drive;

    // Current configuration
    HdcConfig config = {};
    
    // The current controller state
    HdcState hdcState = HDC_UNDETECTED;
    
    // Rom code
    Buffer<u8> rom;
    
    // Number of initialized partitions
    isize numPartitions = 0;

    // Transmitted pointer
    u32 pointer = 0;


    //
    // Methods
    //
    
public:
    
    HdController(Amiga& ref, HardDrive& hdr);

    HdController& operator= (const HdController& other) {

        CLONE(baseAddr)
        CLONE(state)

        CLONE(config)

        CLONE(hdcState)
        CLONE(rom)
        CLONE(numPartitions)
        CLONE(pointer)

        return *this;
    }


    //
    // Methods from Serializable
    //
    
private:

    template <class T>
    void serialize(T& worker)
    {
        if (isSoftResetter(worker)) return;

        worker

        << baseAddr
        << state
        << hdcState
        << numPartitions
        << pointer;

        if (isResetter(worker)) return;

        worker

        << config.connected;

    } SERIALIZERS(serialize);

    void _didReset(bool hard) override;
    

    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(HdcInfo &result) const override;
    void cacheStats(HdcStats &result) const override;


    //
    // Methods from ZorroBoard
    //
    
public:
    
    virtual bool pluggedIn() const override;
    virtual isize pages() const override         { return 1; }
    virtual u8 type() const override             { return ERT_ZORROII | ERTF_DIAGVALID; }
    virtual u8 product() const override          { return 0x88; }
    virtual u8 flags() const override            { return 0x00; }
    virtual u16 manufacturer() const override    { return 0x0539; }
    virtual u32 serialNumber() const override    { return 31415 + u32(objid); }
    virtual u16 initDiagVec() const override     { return 0x40; }
    virtual string vendorName() const override   { return "RASTEC"; }
    virtual string productName() const override  { return "HD controller"; }
    virtual string revisionName() const override { return "0.3"; }

private:
    
    void updateMemSrcTables() override;
    
    
    //
    // Methods from Configurable
    //

public:

    const HdcConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option option) const override;
    void checkOption(Option opt, i64 value) override;
    void setOption(Option option, i64 value) override;

    
    //
    // Analyzing
    //
    
public:
        
    // Returns the current controller state
    HdcState getHdcState() const { return hdcState; }

    // Informs whether the controller is compatible with a certain Kickstart
    bool isCompatible(u32 crc32) const;
    bool isCompatible() const;

private:
    
    void resetHdcState();
    void changeHdcState(HdcState newState);

    
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
    
    void processCmd(u32 ptr);
    void processInit(u32 ptr);
    void processResource(u32 ptr);
    void processInfoReq(u32 ptr);
    void processInitSeg(u32 ptr);
};

}
