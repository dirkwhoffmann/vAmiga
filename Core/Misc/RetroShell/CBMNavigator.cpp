// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "Console.h"
#include "RSError.h"
#include "Emulator.h"
#include "Codecs.h"
#include "HDFFile.h"
#include "utl/chrono.h"
#include "utl/support.h"
#include "utl/io.h"
#include <climits>
#include <format>
#include <regex>

namespace vamiga {

using retro::vault::FSError;
using retro::vault::cbm::FSFormat;
using retro::vault::cbm::FSPattern;
using retro::vault::cbm::PETName;

string
Console::cbmPrompt()
{
    std::stringstream ss;

    if (cbmFs) {

        ss << "[" << std::to_string(cb);

        if (auto ts = cbmFs->getTraits().tsLink(cb))
            ss << " (" << std::to_string(ts->t) << ":" << std::to_string(ts->s) << ")";

        ss << "]";

        auto fsName = cbmFs->stat().name;
        if (!fsName.empty()) ss << " " << fsName << ":";
    }

    ss << "> ";

    return ss.str();
}

void
Console::cbmAutoComplete(Tokens &argv)
{
    // Only proceed if there is anything to complete
    if (argv.empty()) return;

    if (auto [cmd, remaining] = seekCommand(argv); remaining.size() > 0) {

        // First, try to auto-complete the last token with a command name
        if (remaining.size() != 1 || !cmd->autoComplete(argv.back())) {

            // If that didn't work, try to auto-complete with a file name
            try {

                auto prefix = cbmAutoCompleteFilename(argv.back(), cmd->flags);
                if (prefix.size() > argv.back().size()) argv.back() = prefix;

            } catch (...) { }
        }
    }
}

string
Console::cbmAutoCompleteFilename(const string &input, usize flags) const
{
    try {

        cbmRequireFormattedFS();

        // Find matching items
        auto matches = cbmFs->searchDir(FSPattern(input + "*"));

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
Console::cbmHelp(std::ostream &os, const string &argv, isize tabs)
{
    try {

        auto [cmd, args] = seekCommand(argv);

        // Determine the kind of help to display
        bool displayFiles = cbmFs && cbmFs->isFormatted() && cmd && cmd->callback && (cmd->flags & rs::ac);
        bool displayCmds  = true;

        if (displayCmds) {

            // Display the standard command help
            defaultHelp(os, argv, tabs);
        }

        if (displayFiles) {

            // Find matching items
            auto pattern = FSPattern(args.empty() ? "*" : args.back() + "*");
            auto matches = cbmFs->searchDir(pattern);

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
Console::cbmParseBlock(const std::string &argv)
{
    if (auto pos = argv.find(':'); pos != std::string::npos) {

        // TS syntax (track:sector)
        auto lhs = argv.substr(0, pos);
        auto rhs = argv.substr(pos + 1);

        if (lhs.empty() || rhs.empty())
            throw CoreError(CoreError::OPT_INV_ARG, argv);

        auto t = parseNum(lhs);
        auto s = parseNum(rhs);

        if (auto nr = cbmFs->getTraits().blockNr(TSLink{t,s}))
            return *nr;

    } else {

        // Block syntax (single number)
        BlockNr nr = parseNum(argv);

        if (cbmFs->tryFetch(nr))
            return nr;
    }

    throw CoreError(CoreError::OPT_INV_ARG,
                    "0..." + std::to_string(cbmFs->blocks()));
}

BlockNr
Console::cbmParseBlock(const Arguments &argv, const string &token, BlockNr fallback)
{
    auto nr = argv.contains(token) ? BlockNr(cbmParseBlock(argv.at(token))) : fallback;

    if (!cbmFs->tryFetch(nr)) {
        throw CoreError(CoreError::OPT_INV_ARG, "0..." + std::to_string(cbmFs->blocks()));
    }
    return nr;
}

BlockNr
Console::cbmParseBlock(const Arguments &argv, const string &token)
{
    return cbmParseBlock(argv, token, cb);
}

BlockNr
Console::cbmParseFile(const string &arg)
{
    return cbmFs->seek(arg);
}

BlockNr
Console::cbmParseFile(const Arguments &argv, const string &token)
{
    assert(argv.contains(token));
    return cbmParseFile(argv.at(token));
}

BlockNr
Console::cbmParseFile(const Arguments &argv, const string &token, BlockNr fallback)
{
    return argv.contains(token) ? cbmParseFile(argv, token) : fallback;
}

BlockNr
Console::cbmParseFileOrBlock(const string &arg)
{
    try { return cbmParseFile(arg); } catch (...) { return cbmParseBlock(arg); }
}

BlockNr
Console::cbmParseFileOrBlock(const Arguments &argv, const string &token)
{
    assert(argv.contains(token));
    return cbmParseFileOrBlock(argv.at(token));
}

BlockNr
Console::cbmParseFileOrBlock(const Arguments &argv, const string &token, BlockNr fallback)
{
    return argv.contains(token) ? cbmParseFileOrBlock(argv, token) : fallback;
}

void
Console::cbmImport(const fs::path &path, bool recursive, bool contents)
{
    cbmFs->importer.import(path);
}

void
Console::cbmRequireFS() const
{
    if (!cbmFs) throw FSError(FSError::FS_CUSTOM, "No file system present");
}

void
Console::cbmRequireFormattedFS() const
{
    cbmRequireFS();
    cbmFs->require.isFormatted();
}

void
Console::cbmExportBlocks(fs::path path)
{
    cbmFs->exporter.exportVolume(path);
}

std::pair<DumpOpt,DumpFmt>
Console::cbmParseDumpOpts(const Arguments &argv)
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
Console::initCBMNavigatorCommands(RSCommand &root)
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

            if (cbmFs) {

                cbmFs->dumpStatfs(os);

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

    root.add({

        .tokens = { "format" },
        .chelp  = { "Format the file system" },
        .args   = {
            { .name = { "dos", "File system type" }, .key = "{ CBM }" },
            { .name = { "name", "File system name" }, .flags = rs::opt },
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                cbmRequireFS();

                // Determine the DOS type
                auto type = FSFormat::NODOS;
                auto dos = utl::uppercased(args.at("dos"));
                if (dos == "CBM") type = FSFormat::CBM;

                if (type == FSFormat::NODOS)
                    throw RSError(RSError::GENERIC, "Expected values: CBM");

                // Format the device
                auto name = args.contains("name") ? args.at("name") : "New Disk";
                cbmFs->format(type);
                cbmFs->setName(PETName<16>(name));
                cbmFs->dumpStatfs(os);
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

                cbmVol = make_unique<Volume>(*df[values[0]]);
                cbmFs  = make_unique<retro::vault::cbm::FileSystem>(*cbmVol);

                cbmFs->dumpInfo(os);

            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "unmount" },
        .chelp  = { "Unmount the file system" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            cbmRequireFS();

            cbmFs->flush();
            cbmFs = nullptr;
        }
    });
    */

    root.add({

        .tokens = { "flush" },
        .chelp  = { "Flush the file system cache" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            cbmRequireFS();
            cbmFs->flush();
        }
    });

    root.add({

        .tokens = { "import" },
        .ghelp  = { "Import files" },
        .chelp  = { "Import a file or a folder from the host file system" },
        .flags  = vAmigaDOS ? rs::hidden : 0,
        .args   = {
            { .name = { "path", "Host file system directory" } },
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                cbmRequireFormattedFS();

                auto path = args.at("path");
                auto hostPath = host.makeAbsolute(args.at("path"));

                cbmFs->importer.import(hostPath);
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
                cbmVol = make_unique<Volume>(*d64);
                cbmFs  = make_unique<retro::vault::cbm::FileSystem>(*cbmVol);

                // Select the BAM as current working block
                cb = cbmFs->bam();

                cbmFs->dumpStatfs(os);

            }, .payload = {i}
        });
    }

    root.add({

        .tokens = { "import", "block" },
        .chelp  = { "Import a block from a file" },
        .flags  = vAmigaDOS ? rs::disabled : 0,
        .args   = {
            { .name = { "path", "File path" } },
            { .name = { "nr", "Block number" }, .flags = rs::opt }
        },
            .func   = [&] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                cbmRequireFS();

                auto path = host.makeAbsolute(args.at("path"));
                auto nr = cbmParseBlock(args, "nr", cb);

                cbmFs->importer.importBlock(nr, path);
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

                    cbmRequireFormattedFS();

                    throw FSError(FSError::FS_UNSUPPORTED);
                }
        });

    } else {

        root.add({

            .tokens = { "export" },
            .ghelp  = { "Export files or blocks" },
            .chelp  = { "Export a file or directory to the host file system" },
            .flags  = rs::ac,
            .args   = {
                { .name = { "pattern", "File name pattern" } },
                { .name = { "path", "Host file system location" } }
            },
                .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                    cbmRequireFormattedFS();

                    auto path = args.at("path");
                    auto hostPath = host.makeAbsolute(args.at("path"));

                    auto pattern = FSPattern(args.at("pattern"));
                    cbmFs->exporter.exportFiles(pattern, hostPath);
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

                cbmRequireFormattedFS();

                ADFFile adf(cbmFs->getTraits().blocks);
                cbmFs->exporter.exportVolume(adf);
                df[values[0]]->insertImage(adf, false);

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

                cbmRequireFormattedFS();

                auto nr = cbmParseBlock(args, "nr", cb);

                if constexpr (vAmigaDOS) {

                    cbmFs->exporter.exportBlock(nr, "blob");
                    msgQueue.setPayload( { "blob", std::to_string(nr) + ".bin" } );
                    msgQueue.put(Msg::RSH_EXPORT, shell.objid);

                } else {

                    auto path = host.makeAbsolute(args.at("path"));
                    cbmFs->exporter.exportBlock(nr, path);
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
        .args   = { },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            cbmRequireFormattedFS();

            // Read directory
            auto dir = cbmFs->readDir();

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
                    cbmFs->dumpBlocks(os);
                } else {
                    cbmFs->dumpStatfs(os);
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

                cbmRequireFormattedFS();

                auto block = cbmParseFileOrBlock(args, "file", cb);
                cbmFs->doctor.dump(block, os);
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

                cbmRequireFormattedFS();

                auto &file = cbmFs->fetch(cbmParseFile(args, "path"));
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

                cbmRequireFormattedFS();

                auto nr    = cbmParseBlock(args, "nr", cb);
                auto opt   = cbmParseDumpOpts(args);
                auto lines = args.contains("lines") ? parseNum(args.at("lines")) : LONG_MAX;
                auto t     = args.contains("t");

                std::stringstream ss;
                cbmFs->fetch(nr).dump(ss, opt.first, opt.second);

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

                cbmRequireFormattedFS();

                auto nr = cbmParseBlock(args, "nr");
                cbmFs->doctor.dump(nr, os);
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

                cbmRequireFormattedFS();
                auto file = cbmParseFile(args, "file");

                Buffer<u8> buffer;
                cbmFs->extractData(file, buffer);
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

                cbmRequireFormattedFS();
                auto file  = cbmParseFile(args, "file");
                auto opt   = cbmParseDumpOpts(args);
                auto lines = args.contains("lines") ? parseNum(args.at("lines")) : LONG_MAX;
                auto t     = args.contains("t");

                Buffer<u8> buffer;
                cbmFs->extractData(file, buffer);
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

                cbmRequireFormattedFS();

                bool strict = args.contains("s");

                if (args.contains("nr")) {

                    auto nr = cbmParseBlock(args, "nr");

                    if (args.contains("r")) cbmFs->doctor.rectify(nr, strict);
                    if (auto errors = cbmFs->doctor.xray(nr, strict, os); !errors) {
                        os << "No findings." << std::endl;
                    }

                } else {

                    if (args.contains("r")) cbmFs->doctor.rectify(strict);
                    if (auto errors = cbmFs->doctor.xray(strict, os, args.contains("v")); !errors) {
                        os << "No findings." << std::endl;
                    }
                }
            }
    });

    root.add({

        .tokens = { "xray", "bitmap" },
        .chelp  = { "Inspects the block allocation map" },
        .args   = {
            { .name = { "r", "Rectify errors" }, .flags = rs::flag }
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                cbmRequireFormattedFS();

                if (args.contains("r")) cbmFs->doctor.rectifyBitmap();
                if (auto errors = cbmFs->doctor.xrayBitmap(os); !errors) {
                    os << "No findings." << std::endl;
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

                cbmRequireFormattedFS();

                cb = cbmParseFileOrBlock(args, "file");
            }
    });

    root.add({

        .tokens = { "next" },
        .chelp  = { "Take the TS link to the next block" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            cbmRequireFormattedFS();

            auto ts = cbmFs->fetch(cb).tsLink();
            if (auto b = cbmFs->getTraits().blockNr(ts)) {
                cb = *b;
            }
        }
    });


    //
    // Modifying
    //

    RSCommand::currentGroup = "Modify";

    root.add({

        .tokens = { "rename" },
        .chelp  = { "Renames a file" },
        .flags  = rs::ac,
        .args   = {
            { .name = { "source", "Old file name" } },
            { .name = { "target", "New file name" } },
        },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                cbmRequireFormattedFS();

                auto oldName = PETName<16>(args.at("source"));
                auto newName = PETName<16>(args.at("target"));

                // Make sure the source file exists
                if (!cbmFs->trySeek(oldName))
                    throw FSError(FSError::FS_NOT_FOUND, oldName.str());

                // Make sure the target file does not exist
                if (cbmFs->trySeek(newName))
                    throw FSError(FSError::FS_EXISTS, newName.str());

                cbmFs->rename(oldName, newName);
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

                cbmRequireFormattedFS();

                auto &path = cbmFs->fetch(cbmParseFile(args, "path"));
                cbmFs->rm(path.nr);
            }
    });
}

}

