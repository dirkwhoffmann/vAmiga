// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#include "config.h"
#include "Headless.h"
#include "Amiga.h"
#include "Script.h"
#include "DiagRom.h"
#include <chrono>

int main(int argc, char *argv[])
{
    try {
        
        return vamiga::Headless().main(argc, argv);
        
    } catch (vamiga::SyntaxError &e) {
        
        std::cout << "Usage: VAmigaHeadless [-fsdvm] [<script>]" << std::endl;
        std::cout << std::endl;
        std::cout << "       -f or --footprint   Report the size of objects" << std::endl;
        std::cout << "       -s or --smoke       Run smoke tests to test the build" << std::endl;
        std::cout << "       -d or --diagnose    Run DiagRom in the background" << std::endl;
        std::cout << "       -v or --verbose     Print the executed script lines" << std::endl;
        std::cout << "       -m or --messages    Observe the message queue" << std::endl;
        std::cout << "       <script>            Execute a custom script" << std::endl;
        std::cout << std::endl;
        
        if (auto what = std::string(e.what()); !what.empty()) {
            std::cout << what << std::endl;
        }

    } catch (vamiga::AppError &e) {

        std::cout << "Emulator Error: " << e.what() << std::endl;

    } catch (std::exception &e) {

        std::cout << "System Error: " << e.what() << std::endl;

    } catch (...) {
    
        std::cout << "Error" << std::endl;
    }
    
    return 1;
}

