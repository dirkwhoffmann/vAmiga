// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
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
    retroShell.commander.delegates.push_back(this);
    retroShell.debugger.delegates.push_back(this);
}


void
RpcServer::_dump(Category category, std::ostream &os) const
{
    RemoteServer::_dump(category, os);
}

void
RpcServer::didStart()
{
    if (config.verbose) {

        *this << "Remote server is listening at port " << config.port << "\n";
    }
}

string
RpcServer::doReceive()
{
    string payload = connection.recv();

    // Remove LF and CR (if present)
    payload = utl::rtrim(payload, "\n\r");

    if (config.verbose) {

        retroShell << "R: " << utl::makePrintable(payload) << "\n";
        printf("R: %s\n", utl::makePrintable(payload).c_str());
    }

    return payload;
}

void
RpcServer::doSend(const string &payload)
{
    connection.send(payload);

    if (config.verbose) {

        retroShell << "T: " << utl::makePrintable(payload) << "\n";
        printf("T: %s\n", utl::makePrintable(payload).c_str());
    }
}

void
RpcServer::doProcess(const string &payload)
{
    try {

        json request = json::parse(payload);

        // Check input format
        if (!request.contains("method")) {
            throw AppError(RPC::INVALID_REQUEST, "Missing 'method'");
        }
        if (!request.contains("params")) {
            throw AppError(RPC::INVALID_REQUEST, "Missing 'params'");
        }
        if (!request["method"].is_string()) {
            throw AppError(RPC::INVALID_PARAMS, "'method' must be a string");
        }
        if (!request["params"].is_string()) {
            throw AppError(RPC::INVALID_PARAMS, "'params' must be a string");
        }
        if (request["method"] != "retroshell") {
            throw AppError(RPC::INVALID_PARAMS, "method  must be 'retroshell'");
        }

        // Feed the command into the command queue
        retroShell.asyncExec(InputLine {

            .id = request.value("id", 0),
            .type = InputLine::Source::RPC,
            .input = request["params"] });

    } catch (const json::parse_error &) {

        json response = {

            {"jsonrpc", "2.0"},
            {"error", {{"code", RPC::PARSE_ERROR}, {"message", "Parse error: " + payload}}},
            {"id", nullptr}
        };
        send(response.dump());

    } catch (const AppError &e) {

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
    if (const auto *error = dynamic_cast<const AppError *>(&exc)) {
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
