// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Command.h"
#include <algorithm>

namespace vamiga {

std::vector<string> Command::groups;

void
Command::newGroup(const string &description, const string &postfix)
{
    groups.push_back(description + postfix);
}

void
Command::add(const std::vector<string> &tokens,
             const string &help)
{
    add(tokens, help, nullptr, 0, 0);
}

void
Command::add(const std::vector<string> &tokens,
             const string &help,
             void (RetroShell::*action)(Arguments&, long),
             isize numArgs,
             long param)
{
    add(tokens, help, action, { numArgs, numArgs }, param);
}

void
Command::add(const std::vector<string> &tokens,
             const string &help,
             void (RetroShell::*action)(Arguments&, long),
             std::pair <isize,isize> numArgs,
             long param)
{
    assert(!tokens.empty());
    
    // Traverse the node tree
    Command *cmd = seek(std::vector<string> { tokens.begin(), tokens.end() - 1 });
    assert(cmd != nullptr);

    // Install the action handler in the parent node if this is no sub-command
    if (tokens.back() == "") cmd->action = action;

    // Create the instruction
    Command d;
    d.name = tokens.back();
    d.fullName = (cmd->fullName.empty() ? "" : cmd->fullName + " ") + tokens.back();
    d.group = groups.size() - 1;
    d.help = help;
    d.action = action;
    d.minArgs = numArgs.first;
    d.maxArgs = numArgs.second;
    d.param = param;

    // Register the instruction
    cmd->subCommands.push_back(d);
}

void
Command::hide(const std::vector<string> &tokens)
{
    Command *cmd = seek(std::vector<string> { tokens.begin(), tokens.end() });
    assert(cmd != nullptr);

    cmd->hidden = true;
}

void
Command::remove(const string& token)
{
    for(auto it = std::begin(subCommands); it != std::end(subCommands); ++it) {
        if (it->name == token) { subCommands.erase(it); return; }
    }
}

Command *
Command::seek(const string& token)
{
    for (auto &it : subCommands) {
        if (it.name == token) return &it;
    }
    return nullptr;
}

Command *
Command::seek(const std::vector<string> &tokens)
{
    Command *result = this;
    
    for (auto &it : tokens) {
        if ((result = result->seek(it)) == nullptr) break;
    }
    
    return result;
}

std::vector<const Command *>
Command::filterPrefix(const string& prefix) const
{
    std::vector<const Command *> result;
    
    for (auto &it : subCommands) {
        
        if (it.hidden) continue;
        if (it.name.substr(0, prefix.size()) == prefix) result.push_back(&it);
    }

    return result;
}

string
Command::autoComplete(const string& token)
{
    string result = token;
    
    auto matches = filterPrefix(token);
    if (!matches.empty()) {
        
        const Command *first = matches.front();
        for (auto i = token.size(); i < first->name.size(); i++) {
            
            for (auto m: matches) {
                if (m->name.size() <= i || m->name[i] != first->name[i]) {
                    return result;
                }
            }
            result += first->name[i];
        }
    }
    return result;
}

string
Command::usage() const
{
    string arguments;

    if (subCommands.empty()) {

        arguments = minArgs == 0 ? "" : minArgs == 1 ? "<value>" : "<values>";
        if (maxArgs - minArgs == 1) arguments += " [ <value> ]";
        if (maxArgs - minArgs >= 2) arguments += " [ <values> ]";
        if (arguments == "") arguments = "<no arguments>";

    } else {

        arguments = action ? "[ <command> ]" : "<command>";
    }

    return fullName + " " + arguments;
}

}
