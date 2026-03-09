// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "McpServer.h"
#include "Agnus.h"
#include "CPU.h"
#include "Emulator.h"
#include "Memory.h"
#include "Paula.h"
#include "RetroShell.h"
#include "VideoPort.h"
#include "Texture.h"
#include "Colors.h"
#include "Constants.h"
#include "utl/support.h"
#include "utl/support/Strings.h"
#include <iomanip>
#include <limits>
#include <sstream>

namespace vamiga {

using nlohmann::json;

namespace {

constexpr u32 MAX_AMIGA_ADDRESS = 0x01000000;
constexpr u32 MAX_RW_BYTES = 65536;
constexpr u32 MAX_DISASSEMBLY = 10000;

}

void
McpServer::_dump(Category category, std::ostream &os) const
{
    RemoteServer::_dump(category, os);
}

void
McpServer::didStart()
{
    if (config.verbose) {
        *this << "MCP server is listening at port " << config.port << "\n";
    }
}

void
McpServer::didDisconnect()
{
    rxBuffer.clear();
}

string
McpServer::doReceive()
{
    auto payload = connection.recv();

    if (config.verbose) {
        retroShell << "R: " << utl::makePrintable(payload) << "\n";
    }

    return payload;
}

void
McpServer::doSend(const string &payload)
{
    connection.send(payload);

    if (config.verbose) {
        retroShell << "T: " << utl::makePrintable(payload) << "\n";
    }
}

void
McpServer::doProcess(const string &packet)
{
    rxBuffer += packet;

    // Basic DoS protection in case a client sends endless garbage.
    if (rxBuffer.size() > 10 * 1024 * 1024) {
        disconnect();
        return;
    }

    while (auto msg = extractMessage()) {

        bool disconnectClient = false;
        auto response = processMessage(*msg, disconnectClient);

        if (response && !response->empty()) {
            send(*response + "\n");
        }

        if (disconnectClient) {
            disconnect();
            return;
        }
    }
}

std::optional<string>
McpServer::extractMessage()
{
    bool inString = false;
    bool escaped = false;
    isize depth = 0;
    usize start = 0;

    for (usize i = 0; i < rxBuffer.size(); i++) {

        const char c = rxBuffer[i];

        if (inString) {

            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                inString = false;
            }
            continue;
        }

        if (c == '"') {
            inString = true;
            continue;
        }

        if (c == '{') {
            if (depth == 0) start = i;
            depth++;
            continue;
        }

        if (c == '}') {
            if (depth > 0) depth--;

            if (depth == 0) {

                string message = rxBuffer.substr(start, i - start + 1);
                rxBuffer.erase(0, i + 1);
                return message;
            }
        }
    }

    return std::nullopt;
}

std::optional<string>
McpServer::processMessage(const string &message, bool &disconnectClient)
{
    disconnectClient = false;

    try {

        auto req = json::parse(message);

        if (!req.contains("method") || !req["method"].is_string()) {

            if (req.contains("id")) {
                return createErrorResponse(req["id"], -32600, "Invalid Request");
            }
            return std::nullopt;
        }

        auto method = req["method"].get<string>();

        if (method == "initialize") {
            return handleInitialize(req);
        }
        if (method == "mcp/server/tools/list" || method == "tools/list") {
            return handleListTools(req);
        }
        if (method == "mcp/server/tools/call" || method == "tools/call") {
            return handleCallTool(req);
        }
        if (method == "shutdown") {
            return handleShutdown(req);
        }
        if (method == "exit") {
            disconnectClient = true;
            return std::nullopt;
        }
        if (method == "$/cancelRequest" || method == "notifications/initialized") {
            return std::nullopt;
        }
        if (method == "resources/list") {
            return handleListResources(req);
        }
        if (method == "resources/read") {
            return handleReadResource(req);
        }
        if (method == "prompts/list") {
            return handleListPrompts(req);
        }
        if (method == "prompts/get") {
            return handleGetPrompt(req);
        }
        if (method == "ping") {
            return createSuccessResponse(req.value("id", json(nullptr)), json::object());
        }

        if (req.contains("id")) {
            return createErrorResponse(req["id"], -32601, "Method not found");
        }

    } catch (const json::exception &e) {

        return createErrorResponse(json(nullptr), -32700, "Parse error: " + string(e.what()));
    }

    return std::nullopt;
}

string
McpServer::createErrorResponse(const json &id, i64 code, const string &msg) const
{
    json res = {

        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", {{"code", code}, {"message", msg}}}
    };

    return res.dump();
}

string
McpServer::createSuccessResponse(const json &id, const json &result) const
{
    json res = {

        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", result}
    };

    return res.dump();
}

