// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    private func cacheBlitter() {

        blitterInfo = emu.paused ? emu.blitter.info : emu.blitter.cachedInfo
    }

    func refreshBlitter(count: Int = 0, full: Bool = false) {

        cacheBlitter()
        
        if full {
            let elements = [ bltBLTCON0a: Inspector.fmt4,
                             bltBLTCON0b: Inspector.fmt4,
                             bltBLTCON0c: Inspector.fmt8,
                             bltBLTCON1a: Inspector.fmt4,
                             bltBLTCON1b: Inspector.fmt4,
                             bltBLTCON1c: Inspector.fmt8,
                                bltAhold: Inspector.fmt16,
                                bltBhold: Inspector.fmt16,
                                bltChold: Inspector.fmt16,
                                bltDhold: Inspector.fmt16,
                                 bltAold: Inspector.fmt16,
                                 bltBold: Inspector.fmt16,
                                 bltAnew: Inspector.fmt16,
                                 bltBnew: Inspector.fmt16,
                               bltMaskIn: Inspector.fmt16b,
                                 bltAFWM: Inspector.fmt16b,
                                 bltALWM: Inspector.fmt16b,
                              bltMaskOut: Inspector.fmt16b,
                            bltBarrelAIn: Inspector.fmt16b,
                         bltBarrelAShift: Inspector.fmt4,
                           bltBarrelAOut: Inspector.fmt16b,
                            bltBarrelBIn: Inspector.fmt16b,
                         bltBarrelBShift: Inspector.fmt4,
                           bltBarrelBOut: Inspector.fmt16b,
                               bltFillIn: Inspector.fmt16b,
                              bltFillOut: Inspector.fmt16b,
                                  bltLFA: Inspector.fmt16b,
                                  bltLFB: Inspector.fmt16b,
                                  bltLFC: Inspector.fmt16b,
                               bltLF0Val: Inspector.fmt16b,
                               bltLF1Val: Inspector.fmt16b,
                               bltLF2Val: Inspector.fmt16b,
                               bltLF3Val: Inspector.fmt16b,
                               bltLF4Val: Inspector.fmt16b,
                               bltLF5Val: Inspector.fmt16b,
                               bltLF6Val: Inspector.fmt16b,
                               bltLF7Val: Inspector.fmt16b,
                                  bltLFD: Inspector.fmt16b

            ]
            for (c, f) in elements { assignFormatter(f, c!) }
        }

        let bltcon0 = Int(blitterInfo.bltcon0)
        bltBLTCON0a.integerValue = Int(blitterInfo.ash)
        bltBLTCON0b.integerValue = (bltcon0 >> 8) & 0xF
        bltBLTCON0c.integerValue = bltcon0 & 0xFF
        
        let bltcon1 = Int(blitterInfo.bltcon1)
        bltBLTCON1a.integerValue = Int(blitterInfo.bsh)
        bltBLTCON1b.integerValue = (bltcon1 >> 8) & 0xF
        bltBLTCON1c.integerValue = bltcon1 & 0xFF
        bltEFE.state = bltcon1 & 0x10 != 0 ? .on : .off
        bltIFE.state = bltcon1 & 0x08 != 0 ? .on : .off
        bltFCI.state = bltcon1 & 0x04 != 0 ? .on : .off
        bltDESC.state = bltcon1 & 0x02 != 0 ? .on : .off
        bltLINE.state = bltcon1 & 0x01 != 0 ? .on : .off

        bltBBUSY.state = blitterInfo.bbusy ? .on : .off
        
        bltUseA.state = bltcon0 & 0x800 != 0 ? .on : .off
        bltUseB.state = bltcon0 & 0x400 != 0 ? .on : .off
        bltUseC.state = bltcon0 & 0x200 != 0 ? .on : .off
        bltUseD.state = bltcon0 & 0x100 != 0 ? .on : .off
        bltAhold.integerValue = Int(blitterInfo.ahold)
        bltBhold.integerValue = Int(blitterInfo.bhold)
        bltChold.integerValue = Int(blitterInfo.chold)
        bltDhold.integerValue = Int(blitterInfo.dhold)
        bltAold.integerValue = Int(blitterInfo.aold)
        bltBold.integerValue = Int(blitterInfo.bold)
        bltAnew.integerValue = Int(blitterInfo.anew)
        bltBnew.integerValue = Int(blitterInfo.bnew)
        bltBZERO.state = blitterInfo.bzero ? .on : .off
        
        bltMaskIn.integerValue = Int(blitterInfo.anew)
        bltAFWM.integerValue = Int(blitterInfo.bltafwm)
        bltALWM.integerValue = Int(blitterInfo.bltalwm)
        bltMaskOut.integerValue = Int(blitterInfo.aold)

        bltFirstWord.state = blitterInfo.firstWord ? .on : .off
        bltLastWord.state = blitterInfo.lastWord ? .on : .off

        bltBarrelAIn.integerValue = Int(blitterInfo.barrelAin)
        bltBarrelAShift.integerValue = Int(blitterInfo.ash)
        bltBarrelAOut.integerValue = Int(blitterInfo.barrelAout)
        bltBarrelBIn.integerValue = Int(blitterInfo.barrelBin)
        bltBarrelBShift.integerValue = Int(blitterInfo.bsh)
        bltBarrelBOut.integerValue = Int(blitterInfo.barrelBout)

        bltFillIn.integerValue = Int(blitterInfo.fillIn)
        bltFillOut.integerValue = Int(blitterInfo.fillOut)

        let a = Int(blitterInfo.ahold)
        let b = Int(blitterInfo.bhold)
        let c = Int(blitterInfo.chold)
        let d = Int(blitterInfo.dhold)
        let lf7 = bltcon0 & 0x80 != 0
        let lf6 = bltcon0 & 0x40 != 0
        let lf5 = bltcon0 & 0x20 != 0
        let lf4 = bltcon0 & 0x10 != 0
        let lf3 = bltcon0 & 0x08 != 0
        let lf2 = bltcon0 & 0x04 != 0
        let lf1 = bltcon0 & 0x02 != 0
        let lf0 = bltcon0 & 0x01 != 0
        bltLFA.integerValue = a
        bltLFB.integerValue = b
        bltLFC.integerValue = c
        bltLFD.integerValue = d
        bltLF7.state = lf7 ? .on : .off
        bltLF6.state = lf6 ? .on : .off
        bltLF5.state = lf5 ? .on : .off
        bltLF4.state = lf4 ? .on : .off
        bltLF3.state = lf3 ? .on : .off
        bltLF2.state = lf2 ? .on : .off
        bltLF1.state = lf1 ? .on : .off
        bltLF0.state = lf0 ? .on : .off
        bltLF7Val.integerValue = ( a &  b &  c)
        bltLF6Val.integerValue = ( a &  b & ~c)
        bltLF5Val.integerValue = ( a & ~b &  c)
        bltLF4Val.integerValue = ( a & ~b & ~c)
        bltLF3Val.integerValue = (~a &  b &  c)
        bltLF2Val.integerValue = (~a &  b & ~c)
        bltLF1Val.integerValue = (~a & ~b &  c)
        bltLF0Val.integerValue = (~a & ~b & ~c)
    }
}
