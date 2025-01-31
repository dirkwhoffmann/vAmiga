// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include <functional>
#include <vector>
#include <stack>

namespace vamiga {

class RetroShell;

typedef std::vector<string> Arguments;

namespace Arg {

static const std::string address    = "<address>";
static const std::string boolean    = "{ true | false }";
static const std::string command    = "<command>";
static const std::string count      = "<count>";
static const std::string dst        = "<destination>";
static const std::string ignores    = "<ignores>";
static const std::string kb         = "<kb>";
static const std::string nr         = "<nr>";
static const std::string onoff      = "{ on | off }";
static const std::string path       = "<path>";
static const std::string process    = "<process>";
static const std::string seconds    = "<seconds>";
static const std::string value      = "<value>";
static const std::string sequence   = "<byte sequence>";
static const std::string src        = "<source>";
static const std::string volume     = "<volume>";
static const std::string string     = "<string>";

};

struct RetroShellCmdDescriptor {
    
    const std::vector<string> &tokens = {};
    bool hidden = false;
    const std::vector<string> &requiredArgs = {};
    const std::vector<string> &optionalArgs = {};
    const string &helpName = "";
    const string help = "";
    std::function<void (Arguments&, long)> func = nullptr;
    long value = 0;
};
    
struct RetroShellCmd {

    // Used during command registration
    static string currentGroup;

    // Group of this command
    string groupName;

    // Name of this command (e.g., "eject")
    string name;

    // Full name of this command (e.g., "df0 eject")
    string fullName;

    // Name of this command as displayed in help messages (e.g., "[g]oto")
    string helpName;
    
    // Help description of this command (e.g., "Eject disk")
    // std::pair<string, string> help;
    string help;
    
    // List of required arguments
    std::vector<string> requiredArgs;

    // List of optional arguments
    std::vector<string> optionalArgs;

    // List of subcommands
    std::vector<RetroShellCmd> subCommands;

    // Command handler
    std::function<void (Arguments&, long)> callback = nullptr;

    // Additional argument passed to the command handler
    long param = 0;

    // Indicates if this command appears in help descriptions
    bool hidden = false;


    //
    // Methods
    //

    // Creates a new node in the command tree
    void add(const RetroShellCmdDescriptor &descriptor);
    
    // Wrappers (DEPRECATED)
    void add(const std::vector<string> &tokens,
             const string &help,
             std::function<void (Arguments&, long)> func = nullptr, long param = 0);

    void add(const std::vector<string> &tokens,
             std::pair<const string &, const string &> help,
             std::function<void (Arguments&, long)> func = nullptr, long param = 0);

    void add(const std::vector<string> &tokens,
             const std::vector<string> &args,
             const string &help,
             std::function<void (Arguments&, long)> func = nullptr, long param = 0);

    void add(const std::vector<string> &tokens,
             const std::vector<string> &args,
             std::pair<const string &, const string &> help,
             std::function<void (Arguments&, long)> func = nullptr, long param = 0);

    void add(const std::vector<string> &tokens,
             const std::vector<string> &requiredArgs,
             const std::vector<string> &optionalArgs,
             const string &help,
             std::function<void (Arguments&, long)> func = nullptr, long param = 0);

    void add(const std::vector<string> &tokens,
             const std::vector<string> &requiredArgs,
             const std::vector<string> &optionalArgs,
             std::pair<const string &, const string &> help,
             std::function<void (Arguments&, long)> func = nullptr, long param = 0);

    void clone(const string &alias,
               const std::vector<string> &tokens,
               long param = 0);

    void clone(const string &alias,
               const std::vector<string> &tokens,
               const string &help,
               long param = 0);

    // Returns arguments counts
    isize minArgs() const { return isize(requiredArgs.size()); }
    isize optArgs() const { return isize(optionalArgs.size()); }
    isize maxArgs() const { return minArgs() + optArgs(); }

    // Seeks a command object inside the command object tree
    const RetroShellCmd *seek(const string& token) const;
    RetroShellCmd *seek(const string& token);
    const RetroShellCmd *seek(const std::vector<string> &tokens) const;
    RetroShellCmd *seek(const std::vector<string> &tokens);
    RetroShellCmd &operator/(const string& token) { return *seek(token); }

    // Filters the argument list (used by auto-completion)
    std::vector<const RetroShellCmd *> filterPrefix(const string& prefix) const;

    // Automatically completes a partial token string
    string autoComplete(const string& token);

    // Returns a syntax string for this command
    string usage() const;
};

}