string
McpServer::handleInitialize(const json &req)
{
    json result = {

        {"protocolVersion", "2024-11-05"},
        {"capabilities", {{"tools", json::object()}}},
        {"serverInfo", {{"name", "vAmiga-MCP"}, {"version", "1.0.0"}}}
    };

    return createSuccessResponse(req.value("id", json(nullptr)), result);
}

string
McpServer::handleListTools(const json &req)
{
    const json tools = json::array({
        {
            {"name", "get_status"},
            {"description", "Get the current running status of the emulator."},
            {"inputSchema", {{"type", "object"}, {"properties", json::object()}}}
        },
        {
            {"name", "pause"},
            {"description", "Pause the emulator execution."},
            {"inputSchema", {{"type", "object"}, {"properties", json::object()}}}
        },
        {
            {"name", "resume"},
            {"description", "Resume the emulator execution."},
            {"inputSchema", {{"type", "object"}, {"properties", json::object()}}}
        },
        {
            {"name", "hard_reset"},
            {"description", "Perform a hard reset."},
            {"inputSchema", {{"type", "object"}, {"properties", json::object()}}}
        },
        {
            {"name", "read_memory"},
            {"description", "Reads bytes from memory as a hex dump."},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {{"address", {{"type", "integer"}}}, {"size", {{"type", "integer"}}}}},
                {"required", json::array({"address", "size"})}
            }}
        },
        {
            {"name", "write_memory"},
            {"description", "Writes an array of bytes to memory."},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {{"address", {{"type", "integer"}}}, {"data", {{"type", "array"}}}}},
                {"required", json::array({"address", "data"})}
            }}
        },
        {
            {"name", "get_registers"},
            {"description", "Returns CPU and custom chip register state."},
            {"inputSchema", {{"type", "object"}, {"properties", json::object()}}}
        },
        {
            {"name", "set_register"},
            {"description", "Sets a CPU register (PC, SR, USP, ISP, MSP, D0-D7, A0-A7)."},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {{"name", {{"type", "string"}}}, {"value", {{"type", "integer"}}}}},
                {"required", json::array({"name", "value"})}
            }}
        },
        {
            {"name", "set_breakpoint"},
            {"description", "Sets a CPU breakpoint at address."},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {{"address", {{"type", "integer"}}}}},
                {"required", json::array({"address"})}
            }}
        },
        {
            {"name", "remove_breakpoint"},
            {"description", "Removes a CPU breakpoint at address."},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {{"address", {{"type", "integer"}}}}},
                {"required", json::array({"address"})}
            }}
        },
        {
            {"name", "set_watchpoint"},
            {"description", "Sets a CPU watchpoint at address."},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {{"address", {{"type", "integer"}}}}},
                {"required", json::array({"address"})}
            }}
        },
        {
            {"name", "remove_watchpoint"},
            {"description", "Removes a CPU watchpoint at address."},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {{"address", {{"type", "integer"}}}}},
                {"required", json::array({"address"})}
            }}
        },
        {
            {"name", "list_guards"},
            {"description", "Returns all breakpoints and watchpoints."},
            {"inputSchema", {{"type", "object"}, {"properties", json::object()}}}
        },
        {
            {"name", "disassemble"},
            {"description", "Disassembles 68000 instructions."},
            {"inputSchema", {
                {"type", "object"},
                {"properties", {{"address", {{"type", "integer"}}}, {"count", {{"type", "integer"}}}}},
                {"required", json::array({"address", "count"})}
            }}
        }
    });

    return createSuccessResponse(req.value("id", json(nullptr)), {{"tools", tools}});
}

