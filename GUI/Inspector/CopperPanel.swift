// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
    
    func refreshCopper(everything: Bool) {
        
        guard let dma = amigaProxy?.agnus else { return }
        let info = dma.getCopperInfo()
        
        if everything {

            let elements = [ copCOPPC: fmt24,
                             copCOPINS1: fmt16,
                             copCOPINS2: fmt16,
                             copCOP1LC: fmt24,
                             copCOP2LC: fmt24
            ]
            for (c, f) in elements { assignFormatter(f, c!) }
        }
        
        copActive.state = info.active ? .on : .off
        copCDANG.state = info.cdang ? .on : .off
        copCOPPC.integerValue = Int(info.coppc)
        copCOPINS1.integerValue = Int(info.cop1ins)
        copCOPINS2.integerValue = Int(info.cop2ins)
        copCOP1LC.integerValue = Int(info.cop1lc)
        copCOP2LC.integerValue = Int(info.cop2lc)

        copList1.refresh(everything: everything)
        copList2.refresh(everything: everything)
    }
 
    @IBAction func copCOPLCAction(_ sender: NSTextField!) {
        
        track()
    }
    
}
