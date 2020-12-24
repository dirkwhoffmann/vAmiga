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

    func isMouse() -> Bool {
        
        let key = kIOHIDPrimaryUsageKey as CFString
        
        if let value = IOHIDDeviceGetProperty(self, key) as? Int {
            return value == kHIDUsage_GD_Mouse
        } else {
            return false
        }
    }
    
    func listProperties() {
        
        let keys = [
            
            kIOHIDTransportKey, kIOHIDVendorIDKey, kIOHIDVendorIDSourceKey,
            kIOHIDProductIDKey, kIOHIDVersionNumberKey, kIOHIDManufacturerKey,
            kIOHIDProductKey, kIOHIDSerialNumberKey, kIOHIDCountryCodeKey,
            kIOHIDStandardTypeKey, kIOHIDLocationIDKey, kIOHIDDeviceUsageKey,
            kIOHIDDeviceUsagePageKey, kIOHIDDeviceUsagePairsKey,
            kIOHIDPrimaryUsageKey, kIOHIDPrimaryUsagePageKey,
            kIOHIDMaxInputReportSizeKey, kIOHIDMaxOutputReportSizeKey,
            kIOHIDMaxFeatureReportSizeKey, kIOHIDReportIntervalKey,
            kIOHIDSampleIntervalKey, kIOHIDBatchIntervalKey,
            kIOHIDRequestTimeoutKey, kIOHIDReportDescriptorKey,
            kIOHIDResetKey, kIOHIDKeyboardLanguageKey, kIOHIDAltHandlerIdKey,
            kIOHIDBuiltInKey, kIOHIDDisplayIntegratedKey, kIOHIDProductIDMaskKey,
            kIOHIDProductIDArrayKey, kIOHIDPowerOnDelayNSKey, kIOHIDCategoryKey,
            kIOHIDMaxResponseLatencyKey, kIOHIDUniqueIDKey,
            kIOHIDPhysicalDeviceUniqueIDKey
        ]
        
        for key in keys {
            if let prop = property(key: key) {
                print("\t" + key + ": \(prop)")
            }
        }
    }
}
