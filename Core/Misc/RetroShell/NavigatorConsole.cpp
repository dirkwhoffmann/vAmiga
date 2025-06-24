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

    if (fs.initialized()) {

        ss << "[" << std::to_string(fs.curr) << "]";

        auto fsName = fs.getName();
        if (!fsName.empty()) ss << " " << fsName << ":";
        if (fs.pwd().isDirectory()) ss << " " << fs.pwd();
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
    return parsePath(argv, token, fs.pwd());
}

FSPath
NavigatorConsole::parsePath(const Arguments &argv, const string &token, const FSPath &fallback)
{
    if (!argv.contains(token)) return fallback;

    try {

        // Try to find the directory by name
        return fs.pwd().seek(argv.at(token));

    } catch (AppError &) {

        // Treat the argument as a block number
        return FSPath(&fs, parseBlock(argv.at(token)));
    }
}

FSPath
NavigatorConsole::parseFile(const Arguments &argv, const string &token)
{
    return parseFile(argv, token, fs.pwd());
}

FSPath
NavigatorConsole::parseFile(const Arguments &argv, const string &token, const FSPath &fallback)
{
    if (!fs.formatted()) {
        throw AppError(Fault::FS_UNFORMATTED);
    }
    auto path = parsePath(argv, token, fallback);

    if (!path.isFile()) {
        throw AppError(Fault::FS_NOT_A_FILE, "Block " + std::to_string(path.ref));
    }
    return path;
}

FSPath
NavigatorConsole::parseDirectory(const Arguments &argv, const string &token)
{
    return parseDirectory(argv, token, fs.pwd());
}