string
McpServer::handleCallTool(const json &req)
{
    auto id = req.value("id", json(nullptr));

    if (!req.contains("params") || !req["params"].is_object()) {
        return createErrorResponse(id, -32602, "Invalid params");
    }

    auto params = req["params"];
    if (!params.contains("name") || !params["name"].is_string()) {
        return createErrorResponse(id, -32602, "Missing tool name");
    }

    const auto tool = params["name"].get<string>();
    const auto args = params.value("arguments", json::object());

    bool isError = false;
    string text;
    json payload = nullptr;

    try {

        if (tool == "get_status") {

            text = string("Emulator is ") + (isRunning() ? "running" : "paused") +
            ". Powered on: " + (isPoweredOn() ? "yes" : "no");

        } else if (tool == "pause") {

            emulator.pause();
            text = "Emulator paused.";

        } else if (tool == "resume") {

            emulator.run();
            text = "Emulator resumed.";

        } else if (tool == "hard_reset") {

            emulator.hardReset();
            text = "Emulator hard reset triggered.";

        } else if (tool == "read_memory") {

            u32 addr = 0;
            u32 size = 0;

            if (!tryGetU32(args, "address", addr) || !tryGetU32(args, "size", size) ||
                size > MAX_RW_BYTES || addr > MAX_AMIGA_ADDRESS || addr + size > MAX_AMIGA_ADDRESS) {

                isError = true;
                text = "Invalid address or size out of bounds (max size 64KB).";

            } else {

                text = mem.debugger.hexDump<Accessor::CPU>(addr, size);
            }

        } else if (tool == "write_memory") {

            u32 addr = 0;
            if (!tryGetU32(args, "address", addr) || !args.contains("data") || !args["data"].is_array()) {

                isError = true;
                text = "Invalid address or data format.";

            } else {

                const auto &data = args["data"];
                if (data.size() > MAX_RW_BYTES || addr > MAX_AMIGA_ADDRESS || addr + data.size() > MAX_AMIGA_ADDRESS) {

                    isError = true;
                    text = "Invalid address or sizes out of bounds (max 64KB).";

                } else {

                    for (usize i = 0; i < data.size(); i++) {

                        if (!data[i].is_number_integer()) {
                            isError = true;
                            text = "Data array must contain integers in range 0..255.";
                            break;
                        }

                        auto value = data[i].get<i64>();
                        if (value < 0 || value > 255) {
                            isError = true;
                            text = "Data array must contain integers in range 0..255.";
                            break;
                        }

                        mem.poke8<Accessor::CPU>(u32(addr + i), u8(value));
                    }

                    if (!isError) text = "Memory written successfully.";
                }
            }

        } else if (tool == "get_registers") {

            const auto cpuInfo = cpu.info.current();
            const auto agnusInfo = agnus.info.current();
            const auto paulaInfo = paula.info.current();

            payload = {
                {"CPU", {
                    {"PC", cpuInfo.pc0},
                    {"SR", cpuInfo.sr},
                    {"USP", cpuInfo.usp},
                    {"ISP", cpuInfo.isp},
                    {"MSP", cpuInfo.msp},
                    {"D", json::array({cpuInfo.d[0], cpuInfo.d[1], cpuInfo.d[2], cpuInfo.d[3], cpuInfo.d[4], cpuInfo.d[5], cpuInfo.d[6], cpuInfo.d[7]})},
                    {"A", json::array({cpuInfo.a[0], cpuInfo.a[1], cpuInfo.a[2], cpuInfo.a[3], cpuInfo.a[4], cpuInfo.a[5], cpuInfo.a[6], cpuInfo.a[7]})}
                }},
                {"Agnus", {
                    {"DMACON", agnusInfo.dmacon},
                    {"VPOS", agnusInfo.vpos},
                    {"HPOS", agnusInfo.hpos}
                }},
                {"Paula", {
                    {"INTREQ", paulaInfo.intreq},
                    {"INTENA", paulaInfo.intena}
                }}
            };

        } else if (tool == "set_register") {

            if (!args.contains("name") || !args["name"].is_string() || !args.contains("value") || !args["value"].is_number_integer()) {

                isError = true;
                text = "Invalid register name or value.";

            } else {

                auto name = utl::uppercased(args["name"].get<string>());
                auto value = u32(args["value"].get<i64>());

                if (name == "PC") cpu.setPC(value);
                else if (name == "USP") cpu.setUSP(value);
                else if (name == "ISP") cpu.setISP(value);
                else if (name == "MSP") cpu.setMSP(value);
                else if (name == "SR") cpu.setSR((u16)value);
                else if (name.size() == 2 && name[0] == 'D' && name[1] >= '0' && name[1] <= '7') cpu.setD(name[1] - '0', value);
                else if (name.size() == 2 && name[0] == 'A' && name[1] >= '0' && name[1] <= '7') cpu.setA(name[1] - '0', value);
                else {
                    isError = true;
                    text = "Invalid register name.";
                }

                if (!isError) text = "Register updated.";
            }

        } else if (tool == "set_breakpoint") {

            u32 addr = 0;
            if (!tryGetU32(args, "address", addr)) {
                isError = true;
                text = "Invalid address.";
            } else {
                cpu.breakpoints.setAt(addr);
                text = "Breakpoint set.";
            }

        } else if (tool == "remove_breakpoint") {

            u32 addr = 0;
            if (!tryGetU32(args, "address", addr)) {
                isError = true;
                text = "Invalid address.";
            } else {
                cpu.breakpoints.removeAt(addr);
                text = "Breakpoint removed.";
            }

        } else if (tool == "set_watchpoint") {

            u32 addr = 0;
            if (!tryGetU32(args, "address", addr)) {
                isError = true;
                text = "Invalid address.";
            } else {
                cpu.watchpoints.setAt(addr);
                text = "Watchpoint set.";
            }

        } else if (tool == "remove_watchpoint") {

            u32 addr = 0;
            if (!tryGetU32(args, "address", addr)) {
                isError = true;
                text = "Invalid address.";
            } else {
                cpu.watchpoints.removeAt(addr);
                text = "Watchpoint removed.";
            }

        } else if (tool == "list_guards") {

            json breakpoints = json::array();
            for (isize i = 0; i < cpu.breakpoints.elements(); i++) {
                if (auto info = cpu.breakpoints.guardNr(i); info) {
                    breakpoints.push_back({{"address", info->addr}, {"enabled", info->enabled}});
                }
            }

            json watchpoints = json::array();
            for (isize i = 0; i < cpu.watchpoints.elements(); i++) {
                if (auto info = cpu.watchpoints.guardNr(i); info) {
                    watchpoints.push_back({{"address", info->addr}, {"enabled", info->enabled}});
                }
            }

            payload = {
                {"breakpoints", breakpoints},
                {"watchpoints", watchpoints}
            };

        } else if (tool == "disassemble") {

            u32 addr = 0;
            u32 count = 0;

            if (!tryGetU32(args, "address", addr) || !tryGetU32(args, "count", count) || count == 0 || count > MAX_DISASSEMBLY) {

                isError = true;
                text = "Invalid instruction count (max 10000).";

            } else {

                std::stringstream ss;
                for (u32 i = 0; i < count; i++) {

                    isize len = 0;
                    auto instr = cpu.disassembleInstr(addr, &len);

                    ss << std::uppercase << std::setfill('0') << std::hex << std::setw(8) << addr;
                    ss << "    " << (instr ? instr : "") << "\n";

                    if (len <= 0) len = 2;
                    addr += u32(len);
                }
                text = ss.str();
            }

        } else {

            isError = true;
            text = "Tool not found.";
        }

    } catch (const std::exception &e) {

        isError = true;
        text = string("Error executing tool: ") + e.what();
    }

    json content;

    if (!payload.is_null()) {
        content = {{"type", "text"}, {"text", payload.dump(2)}};
    } else {
        content = {{"type", "text"}, {"text", text}};
    }

    return createSuccessResponse(id, {
        {"content", json::array({content})},
        {"isError", isError}
    });
}

