// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// Commands
//

func resetScriptCmd(arguments: [AnyHashable: Any]?) -> Bool {
    
    amigaProxy?.reset()
    return true
}

func configureScriptCmd(arguments: [AnyHashable: Any]?) -> Bool {
    
    // Hardware model
    if let argument = arguments?["VAMModel"] as? String {
        
        switch argument {
            
        case "A1000": amigaProxy?.configureModel(AMIGA_1000.rawValue)
        case "A500": amigaProxy?.configureModel(AMIGA_500.rawValue)
        case "A2000": amigaProxy?.configureModel(AMIGA_2000.rawValue)
            
        default: return false
        }
        return true
    }
    return false
}

func mountScriptCmd(arguments: [AnyHashable: Any]?) -> Bool {
    
    if let argument = arguments?["VAMPath"] as? String {
        let url = URL(fileURLWithPath: argument)
        do {
            try myDocument?.createAmigaAttachment(from: url)
            myDocument?.mountAmigaAttachment()
        } catch {
            track("Remote control: Unable to mount \(url).")
        }
    }
    return false
}

func typeTextCmd(arguments: [AnyHashable: Any]?) -> Bool {
    
    if let text = arguments?[""] as? String {
        myController?.keyboardcontroller.autoType(text)
        return true
    }
    return false
}

func takeScreenshotCmd(arguments: [AnyHashable: Any]?) -> Bool {
    
    guard let path = arguments?["VAMPath"] as? String else {
        return false
    }

    do {
        try myController?.saveScreenshot(url: URL(fileURLWithPath: path))
        return true
    } catch {
        track("Remote control: Failed to save screenshot")
        return false
    }
}

func quitScriptCmd(arguments: [AnyHashable: Any]?) {
    
    myDocument?.updateChangeCount(.changeCleared)
    NSApplication.shared.terminate(nil)
}
