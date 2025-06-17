// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Console.h"
#include "Emulator.h"
#include <regex>

#include "Chrono.h"

namespace vamiga {

void
NavigatorConsole::_pause()
{

}

string
NavigatorConsole::getPrompt()
{
    std::stringstream ss;

    if (fs.numBlocks()) {

        auto fsName = fs.getName();
        if (!fsName.empty()) ss << fsName << ":";
        ss << fs.pwd();
    }

    ss << "> ";
    return ss.str();
}

void
NavigatorConsole::welcome()
{
    storage << "RetroShell Navigator ";
    remoteManager.rshServer << "vAmiga RetroShell Remote Server ";
    *this << Amiga::build() << '\n';
    *this << '\n';
    *this << "Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de" << '\n';
    *this << "https://github.com/dirkwhoffmann/vAmiga" << '\n';
    *this << '\n';

    printHelp();
    retroShell.asyncExec("import df0");
}

void
NavigatorConsole::printHelp()
{
    Console::printHelp();
}

void
NavigatorConsole::pressReturn(bool shift)
{
    Console::pressReturn(shift);
}

Block
NavigatorConsole::parseBlock(const string &argv)
{
    auto nr = Block(parseNum(argv));

    if (!fs.blockPtr(nr)) {

        if (!fs.initialized()) {
            throw AppError(Fault::FS_UNINITIALIZED);
        } else {
            throw AppError(Fault::OPT_INV_ARG, "0..." + std::to_string(fs.numBlocks()));
        }
    }
    return nr;
}

Block
NavigatorConsole::parseBlock(const ParsedArguments &argv, const string &token)
{
    return parseBlock(argv, token, fs.pwd().ref);
}

Block
NavigatorConsole::parseBlock(const ParsedArguments &argv, const string &token, Block fallback)
{
    auto nr = argv.contains(token) ? Block(parseNum(argv.at(token))) : fallback;

    if (!fs.blockPtr(nr)) {

        if (!fs.initialized()) {
            throw AppError(Fault::FS_UNINITIALIZED);
        } else {
            throw AppError(Fault::OPT_INV_ARG, "0..." + std::to_string(fs.numBlocks()));
        }
    }
    return nr;
}

FSPath
NavigatorConsole::parsePath(const ParsedArguments &argv, const string &token)
{
    if (argv.contains("b")) {
        return FSPath(fs, parseBlock(argv, token));
    }
    return parsePath(argv, token, fs.pwd());
}

FSPath
NavigatorConsole::parsePath(const ParsedArguments &argv, const string &token, const FSPath &fallback)
{
    if (argv.contains("b")) {
        return FSPath(fs, parseBlock(argv, token, fallback.ref));
    }

    auto path = argv.contains(token) ? fs.pwd().seek(argv.at(token)) : fallback;
    return path;
}

void
NavigatorConsole::initCommands(RetroShellCmd &root)
{
    std::vector<string> help;

    Console::initCommands(root);

    //
    // Importing and exporting
    //

    RetroShellCmd::currentGroup = "Import and export";

    root.add({ .tokens = { "import" }, .help = { "Import a file system." } });

    for (isize i = 0; i < 4; i++) {

        i == 0 ? help = { "Floppy file system from drive n", "df[n]" } : help = { "" };

        root.add({

            .tokens = { "import", "df" + std::to_string(i) },
            .help   = help,
            .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

                auto n = values[0];

                if (!df[n]->hasDisk()) throw AppError(Fault::DISK_MISSING);
                fs.init(*df[n]);

                

            }, .values = {i}
        });
    }

    RetroShellCmd::currentGroup = "Directories";

    root.add({

        .tokens = { "info" },
        .help   = { "Print a file system summary." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            std::stringstream ss;
            ss << "Type   Size            Used    Free    Full  Name" << std::endl;
            fs.dump(Category::State, ss);
            *this << ss;
        }
    });

    root.add({

        .tokens = { "cd" },
        .argx   = {
            { .name = { "path", "New working directory" }, .flags = arg::opt },
            { .name = { "b", "Specify the directory as a block number" }, .flags = arg::flag }
        },
        .help   = { "Change the working directory." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto path = parsePath(args, "path", fs.rootDir());
            fs.cd(path);
        }
    });

