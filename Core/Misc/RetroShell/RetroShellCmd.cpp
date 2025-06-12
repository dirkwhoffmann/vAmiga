// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RetroShellCmd.h"
#include "StringUtils.h"
#include "Parser.h"
#include <algorithm>
#include <utility>

namespace vamiga {

string RetroShellCmd::currentGroup;

string
RSArgumentDescriptor::usage() const
{
    string result = name;

    if (hidden) return "";
    if (!flag) result = "<" + result + ">";
    if (!required) result = "[" + result + "]";

    return result;
}

void
RetroShellCmd::add(const RetroShellCmdDescriptor &descriptor)
{
    assert(!descriptor.tokens.empty());

    // Cleanse the token list (convert { "aaa bbb" } into { "aaa", "bbb" }
    auto tokens = util::split(descriptor.tokens, ' ');

    // The last entry in the token list is the command name
    auto name = tokens.back();
    
    // Determine how the token is displayed in help messages
    auto helpName = descriptor.help.size() > 1 ? descriptor.help[1] : name;

    // Traversing the command tree
    RetroShellCmd *node = seek(std::vector<string> { tokens.begin(), tokens.end() - 1 });
    assert(node != nullptr);
    
    // Create the instruction
    RetroShellCmd cmd;
    cmd.name = name;
    cmd.fullName = (node->fullName.empty() ? "" : node->fullName + " ") + helpName;
    cmd.helpName = helpName;
    cmd.groupName = currentGroup;
    cmd.requiredArgs = descriptor.args;
    cmd.optionalArgs = descriptor.extra;
    cmd.arguments = descriptor.argx;
    // cmd.requiredArgx = descriptor.argx;
    // cmd.optionalArgx = descriptor.extrx;
    cmd.flags = descriptor.flags;
    cmd.help = descriptor.help;
    cmd.callback = descriptor.func;
    cmd.param = descriptor.values;
    cmd.hidden = descriptor.hidden; //  || descriptor.help.empty();

    if (!cmd.hidden) currentGroup = "";

    // Register the instruction at the proper location
    node->subCommands.push_back(cmd);
}

void
RetroShellCmd::clone(const std::vector<string> &tokens,
                     const string &alias,
                     const std::vector<isize> &values)
{
    assert(!tokens.empty());

    // Find the command to clone
    RetroShellCmd *cmd = seek(std::vector<string> { tokens.begin(), tokens.end() });
    assert(cmd != nullptr);
    
    // Assemble the new token list
    auto newTokens = std::vector<string> { tokens.begin(), tokens.end() - 1 };
    newTokens.push_back(alias);
    
    // Create the instruction
    add(RetroShellCmdDescriptor {
        
        .tokens = newTokens,
        .hidden = true,
        .args   = cmd->requiredArgs,
        .extra  = cmd->optionalArgs,
        .func   = cmd->callback,
        .values = values
    });
}

const RetroShellCmd *
RetroShellCmd::seek(const string& token) const
{
    for (auto &it : subCommands) {
        if (it.name == token) return &it;
    }
    return nullptr;
}

RetroShellCmd *
RetroShellCmd::seek(const string& token)
{
    return const_cast<RetroShellCmd *>(std::as_const(*this).seek(token));
}

const RetroShellCmd *
RetroShellCmd::seek(const std::vector<string> &tokens) const
{
    const RetroShellCmd *result = this;
    
    for (auto &it : tokens) {
        if ((result = result->seek(it)) == nullptr) break;
    }
    
    return result;
}

RetroShellCmd *
RetroShellCmd::seek(const std::vector<string> &tokens)
{
    return const_cast<RetroShellCmd *>(std::as_const(*this).seek(tokens));
}

std::vector<const RetroShellCmd *>
RetroShellCmd::filterPrefix(const string& prefix) const
{
    std::vector<const RetroShellCmd *> result;
    auto uprefix = util::uppercased(prefix);

    for (auto &it : subCommands) {
        
        if (it.hidden) continue;
        auto substr = it.name.substr(0, prefix.size());
        if (util::uppercased(substr) == uprefix) result.push_back(&it);
    }

    return result;
}

string
RetroShellCmd::autoComplete(const string& token)
{
    string result;

    auto matches = filterPrefix(token);

    std::vector<string> tokens;
    for (auto &it : matches) { tokens.push_back(it->name); }

    result = util::commonPrefix(tokens);
    /*
    if (!matches.empty()) {
        
        const RetroShellCmd *first = matches.front();
        for (usize i = 0;; i++) {

            for (auto m: matches) {
                if (m->name.size() <= i || m->name[i] != first->name[i]) {
                    return result;
                }
            }
            result += first->name[i];
        }
    }
    */

    // return result;
    return result.size() >= token.size() ? result : token;
}

string
RetroShellCmd::usage() const
{
    // Returns a usage string for arguments
    auto argString = [&](bool flag) {

        std::vector<string> items;

        for (auto &it : arguments) {

            if (it.flag == flag) items.push_back(it.usage());
        }
        return util::concat(items);
    };

    // Returns a usage string for subcommands
    auto cmdString = [&]() {

        std::vector<string> items; string ldelim, rdelim;

        for (auto &it : subCommands) {

            if (it.hidden) continue;
            if (it.name != "") { ldelim = "["; rdelim = "]"; continue; }
            items.push_back(it.name);
        }
        return ldelim + util::concat(items, " | ", "{ ", " }") + rdelim;
    };

    //
    // Old code
    //

    string resultstr;

    if (arguments.empty()) { // } requiredArgx.empty() && optionalArgx.empty() && flags.empty()) {

        if (subCommands.empty()) {

            string required;
            string optional;

            for (isize i = 0; i < minArgs(); i++) {

                required += requiredArgs[i];
                required += " ";
            }
            for (isize i = 0; i < optArgs(); i++) {

                optional += optionalArgs[i];
                optional += " ";
            }
            if (optional != "") optional = "[ " + optional + "]";

            resultstr = required + optional;

        } else {

            // Collect all sub-commands
            isize count = 0;
            for (auto &it : subCommands) {

                if (it.hidden) continue;

                if (it.name != "") {

                    if (count++) resultstr += " | ";
                    resultstr += it.name;
                }
            }
            if (count > 1) {
                resultstr = "{" + resultstr + "}";
            }
            if (seek("") && resultstr != "") {
                resultstr = "[ " + resultstr + " ]";
            }
        }

        return fullName + " " + resultstr;
    }

    //
    // New code
    //

    if (subCommands.empty()) {
        return util::concat({ fullName, argString(true), argString(false) });
    } else {
        return util::concat({ fullName, cmdString() });
    }
}

namespace arg {

string
Token::autoComplete(const string &prefix) const
{
    auto length = prefix.length();
    if (length > token.length()) return "";
    
    for (usize i = 0; i < length; i++) {
        if (std::toupper(prefix[i]) != std::toupper(token[i])) return "";
    }
    
    return token;
}

}

}