string
McpServer::handleShutdown(const json &req) const
{
    return createSuccessResponse(req.value("id", json(nullptr)), json::object());
}

string
McpServer::handleListResources(const json &req) const
{
    json resources = json::array({
        {
            {"uri", "amiga://screen/current"},
            {"name", "Current Screen Buffer"},
            {"description", "A dump of the current Amiga screen as BMP."},
            {"mimeType", "image/bmp"}
        },
        {
            {"uri", "amiga://memory/dump"},
            {"name", "Memory Dump"},
            {"description", "A dump of the first 4MB of Amiga memory."},
            {"mimeType", "application/octet-stream"}
        }
    });

    return createSuccessResponse(req.value("id", json(nullptr)), {{"resources", resources}});
}

string
McpServer::handleReadResource(const json &req)
{
    auto id = req.value("id", json(nullptr));

    if (!req.contains("params") || !req["params"].is_object() ||
        !req["params"].contains("uri") || !req["params"]["uri"].is_string()) {

        return createErrorResponse(id, -32602, "Invalid params");
    }

    auto uri = req["params"]["uri"].get<string>();

    if (uri == "amiga://screen/current") {

        const auto &tex = videoPort.getTexture();
        const auto *src = tex.pixels.ptr;

        if (!src) {
            return createErrorResponse(id, -32603, "No frame buffer available");
        }

        const u32 width = (u32)HPIXELS;
        const u32 height = (u32)VPIXELS;
        const usize pixelBytes = usize(width) * usize(height) * 4;
        const u32 fileSize = u32(54 + pixelBytes);

        std::vector<u8> bmp;
        bmp.reserve(fileSize);

        auto append32 = [&](u32 value) {
            bmp.push_back(u8(value & 0xFF));
            bmp.push_back(u8((value >> 8) & 0xFF));
            bmp.push_back(u8((value >> 16) & 0xFF));
            bmp.push_back(u8((value >> 24) & 0xFF));
        };
        auto append16 = [&](u16 value) {
            bmp.push_back(u8(value & 0xFF));
            bmp.push_back(u8((value >> 8) & 0xFF));
        };

        // BITMAPFILEHEADER
        bmp.push_back('B');
        bmp.push_back('M');
        append32(fileSize);
        append16(0);
        append16(0);
        append32(54);

        // BITMAPINFOHEADER
        append32(40);
        append32(width);
        append32(u32(-i32(height)));
        append16(1);
        append16(32);
        append32(0);
        append32((u32)pixelBytes);
        append32(0);
        append32(0);
        append32(0);
        append32(0);

        for (usize i = 0; i < usize(width) * usize(height); i++) {

            GpuColor color(src[i]);
            bmp.push_back(color.b());
            bmp.push_back(color.g());
            bmp.push_back(color.r());
            bmp.push_back(color.a());
        }

        auto data = base64Encode(bmp.data(), bmp.size());
        return createSuccessResponse(id, {
            {"contents", json::array({
                {
                    {"uri", uri},
                    {"mimeType", "image/bmp"},
                    {"text", data}
                }
            })}
        });
    }

    if (uri == "amiga://memory/dump") {

        constexpr u32 DUMP_SIZE = 4 * 1024 * 1024;
        std::vector<u8> dump(DUMP_SIZE, 0);

        const bool wasRunning = isRunning();
        if (wasRunning) emulator.pause();

        for (u32 addr = 0; addr < DUMP_SIZE; addr++) {
            dump[addr] = mem.spypeek8<Accessor::CPU>(addr);
        }

        if (wasRunning) emulator.run();

        auto data = base64Encode(dump.data(), dump.size());
        return createSuccessResponse(id, {
            {"contents", json::array({
                {
                    {"uri", uri},
                    {"mimeType", "application/octet-stream"},
                    {"text", data}
                }
            })}
        });
    }

    return createErrorResponse(id, -32602, "Resource not found");
}

