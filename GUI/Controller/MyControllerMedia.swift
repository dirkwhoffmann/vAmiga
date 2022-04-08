// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyDocument {
    
    func attach(hd n: Int, hdf: HDFFileProxy? = nil, force: Bool = false) throws {
        
        var hdn: HardDriveProxy { return amiga.hd(n)! }

        func attach() throws {
                      
            amiga.configure(.HDR_CONNECT, drive: n, enable: true)
            if let proxy = hdf { try hdn.attach(hdf: proxy) }
        }
        
        if force || proceedWithUnsavedHardDisk(drive: hdn) {
            
            if amiga.poweredOff {
                
                try attach()
                
            } else if force || askToPowerOff() {
                
                amiga.powerOff()
                try attach()
                amiga.powerOn()
                try amiga.run()
            }
        }
    }
    
    func detach(hd n: Int, force: Bool = false) throws {
        
        var hdn: HardDriveProxy { return amiga.hd(n)! }

        func detach() throws {
                      
            amiga.configure(.HDR_CONNECT, drive: n, enable: false)
        }
        
        if force || proceedWithUnsavedHardDisk(drive: hdn) {
            
            if amiga.poweredOff {
                
                try detach()
                
            } else if force || askToPowerOff() {
                
                amiga.powerOff()
                try detach()
                amiga.powerOn()
                try amiga.run()
            }
        }
    }
}
