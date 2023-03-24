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
#include <filesystem>
#include <chrono>

#ifndef _WIN32
#include <getopt.h>
#endif

int main(int argc, char *argv[])
{
    try {
        
        return vamiga::Headless().main(argc, argv);
        
    } catch (vamiga::SyntaxError &e) {
        
        std::cout << "Usage: vAmigaCore [-svm] | { [-vm] <script> } " << std::endl;
        std::cout << std::endl;
        std::cout << "       -s or --selftest  Checks the integrity of the build" << std::endl;
        std::cout << "       -v or --verbose   Print executed script lines" << std::endl;
        std::cout << "       -m or --messages  Observe the message queue" << std::endl;
        std::cout << std::endl;
        
        if (auto what = string(e.what()); !what.empty()) {
            std::cout << what << std::endl;
        }
        
        return 1;

    } catch (vamiga::VAError &e) {

        std::cout << "VAError: " << std::endl;
        std::cout << e.what() << std::endl;
        return 1;
        
    } catch (std::exception &e) {

        std::cout << "Error: " << std::endl;
        std::cout << e.what() << std::endl;
        return 1;
    
    } catch (...) {
    
        std::cout << "Error" << std::endl;
    }
    
    return 0;
}

namespace vamiga {

int
Headless::main(int argc, char *argv[])
{
    std::cout << "vAmiga Headless v" << amiga.version();
    std::cout << " - (C)opyright Dirk W. Hoffmann" << std::endl << std::endl;

    // Parse all command line arguments
    parseArguments(argc, argv);

    // Redirect shell output to the console in verbose mode
    if (keys.find("verbose") != keys.end()) amiga.retroShell.setStream(std::cout);

    // Read the input script
    Script script(keys["arg1"]);
    
    // Register message receiver
    // amiga.msgQueue.setListener(this, vamiga::process);

    // Launch the emulator thread
    amiga.launch(this, vamiga::process);

    // Execute the script
    barrier.lock();
    script.execute(amiga);

    while (!returnCode) {
        
        barrier.lock();
        amiga.retroShell.continueScript();
    }

    return *returnCode;
}

#ifdef _WIN32

void
Headless::parseArguments(int argc, char *argv[])
{
    keys["selftest"] = "1";
    keys["verbose"] = "1";
    keys["arg1"] = selfTestScript();
}

#else

void
Headless::parseArguments(int argc, char *argv[])
{
    static struct option long_options[] = {
        
        { "selftest",   no_argument,    NULL,   's' },
        { "verbose",    no_argument,    NULL,   'v' },
        { "messages",   no_argument,    NULL,   'm' },
        { NULL,         0,              NULL,    0  }
    };
    
    // Don't print the default error messages
    opterr = 0;
    
    // Remember the execution path
    keys["exec"] = util::makeAbsolutePath(argv[0]);

    // Parse all options
    while (1) {
        
        int arg = getopt_long(argc, argv, ":svm", long_options, NULL);
        if (arg == -1) break;

        switch (arg) {
                
            case 's':
                keys["selftest"] = "1";
                break;

            case 'v':
                keys["verbose"] = "1";
                break;

            case 'm':
                keys["messages"] = "1";
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
    auto nr = 1;
    while (optind < argc) {
        keys["arg" + std::to_string(nr++)] = util::makeAbsolutePath(argv[optind++]);
    }

    // Check for syntax errors
    checkArguments();

    // Create the selftest script if needed
    if (keys.find("selftest") != keys.end()) keys["arg1"] = selfTestScript();
}

#endif

void
Headless::checkArguments()
{
    if (keys.find("selftest") != keys.end()) {

        // No input file must be given
        if (keys.find("arg1") != keys.end()) {
            throw SyntaxError("No script file must be given in selftest mode");
        }

    } else {

        // The user needs to specify a single input file
        if (keys.find("arg1") == keys.end()) {
            throw SyntaxError("No script file is given");
        }
        if (keys.find("arg2") != keys.end()) {
            throw SyntaxError("More than one script file is given");
        }

        // The input file must exist
        if (!util::fileExists(keys["arg1"])) {
            throw SyntaxError("File " + keys["arg1"] + " does not exist");
        }
    }
}

string
Headless::selfTestScript()
{
    auto path = std::filesystem::temp_directory_path() / "selftest.ini";
    auto file = std::ofstream(path);

    for (isize i = 0; i < isizeof(script) / isizeof(const char *); i++) {
        file << script[i] << std::endl;
    }
    return path.string();
}

void
process(const void *listener, Message msg)
{
    ((Headless *)listener)->process(msg);
}

void
Headless::process(Message msg)
{
    static bool messages = keys.find("messages") != keys.end();
    
    if (messages) {
        
        std::cout << MsgTypeEnum::key(msg.type);
        std::cout << "(" << msg.value << ")";
        std::cout << std::endl;
    }

    switch (msg.type) {
            
        case MSG_SCRIPT_DONE:

            returnCode = 0;
            break;

        case MSG_SCRIPT_ABORT:
        case MSG_ABORT:

            returnCode = 1;
            break;

        case MSG_SCRIPT_PAUSE:

            std::this_thread::sleep_for(std::chrono::seconds(msg.script.delay));
            break;

        default:
            break;
    }

    barrier.unlock();
}

}
