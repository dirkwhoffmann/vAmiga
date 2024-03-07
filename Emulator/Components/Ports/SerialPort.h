// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SerialPortTypes.h"
#include "SubComponent.h"

namespace vamiga {

#define TXD_MASK (1 << 2)
#define RXD_MASK (1 << 3)
#define RTS_MASK (1 << 4)
#define CTS_MASK (1 << 5)
#define DSR_MASK (1 << 6)
#define CD_MASK  (1 << 8)
#define DTR_MASK (1 << 20)
#define RI_MASK  (1 << 22)

class SerialPort : public SubComponent {

    friend class UART;
    
    // Current configuration
    SerialPortConfig config = {};

    // Result of the latest inspection
    mutable SerialPortInfo info = {};

    // The current values of the port pins
    u32 port = 0;

    // Temporary storage for incoming and outgoing bytes
    std::u16string incoming;
    std::u16string outgoing;


    //
    // Initializing
    //

public:

    using SubComponent::SubComponent;

    
    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override { return "SerialPort"; }
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //
    
private:
    
    void _reset(bool hard) override;
    void _inspect() const override;
    
    template <class T>
    void serialize(T& worker)
    {
        worker

        << port;

        if (util::isResetter(worker)) return;

        worker

        << config.device;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Configuring
    //
    
public:

    const SerialPortConfig &getConfig() const { return config; }
    void resetConfig() override;

    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    

    //
    // Analyzing
    //
    
public:

    SerialPortInfo getInfo() const { return CoreComponent::getInfo(info); }


    //
    // Accessing port pins
    //

public:

    // Reads the current value of a certain port pin
    bool getPin(isize nr) const;

    // Modifies the value of a certain port pin
    void setPin(isize nr, bool value);

    // Convenience wrappers
    bool getTXD() const { return getPin(2); }
    bool getRXD() const { return getPin(3); }
    bool getRTS() const { return getPin(4); }
    bool getCTS() const { return getPin(5); }
    bool getDSR() const { return getPin(6); }
    bool getCD () const { return getPin(8); }
    bool getDTR() const { return getPin(20); }
    bool getRI () const { return getPin(22); }

    void setTXD(bool value) { setPin(2, value); }
    void setRXD(bool value) { setPin(3, value); }
    void setRTS(bool value) { setPin(4, value); }
    void setCTS(bool value) { setPin(5, value); }
    void setDSR(bool value) { setPin(6, value); }
    void setCD (bool value) { setPin(8, value); }
    void setDTR(bool value) { setPin(20, value); }
    void setRI (bool value) { setPin(22, value); }

private:

    void setPort(u32 mask, bool value);


    //
    // Accessing the byte buffers
    //

public:

    // Reads and removes the contents of one of the record buffers
    std::u16string readIncoming();
    std::u16string readOutgoing();

    // Reads and removes a single byte from one of the record buffers
    int readIncomingByte();
    int readOutgoingByte();
    int readIncomingPrintableByte();
    int readOutgoingPrintableByte();

    // Feed a string into the UART
    void operator<<(char c);
    void operator<<(const string &s);

private:

    // Called by the UART when a byte has been received or sent
    void recordIncomingByte(int byte);
    void recordOutgoingByte(int byte);

    // Dumps a byte to RetroShell
    void dumpByte(int byte);
};

}
