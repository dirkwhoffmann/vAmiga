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
#include "StringUtils.h"
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

    if (fs.isInitialized()) {

        auto &pwd = fs.pwd();

        ss << "[" << std::to_string(pwd.nr) << "]";

        auto fsName = fs.getName();
        if (!fsName.empty()) ss << " " << fsName << ":";
        if (pwd.isDirectory()) ss << " " << pwd.absName();
    }

    ss << "> ";
    return ss.str();
}

void
NavigatorConsole::welcome()
{
    if (vAmigaDOS) {

        storage << "File System Navigator ";
        *this << Amiga::build() << '\n';
        *this << '\n';

        *this << "Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de" << '\n';
        *this << "https://github.com/vAmigaDOS/vAmigaDOS" << '\n';
        *this << '\n';

    } else {

        Console::welcome();
    }
}

void
NavigatorConsole::summary()
{
    std::stringstream ss;

    if (fs.isInitialized()) {

        fs.dump(Category::Info, ss);

        *this << vspace{1};
        string line;
        while(std::getline(ss, line)) { *this << "    " << line << '\n'; }
        *this << vspace{1};

    } else {

        *this << vspace{1};
        *this << "    No file system present.\n";
        *this << "    Use the 'import' command to load one.";
        *this << vspace{1};
    }
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

void
NavigatorConsole::autoComplete(Tokens &argv)
{
    // Only proceed if there is anything to complete
    if (argv.empty()) return;

    if (auto [cmd, remaining] = seekCommand(argv); remaining.size() > 0) {

        // First, try to auto-complete the last token with a command name
        if (remaining.size() != 1 || !cmd->autoComplete(argv.back())) {

            // If that didn't work, try to auto-complete with a file name
            try {
                auto prefix = autoCompleteFilename(argv.back(), cmd->flags);
                if (prefix.size() > argv.back().size()) argv.back() = prefix;

            } catch (...) { }
        }
    }
}

string
NavigatorConsole::autoCompleteFilename(const string &input, usize flags) const
{
    bool absolute = !input.empty() && input[0] == '/';

    // Seek matching items
    auto matches = fs.match(&fs.pwd(), input + "*");

    // Filter out unwanted items
    if (!matches.empty()) {
        matches.erase(std::remove_if(matches.begin(), matches.end(), [flags](const FSBlock *node) {

            return
            (!(flags & rs::acdir) && node->isDirectory()) ||
            (!(flags & rs::acfile) && node->isFile());

        }), matches.end());
    }

    // Extract names
    std::vector<string> names;
    for (auto &it : matches) {
        names.push_back(absolute ? it->acabsName(): it->acrelName());
    }

    // Auto-complete all common characters
    auto completed = util::commonPrefix(names, false);

    return completed;
}

void
NavigatorConsole::help(std::ostream &os, const string &argv, isize tabs)
{
    auto [cmd, args] = seekCommand(argv);

    // Determine the kind of help to display
    /*
    bool displayFiles = (tabs % 2 == 0) && fs.isFormatted() && cmd && cmd->callback && (cmd->flags & rs::ac);
    bool displayCmds  = (tabs % 2 == 1) || !displayFiles;
    */
    bool displayFiles = fs.isFormatted() && cmd && cmd->callback && (cmd->flags & rs::ac);
    bool displayCmds  = true;

    if (displayCmds) {

        // Display the standard command help
        Console::help(os, argv, tabs);
    }

    if (displayFiles) {

        // Seek matching items
        auto matches = fs.match(&fs.pwd(), args.empty() ? "*" : args.back() + "*");

        // List all nodes
        if (!matches.empty() && displayCmds) os << std::endl;
        FSTree(matches, { .sort = sort::dafa }).list(os, { .indent = 7 });
    }
}

Block
NavigatorConsole::parseBlock(const string &argv)
{
    fs.require_initialized();

    if (auto nr = Block(parseNum(argv)); fs.read(nr)) {
        return nr;
    }

    throw AppError(Fault::OPT_INV_ARG, "0..." + std::to_string(fs.numBlocks()));
}

Block
NavigatorConsole::parseBlock(const Arguments &argv, const string &token)
{
    return parseBlock(argv, token, fs.pwd().nr);
}

Block
NavigatorConsole::parseBlock(const Arguments &argv, const string &token, Block fallback)
{
    auto nr = argv.contains(token) ? Block(parseNum(argv.at(token))) : fallback;

    if (!fs.read(nr)) {

        if (!fs.isInitialized()) {
            throw AppError(Fault::FS_UNINITIALIZED);
        } else {
            throw AppError(Fault::OPT_INV_ARG, "0..." + std::to_string(fs.numBlocks()));
        }
    }
    return nr;
}

FSBlock &
NavigatorConsole::parsePath(const Arguments &argv, const string &token)
{
    assert(argv.contains(token));

    try {
        // Try to find the directory by name
        return fs.seek(fs.pwd(), argv.at(token));

    } catch (...) {

        try {
            // Treat the argument as a block number
            return fs[parseBlock(argv.at(token))];

        } catch (...) {

            // The item does not exist
            throw AppError(Fault::FS_NOT_FOUND, argv.at(token));
        }
    }
}

FSBlock &
NavigatorConsole::parsePath(const Arguments &argv, const string &token, FSBlock &fallback)
{
    return argv.contains(token) ? parsePath(argv, token) : fallback;
}

FSBlock &
NavigatorConsole::parseFile(const Arguments &argv, const string &token)
{
    return parseFile(argv, token, fs.pwd());
}

FSBlock &
NavigatorConsole::parseFile(const Arguments &argv, const string &token, FSBlock &fallback)
{
    if (!fs.isFormatted()) {
        throw AppError(Fault::FS_UNFORMATTED);
    }
    auto &path = parsePath(argv, token, fallback);

    if (!path.isFile()) {
        throw AppError(Fault::FS_NOT_A_FILE, "Block " + std::to_string(path.nr));
    }
    return path;
}

FSBlock &
NavigatorConsole::parseDirectory(const Arguments &argv, const string &token)
{
    return parseDirectory(argv, token, fs.pwd());
}

FSBlock &
NavigatorConsole::parseDirectory(const Arguments &argv, const string &token, FSBlock &fallback)
{
    if (!fs.isFormatted()) {
        throw AppError(Fault::FS_UNFORMATTED);
    }
    auto &path = parsePath(argv, token, fallback);

    if (!path.isDirectory()) {
        throw AppError(Fault::FS_NOT_A_DIRECTORY, "Block " + std::to_string(path.nr));
    }
    return path;
}

void
NavigatorConsole::import(const FloppyDrive &dfn)
{
    fs.init(dfn);
}

void
NavigatorConsole::import(const HardDrive &hdn, isize part)
{
    fs.init(hdn, part);
}

void
NavigatorConsole::importDf(isize n)
{
    assert(n >= 0 && n <= 3);
    import(*amiga.df[n]);
}

void
NavigatorConsole::importHd(isize n, isize part)
{
    assert(n >= 0 && n <= 3);
    import(*amiga.hd[n], part);
}

void
NavigatorConsole::import(const fs::path &path, bool recursive, bool contents)
{
    fs.import(path, recursive, contents);
}

void
NavigatorConsole::exportBlocks(fs::path path)
{
    fs.exportBlocks(path);
}

FSBlock &
NavigatorConsole::matchPath(const Arguments &argv, const string &token, Tokens &notFound)
{
    return matchPath(argv.at(token), notFound);
}

FSBlock &
NavigatorConsole::matchPath(const Arguments &argv, const string &token, Tokens &notFound, FSBlock &fallback)
{
    return argv.contains(token) ? matchPath(argv, token, notFound) : fallback;
}

FSBlock &
NavigatorConsole::matchPath(const string &path, Tokens &notFound)
{
    if (!fs.isFormatted()) throw AppError(Fault::FS_UNFORMATTED);

    auto tokens = util::split(path, '/');
    if (!path.empty() && path[0] == '/') { tokens.insert(tokens.begin(), "/"); }

    auto *p = &fs.pwd();
    while (!tokens.empty()) {

        auto *next = fs.seekPtr(p, FSName(tokens.front()));
        if (!next) break;

        tokens.erase(tokens.begin());
        p = next;
    }
    notFound = tokens;

    return *p;
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

        .tokens = { "." },
        .chelp  = { "Switch to the next console" },
        .flags  = rs::hidden,

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            retroShell.enterCommander();
        }
    });

    root.add({

        .tokens = { ".." },
        .chelp  = { "Switch to the previous console" },
        .flags  = rs::hidden,

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            retroShell.enterDebugger();
        }
    });


    //
    // Importing and exporting
    //

    RSCommand::currentGroup = "Create";

    root.add({

        .tokens = { "create" },
        .chelp  = { "Create a file system with a particular capacity" },
    });

    root.add({

        .tokens = { "create", "SD" },
        .chelp  = { "Create a file system for a single-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.init(FSDescriptor(Diameter::INCH_525, Density::SD, FSFormat::NODOS));
            fs.dump(Category::Info, os);
        }
    });

    root.add({

        .tokens = { "create", "DD" },
        .chelp  = { "Create a file system for a double-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.init(FSDescriptor(Diameter::INCH_35, Density::DD, FSFormat::NODOS));
            fs.dump(Category::Info, os);
        }
    });

    root.add({

        .tokens = { "create", "HD" },
        .chelp  = { "Create a file system for a high-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.init(FSDescriptor(Diameter::INCH_35, Density::HD, FSFormat::NODOS));
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

                fs.init(FSDescriptor(blocks, FSFormat::NODOS));
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
                fs.init(FSDescriptor(geometry, FSFormat::NODOS));
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
                auto type = FSFormat::NODOS;
                auto dos = util::uppercased(args.at("dos"));
                if (dos == "OFS") type = FSFormat::OFS;
                if (dos == "FFS") type = FSFormat::FFS;

                if (type == FSFormat::NODOS) {
                    throw util::ParseError("Expected values: OFS or FFS");
                }

                // Format the device
                fs.format(type, args.contains("name") ? args.at("name") : "New Disk");
                fs.dump(Category::Info, os);
            }
    });

    root.add({

        .tokens = { "import" },
        .ghelp  = { "Import a file system" },
        .chelp  = { "Import a file or a folder from the host file system" },
        .flags  = vAmigaDOS ? rs::hidden : 0,
        .args   = {
            { .name = { "path", "Host file system directory" } },
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                fs.require_formatted();

                auto path = args.at("path");
                auto hostPath = host.makeAbsolute(args.at("path"));
                bool recursive = true;
                bool contents = path.back() == '/';

                fs.import(fs.pwd(), hostPath, recursive, contents);
            }
    });

    root.add({

        .tokens = { "import", "df[n]" },
        .ghelp  = { "Import file system from floppy drive n" },
        .chelp  = { "import { df0 | df1 | df1 | df2 }" },
        .flags  = vAmigaDOS ? rs::disabled : 0
    });

    for (isize i = 0; i < 4; i++) {

        root.add({

            .tokens = { "import", "df" + std::to_string(i) },
            .chelp  = { "Import file system from floppy drive" + std::to_string(i) },
            .flags  = vAmigaDOS ? rs::disabled : rs::shadowed,
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
        .chelp  = { "import { hd0 | hd1 | hd1 | hd2 }" },
        .flags  = vAmigaDOS ? rs::disabled : 0
    });

    for (isize i = 0; i < 4; i++) {

        root.add({

            .tokens = { "import", "hd" + std::to_string(i) },
            .chelp  = { "Import file system from hard drive" + std::to_string(i) },
            .flags  = vAmigaDOS ? rs::disabled : rs::shadowed,
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                auto n = values[0];

                fs.init(*hd[n], 0);
                fs.dump(Category::Info, os);

            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "import", "block" },
        .chelp  = { "Import a block from a file" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .args   = {
            { .name = { "nr", "Block number" }, .flags = rs::opt },
            { .name = { "path", "File path" } },
        },
            .func   = [&] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                auto path = host.makeAbsolute(args.at("path"));
                auto nr = parseBlock(args, "nr", fs.pwd().nr);

                fs.importBlock(nr, path);
            }
    });

    if constexpr (vAmigaDOS) {

        root.add({

            .tokens = { "export" },
            .ghelp  = { "Export the volume, files, directories, or blocks" },
            .chelp  = { "Export the volume, single files or directories" },
            .flags  = rs::ac,
            .args   = {
                { .name = { "file", "Export item" }, .flags = rs::opt },
                { .name = { "r", "Export subdirectories" }, .flags = rs::flag }
            },
                .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                    bool recursive = args.contains("r");
                    std::filesystem::remove_all("/export");

                    if (args.contains("file")) {

                        auto &item = parsePath(args, "file");
                        auto name = item.cppName();
                        if (name.empty()) name = fs.getName().cpp_str();
                        fs.exportFiles(item, "/export", recursive, true);
                        msgQueue.setPayload( { "/export", name } );

                    } else {

                        fs.exportBlocks("/export");
                        auto name = fs.getName().cpp_str();
                        name += fs.getTraits().adf() ? ".adf" : ".hdf";
                        msgQueue.setPayload( { "/export", name } );
                    }
                    
                    msgQueue.put(Msg::RSH_EXPORT);
                }
        });

    } else {

        root.add({

            .tokens = { "export" },
            .ghelp  = { "Export files, directories, or blocks" },
            .chelp  = { "Export a file or directory to the host file system" },
            .flags  = rs::ac,
            .args   = {
                { .name = { "file", "Export item" } },
                { .name = { "path", "Host file system location" } },
                { .name = { "r", "Export subdirectories" }, .flags = rs::flag }
            },
                .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                    auto &item = parsePath(args, "file");
                    bool recursive = args.contains("r");
                    bool contents = args.at("file").back() == '/';

                    auto path = args.at("path");
                    auto hostPath = host.makeAbsolute(args.at("path"));
                    fs.exportFiles(item, hostPath, recursive, contents);
                }
        });
    }

    root.add({

        .tokens = { "export", "df[n]" },
        .ghelp  = { "Export the file system to floppy drive n" },
        .chelp  = { "export { df0 | df1 | df1 | df2 }" },
        .flags  = vAmigaDOS ? rs::disabled : 0
    });

    for (isize i = 0; i < 4; i++) {

        root.add({

            .tokens = { "export", "df" + std::to_string(i) },
            .chelp  = { "Export the file system to floppy drive" + std::to_string(i) },
            .flags  = vAmigaDOS ? rs::disabled : rs::shadowed,
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                auto n = values[0];
                df[n]->insertMediaFile(ADFFile(fs), false);

            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "export", "hd[n]" },
        .ghelp  = { "Export the file system to hard drive n" },
        .chelp  = { "export { hd0 | hd1 | hd1 | hd2 }" },
        .flags  = vAmigaDOS ? rs::disabled : 0
    });

    for (isize i = 0; i < 4; i++) {

        root.add({

            .tokens = { "export", "hd" + std::to_string(i) },
            .chelp  = { "Export the file system to hard drive" + std::to_string(i) },
            .flags  = vAmigaDOS ? rs::disabled : rs::shadowed,
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                auto n = values[0];
                hd[n]->init(fs);

            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "export", "block" },
        .chelp  = { "Export a block to a file" },
        .args   = {
            { .name = { "nr", "Block number" }, .flags = rs::opt },
            { .name = { "path", "File path" }, .flags = vAmigaDOS ? rs::disabled : 0 },
        },
            .func   = [&] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                auto nr = parseBlock(args, "nr", fs.pwd().nr);

                if constexpr (vAmigaDOS) {

                    fs.exportBlock(nr, "blob");
                    msgQueue.setPayload( { "blob", std::to_string(nr) + ".bin" } );
                    msgQueue.put(Msg::RSH_EXPORT);

                } else {

                    auto path = host.makeAbsolute(args.at("path"));
                    fs.exportBlock(nr, path);
                }
            }
    });

    RSCommand::currentGroup = "Navigate";

    root.add({

        .tokens = { "cd" },
        .chelp  = { "Change the working directory" },
        .flags  = rs::acdir,
        .args   = {
            { .name = { "path", "New working directory" }, .flags = rs::opt },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto &path = parsePath(args, "path", fs.root());
            fs.cd(path);
        }
    });

    root.add({

        .tokens = { "dir" },
        .chelp  = { "Display a sorted list of the files in a directory" },
        .flags  = rs::acdir,
        .args   = {
            { .name = { "path", "Path to directory" }, .flags = rs::opt },
            { .name = { "d", "List directories only" }, .flags = rs::flag },
            { .name = { "f", "List files only" }, .flags = rs::flag },
            { .name = { "r", "Display subdirectories" }, .flags = rs::flag }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto &path = parseDirectory(args, "path");
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");

            FSOpt opt = {

                .recursive = r,
                .sort = sort::dafa,
                .filter = [&](const FSBlock &item) {

                    return true;
                },
                .formatter = [&](const FSBlock &node) {

                    return node.cppName() + (node.isDirectory() ? " (dir)" : "\t");
                }
            };

            FSOpt opt2 = {

                .recursive = r,
                .sort = sort::dafa,
                .filter = [&](const FSBlock &item) {

                    return (!d || item.isDirectory()) && (!f || item.isFile());
                },
                .formatter = [&](const FSBlock &node) {

                    return node.cppName() + (node.isDirectory() ? " (dir)" : "\t");
                }
            };

            FSTree(path, opt).list(os, opt2);
        }
    });

    root.add({

        .tokens = { "list" },
        .chelp  = { "List specified information about directories and files" },
        .flags  = rs::acdir,
        .args   = {
            { .name = { "path", "Path to directory" }, .flags = rs::opt },
            { .name = { "d", "List directories only" }, .flags = rs::flag },
            { .name = { "f", "List files only" }, .flags = rs::flag },
            { .name = { "r", "List subdirectories" }, .flags = rs::flag },
            { .name = { "k", "Display keys (start blocks)" }, .flags = rs::flag },
            { .name = { "s", "Sort output" }, .flags = rs::flag } },
        .func   = [this](std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto &path = parseDirectory(args, "path");
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto r = args.contains("r");
            auto k = args.contains("k");
            auto s = args.contains("s");

            FSOpt opt = {

                .recursive = r,
                .sort = s ? sort::alpha : sort::none,
                .filter = [&](const FSBlock &item) {

                    return (!d || item.isDirectory()) && (!f || item.isFile());
                },
                .formatter = [&](const FSBlock &node) {

                    std::stringstream ss;
                    ss << std::left << std::setw(25) << node.cppName();

                    if (k) { ss << std::right << std::setw(9) << ("[" + std::to_string(node.nr) + "] "); }

                    if (node.isDirectory()) {
                        ss << std::right << std::setw(7) << "Dir";
                    } else {
                        ss << std::right << std::setw(7) << std::to_string(node.getFileSize());
                    }
                    ss << " " << node.getProtectionBitString();
                    ss << " " << node.getCreationDate().str();

                    return ss.str();
                }
            };

            FSTree(path, opt).list(os, opt);
        }
    });

    root.add({

        .tokens = { "find" },
        .chelp  = { "Find files or directories" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "name", "Search pattern" } },
            { .name = { "d", "Find directories only" }, .flags = rs::flag },
            { .name = { "f", "Find files only" }, .flags = rs::flag },
            { .name = { "s", "Sort output" }, .flags = rs::flag } },
        .func   = [this](std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto pattern = FSPattern(args.at("name"));
            auto d = args.contains("d");
            auto f = args.contains("f");
            auto s = args.contains("s");

            // Find all items matching the search pattern
            auto matches = fs.find(pattern);

            // Filter the result
            matches.erase(std::remove_if(matches.begin(), matches.end(), [&](auto *node) {
                return (d && !node->isDirectory()) || (f && !node->isFile());
            }), matches.end());

            
            if (s) {

                int tab = 0;

                std::sort(matches.begin(), matches.end(),
                          [](auto *b1, auto *b2) { return b1->getName() < b2->getName(); });

                for (auto &it : matches) {
                    tab = std::max(int(it->cppName().size()), tab);
                }
                for (auto &it : matches) {
                    os << std::setw(tab) << std::left << it->cppName() << " : " << it->absName() << '\n';
                }

            } else {

                for (auto &it : matches) { os << it->absName() << '\n'; }
            }
        }
    });

    RSCommand::currentGroup = "Inspect";

    root.add({

        .tokens = { "info" },
        .chelp  = { "Print a file system summary" },
        .args   = {
            { .name = { "b", "Inspect the block storage" }, .flags = rs::flag },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            if (args.contains("b")) {
                fs.dump(Category::Storage, os);
            } else {
                fs.dump(Category::Info, os);
            }

        }
    });

    root.add({

        .tokens = { "file" },
        .ghelp  = { "Manage files" },
        .chelp  = { "Inspect a single file" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "path", "File path" }, .flags = rs::opt },
            { .name = { "v", "Verbose output" }, .flags = rs::flag },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto &file = parseFile(args, "path");
            file.dump(args.contains("v") ? Category::Blocks : Category::Info, os);
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
            fs.doctor.dump(nr, os);
        }
    });

    root.add({

        .tokens = { "boot" },
        .ghelp  = { "Manage the boot block" },
    });

    root.add({

        .tokens = { "boot", "install" },
        .chelp  = { "Installs a block block" },
    });

    for (const auto& [key, value] : BootBlockIdEnum::pairs()) {

        root.add({

            .tokens = { "boot", "install", key },
            .chelp  = { BootBlockIdEnum::help(BootBlockId(value)) },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                fs.require_formatted();
                fs.makeBootable(BootBlockId(values[0]));

            },  .payload = { value }
        });
    }

    root.add({

        .tokens = { "boot", "scan" },
        .chelp  = { "Scan a boot block for viruses" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.require_formatted();
            os << "Boot block: " << fs.getBootBlockName() << std::endl;
        }
    });

    root.add({

        .tokens = { "boot", "kill" },
        .chelp  = { "Kills a boot block virus" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs.require_formatted();
            fs.killVirus();
        }
    });

    root.add({

        .tokens = { "type" },
        .chelp  = { "Print the contents of a file" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "path", "File path" }, .flags = rs::opt },
            { .name = { "l", "Display a line number in each row" }, .flags = rs::flag },
            { .name = { "t", "Display the last part" }, .flags = rs::flag },
            { .name = { "lines", "Number of displayed rows" }, .flags = rs::keyval|rs::opt },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto &file = parsePath(args, "path", fs.pwd());
            if (!file.isFile()) {
                throw AppError(Fault::FS_NOT_A_FILE, "Block " + std::to_string(file.nr));
            }

            auto lines = args.contains("lines") ? parseNum(args.at("lines")) : -1;

            Buffer<u8> buffer;
            file.extractData(buffer);

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
        .flags  = rs::ac,
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

            auto &file = parseFile(args, "path", fs.pwd());
            auto opt = parseDumpOpts(args);

            Buffer<u8> buffer;
            file.extractData(buffer);
            buffer.dump(os, opt);
        }
    });

    root.add({

        .tokens = { "dump", "block" },
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

            auto nr = parseBlock(args, "nr", fs.pwd().nr);
            auto opt = parseDumpOpts(args);

            if (auto ptr = fs.read(nr); ptr) {

                ptr->hexDump(os, opt);
            }
        }
    });

    root.add({

        .tokens = { "xray" },
        .ghelp  = { "Examines the file system integrity" },
        .chelp  = { "Inspects the entire file system or a single block" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "s", "Strict checking" }, .flags = rs::flag },
            { .name = { "v", "Verbose output" }, .flags = rs::flag },
            { .name = { "r", "Rectify errors" }, .flags = rs::flag },
            { .name = { "nr", "Block number" }, .flags = rs::opt }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            bool strict = args.contains("s");

            if (args.contains("nr")) {

                auto nr = parseBlock(args, "nr");

                if (args.contains("r")) fs.doctor.rectify(nr, strict);
                if (auto errors = fs.doctor.xray(nr, strict, os); !errors) {
                    os << "No findings." << std::endl;
                }

            } else {

                if (args.contains("r")) fs.doctor.rectify(strict);
                if (auto errors = fs.doctor.xray(strict, os, args.contains("v")); !errors) {
                    os << "No findings." << std::endl;
                }
            }
        }
    });

    RSCommand::currentGroup = "Modify";

    root.add({

        .tokens = { "mkdir" },
        .chelp  = { "Create a directory" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "name", "Name of the new directory" } },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            Tokens missing;
            auto &path = matchPath(args.at("name"), missing);

            if (missing.empty()) {
                throw(AppError(Fault::FS_EXISTS, args.at("name")));
            }
            auto *p = &path;
            for (auto &it: missing) {
                if (p) p = &fs.createDir(*p, FSName(it));
            }
        }
    });

    root.add({

        .tokens = { "move" },
        .chelp  = { "Moves a file or directory" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "source", "Item to move" } },
            { .name = { "target", "New name or target directory" } },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto &source = parsePath(args, "source");

            Tokens missing;
            auto &path = matchPath(args.at("target"), missing);

            printf("%s -> '%s' {", source.absName().c_str(), path.absName().c_str());
            for (auto &it : missing) printf(" %s", it.c_str());
            printf(" }\n");

            if (missing.empty()) {

                if (path.isFile()) {

                    throw AppError(Fault::FS_EXISTS, args.at("target"));
                }
                if (path.isDirectory()) {

                    debug(RSH_DEBUG, "Moving '%s' to '%s'\n", source.absName().c_str(), path.absName().c_str());
                    fs.move(source, path);
                }

            } else if (missing.size() == 1) {

                debug(RSH_DEBUG, "Moving '%s' to '%s' / '%s'\n",
                      source.absName().c_str(), path.absName().c_str(), missing.back().c_str());
                fs.move(source, path, missing.back());

            } else {

                throw AppError(Fault::FS_NOT_FOUND, missing.front());
            }
        }
    });

    root.add({

        .tokens = { "copy" },
        .chelp  = { "Copies a file" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "source", "Item to copy" } },
            { .name = { "target", "New name or target directory" } },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto &source = parsePath(args, "source");

            Tokens missing;
            auto &path = matchPath(args.at("target"), missing);

            if (missing.empty()) {

                if (path.isFile()) {

                    throw AppError(Fault::FS_EXISTS, args.at("target"));
                }
                if (path.isDirectory()) {

                    fs.copy(source, path);
                }

            } else if (missing.size() == 1) {

                fs.copy(source, path, missing.back());

            } else {

                throw AppError(Fault::FS_NOT_FOUND, missing.front());
            }
        }
    });

    root.add({

        .tokens = { "delete" },
        .chelp  = { "Deletes a file" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "path", "File to delete" } },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto &path = parsePath(args, "path");

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

