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
    @IBOutlet weak var box: NSBox!
    @IBOutlet weak var item1: NSTextField!
    @IBOutlet weak var item2: NSTextField!
    @IBOutlet weak var item3: NSTextField!
    @IBOutlet weak var item4: NSTextField!
    @IBOutlet weak var item5: NSTextField!
    @IBOutlet weak var build: NSTextField!
    
    override var nibName: NSNib.Name? {
        
        return NSNib.Name("PreviewViewController")
    }

    override func loadView() {
        
        super.loadView()
        
        // Set a fixed preview size
        preferredContentSize = CGSize(width: 800, height: 400)
        
        // Display the image without any translucency
        let backgroundView = NSView(frame: preview.frame)
        backgroundView.wantsLayer = true
        backgroundView.layer?.backgroundColor = NSColor.white.cgColor
        self.view.addSubview(backgroundView, positioned: .below, relativeTo: preview)
    }

    func preparePreviewOfFile(at url: URL) async throws {

        let imgUrl = url.appendingPathComponent("preview.png")
        let xmlUrl = url.appendingPathComponent("machine.plist")

        // Load the preview image
        preview.image = NSImage(contentsOf: imgUrl)

        // Load the machine description
        loadDescription(at: xmlUrl)
    }

    func loadDescription(at url: URL) {
         
        let items = [item1, item2, item3, item4, item5 ]
        var line = 0
        
        func add(_ text: String) {
                        
            if let item = items[line] { item.stringValue = text }
            line += 1
        }
        
        for item in items { item?.stringValue = "" }

        do {
            // Load dictionary
            let data = try Data(contentsOf: url)
            let dict = try PropertyListSerialization.propertyList(from: data, format: nil) as? [String: Any] ?? [:]

            if let model = dict["Model"] as? String { heading.stringValue = model }

            if let kick = dict["Kickstart"] as? String { add(kick) }
            if let agnus = dict["Agnus"] as? String, let denise = dict["Denise"] as? String {
                add(agnus == denise ? "\(agnus) Chipset" : "\(agnus) Agnus, \(denise) Denise")
            }
            if let chip = dict["Chip"] as? Int { add(String(chip) + " KB Chip RAM") }
            if let slow = dict["Slow"] as? Int { add(String(slow) + " KB Slow RAM") }
            if let fast = dict["Fast"] as? Int  { add(String(fast) + " KB Fast RAM") }
            if let version = dict["Version"] as? String { build.stringValue = "Build with vAmiga " + version }
        } catch {
            
        }
    }
}
