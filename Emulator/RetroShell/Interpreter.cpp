// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Amiga.h"
#include <sstream>

namespace vamiga {

Interpreter::Interpreter(Amiga &ref) : SubComponent(ref)
{
    initCommandShell(commandShellRoot);
    initDebugShell(debugShellRoot);
}

Arguments
Interpreter::split(const string& userInput)
{
    std::stringstream ss(userInput);
    Arguments result;

    string token;
    bool str = false; // String mode
    bool esc = false; // Escape mode
    
    for (usize i = 0; i < userInput.size(); i++) {

        char c = userInput[i];
        
        // Check for escape mode
        if (c == '\\') { esc = true; continue; }

        // Switch between string mode and non-string mode if '"' is detected
        if (c == '"' && !esc) { str = !str; continue; }
        
        // Check for special characters in escape mode
        if (esc && c == 'n') c = '\n';
        
        // Process character
        if (c != ' ' || str) {
            token += c;
        } else {
            if (!token.empty()) result.push_back(token);
            token = "";
        }
        esc = false;
    }
    if (!token.empty()) result.push_back(token);
    
    return result;
}

string
Interpreter::autoComplete(const string& userInput)
{
    string result;
    
    // Split input string
    Arguments tokens = split(userInput);
    
    // Complete all tokens
    autoComplete(tokens);

    // Recreate the command string
    for (const auto &it : tokens) { result += (result == "" ? "" : " ") + it; }

    // Add a space if the command has been fully completed
    if (!tokens.empty() && getRoot().seek(tokens)) result += " ";
    
    return result;
}

void
Interpreter::autoComplete(Arguments &argv)
{
    Command *current = &getRoot();
    string prefix, token;

    for (auto it = argv.begin(); current && it != argv.end(); it++) {
        
        *it = current->autoComplete(*it);
        current = current->seek(*it);
    }
}

Command &
Interpreter::getRoot()
{
    switch (shell) {

        case Shell::Command: return commandShellRoot;
        case Shell::Debug: return debugShellRoot;

        default:
            fatalError;
    }
}

void
Interpreter::switchInterpreter()
{
    if (inCommandShell()) {

        shell = Shell::Debug;
        amiga.debugOn();

    } else {

        shell = Shell::Command;
    }

    retroShell.updatePrompt();
}

void
Interpreter::exec(const string& userInput, bool verbose)
{
    // Split the command string
    Arguments tokens = split(userInput);

    // Skip empty lines
    if (tokens.empty()) return;
    
    // Remove the 'try' keyword
    if (tokens.front() == "try") tokens.erase(tokens.begin());
    
    // Auto complete the token list
    autoComplete(tokens);

    // Process the command
    exec(tokens, verbose);
}

void
Interpreter::exec(const Arguments &argv, bool verbose)
{
    // In 'verbose' mode, print the token list
    if (verbose) {
        for (const auto &it : argv) retroShell << it << ' ';
        retroShell << '\n';
    }
    
    // Skip empty lines
    if (argv.empty()) return;
    
    // Seek the command in the command tree
    Command *current = &getRoot(), *next;
    Arguments args = argv;

    while (!args.empty() && ((next = current->seek(args.front())) != nullptr)) {
        
        current = current->seek(args.front());
        args.erase(args.begin());
    }

    // Error out if no command handler is present
    if (current->action == nullptr && !args.empty()) {
        throw util::ParseError(args.front());
    }
    if (current->action == nullptr && args.empty()) {
        throw TooFewArgumentsError(current->tokenList);
    }
    
    // Check the argument count
    printf("current = %p\n", (void *)current); 
    if ((isize)args.size() < current->minArgs) throw TooFewArgumentsError(current->tokenList);
    if ((isize)args.size() > current->maxArgs) throw TooManyArgumentsError(current->tokenList);
    
    // Call the command handler
    (retroShell.*(current->action))(args, current->param);
}

void
Interpreter::usage(const Command& current)
{
    retroShell << "Usage: " << current.usage() << '\n';
}

void
Interpreter::help(const string& userInput)
{    
    // Split the command string
    Arguments tokens = split(userInput);

    // Auto complete the token list
    autoComplete(tokens);

    // Process the command
    help(tokens);
}

void
Interpreter::help(const Arguments &argv)
{
    Command *current = &getRoot();
    string prefix, token;

    for (auto &it : argv) {
        if (current->seek(it) != nullptr) current = current->seek(it);
    }
    
    help(*current);
}

void
Interpreter::help(const Command& current)
{
    // auto tokens = current.tokens();
    // auto length = tokens.size();
    auto indent = string("    ");

    // Print the usage string
    usage(current);
    
    // Collect all argument types
    auto types = current.types();

    // Determine tabular positions to align the output
    isize tab = 0;
    for (auto &it : current.args) {
        tab = std::max(tab, (isize)it.tokenList.length());
        // tab = std::max(tab, 2 + (isize)it.type.length());
    }
    tab += indent.size();

    isize group = -1;

    for (auto &it : current.args) {

        // Only proceed if the command is visible
        if (it.hidden) continue;

        // Print group description (when a new group begins)
        if (group != it.group) {

            group = it.group;
            retroShell << '\n';

            if (!Command::groups[group].empty()) {
                retroShell << Command::groups[group] << '\n' << '\n';
            }
        }

        // Print command descriptioon
        // string name = token + " " + it.token;
        retroShell << indent;
        retroShell << it.tokenList;
        retroShell.tab(tab);
        retroShell << " : ";
        retroShell << it.info;
        retroShell << '\n';
    }
    retroShell << '\n';


    /*
    for (auto &it : types) {
        
        auto opts = current.filterType(it);
        int size = (int)it.length();

        retroShell.tab(tab - size);
        retroShell << "<" << it << "> : ";
        retroShell << (isize)opts.size() << (opts.size() == 1 ? " choice" : " choices");
        retroShell << '\n' << '\n';
        
        for (auto &opt : opts) {

            string name = opt->token == "" ? "<>" : opt->token;
            retroShell.tab(tab + 2 - (isize)name.length());
            retroShell << name;
            retroShell << " : ";
            retroShell << opt->info;
            retroShell << '\n';
        }
        retroShell << '\n';
    }
    */
}

}