string
McpServer::handleListPrompts(const json &req) const
{
    json prompts = json::array({
        {
            {"name", "analyze_crash"},
            {"description", "Analyze Amiga crash context from CPU state."},
            {"arguments", json::array({
                {
                    {"name", "context"},
                    {"description", "Optional user context"},
                    {"required", false}
                }
            })}
        }
    });

    return createSuccessResponse(req.value("id", json(nullptr)), {{"prompts", prompts}});
}

string
McpServer::handleGetPrompt(const json &req)
{
    auto id = req.value("id", json(nullptr));

    if (!req.contains("params") || !req["params"].is_object() ||
        !req["params"].contains("name") || !req["params"]["name"].is_string()) {

        return createErrorResponse(id, -32602, "Invalid params");
    }

    auto name = req["params"]["name"].get<string>();

    if (name != "analyze_crash") {
        return createErrorResponse(id, -32602, "Prompt not found");
    }

    const auto cpuInfo = cpu.info.current();
    std::stringstream ss;

    ss << "The Amiga crashed or needs analysis.\n";
    ss << "PC: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << cpuInfo.pc0;
    ss << ", SR: 0x" << std::setw(4) << cpuInfo.sr;
    ss << ", USP: 0x" << std::setw(8) << cpuInfo.usp;
    ss << ", ISP: 0x" << std::setw(8) << cpuInfo.isp << "\n";
    ss << "Please analyze what might have caused the issue at this program counter.";

    json result = {
        {"description", "Amiga crash state"},
        {"messages", json::array({
            {
                {"role", "user"},
                {"content", {{"type", "text"}, {"text", ss.str()}}}
            }
        })}
    };

    return createSuccessResponse(id, result);
}

string
McpServer::base64Encode(const u8 *data, size_t size)
{
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    string out;
    out.reserve(4 * ((size + 2) / 3));

    for (size_t i = 0; i < size; i += 3) {

        u32 octetA = data[i];
        u32 octetB = (i + 1 < size) ? data[i + 1] : 0;
        u32 octetC = (i + 2 < size) ? data[i + 2] : 0;

        u32 triple = (octetA << 16) | (octetB << 8) | octetC;

        out.push_back(table[(triple >> 18) & 0x3F]);
        out.push_back(table[(triple >> 12) & 0x3F]);
        out.push_back((i + 1 < size) ? table[(triple >> 6) & 0x3F] : '=');
        out.push_back((i + 2 < size) ? table[triple & 0x3F] : '=');
    }

    return out;
}

bool
McpServer::tryGetU32(const json &args, const char *key, u32 &value)
{
    if (!args.contains(key) || !args[key].is_number_integer()) return false;

    auto v = args[key].get<i64>();
    if (v < 0 || v > i64(std::numeric_limits<u32>::max())) return false;

    value = u32(v);
    return true;
}

}