FSPath
NavigatorConsole::parseDirectory(const Arguments &argv, const string &token, const FSPath &fallback)
{
    if (!fs.formatted()) {
        throw AppError(Fault::FS_UNFORMATTED);
    }
    auto path = parsePath(argv, token, fallback);

    if (!path.isDirectory()) {
        throw AppError(Fault::FS_NOT_A_DIRECTORY, "Block " + std::to_string(path.ref));
    }
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
        throw util::ParseError("Flags -a, -o, -d are mutually exclusive");
    }
    if ((int)a + (int)w + (int)l > 1) {
        throw util::ParseError("Flags -a, -w, -l are mutually exclusive");
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

    RSCommand::currentGroup = "General";

    root.add({

        .tokens = { "create" },
        .chelp  = { "Create a file system with a particular capacity" },
    });

    root.add({

        .tokens = { "create", "SD" },
        .chelp  = { "Create a file system for a single-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.init(FileSystemDescriptor(Diameter::INCH_525, Density::SD, FSVolumeType::NODOS));
            fs.dump(Category::Info, os);
        }
    });

    root.add({

        .tokens = { "create", "DD" },
        .chelp  = { "Create a file system for a double-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.init(FileSystemDescriptor(Diameter::INCH_35, Density::DD, FSVolumeType::NODOS));
            fs.dump(Category::Info, os);
        }
    });

    root.add({

        .tokens = { "create", "HD" },
        .chelp  = { "Create a file system for a high-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.init(FileSystemDescriptor(Diameter::INCH_35, Density::HD, FSVolumeType::NODOS));
            fs.dump(Category::Info, os);
        }
    });

    root.add({

        .tokens = { "create", "capacity" },
        .chelp  = { "Create a file system with a particular capacity" },
        .args   = {
            { .name = { "mb", "Capacity in MB" } },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            // Convert the provided capacity to bytes
            auto mb = MB(parseNum(args.at("mb")));

            // Compute the number of needed blocks
            auto blocks = (mb + 511) / 512;

            fs.init(FileSystemDescriptor(blocks, FSVolumeType::NODOS));
            fs.dump(Category::Info, os);
        }
    });

    root.add({

        .tokens = { "create", "custom" },
        .chelp  = { "Create a file system with a custom layout" },
        .args   = {
            { .name = { "cylinders", "Number of cylinders" }, .flags=rs::keyval },
            { .name = { "heads", "Number of drive heads" }, .flags=rs::keyval },
            { .name = { "sectors", "Number of sectors per cylinder" }, .flags=rs::keyval },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            isize c = parseNum(args.at("cylinders"));
            isize h = parseNum(args.at("heads"));
            isize s = parseNum(args.at("sectors"));
            isize b = 512;

            auto geometry = GeometryDescriptor(c, h, s, b);
            fs.init(FileSystemDescriptor(geometry, FSVolumeType::NODOS));
            fs.dump(Category::Info, os);
        }
    });

    root.add({

        .tokens = { "format" },
        .chelp  = { "Format the file system" },
        .args   = {
            { .name = { "dos", "Amiga file system" }, .key = "{ OFS | FFS }" },
            { .name = { "name", "File system name" }, .flags = rs::opt },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            // Determine the DOS type
            auto type = FSVolumeType::NODOS;
            auto dos = util::uppercased(args.at("dos"));
            if (dos == "OFS") type = FSVolumeType::OFS;
            if (dos == "FFS") type = FSVolumeType::FFS;

            if (type == FSVolumeType::NODOS) {
                throw util::ParseError("Expected values: OFS or FFS");
            }

            // Format the device
            fs.format(type, args.contains("name") ? args.at("name") : "New Disk");
            fs.dump(Category::Info, os);
        }
    });

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

    RSCommand::currentGroup = "Navigation";

    root.add({

        .tokens = { "cd" },
        .chelp  = { "Change the working directory" },
        .args   = {
            { .name = { "path", "New working directory" }, .flags = rs::opt },
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
            { .name = { "d", "List directories only" }, .flags = rs::flag },
            { .name = { "f", "List files only" }, .flags = rs::flag },
            { .name = { "r", "Traverse subdirectories" }, .flags = rs::flag }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto path = parseDirectory(args, "path");
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
            { .name = { "d", "List directories only" }, .flags = rs::flag },
            { .name = { "f", "List files only" }, .flags = rs::flag },
            { .name = { "r", "Traverse subdirectories" }, .flags = rs::flag },
            { .name = { "k", "Display keys (start blocks)" }, .flags = rs::flag },
            { .name = { "s", "Sort output" }, .flags = rs::flag } },
        .func   = [this](std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto path = parseDirectory(args, "path");
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
            { .name = { "path", "File path" }, .flags = rs::opt },
            { .name = { "l", "Display a line number in each row" }, .flags = rs::flag },
            { .name = { "t", "Display the last part" }, .flags = rs::flag },
            { .name = { "lines", "Number of displayed rows" }, .flags = rs::keyval|rs::opt },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto file = parsePath(args, "path", fs.pwd());
            if (!file.isFile()) {
                throw AppError(Fault::FS_NOT_A_FILE, "Block " + std::to_string(file.ref));
            }

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

    RSCommand::currentGroup = "Inspection";

    root.add({

        .tokens = { "info" },
        .chelp  = { "Print a file system summary" },
        .args   = {
            { .name = { "b", "Inspect the block storage" }, .flags = rs::flag },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            if (args.contains("b")) {
                fs.dump(Category::Blocks, os);
            } else {
                fs.dump(Category::Info, os);
            }

        }
    });

    root.add({

        .tokens = { "file" },
        .ghelp  = { "Manage files" },
        .chelp  = { "Inspect a single file" },
        /*
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.dump(Category::Info, os);
        }
        */
    });

    root.add({

        .tokens = { "file", "dump" },
        .chelp  = { "Dump the contents of a file" },
        .args   = {
            { .name = { "path", "File path" }, .flags = rs::opt },
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
        .chelp  = { "Dump the contents of a block" },
        .args   = {
            { .name = { "nr", "Block number" }, .flags = rs::opt },
            { .name = { "a", "Output in ASCII, only" }, .flags = rs::flag },
            { .name = { "o", "Output numbers in octal" }, .flags = rs::flag },
            { .name = { "d", "Output numbers in decimal" }, .flags = rs::flag },
            { .name = { "w", "Print in word format" }, .flags = rs::flag },
            { .name = { "l", "Print in long word format" }, .flags = rs::flag },
            { .name = { "t", "Display the last part" }, .flags = rs::flag },
            { .name = { "lines", "Number of displayed rows" }, .flags = rs::keyval|rs::opt },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto nr = parseBlock(args, "nr", fs.pwd().ref);
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

    RSCommand::currentGroup = "Modify";

    root.add({

        .tokens = { "delete" },
        .chelp  = { "Deletes a file" },
        .args   = {
            { .name = { "path", "File to delete" } },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto path = parsePath(args, "path");

            if (path.isFile()) {

                fs.deleteFile(path);

            } else if (path.isDirectory()) {
                throw AppError(Fault::FS_NOT_A_FILE, args.at("path"));
            } else {
                throw util::ParseError("Not a file or directory");
            }

        }
    });

}

}

