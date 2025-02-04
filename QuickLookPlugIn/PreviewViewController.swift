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

    @IBOutlet weak var preview: NSImageView!
    @IBOutlet weak var heading: NSTextField!
    @IBOutlet weak var item1: NSTextField!
    @IBOutlet weak var item2: NSTextField!
    @IBOutlet weak var item3: NSTextField!
    @IBOutlet weak var item4: NSTextField!
    @IBOutlet weak var item5: NSTextField!

    // var dictionary: [String: Any] = [:]
    
    override var nibName: NSNib.Name? {
        
        return NSNib.Name("PreviewViewController")
    }

    override func loadView() {
        
        super.loadView()
        
        // Set a fixed preview size
        preferredContentSize = CGSize(width: 700, height: 334)
    }

    func preparePreviewOfFile(at url: URL) async throws {

        let imgUrl = url.appendingPathComponent("preview.png")
        let xmlUrl = url.appendingPathComponent("info.xml")

        do {
            // Load dictionary
            let data = try Data(contentsOf: xmlUrl)
            let dictionary = try PropertyListSerialization.propertyList(from: data, format: nil) as? [String: Any] ?? [:]
            
            let chip = dictionary["chip"] as? String
            let slow = dictionary["slow"] as? String
            let fast = dictionary["fast"] as? String
            
            var mem = ""
            if let chip = chip { mem += chip + "KB" }
            if let slow = slow { mem += ", " + slow + "KB" }
            if let fast = fast { mem += ", " + fast + "KB" }
            
            item1.stringValue = mem
            
        } catch {
            
        }

        heading.stringValue = url.deletingPathExtension().lastPathComponent
        preview.image = NSImage(contentsOf: imgUrl)


    }

}
