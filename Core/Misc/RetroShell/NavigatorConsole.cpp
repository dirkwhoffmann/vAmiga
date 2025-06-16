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
            { .name = { "b", "Specify directory as a block number" }, .flags = arg::flag }
        },
        .help   = { "Change the working directory." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            string path;

            if (!args.contains("path")) {
                fs.cd(string("/"));
            } else if (args.contains("b")) {
                auto fspath = FSPath(fs, (Block)parseNum(args.at("path")));
                fs.cd(fspath);
            } else {
                fs.cd(args.at("path"));
            }
        }
    });

    root.add({

        .tokens = { "dir" },
        .argx   = {
            { .name = { "path", "Path to directory" }, .flags = arg::opt },
            { .name = { "d", "List directories only" }, .flags = arg::flag },
            { .name = { "f", "List files only" }, .flags = arg::flag },
            { .name = { "r", "Traverse subdirectories" }, .flags = arg::flag }
        },
        .help   = { "Display a sorted list of the files in a directory" },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto path = FSString(args, "path", ".");
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");

            auto dir = fs.pwd().seek(path);

            FSOpt opt = {

                .recursive = r,
                .sort = true,
                
                .filter = [&](const FSPath &item) {

                    return (!d || item.isDirectory()) && (!f || item.isFile());
                }
            };

            std::stringstream ss;
            fs.ls(ss, dir, opt);
            *this << ss;
        }
    });

    root.add({

        .tokens = { "list" },
        .argx   = {
            { .name = { "path", "Path to directory" }, .flags = arg::opt },
            { .name = { "d", "List directories only" }, .flags = arg::flag },
            { .name = { "f", "List files only" }, .flags = arg::flag },
            { .name = { "r", "Traverse subdirectories" }, .flags = arg::flag },
            { .name = { "k", "Display keys (start blocks)" }, .flags = arg::flag },
            { .name = { "s", "Sort output" }, .flags = arg::flag } },
        .help   = { "List specified information about directories and files" },
        .func   = [this](Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto path = FSString(args, "path", ".");
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");
            auto k = args.contains("k");
            auto s = args.contains("s");

            auto dir = fs.pwd().seek(path);

            FSOpt opt = {

                .recursive = r,
                .sort = s,

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
            fs.list(ss, dir, opt);
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
            auto path = FSString(args, "path", ".");
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");
            auto s = args.contains("s");

            auto dir = fs.pwd().seek(path);

            FSOpt opt = {

                .recursive = r,
                .sort = s,

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

            std::vector<FSPath> matches;
            fs.collect(dir, matches, opt);
            for (auto &it : matches) { *this << opt.formatter(it) << '\n'; }
        }
    });

    root.add({

        .tokens = { "type" },
        .extra  = { arg::path },
        .argx   = {
            { .name = { "h", "Print hex dump" }, .flags = arg::flag },
            { .name = { "c", "Print characters" }, .flags = arg::flag },
            { .name = { "path", "File path" }, }
        },
        .help   = { "Print the contents of a file" },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto file = fs.pwd().seekFile(argv[0]);

            std::stringstream ss;
            Buffer<u8> buffer;
            file.ptr()->writeData(buffer);

            if (args.contains("h")) {
                buffer.memDump(ss);
            } else if (args.contains("c")) {
                buffer.ascDump(ss);
            } else {
                buffer.type(ss);
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

            auto nr = args.contains("nr") ? parseBlock(args.at("nr")) : fs.pwd().ref;

            if (auto ptr = fs.blockPtr(nr)) {

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

    root.add({

        .tokens = { "hexdump" },
        .extra  = { arg::path },
        .argx   = { { .name = { "path", "File" } } },
        .help   = { "Dump the binary contents of a file" },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto file = fs.pwd().seekFile(argv[0]);

            Buffer<u8> buffer;
            file.ptr()->writeData(buffer);

            std::stringstream ss;
            buffer.memDump(ss);

            *this << ss;
        }
    });

    root.add({

        .tokens = { "test" },
        .argx   = {
            { .name = { "path", "File" } },
            { .name = { "dir", "Another param" } },
            { .name = { "name", "Yet another param" }, .flags = arg::opt }
        },
        .help   = { "Dump the binary contents of a file" },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            *this << "Holla, die Waldfee";
        }
    });
}

}

