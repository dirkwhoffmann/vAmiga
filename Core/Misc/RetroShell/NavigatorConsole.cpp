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
    Console::welcome();
}

void
NavigatorConsole::printHelp(isize tab)
{
    Console::printHelp(tab);
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
NavigatorConsole::parseBlock(const Arguments &argv, const string &token)
{
    return parseBlock(argv, token, fs.pwd().ref);
}

Block
NavigatorConsole::parseBlock(const Arguments &argv, const string &token, Block fallback)
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
NavigatorConsole::parsePath(const Arguments &argv, const string &token)
{
    if (argv.contains("b")) {
        return FSPath(fs, parseBlock(argv, token));
    }
    return parsePath(argv, token, fs.pwd());
}

FSPath
NavigatorConsole::parsePath(const Arguments &argv, const string &token, const FSPath &fallback)
{
    if (argv.contains("b")) {
        return FSPath(fs, parseBlock(argv, token, fallback.ref));
    }

    auto path = argv.contains(token) ? fs.pwd().seek(argv.at(token)) : fallback;
    return path;
}

util::DumpOpt
NavigatorConsole::parseDumpOpts(const Arguments &argv)
{
    auto lines = argv.contains("lines") ? parseNum(argv.at("lines")) : -1;
    auto a = argv.contains("a");
    auto o = argv.contains("o");
    auto d = argv.contains("d");
    auto t = argv.contains("t");
    auto w = argv.contains("w");
    auto l = argv.contains("l");
    auto size = l ? 4 : w ? 2 : 1;

    if ((int)a + (int)o + (int)d > 1) {
        throw util::ParseError("Flags -a, -o, -d are mutually exclusive.");
    }
    if ((int)a + (int)w + (int)l > 1) {
        throw util::ParseError("Flags -a, -w, -l are mutually exclusive.");
    }
    if (o) return {

        .base = 8,
        .size = size,
        .prefix = 2,
        .columns = 16 / size,
        .lines = lines,
        .tail = t,
        .offset = true,
        .ascii = true
    };
    if (d) return {

        .base = 10,
        .size = size,
        .prefix = 2,
        .columns = 16 / size,
        .lines = lines,
        .tail = t,
        .offset = true,
        .ascii = true
    };
    if (a) return {

        .base = 0,
        .size = size,
        .prefix = 2,
        .columns = 64,
        .lines = lines,
        .tail = t,
        .offset = true,
        .ascii = true
    };
    return {

        .base = 16,
        .size = size,
        .prefix = 2,
        .columns = 16 / size,
        .lines = lines,
        .tail = t,
        .offset = true,
        .ascii = true
    };
}

void
NavigatorConsole::initCommands(RSCommand &root)
{
    std::vector<string> help;

    Console::initCommands(root);

    //
    // Console management
    //

    root.add({

        .tokens = { "next" },
        .chelp  = { "Switch to the next console" },
        .flags  = rs::hidden,

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            retroShell.enterCommander();
        }
    });

    root.add({

        .tokens = { "prev" },
        .chelp  = { "Switch to the previous console" },
        .flags  = rs::hidden,

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            retroShell.enterDebugger();
        }
    });


    //
    // Importing and exporting
    //

    RSCommand::currentGroup = "Import and export";

    root.add({ .tokens = { "import" }, .ghelp = { "Import a file system" } });

    root.add({

        .tokens = { "import", "df[n]" },
        .ghelp  = { "Import file system from floppy drive n" },
        .chelp  = { "import { df0 | df1 | df1 | df2 }" }
    });

    for (isize i = 0; i < 4; i++) {

        root.add({

            .tokens = { "import", "df" + std::to_string(i) },
            .chelp  = { "Import file system from floppy drive" + std::to_string(i) },
            .flags  = rs::shadowed,
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                auto n = values[0];

                fs.init(*df[n]);
                fs.dump(Category::Info, os);

            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "import", "hd[n]" },
        .ghelp  = { "Import file system from hard drive n" },
        .chelp  = { "import { hd0 | hd1 | hd1 | hd2 }" }
    });

    for (isize i = 0; i < 4; i++) {

        root.add({

            .tokens = { "import", "hd" + std::to_string(i) },
            .chelp  = { "Import file system from hard drive" + std::to_string(i) },
            .flags  = rs::shadowed,
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                auto n = values[0];

                fs.init(*hd[n], 0);
                fs.dump(Category::Info, os);

            }, .payload = {i}
        });
    }

    RSCommand::currentGroup = "Directories";

    root.add({

        .tokens = { "info" },
        .chelp  = { "Print a file system summary" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.dump(Category::Info, os);
        }
    });

    root.add({

        .tokens = { "cd" },
        .chelp  = { "Change the working directory." },
        .args   = {
            { .name = { "path", "New working directory" }, .flags = rs::opt },
            { .name = { "b", "Specify the directory as a block number" }, .flags = rs::flag }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto path = parsePath(args, "path", fs.rootDir());
            fs.cd(path);
        }
    });

    root.add({

        .tokens = { "dir" },
        .chelp  = { "Display a sorted list of the files in a directory" },
        .args   = {
            { .name = { "path", "Path to directory" }, .flags = rs::opt },
            { .name = { "b", "Specify the directory as a block number" }, .flags = rs::flag },
            { .name = { "d", "List directories only" }, .flags = rs::flag },
            { .name = { "f", "List files only" }, .flags = rs::flag },
            { .name = { "r", "Traverse subdirectories" }, .flags = rs::flag }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

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

            fs.list(os, path, opt);
        }
    });

    root.add({

        .tokens = { "list" },
        .chelp  = { "List specified information about directories and files" },
        .args   = {
            { .name = { "path", "Path to directory" }, .flags = rs::opt },
            { .name = { "b", "Specify the directory as a block number" }, .flags = rs::flag },
            { .name = { "d", "List directories only" }, .flags = rs::flag },
            { .name = { "f", "List files only" }, .flags = rs::flag },
            { .name = { "r", "Traverse subdirectories" }, .flags = rs::flag },
            { .name = { "k", "Display keys (start blocks)" }, .flags = rs::flag },
            { .name = { "s", "Sort output" }, .flags = rs::flag } },
        .func   = [this](std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

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

            fs.list(os, path, opt);
        }
    });

    root.add({

        .tokens = { "find" },
        .chelp  = { "Find files or directories" },
        .args   = {
            { .name = { "name", "Search pattern" } },
            { .name = { "path", "Directory to search in" }, .flags = rs::opt },
            { .name = { "b", "Specify the directory as a block number" }, .flags = rs::flag },
            { .name = { "d", "Find directories only" }, .flags = rs::flag },
            { .name = { "f", "Find files only" }, .flags = rs::flag },
            { .name = { "r", "Search subdirectories, too" }, .flags = rs::flag },
            { .name = { "s", "Sort output" }, .flags = rs::flag } },
        .func   = [this](std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

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
            for (auto &it : matches) { os << it << '\n'; }
        }
    });

    root.add({

        .tokens = { "type" },
        .chelp  = { "Print the contents of a file" },
        .args   = {
            { .name = { "path", "File path" } },
            { .name = { "b", "Specify the path as a block number" }, .flags = rs::flag },
            { .name = { "l", "Display a line number in each row" }, .flags = rs::flag },
            { .name = { "t", "Display the last part" }, .flags = rs::flag },
            { .name = { "lines", "Number of displayed rows" }, .flags = rs::keyval|rs::opt },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto file = parsePath(args, "path", fs.pwd());
            auto lines = args.contains("lines") ? parseNum(args.at("lines")) : -1;

            Buffer<u8> buffer;
            file.ptr()->writeData(buffer);

            buffer.type(os, {

                .lines = lines,
                .tail = args.contains("t"),
                .nr = args.contains("l")
            });
        }
    });

    root.add({

        .tokens = { "dump" },
        .chelp  = { "Dump the contents of a file" },
        .args   = {
            { .name = { "path", "File path" } },
            { .name = { "b", "Specify the path as a block number" }, .flags = rs::flag },
            { .name = { "a", "Output in ASCII, only" }, .flags = rs::flag },
            { .name = { "o", "Output numbers in octal" }, .flags = rs::flag },
            { .name = { "d", "Output numbers in decimal" }, .flags = rs::flag },
            { .name = { "w", "Print in word format" }, .flags = rs::flag },
            { .name = { "l", "Print in long word format" }, .flags = rs::flag },
            { .name = { "t", "Display the last part" }, .flags = rs::flag },
            { .name = { "lines", "Number of displayed rows" }, .flags = rs::keyval|rs::opt },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto file = parsePath(args, "path", fs.pwd());
            auto opt = parseDumpOpts(args);

            Buffer<u8> buffer;
            file.ptr()->writeData(buffer);
            buffer.dump(os, opt);
        }
    });

    root.add({

        .tokens = { "block" },
        .ghelp  = { "Manage blocks" },
        .chelp  = { "Inspect a block" },
        .args   = {
            { .name = { "nr", "Block number" }, .flags = rs::opt },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto nr = parseBlock(args, "nr");

            if (auto ptr = fs.blockPtr(nr); ptr) {
                ptr->dump(os);
            }
        }
    });

    root.add({

        .tokens = { "block", "dump" },
        .chelp  = { "Import a block from a file" },
        .args   = {
            { .name = { "nr", "Block number" } },
            { .name = { "a", "Output in ASCII, only" }, .flags = rs::flag },
            { .name = { "o", "Output numbers in octal" }, .flags = rs::flag },
            { .name = { "d", "Output numbers in decimal" }, .flags = rs::flag },
            { .name = { "w", "Print in word format" }, .flags = rs::flag },
            { .name = { "l", "Print in long word format" }, .flags = rs::flag },
            { .name = { "t", "Display the last part" }, .flags = rs::flag },
            { .name = { "lines", "Number of displayed rows" }, .flags = rs::keyval|rs::opt },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto nr = parseBlock(args, "nr");
            auto opt = parseDumpOpts(args);

            if (auto ptr = fs.blockPtr(nr); ptr) {

                ptr->hexDump(os, opt);
            }
        }
    });

    root.add({

        .tokens = { "block", "import" },
        .chelp  = { "Import a block from a file" },
        .args   = {
            { .name = { "nr", "Block number" } },
            { .name = { "path", "File path" } },
        },
        .func   = [] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            os << "Not implemented, yet!" << '\n';
        }
    });

    root.add({

        .tokens = { "block", "export" },
        .chelp  = { "Export a block to a file" },
        .args   = {
            { .name = { "nr", "Block number" } },
            { .name = { "path", "File path" } },
        },
        .func   = [] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            os << "Not implemented, yet!" << '\n';
        }
    });
}

}

