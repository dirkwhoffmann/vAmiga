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
NavigatorConsole::help(const string &userInput)
{
    // Extract tokens
    Arguments tokens = split(userInput);

    // Split into the command part and the argument part
    Arguments cmds, args; split(tokens, cmds, args);

    // Find the command
    auto *cmd = findCommand(cmds);

    // Print help message
    if (cmd) Console::help(*cmd);
}

string
NavigatorConsole::autoComplete(const string& userInput)
{
    string result;

    // Extract tokens
    Arguments tokens = split(userInput);

    // Split into the command part and the argument part
    // Arguments cmds, args; split(tokens, cmds, args);

    // Complete all tokens
    newAutoComplete(tokens);

    // Recreate the command string
    // for (const auto &it : tokens) { result += (result == "" ? "" : " ") + it; }
    result = util::concat(tokens, " ");

    // Add a space if the command has been fully completed ...
    if (auto *cmd = findCommand(tokens)) {

        // ... and there are additional subcommands or arguments
        if (!cmd->args.empty() || !cmd->extra.empty() || !cmd->switches.empty()) result += " ";
    }

    return result;
}


void
NavigatorConsole::initCommands(RetroShellCmd &root)
{
    using namespace rs;

    std::vector<string> help;

    Console::initCommands(root);

    //
    // Experimental test commands

    rs::Command::currentGroup = "Import and export";

    auto &cmd = add(top, { .name = { "import" }, .help = { "Import file system" } });

    for (isize i = 0; i < 4; i++) {

        // i == 0 ? help = { "Floppy file system from drive n", "df[n]" } : help = { "" };

        add(cmd, {

            // .tokens = { "import", "df" + std::to_string(i) },
            .name   = { "df" + std::to_string(i), "df[n]" },
            .help   = { "Floppy file system from drive n" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

                auto n = values[0];

                if (!df[n]->hasDisk()) throw AppError(Fault::DISK_MISSING);
                fs.init(*df[n]);

            }, .values = {i}
        });
    }


    add({
        .name       = { "test", "me" },
        .args       = { String("first"), Addr("second") },
        .extra      = { Range("second", -1, 1) },
        .switches   = { { "-v", "<level>" } }
    });

    //
    // Importing and exporting
    //

    RetroShellCmd::currentGroup = "Import and export";

    root.add({ .tokens = { "import" }, .help = { "Import file system" } });

    for (isize i = 0; i < 4; i++) {

        i == 0 ? help = { "Floppy file system from drive n", "df[n]" } : help = { "" };

        root.add({

            .tokens = { "import", "df" + std::to_string(i) },
            .help   = help,
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

                auto n = values[0];

                if (!df[n]->hasDisk()) throw AppError(Fault::DISK_MISSING);
                fs.init(*df[n]);

            }, .values = {i}
        });
    }

    RetroShellCmd::currentGroup = "Directories";

    /*
    root.add({

        .tokens = { "pwd" },
        .help   = { "Prints the name of the working directory" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

            *this << fs.pwd() << '\n';
        }
    });
    */

    root.add({

        .tokens = { "info" },
        .help   = { "Prints a file system summary" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

            std::stringstream ss;
            ss << "Type   Size            Used    Free    Full  Name" << std::endl;
            fs.dump(Category::State, ss);
            *this << ss;
        }
    });

    root.add({

        .tokens = { "dir" },
        .extra  = { Arg::path },
        .help   = { "Displays a sorted list of the files in a directory" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

            std::stringstream ss;
            argv.empty() ? fs.ls(ss) : fs.ls(ss, fs.pwd().seek(argv[0]));
            *this << ss;
        }
    });

    root.add({

        .tokens = { "list" },
        .extra  = { Arg::path },
        .help   = { "Lists specified information about directories and files" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

            std::stringstream ss;
            argv.empty() ? fs.list(ss) : fs.list(ss, fs.pwd().seek(argv[0]));
            *this << ss;
        }
    });

    root.add({

        .tokens = { "cd" },
        .extra  = { Arg::path },
        .help   = { "Changes the working directory" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

            argv.empty() ? fs.cd(fs.rootDir()) : fs.cd(argv[0]);
        }
    });

    root.add({

        .tokens = { "type" },
        .extra  = { Arg::path },
        .help   = { "Prints the contents of a file" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

            auto file = fs.pwd().seekFile(argv[0]);

            Buffer<u8> buffer;
            file.ptr()->writeData(buffer);

            std::stringstream ss;
            buffer.dump(ss, "%c");

            *this << ss;
        }
    });

    root.add({

        .tokens = { "hexdump" },
        .extra  = { Arg::path },
        .help   = { "Dumps the binary contents of a file" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

            auto file = fs.pwd().seekFile(argv[0]);

            Buffer<u8> buffer;
            file.ptr()->writeData(buffer);

            std::stringstream ss;
            buffer.memDump(ss);

            *this << ss;
        }
    });

    root.add({

        .tokens = { "block" },
        .help   = { "Manages blocks" }
    });

    root.add({

        .tokens = { "block", "dump" },
        .extra  = { Arg::nr },
        .help   = { "Dumps the contents of a block" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

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

