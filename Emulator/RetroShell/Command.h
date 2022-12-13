// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include <vector>

namespace vamiga {

class RetroShell;

typedef std::vector<string> Arguments;

struct Command {

    //Textual descriptions of all command groups
    static std::vector<string> groups;

    // Name of this command (e.g., "eject")
    string name;

    // Full name of this command (e.g., "df0 eject")
    string fullName;

    // Command group of this command
    isize group;

    // String describing the token type (DEPRECATED)
    // string type;
    
    // Help string
    string info;
    
    // Sub commands
    std::vector<Command> subCommands;
    
    // Command handler
    void (RetroShell::*action)(Arguments&, long) = nullptr;
    
    // Minimum and maximum number of arguments expected by the command handler
    isize minArgs = 0;
    isize maxArgs = 0;
    
    // Additional paramter passed to the command handler
    long param = 0;
    
    // Indicates if this command appears in the help descriptions
    bool hidden = false;

    // Creates a new command group
    void addGroup(const string &description);

    // Creates a new node in the command tree
    void add(const std::vector<string> &tokens,
             const string &help);
    
    void add(const std::vector<string> &tokens,
             const string &help,
             void (RetroShell::*action)(Arguments&, long),
             isize numArgs,
             long param = 0);
    
    void add(const std::vector<string> &tokens,
             const string &help,
             void (RetroShell::*action)(Arguments&, long),
             std::pair <isize,isize> numArgs,
             long param = 0);

    // Removes a registered command
    void remove(const string& token);
    
    // Seeks a command object inside the command object tree
    Command *seek(const string& token);
    Command *seek(const std::vector<string> &tokens);
        
    // Filters the argument list
    std::vector<const Command *> filterPrefix(const string& prefix) const;

    // Automatically completes a partial token string
    string autoComplete(const string& token);
        
    // Returns a syntax string for this command
    string usage() const;
};

}
