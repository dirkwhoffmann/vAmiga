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
    Console::welcome();
}

void
CommanderConsole::summary()
{
    std::stringstream ss;

    // ss << "RetroShell Commander" << std::endl << std::endl;

    ss << "Model   Chip    Slow    Fast    Agnus   Denise  ROM" << std::endl;
    ss << std::setw(8) << std::left << BankMapEnum::key(BankMap(amiga.get(Opt::MEM_BANKMAP)));
    ss << std::setw(8) << std::left << (std::to_string(amiga.get(Opt::MEM_CHIP_RAM)) + " MB");
    ss << std::setw(8) << std::left << (std::to_string(amiga.get(Opt::MEM_SLOW_RAM)) + " MB");
    ss << std::setw(8) << std::left << (std::to_string(amiga.get(Opt::MEM_FAST_RAM)) + " MB");
    ss << std::setw(8) << std::left << (agnus.isECS() ? "ECS" : "OCS");
    ss << std::setw(8) << std::left << (denise.isECS() ? "ECS" : "OCS");
    ss << mem.getRomTraits().title << std::endl;

    *this << vspace{1};
    string line;
    while(std::getline(ss, line)) { *this << "    " << line << '\n'; }
    // *this << ss;
    *this << vspace{1};
}

void
CommanderConsole::printHelp(isize tab)
{
    Console::printHelp(tab);
}

void
CommanderConsole::pressReturn(bool shift)
{
    Console::pressReturn(shift);
}

