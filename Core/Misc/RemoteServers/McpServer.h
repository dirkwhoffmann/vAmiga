// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SocketServer.h"
#include "json.h"
#include <optional>

namespace vamiga {

class McpServer final : public SocketServer {

public:

    using SocketServer::SocketServer;

    McpServer& operator= (const McpServer& other) {

        SocketServer::operator = (other);
        rxBuffer = other.rxBuffer;
        inString = other.inString;
        escaped = other.escaped;
        braceStack = other.braceStack;
        start = other.start;
        scanPos = other.scanPos;
        return *this;
    }


    //
    // Methods from CoreObject
    //

protected:

    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from RemoteServer
    //

public:

    bool canRun() override { return true; }
    void didStart() override;
    void didDisconnect() override;


    //
    // Methods from SocketServer
    //

private:

    string doReceive() override;
    void doProcess(const string &packet) override;
    void doSend(const string &packet) override;


    //
    // MCP handlers
    //

private:

    string rxBuffer;
    bool inString = false;
    bool escaped = false;
    string braceStack;
    usize start = 0;
    usize scanPos = 0;

    std::optional<string> extractMessage();
    std::optional<string> processMessage(const string &message, bool &disconnectClient);

    string createErrorResponse(const nlohmann::json &id, i64 code, const string &msg) const;
    string createSuccessResponse(const nlohmann::json &id, const nlohmann::json &result) const;

    string handleInitialize(const nlohmann::json &req);
    string handleListTools(const nlohmann::json &req);
    string handleCallTool(const nlohmann::json &req);
    string handleShutdown(const nlohmann::json &req) const;

    string handleListResources(const nlohmann::json &req) const;
    string handleReadResource(const nlohmann::json &req);
    string handleListPrompts(const nlohmann::json &req) const;
    string handleGetPrompt(const nlohmann::json &req);

    static string base64Encode(const u8 *data, usize size);
    static bool tryGetU32(const nlohmann::json &args, const char *key, u32 &value);
};

}
