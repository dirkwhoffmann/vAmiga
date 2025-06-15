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
RSArgDescriptor::nameStr() const
{
    return name[0];
}

string
RSArgDescriptor::helpStr() const
{
    return name.size() > 1 ? name[1] : "" ;
}

string
RSArgDescriptor::keyStr() const
{
    if (key.empty()) {

        if (isStdArg())         return "";
        if (isKeyValuePair())   return nameStr();
        if (isFlag())           return "-" + nameStr();
    }
    return key;
}

string
RSArgDescriptor::valueStr() const
{
    if (value.empty()) {

        if (isStdArg())         return "<" + nameStr() + ">";
        if (isKeyValuePair())   return "<arg>";
        if (isFlag())           return "";
    }
    return value;
}

string
RSArgDescriptor::keyValueStr() const
{
    if (key.empty()) {

        if (isStdArg())         return valueStr();
        if (isKeyValuePair())   return keyStr() + "=" + valueStr();
        if (isFlag())           return keyStr();
    }
    return key;
}

string
RSArgDescriptor::usageStr() const
{
    return isHidden() ? "" : isRequired() ? keyValueStr() : "[" + keyValueStr() + "]";
}

void
RetroShellCmd::add(const RSCmdDescriptor &descriptor)
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
    cmd.fullName = util::concat({ node->fullName, helpName }, " ");
    // cmd.helpName = helpName;
    cmd.groupName = currentGroup;
    cmd.requiredArgs = descriptor.args;
    cmd.optionalArgs = descriptor.extra;
    cmd.arguments = descriptor.argx;
    cmd.help = descriptor.help;
    cmd.chelp = descriptor.help.empty() ? "" : descriptor.help[0];
    cmd.ghelp = descriptor.ghelp.empty() ? cmd.chelp : descriptor.ghelp;
    cmd.thelp = helpName;
    cmd.callback = descriptor.func;
    cmd.param = descriptor.values;
    cmd.hidden = descriptor.hidden;

    // Reset the group
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
    add(RSCmdDescriptor {
        
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
    return result.size() >= token.size() ? result : token;
}

string
RetroShellCmd::cmdUsage() const
{
    std::vector<string> items;

    for (auto &it : subCommands) {
        if (!it.hidden) items.push_back(it.name);
    }
    auto combined = util::concat(items, " | ", callback ? "[ " : "{ ", callback ? " ]" : " }");
    return  util::concat({ fullName, combined });
}

string
RetroShellCmd::argUsage() const
{
    // Create a common usage string for all flags
    string flags = "";

    for (auto &it : arguments) {
        if (it.isFlag()) flags += it.nameStr()[0];
    }
    if (!flags.empty()) flags = "[-" + flags + "]";

    // Create a usage string for all other arguments
    std::vector<string> items;

    for (auto &it : arguments) {
        if (!it.isFlag()) items.push_back(it.usageStr());
    }
    string other = util::concat(items);

    return util::concat({ fullName, flags, other });
}

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
