// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension IOHIDDevice {
    
    func property(key: String) -> String? {
            
        if let prop = IOHIDDeviceGetProperty(self, key as CFString) {
            return "\(prop)"
        }
        return nil
    }
    
    var vendorID: String { return property(key: kIOHIDVendorIDKey) ?? "" }
    
    var productID: String { return property(key: kIOHIDProductIDKey) ?? "" }

    var locationID: String { return property(key: kIOHIDLocationIDKey) ?? "" }
}
