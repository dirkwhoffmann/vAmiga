// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyAppDelegate {
    
    @objc func vamResetCommand(_ notification: Notification) {
        
        if !resetScriptCmd(arguments: notification.userInfo) {
            track("Failed to execute 'reset' command.")
        }
    }
     
    @objc func vamMountCommand(_ notification: Notification) {
        
        if !mountScriptCmd(arguments: notification.userInfo) {
            track("Failed to execute 'mount' command.")
        }
    }
    
    @objc func vamTypeTextCommand(_ notification: Notification) {
        
        if !typeTextCmd(arguments: notification.userInfo) {
            track("Failed to execute 'typeText' command.")
        }
    }
    
    @objc func vamTakeScreenshotCommand(_ notification: Notification) {
        
        if !takeScreenshotCmd(arguments: notification.userInfo) {
            track("Failed to execute 'takeScreenshot' command.")
        }
    }
    
    @objc func vamQuitCommand(_ notification: Notification) {
        
        quitScriptCmd(arguments: notification.userInfo)
    }
}
