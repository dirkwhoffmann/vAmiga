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
#include "Amiga.h"
#include "Script.h"
#include <filesystem>
#include <chrono>

int main(int argc, char *argv[])
{
    try {
        
        return vamiga::Headless().main(argc, argv);
        
    } catch (vamiga::SyntaxError &e) {
        
        std::cout << "Usage: vAmigaCore [-svm] [<script>]" << std::endl;
        std::cout << std::endl;
        std::cout << "       -s or --size      Reports the size of certain objects" << std::endl;
        std::cout << "       -v or --verbose   Print executed script lines" << std::endl;
        std::cout << "       -m or --messages  Observe the message queue" << std::endl;
        std::cout << "       <script>          Execute this script instead of the default" << std::endl;
        std::cout << std::endl;
        
        if (auto what = string(e.what()); !what.empty()) {
            std::cout << what << std::endl;
        }
        return 1;

    } catch (vamiga::Error &e) {

        std::cout << "VAError: " << e.what() << std::endl;
        return 1;
        
    } catch (std::exception &e) {

        std::cout << "System Error: " << e.what() << std::endl;
        return 1;
    
    } catch (...) {
    
        std::cout << "Error" << std::endl;
        return 1;
    }
    
    return 0;
}

namespace vamiga {

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

void
Headless::parseArguments(int argc, char *argv[])
{
    // Remember the execution path
    keys["exec"] = std::filesystem::absolute(std::filesystem::path(argv[0])).string();

    // Parse command line arguments
    for (isize i = 1, n = 1; i < argc; i++) {

        auto arg = string(argv[i]);

        if (arg[0] == '-') {

            if (arg == "-s" || arg == "--size")     { keys["size"] = "1"; continue; }
            if (arg == "-v" || arg == "--verbose")  { keys["verbose"] = "1"; continue; }
            if (arg == "-m" || arg == "--messages") { keys["messages"] = "1"; continue; }

            throw SyntaxError("Invalid option '" + arg + "'");
        }

        auto path = std::filesystem::path(arg);
        keys["arg" + std::to_string(n++)] = std::filesystem::absolute(path).string();
    }

    // Check for syntax errors
    checkArguments();

    // Create the selftest script if no custom script is specified
    if (keys.find("arg1") == keys.end()) keys["arg1"] = selfTestScript();
}

void
Headless::checkArguments()
{
    // At most one file must be specified
    if (keys.find("arg2") != keys.end()) {
        throw SyntaxError("More than one script file is given");
    }

    // The input file must exist
    if (keys.find("arg1") != keys.end() && !util::fileExists(keys["arg1"])) {
        throw SyntaxError("File " + keys["arg1"] + " does not exist");
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
            
        case MSG_RSH_EXEC:

            returnCode = 0;
            barrier.unlock();
            break;

        case MSG_RSH_ERROR:
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
    msg("         AudioPort : %zu bytes\n", sizeof(AudioPort));
    msg("             Paula : %zu bytes\n", sizeof(Paula));
    msg("       PixelEngine : %zu bytes\n", sizeof(PixelEngine));
    msg("     RemoteManager : %zu bytes\n", sizeof(RemoteManager));
    msg("               RTC : %zu bytes\n", sizeof(RTC));
    msg("        RetroShell : %zu bytes\n", sizeof(RetroShell));
    msg("           Sampler : %zu bytes\n", sizeof(Sampler));
    msg("        SerialPort : %zu bytes\n", sizeof(SerialPort));
    msg("             Zorro : %zu bytes\n", sizeof(ZorroManager));
    msg("\n");
}

int
Headless::execScript()
{
    // Create an emulator instance
    VAmiga vamiga;
    
    // Redirect shell output to the console in verbose mode
    if (keys.find("verbose") != keys.end()) vamiga.retroShell.setStream(std::cout);

    // Read the input script
    Script script(keys["arg1"]);

    // Launch the emulator thread
    vamiga.launch(this, vamiga::process);

    // Execute script
    barrier.lock();
    vamiga.retroShell.execScript(script);
    barrier.lock();

    return *returnCode;
}

}
