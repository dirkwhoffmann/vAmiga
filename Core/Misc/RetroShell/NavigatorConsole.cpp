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
#include "Codecs.h"
#include "HDFFile.h"
#include "utl/chrono.h"
#include "utl/support.h"
#include "utl/io.h"
#include <climits>
#include <regex>

namespace vamiga {

namespace accept = retro::vault::amiga::accept;
namespace sort = retro::vault::amiga::sort;
using retro::vault::FSError;
using retro::vault::amiga::BootBlockId;
using retro::vault::amiga::BootBlockIdEnum;
using retro::vault::amiga::FSPath;
using retro::vault::amiga::FSPattern;
using retro::vault::amiga::FSTree;

string
Console::navPrompt()
{
    std::stringstream ss;

    if (amigaFs) {

        auto &pwd = amigaFs->fetch(amigaFs->pwd());

        ss << "[" << std::to_string(pwd.nr) << "]";

        auto fsName = amigaFs->stat().name;
        if (!fsName.empty()) ss << " " << fsName << ":";
        if (pwd.isDirectory()) ss << " " << pwd.absName();
    }

    ss << "> ";
    return ss.str();
}

void
Console::navAutoComplete(Tokens &argv)
{
    // Only proceed if there is anything to complete
    if (argv.empty()) return;

    if (auto [cmd, remaining] = seekCommand(argv); remaining.size() > 0) {

        // First, try to auto-complete the last token with a command name
        if (remaining.size() != 1 || !cmd->autoComplete(argv.back())) {

            // If that didn't work, try to auto-complete with a file name
            try {
                auto prefix = navAutoCompleteFilename(argv.back(), cmd->flags);
                if (prefix.size() > argv.back().size()) argv.back() = prefix;

            } catch (...) { }
        }
    }
}

string
Console::navAutoCompleteFilename(const string &input, usize flags) const
{
    try {

        navRequireFormattedFS();

        auto path = FSPath(input);
        auto dir  = path.parentPath();

        // Find all matching items
        auto matches = amigaFs->match(input + "*");

        // Case 1: The completion was unique
        if (matches.size() == 1) {

            auto &node = amigaFs->fetch(matches[0]);
            auto name = dir / node.name();
            return name.cpp_str() + (node.isDirectory() ? "/" : "");
        }

        // Case 2: Multiple files match
        std::vector<string> names;
        for (auto &it : matches) {

            auto name = dir / amigaFs->fetch(it).name();
            names.push_back(name.cpp_str());
        }

        // Auto-complete all common characters
        return utl::commonPrefix(names, false);

    } catch (...) {

        return input;
    }
}

void
Console::navHelp(std::ostream &os, const string &argv, isize tabs)
{
    try {

        auto [cmd, args] = seekCommand(argv);

        // Determine the kind of help to display
        bool displayFiles = amigaFs && amigaFs->isFormatted() && cmd && cmd->callback && (cmd->flags & rs::ac);
        bool displayCmds  = true;

        if (displayCmds) {

            // Display the standard command help
            defaultHelp(os, argv, tabs);
        }

        if (displayFiles) {

            // Find matching items
            auto matches = amigaFs->match(args.empty() ? "*" : args.back() + "*");

            // Extract names
            vector<string> dirs, files;
            for (auto &it : matches) {

                auto &block = amigaFs->fetch(it);
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
Console::navParseBlock(const string &argv)
{
    if (auto nr = BlockNr(parseNum(argv)); amigaFs->tryFetch(nr)) {
        return nr;
    }

    throw CoreError(CoreError::OPT_INV_ARG, "0..." + std::to_string(amigaFs->blocks()));
}

BlockNr
Console::navParseBlock(const Arguments &argv, const string &token)
{
    return navParseBlock(argv, token, amigaFs->pwd());
}

BlockNr
Console::navParseBlock(const Arguments &argv, const string &token, BlockNr fallback)
{
    auto nr = argv.contains(token) ? BlockNr(parseNum(argv.at(token))) : fallback;

    if (!amigaFs->tryFetch(nr)) {
        throw CoreError(CoreError::OPT_INV_ARG, "0..." + std::to_string(amigaFs->blocks()));
    }
    return nr;
}

BlockNr
Console::navParsePath(const Arguments &argv, const string &token)
{
    amigaFs->require.isFormatted();

    assert(argv.contains(token));

    try {
        // Try to find the directory by name
        return amigaFs->seek(argv.at(token));

    } catch (...) {

        try {
            // Treat the argument as a block number
            return navParseBlock(argv.at(token));

        } catch (...) {

            // The item does not exist
            throw FSError(FSError::FS_NOT_FOUND, argv.at(token));
        }
    }
}

BlockNr
Console::navParsePath(const Arguments &argv, const string &token, BlockNr fallback)
{
    return argv.contains(token) ? navParsePath(argv, token) : fallback;
}

BlockNr
Console::navParseFile(const Arguments &argv, const string &token)
{
    return navParseFile(argv, token, amigaFs->pwd());
}

BlockNr
Console::navParseFile(const Arguments &argv, const string &token, BlockNr fallback)
{
    auto path = navParsePath(argv, token, fallback);
    amigaFs->require.file(path);

    return path;
}

BlockNr
Console::navParseDirectory(const Arguments &argv, const string &token)
{
    return navParseDirectory(argv, token, amigaFs->pwd());
}

BlockNr
Console::navParseDirectory(const Arguments &argv, const string &token, BlockNr fallback)
{
    auto path = navParsePath(argv, token, fallback);
    amigaFs->require.directory(path);

    return path;
}

void
Console::navImport(const FloppyDrive &dfn)
{
    // Later: Directly mount the file system on top of the drive

    // Create a block device
    adf = Codec::makeADF(dfn);

    // Create a file system on top
    amigaVol = make_unique<Volume>(*adf);
    amigaFs = make_unique<FileSystem>(*amigaVol);
}

void
Console::navImport(const HardDrive &hdn, isize part)
{
    throw FSError(FSError::FS_UNSUPPORTED);
    /*
     FileSystemFactory::initFromHardDrive(*amigaFs, hdn);
     */
}

void
Console::navImportDf(isize n)
{
    assert(n >= 0 && n <= 3);
    navImport(*amiga.df[n]);
}

void
Console::navImportHd(isize n, isize part)
{
    assert(n >= 0 && n <= 3);
    navImport(*amiga.hd[n], part);
}

void
Console::navImport(const fs::path &path, bool recursive, bool contents)
{
    amigaFs->importer.import(path, recursive, contents);
}

void
Console::navRequireFS() const
{
    if (!amigaFs) throw FSError(FSError::FS_CUSTOM, "No file system present");
}

void
Console::navRequireFormattedFS() const
{
    navRequireFS();
    amigaFs->require.isFormatted();
}

void
Console::navExportBlocks(fs::path path)
{
    amigaFs->exporter.exportVolume(path);
}

BlockNr
Console::navMatchPath(const Arguments &argv, const string &token, Tokens &notFound)
{
    return navMatchPath(argv.at(token), notFound);
}

BlockNr
Console::navMatchPath(const Arguments &argv, const string &token, Tokens &notFound, BlockNr fallback)
{
    return argv.contains(token) ? navMatchPath(argv, token, notFound) : fallback;
}

BlockNr
Console::navMatchPath(const string &path, Tokens &notFound)
{
    amigaFs->require.isFormatted();

    auto tokens = utl::split(path, '/');
    if (!path.empty() && path[0] == '/') { tokens.insert(tokens.begin(), "/"); }

    auto p = amigaFs->pwd();
    while (!tokens.empty()) {

        auto next = amigaFs->trySeek(tokens.front());
        if (!next) break;

        tokens.erase(tokens.begin());
        p = *next;
    }
    notFound = tokens;

    return p;
}

std::pair<DumpOpt,DumpFmt>
Console::navParseDumpOpts(const Arguments &argv)
{
    DumpOpt opt; DumpFmt fmt;

    auto a = argv.contains("a");
    auto o = argv.contains("o");
    auto d = argv.contains("d");
    auto w = argv.contains("w");
    auto l = argv.contains("l");
    auto size = l ? 'l' : w ? 'w' : 'b';
    auto columns = l ? 4 : w ? 8 : 16;

    if ((int)a + (int)o + (int)d > 1) {
        throw RSError(RSError::GENERIC, "Flags -a, -o, -d are mutually exclusive");
    }
    if ((int)a + (int)w + (int)l > 1) {
        throw RSError(RSError::GENERIC, "Flags -a, -w, -l are mutually exclusive");
    }
    if (o) {

        opt = { .base = 8 };
        fmt = { .size = size, .columns = columns, .offset = true, .ascii = true };

    } else if (d) {

        opt = { .base = 10 };
        fmt = { .size = size, .columns = columns, .offset = true, .ascii = true };

    } else if (a) {

        opt = { .base = 0 };
        fmt = { .size = 0, .columns = 64, .offset = true, .ascii = true };

    } else {

        opt = { .base = 16 };
        fmt = { .size = size, .columns = columns, .offset = true, .ascii = true };
    }

    return { opt, fmt };
}

void
Console::initNavigatorCommands(RSCommand &root)
{
    std::vector<string> help;
    
    //
    // Empty command
    //

    root.add({

        .tokens = { "return" },
        .chelp  = { "Print status information" },
        .flags  = rs::hidden,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            if (amigaFs) {

                amigaFs->dumpInfo(os);

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
            
            shell.enterCommander();
        }
    });
    
    root.add({
        
        .tokens = { ".." },
        .chelp  = { "Switch to the previous console" },
        .flags  = rs::hidden,
        
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            shell.enterDebugger();
        }
    });
    
    
    //
    // Creating
    //

    RSCommand::currentGroup = "Create";
    /*

    root.add({
        
        .tokens = { "create" },
        .chelp  = { "Create a file system with a particular capacity" },
    });
    
    root.add({
        
        .tokens = { "create", "SD" },
        .chelp  = { "Create a file system for a single-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            amigaFs->init(FSDescriptor(Diameter::INCH_525, Density::SD, FSFormat::NODOS));
            amigaFs->dumpInfo(os);
        }
    });
    
    root.add({
        
        .tokens = { "create", "DD" },
        .chelp  = { "Create a file system for a double-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            amigaFs->init(FSDescriptor(Diameter::INCH_35, Density::DD, FSFormat::NODOS));
            amigaFs->dumpInfo(os);
        }
    });
    
    root.add({
        
        .tokens = { "create", "HD" },
        .chelp  = { "Create a file system for a high-density floppy disk" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            amigaFs->init(FSDescriptor(Diameter::INCH_35, Density::HD, FSFormat::NODOS));
            amigaFs->dumpInfo(os);
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
                amigaFs = make_unique<FileSystem>(*dev, FSDescriptor(blocks, FSFormat::NODOS));
                amigaFs->dumpInfo(os);
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
                amigaFs = make_unique<FileSystem>(*dev, FSDescriptor(geometry, FSFormat::NODOS));
                amigaFs->dumpInfo(os);
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

                navRequireFS();

                // Determine the DOS type
                auto type = amiga::FSFormat::NODOS;
                auto dos = utl::uppercased(args.at("dos"));
                if (dos == "OFS") type = amiga::FSFormat::OFS;
                if (dos == "FFS") type = amiga::FSFormat::FFS;

                if (type == amiga::FSFormat::NODOS) {
                    throw RSError(RSError::GENERIC, "Expected values: OFS or FFS");
                }
                
                // Format the device
                auto name = args.contains("name") ? args.at("name") : "New Disk";
                amigaFs->format(type);
                amigaFs->setName(FSName(name));
                amigaFs->dumpInfo(os);
            }
    });

    root.add({

        .tokens = { "mount" },
        .ghelp  = { "Mounts an Amiga device" }
    });

    root.add({

        .tokens = { "mount", "df[n]" },
        .ghelp  = { "Mount floppy drive n" },
        .chelp  = { "mount { df0 | df1 | df1 | df2 }" },
        .flags  = vAmigaDOS ? rs::disabled : 0
    });

    for (isize i = 0; i < 4; i++) {

        root.add({

            .tokens = { "mount", "df" + std::to_string(i) },
            .chelp  = { "Mount floppy drive" + std::to_string(i) },
            .flags  = vAmigaDOS ? rs::disabled : rs::shadowed,
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                amigaVol = make_unique<Volume>(*df[values[0]]);
                amigaFs  = make_unique<FileSystem>(*amigaVol);

                amigaFs->dumpInfo(os);

            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "unmount" },
        .chelp  = { "Unmount the file system" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            navRequireFS();

            amigaFs->flush();
            amigaFs = nullptr;
        }
    });

    root.add({

        .tokens = { "flush" },
        .chelp  = { "Flush the file system cache" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            navRequireFS();

            amigaFs->flush();
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

                navRequireFormattedFS();

                auto path = args.at("path");
                auto hostPath = host.makeAbsolute(args.at("path"));
                bool recursive = true;
                bool contents = path.back() == '/';
                
                amigaFs->importer.import(amigaFs->pwd(), hostPath, recursive, contents);
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

                adf = Codec::makeADF(*df[n]);
                amigaVol = make_unique<Volume>(*adf);
                amigaFs  = make_unique<FileSystem>(*amigaVol);

                amigaFs->dumpInfo(os);

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
                amigaFs = FileSystemFactory::fromHardDrive(*dev, *hd[n]);
                amigaFs->dumpInfo(os);

            }, .payload = {i}
        });
    }
    */
    
    root.add({
        
        .tokens = { "import", "block" },
        .chelp  = { "Import a block from a file" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .args   = {
            { .name = { "path", "File path" } },
            { .name = { "nr", "Block number" }, .flags = rs::opt }
        },
            .func   = [&] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                navRequireFS();

                auto path = host.makeAbsolute(args.at("path"));
                auto nr = navParseBlock(args, "nr", amigaFs->pwd());

                amigaFs->importer.importBlock(nr, path);
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

                    navRequireFormattedFS();

                    bool recursive = args.contains("r");
                    std::filesystem::remove_all("/export");
                    
                    if (args.contains("file")) {
                        
                        auto item = navParsePath(args, "file");
                        // auto &item = amigaFs->fetch(itemNr);
                        auto name = amigaFs->fetch(item).cppName();
                        if (name.empty()) name = amigaFs->stat().name.cpp_str();
                        amigaFs->exporter.exportFiles(item, "/export", recursive, true);
                        msgQueue.setPayload( { "/export", name } );
                        
                    } else {
                        
                        amigaFs->exporter.exportVolume("/export");
                        auto name = amigaFs->stat().name.cpp_str();
                        name += amigaFs->getTraits().adf() ? ".adf" : ".hdf";
                        msgQueue.setPayload( { "/export", name } );
                    }
                    
                    msgQueue.put(Msg::RSH_EXPORT, shell.objid);
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

                    navRequireFormattedFS();

                    auto itemNr = navParsePath(args, "file");
                    bool recursive = args.contains("r");
                    bool contents = args.at("file").back() == '/';
                    
                    auto path = args.at("path");
                    auto hostPath = host.makeAbsolute(args.at("path"));
                    amigaFs->exporter.exportFiles(itemNr, hostPath, recursive, contents);
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

                navRequireFormattedFS();

                ADFFile adf(amigaFs->getTraits().blocks);
                amigaFs->exporter.exportVolume(adf);
                df[values[0]]->insertImage(adf, false);

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

                navRequireFormattedFS();

                auto n = values[0];
                hd[n]->init(*amigaFs);

            }, .payload = {i}
        });
    }
    
    root.add({
        
        .tokens = { "export", "block" },
        .chelp  = { "Export a block to a file" },
        .args   = {
            { .name = { "path", "File path" }, .flags = vAmigaDOS ? rs::disabled : 0 },
            { .name = { "nr", "Block number" }, .flags = rs::opt }
        },
            .func   = [&] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                navRequireFormattedFS();

                auto nr = navParseBlock(args, "nr", amigaFs->pwd());

                if constexpr (vAmigaDOS) {
                    
                    amigaFs->exporter.exportBlock(nr, "blob");
                    msgQueue.setPayload( { "blob", std::to_string(nr) + ".bin" } );
                    msgQueue.put(Msg::RSH_EXPORT, shell.objid);
                    
                } else {
                    
                    auto path = host.makeAbsolute(args.at("path"));
                    amigaFs->exporter.exportBlock(nr, path);
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

                navRequireFormattedFS();

                auto path = navParsePath(args, "path", amigaFs->root());
                amigaFs->cd(path);
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

                navRequireFormattedFS();

                auto path = navParseDirectory(args, "path");
                auto d = args.contains("d");
                auto f = args.contains("f");
                auto r = args.contains("r");

                // Collect the directories to print
                FSTree tree = amigaFs->build(path, {
                    .accept = accept::directories,
                    .sort   = sort::alpha,
                    .depth  = r ? MAX_ISIZE : 0
                });

                // For each directory...
                for (const auto &node : tree.dfs()) {

                    // Print header
                    if (node.nr != tree.nr) os << "\n";
                    os << "Directory " << amigaFs->fetch(node.nr).absName() << ":\n\n";

                    if (!f) {

                        // Collect directory items
                        FSTree items = amigaFs->build(node.nr, {
                            .accept = accept::directories,
                            .sort   = sort::alpha,
                            .depth  = 1
                        });

                        // Extract names
                        vector<string> names;
                        for (const auto &child : items.children) {
                            names.push_back(amigaFs->fetch(child.nr).cppName() + " (dir)");
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
                        FSTree items = amigaFs->build(node.nr, {
                            .accept = accept::files,
                            .sort   = sort::alpha,
                            .depth  = 1
                        });

                        // Extract names
                        vector<string> names;
                        for (const auto &child : items.children) {
                            names.push_back(amigaFs->fetch(child.nr).cppName());
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

                navRequireFormattedFS();

                auto path = navParseDirectory(args, "path");
                auto d = args.contains("d");
                auto f = args.contains("f");
                auto r = args.contains("r");
                auto k = args.contains("k");
                auto s = args.contains("s");

                // Formats the output for a single item
                auto formatted = [&](BlockNr nr) {

                    auto &node = amigaFs->fetch(nr);

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
                FSTree tree = amigaFs->build(path, {
                    .accept = accept::directories,
                    .sort   = s ? sort::alpha : sort::none,
                    .depth  = r ? MAX_ISIZE : 0
                });

                // For each directory...
                for (const auto &node : tree.dfs()) {

                    // Print header
                    if (node.nr != tree.nr) os << "\n";
                    os << "Directory " << amigaFs->fetch(node.nr).absName() << ":\n\n";

                    // Collect items
                    FSTree items = amigaFs->build(node.nr, {
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

                navRequireFormattedFS();

                auto pattern = FSPattern(args.at("name"));
                auto d = args.contains("d");
                auto f = args.contains("f");
                auto s = args.contains("s");

                // Determine the start node
                auto start = amigaFs->pwd();

                // Build a directory tree
                FSTree tree = amigaFs->build(start, {
                    .accept = accept::all,
                    .sort   = sort::none,
                    .depth  = MAX_ISIZE
                });

                // Traverse the tree and find matches
                vector<const FSBlock *> matching;
                for (const auto &node : tree.bfs()) {

                    auto &block = amigaFs->fetch(node.nr);

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

                navRequireFormattedFS();

                // Find matches
                vector <BlockNr> matches = amigaFs->match(args.at("name"));

                // Print the result
                for (auto &it : matches) { os << amigaFs->fetch(it).absName() << '\n'; }
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

                navRequireFS();

                if (args.contains("b")) {
                    amigaFs->dumpBlocks(os);
                } else {
                    amigaFs->dumpInfo(os);
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

                navRequireFormattedFS();

                auto &file = amigaFs->fetch(navParseFile(args, "path"));
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

                navRequireFormattedFS();

                auto nr = navParseBlock(args, "nr");
                amigaFs->doctor.dump(nr, os);
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
                
                navRequireFormattedFS();

                amigaFs->makeBootable(BootBlockId(values[0]));

            },  .payload = { value }
        });
    }
    
    root.add({
        
        .tokens = { "boot", "scan" },
        .chelp  = { "Scan a boot block for viruses" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            navRequireFormattedFS();

            os << "Boot block: " << amigaFs->bootStat().name << std::endl;
        }
    });
    
    root.add({
        
        .tokens = { "boot", "kill" },
        .chelp  = { "Kills a boot block virus" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            navRequireFormattedFS();

            amigaFs->killVirus();
        }
    });
    
    root.add({
        
        .tokens = { "type" },
        .chelp  = { "Print the contents of a file" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "path", "File path" }, .flags = rs::opt }
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                navRequireFormattedFS();

                auto &file = amigaFs->fetch(navParsePath(args, "path", amigaFs->pwd()));
                if (!file.isFile()) {
                    throw FSError(FSError::FS_NOT_A_FILE, "Block " + std::to_string(file.nr));
                }

                Buffer<u8> buffer;
                file.extractData(buffer);
                buffer.txtDump(os);
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

                navRequireFormattedFS();

                auto &file = amigaFs->fetch(navParseFile(args, "path", amigaFs->pwd()));
                auto opt   = navParseDumpOpts(args);
                auto lines = args.contains("lines") ? parseNum(args.at("lines")) : LONG_MAX;
                auto t     = args.contains("t");

                Buffer<u8> buffer;
                file.extractData(buffer);

                std::stringstream ss;
                buffer.dump(ss, opt.first, opt.second);

                t ? tail(ss, os, lines) : head(ss, os, lines);
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

                navRequireFormattedFS();

                auto nr    = navParseBlock(args, "nr", amigaFs->pwd());
                auto opt   = navParseDumpOpts(args);
                auto lines = args.contains("lines") ? parseNum(args.at("lines")) : LONG_MAX;
                auto t     = args.contains("t");

                std::stringstream ss;
                amigaFs->fetch(nr).dump(ss, opt.first, opt.second);

                t ? tail(ss, os, lines) : head(ss, os, lines);
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

                navRequireFormattedFS();

                bool strict = args.contains("s");
                
                if (args.contains("nr")) {
                    
                    auto nr = navParseBlock(args, "nr");
                    
                    if (args.contains("r")) amigaFs->doctor.rectify(nr, strict);
                    if (auto errors = amigaFs->doctor.xray(nr, strict, os); !errors) {
                        os << "No findings." << std::endl;
                    }
                    
                } else {
                    
                    if (args.contains("r")) amigaFs->doctor.rectify(strict);
                    if (auto errors = amigaFs->doctor.xray(strict, os, args.contains("v")); !errors) {
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

                navRequireFormattedFS();

                Tokens missing;
                auto path = navMatchPath(args.at("name"), missing);
                
                if (missing.empty()) {
                    throw(FSError(FSError::FS_EXISTS, args.at("name")));
                }

                auto p = path;
                for (auto &it: missing) {
                    p = amigaFs->mkdir(p, FSName(it));
                }
                
                /*
                auto *p = &path.mutate();
                for (auto &it: missing) {
                    if (p) p = &amigaFs->mkdir(*p, FSName(it));
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

                navRequireFormattedFS();

                auto sourceNr = navParsePath(args, "source");
                auto &source = amigaFs->fetch(sourceNr);

                Tokens missing;
                auto pathNr = navMatchPath(args.at("target"), missing);
                auto &path = amigaFs->fetch(pathNr);

                printf("%s -> '%s' {", source.absName().c_str(), path.absName().c_str());
                for (auto &it : missing) printf(" %s", it.c_str());
                printf(" }\n");
                
                if (missing.empty()) {
                    
                    if (path.isFile()) {
                        
                        throw FSError(FSError::FS_EXISTS, args.at("target"));
                    }
                    if (path.isDirectory()) {
                        
                        logmsg(LOG_RSH, "Moving '%s' to '%s'\n", source.absName().c_str(), path.absName().c_str());
                        amigaFs->move(sourceNr, pathNr);
                    }
                    
                } else if (missing.size() == 1) {
                    
                    logmsg(LOG_RSH, "Moving '%s' to '%s' / '%s'\n",
                          source.absName().c_str(), path.absName().c_str(), missing.back().c_str());
                    amigaFs->move(sourceNr, pathNr, FSName(missing.back()));

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

                navRequireFormattedFS();

                auto sourceNr = navParsePath(args, "source");

                Tokens missing;
                auto pathNr = navMatchPath(args.at("target"), missing);
                auto &path = amigaFs->fetch(pathNr);

                if (missing.empty()) {
                    
                    if (path.isFile()) {
                        
                        throw FSError(FSError::FS_EXISTS, args.at("target"));
                    }
                    if (path.isDirectory()) {
                        
                        amigaFs->copy(sourceNr, pathNr);
                    }
                    
                } else if (missing.size() == 1) {
                    
                    amigaFs->copy(sourceNr, pathNr, FSName(missing.back()));

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

                navRequireFormattedFS();

                auto &path = amigaFs->fetch(navParsePath(args, "path"));

                if (path.isFile()) {
                    amigaFs->rm(path.nr);
                } else if (path.isDirectory()) {
                    throw FSError(FSError::FS_NOT_A_FILE, args.at("path"));
                } else {
                    throw FSError(FSError::FS_INVALID_PATH, args.at("path"));
                }
                
            }
    });
}

}

