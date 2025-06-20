// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Infrastructure/Reflection.h"
#include <functional>
#include <stack>

namespace vamiga {

class RetroShell;

typedef std::vector<string> Arguments;
typedef std::map<string,string> ParsedArguments;

namespace arg {

static const std::string address    = "<address>";
static const std::string boolean    = "{true|false}";
static const std::string command    = "<command>";
static const std::string count      = "<count>";
static const std::string dst        = "<destination>";
static const std::string ignores    = "<ignores>";
static const std::string kb         = "<kb>";
static const std::string nr         = "<nr>";
static const std::string onoff      = "{on|off}";
static const std::string path       = "<path>";
static const std::string process    = "<process>";
static const std::string seconds    = "<seconds>";
static const std::string value      = "<value>";
static const std::string sequence   = "<byte sequence>";
static const std::string src        = "<source>";
static const std::string volume     = "<volume>";
static const std::string string     = "<string>";

static const usize opt              = 1LL << 0;
static const usize keyval           = 1LL << 1;
static const usize hidden           = 1LL << 2 | opt;
static const usize flag             = 1LL << 3 | opt;

}

struct Token {

    string token;

    Token(const string &s) : token(s) { };
    Token(const char *s) : token(string(s)) { };
    string autoComplete(const std::string &prefix) const;
};

struct RSArgDescriptor {

    std::vector<string> name;
    string key;
    string value;
    usize flags;

    bool isFlag() const { return (flags & arg::flag) == arg::flag; }
    bool isKeyValuePair() const { return (flags & arg::keyval) == arg::keyval; }
    bool isStdArg() const { return !isFlag() && !isKeyValuePair(); }
    bool isHidden() const { return (flags & arg::hidden) == arg::hidden; }
    bool isOptional() const { return (flags & arg::opt) == arg::opt; }
    bool isRequired() const { return !isOptional(); }

    string nameStr() const;
    string helpStr() const;
    string keyStr() const;
    string valueStr() const;
    string keyValueStr() const;
    string usageStr() const;
};

struct RSCmdDescriptor {
    
    const std::vector<string> &tokens = {};
    const std::vector<string> &hide = {};
    string thelp = {};      // Command token help
    string ghelp = {};      // Command group help
    string chelp = {};      // Command help
    bool hidden = false;    // Invisible in help, ignored in auto-completion
    bool shadow = false;    // Invisible in help, subject to auto-completion
    const std::vector<RSArgDescriptor> &argx = {}; // TODO: Rename to args
    std::function<void (std::ostream &os, Arguments&, const ParsedArguments &args, const std::vector<isize> &)> func = nullptr;
    const std::vector<isize> &values = {};
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

    // Command help
    string chelp;

    // Command group help (defaults to the command help)
    string ghelp;

    // Command token help (e.g. "[g]oto") (defaults to the command name)
    // string thelp;

    // Argument list
    std::vector<RSArgDescriptor> arguments;

    // List of subcommands
    std::vector<RetroShellCmd> subCommands;

    // Command handler
    std::function<void (std::ostream&, Arguments&, const ParsedArguments&, const std::vector<isize>&)> callback = nullptr;

    // Additional argument passed to the command handler
    std::vector<isize> param;

    // Indicates if this command appears in help descriptions and is ignores in auto-completion
    bool hidden = false;

    // Indicates if this command appears in help descriptions
    bool shadowed = false;


    //
    // Working with the command tree
    //

    // Creates a new node in the command tree
    void add(const RSCmdDescriptor &descriptor);

    // Registers an alias name for an existing command
    void clone(const std::vector<string> &tokens,
               const string &alias,
               const std::vector<isize> &values = { });

    // Returns arguments counts
    /*
    isize minArgs() const { return isize(requiredArgs.size()); }
    isize optArgs() const { return isize(optionalArgs.size()); }
    isize maxArgs() const { return minArgs() + optArgs(); }
    */

    // Seeks a command object inside the command object tree
    const RetroShellCmd *seek(const string& token) const;
    RetroShellCmd *seek(const string& token);
    const RetroShellCmd *seek(const std::vector<string> &tokens) const;
    RetroShellCmd *seek(const std::vector<string> &tokens);
    RetroShellCmd &operator/(const string& token) { return *seek(token); }


    //
    // Auto-completing user input
    //

public:

    // Automatically completes a partial token string
    string autoComplete(const string& token);

private:

    // Filters the argument list (used by auto-completion)
    std::vector<const RetroShellCmd *> filterPrefix(const string& prefix) const;


    //
    // Generating help messages
    //

public:

    // Returns a syntax description for subcommands or arguments
    string cmdUsage() const;
    string argUsage() const;

    // Displays a help text for a (partially typed in) command
    void printHelp(std::ostream &os);
    void printArgumentHelp(std::ostream &os, isize indent, bool verbose = true);
    void printSubcmdHelp(std::ostream &os, isize indent, bool verbose = true);

};

}
