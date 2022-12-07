// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "RemoteManagerTypes.h"
#include "SerServer.h"
#include "RshServer.h"
#include "GdbServer.h"

namespace vamiga {

class RemoteManager : public SubComponent {

public:
    
    // The remote servers
    SerServer serServer = SerServer(amiga);
    RshServer rshServer = RshServer(amiga);
    GdbServer gdbServer = GdbServer(amiga);
    
    // Convenience wrapper
    std::vector <RemoteServer *> servers = {
        &serServer, &rshServer, &gdbServer
    };

    
    //
    // Initializing
    //
    
public:
    
    RemoteManager(Amiga& ref);
    // ~RemoteManager();
    
    
    //
    // Methods from AmigaObject
    //
    
protected:
    
    const char *getDescription() const override { return "RemoteManager"; }
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override { }
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }
    
    
    //
    // Configuring
    //
    
public:

    i64 getConfigItem(Option option, long id) const;
    void setConfigItem(Option option, i64 value);
    void setConfigItem(Option option, long id, i64 value);

    
    //
    // Managing connections
    //
    
public:
    
    // Returns the number of servers being in a certain state
    isize numLaunching() const;
    isize numListening() const;
    isize numConnected() const;
    isize numErroneous() const;


    //
    // Servicing events
    //
    
public:
    
    void serviceServerEvent();
};

}
