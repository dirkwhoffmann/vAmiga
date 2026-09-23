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
#include "httplib.h"
#include "utl/support.h"
#include <thread>

namespace vamiga {

using nlohmann::json;

void
RpcServer::_initialize()
{
    // The RPC server executes commands through the standard shell, the same
    // one the GUI uses. InputLine::isRpcCommand() lets didExecute() tell
    // RPC-issued commands apart from ones typed by the user or another
    // server sharing the same shell.
    retroShell.console.delegates.push_back(this);
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

        case TransportProtocol::STDIO: return stdio;
        case TransportProtocol::TCP:   return tcp;
        case TransportProtocol::HTTP:  return http;

        default:
            fatalError;
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
    switch (protocol) {

        case TransportProtocol::STDIO:  return true;
        case TransportProtocol::TCP:    return true;
        case TransportProtocol::HTTP:   return true;

        default:
            return false;
    }
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

    if (auto response = process(trimmed, false); response) {

        send(*response);
    }
}

void
RpcServer::didReceive(const httplib::Request &req, httplib::Response &res)
{
    if (auto response = process(req.body, true); response) {
        res.set_content(*response, "text/plain");
    }
}

optional<string>
RpcServer::process(const string &payload, bool blocking)
{
    try {

        json request = json::parse(payload);

        /* Check the envelope. Only 'method' is required of every packet:
         * 'params' is checked inside the branch that needs it, because what
         * a method takes is the method's own business. Demanding it here
         * would reject the parameterless app-level notifications that are
         * none of the core's concern (Silicium's "svmChanged", say) before
         * the fall-through below ever gets to ignore them.
         */
        if (!request.contains("method")) {
            throw utl::Error(RPC::INVALID_REQUEST, "Missing 'method'");
        }
        if (!request["method"].is_string()) {
            throw utl::Error(RPC::INVALID_PARAMS, "'method' must be a string");
        }
        if (request["method"] == "retroshell") {

            if (!request.contains("params")) {
                throw utl::Error(RPC::INVALID_REQUEST, "Missing 'params'");
            }
            if (!request["params"].is_string()) {
                throw utl::Error(RPC::INVALID_PARAMS, "'params' must be a string");
            }

            auto id = request.value("id", 0);

            if (blocking) {
                return execBlocking(request["params"], id);
            } else {
                return execNonBlocking(request["params"], id);
            }
        }

        /* Any other method is not handled by the core. Such packets are
         * app-level notifications (e.g., Silicium's "prefsChanged"), which
         * the app processes by observing the traffic log (Msg::SRV_RECEIVE).
         * The core sends no response for them, matching the JSON-RPC rule
         * that notifications (requests without an "id") are never answered.
         */
        return { };

    } catch (const json::parse_error &) {

        json response = {

            {"jsonrpc", "2.0"},
            {"error", {{"code", RPC::PARSE_ERROR}, {"message", "Parse error: " + payload}}},
            {"id", nullptr}
        };
        return response.dump();

    } catch (const utl::Error &e) {

        /* utl::Error, not CoreError: the codes above are JSON-RPC wire codes
         * (-32600 and friends), which have nothing to do with the core's own
         * fault numbering. Routing them through CoreError landed them in its
         * default branch, which discards the message it was given and reports
         * "CoreError -32600 (???)." instead -- throwing away the one piece of
         * information the client needs. CoreError derives from utl::Error, so
         * a fault escaping a blocking retroshell call is still caught here,
         * and still reports its own message and fault number.
         */
        json response = {

            {"jsonrpc", "2.0"},
            {"error", {{"code", e.payload}, {"message", e.what()}}},
            {"id", nullptr}
        };
        return response.dump();
    }
}

optional<string>
RpcServer::execNonBlocking(const string &command, isize id)
{
    // Feed the command into the command queue and return a nullopt
    retroShell.asyncExec(InputLine {

        .id = id,
        .type = InputLine::Source::RPC,
        .input = command
    });

    return { };
}

optional<string>
RpcServer::execBlocking(const string &command, isize id)
{
    // To block the caller, we pass a promise to RetroShell
    auto p = std::make_shared<std::promise<string>>();
    auto future = p->get_future();

    // Feed the command, with the promise attached, into the command queue
    retroShell.asyncExec(InputLine {

        .id = id,
        .type = InputLine::Source::RPC,
        .input = command,
        .promise = p
    });

    // Wait until the promise gets fulfilled
    return future.get();
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

    // If a promise is attached, fulfill it
    if (input.promise) { input.promise->set_value(response.dump()); }

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

    // If a promise is attached, fulfill it
    if (input.promise) { input.promise->set_value(response.dump()); }

    send(response.dump());
}

}
