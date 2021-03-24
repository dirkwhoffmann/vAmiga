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

RetroShell::RetroShell(Amiga& ref) : AmigaComponent(ref), interpreter(ref)
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

/*
void
RetroShell::replace(const string& text, const string& prefix)
{
    storage.back() = prefix + text;
}
*/

void
RetroShell::pressUp()
{
    if (ipos == (isize)input.size() - 1) {
        lastInput() = lastLine().substr(cposMin);
    }
    
    if (ipos > 0) ipos--;
    if (ipos < (isize)input.size()) lastLine() = prompt + input[ipos];
}

void
RetroShell::pressDown()
{
    if (ipos + 1 < (isize)input.size()) ipos++;
    if (ipos < (isize)input.size()) lastLine() = prompt + input[ipos];
}

void
RetroShell::pressLeft()
{
    cpos = std::max(cpos - 1, cposMin);
}

void
RetroShell::pressRight()
{
    cpos = std::min(cpos + 1, (isize)lastLine().size());
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
RetroShell::pressBackspace()
{
    if (cpos > cposMin) {
        lastLine().erase(lastLine().begin() + --cpos);
    }
}

void
RetroShell::pressDelete()
{
    if (cpos < (isize)lastLine().size()) {
        lastLine().erase(lastLine().begin() + cpos);
    }
}

void
RetroShell::pressReturn()
{
    // Get the last line without the prompt
    string command = lastLine().substr(cposMin);
    
    *this << '\n';
    
    // Print help message if there was no input
    if (command.empty()) {
        printHelp();
        printPrompt();
        return;
    }
    
    // Add command to the command history buffer
    input[input.size() - 1] = command;
    input.push_back("");
    ipos = (isize)input.size() - 1;
    
    // Execute the command
    exec(command);
    printPrompt();
}

void
RetroShell::pressKey(char c)
{
    printf("pressKey %c\n", c);
    
    if (isprint(c)) {
                
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
        all += storage[numRows - 1] + " ";        
    }
    
    return all.c_str();
}

bool
RetroShell::exec(const string &command, bool verbose)
{
    bool success = false;
    
    // Print the command string if requested
    if (verbose) *this << command << '\n';
        
    printf("Command: %s\n", command.c_str());
 
    try {
        
        // Hand the command over to the intepreter
        interpreter.exec(command);
        success = true;
               
    } catch (TooFewArgumentsError &err) {
        *this << err.what() << ": Too few arguments";
        *this << '\n';
        
    } catch (TooManyArgumentsError &err) {
        *this << err.what() << ": Too many arguments";
        *this << '\n';
            
    } catch (EnumParseError &err) {
        *this << err.token << " is not a valid key" << '\n';
        *this << "Expected: " << err.expected << '\n';
        
    } catch (ParseNumError &err) {
        *this << err.token << " is not a number" << '\n';

    } catch (ParseBoolError &err) {
        *this << err.token << " must be true or false" << '\n';

    } catch (ParseError &err) {
        *this << err.what() << ": Syntax error";
        *this << '\n';
        
    } catch (ConfigUnsupportedError) {
        *this << "This option is not yet supported.";
        *this << '\n';
        
    } catch (ConfigLockedError &err) {
        *this << "This option is locked because the Amiga is powered on.";
        *this << '\n';
        
    } catch (ConfigArgError &err) {
        *this << "Error: Invalid argument. Expected: " << err.what();
        *this << '\n';
        
    } catch (ConfigFileNotFoundError &err) {
        *this << err.what() << " not found";
        *this << '\n';
        success = true; // Don't break the execution
        
    } catch (ConfigFileReadError &err) {
        *this << "Error: Unable to read file " << err.what();
        *this << '\n';
        
    } catch (VAError &err) {
        *this << err.what();
        *this << '\n';
    }
    
    // Print a new prompt
    // printf("Command: %s\n", command.c_str());
    // *this << string(prompt);
    
    // printPrompt();
    
    return success;
}

void
RetroShell::exec(std::istream &stream)
{
    isize line = 0;
    string command;
        
    while(std::getline(stream, command)) {

        line++;
        printf("Line %zd: %s\n", line, command.c_str());

        // Skip empty lines
        if (command == "") continue;

        // Skip comments
        if (command.substr(0,1) == "#") continue;
        
        // Execute the command
        if (!exec(command, true)) {
            throw my::Exception(command, line);
        }
    }
}

}
