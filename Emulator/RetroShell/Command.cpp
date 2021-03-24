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

namespace va {

Command *
Command::add(const string &token,
             const string &a1,
             const string &help,
             void *ptr,
             isize num, long param)
{
    // Make sure the key does not yet exist
    assert(seek(token) == nullptr);

    // Expand template tokens
    /*
    if (token == "controlport") {
        add("controlport1", a1, help, func, num, PORT_1);
        add("controlport2", a1, help, func, num, PORT_2);
        return nullptr;
    }
    if (token == "cia") {
        add("ciaa", a1, help, func, num, 0);
        add("ciab", a1, help, func, num, 1);
        return nullptr;
    }
    if (token == "df") {
        add("df0", a1, help, func, num, 0);
        add("df1", a1, help, func, num, 1);
        add("df2", a1, help, func, num, 2);
        add("df3", a1, help, func, num, 3);
        return nullptr;
    }
    */
    
    // Register instruction
    Command d { this, token, a1, help, std::list<Command>(), func, num, param };
    args.push_back(d);
    
    return seek(token);
}

Command *
Command::add(const string &t1, const string &t2,
             const string &a1,
             const string &help,
             void *func,
             // void (Controller::*func)(Arguments&, long),
             isize num, long param)
{
    // Expand template tokens
    /*
    if (t1 == "controlport") {
        add("controlport1", t2, a1, help, func, num, PORT_1);
        add("controlport2", t2, a1, help, func, num, PORT_2);
        return nullptr;
    }
    if (t1 == "cia") {
        add("ciaa", t2, a1, help, func, num, 0);
        add("ciab", t2, a1, help, func, num, 1);
        return nullptr;
    }
    if (t1 == "df") {
        add("df0", t2, a1, help, func, num, 0);
        add("df1", t2, a1, help, func, num, 1);
        add("df2", t2, a1, help, func, num, 2);
        add("df3", t2, a1, help, func, num, 3);
        if (t2 == "set") { add("dfn", t2, a1, help, func, num, 4); }
        return nullptr;
    }
    */
    
    return seek(t1)->add(t2, a1, help, func, num, param);
}

Command *
Command::add(const string &t1, const string &t2, const string &t3,
             const string &a1,
             const string &help,
             // void (Controller::*func)(Arguments&, long),
             void *func,
             isize num, long param)
{
    // Expand template tokens
    /*
    if (t1 == "controlport") {
        add("controlport1", t2, t3, a1, help, func, num, PORT_1);
        add("controlport2", t2, t3, a1, help, func, num, PORT_2);
        return nullptr;
    }
    if (t1 == "cia") {
        add("ciaa", t2, t3, a1, help, func, num, 0);
        add("ciab", t2, t3, a1, help, func, num, 1);
        return nullptr;
    }
    if (t1 == "df") {
        add("df0", t2, t3, a1, help, func, num, 0);
        add("df1", t2, t3, a1, help, func, num, 1);
        add("df2", t2, t3, a1, help, func, num, 2);
        add("df3", t2, t3, a1, help, func, num, 3);
        if (t2 == "set") { add("dfn", t2, t3, a1, help, func, num, 4); }
        return nullptr;
    }
    */
    return seek(t1)->add(t2, t3, a1, help, func, num, param);
}

Command *
Command::add(const string &t1, const string &t2, const string &t3, const string &t4,
             const string &a1,
             const string &help,
             // void (Controller::*func)(Arguments&, long),
             void *func,
             isize num, long param)
{
    assert(t1 != "controlport");
    assert(t1 != "cia");
    assert(t1 != "df");
    
    return seek(t1)->add(t2, t3, t4, a1, help, func, num, param);
}

void
Command::remove(const string& token)
{
    for(auto it = std::begin(args); it != std::end(args); ++it) {
        if (it->token == token) { args.erase(it); return; }
    }
}

Command *
Command::seek(const string& token)
{
    for (auto& it : args) {
        if (it.token == token) return &it;
    }
    return nullptr;
}

Command *
Command::seek(Arguments argv)
{
    Command *result = this;
    
    for (auto& it : argv) {
        if (!(result = result->seek(it))) break;
    }
    
    return result;
}

std::vector<string>
Command::types()
{
    std::vector<string> result;
    
    for (auto &it : args) {
        
        if (it.hidden) continue;
        
        if (std::find(result.begin(), result.end(), it.type) == result.end()) {
            result.push_back(it.type);
        }
    }
    
    return result;
}

std::vector<Command *>
Command::filterType(const string& type)
{
    std::vector<Command *> result;
    
    for (auto &it : args) {
        
        if (it.hidden) continue;
        if (it.type == type) result.push_back(&it);
    }
    
    return result;
}
std::vector<Command *>
Command::filterPrefix(const string& prefix)
{
    std::vector<Command *> result;
    
    for (auto &it : args) {
        if (it.hidden) continue;
        if (it.token.substr(0, prefix.size()) == prefix) result.push_back(&it);
    }

    return result;
}

void
Command::autoComplete(string& token)
{
    auto matches = filterPrefix(token);
    if (!matches.empty()) {
        
        Command *first = matches.front();
        for (usize i = token.size(); i < first->token.size(); i++) {
            
            for (auto m: matches) {
                if (m->token.size() <= i || m->token[i] != first->token[i]) {
                    return;
                }
            }
            token += first->token[i];
        }
    }
}

string
Command::tokens()
{
    string result = this->parent ? this->parent->tokens() : "";
    return result == "" ? token : result + " " + token;
}

string
Command::usage()
{
    string firstArg, otherArgs;
    
    if (args.empty()) {

        firstArg = numArgs == 0 ? "" : numArgs == 1 ? "<value>" : "<values>";

    } else {
        
        // Collect all argument types
        auto t = types();
        
        // Describe the first argument
        for (usize i = 0; i < t.size(); i++) {
            firstArg += (i == 0 ? "" : "|") + t[i];
        }
        firstArg = "<" + firstArg + ">";
        
        // Describe the remaining arguments (if any)
        bool printArg = false, printOpt = false;
        for (auto &it : args) {
            if (it.func != nullptr && it.numArgs == 0) printOpt = true;
            if (it.numArgs > 0 || !it.args.empty()) printArg = true;
        }
        if (printArg) {
            otherArgs = printOpt ? "[<arguments>]" : "<arguments>";
        }
    }
    
    return tokens() + " " + firstArg + " " + otherArgs;
}

}
