// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Console.h"
#include "Emulator.h"
#include "Option.h"

namespace vamiga {

void
CommanderConsole::_pause()
{
    
}

string
CommanderConsole::getPrompt()
{
    return "vAmiga% ";
}

void
CommanderConsole::welcome()
{
    storage << "RetroShell Commander ";
    remoteManager.rshServer << "vAmiga RetroShell Remote Server ";
    *this << Amiga::build() << '\n';
    *this << '\n';
    *this << "Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de" << '\n';
    *this << "https://github.com/dirkwhoffmann/vAmiga" << '\n';
    *this << '\n';
    
    printHelp();
}

void
CommanderConsole::printHelp()
{
    Console::printHelp();
}

void
CommanderConsole::pressReturn(bool shift)
{
    Console::pressReturn(shift);
}

void
CommanderConsole::initCommands(RetroShellCmd &root)
{
    Console::initCommands(root);
    
    //
    // Workspace management
    //
    
    root.add({
        
        .tokens = { "workspace" },
        .help   = { "Workspace management" },
        .hidden = releaseBuild
    });
    
    root.add({
        
        .tokens = { "workspace init" },
        .help   = { "First command of a workspace script" },
        .hidden = releaseBuild,
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            amiga.initWorkspace();
        }
    });

    root.add({
        
        .tokens = { "workspace activate" },
        .help   = { "Last command of a workspace script" },
        .hidden = releaseBuild,
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            amiga.activateWorkspace();
        }
    });

    
    //
    // Regression tester
    //
    
    RetroShellCmd::currentGroup = "Regression testing";
    
    root.add({
        
        .tokens = { "regression" },
        .help   = { "Runs the regression tester" },
        .hidden = releaseBuild
    });

    /*
    root.add({
        
        .tokens = { "regression", "setup" },
        .help   = { "Initializes the test environment" },
        .args   = { ConfigSchemeEnum::argList() },
        .extra  = { arg::path, arg::path },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto scheme = ConfigScheme(parseEnum<ConfigSchemeEnum>(argv[0]));
            auto rom = argv.size() > 1 ? host.makeAbsolute(argv[1]) : "";
            auto ext = argv.size() > 2 ? host.makeAbsolute(argv[2]) : "";
            
            amiga.regressionTester.prepare(scheme, rom, ext);
        }
    });
    */
    root.add({

        .tokens = { "regression", "setup" },
        .ghelp  = { "Initializes the test environment" },
    });

    for (auto &it : ConfigSchemeEnum::elements()) {

        root.add({

            .tokens = { "regression", "setup", ConfigSchemeEnum::key(it) },
            .help   = { ConfigSchemeEnum::help(it) },
            .argx   = {
                { .name = { "rom", "ROM file" }, .flags = arg::keyval | arg::opt },
                { .name = { "ext", "Extension ROM file" }, .flags = arg::keyval | arg::opt }
            },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

                auto rom = args.contains("rom") ? host.makeAbsolute(args.at("rom")) : "";
                auto ext = args.contains("ext") ? host.makeAbsolute(args.at("ext")) : "";

                amiga.regressionTester.prepare(ConfigScheme(values[0]), rom, ext);
                emulator.set(ConfigScheme(values[0]));
            }, .values = { isize(it) }
        });
    }

    root.add({
        
        .tokens = { "regression", "run" },
        .help   = { "Launches a regression test" },
        .argx   = { { .name = { "path", "Regression test script" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            amiga.regressionTester.run(path);
        }
    });
    
    root.add({
        
        .tokens = { "screenshot" },
        .help   = { "Manages screenshots" },
        .hidden = releaseBuild
    });
    
    root.add({
        
        .tokens = { "screenshot", "set" },
        .help   = { "Configure the screenshot" }
    });
    
    root.add({
        
        .tokens = { "screenshot", "set", "filename" },
        .help   = { "Assign the screenshot filename" },
        .argx   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            amiga.regressionTester.dumpTexturePath = path;
        }
    });
    
    root.add({
        
        .tokens = { "screenshot", "set", "cutout" },
        .help   = { "Adjust the texture cutout" },
        .argx   = {
            { .name = { "x1", "Left x coordinate" }, .flags = arg::keyval },
            { .name = { "x2", "Right x coordinate" }, .flags = arg::keyval },
            { .name = { "y1", "Lower y coordinate" }, .flags = arg::keyval },
            { .name = { "y2", "Upper y coordinate" }, .flags = arg::keyval }
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            isize x1 = parseNum(args.at("x1"));
            isize y1 = parseNum(args.at("y1"));
            isize x2 = parseNum(args.at("x2"));
            isize y2 = parseNum(args.at("y2"));

            amiga.regressionTester.x1 = x1;
            amiga.regressionTester.y1 = y1;
            amiga.regressionTester.x2 = x2;
            amiga.regressionTester.y2 = y2;
        }
    });
    
    root.add({
        
        .tokens = { "screenshot", "save" },
        .help   = { "Saves a screenshot and exits the emulator" },
        .argx   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            amiga.regressionTester.dumpTexture(amiga, path);
        }
    });
    
    
    //
    // Components
    //
    
    RetroShellCmd::currentGroup = "Components";
    
    //
    // Components (Amiga)
    //
    
    auto cmd = registerComponent(amiga);
    
    root.add({
        
        .tokens = { cmd, "defaults" },
        .help   = { "Displays the user defaults storage" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(emulator, Category::Defaults);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "power" },
        .help   = { "Switches the Amiga on or off" },
        .argx   = { { .name = { "onoff", "Power switch state" }, .key = "{ on | off }" } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            parseOnOff(args.at("onoff")) ? emulator.run() : emulator.powerOff();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "reset" },
        .help   = { "Performs a hard reset" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            amiga.hardReset();
        }
    });

    /*
    root.add({
        
        .tokens = { cmd, "init" },
        .help   = { "Initializes the Amiga with a predefined scheme" },
        .args   = { ConfigSchemeEnum::argList() },
        .argx   = { { .name = { "scheme", "Configuration scheme" }, .key = ConfigSchemeEnum::argList() } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            emulator.set(parseEnum<ConfigScheme, ConfigSchemeEnum>(argv[0]));
        }
    });
    */
    root.add({

        .tokens = { cmd, "init" },
        .ghelp  = { "Initializes the Amiga with a predefined scheme" },
    });

    for (auto &it : ConfigSchemeEnum::elements()) {

        root.add({

            .tokens = { cmd, "init", ConfigSchemeEnum::key(it) },
            .help   = { ConfigSchemeEnum::help(it) },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

                emulator.set(ConfigScheme(values[0]));
            }, .values = { isize(it) }
        });
    }


    //
    // Components (Memory)
    //
    
    cmd = registerComponent(mem);
    
    root.add({
        
        .tokens = { cmd, "load" },
        .help   = { "Loads memory contents from a file" }
    });
    
    root.add({
        
        .tokens = { cmd, "load", "rom" },
        .help   = { "Installs a Kickstart Rom" },
        .argx   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            mem.loadRom(path);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "load", "ext" },
        .help   = { "Installs an extension Rom" },
        .argx   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {

            auto path = host.makeAbsolute(args.at("path"));
            mem.loadExt(path);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "load", "bin" },
        .help   = { "Loads a chunk of memory" },
        .argx   = {
            { .name = { "path", "File path" } },
            { .name = { "address", "Target memory address" } },
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            mem.debugger.load(path, parseAddr(args.at("address")));
        }
    });
    
    root.add({
        
        .tokens = { cmd, "save" },
        .help   = { "Save memory contents to a file" }
    });
    
    root.add({
        
        .tokens = { cmd, "save", "rom" },
        .help   = { "Saves the Kickstart Rom" },
        .argx   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            mem.saveRom(path);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "save", "ext" },
        .help   = { "Saves the extension Rom" },
        .argx   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            mem.saveExt(path);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "save", "bin" },
        .help   = { "Loads a chunk of memory" },
        .argx   = {
            { .name = { "path", "File path" } },
            { .name = { "address", "Memory address" } },
            { .name = { "count", "Number of bytes" } },
        },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            fs::path path(args.at("path"));
            mem.debugger.save(path, parseAddr(args.at("address")), parseNum(args.at("count")));
        }
    });
    
    
    //
    // Components (CPU)
    //
    
    cmd = registerComponent(cpu);
    
    
    //
    // Components (CIA)
    //
    
    cmd = registerComponent(ciaa);
    cmd = registerComponent(ciab);
    
    
    //
    // Components (Agnus)
    //
    
    cmd = registerComponent(agnus);
    
    
    //
    // Components (Blitter)
    //
    
    cmd = registerComponent(blitter);
    
    
    //
    // Components (Denise)
    //
    
    cmd = registerComponent(denise);
    
    
    //
    // Components (Paula)
    //
    
    cmd = registerComponent(paula);
    cmd = registerComponent(diskController);
    
    
    //
    // Components (RTC)
    //
    
    cmd = registerComponent(rtc);
    
    
    //
    // Components (Hard-drive controller)
    //
    
    /*
    cmd = registerComponent(hd0con);
    cmd = registerComponent(hd1con);
    cmd = registerComponent(hd2con);
    cmd = registerComponent(hd3con);
    */
    
    
    //
    // Ports
    //
    
    RetroShellCmd::currentGroup = "Ports";
    
    
    //
    // Ports (Audio port)
    //
    
    cmd = registerComponent(audioPort);
    cmd = registerComponent(audioPort.filter);
    
    
    //
    // Ports (Video port)
    //
    
    cmd = registerComponent(videoPort);
    
    
    //
    // Ports (Serial port)
    //
    
    cmd = registerComponent(serialPort);
    
    root.add({
        
        .tokens = { cmd, "send" },
        .help   = { "Sends a text to the serial port" },
        .argx   = { { .name = { "text", "Text message" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            amiga.serialPort << args.at("text");
        }
    });
    
    
    //
    // Peripherals
    //
    
    RetroShellCmd::currentGroup = "Peripherals";
    
    
    //
    // Peripherals (Monitor)
    //
    
    cmd = registerComponent(monitor);
    
    
    //
    // Peripherals (Keyboard)
    //
    
    cmd = registerComponent(keyboard);
    
    root.add({
        
        .tokens = { cmd, "press" },
        .help   = { "Sends a keycode to the keyboard" },
        .argx   = { { .name = { "keycode", "Numerical code of the Amiga key" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto code = KeyCode(parseNum(args.at("keycode")));
            emulator.put(Command(Cmd::KEY_PRESS, KeyCmd { .keycode = code, .delay = 0.0 }));
            emulator.put(Command(Cmd::KEY_RELEASE, KeyCmd { .keycode = code, .delay = 0.5 }));
        }
    });
    
    
    //
    // Peripherals (Joystick)
    //
    
    for (isize i = 0; i <= 1; i++) {
        
        if (i == 0) cmd = registerComponent(controlPort1.joystick);
        if (i == 1) cmd = registerComponent(controlPort2.joystick);
        
        root.add({
                 
            .tokens = { cmd, "press" },
            .help   = { "Presses a joystick button" },
            .argx   = { { .name = { "button", "Button number" } } },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(args.at("button"));

                switch (nr) {
                        
                    case 1: port.joystick.trigger(GamePadAction::PRESS_FIRE); break;
                    case 2: port.joystick.trigger(GamePadAction::PRESS_FIRE2); break;
                    case 3: port.joystick.trigger(GamePadAction::PRESS_FIRE3); break;
                        
                    default:
                        throw AppError(Fault::OPT_INV_ARG, "1...3");
                }
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "unpress" },
            .help   = { "Releases a joystick button" },
            .argx   = { { .name = { "button", "Button number" } } },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(args.at("button"));

                switch (nr) {
                        
                    case 1: port.joystick.trigger(GamePadAction::RELEASE_FIRE); break;
                    case 2: port.joystick.trigger(GamePadAction::RELEASE_FIRE2); break;
                    case 3: port.joystick.trigger(GamePadAction::RELEASE_FIRE3); break;
                        
                    default:
                        throw AppError(Fault::OPT_INV_ARG, "1...3");
                }
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull" },
            .help   = { "Pulls the joystick" }
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "left" },
            .help   = { "Pulls the joystick left" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_LEFT);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "right" },
            .help   = { "Pulls the joystick right" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_RIGHT);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "up" },
            .help   = { "Pulls the joystick up" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_UP);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "down" },
            .help   = { "Pulls the joystick down" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_DOWN);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "release" },
            .help   = { "Release a joystick axis" }
        });
        
        root.add({
            
            .tokens = { cmd, "release", "x" },
            .help   = { "Releases the x-axis" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::RELEASE_X);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "release", "y" },
            .help   = { "Releases the y-axis" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::RELEASE_Y);
                
            }, .values = {i}
        });
    }
    
    
    //
    // Peripherals (Mouse)
    //
    
    for (isize i = 0; i <= 1; i++) {
        
        if (i == 0) cmd = registerComponent(controlPort1.mouse);
        if (i == 1) cmd = registerComponent(controlPort2.mouse);
        
        root.add({
            
            .tokens = { cmd, "press" },
            .help   = { "Presses a mouse button" }
        });
        
        root.add({
            
            .tokens = { cmd, "press", "left" },
            .help   = { "Presses the left mouse button" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.mouse.pressAndReleaseLeft();
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "press", "middle" },
            .help   = { "Presses the middle mouse button" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.mouse.pressAndReleaseMiddle();
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "press", "right" },
            .help   = { "Presses the right mouse button" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.mouse.pressAndReleaseRight();
                
            }, .values = {i}
        });
    }
    
    
    //
    // Peripherals (Df0, Df1, Df2, Df3)
    //
    
    for (isize i = 0; i <= 3; i++) {
        
        cmd = registerComponent(*df[i]);
        
        if (i >= 1 && i <= 3) {
            
            root.add({
                
                .tokens = { cmd, "connect" },
                .help   = { "Connects the drive" },
                .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                    
                    emulator.set(Opt::DRIVE_CONNECT, true, values);
                    
                }, .values = {i}
            });
            
            root.add({
                
                .tokens = { cmd, "disconnect" },
                .help   = { "Disconnects the drive" },
                .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                    
                    emulator.set(Opt::DRIVE_CONNECT, false, values);
                    
                }, .values = {i}
            });
        }
        
        root.add({
            
            .tokens = { cmd, "eject" },
            .help   = { "Ejects a floppy disk" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                amiga.df[values[0]]->ejectDisk();
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "insert" },
            .argx   = { { .name = { "path", "File path" } } },
            .help   = { "Inserts a floppy disk" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto path = host.makeAbsolute(args.at("path"));
                amiga.df[values[0]]->swapDisk(path);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "protect" },
            .help   = { "Enables write protection" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                amiga.df[values[0]]->setFlag(DiskFlags::PROTECTED, true);
                
            }, .values = {i}
        });
 
        root.add({
            
            .tokens = { cmd, "unprotect" },
            .help   = { "Disables write protection" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                amiga.df[values[0]]->setFlag(DiskFlags::PROTECTED, false);
                
            }, .values = {i}
        });
    }
    
    
    //
    // Peripherals (Hd0, Hd1, Hd2, Hd3)
    //
    
    for (isize i = 0; i <= 3; i++) {
        
        cmd = registerComponent(*hd[i]);
        
        root.add({
            
            .tokens = { cmd, "connect" },
            .help   = { "Connects the hard drive" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                emulator.set(Opt::HDC_CONNECT, true, values);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "disconnect" },
            .help   = { "Disconnects the hard drive" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                emulator.set(Opt::HDC_CONNECT, false, values);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "attach" },
            .help   = { "Attaches a hard drive image" },
            .argx   = { { .name = { "path", "Hard drive image file" } } },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                                
                // Make sure the hard-drive controller board is plugged in
                emulator.set(Opt::HDC_CONNECT, true, values);

                // Connect the drive
                auto path = host.makeAbsolute(args.at("path"));
                amiga.hd[values[0]]->init(path);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "geometry" },
            .help   = { "Changes the disk geometry" },
            .argx   = {
                { .name = { "cylinders", "Number of cylinders" }, .flags = arg::keyval },
                { .name = { "heads", "Number of drive heads" }, .flags = arg::keyval },
                { .name = { "sectors", "Number of sectors per cylinder" }, .flags = arg::keyval },
            },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                auto c = util::parseNum(args.at("cylinders"));
                auto h = util::parseNum(args.at("heads"));
                auto s = util::parseNum(args.at("sectors"));

                amiga.hd[values[0]]->changeGeometry(c, h, s);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "protect" },
            .help   = { "Enables write protection" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                amiga.hd[values[0]]->setFlag(DiskFlags::PROTECTED, true);
                
            }, .values = {i}
        });
 
        root.add({
            
            .tokens = { cmd, "unprotect" },
            .help   = { "Disables write protection" },
            .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                
                amiga.hd[values[0]]->setFlag(DiskFlags::PROTECTED, false);
                
            }, .values = {i}
        });
    }
    
    
    //
    // Miscellaneous
    //
    
    RetroShellCmd::currentGroup = "Miscellaneous";
    
    
    //
    // Miscellaneous (Config)
    //
    
    root.add({
        
        .tokens = { "config" },
        .help   = { "Virtual machine configuration" }
    });
    
    root.add({
        
        .tokens = { "config", "" },
        .help   = { "Displays the current configuration" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            amiga.exportConfig(ss);
            *this << ss;
        }
    });
    
    root.add({
        
        .tokens = { "config", "diff" },
        .help   = { "Displays the difference to the default configuration" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            std::stringstream ss;
            amiga.exportDiff(ss);
            *this << ss;
        }
    });
    
    
    //
    // Miscellaneous (DMA Debugger)
    //
    
    cmd = registerComponent(dmaDebugger);
    
    root.add({
        
        .tokens = { cmd, "open" },
        .help   = { "Opens the DMA debugger" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            emulator.set(Opt::DMA_DEBUG_ENABLE, true);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "close" },
        .help   = { "Closes the DMA debugger" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            emulator.set(Opt::DMA_DEBUG_ENABLE, false);
        }
    });
    
    
    //
    // Miscellaneous (Logic Analyzer)
    //
    
    cmd = registerComponent(logicAnalyzer);
    
    
    //
    // Miscellaneous (Host)
    //
    
    cmd = registerComponent(host);
    
    root.add({
        
        .tokens = { cmd, "searchpath" },
        .help   = { "Sets the search path for media files" },
        .argx   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            auto path = fs::path(args.at("path"));
            host.setSearchPath(path);
        }
    });
    
    
    //
    // Miscellaneous (Remote server)
    //
    
    
    root.add({
        
        .tokens = { "server" },
        .help   = { "Remote connections" }
    });
    
    root.add({
        
        .tokens = { "server", "" },
        .help   = { "Displays a server status summary" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            dump(remoteManager, Category::Status);
        }
    });
    
    cmd = registerComponent(remoteManager.serServer);
    
    cmd = registerComponent(remoteManager.rshServer);
    
    root.add({
        
        .tokens = { cmd, "start" },
        .help   = { "Starts the retro shell server" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            remoteManager.rshServer.start();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "stop" },
        .help   = { "Stops the retro shell server" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            remoteManager.rshServer.stop();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "disconnect" },
        .help   = { "Disconnects a client" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
                 
                 remoteManager.rshServer.disconnect();
             }
    });
    
    cmd = registerComponent(remoteManager.promServer);
    
    root.add({
        
        .tokens = { cmd, "start" },
        .help   = { "Starts the Prometheus server" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            remoteManager.promServer.start();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "stop" },
        .help   = { "Stops the Prometheus server" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            remoteManager.promServer.stop();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "disconnect" },
        .help   = { "Disconnects a client" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            remoteManager.promServer.disconnect();
        }
    });
    
    cmd = registerComponent(remoteManager.gdbServer);
    
    root.add({
        
        .tokens = { cmd, "attach" },
        .help   = { "Attaches the GDB server to a process" },
        .argx   = { { .name = { "process", "Process number" } } },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            remoteManager.gdbServer.attach(args.at("process"));
        }
    });
    
    root.add({
        
        .tokens = { cmd, "detach" },
        .help   = { "Detaches the GDB server from a process" },
        .func   = [this] (std::ostream &os, Arguments& argv, const ParsedArguments &args, const std::vector<isize> &values) {
            
            remoteManager.gdbServer.detach();
        }
    });
}

}
