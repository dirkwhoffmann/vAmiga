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

namespace arg {

// enum class type { std, flag, keyval };

static constexpr std::string address    = "<address>";
static constexpr std::string boolean    = "{true|false}";
static constexpr std::string command    = "<command>";
static constexpr std::string count      = "<count>";
static constexpr std::string dst        = "<destination>";
static constexpr std::string ignores    = "<ignores>";
static constexpr std::string kb         = "<kb>";
static constexpr std::string nr         = "<nr>";
static constexpr std::string onoff      = "{on|off}";
static constexpr std::string path       = "<path>";
static constexpr std::string process    = "<process>";
static constexpr std::string seconds    = "<seconds>";
static constexpr std::string value      = "<value>";
static constexpr std::string sequence   = "<byte sequence>";
static constexpr std::string src        = "<source>";
static constexpr std::string volume     = "<volume>";
static constexpr std::string string     = "<string>";

static constexpr usize opt              = 1LL << 0;
static constexpr usize keyval           = 1LL << 1;
static constexpr usize hidden           = 1LL << 2 | opt;
static constexpr usize flag             = 1LL << 3 | opt;

}

struct Token {

    string token;

    Token(const string &s) : token(s) { };
    Token(const char *s) : token(string(s)) { };
    string autoComplete(const std::string &prefix) const;
};

struct RSArgumentDescriptor {

    std::vector<string> name;
    // arg::type type;
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

struct RetroShellCmdDescriptor {
    
    const std::vector<string> &tokens = {};
    bool hidden = false;
    const std::vector<string> &args = {}; // DEPRECATED
    const std::vector<string> &extra = {}; // DEPRECATED
    const std::vector<RSArgumentDescriptor> &argx = {}; // TODO: Rename to args
    const std::vector<string> help = {};
    std::function<void (Arguments&, const std::vector<isize> &)> func = nullptr;
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

    // Name of this command as displayed in help messages (e.g., "[g]oto")
    string helpName;
    
    // Help description of this command (e.g., "Eject disk")
    std::vector<string> help;
    
    // List of required arguments (DEPRECATED)
    std::vector<string> requiredArgs;

    // List of optional arguments (DEPRECATED)
    std::vector<string> optionalArgs;

    // Argument list
    std::vector<RSArgumentDescriptor> arguments;

    // Mandatory arguments (must appear in order)Add commentMore actions
    // std::vector<arg::Argument> requiredArgx; // TODO: Rename to requiredArgs

    // Optional arguments (must appear in order)
    // std::vector<arg::Argument> optionalArgx; // TODO: Rename to optionalArgs

    // Flags (may appear everywhere)
    // std::vector<arg::Flag> flags;

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

}
