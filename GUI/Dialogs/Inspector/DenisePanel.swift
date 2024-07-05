// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    private func cacheDenise() {

        deniseInfo = emu.paused ? emu.denise.info : emu.denise.cachedInfo
        spriteInfo = emu.denise.getSpriteInfo(selectedSprite)
    }

    func refreshDenise(count: Int = 0, full: Bool = false) {

        cacheDenise()

        if full {

            let elements = [ deniseBPLCON0: fmt16,
                             deniseBPLCON1: fmt16,
                             deniseBPLCON2: fmt16,
                             deniseDIWSTRT: fmt16,
                             deniseDIWSTOP: fmt16,
                             deniseCLXDAT: fmt16
            ]
            for (c, f) in elements { assignFormatter(f, c!) }
        }

        // Bitplane section
        let bplcon0 = Int(deniseInfo.bplcon0)
        let bplcon1 = Int(deniseInfo.bplcon1)
        let bplcon2 = Int(deniseInfo.bplcon2)
        let bpu     = Int(deniseInfo.bpu)

        deniseBPLCON0.integerValue = bplcon0
        deniseHIRES.state = (bplcon0 & 0b1000000000000000 != 0) ? .on : .off
        deniseBPU.integerValue = bpu
        deniseHOMOD.state = (bplcon0 & 0b0000100000000000 != 0) ? .on : .off
        deniseDBPLF.state = (bplcon0 & 0b0000010000000000 != 0) ? .on : .off
        deniseLACE.state = (bplcon0 & 0b0000000000000100 != 0) ? .on : .off
        deniseSHRES.state = (bplcon0 & 0b0000000001000000 != 0) ? .on : .off
        deniseSHRES.isEnabled = deniseInfo.ecs

        deniseBPLCON1.integerValue = bplcon1
        deniseP1H.integerValue = (bplcon1 & 0b00001111)
        deniseP2H.integerValue = (bplcon1 & 0b11110000) >> 4

        deniseBPLCON2.integerValue = bplcon2
        denisePF2PRI.state = (bplcon2 & 0b1000000 != 0) ? .on : .off
        denisePF2P2.state  = (bplcon2 & 0b0100000 != 0) ? .on : .off
        denisePF2P1.state  = (bplcon2 & 0b0010000 != 0) ? .on : .off
        denisePF2P0.state  = (bplcon2 & 0b0001000 != 0) ? .on : .off
        denisePF1P2.state  = (bplcon2 & 0b0000100 != 0) ? .on : .off
        denisePF1P1.state  = (bplcon2 & 0b0000010 != 0) ? .on : .off
        denisePF1P0.state  = (bplcon2 & 0b0000001 != 0) ? .on : .off

        // Display window section
        let hstrt = deniseInfo.viewport.hstrt
        let vstrt = deniseInfo.viewport.vstrt
        let hstop = deniseInfo.viewport.hstop
        let vstop = deniseInfo.viewport.vstop
        deniseDIWSTRT.integerValue = Int(deniseInfo.diwstrt)
        deniseDIWSTRTText.stringValue = "(\(hstrt),\(vstrt))"
        deniseDIWSTOP.integerValue = Int(deniseInfo.diwstop)
        deniseDIWSTOPText.stringValue = "(\(hstop),\(vstop))"

        // Auxiliary register section
        deniseCLXDAT.integerValue = Int(deniseInfo.clxdat)

        // Sprite section
        sprArmed1.state = emu.denise.sprDataLines(0) != 0 ? .on : .off
        sprArmed2.state = emu.denise.sprDataLines(1) != 0 ? .on : .off
        sprArmed3.state = emu.denise.sprDataLines(2) != 0 ? .on : .off
        sprArmed4.state = emu.denise.sprDataLines(3) != 0 ? .on : .off
        sprArmed5.state = emu.denise.sprDataLines(4) != 0 ? .on : .off
        sprArmed6.state = emu.denise.sprDataLines(5) != 0 ? .on : .off
        sprArmed7.state = emu.denise.sprDataLines(6) != 0 ? .on : .off
        sprArmed8.state = emu.denise.sprDataLines(7) != 0 ? .on : .off

        sprHStart.integerValue = Int(spriteInfo.hstrt)
        sprVStart.integerValue = Int(spriteInfo.vstrt)
        sprVStop.integerValue = Int(spriteInfo.vstop)
        sprAttach.state = spriteInfo.attach ? .on : .off

        // Color section
        deniseCol0.color = NSColor(amigaRGB: deniseInfo.colorReg.0)
        deniseCol1.color = NSColor(amigaRGB: deniseInfo.colorReg.1)
        deniseCol2.color = NSColor(amigaRGB: deniseInfo.colorReg.2)
        deniseCol3.color = NSColor(amigaRGB: deniseInfo.colorReg.3)
        deniseCol4.color = NSColor(amigaRGB: deniseInfo.colorReg.4)
        deniseCol5.color = NSColor(amigaRGB: deniseInfo.colorReg.5)
        deniseCol6.color = NSColor(amigaRGB: deniseInfo.colorReg.6)
        deniseCol7.color = NSColor(amigaRGB: deniseInfo.colorReg.7)
        deniseCol8.color = NSColor(amigaRGB: deniseInfo.colorReg.8)
        deniseCol9.color = NSColor(amigaRGB: deniseInfo.colorReg.9)
        deniseCol10.color = NSColor(amigaRGB: deniseInfo.colorReg.10)
        deniseCol11.color = NSColor(amigaRGB: deniseInfo.colorReg.11)
        deniseCol12.color = NSColor(amigaRGB: deniseInfo.colorReg.12)
        deniseCol13.color = NSColor(amigaRGB: deniseInfo.colorReg.13)
        deniseCol14.color = NSColor(amigaRGB: deniseInfo.colorReg.14)
        deniseCol15.color = NSColor(amigaRGB: deniseInfo.colorReg.15)
        deniseCol16.color = NSColor(amigaRGB: deniseInfo.colorReg.16)
        deniseCol17.color = NSColor(amigaRGB: deniseInfo.colorReg.17)
        deniseCol18.color = NSColor(amigaRGB: deniseInfo.colorReg.18)
        deniseCol19.color = NSColor(amigaRGB: deniseInfo.colorReg.19)
        deniseCol20.color = NSColor(amigaRGB: deniseInfo.colorReg.20)
        deniseCol21.color = NSColor(amigaRGB: deniseInfo.colorReg.21)
        deniseCol22.color = NSColor(amigaRGB: deniseInfo.colorReg.22)
        deniseCol23.color = NSColor(amigaRGB: deniseInfo.colorReg.23)
        deniseCol24.color = NSColor(amigaRGB: deniseInfo.colorReg.24)
        deniseCol25.color = NSColor(amigaRGB: deniseInfo.colorReg.25)
        deniseCol26.color = NSColor(amigaRGB: deniseInfo.colorReg.26)
        deniseCol27.color = NSColor(amigaRGB: deniseInfo.colorReg.27)
        deniseCol28.color = NSColor(amigaRGB: deniseInfo.colorReg.28)
        deniseCol29.color = NSColor(amigaRGB: deniseInfo.colorReg.29)
        deniseCol30.color = NSColor(amigaRGB: deniseInfo.colorReg.30)
        deniseCol31.color = NSColor(amigaRGB: deniseInfo.colorReg.31)

        sprTableView.refresh(count: count, full: full)
    }

    @IBAction func selectSpriteAction(_ sender: Any!) {

        fullRefresh()
    }
}
