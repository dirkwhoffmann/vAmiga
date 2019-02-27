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
        
        guard let dma = amigaProxy?.dma else { return }
        let info = dma.getCopperInfo()
        
        track("Refreshing Copper inspector tab")
        
        if everything {
            
            for (c,f) in [ copCOPPC:   fmt24,
                           copCOPINS1: fmt16,
                           copCOPINS2: fmt16,
                           copCOP1LC:  fmt24,
                           copCOP2LC:  fmt24,
                           ]
            {
                assignFormatter(f, c!)
            }
        }
        
        copActive.state = info.active ? .on : .off
        copCDANG.state = info.cdang ? .on : .off
        copCOPPC.integerValue = Int(info.coppc)
        copCOPINS1.integerValue = Int(info.copins.0)
        copCOPINS2.integerValue = Int(info.copins.1)
        copCOP1LC.integerValue = Int(info.coplc.0)
        copCOP2LC.integerValue = Int(info.coplc.1)

        copList1.refresh(everything: everything)
        copList2.refresh(everything: everything)
    }
 
    @IBAction func copCOPLCAction(_ sender: NSTextField!) {
        
        track()
    }
    
}
