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
#include <format>
#include <regex>

namespace vamiga {

using retro::vault::cbm::FSError;
using retro::vault::cbm::FSFormat;
using retro::vault::cbm::FSPattern;
using retro::vault::cbm::PETName;

void
CBMNavigator::_pause()
{

}

string
CBMNavigator::prompt()
{
    std::stringstream ss;

    if (fs) {

        ss << "[" << std::to_string(cb);

        if (auto ts = fs->getTraits().tsLink(cb))
            ss << " (" << std::to_string(ts->t) << ":" << std::to_string(ts->s) << ")";

        ss << "]";

        auto fsName = fs->stat().name;
        if (!fsName.empty()) ss << " " << fsName << ":";
    }

    ss << "> ";

    return ss.str();
}

void
CBMNavigator::didActivate()
{

}

void
CBMNavigator::didDeactivate()
{

}

void
CBMNavigator::autoComplete(Tokens &argv)
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
CBMNavigator::autoCompleteFilename(const string &input, usize flags) const
{
    try {

        requireFormattedFS();

        // Find matching items
        auto matches = fs->searchDir(FSPattern(input + "*"));

        if (!matches.empty()) {

            // Extract names
            std::vector<string> names;
            for (auto &it : matches) names.push_back(it.getName().str());

            // Auto-complete all common characters
            return utl::commonPrefix(names, false);
        }

    } catch (...) { }

    return input;
}

void
CBMNavigator::help(std::ostream &os, const string &argv, isize tabs)
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
            auto pattern = FSPattern(args.empty() ? "*" : args.back() + "*");
            auto matches = fs->searchDir(pattern);

            // Extract names
            std::vector<string> names;
            for (auto &it : matches) names.push_back(it.getName().str());

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
            std::sort(names.begin(), names.end(), ciLess);

