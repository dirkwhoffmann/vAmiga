// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "config.h"
#include "VAmiga.h"
#include "Emulator.h"
#include "Amiga.h"
#include "Media.h"

using std::map;
using std::vector;

namespace vamiga {

struct SyntaxError : public std::runtime_error {
    using runtime_error::runtime_error;
};

void process(const void *listener, Message msg);

class Headless {

    // Parsed command line arguments
    map<string,string> keys;

    // Barrier for syncing script execution
    util::Mutex barrier;

    // Return code
    std::optional<int> returnCode;

    
    //
    // Launching
    //
    
public:

    // Main entry point
    int main(int argc, char *argv[]);

private:

    // Parses the command line arguments
    void parseArguments(int argc, char *argv[]);

    // Checks all command line arguments for conistency
    void checkArguments() throws;

    // Returns the path to the self-test script
    string selfTestScript();

    // Executes the provided script
    int execScript();

    
    //
    // Running
    //

public:
    
    // Reports size information
    void reportSize();

    // Processes an incoming message
    void process(Message msg);
};

//
// Self-test scripts
//

static const char *script[] = {

    "# Self-test script for vAmiga",
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

    "",
    "regression setup A1000_OCS_1MB",
    "regression setup A500_OCS_1MB",
    "regression setup A500_ECS_1MB",
    "regression setup A500_PLUS_1MB",

    "",
    "amiga",
    "amiga defaults",
    "amiga set VIDEO_FORMAT PAL",
    "amiga set VIDEO_FORMAT NTSC",
    "amiga set WARP_BOOT 0",
    "amiga set WARP_BOOT 10",

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

    "joystick 1",
    "joystick 1 set AUTOFIRE true",
    "joystick 1 set AUTOFIRE false",
    "joystick 1 set AUTOFIRE_BULLETS 10",
    "joystick 1 set AUTOFIRE_DELAY 10",
    "joystick 1 press 1",
    "joystick 1 press 2",
    "joystick 1 press 3",
    "joystick 1 unpress 1",
    "joystick 1 unpress 2",
    "joystick 1 unpress 3",
    "joystick 1 pull left",
    "joystick 1 pull right",
    "joystick 1 pull up",
    "joystick 1 pull down",
    "joystick 1 release x",
    "joystick 1 release y",

    "joystick 2",
    "joystick 2 set AUTOFIRE true",
    "joystick 2 set AUTOFIRE false",
    "joystick 2 set AUTOFIRE_BULLETS 10",
    "joystick 2 set AUTOFIRE_DELAY 10",
    "joystick 2 press 1",
    "joystick 2 press 2",
    "joystick 2 press 3",
    "joystick 2 unpress 1",
    "joystick 2 unpress 2",
    "joystick 2 unpress 3",
    "joystick 2 pull left",
    "joystick 2 pull right",
    "joystick 2 pull up",
    "joystick 2 pull down",
    "joystick 2 release x",
    "joystick 2 release y",

    "mouse 1",
    "mouse 1 set PULLUP_RESISTORS true",
    "mouse 1 set PULLUP_RESISTORS false",
    "mouse 1 set SHAKE_DETECTION true",
    "mouse 1 set SHAKE_DETECTION false",
    "mouse 1 set VELOCITY 50",
    "mouse 1 press left",
    "mouse 1 press middle",
    "mouse 1 press right",

    "mouse 2",
    "mouse 2 set PULLUP_RESISTORS true",
    "mouse 2 set PULLUP_RESISTORS false",
    "mouse 2 set SHAKE_DETECTION true",
    "mouse 2 set SHAKE_DETECTION false",
    "mouse 2 set VELOCITY 50",
    "mouse 2 press left",
    "mouse 2 press middle",
    "mouse 2 press right",

    "serial",
    "serial set DEVICE NONE",
    "serial set DEVICE NULLMODEM",
    "serial set DEVICE LOOPBACK",
    "serial set DEVICE RETROSHELL",

    "df0",
    "df0 eject",
    "df0 searchpath \"tmp/\"",
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
    "df1 searchpath \"tmp/\"",
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
    "server gdb set VERBOSE false"
};

/*
static const char *debugScript[] = {

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
    "i amiga",

    "",
    "i memory",
    "i memory bankmap",

    "",
    "i cpu",

    "",
    "i ciaa",
    "i ciaa tod",

    "",
    "i ciab",
    "i ciab tod",

    "",
    "i agnus",
    "i agnus beam",
    "i agnus dma",
    "i agnus sequencer",
    "i agnus events",

    "",
    "i blitter",

    "",
    "i paula audio",
    "i paula audio filter",
    "i paula dc",
    "i paula uart",

    "",
    "i denise",

    "",
    "i rtc",

    "",
    "i zorro",
    "i zorro board 0",
    "i zorro board 1",

    "",
    "i controlport 1",
    "i controlport 2",

    "",
    "i serial",

    "",
    "i keyboard",

    "",
    "i mouse 1",
    "i mouse 2",

    "",
    "i joystick 1",
    "i joystick 2",

    "i df0",
    "i df0 disk",
    "i df1",
    "i df1 disk",
    "i df2",
    "i df2 disk",
    "i df3",
    "i df3 disk",

    "i host",
    "i server"
};
*/

}
