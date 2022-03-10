// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Headless.h"
#include "Script.h"
#include <getopt.h>

int main(int argc, char *argv[])
{
    try {
        
        Headless().main(argc, argv);
        
    } catch (SyntaxError &e) {
        
        std::cout << "Usage: ";
        std::cout << "vAmigaCore [-v] <script>" << std::endl;
        std::cout << std::endl;
        std::cout << "       -v or --verbose  Print additional information" << std::endl;
        std::cout << std::endl;
        
        if (auto what = string(e.what()); !what.empty()) {
            std::cout << what << std::endl;
        }
        
        return 1;

    } catch (VAError &e) {

        std::cout << "Error" << std::endl;
        std::cout << e.what() << std::endl;
        return 1;
        
    } catch (std::exception &e) {

        std::cout << "Error" << std::endl;
        std::cout << e.what() << std::endl;
        return 1;
    
    } catch (...) {
    
        std::cout << "Error ..." << std::endl;
    }
    
    return 0;
}

void
process(const void *listener, long type, u32 data1, u32 data2)
{
    ((Headless *)listener)->process(type, data1, data2);
}

void
Headless::process(long type, u32 data1, u32 data2)
{
    if (verbose) {
        
        std::cout << MsgTypeEnum::key(type);
        std::cout << "(" << data1 << ", " << data2 << ")" << std::endl;
    }
        
    switch (type) {
            
        case MSG_SCRIPT_WAKEUP:

            barrier.unlock();
            break;
            
        case MSG_SCRIPT_DONE:
        case MSG_SCRIPT_ABORT:
            
            amiga.halt();
            std::cout << "Bye" << std::endl;
            exit(0);

        default:
            break;
    }
}

void
Headless::main(int argc, char *argv[])
{
    map<string,string> keys;
    string option;

    std::cout << "vAmiga Headless v" << amiga.version();
    std::cout << " - (C)opyright Dirk W. Hoffmann" << std::endl << std::endl;

    // Parse all command line arguments
    parseArguments(argc, argv, keys);
    
    // Enter the main execution function
    run(keys);
}

void
Headless::run(map<string,string> &keys)
{
    // Parse command line arguments
    verbose = keys.find("verbose") != keys.end();

    // Read script file
    Script script(inputs[0]);

    // Redirect shell output to the console
    amiga.retroShell.setStream(std::cout);
    
    // Register message receiver
    amiga.msgQueue.setListener(this, ::process);
    
    // Execute script
    barrier.lock();
    script.execute(amiga);

    while (1) {
        
        barrier.lock();
        amiga.retroShell.continueScript();
    }
}

void
Headless::parseArguments(int argc, char *argv[], map<string,string> &keys)
{
    static struct option long_options[] = {
        
        { "verbose",    no_argument,    NULL,   'v' },
        { NULL,         0,              NULL,    0  }
    };
    
    // Don't print the default error messages
    opterr = 0;
    
    // Remember the execution path
    keys["exec"] = util::makeAbsolutePath(argv[0]);

    // Parse all options
    while (1) {
        
        int arg = getopt_long(argc, argv, ":v:o:", long_options, NULL);
        if (arg == -1) break;

        switch (arg) {
                
            case 'v':
                keys["verbose"] = "1";
                break;
                
            case ':':
                throw SyntaxError("Missing argument for option '" +
                                  string(argv[optind - 1]) + "'");
                
            default:
                throw SyntaxError("Invalid option '" +
                                  string(argv[optind - 1]) + "'");
        }
    }
    
    // Parse all remaining arguments
    while (optind < argc) {
        inputs.push_back(util::makeAbsolutePath(argv[optind++]));
    }
    
    checkArguments(keys);
}

void
Headless::checkArguments(map<string,string> &keys)
{
    // The user needs to specify a single input file
    if (inputs.size() < 1) throw SyntaxError("No script file is given");
    if (inputs.size() > 1) throw SyntaxError("More than one script file is given");
    
    // The input file must exist
    if (!util::fileExists(inputs[0])) {
        throw SyntaxError("File " + inputs[0] + " does not exist");
    }
}
