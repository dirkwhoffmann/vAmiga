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
        .help   = { "Dumps the contents of a file" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

            auto file = fs.pwd().seekFile(argv[0]);

            Buffer<u8> buffer;
            file.ptr()->writeData(buffer);

            std::stringstream ss;
            buffer.memDump(ss);

            *this << ss;
        }
    });
}

}

