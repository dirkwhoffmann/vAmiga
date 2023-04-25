// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "vamiga.h"

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

    // The emulator instance
    Amiga amiga;

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


    //
    // Running
    //

public:
    
    // Processes an incoming message
    void process(Message msg);
};

//
// Self-test script
//

static const char *script[] = {

    "# Self-test script for vAmiga",
    "# ",
    "# This script is executed in nightly-builts to check the integrity of ",
    "# the compiled application. It runs several shell commands to check if ",
    "# anything breaks.",
    "# ",
    "# Dirk W. Hoffmann, 2023",

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
    "amiga set type PAL",
    "amiga set type NTSC",
    "amiga set fps 50",
    "amiga set fps 60",
    "amiga init A1000_OCS_1MB",
    "amiga init A500_OCS_1MB",
    "amiga init A500_ECS_1MB",
    "amiga init A500_PLUS_1MB",
    "amiga power off",
    "amiga reset",

    "",
    "memory",
    "memory set chip 256",
    "memory set chip 512",
    "memory set chip 1024",
    "memory set slow 0",
    "memory set slow 512",
    "memory set fast 0",
    "memory set fast 256",
    "memory set fast 512",
    "memory set fast 1024",
    "memory set fast 2048",
    "memory set fast 4096",
    "memory set fast 8192",
    "memory set extstart 0xE0",
    "memory set extstart 0xF0",
    "memory set saveroms true",
    "memory set saveroms false",
    "memory set slowramdelay true",
    "memory set slowramdelay false",
    "memory set bankmap A500",
    "memory set bankmap A1000",
    "memory set bankmap A2000A",
    "memory set bankmap A2000B",
    "memory set raminit ALL_ZEROES",
    "memory set raminit ALL_ONES",
    "memory set raminit RANDOMIZED",
    "memory set unmapped FLOATING",
    "memory set unmapped ALL_ZEROES",
    "memory set unmapped ALL_ONES",

    "",
    "cpu",
    "cpu set revision 68000",
    "cpu set revision 68010",
    "cpu set revision 68EC020",
    "cpu set dasm revision 68000",
    "cpu set dasm revision 68010",
    "cpu set dasm revision 68EC020",
    "cpu set dasm revision 68020",
    "cpu set dasm revision 68EC030",
    "cpu set dasm revision 68030",
    "cpu set dasm revision 68EC040",
    "cpu set dasm revision 68LC040",
    "cpu set dasm revision 68040",
    "cpu set dasm syntax MOIRA",
    "cpu set dasm syntax MOIRA_MIT",
    "cpu set dasm syntax GNU",
    "cpu set dasm syntax GNU_MIT",
    "cpu set dasm syntax MUSASHI",
    "cpu set overclocking 0",
    "cpu set overclocking 1",
    "cpu set overclocking 2",
    "cpu set regreset 0",

    "",
    "ciaa",
    "ciaa set revision MOS_8520_DIP",
    "ciaa set revision MOS_8520_PLCC",
    "ciaa set todbug true",
    "ciaa set todbug false",
    "ciaa set esync true",
    "ciaa set esync false",
    "ciab",

    "",
    "ciab set revision MOS_8520_DIP",
    "ciab set revision MOS_8520_PLCC",
    "ciab set todbug true",
    "ciab set todbug false",
    "ciab set esync true",
    "ciab set esync false",

    "",
    "agnus",
    "agnus set revision OCS_OLD",
    "agnus set revision OCS",
    "agnus set revision ECS_1MB",
    "agnus set revision ECS_2MB",
    "agnus set slowrammirror true",
    "agnus set slowrammirror false",
    "agnus set ptrdrops true",
    "agnus set ptrdrops false",

    "blitter",
    "blitter set accuracy 0",
    "blitter set accuracy 1",
    "blitter set accuracy 2",

    "denise",
    "denise set revision OCS",
    "denise set revision ECS",
    "denise set tracking true",
    "denise set tracking false",
    "denise set clxsprspr true",
    "denise set clxsprspr false",
    "denise set clxsprplf true",
    "denise set clxsprplf false",
    "denise set clxplfplf true",
    "denise set clxplfplf false",
    "denise set hidden bitplanes 0",
    "denise set hidden bitplanes 0xFF",
    "denise set hidden sprites 0",
    "denise set hidden sprites 0xFF",
    "denise set hidden layers 0",
    "denise set hidden layers 0xFF",

    "dmadebugger open",
    "dmadebugger close",
    "dmadebugger copper on",
    "dmadebugger copper off",
    "dmadebugger blitter on",
    "dmadebugger blitter off",
    "dmadebugger disk on",
    "dmadebugger disk off",
    "dmadebugger audio on",
    "dmadebugger audio off",
    "dmadebugger sprites on",
    "dmadebugger sprites off",
    "dmadebugger bitplanes on",
    "dmadebugger bitplanes off",
    "dmadebugger cpu on",
    "dmadebugger cpu off",
    "dmadebugger refresh on",
    "dmadebugger refresh off",

    "monitor",
    "monitor set palette COLOR",
    "monitor set palette BLACK_WHITE",
    "monitor set palette PAPER_WHITE",
    "monitor set palette GREEN",
    "monitor set palette AMBER",
    "monitor set palette SEPIA",
    "monitor set brightness 50",
    "monitor set contrast 50",
    "monitor set saturation 50",

    "paula audio",
    "paula audio filter",
    "paula audio filter set type NONE",
    "paula audio filter set type A500",
    "paula audio filter set type A1000",
    "paula audio filter set type A1200",
    "paula audio filter set type LOW",
    "paula audio filter set type LED",
    "paula audio filter set type HIGH",
    "paula audio set sampling NONE",
    "paula audio set sampling NEAREST",
    "paula audio set sampling LINEAR",
    "paula audio set volume channel0 50",
    "paula audio set volume channel1 50",
    "paula audio set volume channel2 50",
    "paula audio set volume channel3 50",
    "paula audio set volume left 50",
    "paula audio set volume right 50",
    "paula audio set pan channel0 50",
    "paula audio set pan channel1 50",
    "paula audio set pan channel2 50",
    "paula audio set pan channel3 50",

    "paula dc",
    "paula dc set speed 2",
    "paula dc dsksync auto true",
    "paula dc dsksync auto false",
    "paula dc dsksync lock true",
    "paula dc dsksync lock false",

    "rtc",
    "rtc set revision NONE",
    "rtc set revision OKI",
    "rtc set revision RICOH",

    "keyboard",
    "keyboard set accuracy 0",
    "keyboard set accuracy 1",
    "keyboard press 32",

    "joystick 1",
    "joystick 1 set autofire true",
    "joystick 1 set autofire false",
    "joystick 1 set bullets 10",
    "joystick 1 set delay 10",
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
    "joystick 2 set autofire true",
    "joystick 2 set autofire false",
    "joystick 2 set bullets 10",
    "joystick 2 set delay 10",
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
    "mouse 1 set pullup true",
    "mouse 1 set pullup false",
    "mouse 1 set shakedetector true",
    "mouse 1 set shakedetector false",
    "mouse 1 set velocity 50",
    "mouse 1 press left",
    "mouse 1 press middle",
    "mouse 1 press right",

    "mouse 2",
    "mouse 2 set pullup true",
    "mouse 2 set pullup false",
    "mouse 2 set shakedetector true",
    "mouse 2 set shakedetector false",
    "mouse 2 set velocity 50",
    "mouse 2 press left",
    "mouse 2 press middle",
    "mouse 2 press right",

    "serial",
    "serial set device NONE",
    "serial set device NULLMODEM",
    "serial set device LOOPBACK",

    "df0",
    "df0 eject",
    "df0 set model DD_35",
    "df0 set rpm 300",
    "df0 set mechanics NONE",
    "df0 set mechanics A1010",
    "df0 set searchpath \"tmp/\"",
    "df0 set swapdelay 2",
    "df0 set pan 50",
    "df0 audiate insert 50",
    "df0 audiate eject 50",
    "df0 audiate step 50",
    "df0 audiate poll 50",

    "df1 connect",
    "df1 disconnect",
    "df1 eject",
    "df1 set model DD_35",
    "df1 set rpm 300",
    "df1 set mechanics NONE",
    "df1 set mechanics A1010",
    "df1 set searchpath \"tmp/\"",
    "df1 set swapdelay 2",
    "df1 set pan 50",
    "df1 audiate insert 50",
    "df1 audiate eject 50",
    "df1 audiate step 50",
    "df1 audiate poll 50",

    "hd0",
    "hd0 disconnect",
    "hd0 connect",
    "hd0 set pan 50",
    "hd0 set geometry 320 2 32",
    "hd0 audiate step 50",

    "hd1",
    "hd1 disconnect",
    "hd1 connect",
    "hd1 set pan 50",
    "hd1 set geometry 320 2 32",
    "hd1 audiate step 50",

    "server",
    "server serial",
    "server serial set port 8000",
    "server serial set verbose true",
    "server serial set verbose false",

    "server rshell",
    "server rshell set port 8000",
    "server rshell set verbose true",
    "server rshell set verbose false",

    "server gdb",
    "server gdb set port 8000",
    "server gdb set verbose true",
    "server gdb set verbose false",

    "",
    "# Entering the debugger...",
    ".",

    "",
    "break",
    "break at 1024",
    "break at $A000",
    "break at 0xB000",
    "break",
    "break enable 0",
    "break disable 1",
    "break delete 2",
    "break ignore 0 20",
    "break",

    "",
    "watch",
    "watch at 1024",
    "watch at $A000",
    "watch at 0xB000",
    "watch",
    "watch enable 0",
    "watch disable 1",
    "watch delete 2",
    "watch ignore 0 20",
    "watch",

    "catch",
    "catch vector 12",
    "catch interrupt 2",
    "catch trap 12",
    "catch",
    "catch enable 0",
    "catch disable 1",
    "catch delete 2",
    "catch ignore 0 20",
    "catch",

    "cbreak",
    "cbreak at 1024",
    "cbreak at $A000",
    "cbreak at 0xB000",
    "cbreak",
    "cbreak enable 0",
    "cbreak disable 1",
    "cbreak delete 2",
    "cbreak ignore 0 20",
    "cbreak",

    "",
    "cwatch",
    "cwatch at 1024",
    "cwatch at $A000",
    "cwatch at 0xB000",
    "cwatch",
    "cwatch enable 0",
    "cwatch disable 1",
    "cwatch delete 2",
    "cwatch ignore 0 20",
    "cwatch",

    "",
    "amiga",
    "host",

    "",
    "memory",
    "memory dump $A00000",
    "memory bankmap",
    "memory write $A00000, $FFFF",

    "",
    "cpu",
    "cpu vectors",

    "",
    "ciaa",
    "ciaa tod",

    "",
    "ciab",
    "ciab tod",

    "",
    "agnus",
    "agnus beam",
    "agnus dma",
    "agnus events",

    "",
    "blitter",

    "",
    "copper",
    "copper list 1",
    "copper list 2",

    "",
    "paula",
    "paula audio",
    "paula audio filter",
    "paula dc",
    "paula uart",

    "",
    "denise",

    "",
    "rtc",

    "",
    "zorro",
    "zorro inspect 0",
    "zorro inspect 1",

    "",
    "controlport 1",
    "controlport 2",

    "",
    "serial",

    "",
    "keyboard",

    "",
    "mouse 1",
    "mouse 2",

    "",
    "joystick 1",
    "joystick 2",

    "df0",
    "df0 disk",
    "df1",
    "df1 disk",
    "df2",
    "df2 disk",
    "df3",
    "df3 disk",
};

}
