// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RSCommand.h"
#include "StringUtils.h"
#include "Parser.h"
#include <algorithm>
#include <utility>

namespace vamiga {

string RSCommand::currentGroup;

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
RSCommand::add(const RSCmdDescriptor &descriptor)
{
    assert(!descriptor.tokens.empty());
    assert(!descriptor.chelp.empty() || !descriptor.ghelp.empty());

    // Cleanse the token list (convert { "aaa bbb" } into { "aaa", "bbb" }
    auto tokens = util::split(descriptor.tokens, ' ');

    // The last entry in the token list is the command name
    auto name = tokens.back();

    // Traversing the command tree
    RSCommand *node = seek(std::vector<string> { tokens.begin(), tokens.end() - 1 });
    assert(node != nullptr);
    
    // Create the instruction
    RSCommand cmd;
    cmd.name = name;
    cmd.ghelp = !descriptor.ghelp.empty() ? descriptor.ghelp : descriptor.chelp;
    cmd.chelp = !descriptor.chelp.empty() ? descriptor.chelp : "???";
    cmd.fullName = util::concat({ node->fullName, name }, " ");
    cmd.groupName = currentGroup;
    cmd.arguments = descriptor.argx;
    cmd.callback = descriptor.func;
    cmd.param = descriptor.values;
    cmd.hidden = descriptor.hidden;
    cmd.shadowed = descriptor.shadow;

    // Reset the group
    if (!cmd.hidden) currentGroup = "";

    // Register the instruction at the proper location
    node->subCommands.push_back(cmd);
}

void
RSCommand::clone(const std::vector<string> &tokens,
                     const string &alias,
                     const std::vector<isize> &values)
{
    assert(!tokens.empty());

    // Find the command to clone
    RSCommand *cmd = seek(std::vector<string> { tokens.begin(), tokens.end() });
    assert(cmd != nullptr);
    
    // Assemble the new token list
    auto newTokens = std::vector<string> { tokens.begin(), tokens.end() - 1 };
    newTokens.push_back(alias);
    
    // Create the instruction
    add(RSCmdDescriptor {
        
        .tokens = newTokens,
        .ghelp  = cmd->ghelp,
        .chelp  = cmd->chelp,
        .hidden = true,
        .argx   = cmd->arguments,
        .func   = cmd->callback,
        .values = values
    });
}

const RSCommand *
RSCommand::seek(const string& token) const
{
    for (auto &it : subCommands) {
        if (it.name == token) return &it;
    }
    return nullptr;
}

RSCommand *
RSCommand::seek(const string& token)
{
    return const_cast<RSCommand *>(std::as_const(*this).seek(token));
}

const RSCommand *
RSCommand::seek(const std::vector<string> &tokens) const
{
    const RSCommand *result = this;
    
    for (auto &it : tokens) {
        if ((result = result->seek(it)) == nullptr) break;
    }
    
    return result;
}

RSCommand *
RSCommand::seek(const std::vector<string> &tokens)
{
    return const_cast<RSCommand *>(std::as_const(*this).seek(tokens));
}

std::vector<const RSCommand *>
RSCommand::filterPrefix(const string& prefix) const
{
    std::vector<const RSCommand *> result;
    auto uprefix = util::uppercased(prefix);

    for (auto &it : subCommands) {
        
        if (it.hidden) continue;
        auto substr = it.name.substr(0, prefix.size());
        if (util::uppercased(substr) == uprefix) result.push_back(&it);
    }

    return result;
}

string
RSCommand::autoComplete(const string& token)
{
    string result;

    auto matches = filterPrefix(token);

    std::vector<string> tokens;
    for (auto &it : matches) { tokens.push_back(it->name); }

    result = util::commonPrefix(tokens);
    return result.size() >= token.size() ? result : token;
}

void
RSCommand::printHelp(std::ostream &os)
{
    string prefix;

    if (!subCommands.empty()) {

        // Describe all subcommands
        prefix = "Cmds: ";
        os << prefix + cmdUsage() << std::endl;
        printSubcmdHelp(os, isize(prefix.size()));

        if (callback && !arguments.empty()) {

            // Describe the current command
            prefix = string(prefix.size(), ' ') + "Usage: ";
            os << prefix + argUsage() << std::endl;
            printArgumentHelp(os, isize(prefix.size()), false);
        }

    } else {

        // Describe the current command
        prefix = "Usage: ";
        os << prefix + argUsage() << std::endl;
        printArgumentHelp(os, isize(prefix.size()));
    }
}

void
RSCommand::printArgumentHelp(std::ostream &os, isize indent, bool verbose)
{
    auto skip = [](const RSArgDescriptor &it) { return it.isHidden() || it.helpStr().empty(); };

    // Gather all arguments with a help description
    std::vector<RSArgDescriptor *> args;
    for (auto &it : arguments) { if (!skip(it)) args.push_back(&it); }

    // Determine the tabular position to align the output
    isize tab = 0;
    for (auto &it : args) { tab = std::max(tab, (isize)it->keyValueStr().length()); }

    // Print command description
    if (verbose) os << std::endl << string(indent, ' ') << chelp << std::endl;

    if (!args.empty()) {

        os << std::endl;

        // Print argument descriptions
        for (auto &it : args) {

            os << string(indent, ' ') << std::left << std::setw(int(tab)) << it->keyValueStr() << " : ";
            os << it->helpStr() << std::endl;
        }
    }
    os << std::endl;
}

void
RSCommand::printSubcmdHelp(std::ostream &os, isize indent, bool verbose)
{
    if (subCommands.empty()) return;

    // Collect all commands
    std::vector<const RSCommand *> cmds;
    if (callback) cmds.push_back(this);
    for (auto &it : subCommands) { if (!it.hidden && !it.shadowed) cmds.push_back(&it); }

    // Determine alignment parameters to get a properly formatted output
    isize newlines = 1, tab = 0;
    for (auto &it : cmds) {
        tab = std::max(tab, (isize)it->fullName.length());
    }

    for (auto &it : cmds) {

        // For top-level commands, print the command group (if present)
        if (!it->groupName.empty() && name.empty()) {

            // *this << '\n' << it->groupName << '\n';
            os << std::endl << it->groupName << std::endl;
            newlines = 1;
        }

        // Print newlines
        for (; newlines > 0; newlines--) os << std::endl;

        // Print command description
        os << string(indent, ' ') << std::left << std::setw(int(tab)) << it->fullName << " : ";
        os << (it == this ? it->chelp : it->ghelp) << std::endl;
    }
    os << std::endl;
}

string
RSCommand::cmdUsage() const
{
    std::vector<string> items;

    for (auto &it : subCommands) {
        if (!it.hidden) items.push_back(it.name);
    }
    auto combined = util::concat(items, " | ", callback ? "[ " : "{ ", callback ? " ]" : " }");
    return  util::concat({ fullName, combined });
}

string
RSCommand::argUsage() const
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
