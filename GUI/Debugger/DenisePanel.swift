// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension Inspector {
        
    func refreshDenise(everything: Bool, inspect: Bool = false) {
        
        guard let denise = amigaProxy?.denise else { return }

        // If requested, force an inspection before calling getInfo()
        if inspect { denise.inspect() }

        // Read the latest inspection record
        let info = denise.getInfo()
        let sprInfo = denise.getSpriteInfo(selectedSprite)
        
        if everything {

            let elements = [ deniseBPLCON0: fmt16,
                             deniseBPLCON1: fmt16,
                             deniseBPLCON2: fmt16,
                             deniseDIWSTRT: fmt16,
                             deniseDIWSTOP: fmt16,
                             deniseJOY0DAT: fmt16,
                             deniseJOY1DAT: fmt16,
                             deniseCLXDAT: fmt16,
                             sprPtr: fmt24
            ]
            for (c, f) in elements { assignFormatter(f, c!) }
        }
        
        //
        // Bitplane section
        //
        
        deniseBPLCON0.integerValue = Int(info.bplcon0)
        deniseHIRES.state = (info.bplcon0 & 0b1000000000000000 != 0) ? .on : .off
        deniseBPU.integerValue = Int(info.bpu)
        deniseBPUStepper.integerValue = Int(info.bpu)
        deniseHOMOD.state = (info.bplcon0 & 0b0000100000000000 != 0) ? .on : .off
        deniseDBPLF.state = (info.bplcon0 & 0b0000010000000000 != 0) ? .on : .off
        deniseLACE.state = (info.bplcon0 & 0b0000000000000100 != 0) ? .on : .off

        let p1H = (info.bplcon1 & 0b00001111)
        let p2H = (info.bplcon1 & 0b11110000) >> 4

        deniseBPLCON1.integerValue = Int(info.bplcon1)
        deniseP1H.integerValue = Int(p1H)
        deniseP1HStepper.integerValue = Int(p1H)
        deniseP2H.integerValue = Int(p2H)
        deniseP2HStepper.integerValue = Int(p2H)

        deniseBPLCON2.integerValue = Int(info.bplcon2)
        denisePF2PRI.state = (info.bplcon2 & 0b1000000 != 0) ? .on : .off
        denisePF2P2.state = (info.bplcon2 & 0b0100000 != 0) ? .on : .off
        denisePF2P1.state = (info.bplcon2 & 0b0010000 != 0) ? .on : .off
        denisePF2P0.state = (info.bplcon2 & 0b0001000 != 0) ? .on : .off
        denisePF1P2.state = (info.bplcon2 & 0b0000100 != 0) ? .on : .off
        denisePF1P1.state = (info.bplcon2 & 0b0000010 != 0) ? .on : .off
        denisePF1P0.state = (info.bplcon2 & 0b0000001 != 0) ? .on : .off

        //
        // Display window section
        //

        let hstrt = info.hstrt
        let vstrt = info.vstrt
        let hstop = info.hstop
        let vstop = info.vstop
        deniseDIWSTRT.integerValue = Int(info.diwstrt)
        deniseDIWSTRTText.stringValue = "(\(hstrt),\(vstrt))"
        deniseDIWSTOP.integerValue = Int(info.diwstop)
        deniseDIWSTOPText.stringValue = "(\(hstop),\(vstop))"

        //
        // Auxiliary register section
        //

        deniseJOY0DAT.integerValue = Int(info.joydat.0)
        deniseJOY1DAT.integerValue = Int(info.joydat.1)
        deniseCLXDAT.integerValue = Int(info.clxdat)

        //
        // Sprite section
        //

        sprHStart.integerValue = Int(sprInfo.hstrt)
        sprVStart.integerValue = Int(sprInfo.vstrt)
        sprVStop.integerValue = Int(sprInfo.vstop)
        sprPtr.integerValue = Int(sprInfo.ptr)
        sprAttach.state = sprInfo.attach ? .on : .off
        sprTableView.refresh(everything: everything)

        //
        // Color section
        //

        deniseCol0.color = NSColor.init(amigaRGB: info.colorReg.0)
        deniseCol1.color = NSColor.init(amigaRGB: info.colorReg.1)
        deniseCol2.color = NSColor.init(amigaRGB: info.colorReg.2)
        deniseCol3.color = NSColor.init(amigaRGB: info.colorReg.3)
        deniseCol4.color = NSColor.init(amigaRGB: info.colorReg.4)
        deniseCol5.color = NSColor.init(amigaRGB: info.colorReg.5)
        deniseCol6.color = NSColor.init(amigaRGB: info.colorReg.6)
        deniseCol7.color = NSColor.init(amigaRGB: info.colorReg.7)
        deniseCol8.color = NSColor.init(amigaRGB: info.colorReg.8)
        deniseCol9.color = NSColor.init(amigaRGB: info.colorReg.9)
        deniseCol10.color = NSColor.init(amigaRGB: info.colorReg.10)
        deniseCol11.color = NSColor.init(amigaRGB: info.colorReg.11)
        deniseCol12.color = NSColor.init(amigaRGB: info.colorReg.12)
        deniseCol13.color = NSColor.init(amigaRGB: info.colorReg.13)
        deniseCol14.color = NSColor.init(amigaRGB: info.colorReg.14)
        deniseCol15.color = NSColor.init(amigaRGB: info.colorReg.15)
        deniseCol16.color = NSColor.init(amigaRGB: info.colorReg.16)
        deniseCol17.color = NSColor.init(amigaRGB: info.colorReg.17)
        deniseCol18.color = NSColor.init(amigaRGB: info.colorReg.18)
        deniseCol19.color = NSColor.init(amigaRGB: info.colorReg.19)
        deniseCol20.color = NSColor.init(amigaRGB: info.colorReg.20)
        deniseCol21.color = NSColor.init(amigaRGB: info.colorReg.21)
        deniseCol22.color = NSColor.init(amigaRGB: info.colorReg.22)
        deniseCol23.color = NSColor.init(amigaRGB: info.colorReg.23)
        deniseCol24.color = NSColor.init(amigaRGB: info.colorReg.24)
        deniseCol25.color = NSColor.init(amigaRGB: info.colorReg.25)
        deniseCol26.color = NSColor.init(amigaRGB: info.colorReg.26)
        deniseCol27.color = NSColor.init(amigaRGB: info.colorReg.27)
        deniseCol28.color = NSColor.init(amigaRGB: info.colorReg.28)
        deniseCol29.color = NSColor.init(amigaRGB: info.colorReg.29)
        deniseCol30.color = NSColor.init(amigaRGB: info.colorReg.30)
        deniseCol31.color = NSColor.init(amigaRGB: info.colorReg.31)
    }

    @IBAction func deniseBPLCON0Action(_ sender: NSTextField!) {

        amigaProxy?.denise.setBPLCONx(0, value: sender!.integerValue)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func deniseBPLCON0ButtonAction(_ sender: NSButton!) {

        let value = sender.state == .on
        amigaProxy?.denise.setBPLCONx(0, bit: sender.tag, value: value)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func deniseBPUAction(_ sender: NSTextField!) {

        amigaProxy?.denise.setBPU(sender.integerValue)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func deniseBPUStepperAction(_ sender: NSStepper!) {

        amigaProxy?.denise.setBPU(sender.integerValue)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func deniseBPLCON1Action(_ sender: NSTextField!) {

        amigaProxy?.denise.setBPLCONx(1, value: sender!.integerValue)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func deniseBPLCON2NibbleAction(_ sender: NSTextField!) {

        track("\(sender.tag) \(sender.integerValue)")
        amigaProxy?.denise.setBPLCONx(1, nibble: sender.tag, value: sender.integerValue)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func deniseBPLCON1StepperAction(_ sender: NSStepper!) {

        // track("\(sender.tag) \(sender.integerValue)")
        amigaProxy?.denise.setBPLCONx(1, nibble: sender.tag, value: sender.integerValue)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func deniseBPLCON2Action(_ sender: NSTextField!) {

        amigaProxy?.denise.setBPLCONx(2, value: sender!.integerValue)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func deniseBPLCON2ButtonAction(_ sender: NSButton!) {

        let value = sender.state == .on
        amigaProxy?.denise.setBPLCONx(2, bit: sender.tag, value: value)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func deniseColorAction(_ sender: NSColorWell!) {

        let color = sender.color.amigaRGB()
        amigaProxy?.denise.pokeColorReg(sender.tag, value: color)
        refreshDenise(everything: false, inspect: true)
    }

    @IBAction func selectSpriteAction(_ sender: Any!) {
        
        refreshDenise(everything: false, inspect: true)
    }
}
