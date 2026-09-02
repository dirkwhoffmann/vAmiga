// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "RpcServer.h"
#include "Emulator.h"
#include "json.h"
#include "utl/support.h"
#include <thread>

namespace vamiga {

using nlohmann::json;

void
RpcServer::_initialize()
{
    // The RPC server drives its own shell, independent of the GUI's one
    rpcShell.console.delegates.push_back(this);
}

void
RpcServer::_dump(Category category, std::ostream &os) const
{
    RemoteServer::_dump(category, os);
}

Transport &
RpcServer::transport()
{
    switch (config.transport) {

        case TransportProtocol::TCP: return tcp;

        default:
            return tcp;
            // fatalError;
    }
}

const Transport &
RpcServer::transport() const
{
    return const_cast<RpcServer *>(this)->transport();
}

bool
RpcServer::isSupported(TransportProtocol protocol) const
{
    return protocol == TransportProtocol::TCP;
}

void
RpcServer::didConnect()
{
    // Hand the client a fresh shell
    rpcShell.newSession();
}

void
RpcServer::didDisconnect()
{
    // Discard the client's session
    rpcShell.newSession();
}

void
RpcServer::didStart()
{
    if (config.verbose) {

        *this << "Remote server is listening at port " << config.port << "\n";
    }
}

void
RpcServer::send(const string &payload)
{
    transport().send(payload);

    if (config.verbose) {

        retroShell << "T: " << utl::makePrintable(payload) << "\n";
        printf("T: %s\n", utl::makePrintable(payload).c_str());
    }
}

void
RpcServer::didReceive(const string &payload)
{
    // Remove LF and CR (if present)
    auto trimmed = utl::rtrim(payload, "\n\r");

    if (config.verbose) {

        retroShell << "R: " << utl::makePrintable(trimmed) << "\n";
        printf("R: %s\n", utl::makePrintable(trimmed).c_str());
    }

    try {

        json request = json::parse(trimmed);

        // Check input format
        if (!request.contains("method")) {
            throw CoreError(RPC::INVALID_REQUEST, "Missing 'method'");
        }
        if (!request.contains("params")) {
            throw CoreError(RPC::INVALID_REQUEST, "Missing 'params'");
        }
        if (!request["method"].is_string()) {
            throw CoreError(RPC::INVALID_PARAMS, "'method' must be a string");
        }
        if (!request["params"].is_string()) {
            throw CoreError(RPC::INVALID_PARAMS, "'params' must be a string");
        }
        if (request["method"] != "retroshell") {
            throw CoreError(RPC::INVALID_PARAMS, "method  must be 'retroshell'");
        }

        // Feed the command into the command queue
        rpcShell.asyncExec(InputLine {

            .id = request.value("id", 0),
            .type = InputLine::Source::RPC,
            .input = request["params"] });

    } catch (const json::parse_error &) {

        json response = {

            {"jsonrpc", "2.0"},
            {"error", {{"code", RPC::PARSE_ERROR}, {"message", "Parse error: " + trimmed}}},
            {"id", nullptr}
        };
        send(response.dump());

    } catch (const CoreError &e) {

        json response = {

            {"jsonrpc", "2.0"},
            {"error", {{"code", e.payload}, {"message", e.what()}}},
            {"id", nullptr}
        };
        send(response.dump());
    }
}

void
RpcServer::willExecute(const InputLine &input)
{

}

void
RpcServer::didExecute(const InputLine& input, std::stringstream &ss)
{
    if (!input.isRpcCommand()) return;

    json response = {

        {"jsonrpc", "2.0"},
        {"result", ss.str()},
        {"id", input.id}
    };

    send(response.dump());
}

void
RpcServer::didExecute(const InputLine& input, std::stringstream &ss, std::exception &exc)
{
    if (!input.isRpcCommand()) return;

    // By default, signal an internal error
    i64 code = -32603;

    // For parse errors, use a value from the server-defined error range
    if (dynamic_cast<const utl::ParseError *>(&exc)) {
        code = -32000;
    }

    // For application errors, use the fault identifier
    if (const auto *error = dynamic_cast<const CoreError *>(&exc)) {
        code = i64(error->fault());
    }

    json response = {

        {"jsonrpc", "2.0"},
        {"error", {
            {"code", code},
            {"message", exc.what()}
        }},
        {"id", input.id}
    };

    send(response.dump());
}

}