            if (!matches.empty()) {

                os << std::endl;
                Formatter::printTable(os, names, {
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
CBMNavigator::parseBlock(const std::string &argv)
{
    if (auto pos = argv.find(':'); pos != std::string::npos) {

        // TS syntax (track:sector)
        auto lhs = argv.substr(0, pos);
        auto rhs = argv.substr(pos + 1);

        if (lhs.empty() || rhs.empty()) {
            throw CoreError(CoreError::OPT_INV_ARG, argv);
        }

        auto t = parseNum(lhs);
        auto s = parseNum(rhs);

        if (auto nr = fs->getTraits().blockNr(TSLink{t,s})) {
            printf("t: %ld s: %ld nr: %ld\n", t, s, *nr);
            return *nr;
        }

    } else {

        // Block syntax (single number)
        BlockNr nr = parseNum(argv);

        if (fs->tryFetch(nr)) {
            return nr;
        }
    }

    throw CoreError(CoreError::OPT_INV_ARG,
                    "0..." + std::to_string(fs->blocks()));
}

BlockNr
CBMNavigator::parseBlock(const Arguments &argv, const string &token, BlockNr fallback)
{
    auto nr = argv.contains(token) ? BlockNr(parseBlock(argv.at(token))) : fallback;

    if (!fs->tryFetch(nr)) {
        throw CoreError(CoreError::OPT_INV_ARG, "0..." + std::to_string(fs->blocks()));
    }
    return nr;
}

BlockNr
CBMNavigator::parseBlock(const Arguments &argv, const string &token)
{
    return parseBlock(argv, token, cb);
}

BlockNr
CBMNavigator::parseFile(const string &arg)
{
    return fs->seek(arg);
}

BlockNr
CBMNavigator::parseFile(const Arguments &argv, const string &token)
{
    assert(argv.contains(token));
    return parseFile(argv.at(token));
}

BlockNr
CBMNavigator::parseFile(const Arguments &argv, const string &token, BlockNr fallback)
{
    return argv.contains(token) ? parseFile(argv, token) : fallback;
}

BlockNr
CBMNavigator::parseFileOrBlock(const string &arg)
{
    try { return parseFile(arg); } catch (...) { return parseBlock(arg); }
}

BlockNr
CBMNavigator::parseFileOrBlock(const Arguments &argv, const string &token)
{
    assert(argv.contains(token));
    return parseFileOrBlock(argv.at(token));
}

BlockNr
CBMNavigator::parseFileOrBlock(const Arguments &argv, const string &token, BlockNr fallback)
{
    return argv.contains(token) ? parseFileOrBlock(argv, token) : fallback;
}

void
CBMNavigator::import(const fs::path &path, bool recursive, bool contents)
{
    fs->importer.import(path);
}

void
CBMNavigator::requireFS() const
{
    if (!fs) throw FSError(FSError::FS_UNKNOWN, "No file system present");
}

void
CBMNavigator::requireFormattedFS() const
{
    requireFS();
    fs->require.isFormatted();
}

void
CBMNavigator::exportBlocks(fs::path path)
{
    fs->exporter.exportVolume(path);
}

/*
BlockNr
CBMNavigator::matchPath(const Arguments &argv, const string &token, Tokens &notFound)
{
    return matchPath(argv.at(token), notFound);
}

BlockNr
CBMNavigator::matchPath(const Arguments &argv, const string &token, Tokens &notFound, BlockNr fallback)
{
    return argv.contains(token) ? matchPath(argv, token, notFound) : fallback;
}

BlockNr
CBMNavigator::matchPath(const string &path, Tokens &notFound)
{
    fs->require.isFormatted();

    auto tokens = utl::split(path, '/');
    if (!path.empty() && path[0] == '/') { tokens.insert(tokens.begin(), "/"); }

    auto p = fs->bam();
    while (!tokens.empty()) {

        auto next = fs->trySeek(tokens.front());
        if (!next) break;

        tokens.erase(tokens.begin());
        p = *next;
    }
    notFound = tokens;

    return p;
}
*/

std::pair<DumpOpt,DumpFmt>
CBMNavigator::parseDumpOpts(const Arguments &argv)
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
CBMNavigator::initCommands(RSCommand &root)
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

                fs->dumpStatfs(os);

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
    // Creating
    //

    RSCommand::currentGroup = "Create";

    root.add({

        .tokens = { "format" },
        .chelp  = { "Format the file system" },
        .args   = {
            { .name = { "dos", "File system type" }, .key = "{ CBM }" },
            { .name = { "name", "File system name" }, .flags = rs::opt },
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                requireFS();

                // Determine the DOS type
                auto type = FSFormat::NODOS;
                auto dos = utl::uppercased(args.at("dos"));
                if (dos == "CBM") type = FSFormat::CBM;

                if (type == FSFormat::NODOS)
                    throw RSError(RSError::GENERIC, "Expected values: CBM");

                // Format the device
                auto name = args.contains("name") ? args.at("name") : "New Disk";
                fs->format(type);
                fs->setName(PETName<16>(name));
                fs->dumpStatfs(os);
            }
    });

    /*
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

                vol = make_unique<Volume>(*df[values[0]]);
                fs  = make_unique<retro::vault::cbm::FileSystem>(*vol);

                fs->dumpInfo(os);

            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "unmount" },
        .chelp  = { "Unmount the file system" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            requireFS();

            fs->flush();
            fs = nullptr;
        }
    });
    */

    root.add({

        .tokens = { "flush" },
        .chelp  = { "Flush the file system cache" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            requireFS();
            fs->flush();
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

                fs->importer.import(hostPath);
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

                d64 = Codec::makeD64(*df[values[0]]);
                vol = make_unique<Volume>(*d64);
                fs  = make_unique<retro::vault::cbm::FileSystem>(*vol);

                // Select the BAM as current working block
                cb = fs->bam();

                fs->dumpStatfs(os);

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

                requireFS();

                auto path = host.makeAbsolute(args.at("path"));
                auto nr = parseBlock(args, "nr", fs->bam());

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

                    throw FSError(FSError::FS_UNSUPPORTED);
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

                    auto itemNr = parseFile(args, "file");
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

                ADFFile adf(fs->getTraits().blocks);
                fs->exporter.exportVolume(adf);
                df[values[0]]->insertImage(adf, false);

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

                auto nr = parseBlock(args, "nr", fs->bam());

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

    //
    // Inspecting
    //

    RSCommand::currentGroup = "Inspect";

    root.add({

        .tokens = { "dir" },
        .chelp  = { "Display a sorted list of the files in a directory" },
        .flags  = rs::acdir,
        .args   = { },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            requireFormattedFS();

            // Read directory
            auto dir = fs->readDir();

            // Print items
            std::stringstream ss;

            for (auto &it : dir) {

                auto name = "\"" + it.getName().str() + "\"";
                auto size = it.getFileSize();
                auto type = it.typeString();

                if (!it.empty())
                    ss << std::format("{:<5} {:<16} {}\n", size, name, type);
            }

            os << ss.str();
        }
    });

    root.add({

        .tokens = { "statfs" },
        .chelp  = { "Print a file system summary" },
        .args   = {
            { .name = { "b", "Inspect the block storage" }, .flags = rs::flag },
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                if (args.contains("b")) {
                    fs->dumpBlocks(os);
                } else {
                    fs->dumpStatfs(os);
                }

            }
    });

    root.add({

        .tokens = { "stat" },
        .chelp  = { "Inform about a file or block" },
        .args   = {
            { .name = { "file", "File name or block number" }, .flags = rs::opt }
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                requireFormattedFS();

                auto block = parseFileOrBlock(args, "file", cb);
                fs->doctor.dump(block, os);
            }
    });

    /*
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
    */

    root.add({

        .tokens = { "block" },
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

                auto nr    = parseBlock(args, "nr", cb);
                auto opt   = parseDumpOpts(args);
                auto lines = args.contains("lines") ? parseNum(args.at("lines")) : LONG_MAX;
                auto t     = args.contains("t");

                std::stringstream ss;
                fs->fetch(nr).dump(ss, opt.first, opt.second);

                t ? tail(ss, os, lines) : head(ss, os, lines);
            }
    });

    /*
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
    */

    root.add({

        .tokens = { "type" },
        .chelp  = { "Print the contents of a file" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "file", "File name" } }
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                requireFormattedFS();
                auto file = parseFile(args, "file");

                Buffer<u8> buffer;
                fs->extractData(file, buffer);
                buffer.txtDump(os);
            }
    });

    root.add({

        .tokens = { "dump" },
        .chelp  = { "Dump the contents of a file" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "file", "File name" } },
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
                auto file  = parseFile(args, "file");
                auto opt   = parseDumpOpts(args);
                auto lines = args.contains("lines") ? parseNum(args.at("lines")) : LONG_MAX;
                auto t     = args.contains("t");

                Buffer<u8> buffer;
                fs->extractData(file, buffer);
                std::stringstream ss;
                buffer.dump(ss, opt.first, opt.second);
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

    //
    // Navigating
    //

    RSCommand::currentGroup = "Navigate";

    root.add({

        .tokens = { "select" },
        .chelp  = { "Selects the current working block" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "file", "File name or block number" } }
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                requireFormattedFS();

                cb = parseFileOrBlock(args, "file");
            }
    });

    root.add({

        .tokens = { "next" },
        .chelp  = { "Take the TS link to the next block" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            requireFormattedFS();
            if (auto ts = fs->getTraits().tsLink(cb)) {
                if (auto b = fs->getTraits().blockNr(*ts)) {
                    cb = *b;
                }
            }
        }
    });


    //
    // Modifying
    //

    RSCommand::currentGroup = "Modify";

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

                /*
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

                        loginfo(RSH_DEBUG, "Moving '%s' to '%s'\n", source.absName().c_str(), path.absName().c_str());
                        fs->move(sourceNr, pathNr);
                    }

                } else if (missing.size() == 1) {

                    loginfo(RSH_DEBUG, "Moving '%s' to '%s' / '%s'\n",
                          source.absName().c_str(), path.absName().c_str(), missing.back().c_str());
                    fs->move(sourceNr, pathNr, FSName(missing.back()));

                } else {

                    throw FSError(FSError::FS_NOT_FOUND, missing.front());
                }
                */
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

                /*
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
                */
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

                auto &path = fs->fetch(parseFile(args, "path"));
                fs->rm(path.nr);
            }
    });
}

}

