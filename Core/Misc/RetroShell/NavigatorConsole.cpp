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
}

void
NavigatorConsole::printHelp()
{
    storage << "Type 'help' or press 'TAB' twice for help.\n";
    storage << "Press 'SHIFT+LEFT' or 'SHIFT+RIGHT' to switch to another console.";

    remoteManager.rshServer << "Type 'help' for help.\n";

    *this << vspace{1};
}

void
NavigatorConsole::pressReturn(bool shift)
{
    Console::pressReturn(shift);
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

        .tokens = { "dir" },
        .argx   = { { .name = { "path", "Path to directory" }, .flags = arg::opt } },
        .help   = { "Display a sorted list of the files in a directory." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto path = fs.pwd();

            if (auto arg = args.find("path"); arg != args.end()) {
                path = path.seek(arg->second);
            }
            std::stringstream ss;
            fs.ls(ss, path);
            *this << ss;
        }
    });

    root.add({

        .tokens = { "list" },
        .extra  = { arg::path },
        .argx   = {
            { .name = { "path", "Path to directory" }, .flags = arg::opt },
            { .name = { "d", "List directories only" }, .flags = arg::flag },
            { .name = { "f", "List files only" }, .flags = arg::flag },
            { .name = { "r", "Traverse subdirectories" }, .flags = arg::flag },
            { .name = { "k", "Display keys (start blocks)" }, .flags = arg::flag },
            { .name = { "s", "Sort output" }, .flags = arg::flag } },
        .help   = { "List specified information about directories and files." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto path = fs.pwd();
            if (auto arg = args.find("path"); arg != args.end()) {
                path = path.seek(arg->second);
            }
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");
            auto k = args.contains("k");
            auto s = args.contains("s");

            FSOpt opt;

            if (s) opt.sort = true;
            if (r) opt.recursive = true;

            opt.filter = [&](const FSPath &item) {

                return (!d || item.isDirectory()) && (!f || item.isFile());
            };

            opt.formatter = [&](const FSPath &item) {

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
            };

            std::stringstream ss;
            fs.list(ss, path, opt);
            *this << ss;
        }
    });

    root.add({

        .tokens = { "find" },
        .extra  = { arg::path },
        .argx   = {
            { .name = { "name", "The searched item" } },
            { .name = { "path", "Start searching here" }, .flags = arg::opt },
            { .name = { "d", "Find directories only" }, .flags = arg::flag },
            { .name = { "f", "Find files only" }, .flags = arg::flag },
            { .name = { "r", "Search subdirectories, too" }, .flags = arg::flag },
            { .name = { "s", "Sort output" }, .flags = arg::flag } },
        .help   = { "Find files or directories." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto name = FSString(args, "name");
            auto path = FSString(args, "path", ".");
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");
            auto s = args.contains("s");

            auto dir = fs.pwd().seek(path);

            FSOpt opt;

            opt.sort = s;
            opt.recursive = r;

            opt.filter = [&](const FSPath &item) {

                printf("filter %s\n", item.name().c_str());
                printf("   last %s\n", item.last().c_str());
                printf("   name %s\n", name.c_str());

                return
                item.last() == name &&
                (!d || item.isDirectory()) &&
                (!f || item.isFile());
            };

            opt.formatter = [&](const FSPath &item) {

                std::stringstream ss;

                if (item.isDirectory()) {
                    ss << std::left << std::setw(30) << (item.name() + " (dir)");
                } else {
                    ss << std::left << std::setw(30) << item.name();
                }

                return ss.str();
            };

            std::vector<FSPath> matches;
            fs.collect(dir, matches, opt);

            for (auto &it : matches) { *this << opt.formatter(it) << '\n'; }
        }
    });

    root.add({

        .tokens = { "cd" },
        .extra  = { arg::path },
        .argx   = {
            { .name = { "path", "New working directory" }, .flags = arg::opt }
        },
        .help   = { "Change the working directory." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            argv.empty() ? fs.cd(fs.rootDir()) : fs.cd(argv[0]);
        }
    });

    root.add({

        .tokens = { "type" },
        .extra  = { arg::path },
        .argx   = {
            { .name = { "h", "Print hex dump" }, .flags = arg::flag },
            { .name = { "c", "Print characters" }, .flags = arg::flag },
            { .name = { "format", "Display format" }, .value = "{OFS|FFS}", .flags = arg::keyval|arg::opt },
            { .name = { "path", "File path" }, }
        },
        .help   = { "Print the contents of a file." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

                auto file = fs.pwd().seekFile(argv[0]);

                Buffer<u8> buffer;
                file.ptr()->writeData(buffer);

                std::stringstream ss;
                buffer.dump(ss, "%c");

                *this << ss;
            }
    });

    root.add({

        .tokens = { "create" },
        .extra  = { arg::path },
        .argx   = {
            { .name = { "c", "Number of cylinders" }, .value = arg::count, .flags = arg::keyval },
            { .name = { "h", "Number of heads" }, .value = arg::count, .flags = arg::keyval },
            { .name = { "s", "Number of sectors" }, .value = arg::count, .flags = arg::keyval }
        },
        .help   = { "Create a new file system." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            *this << "Holla, die Waldfee!\n";
        }
    });

    root.add({

        .tokens = { "hexdump" },
        .extra  = { arg::path },
        .argx   = { { .name = { "path", "File" } } },
        .help   = { "Dump the binary contents of a file." },
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
        .help   = { "Dump the binary contents of a file." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            *this << "Holla, die Waldfee";
        }
    });

    root.add({

        .tokens = { "block" },
        .help   = { "Manages blocks" }
    });

    root.add({

        .tokens = { "block", "dump" },
        .extra  = { arg::nr },
        .argx   = { { .name = { "nr", "Block number" } } },
        .help   = { "Dump the contents of a block." },
        .func   = [this] (Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            if (auto ptr = fs.blockPtr((Block)parseNum(argv[0])); ptr) {

                std::stringstream ss;
                ptr->hexDump(ss);
                *this << ss;

            } else {

                throw AppError(Fault::FS_INVALID_BLOCK_TYPE);
            }
        }
    });

}

}