    root.add({

        .tokens = { "dir" },
        .argx   = {
            { .name = { "path", "Path to directory" }, .flags = arg::opt },
            { .name = { "b", "Specify the directory as a block number" }, .flags = arg::flag },
            { .name = { "d", "List directories only" }, .flags = arg::flag },
            { .name = { "f", "List files only" }, .flags = arg::flag },
            { .name = { "r", "Traverse subdirectories" }, .flags = arg::flag }
        },
        .help   = { "Display a sorted list of the files in a directory" },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto path = parsePath(args, "path", fs.pwd());
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");

            FSOpt opt = {

                .recursive = r,
                .sort = sort::dafa,

                .filter = [&](const FSPath &item) {

                    return (!d || item.isDirectory()) && (!f || item.isFile());
                },

                .formatter = [&](const FSPath &item) {

                    return item.last().cpp_str() + (item.isDirectory() ? " (dir)" : "\t");
                }
            };

            std::stringstream ss;
            fs.list(ss, path, opt);
            *this << ss;
        }
    });

    root.add({

        .tokens = { "list" },
        .argx   = {
            { .name = { "path", "Path to directory" }, .flags = arg::opt },
            { .name = { "b", "Specify the directory as a block number" }, .flags = arg::flag },
            { .name = { "d", "List directories only" }, .flags = arg::flag },
            { .name = { "f", "List files only" }, .flags = arg::flag },
            { .name = { "r", "Traverse subdirectories" }, .flags = arg::flag },
            { .name = { "k", "Display keys (start blocks)" }, .flags = arg::flag },
            { .name = { "s", "Sort output" }, .flags = arg::flag } },
        .help   = { "List specified information about directories and files" },
        .func   = [this](Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto path = parsePath(args, "path", fs.pwd());
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");
            auto k = args.contains("k");
            auto s = args.contains("s");

            FSOpt opt = {

                .recursive = r,
                .sort = s ? sort::alpha : sort::none,

                .filter = [&](const FSPath &item) {

                    return (!d || item.isDirectory()) && (!f || item.isFile());
                },

                .formatter = [&](const FSPath &item) {

                    std::stringstream ss;
                    ss << std::left << std::setw(25) << item.last();

                    if (k) { ss << std::right << std::setw(9) << ("[" + std::to_string(item.ref) + "] "); }

                    if (item.isDirectory()) {
                        ss << std::right << std::setw(7) << "Dir";
                    } else {
                        ss << std::right << std::setw(7) << std::to_string(item.ptr()->getFileSize());
                    }
                    ss << " " << item.getProtectionBitString();
                    ss << " " << item.ptr()->getCreationDate().str();

                    return ss.str();
                }
            };

            std::stringstream ss;
            fs.list(ss, path, opt);
            *this << ss;
        }
    });

    root.add({

        .tokens = { "find" },
        .argx   = {
            { .name = { "name", "Search pattern" } },
            { .name = { "path", "Directory to search in" }, .flags = arg::opt },
            { .name = { "d", "Find directories only" }, .flags = arg::flag },
            { .name = { "f", "Find files only" }, .flags = arg::flag },
            { .name = { "r", "Search subdirectories, too" }, .flags = arg::flag },
            { .name = { "s", "Sort output" }, .flags = arg::flag } },
        .help   = { "Find files or directories" },
        .func   = [this](Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto pattern = FSPattern(args.at("name"));
            auto path = parsePath(args, "path", fs.pwd());
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");
            auto s = args.contains("s");

            FSOpt opt = {

                .recursive = r,
                .sort = s ? sort::alpha : sort::none,

                .filter = [&](const FSPath &item) {

                    return pattern.match(item.last()) &&
                    (!d || item.isDirectory()) &&
                    (!f || item.isFile());
                },

                .formatter = [&](const FSPath &item) {

                    std::stringstream ss;
                    ss << item.name() + (item.isDirectory() ? " (dir)" : "");
                    return ss.str();
                }
            };

            std::vector<string> matches;
            fs.collect(path, matches, opt);
            for (auto &it : matches) { *this << it << '\n'; }
        }
    });

    root.add({

        .tokens = { "type" },
        .extra  = { arg::path },
        .argx   = {
            { .name = { "path", "File path" } },
            { .name = { "h", "Print hex dump" }, .flags = arg::flag },
            { .name = { "c", "Print characters" }, .flags = arg::flag }
        },
        .help   = { "Print the contents of a file" },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto file = fs.pwd().seekFile(args.at("path"));

            std::stringstream ss;
            Buffer<u8> buffer;
            file.ptr()->writeData(buffer);

            if (args.contains("h")) {
                buffer.memDump(ss);
            } else if (args.contains("c")) {
                buffer.ascDump(ss);
            } else {
                buffer.txtDump(ss);
            }

            *this << ss;
        }
    });

    root.add({

        .tokens = { "block" },
        .argx   = {
            { .name = { "nr", "Block number" }, .flags = arg::opt },
            { .name = { "h", "Print a hex dump" }, .flags = arg::flag },
        },
        .help   = { "Inspect a block" },
        .ghelp  = { "Manage blocks" },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            // auto nr = args.contains("nr") ? parseBlock(args.at("nr")) : fs.pwd().ref;
            auto nr = parseBlock(args, "nr");

            if (auto ptr = fs.blockPtr(nr); ptr) {

                std::stringstream ss;

                if (args.contains("h")) {
                    ptr->hexDump(ss);
                } else {
                    ptr->dump(ss);
                }

                *this << '\n' << ss << '\n';
            }
        }
    });

    root.add({

        .tokens = { "block", "import" },
        .argx   = {
            { .name = { "nr", "Block number" } },
            { .name = { "path", "File path" } },
        },
        .help   = { "Import a block from a file" },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            *this << "Holla, die Waldfee!" << '\n';
            *this << "This is not implemented, yet!" << '\n';
        }
    });

    root.add({

        .tokens = { "block", "export" },
        .argx   = {
            { .name = { "nr", "Block number" } },
            { .name = { "path", "File path" } },
        },
        .help   = { "Export a block to a file" },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            *this << "Holla, die Waldfee!" << '\n';
            *this << "This is not implemented, yet!" << '\n';
        }
    });
}

}