void
CommanderConsole::initCommands(RSCommand &root)
{
    Console::initCommands(root);

    //
    // Console management
    //

    /*
    root.add({

        .tokens = { "." },
        .chelp  = { "Switch to the next console" },
        .flags  = rs::hidden,

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            retroShell.enterDebugger();
        }
    });

    root.add({

        .tokens = { ".." },
        .chelp  = { "Switch to the previous console" },
        .flags  = rs::hidden,

        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            retroShell.enterNavigator();
        }
    });
    */

    
    //
    // Workspace management
    //
    
    root.add({
        
        .tokens = { "workspace" },
        .ghelp  = { "Workspace management" },
        .flags  = releaseBuild ? rs::hidden : 0
    });
    
    root.add({
        
        .tokens = { "workspace init" },
        .chelp  = { "First command of a workspace script" },
        .flags  = releaseBuild ? rs::hidden : 0,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            amiga.initWorkspace();
        }
    });

    root.add({
        
        .tokens = { "workspace activate" },
        .chelp  = { "Last command of a workspace script" },
        .flags  = releaseBuild ? rs::hidden : 0,
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            amiga.activateWorkspace();
        }
    });

    
    //
    // Regression tester
    //
    
    RSCommand::currentGroup = "Regression testing";
    
    root.add({
        
        .tokens = { "regression" },
        .ghelp  = { "Runs the regression tester" },
        .flags  = releaseBuild ? rs::hidden : 0
    });

    root.add({

        .tokens = { "regression", "setup" },
        .ghelp  = { "Initializes the test environment" },
    });

    for (auto &it : ConfigSchemeEnum::elements()) {

        root.add({

            .tokens = { "regression", "setup", ConfigSchemeEnum::key(it) },
            .chelp  = { ConfigSchemeEnum::help(it) },
            .args   = {
                { .name = { "rom", "ROM file" }, .flags = rs::keyval | rs::opt },
                { .name = { "ext", "Extension ROM file" }, .flags = rs::keyval | rs::opt }
            },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                auto rom = args.contains("rom") ? host.makeAbsolute(args.at("rom")) : "";
                auto ext = args.contains("ext") ? host.makeAbsolute(args.at("ext")) : "";

                amiga.regressionTester.prepare(ConfigScheme(values[0]), rom, ext);
                emulator.set(ConfigScheme(values[0]));
            }, .payload = { isize(it) }
        });
    }

    root.add({
        
        .tokens = { "regression", "run" },
        .chelp  = { "Launches a regression test" },
        .args   = { { .name = { "path", "Regression test script" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            amiga.regressionTester.run(path);
        }
    });
    
    root.add({
        
        .tokens = { "screenshot" },
        .ghelp  = { "Manages screenshots" },
        .flags  = releaseBuild ? rs::hidden : 0
    });
    
    root.add({
        
        .tokens = { "screenshot", "set" },
        .ghelp  = { "Configure the screenshot" }
    });
    
    root.add({
        
        .tokens = { "screenshot", "set", "filename" },
        .chelp  = { "Assign the screenshot filename" },
        .args   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            amiga.regressionTester.dumpTexturePath = path;
        }
    });
    
    root.add({
        
        .tokens = { "screenshot", "set", "cutout" },
        .chelp  = { "Adjust the texture cutout" },
        .args   = {
            { .name = { "x1", "Left x coordinate" }, .flags = rs::keyval },
            { .name = { "x2", "Right x coordinate" }, .flags = rs::keyval },
            { .name = { "y1", "Lower y coordinate" }, .flags = rs::keyval },
            { .name = { "y2", "Upper y coordinate" }, .flags = rs::keyval }
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        .chelp  = { "Saves a screenshot and exits the emulator" },
        .args   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            amiga.regressionTester.dumpTexture(amiga, path);
        }
    });
    
    
    //
    // Components
    //
    
    RSCommand::currentGroup = "Components";
    
    //
    // Components (Amiga)
    //
    
    auto cmd = registerComponent(amiga);
    
    root.add({
        
        .tokens = { cmd, "defaults" },
        .chelp  = { "Displays the user defaults storage" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, emulator, Category::Defaults);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "power" },
        .chelp  = { "Switches the Amiga on or off" },
        .args   = { { .name = { "onoff", "Power switch state" }, .key = "{ on | off }" } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            parseOnOff(args.at("onoff")) ? emulator.run() : emulator.powerOff();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "reset" },
        .chelp  = { "Performs a hard reset" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            amiga.hardReset();
        }
    });

    root.add({

        .tokens = { cmd, "init" },
        .ghelp  = { "Initializes the Amiga with a predefined scheme" },
    });

    for (auto &it : ConfigSchemeEnum::elements()) {

        root.add({

            .tokens = { cmd, "init", ConfigSchemeEnum::key(it) },
            .chelp  = { ConfigSchemeEnum::help(it) },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

                emulator.set(ConfigScheme(values[0]));
            }, .payload = { isize(it) }
        });
    }


    //
    // Components (Memory)
    //
    
    cmd = registerComponent(mem);
    
    root.add({
        
        .tokens = { cmd, "load" },
        .ghelp  = { "Loads memory contents from a file" }
    });
    
    root.add({
        
        .tokens = { cmd, "load", "rom" },
        .chelp  = { "Installs a Kickstart Rom" },
        .args   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            mem.loadRom(path);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "load", "ext" },
        .chelp  = { "Installs an extension Rom" },
        .args   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {

            auto path = host.makeAbsolute(args.at("path"));
            mem.loadExt(path);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "load", "bin" },
        .chelp  = { "Loads a chunk of memory" },
        .args   = {
            { .name = { "path", "File path" } },
            { .name = { "address", "Target memory address" } },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            mem.debugger.load(path, parseAddr(args.at("address")));
        }
    });
    
    root.add({
        
        .tokens = { cmd, "save" },
        .chelp  = { "Save memory contents to a file" }
    });
    
    root.add({
        
        .tokens = { cmd, "save", "rom" },
        .chelp  = { "Saves the Kickstart Rom" },
        .args   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            mem.saveRom(path);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "save", "ext" },
        .chelp  = { "Saves the extension Rom" },
        .args   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto path = host.makeAbsolute(args.at("path"));
            mem.saveExt(path);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "save", "bin" },
        .chelp  = { "Loads a chunk of memory" },
        .args   = {
            { .name = { "path", "File path" } },
            { .name = { "address", "Memory address" } },
            { .name = { "count", "Number of bytes" } },
        },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
    
    RSCommand::currentGroup = "Ports";
    
    
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
        .chelp  = { "Sends a text to the serial port" },
        .args   = { { .name = { "text", "Text message" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            amiga.serialPort << args.at("text");
        }
    });
    
    
    //
    // Peripherals
    //
    
    RSCommand::currentGroup = "Peripherals";
    
    
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
        .chelp  = { "Sends a keycode to the keyboard" },
        .args   = { { .name = { "keycode", "Numerical code of the Amiga key" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto code = KeyCode(parseNum(args.at("keycode")));
            emulator.put(Command(Cmd::KEY_PRESS, KeyCmd { .keycode = code, .delay = 0.0 }));
            emulator.put(Command(Cmd::KEY_RELEASE, KeyCmd { .keycode = code, .delay = 0.5 }));
        }
    });
    
    
    //
    // Peripherals (Joystick)
    //

    root.add({

        .tokens = { "joystick[n]" },
        .ghelp  = { "Joystick n" },
        .chelp  = { "Commands: joystick1, joystick2" }
    });

    for (isize i = 0; i <= 1; i++) {
        
        if (i == 0) cmd = registerComponent(controlPort1.joystick, true);
        if (i == 1) cmd = registerComponent(controlPort2.joystick, true);

        root.add({
                 
            .tokens = { cmd, "press" },
            .chelp  = { "Presses a joystick button" },
            .args   = { { .name = { "button", "Button number" } } },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(args.at("button"));

                switch (nr) {
                        
                    case 1: port.joystick.trigger(GamePadAction::PRESS_FIRE); break;
                    case 2: port.joystick.trigger(GamePadAction::PRESS_FIRE2); break;
                    case 3: port.joystick.trigger(GamePadAction::PRESS_FIRE3); break;
                        
                    default:
                        throw AppError(Fault::OPT_INV_ARG, "1...3");
                }
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "unpress" },
            .chelp  = { "Releases a joystick button" },
            .args   = { { .name = { "button", "Button number" } } },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(args.at("button"));

                switch (nr) {
                        
                    case 1: port.joystick.trigger(GamePadAction::RELEASE_FIRE); break;
                    case 2: port.joystick.trigger(GamePadAction::RELEASE_FIRE2); break;
                    case 3: port.joystick.trigger(GamePadAction::RELEASE_FIRE3); break;
                        
                    default:
                        throw AppError(Fault::OPT_INV_ARG, "1...3");
                }
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull" },
            .ghelp  = { "Pulls the joystick" }
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "left" },
            .chelp  = { "Pulls the joystick left" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_LEFT);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "right" },
            .chelp  = { "Pulls the joystick right" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_RIGHT);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "up" },
            .chelp  = { "Pulls the joystick up" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_UP);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "down" },
            .chelp  = { "Pulls the joystick down" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_DOWN);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "release" },
            .ghelp  = { "Release a joystick axis" }
        });
        
        root.add({
            
            .tokens = { cmd, "release", "x" },
            .chelp  = { "Releases the x-axis" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::RELEASE_X);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "release", "y" },
            .chelp  = { "Releases the y-axis" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::RELEASE_Y);
                
            }, .payload = {i}
        });
    }
    
    
    //
    // Peripherals (Mouse)
    //

    root.add({

        .tokens = { "mouse[n]" },
        .ghelp  = { "Mouse n" },
        .chelp  = { "Commands: mouse1, mouse2" }
    });

    for (isize i = 0; i <= 1; i++) {
        
        if (i == 0) cmd = registerComponent(controlPort1.mouse, true);
        if (i == 1) cmd = registerComponent(controlPort2.mouse, true);

        root.add({
            
            .tokens = { cmd, "press" },
            .ghelp  = { "Presses a mouse button" }
        });
        
        root.add({
            
            .tokens = { cmd, "press", "left" },
            .chelp  = { "Presses the left mouse button" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.mouse.pressAndReleaseLeft();
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "press", "middle" },
            .chelp  = { "Presses the middle mouse button" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.mouse.pressAndReleaseMiddle();
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "press", "right" },
            .chelp  = { "Presses the right mouse button" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.mouse.pressAndReleaseRight();
                
            }, .payload = {i}
        });
    }
    
    
    //
    // Peripherals (Df0, Df1, Df2, Df3)
    //

    root.add({

        .tokens = { "df[n]" },
        .ghelp  = { "Floppy drive n" },
        .chelp  = { "Commands: df0, df1, df2, df3" }
    });

    for (isize i = 0; i <= 3; i++) {
        
        cmd = registerComponent(*df[i], true);

        if (i >= 1 && i <= 3) {
            
            root.add({
                
                .tokens = { cmd, "connect" },
                .chelp  = { "Connects the drive" },
                .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                    
                    emulator.set(Opt::DRIVE_CONNECT, true, values);
                    
                }, .payload = {i}
            });
            
            root.add({
                
                .tokens = { cmd, "disconnect" },
                .chelp  = { "Disconnects the drive" },
                .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                    
                    emulator.set(Opt::DRIVE_CONNECT, false, values);
                    
                }, .payload = {i}
            });
        }
        
        root.add({
            
            .tokens = { cmd, "eject" },
            .chelp  = { "Ejects a floppy disk" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                amiga.df[values[0]]->ejectDisk();
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "insert" },
            .chelp  = { "Inserts a floppy disk" },
            .args   = {
                { .name = { "path", "File path" } }
            },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto path = host.makeAbsolute(args.at("path"));
                amiga.df[values[0]]->swapDisk(path);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "protect" },
            .chelp  = { "Enables write protection" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                amiga.df[values[0]]->setFlag(DiskFlags::PROTECTED, true);
                
            }, .payload = {i}
        });
 
        root.add({
            
            .tokens = { cmd, "unprotect" },
            .chelp  = { "Disables write protection" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                amiga.df[values[0]]->setFlag(DiskFlags::PROTECTED, false);
                
            }, .payload = {i}
        });
    }
    
    
    //
    // Peripherals (Hd0, Hd1, Hd2, Hd3)
    //

    root.add({

        .tokens = { "hd[n]" },
        .ghelp  = { "Hard drive n" },
        .chelp  = { "Commands: hd0, hd1, hd2, hd3" }
    });

    for (isize i = 0; i <= 3; i++) {
        
        cmd = registerComponent(*hd[i], true);

        root.add({
            
            .tokens = { cmd, "connect" },
            .chelp  = { "Connects the hard drive" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                emulator.set(Opt::HDC_CONNECT, true, values);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "disconnect" },
            .chelp  = { "Disconnects the hard drive" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                emulator.set(Opt::HDC_CONNECT, false, values);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "attach" },
            .chelp  = { "Attaches a hard drive image" },
            .args   = { { .name = { "path", "Hard drive image file" } } },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                                
                // Make sure the hard-drive controller board is plugged in
                emulator.set(Opt::HDC_CONNECT, true, values);

                // Connect the drive
                auto path = host.makeAbsolute(args.at("path"));
                amiga.hd[values[0]]->init(path);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "geometry" },
            .chelp  = { "Changes the disk geometry" },
            .args   = {
                { .name = { "cylinders", "Number of cylinders" }, .flags = rs::keyval },
                { .name = { "heads", "Number of drive heads" }, .flags = rs::keyval },
                { .name = { "sectors", "Number of sectors per cylinder" }, .flags = rs::keyval },
            },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                auto c = util::parseNum(args.at("cylinders"));
                auto h = util::parseNum(args.at("heads"));
                auto s = util::parseNum(args.at("sectors"));

                amiga.hd[values[0]]->changeGeometry(c, h, s);
                
            }, .payload = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "protect" },
            .chelp  = { "Enables write protection" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                amiga.hd[values[0]]->setFlag(DiskFlags::PROTECTED, true);
                
            }, .payload = {i}
        });
 
        root.add({
            
            .tokens = { cmd, "unprotect" },
            .chelp  = { "Disables write protection" },
            .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                
                amiga.hd[values[0]]->setFlag(DiskFlags::PROTECTED, false);
                
            }, .payload = {i}
        });
    }
    
    
    //
    // Miscellaneous
    //
    
    RSCommand::currentGroup = "Miscellaneous";
    
    
    //
    // Miscellaneous (Config)
    //
    
    root.add({
        
        .tokens = { "config" },
        .ghelp  = { "Virtual machine configuration" },
        .chelp  = { "Displays the current configuration" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            amiga.exportConfig(os);
        }
    });
    
    root.add({
        
        .tokens = { "config", "diff" },
        .chelp  = { "Displays the difference to the default configuration" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            amiga.exportDiff(os);
        }
    });
    
    
    //
    // Miscellaneous (DMA Debugger)
    //
    
    cmd = registerComponent(dmaDebugger);
    
    root.add({
        
        .tokens = { cmd, "open" },
        .chelp  = { "Opens the DMA debugger" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            emulator.set(Opt::DMA_DEBUG_ENABLE, true);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "close" },
        .chelp  = { "Closes the DMA debugger" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
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
        .chelp  = { "Sets the search path for media files" },
        .args   = { { .name = { "path", "File path" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            auto path = fs::path(args.at("path"));
            host.setSearchPath(path);
        }
    });
    
    
    //
    // Miscellaneous (Remote server)
    //
    
    
    root.add({
        
        .tokens = { "server" },
        .ghelp  = { "Remote connections" },
        .chelp  = { "Displays a server status summary" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            dump(os, remoteManager, Category::Status);
        }
    });
    
    cmd = registerComponent(remoteManager.serServer);
    
    cmd = registerComponent(remoteManager.rshServer);
    
    root.add({
        
        .tokens = { cmd, "start" },
        .chelp  = { "Starts the retro shell server" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            remoteManager.rshServer.start();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "stop" },
        .chelp  = { "Stops the retro shell server" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            remoteManager.rshServer.stop();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "disconnect" },
        .chelp  = { "Disconnects a client" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
                 
                 remoteManager.rshServer.disconnect();
             }
    });
    
    cmd = registerComponent(remoteManager.promServer);
    
    root.add({
        
        .tokens = { cmd, "start" },
        .chelp  = { "Starts the Prometheus server" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            remoteManager.promServer.start();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "stop" },
        .chelp  = { "Stops the Prometheus server" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            remoteManager.promServer.stop();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "disconnect" },
        .chelp  = { "Disconnects a client" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            remoteManager.promServer.disconnect();
        }
    });
    
    cmd = registerComponent(remoteManager.gdbServer);
    
    root.add({
        
        .tokens = { cmd, "attach" },
        .chelp  = { "Attaches the GDB server to a process" },
        .args   = { { .name = { "process", "Process number" } } },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            remoteManager.gdbServer.attach(args.at("process"));
        }
    });
    
    root.add({
        
        .tokens = { cmd, "detach" },
        .chelp  = { "Detaches the GDB server from a process" },
        .func   = [this] (std::ostream &os, const Arguments &args, const std::vector<isize> &values) {
            
            remoteManager.gdbServer.detach();
        }
    });
}

}
