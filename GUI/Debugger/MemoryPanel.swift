// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


extension NSColor {
    
    convenience init(r: Int, g: Int, b: Int, a: Int) {
        
        self.init(red: CGFloat(r) / 0xFF,
                  green: CGFloat(g) / 0xFF,
                  blue: CGFloat(b) / 0xFF,
                  alpha: CGFloat(a) / 0xFF)
    }
}

struct MemColors {

    static let unmapped = NSColor.gray // init(r: 0xFF, g: 0xFF, b: 0xFF, a: 0x00)
    static let chipRam  = NSColor.init(r: 0x80, g: 0xFF, b: 0x00, a: 0xFF)
    static let fastRam  = NSColor.init(r: 0x00, g: 0xCC, b: 0x00, a: 0xFF)
    static let slowRam  = NSColor.init(r: 0x00, g: 0x99, b: 0x4C, a: 0xFF)
    static let io       = NSColor.init(r: 0xFF, g: 0xFF, b: 0x00, a: 0xFF)
    static let rom      = NSColor.init(r: 0xFF, g: 0x00, b: 0x00, a: 0xFF)
}

extension Inspector {
    
    func refreshMemory() {
        
        memTableView.refresh()
    }
    
    func refreshMemoryLayout() {
        
        guard let config = amigaProxy?.config() else { return }
        let chipRamKB = config.chipRamSize
        let fastRamKB = config.fastRamSize
        let slowRamKB = config.slowRamSize
        let size = NSSize(width: 16, height: 16)
        
        memLayoutButton.image = memLayoutImage
        memChipRamButton.image = NSImage.init(color: MemColors.chipRam, size: size)
        memFastRamButton.image = NSImage.init(color: MemColors.fastRam, size: size)
        memSlowRamButton.image = NSImage.init(color: MemColors.slowRam, size: size)
        memIOButton.image = NSImage.init(color: MemColors.io, size: size)
        memRomButton.image = NSImage.init(color: MemColors.rom, size: size)

        memChipRamText.stringValue = String.init(format: "%d KB", chipRamKB)
        memFastRamText.stringValue = String.init(format: "%d KB", fastRamKB)
        memSlowRamText.stringValue = String.init(format: "%d KB", slowRamKB)

        
    }
    
    
    var memLayoutImage : NSImage? {
        get {
            
            guard let memory = amigaProxy?.mem else { return nil }
            
            // Create image representation in memory
            let size = CGSize.init(width: 256, height: 16)
            let cap = Int(size.width) * Int(size.height)
            let mask = calloc(cap, MemoryLayout<UInt32>.size)!
            let ptr = mask.bindMemory(to: UInt32.self, capacity: cap)
            
            // Create image data
            for x in 0...255 {
                
                let src = memory.memSrc(x << 16).rawValue
                var color : NSColor
                
                switch (src) {
                case MEM_UNMAPPED.rawValue: color = MemColors.unmapped
                case MEM_CHIP.rawValue:     color = MemColors.chipRam
                case MEM_FAST.rawValue:     color = MemColors.fastRam
                case MEM_CIA.rawValue:      color = MemColors.io
                case MEM_SLOW.rawValue:     color = MemColors.slowRam
                case MEM_RTC.rawValue:      color = MemColors.io
                case MEM_CUSTOM.rawValue:   color = MemColors.io
                case MEM_BOOT.rawValue:     color = MemColors.rom
                case MEM_KICK.rawValue:     color = MemColors.rom
                default:                    fatalError()
                }
                let ciColor: CIColor = CIColor(color: color)!
                
                for y in 0...15 {
                    let c = 2
                    let r = Int(ciColor.red * CGFloat(255 - y*c))
                    let g = Int(ciColor.green * CGFloat(255 - y*c))
                    let b = Int(ciColor.blue * CGFloat(255 - y*c))
                    let a = Int(ciColor.alpha)
                    
                    ptr[x + 256*y] = UInt32(r | g << 8 | b << 16 | a << 24)
                }
            }
            
            // Create image
            let image = NSImage.make(data: mask, rect: size)
            let resizedImage = image?.resizeSharp(width: 512, height: 16)
            // resizedImage?.makeGlossy()
            // return resizedImage?.roundCorners(withRadius: 4)
            return resizedImage
        }
    }
    
    func setBank(_ value: Int) {
        
        if value >= 0 && value <= 0xFF {
            
            bank = value
            memSrc = amigaProxy?.mem.memSrc(bank << 16) ?? MEM_UNMAPPED
            track("Switching to bank \(value)")
            // memBankField.integerValue = value
            // memBankStepper.integerValue = value
            memTableView.scrollRowToVisible(0)
            bankTableView.scrollRowToVisible(value)
            bankTableView.selectRowIndexes([value], byExtendingSelection: false)
            memTableView.refresh()
        }
    }
    
    func setSelected(_ value: Int) {
        
        if value >= 0 && value <= 0xFFFFFF {
            
            selected = value
            track("Moving to memory location \(value)")
            setBank(value >> 16)
            let row = (selected / 16) % 4096
            memTableView.scrollRowToVisible(row)
            memTableView.selectRowIndexes([row], byExtendingSelection: false)
        }
    }
    
    /*
    @IBAction func memBankAction(_ sender: NSTextField!) {
     
        track("\(sender.integerValue)")
        setBank(sender.integerValue)
    }
    
    @IBAction func memStepperAction(_ sender: NSStepper!) {
        
        track("\(sender.integerValue)")
        setBank(sender.integerValue)
    }
    
    @IBAction func memAreaAction(_ sender: NSPopUpButton!) {
        
        switch (UInt32(sender.selectedTag())) {
            
        case MEM_CHIP.rawValue:   setBank(0x00)
        case MEM_FAST.rawValue:   setBank(0x20)
        case MEM_CIA.rawValue:    setBank(0xA0)
        case MEM_SLOW.rawValue:   setBank(0xC0)
        case MEM_RTC.rawValue:    setBank(0xDC)
        case MEM_CUSTOM.rawValue: setBank(0xDF)
        case MEM_KICK.rawValue:   setBank(0xFC)
            
        default: fatalError()
        }
    }
    */
    @IBAction func memSliderAction(_ sender: NSSlider!) {
        
        let value = sender.integerValue
        track("\(value)")
    }
    
    @IBAction func memSearchAction(_ sender: NSTextField!) {
        
        let input = sender.stringValue
        track("Going to address string \(input)")
        
        guard let addr = Int(input, radix: 16), input != "" else {
            
            sender.stringValue = ""
            selected = -1
            memTableView.refresh()
            return
        }
        
        track("Going to address \(addr)")
        sender.stringValue = String(format: "%06X", addr)
        setSelected(addr)
    }
}
