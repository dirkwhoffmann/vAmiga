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
        
    func refreshDenise(everything: Bool) {
        
        guard let denise = amigaProxy?.denise else { return }
        
        let info = denise.getInfo()
        let sprInfo = denise.getSpriteInfo(selectedSprite)
        
        if everything {

            let elements = [ deniseBPLCON0: fmt24,
                             deniseBPLCON1: fmt24,
                             deniseBPLCON2: fmt24,
                             sprPtr: fmt24
            ]
            for (c, f) in elements { assignFormatter(f, c!) }
        }
        
        //
        // Bitplane section
        //
        
        deniseBPLCON0.integerValue = Int(info.bplcon0)
        deniseHIRES.state = (info.bplcon0 & 0b1000000000000000 != 0) ? .on : .off
        deniseHOMOD.state = (info.bplcon0 & 0b0000100000000000 != 0) ? .on : .off
        deniseDBPLF.state = (info.bplcon0 & 0b0000010000000000 != 0) ? .on : .off
        deniseLACE.state  = (info.bplcon0 & 0b0000000000000100 != 0) ? .on : .off
        deniseBPLCON1.integerValue = Int(info.bplcon1)
        deniseBPLCON2.integerValue = Int(info.bplcon2)

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

        //
        // Sprite section
        //
        
        sprHStart.integerValue = Int(sprInfo.hstrt)
        sprVStart.integerValue = Int(sprInfo.vstrt)
        sprVStop.integerValue = Int(sprInfo.vstop)
        sprPtr.integerValue = Int(sprInfo.ptr)
        sprAttach.state = sprInfo.attach ? .on : .off
        sprTableView.refresh(everything: everything)
    }
    
    @IBAction func deniseBPLCON0ButtonAction(_ sender: NSButton!) {
        
        amigaProxy?.denise.setBPLCON0Bit(sender.tag, value: sender.state == .on)
        refresh(everything: false)
    }

    @IBAction func deniseColorAction(_ sender: NSColorWell!) {

        let color = sender.color.amigaRGB()

        // track("\(sender.tag) \(color)")

        amigaProxy?.denise.pokeColorReg(sender.tag, value: color)
        refreshDenise(everything: false)
    }

    @IBAction func selectSpriteAction(_ sender: Any!) {
        
        refreshDenise(everything: true)
    }
}
