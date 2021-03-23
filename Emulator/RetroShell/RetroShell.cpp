// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RetroShell.h"

namespace va {

RetroShell::RetroShell(Amiga& ref) : AmigaComponent(ref)
{
    // Initialize the text storage
    storage.push_back("");

    // Initialize the input buffer
    input.push_back("");
    
    // Print a startup message
    *this << "Retro shell 0.1, ";
    *this << "Dirk W. Hoffmann, ";
    *this << __DATE__ << " " << __TIME__ << "." << '\n';
    *this << "Linked to vAmiga core ";
    *this << V_MAJOR << '.' << V_MINOR << '.' << V_SUBMINOR << '.' << '\n' << '\n';
    
    printHelp();
    *this << '\n';
    printPrompt();
}

RetroShell&
RetroShell::operator<<(char value)
{
    if (value == '\n') {

        // Newline (appends an empty line)
        storage.push_back("");
        cpos = cposMin = 0;
        shorten();

    } else if (value == '\r') {

        // Carriage return (clears the current line)
        storage.back() = "";
        
    } else {
        
        // Add a single character
        // if (storage.back().length() >= numCols) storage.push_back("");
        storage.back() += value;
    }
    
    shorten();
    return *this;
}

RetroShell&
RetroShell::operator<<(const string& text)
{
    storage.back() += text;
    return *this;
}

RetroShell&
RetroShell::operator<<(int value)
{
    *this << std::to_string(value);
    return *this;
}

RetroShell&
RetroShell::operator<<(long value)
{
    *this << std::to_string(value);
    return *this;
}

isize
RetroShell::cposRel()
{
    isize lineLength = (isize)lastLine().size();
    
    return cpos >= lineLength ? 0 : lineLength - cpos;
}

void
RetroShell::clear()
{
    printf("IMPLEMENTATION MISSING\n");
    assert(false);
}

void
RetroShell::printHelp()
{
    *this << "Press 'TAB' twice for help." << '\n';
}

void
RetroShell::printPrompt()
{
    // Finish the current line (if neccessary)
    if (!lastLine().empty()) *this << '\n';

    // Print the prompt
    *this << prompt;
    cpos = cposMin = prompt.size();
}

void
RetroShell::shorten()
{
    while (storage.size() > 600) {
        
        storage.erase(storage.begin());
    }
}

void
RetroShell::tab(int hpos)
{
    int delta = hpos - (int)storage.back().length();
    for (int i = 0; i < delta; i++) {
        *this << ' ';
    }
}

void
RetroShell::replace(const string& text, const string& prefix)
{
    storage.back() = prefix + text;
}

void
RetroShell::pressUp()
{
    printf("CURSOR UP: TO BE IMPLEMENTED\n");
}

void
RetroShell::pressDown()
{
    printf("CURSOR DOWN: TO BE IMPLEMENTED\n");
}

void
RetroShell::pressLeft()
{
    cpos = std::max(cpos - 1, cposMin);
    // if (cpos > cposMin) cpos--;
    // printf("CURSOR LEFT: %zd\n", cpos);
}

void
RetroShell::pressRight()
{
    cpos = std::min(cpos + 1, (isize)lastLine().size());
    // if (cpos < (isize)lastLine().size()) cpos++;
}

void
RetroShell::pressHome()
{
    cpos = cposMin;
}

void
RetroShell::pressEnd()
{
    cpos = (isize)lastLine().size();
}

void
RetroShell::pressTab()
{
    if (tabPressed) {
        
        // TAB was pressed twice
        printf("TAB TWICE\n");
        // *this << '\n';
        
        // Print the instructions for this command
        printf("TODO: CALL interpreter.help\n");
        // interpreter.help(input[ipos]);
        
        // Repeat the old input string
        // *this << string(prompt) << input[ipos];
        
    } else {
        
        printf("TODO: CALL interpreter.autoComplete\n");
        // interpreter.autoComplete(input[ipos]);
        // cpos = (isize)input[ipos].length();
        // replace(input[ipos]);
    }
}

void
RetroShell::pressReturn()
{
    printf("RETURN key\n");
    
    // Get the last line without the prompt
    string command = lastLine().substr(cposMin);
    
    printf("last: %s\n", lastLine().c_str());
    printf("stripped: '%s'\n", command.c_str());
    
    *this << '\n';
    
    if (input.empty()) {
        printHelp();
    } else {
        input.push_back(command);
        ipos = (isize)input.size() - 1;
    }
    
    // Execute the command
    // exec(command);
    
    printPrompt();
}

void
RetroShell::pressKey(char c)
{
    printf("pressKey %c\n", c);
    
    if (isprint(c)) {
        
        printf("ipos = %zd input.size = %zu\n", ipos, input.size());
        printf("input[%zd] = %s\n", ipos, input[ipos].c_str());
        
        if (cpos < (isize)lastLine().size()) {
            lastLine().insert(lastLine().begin() + cpos, c);
        } else {
            lastLine() += c;
        }
        cpos++;
        
        
        // input[ipos].insert(input[ipos].begin() + cpos++, c);
        // *this << '\r' << string(prompt) << input[ipos];
        isDirty = true;
        printf("END presskey\n");
    }
    
/*
        case '\b':
            
            printf("BACKSPACE key\n");
            
            if (cpos > 0) {
                input[ipos].erase(input[ipos].begin() + --cpos);
            }
            // *this << '\r' << string(prompt) << input[ipos];
            break;
            
        case '\t':
            
*/
            /*
             case sf::Keyboard::Up:
             
             if (ipos > 0) {
             ipos--;
             cpos = (isize)input[ipos].size();
             
             replace(input[ipos]);
             }
             break;
             
             case sf::Keyboard::Down:
             
             if (ipos < input.size() - 1) {
             ipos++;
             cpos = (isize)input[ipos].size();
             
             replace(input[ipos]);
             }
             break;
             
             case sf::Keyboard::Left:
             
             if (cpos > 0) {
             cpos--;
             }
             break;
             
             case sf::Keyboard::Right:
             
             if (cpos < input[ipos].length()) {
             cpos++;
             }
             break;
             
             case sf::Keyboard::Home:
             
             cpos = 0;
             break;
             
             case sf::Keyboard::End:
             
             cpos = (isize)input[ipos].length();
             break;
             */
}

const char *
RetroShell::text()
{
    all = "";
    
    if (auto numRows = storage.size()) {
        
        // Add all rows except the last one
        for (usize i = 0; i < numRows - 1; i++) all += storage[i] + "\n";
        
        // Add the last row
        all += storage[numRows - 1] + "_";
        
        // Add a space for the cursor if neccessary
        // printf("cpos = %zd size = %zd\n", cpos, (isize)storage[numRows - 1].size());
        // if (cpos >= (isize)storage[numRows - 1].size()) all += "_";
    }
    
    return all.c_str();
}

}
