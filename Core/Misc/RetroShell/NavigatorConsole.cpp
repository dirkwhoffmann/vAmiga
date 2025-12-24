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
#include "RSError.h"
#include "Emulator.h"
#include "ADFFactory.h"
#include "MediaFile.h"
#include "HDFFile.h"
#include "utl/chrono.h"
#include "utl/support.h"
#include "utl/io.h"
#include <regex>

namespace vamiga {

void
NavigatorConsole::_pause()
{
    
}

string
NavigatorConsole::prompt()
{
    std::stringstream ss;
    
    if (fs) {

        auto &pwd = fs->fetch(fs->pwd());

        ss << "[" << std::to_string(pwd.nr) << "]";
        
        auto fsName = fs->stat().name;
        if (!fsName.empty()) ss << " " << fsName << ":";
        if (pwd.isDirectory()) ss << " " << pwd.absName();
    }
    
    ss << "> ";
    return ss.str();
}

void
NavigatorConsole::didActivate()
{

}

void
NavigatorConsole::didDeactivate()
{

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
    try {

        requireFormattedFS();

        auto path = FSPath(input);
        auto dir  = path.parentPath();

        // Find all matching items
        auto matches = fs->match(input + "*");

        // Case 1: The completion was unique
        if (matches.size() == 1) {

            auto &node = fs->fetch(matches[0]);
            auto name = dir / node.name();
            return name.cpp_str() + (node.isDirectory() ? "/" : "");
        }

        // Case 2: Multiple files match
        std::vector<string> names;
        for (auto &it : matches) {

            auto name = dir / fs->fetch(it).name();
            names.push_back(name.cpp_str());
        }

        // Auto-complete all common characters
        return utl::commonPrefix(names, false);

    } catch (...) {

        return input;
    }
}

void
NavigatorConsole::help(std::ostream &os, const string &argv, isize tabs)
{
    try {

        auto [cmd, args] = seekCommand(argv);

        // Determine the kind of help to display
        bool displayFiles = fs && fs->isFormatted() && cmd && cmd->callback && (cmd->flags & rs::ac);
        bool displayCmds  = true;

        if (displayCmds) {

            // Display the standard command help
            Console::help(os, argv, tabs);
        }

        if (displayFiles) {

            // Find matching items
            auto matches = fs->match(args.empty() ? "*" : args.back() + "*");

            // Extract names
            vector<string> dirs, files;
            for (auto &it : matches) {

                auto &block = fs->fetch(it);
                auto name = block.name().cpp_str();

                if (block.isDirectory()) {
                    dirs.push_back(name + " (dir)");
                } else {
                    files.push_back(name);
                }
            }

            // Sort
            auto ciLess = [](const std::string &a, const std::string &b) {
                return std::lexicographical_compare(
                                                    a.begin(), a.end(),
                                                    b.begin(), b.end(),
                                                    [](unsigned char x, unsigned char y) {
                                                        return std::tolower(x) < std::tolower(y);
                                                    }
                                                    );
            };

            std::sort(dirs.begin(), dirs.end(), ciLess);
            std::sort(files.begin(), files.end(), ciLess);

            // Print
            if (!matches.empty() && displayCmds) {

                os << std::endl;
                Formatter::printTable(os, dirs, {
                    .columns = {
                        { .align = 'l', .width = 35 }
                    },
                        .layout = Formatter::Layout::RowMajor,
                        .inset  = string(7, ' ')
                });
                Formatter::printTable(os, files, {
                    .columns = {
                        { .align = 'l', .width = 35 },
                        { .align = 'l', .width = 35 }
                    },
                        .layout = Formatter::Layout::RowMajor,
                        .inset  = string(7, ' ')
                });
            }
        }

    }
    catch (...) { }
}

BlockNr
NavigatorConsole::parseBlock(const string &argv)
{
    if (auto nr = BlockNr(parseNum(argv)); fs->tryFetch(nr)) {
        return nr;
    }
    
    throw CoreError(CoreError::OPT_INV_ARG, "0..." + std::to_string(fs->blocks()));
}

BlockNr
NavigatorConsole::parseBlock(const Arguments &argv, const string &token)
{
    return parseBlock(argv, token, fs->pwd());
}

BlockNr
NavigatorConsole::parseBlock(const Arguments &argv, const string &token, BlockNr fallback)
{
    auto nr = argv.contains(token) ? BlockNr(parseNum(argv.at(token))) : fallback;
    
    if (!fs->tryFetch(nr)) {
        throw CoreError(CoreError::OPT_INV_ARG, "0..." + std::to_string(fs->blocks()));
    }
    return nr;
}

BlockNr
NavigatorConsole::parsePath(const Arguments &argv, const string &token)
{
    fs->require.isFormatted();

    assert(argv.contains(token));

    try {
        // Try to find the directory by name
        return fs->seek(argv.at(token));

    } catch (...) {
        
        try {
            // Treat the argument as a block number
            // return (*fs)[parseBlock(argv.at(token))];
            return parseBlock(argv.at(token));

        } catch (...) {
            
            // The item does not exist
            throw FSError(FSError::FS_NOT_FOUND, argv.at(token));
        }
    }
}

BlockNr
NavigatorConsole::parsePath(const Arguments &argv, const string &token, BlockNr fallback)
{
    return argv.contains(token) ? parsePath(argv, token) : fallback;
}

BlockNr
NavigatorConsole::parseFile(const Arguments &argv, const string &token)
{
    return parseFile(argv, token, fs->pwd());
}

BlockNr
NavigatorConsole::parseFile(const Arguments &argv, const string &token, BlockNr fallback)
{
    auto path = parsePath(argv, token, fallback);
    fs->require.file(path);

    return path;
}

BlockNr
NavigatorConsole::parseDirectory(const Arguments &argv, const string &token)
{
    return parseDirectory(argv, token, fs->pwd());
}

BlockNr
NavigatorConsole::parseDirectory(const Arguments &argv, const string &token, BlockNr fallback)
{
    auto path = parsePath(argv, token, fallback);
    fs->require.directory(path);

    return path;
}

void
NavigatorConsole::import(const FloppyDrive &dfn)
{
    // Later: Directly mount the file system on top of the drive

    // Create a block device
    adf = ADFFactory::make(dfn);

    // Create a file system on top
    auto vol = Volume(*adf);
    fs = make_unique<FileSystem>(vol);
}

void
NavigatorConsole::import(const HardDrive &hdn, isize part)
{
    throw FSError(FSError::FS_UNSUPPORTED);
    /*
    FileSystemFactory::initFromHardDrive(*fs, hdn);
    */
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
    fs->importer.import(path, recursive, contents);
}

void
NavigatorConsole::requireFS() const
{
    if (!fs) throw FSError(FSError::FS_UNKNOWN, "No file system present");
}

void
NavigatorConsole::requireFormattedFS() const
{
    requireFS();
    fs->require.isFormatted();
}

void
NavigatorConsole::exportBlocks(fs::path path)
{
    fs->exporter.exportBlocks(path);
}

BlockNr
NavigatorConsole::matchPath(const Arguments &argv, const string &token, Tokens &notFound)
{
    return matchPath(argv.at(token), notFound);
}

BlockNr
NavigatorConsole::matchPath(const Arguments &argv, const string &token, Tokens &notFound, BlockNr fallback)
{
    return argv.contains(token) ? matchPath(argv, token, notFound) : fallback;
}

BlockNr
NavigatorConsole::matchPath(const string &path, Tokens &notFound)
{
    fs->require.isFormatted();

    auto tokens = utl::split(path, '/');
    if (!path.empty() && path[0] == '/') { tokens.insert(tokens.begin(), "/"); }
    
    auto p = fs->pwd();
    while (!tokens.empty()) {
        
        auto next = fs->trySeek(tokens.front());
        if (!next) break;

        tokens.erase(tokens.begin());
        p = *next;
    }
    notFound = tokens;
    
    return p;
}

DumpOpt
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
        throw RSError(RSError::GENERIC, "Flags -a, -o, -d are mutually exclusive");
    }
    if ((int)a + (int)w + (int)l > 1) {
        throw RSError(RSError::GENERIC, "Flags -a, -w, -l are mutually exclusive");
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
    // Empty command
    //

    root.add({

        .tokens = { "return" },
        .chelp  = { "Print status information" },
        .flags  = rs::hidden,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            if (fs) {

                fs->dumpInfo(os);

            } else {

                os << "    No file system present.\n";
                os << "    Use the 'import' command to load one.";
            }
        }
    });


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

    /*
    RSCommand::currentGroup = "Create";
    
    root.add({
        
        .tokens = { "create" },
        .chelp  = { "Create a file system with a particular capacity" },
    });
    
    root.add({
        
        .tokens = { "create", "SD" },
        .chelp  = { "Create a file system for a single-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs->init(FSDescriptor(Diameter::INCH_525, Density::SD, FSFormat::NODOS));
            fs->dumpInfo(os);
        }
    });
    
    root.add({
        
        .tokens = { "create", "DD" },
        .chelp  = { "Create a file system for a double-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs->init(FSDescriptor(Diameter::INCH_35, Density::DD, FSFormat::NODOS));
            fs->dumpInfo(os);
        }
    });
    
    root.add({
        
        .tokens = { "create", "HD" },
        .chelp  = { "Create a file system for a high-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            fs->init(FSDescriptor(Diameter::INCH_35, Density::HD, FSFormat::NODOS));
            fs->dumpInfo(os);
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

                dev = make_unique<Device>(GeometryDescriptor(blocks));
                fs = make_unique<FileSystem>(*dev, FSDescriptor(blocks, FSFormat::NODOS));
                fs->dumpInfo(os);
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
                dev = make_unique<Device>(geometry);
                fs = make_unique<FileSystem>(*dev, FSDescriptor(geometry, FSFormat::NODOS));
                fs->dumpInfo(os);
            }
    });
    */
    
    root.add({
        
        .tokens = { "format" },
        .chelp  = { "Format the file system" },
        .args   = {
            { .name = { "dos", "Amiga file system" }, .key = "{ OFS | FFS }" },
            { .name = { "name", "File system name" }, .flags = rs::opt },
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                requireFS();

                // Determine the DOS type
                auto type = FSFormat::NODOS;
                auto dos = utl::uppercased(args.at("dos"));
                if (dos == "OFS") type = FSFormat::OFS;
                if (dos == "FFS") type = FSFormat::FFS;
                
                if (type == FSFormat::NODOS) {
                    throw RSError(RSError::GENERIC, "Expected values: OFS or FFS");
                }
                
                // Format the device
                auto name = args.contains("name") ? args.at("name") : "New Disk";
                fs->format(type);
                fs->setName(FSName(name));
                fs->dumpInfo(os);
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

                requireFormattedFS();

                auto path = args.at("path");
                auto hostPath = host.makeAbsolute(args.at("path"));
                bool recursive = true;
                bool contents = path.back() == '/';
                
                fs->importer.import(fs->pwd(), hostPath, recursive, contents);
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

                adf = ADFFactory::make(*df[n]);
                vol = make_unique<Volume>(*adf);
                fs  = make_unique<FileSystem>(*vol);

                fs->dumpInfo(os);

            }, .payload = {i}
        });
    }

    /* UNCOMMENT THIS LATER...
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

                auto desc = hd[n]->getPartitionDescriptor(0);
                dev = make_unique<Device>(desc.geometry());
                fs = FileSystemFactory::fromHardDrive(*dev, *hd[n]);
                fs->dumpInfo(os);

            }, .payload = {i}
        });
    }
    */
    
    root.add({
        
        .tokens = { "import", "block" },
        .chelp  = { "Import a block from a file" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .args   = {
            { .name = { "nr", "Block number" }, .flags = rs::opt },
            { .name = { "path", "File path" } },
        },
            .func   = [&] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                requireFS();

                auto path = host.makeAbsolute(args.at("path"));
                auto nr = parseBlock(args, "nr", fs->pwd());

                fs->importer.importBlock(nr, path);
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

                    requireFormattedFS();

                    bool recursive = args.contains("r");
                    std::filesystem::remove_all("/export");
                    
                    if (args.contains("file")) {
                        
                        auto item = parsePath(args, "file");
                        // auto &item = fs->fetch(itemNr);
                        auto name = fs->fetch(item).cppName();
                        if (name.empty()) name = fs->stat().name.cpp_str();
                        fs->exporter.exportFiles(item, "/export", recursive, true);
                        msgQueue.setPayload( { "/export", name } );
                        
                    } else {
                        
                        fs->exporter.exportBlocks("/export");
                        auto name = fs->stat().name.cpp_str();
                        name += fs->getTraits().adf() ? ".adf" : ".hdf";
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

                    requireFormattedFS();

                    auto itemNr = parsePath(args, "file");
                    bool recursive = args.contains("r");
                    bool contents = args.at("file").back() == '/';
                    
                    auto path = args.at("path");
                    auto hostPath = host.makeAbsolute(args.at("path"));
                    fs->exporter.exportFiles(itemNr, hostPath, recursive, contents);
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

                requireFormattedFS();

                auto n = values[0];
                auto tmp = MediaFile(ADFFactory::make(*fs));
                df[n]->insertMediaFile(tmp, false);

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

                requireFormattedFS();

                auto n = values[0];
                hd[n]->init(*fs);

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

                requireFormattedFS();

                auto nr = parseBlock(args, "nr", fs->pwd());

                if constexpr (vAmigaDOS) {
                    
                    fs->exporter.exportBlock(nr, "blob");
                    msgQueue.setPayload( { "blob", std::to_string(nr) + ".bin" } );
                    msgQueue.put(Msg::RSH_EXPORT);
                    
                } else {
                    
                    auto path = host.makeAbsolute(args.at("path"));
                    fs->exporter.exportBlock(nr, path);
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

                requireFormattedFS();

                auto path = parsePath(args, "path", fs->root());
                fs->cd(path);
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

                requireFormattedFS();

                auto path = parseDirectory(args, "path");
                auto d = args.contains("d");
                auto f = args.contains("f");
                auto r = args.contains("r");

                // Collect the directories to print
                FSTree tree = fs->build(path, {
                    .accept = accept::directories,
                    .sort   = sort::alpha,
                    .depth  = r ? MAX_ISIZE : 0
                });

                // For each directory...
                for (const auto &node : tree.dfs()) {

                    // Print header
                    if (node.nr != tree.nr) os << "\n";
                    os << "Directory " << fs->fetch(node.nr).absName() << ":\n\n";

                    if (!f) {

                        // Collect directory items
                        FSTree items = fs->build(node.nr, {
                            .accept = accept::directories,
                            .sort   = sort::alpha,
                            .depth  = 1
                        });

                        // Extract names
                        vector<string> names;
                        for (const auto &child : items.children) {
                            names.push_back(fs->fetch(child.nr).cppName() + " (dir)");
                        }

                        // Print names
                        Formatter::printTable(os, names, {
                            .columns = {
                                { .align = 'l', .width = 0  }
                            }
                        });
                    }

                    if (!d) {

                        // Collect file items
                        FSTree items = fs->build(node.nr, {
                            .accept = accept::files,
                            .sort   = sort::alpha,
                            .depth  = 1
                        });

                        // Extract names
                        vector<string> names;
                        for (const auto &node : items.children) {
                            names.push_back(fs->fetch(node.nr).cppName());
                        }

                        // Print names
                        Formatter::printTable(os, names, {
                            .columns = {
                                { .align = 'l', .width = 35 },
                                { .align = 'l', .width = 0  }
                            },
                            .layout = Formatter::Layout::RowMajor
                        });
                    }
                }
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

                requireFormattedFS();

                auto path = parseDirectory(args, "path");
                auto d = args.contains("d");
                auto f = args.contains("f");
                auto r = args.contains("r");
                auto k = args.contains("k");
                auto s = args.contains("s");

                // Formats the output for a single item
                auto formatted = [&](BlockNr nr) {

                    auto &node = fs->fetch(nr);

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
                };

                // Collect the directories to print
                FSTree tree = fs->build(path, {
                    .accept = accept::directories,
                    .sort   = s ? sort::alpha : sort::none,
                    .depth  = r ? MAX_ISIZE : 0
                });

                // For each directory...
                for (const auto &node : tree.dfs()) {

                    // Print header
                    if (node.nr != tree.nr) os << "\n";
                    os << "Directory " << fs->fetch(node.nr).absName() << ":\n\n";

                    // Collect items
                    FSTree items = fs->build(node.nr, {
                        .accept = f ? accept::files : d ? accept::directories : accept::all,
                        .sort   = sort::alpha,
                        .depth  = 1
                    });

                    // Print items
                    for (auto &it : items.children) {
                        os << formatted(it.nr) << "\n";
                    }
                }
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

                requireFormattedFS();

                auto pattern = FSPattern(args.at("name"));
                auto d = args.contains("d");
                auto f = args.contains("f");
                auto s = args.contains("s");

                // Determine the start node
                auto start = fs->pwd();

                // Build a directory tree
                FSTree tree = fs->build(start, {
                    .accept = accept::all,
                    .sort   = sort::none,
                    .depth  = MAX_ISIZE
                });

                // Traverse the tree and find matches
                vector<const FSBlock *> matching;
                for (const auto &node : tree.bfs()) {

                    auto &block = fs->fetch(node.nr);

                    if (!pattern.match(block.cppName())) continue;
                    if (d && !block.isDirectory())       continue;
                    if (f && !block.isFile())            continue;

                    matching.push_back(&block);
                }

                // Print the result
                if (s) {

                    int tab = 0;

                    std::sort(matching.begin(), matching.end(), sort::alphaPtr);

                    for (auto &it : matching) {
                        tab = std::max(int(it->cppName().size()), tab);
                    }
                    for (auto &it : matching) {
                        os << std::setw(tab) << std::left << it->cppName() << " : " << it->absName() << '\n';
                    }

                } else {

                    for (auto &it : matching) { os << it->absName() << '\n'; }
                }
            }
    });

    root.add({

        .tokens = { "resolve" },
        .chelp  = { "Resolves a path name" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "name", "Search pattern" } },
        },
            .func   = [this](std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                requireFormattedFS();

                // Find matches
                vector <BlockNr> matches = fs->match(args.at("name"));

                // Print the result
                for (auto &it : matches) { os << fs->fetch(it).absName() << '\n'; }
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
                    fs->dumpBlocks(os);
                } else {
                    fs->dumpInfo(os);
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

                requireFormattedFS();

                auto &file = fs->fetch(parseFile(args, "path"));
                args.contains("v") ? file.dumpBlocks(os) : file.dumpInfo(os);
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

                requireFormattedFS();

                auto nr = parseBlock(args, "nr");
                fs->doctor.dump(nr, os);
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
                
                requireFormattedFS();

                fs->makeBootable(BootBlockId(values[0]));

            },  .payload = { value }
        });
    }
    
    root.add({
        
        .tokens = { "boot", "scan" },
        .chelp  = { "Scan a boot block for viruses" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            requireFormattedFS();

            os << "Boot block: " << fs->bootStat().name << std::endl;
        }
    });
    
    root.add({
        
        .tokens = { "boot", "kill" },
        .chelp  = { "Kills a boot block virus" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            requireFormattedFS();

            fs->killVirus();
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

                requireFormattedFS();

                auto &file = fs->fetch(parsePath(args, "path", fs->pwd()));
                if (!file.isFile()) {
                    throw FSError(FSError::FS_NOT_A_FILE, "Block " + std::to_string(file.nr));
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

                requireFormattedFS();

                auto &file = fs->fetch(parseFile(args, "path", fs->pwd()));
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

                requireFormattedFS();

                auto nr = parseBlock(args, "nr", fs->pwd());
                auto opt = parseDumpOpts(args);

                fs->fetch(nr).hexDump(os, opt);
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

                requireFormattedFS();

                bool strict = args.contains("s");
                
                if (args.contains("nr")) {
                    
                    auto nr = parseBlock(args, "nr");
                    
                    if (args.contains("r")) fs->doctor.rectify(nr, strict);
                    if (auto errors = fs->doctor.xray(nr, strict, os); !errors) {
                        os << "No findings." << std::endl;
                    }
                    
                } else {
                    
                    if (args.contains("r")) fs->doctor.rectify(strict);
                    if (auto errors = fs->doctor.xray(strict, os, args.contains("v")); !errors) {
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

                requireFormattedFS();

                Tokens missing;
                auto path = matchPath(args.at("name"), missing);
                
                if (missing.empty()) {
                    throw(FSError(FSError::FS_EXISTS, args.at("name")));
                }

                auto p = path;
                for (auto &it: missing) {
                    p = fs->mkdir(p, FSName(it));
                }
                
                /*
                auto *p = &path.mutate();
                for (auto &it: missing) {
                    if (p) p = &fs->mkdir(*p, FSName(it));
                }
                */
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

                requireFormattedFS();

                auto sourceNr = parsePath(args, "source");
                auto &source = fs->fetch(sourceNr);

                Tokens missing;
                auto pathNr = matchPath(args.at("target"), missing);
                auto &path = fs->fetch(pathNr);

                printf("%s -> '%s' {", source.absName().c_str(), path.absName().c_str());
                for (auto &it : missing) printf(" %s", it.c_str());
                printf(" }\n");
                
                if (missing.empty()) {
                    
                    if (path.isFile()) {
                        
                        throw FSError(FSError::FS_EXISTS, args.at("target"));
                    }
                    if (path.isDirectory()) {
                        
                        debug(RSH_DEBUG, "Moving '%s' to '%s'\n", source.absName().c_str(), path.absName().c_str());
                        fs->move(sourceNr, pathNr);
                    }
                    
                } else if (missing.size() == 1) {
                    
                    debug(RSH_DEBUG, "Moving '%s' to '%s' / '%s'\n",
                          source.absName().c_str(), path.absName().c_str(), missing.back().c_str());
                    fs->move(sourceNr, pathNr, FSName(missing.back()));

                } else {
                    
                    throw FSError(FSError::FS_NOT_FOUND, missing.front());
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

                requireFormattedFS();

                auto sourceNr = parsePath(args, "source");

                Tokens missing;
                auto pathNr = matchPath(args.at("target"), missing);
                auto &path = fs->fetch(pathNr);

                if (missing.empty()) {
                    
                    if (path.isFile()) {
                        
                        throw FSError(FSError::FS_EXISTS, args.at("target"));
                    }
                    if (path.isDirectory()) {
                        
                        fs->copy(sourceNr, pathNr);
                    }
                    
                } else if (missing.size() == 1) {
                    
                    fs->copy(sourceNr, pathNr, FSName(missing.back()));

                } else {
                    
                    throw FSError(FSError::FS_NOT_FOUND, missing.front());
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

                requireFormattedFS();

                auto &path = fs->fetch(parsePath(args, "path"));

                if (path.isFile()) {
                    fs->rm(path.nr);
                } else if (path.isDirectory()) {
                    throw FSError(FSError::FS_NOT_A_FILE, args.at("path"));
                } else {
                    throw FSError(FSError::FS_NOT_A_FILE_OR_DIRECTORY, args.at("path"));
                }
                
            }
    });
}

}

