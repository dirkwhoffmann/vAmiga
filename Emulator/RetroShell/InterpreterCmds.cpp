// -----------------------------------------------------------------------------
// This file is part of vAmiga Bare Metal
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Interpreter.h"

namespace va {

void
Interpreter::registerInstructions()
{
    msg("registerInstructions()");
    
    
    //
    // Commands
    //

    root.add("clear",
             "command", "Clears the console window",
             nullptr, 0, 0);
    root.seek("clear")->hidden = true;

    /*
    root.add("clear",
             "command", "Clears the console window",
             &Controller::exec <Token::clear>, 0, 0);
    root.seek("clear")->hidden = true;

    root.add("close",
             "command", "Hides the debug console",
             &Controller::exec <Token::close>, 0, 0);
    root.seek("close")->hidden = true;

    root.add("joshua",
             "command", "",
             &Controller::exec <Token::easteregg>, 0, 0);
    root.seek("joshua")->hidden = true;

    root.add("list",
             "command", "Prints system information");

    root.add("list", "devices",
             "command", "Lists the available input devices",
             &Controller::exec <Token::list, Token::devices>, 0, 0);

    root.add("source",
             "command", "Processes a command script",
             &Controller::exec <Token::source>, 1, 0);
    
    
    //
    // Amiga
    //
    
    root.add("amiga",
             "component", "The virtual Amiga");
        
    root.add("amiga", "power",
             "command", "Switches the Amiga on or off");
    
    root.add("amiga", "power", "on",
             "state", "Switches the Amiga on",
             &Controller::exec <Token::amiga, Token::on>, 0, 0);

    root.add("amiga", "power", "off",
             "state", "Switches the Amiga off",
             &Controller::exec <Token::amiga, Token::off>, 0, 0);

    root.add("amiga", "run",
             "command", "Starts the emulator thread",
             &Controller::exec <Token::amiga, Token::run>, 0, 0);

    root.add("amiga", "pause",
             "command", "Halts the emulator thread",
             &Controller::exec <Token::amiga, Token::pause>, 0, 0);
    
    root.add("amiga", "reset",
             "command", "Performs a hard reset",
             &Controller::exec <Token::amiga, Token::reset>, 0, 0);
    
    root.add("amiga", "inspect",
             "command", "Displays the component state",
             &Controller::exec <Token::amiga, Token::inspect>, 0, 0);

    
    //
    // Memory
    //
    
    root.add("memory",
             "component", "Ram and Rom");
    
    root.add("memory", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::memory, Token::config>, 0, 0);

    root.add("memory", "set",
             "command", "Configures the component");
        
    root.add("memory", "set", "chip" ,
             "key", "Configures the amouts of chip memory",
             &Controller::exec <Token::memory, Token::set, Token::chip>, 1, 0);

    root.add("memory", "set", "slow" ,
             "key", "Configures the amouts of slow memory",
             &Controller::exec <Token::memory, Token::set, Token::slow>, 1, 0);

    root.add("memory", "set", "fast" ,
             "key", "Configures the amouts of flow memory",
             &Controller::exec <Token::memory, Token::set, Token::fast>, 1, 0);

    root.add("memory", "set", "extstart" ,
             "key", "Sets the start address for Rom extensions",
             &Controller::exec <Token::memory, Token::set, Token::extstart>, 1, 0);

    root.add("memory", "set", "slowramdelay" ,
             "key", "Enables or disables slow Ram bus delays",
             &Controller::exec <Token::memory, Token::set, Token::slowramdelay>, 1, 0);

    root.add("memory", "set", "bankmap" ,
             "key", "Selects the bank mapping scheme",
             &Controller::exec <Token::memory, Token::set, Token::bankmap>, 1, 0);

    root.add("memory", "set", "unmapped" ,
             "key", "Determines the behaviour of unmapped memory",
             &Controller::exec <Token::memory, Token::set, Token::unmappingtype>, 1, 0);

    root.add("memory", "set", "raminit" ,
             "key", "Determines how Ram is initialized on startup",
             &Controller::exec <Token::memory, Token::set, Token::raminitpattern>, 1, 0);
    
    root.add("memory", "load",
             "command", "Installs a Rom image");
            
    root.add("memory", "load", "rom",
             "command", "Installs a Kickstart Rom",
             &Controller::exec <Token::memory, Token::load, Token::rom>, 1, 0);

    root.add("memory", "load", "extrom",
             "command", "Installs a Rom extension",
             &Controller::exec <Token::memory, Token::load, Token::extrom>, 1, 0);

    root.add("memory", "inspect",
             "command", "Displays the component state");

    root.add("memory", "inspect", "state",
             "command", "Displays the current state",
             &Controller::exec <Token::memory, Token::inspect, Token::state>, 0, 0);

    root.add("memory", "inspect", "bankmap",
             "command", "Displays the bank map",
             &Controller::exec <Token::memory, Token::inspect, Token::bankmap>, 0, 0);

    root.add("memory", "inspect", "checksum",
             "command", "Computes memory checksums",
             &Controller::exec <Token::memory, Token::inspect, Token::checksums>, 0, 0);

    
    //
    // CPU
    //
    
    root.add("cpu",
             "component", "Motorola 68k CPU");
    
    root.add("cpu", "inspect",
             "command", "Displays the component state");

    root.add("cpu", "inspect", "state",
             "command", "Displays the current state",
             &Controller::exec <Token::cpu, Token::inspect, Token::state>, 0, 0);

    root.add("cpu", "inspect", "registers",
             "command", "Displays the current register values",
             &Controller::exec <Token::cpu, Token::inspect, Token::registers>, 0, 0);

    
    //
    // CIA
    //
    
    root.add("ciaa",
             "component", "Complex Interface Adapter A");

    root.add("ciab",
             "component", "Complex Interface Adapter B");

    root.add("cia", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::cia, Token::config>, 0, 0);

    root.add("cia", "set",
             "command", "Configures the component");
        
    root.add("cia", "set", "revision" ,
             "key", "Selects the emulated chip model",
             &Controller::exec <Token::cia, Token::set, Token::revision>, 1, 0);

    root.add("cia", "set", "todbug" ,
             "key", "Enables or disables the TOD hardware bug",
             &Controller::exec <Token::cia, Token::set, Token::todbug>, 1, 0);

    root.add("cia", "set", "esync" ,
             "key", "Turns E-clock syncing on or off",
             &Controller::exec <Token::cia, Token::set, Token::esync>, 1, 0);

    root.add("cia", "inspect",
             "command", "Displays the component state");

    root.add("cia", "inspect", "state",
             "category", "Displays the current state",
             &Controller::exec <Token::cia, Token::inspect, Token::state>, 0, 0);

    root.add("cia", "inspect", "registers",
             "category", "Displays the current register values",
             &Controller::exec <Token::cia, Token::inspect, Token::registers>, 0, 0);

    root.add("cia", "inspect", "tod",
             "category", "Displays the state of the 24-bit counter",
             &Controller::exec <Token::cia, Token::inspect, Token::tod>, 0, 0);

    
    //
    // Agnus
    //

    root.add("agnus",
             "component", "Custom chip");

    root.add("agnus", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::agnus, Token::config>, 0, 0);
    
    root.add("agnus", "set",
             "command", "Configures the component");
        
    root.add("agnus", "set", "revision" ,
             "key", "Selects the emulated chip model",
             &Controller::exec <Token::agnus, Token::set, Token::revision>, 1, 0);

    root.add("agnus", "set", "slowrammirror",
             "key", "Enables or disables ECS Slow Ram mirroring",
             &Controller::exec <Token::agnus, Token::set, Token::slowrammirror>, 1, 0);

    root.add("agnus", "inspect",
             "command", "Displays the internal state");

    root.add("agnus", "inspect", "state",
             "category", "Displays the current state",
             &Controller::exec <Token::agnus, Token::inspect, Token::state>, 0, 0);

    root.add("agnus", "inspect", "registers",
             "category", "Displays the current register value",
             &Controller::exec <Token::agnus, Token::inspect, Token::registers>, 0, 0);

    root.add("agnus", "inspect", "events",
             "category", "Displays scheduled events",
             &Controller::exec <Token::agnus, Token::inspect, Token::events>, 0, 0);
    
    
    //
    // Blitter
    //
    
    root.add("blitter",
             "component", "Custom Chip (Agnus)");
    
    root.add("blitter", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::blitter, Token::config>, 0, 0);
    
    root.add("blitter", "set",
             "command", "Configures the component");
        
    root.add("blitter", "set", "accuracy",
             "level", "Selects the emulation accuracy level",
             &Controller::exec <Token::blitter, Token::set, Token::accuracy>, 1, 0);

    root.add("blitter", "inspect",
             "command", "Displays the internal state");

    root.add("blitter", "inspect", "state",
             "category", "Displays the internal state",
             &Controller::exec <Token::blitter, Token::inspect, Token::state>, 0, 0);

    root.add("blitter", "inspect", "registers",
             "category", "Displays the current register value",
             &Controller::exec <Token::blitter, Token::inspect, Token::registers>, 0, 0);

    
    //
    // Copper
    //
    
    root.add("copper",
             "component", "Custom Chip (Agnus)");
    
    root.add("copper", "inspect",
             "command", "Displays the internal state");

    root.add("copper", "inspect", "state",
             "category", "Displays the current state",
             &Controller::exec <Token::copper, Token::inspect, Token::state>, 0, 0);

    root.add("copper", "inspect", "registers",
             "category", "Displays the current register value",
             &Controller::exec <Token::copper, Token::inspect, Token::registers>, 0, 0);

    
    //
    // Denise
    //
    
    root.add("denise",
             "component", "Custom chip");
    
    root.add("denise", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::denise, Token::config>, 0, 0);

    root.add("denise", "set",
             "command", "Configures the component");

    root.add("denise", "set", "revision" ,
             "key", "Selects the emulated chip model",
             &Controller::exec <Token::denise, Token::set, Token::revision>, 1, 0);
    
    root.add("denise", "set", "clxsprspr" ,
             "key", "Enables or disables sprite-sprite collision detection",
             &Controller::exec <Token::denise, Token::set, Token::clxsprspr>, 1, 0);

    root.add("denise", "set", "clxsprplf" ,
             "key", "Enables or disables sprite-playfield collision detection",
             &Controller::exec <Token::denise, Token::set, Token::clxsprplf>, 1, 0);

    root.add("denise", "set", "clxplfplf" ,
             "key", "Enables or disables playfield-playfield collision detection",
             &Controller::exec <Token::denise, Token::set, Token::clxplfplf>, 1, 0);
    
    root.add("denise", "inspect",
             "command", "Displays the internal state");

    root.add("denise", "inspect", "state",
             "category", "Displays the current state",
             &Controller::exec <Token::denise, Token::inspect, Token::state>, 0, 0);

    root.add("denise", "inspect", "registers",
             "category", "Displays the current register value",
             &Controller::exec <Token::denise, Token::inspect, Token::registers>, 0, 0);

    
    //
    // Monitor
    //

    root.add("monitor",
             "component", "Amiga monitor");

    root.add("monitor", "set",
             "command", "Configures the component");

    root.add("monitor", "set", "palette",
             "key", "Selects the color palette",
             &Controller::exec <Token::monitor, Token::set, Token::palette>, 1, 0);

    root.add("monitor", "set", "brightness",
             "key", "Adjusts the brightness of the Amiga texture",
             &Controller::exec <Token::monitor, Token::set, Token::brightness>, 1, 0);

    root.add("monitor", "set", "contrast",
             "key", "Adjusts the contrast of the Amiga texture",
             &Controller::exec <Token::monitor, Token::set, Token::contrast>, 1, 0);

    root.add("monitor", "set", "saturation",
             "key", "Adjusts the saturation of the Amiga texture",
             &Controller::exec <Token::monitor, Token::set, Token::saturation>, 1, 0);

    
    //
    // Audio
    //
    
    root.add("audio",
             "component", "Audio Unit (Paula)");
    
    root.add("audio", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::audio, Token::config>, 0, 0);

    root.add("audio", "set",
             "command", "Configures the component");

    root.add("audio", "set", "sampling",
             "key", "Selects the sampling method",
             &Controller::exec <Token::audio, Token::set, Token::sampling>, 1, 0);

    root.add("audio", "set", "filter",
             "key", "Configures the audio filter",
             &Controller::exec <Token::audio, Token::set, Token::filter>, 1, 0);
    
    root.add("audio", "set", "volume",
             "key", "Sets the volume");

    root.add("audio", "set", "volume", "channel0",
             "key", "Sets the volume for audio channel 0",
             &Controller::exec <Token::audio, Token::set, Token::volume>, 1, 0);
    
    root.add("audio", "set", "volume", "channel1",
             "key", "Sets the volume for audio channel 1",
             &Controller::exec <Token::audio, Token::set, Token::volume>, 1, 1);
    
    root.add("audio", "set", "volume", "channel2",
             "key", "Sets the volume for audio channel 2",
             &Controller::exec <Token::audio, Token::set, Token::volume>, 1, 2);
    
    root.add("audio", "set", "volume", "channel3",
             "key", "Sets the volume for audio channel 3",
             &Controller::exec <Token::audio, Token::set, Token::volume>, 1, 3);
    
    root.add("audio", "set", "volume", "left",
             "key", "Sets the master volume for the left speaker",
             &Controller::exec <Token::audio, Token::set, Token::volume>, 1, 4);
    
    root.add("audio", "set", "volume", "right",
             "key", "Sets the master volume for the right speaker",
             &Controller::exec <Token::audio, Token::set, Token::volume>, 1, 5);

    root.add("audio", "set", "pan",
             "key", "Sets the pan for one of the four audio channels");
    
    root.add("audio", "set", "pan", "channel0",
             "key", "Sets the pan for audio channel 0",
             &Controller::exec <Token::audio, Token::set, Token::pan>, 1, 0);
    
    root.add("audio", "set", "pan", "channel1",
             "key", "Sets the pan for audio channel 1",
             &Controller::exec <Token::audio, Token::set, Token::pan>, 1, 1);
    
    root.add("audio", "set", "pan", "channel2",
             "key", "Sets the pan for audio channel 2",
             &Controller::exec <Token::audio, Token::set, Token::pan>, 1, 2);
    
    root.add("audio", "set", "pan", "channel3",
             "key", "Sets the pan for audio channel 3",
             &Controller::exec <Token::audio, Token::set, Token::pan>, 1, 3);

    root.add("audio", "inspect",
             "command", "Displays the internal state");

    root.add("audio", "inspect", "state",
             "category", "Displays the current state",
             &Controller::exec <Token::audio, Token::inspect, Token::state>, 0, 0);

    root.add("audio", "inspect", "registers",
             "category", "Displays the current register value",
             &Controller::exec <Token::audio, Token::inspect, Token::registers>, 0, 0);
    
    
    //
    // Paula
    //
    
    root.add("paula",
             "component", "Custom chip");

    root.add("paula", "inspect",
             "command", "Displays the internal state");

    root.add("paula", "inspect", "state",
             "command", "Displays the current register value",
             &Controller::exec <Token::paula, Token::inspect, Token::state>, 0, 0);

    root.add("paula", "inspect", "registers",
             "command", "Displays the current register value",
             &Controller::exec <Token::paula, Token::inspect, Token::registers>, 0, 0);

    
    //
    // RTC
    //

    root.add("rtc",
             "component", "Real-time clock");

    root.add("rtc", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::rtc, Token::config>, 0, 0);

    root.add("rtc", "set",
             "command", "Configures the component");
        
    root.add("rtc", "set", "revision" ,
             "key", "Selects the emulated chip model",
             &Controller::exec <Token::rtc, Token::set, Token::revision>, 1, 0);

    root.add("rtc", "inspect",
             "command", "Displays the internal state");

    root.add("rtc", "inspect", "registers",
             "command", "Displays the current register value",
             &Controller::exec <Token::rtc, Token::inspect, Token::registers>, 0, 0);

    
    //
    // Control port
    //

    root.add("controlport",
             "component", "Control port");

    root.add("controlport", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::controlport, Token::config>, 0, 0);
    
    root.add("controlport", "connect",
             "command", "Connects a device");

    root.add("controlport", "connect", "joystick",
             "device", "Connects a joystick",
             &Controller::exec <Token::controlport, Token::connect, Token::joystick>, 1, 0);

    root.add("controlport", "connect", "keyset",
             "device", "Connects a joystick keyset",
             &Controller::exec <Token::controlport, Token::connect, Token::keyset>, 1, 0);

    root.add("controlport", "connect", "mouse",
             "device", "Connects a mouse",
             &Controller::exec <Token::controlport, Token::connect, Token::mouse>, 1, 0);
    
    root.add("controlport", "inspect",
             "command", "Displays the internal state",
             &Controller::exec <Token::controlport, Token::inspect>, 0, 0);
    

    //
    // Keyboard
    //

    root.add("keyboard",
             "component", "Keyboard");

    root.add("keyboard", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::keyboard, Token::config>, 0, 0);
    
    root.add("keyboard", "set",
             "command", "Configures the component");
        
    root.add("keyboard", "set", "accuracy",
             "key", "Determines the emulation accuracy level",
             &Controller::exec <Token::keyboard, Token::set, Token::accuracy>, 1, 0);

    root.add("keyboard", "inspect",
             "command", "Displays the internal state",
             &Controller::exec <Token::keyboard, Token::inspect>, 0, 0);

    
    //
    // Mouse
    //

    root.add("mouse",
             "component", "Mouse");

    root.add("mouse", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::mouse, Token::config>, 0, 0);
    
    root.add("mouse", "set",
             "command", "Configures the component");
        
    root.add("mouse", "set", "pullup",
             "key", "Enables or disables the emulation of pull-up resistors",
             &Controller::exec <Token::mouse, Token::set, Token::pullup>, 1, 0);

    root.add("mouse", "set", "shakedetector",
             "key", "Enables or disables the shake detector",
             &Controller::exec <Token::mouse, Token::set, Token::shakedetector>, 1, 0);

    root.add("mouse", "set", "velocity",
             "key", "Sets the horizontal and vertical mouse velocity",
             &Controller::exec <Token::mouse, Token::set, Token::velocity>, 1, 0);

    root.add("mouse", "inspect",
             "command", "Displays the internal state",
             &Controller::exec <Token::mouse, Token::inspect>, 0, 0);
    
    
    //
    // Serial port
    //
    
    root.add("serial",
             "component", "Serial port");

    root.add("serial", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::serial, Token::config>, 0, 0);

    root.add("serial", "set",
             "command", "Configures the component");
        
    root.add("serial", "set", "device",
             "key", "",
             &Controller::exec <Token::serial, Token::set, Token::device>, 1, 0);

    root.add("serial", "inspect",
             "command", "Displays the internal state",
             &Controller::exec <Token::serial, Token::inspect>, 0, 0);

    
    //
    // Disk controller
    //
    
    root.add("diskcontroller",
             "component", "Disk Controller");

    root.add("diskcontroller", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::dc, Token::config>, 0, 0);

    root.add("diskcontroller", "set",
             "command", "Configures the component");
        
    root.add("diskcontroller", "set", "speed" ,
             "key", "Configures the drive speed",
             &Controller::exec <Token::dc, Token::speed>, 1, 0);

    root.add("diskcontroller", "dsksync",
             "command", "Secures the DSKSYNC register");

    root.add("diskcontroller", "dsksync", "auto" ,
             "key", "Always receive a SYNC signal",
             &Controller::exec <Token::dc, Token::dsksync, Token::autosync>, 1, 0);

    root.add("diskcontroller", "dsksync", "lock",
             "key", "Prevents writes to DSKSYNC",
             &Controller::exec <Token::dc, Token::dsksync, Token::lock>, 1, 0);
        
    root.add("diskcontroller", "inspect",
             "command", "Displays the internal state",
             &Controller::exec <Token::dc, Token::inspect>, 0, 0);


    //
    // Df0, Df1, Df2, Df3
    //
    
    root.add("df",
             "component", "Floppy drive");

    root.add("dfn",
             "component", "All connected drives");

    root.add("df", "config",
             "command", "Displays the current configuration",
             &Controller::exec <Token::dfn, Token::config>, 0, 0);

    root.add("df", "connect",
             "command", "Connects the drive",
             &Controller::exec <Token::dfn, Token::connect>, 0, 0);
    root.seek("df0")->remove("connect");

    root.add("df", "disconnect",
             "command", "Disconnects the drive",
             &Controller::exec <Token::dfn, Token::disconnect>, 0, 0);
    root.seek("df0")->remove("disconnect");

    root.add("df", "eject",
             "command", "Ejects a floppy disk",
             &Controller::exec <Token::dfn, Token::eject>, 0, 0);

    root.add("df", "insert",
             "command", "Inserts a floppy disk",
             &Controller::exec <Token::dfn, Token::insert>, 1, 0);

    root.add("df", "set",
             "command", "Configures the component");
        
    root.add("df", "set", "model",
             "key", "Selects the drive model",
             &Controller::exec <Token::dfn, Token::set, Token::model>, 1, 0);

    root.add("df", "set", "mechanics",
             "key", "Enables or disables the emulation of mechanical delays",
             &Controller::exec <Token::dfn, Token::set, Token::mechanics>, 1, 0);

    root.add("df", "set", "searchpath",
             "key", "Sets the search path for media files",
             &Controller::exec <Token::dfn, Token::set, Token::searchpath>, 1, 0);
    
    root.add("df", "set", "defaultfs",
             "key", "Determines the default file system type for blank disks",
             &Controller::exec <Token::dfn, Token::set, Token::defaultfs>, 1, 0);

    root.add("df", "set", "defaultbb",
             "key", "Determines the default boot block type for blank disks",
             &Controller::exec <Token::dfn, Token::set, Token::defaultbb>, 1, 0);
    
    root.add("df", "set", "pan",
             "key", "Sets the pan for drive sounds",
             &Controller::exec <Token::dfn, Token::set, Token::pan>, 1, 0);
    
    root.add("df", "audiate",
             "command", "Sets the volume of drive sounds",
             &Controller::exec <Token::dfn, Token::set, Token::mechanics>, 0, 0);

    root.add("df", "audiate", "insert",
             "command", "Makes disk insertions audible",
             &Controller::exec <Token::dfn, Token::audiate, Token::insert>, 1, 0);

    root.add("df", "audiate", "eject",
             "command", "Makes disk ejections audible",
             &Controller::exec <Token::dfn, Token::audiate, Token::eject>, 1, 0);

    root.add("df", "audiate", "step",
             "command", "Makes disk ejections audible",
             &Controller::exec <Token::dfn, Token::audiate, Token::step>, 1, 0);

    root.add("df", "audiate", "poll",
             "command", "Makes polling clicks audible",
             &Controller::exec <Token::dfn, Token::audiate, Token::poll>, 1, 0);

    root.add("df", "inspect",
             "command", "Displays the internal state",
             &Controller::exec <Token::dfn, Token::inspect>, 0, 0);
     */
}

}
