//
//  PreviewViewController.swift
//  QuickLookPlugIn
//
//  Created by Dirk Hoffmann on 04.02.25.
//  Copyright © 2025 Dirk Hoffmann. All rights reserved.
//

import Cocoa
import Quartz

class PreviewViewController: NSViewController, QLPreviewingController {

    @IBOutlet weak var textField: NSTextField!
    @IBOutlet weak var imageView: NSImageView!

    override var nibName: NSNib.Name? {
        return NSNib.Name("PreviewViewController")
    }

    override func loadView() {
        super.loadView()
        // Do any additional setup after loading the view.
        
        NSLog("dirkwhoffmann: loadView()")
    }

    /*
    func preparePreviewOfSearchableItem(identifier: String, queryString: String?) async throws {
        // Implement this method and set QLSupportsSearchableItems to YES in the Info.plist of the extension if you support CoreSpotlight.

        // Perform any setup necessary in order to prepare the view.
        // Quick Look will display a loading spinner until this returns.
    }
    */

    func preparePreviewOfFile(at url: URL) async throws {
        // Add the supported content types to the QLSupportedContentTypes array in the Info.plist of the extension.

        // Perform any setup necessary in order to prepare the view.

        // Quick Look will display a loading spinner until this returns.
        let newUrl = url.appendingPathComponent("preview.png")

        textField.stringValue = newUrl.relativePath
        imageView.image = NSImage(contentsOf: url.appendingPathComponent("preview.png"))
        NSLog("dirkwhoffmann: preparePreviewOfFile(/url))")
    }

}