namespace vamiga {

int
Headless::main(int argc, char *argv[])
{
    std::cout << "vAmiga Headless v" << VAmiga::version();
    std::cout << " - (C)opyright Dirk W. Hoffmann" << std::endl << std::endl;

    // Parse all command line arguments
    parseArguments(argc, argv);

    // Check options
    if (keys.find("footprint") != keys.end())   { reportSize(); }
    if (keys.find("smoke") != keys.end())       { runScript(smokeTestScript); }
    if (keys.find("diagnose") != keys.end())    { runScript(selfTestScript); }
    if (keys.find("arg1") != keys.end())        { runScript(keys["arg1"]); }

    return returnCode;
}

void
Headless::parseArguments(int argc, char *argv[])
{
    // Remember the execution path
    keys["exec"] = fs::absolute(fs::path(argv[0])).string();

    // Parse command line arguments
    for (isize i = 1, n = 1; i < argc; i++) {

        auto arg = string(argv[i]);

        if (arg[0] == '-') {

            if (arg == "-f" || arg == "--footprint") { keys["footprint"] = "1"; continue; }
            if (arg == "-s" || arg == "--smoke")     { keys["smoke"] = "1"; continue; }
            if (arg == "-d" || arg == "--diagnose")  { keys["diagnose"] = "1"; continue; }
            if (arg == "-v" || arg == "--verbose")   { keys["verbose"] = "1"; continue; }
            if (arg == "-m" || arg == "--messages")  { keys["messages"] = "1"; continue; }

            throw SyntaxError("Invalid option '" + arg + "'");
        }

        auto path = fs::path(arg);
        keys["arg" + std::to_string(n++)] = fs::absolute(path).string();
    }

    // Check for syntax errors
    checkArguments();
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

void
Headless::runScript(const char **script)
{
    auto path = fs::temp_directory_path() / "script.retrosh";
    auto file = std::ofstream(path, std::ios::binary);

    for (isize i = 0; script[i] != nullptr; i++) {
        file << script[i] << std::endl;
    }
    runScript(path);
}

void
Headless::runScript(const fs::path &path)
{
    // Read the input script
    Script script(path);

    // Create an emulator instance
    VAmiga vamiga;

    // Plug in DiagRom
    vamiga.mem.loadRom(diagROM13, sizeofDiagRom13);

    // Redirect shell output to the console in verbose mode
    if (keys.find("verbose") != keys.end()) vamiga.retroShell.setStream(std::cout);

    // Launch the emulator thread
    vamiga.launch(this, vamiga::process);

    // Execute script
    const auto timeout = util::Time::seconds(500.0);
    vamiga.retroShell.execScript(script);
    waitForWakeUp(timeout);
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
        
        std::cout << MsgEnum::key(msg.type);
        std::cout << "(" << msg.value << ")";
        std::cout << std::endl;
    }

    switch (msg.type) {
            
        case Msg::RSH_ERROR:

            returnCode = 1;
            wakeUp();
            break;

        case Msg::ABORT:

            wakeUp();
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

const char *
Headless::selfTestScript[] = {

    "# vAmiga Self Check",
    "# ",
    "# Dirk W. Hoffmann, 2024",
    "",
    "# Attach RetroShell to the serial port to report debug output",
    "serial set DEVICE RETROSHELL",
    "",
    "# Power up the emulator",
    "amiga power on",
    "",
    "# Let DiagRom launch and press the right mouse button after a while",
    "wait 15 seconds",
    "mouse1 press right",
    ""
    "# Let DiagRom run for some more time",
    "wait 10 seconds",
    "",
    "# Enough testing. Terminate the application",
    "shutdown",

    nullptr
};

const char *
Headless::smokeTestScript[] = {

    "# RetroShell Syntax Check",
    "# ",
    "# This script is executed in nightly-builts to check the integrity of ",
    "# the compiled application. It runs several shell commands to check if ",
    "# anything breaks.",
    "# ",
    "# Dirk W. Hoffmann, 2024",

    "",
    "help",
    "hel",          // Test auto-completion
    "joshua",       // Test a hidden command
    "try joshu",    // Catch an error with 'try'

#ifndef _WIN32
    "",
    "regression setup A1000_OCS_1MB",
    "regression setup A500_OCS_1MB",
    "regression setup A500_ECS_1MB",
    "regression setup A500_PLUS_1MB",
#endif

    "",
    "amiga",
    "amiga defaults",
    "amiga set VIDEO_FORMAT PAL",
    "amiga set VIDEO_FORMAT NTSC",
    "amiga set WARP_BOOT 0",
    "amiga set WARP_BOOT 10",
    "amiga set WARP_MODE ALWAYS",
    "amiga set WARP_MODE AUTO",
    "amiga set WARP_MODE NEVER",
    
    "amiga init A1000_OCS_1MB",
    "amiga init A500_OCS_1MB",
    "amiga init A500_ECS_1MB",
    "amiga init A500_PLUS_1MB",
    "amiga power off",
    "amiga reset",

    "",
    "mem",
    "mem set CHIP_RAM 256",
    "mem set CHIP_RAM 512",
    "mem set CHIP_RAM 1024",
    "mem set SLOW_RAM 0",
    "mem set SLOW_RAM 512",
    "mem set FAST_RAM 0",
    "mem set FAST_RAM 256",
    "mem set FAST_RAM 512",
    "mem set FAST_RAM 1024",
    "mem set FAST_RAM 2048",
    "mem set FAST_RAM 4096",
    "mem set FAST_RAM 8192",
    "mem set EXT_START 0xE0",
    "mem set EXT_START 0xF0",
    "mem set SAVE_ROMS true",
    "mem set SAVE_ROMS false",
    "mem set SLOW_RAM_DELAY true",
    "mem set SLOW_RAM_DELAY false",
    "mem set SLOW_RAM_MIRROR true",
    "mem set SLOW_RAM_MIRROR false",
    "mem set BANKMAP A500",
    "mem set BANKMAP A1000",
    "mem set BANKMAP A2000A",
    "mem set BANKMAP A2000B",
    "mem set UNMAPPING_TYPE FLOATING",
    "mem set UNMAPPING_TYPE ALL_ZEROES",
    "mem set UNMAPPING_TYPE ALL_ONES",
    "mem set RAM_INIT_PATTERN ALL_ZEROES",
    "mem set RAM_INIT_PATTERN ALL_ONES",
    "mem set RAM_INIT_PATTERN RANDOMIZED",

    "",
    "cpu",
    "cpu set REVISION 68000",
    "cpu set REVISION 68010",
    "cpu set REVISION 68EC020",
    "cpu set DASM_REVISION 68000",
    "cpu set DASM_REVISION 68010",
    "cpu set DASM_REVISION 68EC020",
    "cpu set DASM_REVISION 68020",
    "cpu set DASM_REVISION 68EC030",
    "cpu set DASM_REVISION 68030",
    "cpu set DASM_REVISION 68EC040",
    "cpu set DASM_REVISION 68LC040",
    "cpu set DASM_REVISION 68040",
    "cpu set DASM_SYNTAX MOIRA",
    "cpu set DASM_SYNTAX MOIRA_MIT",
    "cpu set DASM_SYNTAX GNU",
    "cpu set DASM_SYNTAX GNU_MIT",
    "cpu set DASM_SYNTAX MUSASHI",
    "cpu set OVERCLOCKING 0",
    "cpu set OVERCLOCKING 1",
    "cpu set OVERCLOCKING 2",
    "cpu set RESET_VAL 0",

    "",
    "ciaa",
    "ciaa set REVISION MOS_8520_DIP",
    "ciaa set REVISION MOS_8520_PLCC",
    "ciaa set TODBUG true",
    "ciaa set TODBUG false",
    "ciaa set ECLOCK_SYNCING true",
    "ciaa set ECLOCK_SYNCING false",
    "ciaa set IDLE_SLEEP true",
    "ciaa set IDLE_SLEEP false",

    "",
    "ciab",
    "ciab set REVISION MOS_8520_DIP",
    "ciab set REVISION MOS_8520_PLCC",
    "ciab set TODBUG true",
    "ciab set TODBUG false",
    "ciab set ECLOCK_SYNCING true",
    "ciab set ECLOCK_SYNCING false",
    "ciab set IDLE_SLEEP true",
    "ciab set IDLE_SLEEP false",


    "",
    "agnus",
    "agnus set REVISION OCS_OLD",
    "agnus set REVISION OCS",
    "agnus set REVISION ECS_1MB",
    "agnus set REVISION ECS_2MB",
    "agnus set PTR_DROPS true",
    "agnus set PTR_DROPS false",

    "blitter",
    "blitter set ACCURACY 0",
    "blitter set ACCURACY 1",
    "blitter set ACCURACY 2",

    "denise",
    "denise set REVISION OCS",
    "denise set REVISION ECS",
    "denise set VIEWPORT_TRACKING true",
    "denise set VIEWPORT_TRACKING false",
    "denise set CLX_SPR_SPR true",
    "denise set CLX_SPR_SPR false",
    "denise set CLX_SPR_PLF true",
    "denise set CLX_SPR_PLF false",
    "denise set CLX_PLF_PLF true",
    "denise set CLX_PLF_PLF false",
    "denise set HIDDEN_BITPLANES 0",
    "denise set HIDDEN_BITPLANES 0xFF",
    "denise set HIDDEN_SPRITES 0",
    "denise set HIDDEN_SPRITES 0xFF",
    "denise set HIDDEN_LAYERS 0",
    "denise set HIDDEN_LAYERS 0xFF",

    "dmadebugger open",
    "dmadebugger close",
    "dmadebugger set DEBUG_CHANNEL0 true",
    "dmadebugger set DEBUG_CHANNEL0 false",
    "dmadebugger set DEBUG_CHANNEL1 true",
    "dmadebugger set DEBUG_CHANNEL1 false",
    "dmadebugger set DEBUG_CHANNEL2 true",
    "dmadebugger set DEBUG_CHANNEL2 false",
    "dmadebugger set DEBUG_CHANNEL3 true",
    "dmadebugger set DEBUG_CHANNEL3 false",
    "dmadebugger set DEBUG_CHANNEL4 true",
    "dmadebugger set DEBUG_CHANNEL4 false",
    "dmadebugger set DEBUG_CHANNEL5 true",
    "dmadebugger set DEBUG_CHANNEL5 false",
    "dmadebugger set DEBUG_CHANNEL6 true",
    "dmadebugger set DEBUG_CHANNEL6 false",
    "dmadebugger set DEBUG_CHANNEL7 true",
    "dmadebugger set DEBUG_CHANNEL7 false",

    "monitor",
    "monitor set PALETTE COLOR",
    "monitor set PALETTE BLACK_WHITE",
    "monitor set PALETTE PAPER_WHITE",
    "monitor set PALETTE GREEN",
    "monitor set PALETTE AMBER",
    "monitor set PALETTE SEPIA",
    "monitor set BRIGHTNESS 50",
    "monitor set CONTRAST 50",
    "monitor set SATURATION 50",

    "audio",
    "audio filter",
    "audio filter set FILTER_TYPE NONE",
    "audio filter set FILTER_TYPE A500",
    "audio filter set FILTER_TYPE A1000",
    "audio filter set FILTER_TYPE A1200",
    "audio filter set FILTER_TYPE LOW",
    "audio filter set FILTER_TYPE LED",
    "audio filter set FILTER_TYPE HIGH",
    "audio set SAMPLING_METHOD NONE",
    "audio set SAMPLING_METHOD NEAREST",
    "audio set SAMPLING_METHOD LINEAR",
    "audio set VOL0 50",
    "audio set VOL1 50",
    "audio set VOL2 50",
    "audio set VOL3 50",
    "audio set VOLL 50",
    "audio set VOLR 50",
    "audio set PAN0 50",
    "audio set PAN1 50",
    "audio set PAN2 50",
    "audio set PAN3 50",

    "paula dc",
    "paula dc set SPEED 2",
    "paula dc set AUTO_DSKSYNC true",
    "paula dc set AUTO_DSKSYNC false",
    "paula dc set LOCK_DSKSYNC true",
    "paula dc set LOCK_DSKSYNC false",

    "rtc",
    "rtc set MODEL NONE",
    "rtc set MODEL OKI",
    "rtc set MODEL RICOH",

    "keyboard",
    "keyboard set ACCURACY 0",
    "keyboard set ACCURACY 1",
    "keyboard press 32",

    "joystick1",
    "joystick1 set AUTOFIRE true",
    "joystick1 set AUTOFIRE false",
    "joystick1 set AUTOFIRE_BULLETS 10",
    "joystick1 set AUTOFIRE_DELAY 10",
    "joystick1 press 1",
    "joystick1 press 2",
    "joystick1 press 3",
    "joystick1 unpress 1",
    "joystick1 unpress 2",
    "joystick1 unpress 3",
    "joystick1 pull left",
    "joystick1 pull right",
    "joystick1 pull up",
    "joystick1 pull down",
    "joystick1 release x",
    "joystick1 release y",

    "joystick2",
    "joystick2 set AUTOFIRE true",
    "joystick2 set AUTOFIRE false",
    "joystick2 set AUTOFIRE_BULLETS 10",
    "joystick2 set AUTOFIRE_DELAY 10",
    "joystick2 press 1",
    "joystick2 press 2",
    "joystick2 press 3",
    "joystick2 unpress 1",
    "joystick2 unpress 2",
    "joystick2 unpress 3",
    "joystick2 pull left",
    "joystick2 pull right",
    "joystick2 pull up",
    "joystick2 pull down",
    "joystick2 release x",
    "joystick2 release y",

    "mouse1",
    "mouse1 set PULLUP_RESISTORS true",
    "mouse1 set PULLUP_RESISTORS false",
    "mouse1 set SHAKE_DETECTION true",
    "mouse1 set SHAKE_DETECTION false",
    "mouse1 set VELOCITY 50",
    "mouse1 press left",
    "mouse1 press middle",
    "mouse1 press right",

    "mouse2",
    "mouse2 set PULLUP_RESISTORS true",
    "mouse2 set PULLUP_RESISTORS false",
    "mouse2 set SHAKE_DETECTION true",
    "mouse2 set SHAKE_DETECTION false",
    "mouse2 set VELOCITY 50",
    "mouse2 press left",
    "mouse2 press middle",
    "mouse2 press right",

    "serial",
    "serial set DEVICE NONE",
    "serial set DEVICE NULLMODEM",
    "serial set DEVICE LOOPBACK",
    "serial set DEVICE RETROSHELL",

    "df0",
    "df0 eject",
    "df0 set TYPE DD_35",
    "df0 set RPM 300",
    "df0 set MECHANICS NONE",
    "df0 set MECHANICS A1010",
    "df0 set SWAP_DELAY 2",
    "df0 set PAN 50",
    "df0 set STEP_VOLUME 50",
    "df0 set POLL_VOLUME 50",
    "df0 set INSERT_VOLUME 50",
    "df0 set EJECT_VOLUME 50",

    "df1",
    "df1 eject",
    "df1 set CONNECT true",
    "df1 set TYPE DD_35",
    "df1 set RPM 300",
    "df1 set MECHANICS NONE",
    "df1 set MECHANICS A1010",
    "df1 set SWAP_DELAY 2",
    "df1 set PAN 50",
    "df1 set STEP_VOLUME 50",
    "df1 set POLL_VOLUME 50",
    "df1 set INSERT_VOLUME 50",
    "df1 set EJECT_VOLUME 50",

    "hd0",
    "hd0 disconnect",
    "hd0 connect",
    "hd0 geometry 320 2 32",
    "hd0 set PAN 50",
    "hd0 set STEP_VOLUME 50",

    "hd1",
    "hd1 disconnect",
    "hd1 connect",
    "hd1 geometry 320 2 32",
    "hd1 set PAN 50",
    "hd1 set STEP_VOLUME 50",

    "server",
    "server serial",
    "server serial set PORT 8000",
    "server serial set VERBOSE true",
    "server serial set VERBOSE false",

    "server rshell",
    "server rshell set PORT 8000",
    "server rshell set VERBOSE true",
    "server rshell set VERBOSE false",

    "server gdb",
    "server gdb set PORT 8000",
    "server gdb set VERBOSE true",
    "server gdb set VERBOSE false",

    // Enter debugger
    ".",

    "",
    "break",
    "break at 1024",
    "break at $A000",
    "break at 0xB000",
    "break delete 2",

    "",
    "watch",
    "watch at 1024",
    "watch at $A000",
    "watch at 0xB000",
    "watch delete 2",

    "catch",
    "catch vector 12",
    "catch interrupt 2",
    "catch trap 12",
    "catch delete 2",

    "cbreak",
    "cbreak at 1024",
    "cbreak at $A000",
    "cbreak at 0xB000",
    "cbreak delete 2",

    "",
    "cwatch",
    "cwatch at 1024",
    "cwatch at $A000",
    "cwatch at 0xB000",
    "cwatch delete 2",

    "",
    "? amiga",

    "",
    "? memory",
    "? memory bankmap",

    "",
    "? cpu",

    "",
    "? ciaa",
    "? ciaa tod",

    "",
    "? ciab",
    "? ciab tod",

    "",
    "? agnus",
    "? agnus beam",
    "? agnus dma",
    "? agnus sequencer",
    "? agnus events",

    "",
    "? blitter",

    "",
    "? paula audio",
    "? paula audio filter",
    "? paula dc",
    "? paula uart",

    "",
    "? denise",

    "",
    "? rtc",

    "",
    "? zorro",
    "? zorro board 0",
    "? zorro board 1",

    "",
    "? controlport 1",
    "? controlport 2",

    "",
    "? serial",

    "",
    "? keyboard",

    "",
    "? mouse 1",
    "? mouse 2",

    "",
    "? joystick 1",
    "? joystick 2",

    "? df0",
    "? df0 disk",
    "? df1",
    "? df1 disk",
    "? df2",
    "? df2 disk",
    "? df3",
    "? df3 disk",

    "? hd0",
    "? hd1",
    "? hd2",
    "? hd3",
    
    "? thread",
    "? server",

    "shutdown",

    nullptr
};

}
