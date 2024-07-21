// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyDocument {

    func insert(df n: Int, file: MediaFileProxy, force: Bool = false) throws {

        var dfn: FloppyDriveProxy { return amiga.df(n)! }

        if force || proceedWithUnsavedFloppyDisk(drive: dfn) {

            try dfn.swap(file: file)
        }
    }

    func attach(hd n: Int, file: MediaFileProxy? = nil, force: Bool = false) throws {

        var hdn: HardDriveProxy { return amiga.hd(n)! }

        func attach() throws {

            amiga.set(.HDC_CONNECT, drive: n, enable: true)
            if let proxy = file { try hdn.attach(file: proxy) }
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
                      
            amiga.set(.HDC_CONNECT, drive: n, enable: false)
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
