// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    private func cacheAgnus() {

        agnusInfo = emu.paused ? emu.agnus.info : emu.agnus.cachedInfo
    }

    func refreshAgnus(count: Int = 0, full: Bool = false) {

        cacheAgnus()
        
        if full {
            let elements = [ dmaVPOS: fmt16,
                             dmaHPOS: fmt16,
                             
                           dmaDMACON: fmt16,
                          dmaBPL0CON: fmt16,
                          dmaDDFSTRT: fmt16,
                          dmaDDFSTOP: fmt16,
                          dmaDIWSTRT: fmt16,
                          dmaDIWSTOP: fmt16,
                             
                          dmaBLTAMOD: fmt16,
                          dmaBLTBMOD: fmt16,
                          dmaBLTCMOD: fmt16,
                          dmaBLTDMOD: fmt16,
                          dmaBPL1MOD: fmt16,
                          dmaBPL2MOD: fmt16,
                             
                           dmaBPL1PT: fmt24,
                           dmaBPL2PT: fmt24,
                           dmaBPL3PT: fmt24,
                           dmaBPL4PT: fmt24,
                           dmaBPL5PT: fmt24,
                           dmaBPL6PT: fmt24,
                             
                           dmaAUD0PT: fmt24,
                           dmaAUD1PT: fmt24,
                           dmaAUD2PT: fmt24,
                           dmaAUD3PT: fmt24,
                           dmaAUD0LC: fmt24,
                           dmaAUD1LC: fmt24,
                           dmaAUD2LC: fmt24,
                           dmaAUD3LC: fmt24,
                             
                           dmaBLTAPT: fmt24,
                           dmaBLTBPT: fmt24,
                           dmaBLTCPT: fmt24,
                           dmaBLTDPT: fmt24,
                             
                            dmaCOPPC: fmt24,
                             
                           dmaSPR0PT: fmt24,
                           dmaSPR1PT: fmt24,
                           dmaSPR2PT: fmt24,
                           dmaSPR3PT: fmt24,
                           dmaSPR4PT: fmt24,
                           dmaSPR5PT: fmt24,
                           dmaSPR6PT: fmt24,
                           dmaSPR7PT: fmt24,
                             
                            dmaDSKPT: fmt24
            ]
            for (c, f) in elements { assignFormatter(f, c!) }
        }

        dmaVPOS.integerValue = Int(agnusInfo.vpos)
        dmaHPOS.integerValue = Int(agnusInfo.hpos)

        let dmacon = Int(agnusInfo.dmacon)
        let bplcon = Int(agnusInfo.bplcon0)
        let bltcon = Int(agnusInfo.bltcon0)

        dmaDMACON.integerValue = dmacon
        let bltpri  = (dmacon & 0b0000010000000000) != 0
        let dmaen   = (dmacon & 0b0000001000000000) != 0
        let bplen   = (dmacon & 0b0000000100000000) != 0 && dmaen
        let copen   = (dmacon & 0b0000000010000000) != 0 && dmaen
        let blten   = (dmacon & 0b0000000001000000) != 0 && dmaen
        let spren   = (dmacon & 0b0000000000100000) != 0 && dmaen
        let dsken   = (dmacon & 0b0000000000010000) != 0 && dmaen
        let aud3en  = (dmacon & 0b0000000000001000) != 0 && dmaen
        let aud2en  = (dmacon & 0b0000000000000100) != 0 && dmaen
        let aud1en  = (dmacon & 0b0000000000000010) != 0 && dmaen
        let aud0en  = (dmacon & 0b0000000000000001) != 0 && dmaen
        let bltA    = (bltcon & 0b0000100000000000) != 0 && blten
        let bltB    = (bltcon & 0b0000010000000000) != 0 && blten
        let bltC    = (bltcon & 0b0000001000000000) != 0 && blten
        let bltD    = (bltcon & 0b0000000100000000) != 0 && blten

        dmaBPL0CON.integerValue = bplcon
        let bpu     = (bplcon >> 12) & 0b111

        dmaDIWSTRT.integerValue = Int(agnusInfo.diwstrt)
        dmaDIWSTOP.integerValue = Int(agnusInfo.diwstop)
        dmaDDFSTRT.integerValue = Int(agnusInfo.ddfstrt)
        dmaDDFSTOP.integerValue = Int(agnusInfo.ddfstop)

        dmaBLTAMOD.integerValue = Int(agnusInfo.bltamod)
        dmaBLTBMOD.integerValue = Int(agnusInfo.bltbmod)
        dmaBLTCMOD.integerValue = Int(agnusInfo.bltcmod)
        dmaBLTDMOD.integerValue = Int(agnusInfo.bltdmod)
        dmaBPL1MOD.integerValue = Int(agnusInfo.bpl1mod)
        dmaBPL2MOD.integerValue = Int(agnusInfo.bpl2mod)

        dmaBPL1PT.integerValue = Int(agnusInfo.bplpt.0)
        dmaBPL2PT.integerValue = Int(agnusInfo.bplpt.1)
        dmaBPL3PT.integerValue = Int(agnusInfo.bplpt.2)
        dmaBPL4PT.integerValue = Int(agnusInfo.bplpt.3)
        dmaBPL5PT.integerValue = Int(agnusInfo.bplpt.4)
        dmaBPL6PT.integerValue = Int(agnusInfo.bplpt.5)
        dmaBPL1Enable.state = bplen && dmaen && bpu >= 1 ? .on : .off
        dmaBPL2Enable.state = bplen && dmaen && bpu >= 2 ? .on : .off
        dmaBPL3Enable.state = bplen && bpu >= 3 ? .on : .off
        dmaBPL4Enable.state = bplen && bpu >= 4 ? .on : .off
        dmaBPL5Enable.state = bplen && bpu >= 5 ? .on : .off
        dmaBPL6Enable.state = bplen && bpu >= 6 ? .on : .off

        dmaAUD0PT.integerValue = Int(agnusInfo.audpt.0)
        dmaAUD1PT.integerValue = Int(agnusInfo.audpt.1)
        dmaAUD2PT.integerValue = Int(agnusInfo.audpt.2)
        dmaAUD3PT.integerValue = Int(agnusInfo.audpt.3)
        dmaAUD0Enable.state = aud0en ? .on : .off
        dmaAUD1Enable.state = aud1en ? .on : .off
        dmaAUD2Enable.state = aud2en ? .on : .off
        dmaAUD3Enable.state = aud3en ? .on : .off
        dmaAUD0LC.integerValue = Int(agnusInfo.audlc.0)
        dmaAUD1LC.integerValue = Int(agnusInfo.audlc.1)
        dmaAUD2LC.integerValue = Int(agnusInfo.audlc.2)
        dmaAUD3LC.integerValue = Int(agnusInfo.audlc.3)

        dmaBLTAPT.integerValue = Int(agnusInfo.bltpt.0)
        dmaBLTBPT.integerValue = Int(agnusInfo.bltpt.1)
        dmaBLTCPT.integerValue = Int(agnusInfo.bltpt.2)
        dmaBLTDPT.integerValue = Int(agnusInfo.bltpt.3)
        dmaBLTAEnable.state = bltA ? .on : .off
        dmaBLTBEnable.state = bltB ? .on : .off
        dmaBLTCEnable.state = bltC ? .on : .off
        dmaBLTDEnable.state = bltD ? .on : .off
        dmaBLTPRI.state = bltpri ? .on : .off
        dmaBLS.state = agnusInfo.bls ? .on : .off

        dmaCOPPC.integerValue = Int(agnusInfo.coppc0)
        dmaCOPEnable.state = copen ? .on : .off

        dmaSPR0PT.integerValue = Int(agnusInfo.sprpt.0)
        dmaSPR1PT.integerValue = Int(agnusInfo.sprpt.1)
        dmaSPR2PT.integerValue = Int(agnusInfo.sprpt.2)
        dmaSPR3PT.integerValue = Int(agnusInfo.sprpt.3)
        dmaSPR4PT.integerValue = Int(agnusInfo.sprpt.4)
        dmaSPR5PT.integerValue = Int(agnusInfo.sprpt.5)
        dmaSPR6PT.integerValue = Int(agnusInfo.sprpt.6)
        dmaSPR7PT.integerValue = Int(agnusInfo.sprpt.7)
        dmaSPR0Enable.state = spren ? .on : .off
        dmaSPR1Enable.state = spren ? .on : .off
        dmaSPR2Enable.state = spren ? .on : .off
        dmaSPR3Enable.state = spren ? .on : .off
        dmaSPR4Enable.state = spren ? .on : .off
        dmaSPR5Enable.state = spren ? .on : .off
        dmaSPR6Enable.state = spren ? .on : .off
        dmaSPR7Enable.state = spren ? .on : .off

        dmaDSKPT.integerValue = Int(agnusInfo.dskpt)
        dmaDSKEnable.state = dsken ? .on : .off
    }
}
