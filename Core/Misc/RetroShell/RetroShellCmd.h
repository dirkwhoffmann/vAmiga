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
    const std::vector<string> &args = {};
    const std::vector<string> &extra = {};
    const std::vector<string> help = {};
    std::function<void (Arguments&, const std::vector<isize> &)> func = nullptr;
    const std::vector<isize> &values = {};
};
    
struct RetroShellCmd { // DEPRECATED

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
    std::vector<string> help;
    
    // List of required arguments
    std::vector<string> requiredArgs;

    // List of optional arguments
    std::vector<string> optionalArgs;

    // List of subcommands
    std::vector<RetroShellCmd> subCommands;

    // Command handler
    std::function<void (Arguments&, const std::vector<isize> &)> callback = nullptr;

    // Additional argument passed to the command handler
    std::vector<isize> param;

    // Indicates if this command appears in help descriptions
    bool hidden = false;


    //
    // Methods
    //

    // Creates a new node in the command tree
    void add(const RetroShellCmdDescriptor &descriptor);
    
    // Registers an alias name for an existing command 
    void clone(const std::vector<string> &tokens,
               const string &alias,
               const std::vector<isize> &values = { });
    
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

//
// New code
//

namespace rs {

struct Token {

    string token;

    Token(const string &s) : token(s) { };
    Token(const char *s) : token(string(s)) { };
    string autoComplete(const string &prefix) const;
};

struct Argument {

    string arg;

    Argument(const char *s) : arg(string(s)) { };
};

struct String : Argument {

    using Argument::Argument;
};

struct Addr : Argument {

    using Argument::Argument;
};

struct Range : Argument {

    isize min, max;

    Range(const char *s, isize min, isize max) : Argument(s), min(min), max(max) { }
};

struct Switch {

    Token name;
    std::vector<Argument> args;

    Switch(const char *s, Argument arg) : name(s), args({arg}) { }
};

struct Command {

    // Used during command registration
    static string currentGroup;

    // Parent command (if any)
    Command *parent = nullptr;

    // Group of this command
    string groupName;

    // Name (e.g. "goto")
    string name;

    // Name as displayed in help messages (e.g. "[g]oto")
    string helpName;

    // Command description
    string help = "Help me!";

    // Hidden flag
    bool hidden = false;

    // Subcommands (if any)
    std::vector<Command> subcommands;

    // Mandatory arguments (must appear in order)
    std::vector<Argument> args;

    // Optional arguments (must appear in order)
    std::vector<Argument> extra;

    // Switches (may appear everywhere)
    std::vector<Switch> switches;

    string fullName() const; 
};

struct Descriptor {

    const std::vector<Token> &tokens = {};
    bool hidden = false;
    const std::vector<string> name = {};
    const std::vector<Argument> &args = {};
    const std::vector<Argument> &extra = {};
    const std::vector<Switch> &switches = {};
    const std::vector<string> help = {};
    std::function<void (Arguments&, const std::vector<isize> &)> func = nullptr;
    const std::vector<isize> &values = {};
};

}

}
