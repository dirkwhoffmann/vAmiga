// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RemoteServer.h"

class RshServer : public RemoteServer {
  
public:
    
    using RemoteServer::RemoteServer;
    
    //
    // Methods from AmigaObject
    //
    
protected:
    
    const char *getDescription() const override { return "RshServer"; }
    void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Methods from RemoteServer
    //
    
    isize _defaultPort() const override { return 8081; }
    bool _launchable() override { return true; }
    void didConnect() override;
    string _receive() override throws;
    void _process(const string &packet) override throws;
    void _send(const string &packet) override throws;
};
