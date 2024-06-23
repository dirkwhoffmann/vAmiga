// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
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
        
        std::cout << "Usage: vAmigaCore [-csvm] | { [-vm] <script> } " << std::endl;
        std::cout << std::endl;
        std::cout << "       -c or --check     Checks the integrity of the build" << std::endl;
        std::cout << "       -s or --size      Reports the size of certain objects" << std::endl;
        std::cout << "       -v or --verbose   Print executed script lines" << std::endl;
        std::cout << "       -m or --messages  Observe the message queue" << std::endl;
        std::cout << std::endl;
        
        if (auto what = string(e.what()); !what.empty()) {
            std::cout << what << std::endl;
        }
        
        return 1;

    } catch (vamiga::Error &e) {

        std::cout << "Error: " << std::endl;
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

Headless::Headless() { // }: amiga(&vamiga.emu->main) {

}

int
Headless::main(int argc, char *argv[])
{
    std::cout << "vAmiga Headless v" << VAmiga::version();
    std::cout << " - (C)opyright Dirk W. Hoffmann" << std::endl << std::endl;

    // Parse all command line arguments
    parseArguments(argc, argv);

    // Check for the --size option
    if (keys.find("size") != keys.end()) {

        reportSize();
        return 0;

    } else {

        return execScript();
    }
}

#ifdef _WIN32

void
Headless::parseArguments(int argc, char *argv[])
{
    keys["check"] = "1";
    keys["size"] = "1";
    keys["verbose"] = "1";
    keys["arg1"] = selfTestScript();
}

#else

void
Headless::parseArguments(int argc, char *argv[])
{
    static struct option long_options[] = {
        
        { "check",      no_argument,    NULL,   'c' },
        { "size",       no_argument,    NULL,   's' },
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
        
        int arg = getopt_long(argc, argv, ":csvm", long_options, NULL);
        if (arg == -1) break;

        switch (arg) {
                
            case 'c':
                keys["check"] = "1";
                break;

            case 's':
                keys["size"] = "1";
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
    if (keys.find("check") != keys.end()) keys["arg1"] = selfTestScript();
}

#endif

void
Headless::checkArguments()
{
    if (keys.find("check") != keys.end() || keys.find("size") != keys.end()) {

        // No input file must be given
        if (keys.find("arg1") != keys.end()) {
            throw SyntaxError("No script file must be given");
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
            barrier.unlock();
            break;

        case MSG_SCRIPT_ABORT:
        case MSG_ABORT:

            returnCode = 1;
            barrier.unlock();
            break;

        default:
            break;
    }
}

void 
Headless::reportSize()
{
    msg("             Amiga : %zu bytes\n", sizeof(Amiga));
    msg("             Agnus : %zu bytes\n", sizeof(Agnus));
    msg("       AudioFilter : %zu bytes\n", sizeof(AudioFilter));
    msg("               CIA : %zu bytes\n", sizeof(CIA));
    msg("       ControlPort : %zu bytes\n", sizeof(ControlPort));
    msg("               CPU : %zu bytes\n", sizeof(CPU));
    msg("            Denise : %zu bytes\n", sizeof(Denise));
    msg("             Drive : %zu bytes\n", sizeof(FloppyDrive));
    msg("          Keyboard : %zu bytes\n", sizeof(Keyboard));
    msg("            Memory : %zu bytes\n", sizeof(Memory));
    msg("moira::Breakpoints : %zu bytes\n", sizeof(moira::Breakpoints));
    msg("moira::Watchpoints : %zu bytes\n", sizeof(moira::Watchpoints));
    msg("   moira::Debugger : %zu bytes\n", sizeof(moira::Debugger));
    msg("      moira::Moira : %zu bytes\n", sizeof(moira::Moira));
    msg("             Muxer : %zu bytes\n", sizeof(Muxer));
    msg("             Paula : %zu bytes\n", sizeof(Paula));
    msg("       PixelEngine : %zu bytes\n", sizeof(PixelEngine));
    msg("     RemoteManager : %zu bytes\n", sizeof(RemoteManager));
    msg("               RTC : %zu bytes\n", sizeof(RTC));
    msg("        RetroShell : %zu bytes\n", sizeof(RetroShell));
    msg("           Sampler : %zu bytes\n", sizeof(Sampler));
    msg("        SerialPort : %zu bytes\n", sizeof(SerialPort));
    msg("            Volume : %zu bytes\n", sizeof(Volume));
    msg("             Zorro : %zu bytes\n", sizeof(ZorroManager));
    msg("\n");
}

int
Headless::execScript()
{
    // Create an emulator instance
    VAmiga vamiga;
    
    // Redirect shell output to the console in verbose mode
    if (keys.find("verbose") != keys.end()) vamiga.amiga.amiga->retroShell.setStream(std::cout);

    // Read the input script
    Script script(keys["arg1"]);

    // Launch the emulator thread
    vamiga.launch(this, vamiga::process);

    // Execute the script
    barrier.lock();
    script.execute(*vamiga.amiga.amiga);
    
    return *returnCode;
}

}
