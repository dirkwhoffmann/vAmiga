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

// Represents an uninterpreted sequence of tokens (e.g. "type -h lines=100")
typedef std::vector<string> Tokens;

// Parsed arguments (e.g. { {"h", "true"}, {"lines", "100"} })
typedef std::map<string,string> Arguments;

// Argument flags
namespace arg {

static const usize opt              = 1LL << 0;
static const usize keyval           = 1LL << 1;
static const usize hidden           = 1LL << 2 | opt;
static const usize flag             = 1LL << 3 | opt;

}

struct RSArgumentDescriptor {

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

struct RSCommandDescriptor {

    // Tokens the command is composed of
    const std::vector<string> &tokens = {};

    // General description of this command and all subcommands
    string ghelp = {};

    // Specific description of this command
    string chelp = {};

    // Hidden commands are not shown in help texts and are ingored in auto-completion
    bool hidden = false;

    // Shadowed commands are not shown in help texts, but auto-completion works as usual
    bool shadow = false;

    // Argument descriptions of this command
    const std::vector<RSArgumentDescriptor> &args = {};

    // The command callback
    std::function<void (std::ostream&, const Arguments&, const std::vector<isize>&)> func = nullptr;

    // Addition values passed to the command callback as last argument
    const std::vector<isize> &payload = {};
};
    
struct RSCommand {

    // Used during command registration
    static string currentGroup;

    // Group of this command
    string groupName;

    // Name of this command (e.g., "eject")
    string name;

    // Full name of this command (e.g., "df0 eject")
    string fullName;

    // General description of this command and all subcommands
    string ghelp;

    // Specific description of this command
    string chelp;

    // Hidden commands are not shown in help texts and are ingored in auto-completion
    bool hidden = false;

    // Shadowed commands are not shown in help texts, but auto-completion works as usual
    bool shadow = false;

    // Argument descriptions of this command
    std::vector<RSArgumentDescriptor> args;

    // Command handler
    std::function<void (std::ostream&, const Arguments&, const std::vector<isize>&)> callback = nullptr;

    // Addition values passed to the command callback as last argument
    std::vector<isize> payload;

    // List of subcommands
    std::vector<RSCommand> subcommands;


    //
    // Querying properties
    //

    bool isHidden() const { return hidden; } // (flags & arg::hidden) == arg::hidden; }
    bool isShadowed() const { return shadow; }


    //
    // Working with the command tree
    //

    // Creates a new node in the command tree
    void add(const RSCommandDescriptor &descriptor);

    // Registers an alias name for an existing command
    void clone(const std::vector<string> &tokens,
               const string &alias,
               const std::vector<isize> &values = { });

    // Seeks a command object inside the command object tree
    const RSCommand *seek(const string& token) const;
    const RSCommand *seek(const std::vector<string> &tokens) const;
    const RSCommand &operator/(const string& token) const { return *seek(token); }

    RSCommand *seek(const string& token);
    RSCommand *seek(const std::vector<string> &tokens);
    RSCommand &operator/(const string& token) { return *seek(token); }


    //
    // Auto-completing user input
    //

public:

    // Automatically completes a partial token string
    string autoComplete(const string& token);

private:

    // Filters the argument list (used by auto-completion)
    std::vector<const RSCommand *> filterPrefix(const string& prefix) const;


    //
    // Generating help messages
    //

public:

    // Returns a syntax description for subcommands or arguments
    string cmdUsage() const;
    string argUsage() const;

    // Displays a help text for a (partially typed in) command
    void printHelp(std::ostream &os);

private:

    void printArgumentHelp(std::ostream &os, isize indent, bool verbose = true);
    void printSubcmdHelp(std::ostream &os, isize indent, bool verbose = true);
};

}
