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

    static let unmapped = NSColor.gray

    static let chipRam = NSColor.init(r: 0x80, g: 0xFF, b: 0x00, a: 0xFF)
    static let slowRam = NSColor.init(r: 0x66, g: 0xCC, b: 0x00, a: 0xFF)
    static let fastRam = NSColor.init(r: 0x4C, g: 0x99, b: 0x00, a: 0xFF)

    static let rom = NSColor.init(r: 0xFF, g: 0x00, b: 0x00, a: 0xFF)
    static let wom = NSColor.init(r: 0xCC, g: 0x00, b: 0x00, a: 0xFF)
    static let ext = NSColor.init(r: 0x99, g: 0x00, b: 0x00, a: 0xFF)

    static let cia = NSColor.init(r: 0x66, g: 0xB2, b: 0xFF, a: 0xFF)
    static let rtc = NSColor.init(r: 0xB2, g: 0x66, b: 0xFF, a: 0xFF)
    static let ocs = NSColor.init(r: 0xFF, g: 0xFF, b: 0x66, a: 0xFF)
    static let autoConf = NSColor.init(r: 0xFF, g: 0x66, b: 0xB2, a: 0xFF)
}

extension Inspector {
    
    func refreshMemory(everything: Bool) {
        
        // track("Refreshing memory inspector tab")
        
        if everything {
            refreshMemoryLayout()
            memBankTableView.refresh()
        }
        
        memTableView.refresh()
    }
    
    func refreshMemoryLayout() {

        guard let config = amigaProxy?.config() else { return }

        let size = NSSize(width: 16, height: 16)
        memLayoutButton.image   = memLayoutImage
        memChipRamButton.image  = NSImage.init(color: MemColors.chipRam, size: size)
        memFastRamButton.image  = NSImage.init(color: MemColors.fastRam, size: size)
        memSlowRamButton.image  = NSImage.init(color: MemColors.slowRam, size: size)
        memRomButton.image      = NSImage.init(color: MemColors.rom, size: size)
        memWomButton.image      = NSImage.init(color: MemColors.wom, size: size)
        memExtButton.image      = NSImage.init(color: MemColors.ext, size: size)
        memCIAButton.image      = NSImage.init(color: MemColors.cia, size: size)
        memRTCButton.image      = NSImage.init(color: MemColors.rtc, size: size)
        memOCSButton.image      = NSImage.init(color: MemColors.ocs, size: size)
        memAutoConfButton.image = NSImage.init(color: MemColors.autoConf, size: size)

        let chipRamKB = config.mem.chipRamSize / 1024
        let fastRamKB = config.mem.fastRamSize / 1024
        let slowRamKB = config.mem.slowRamSize / 1024
        let romKB = config.mem.romSize / 1024
        let womKB = config.mem.womSize / 1024
        let extKB = config.mem.extSize / 1024
        memChipRamText.stringValue = String.init(format: "%d KB", chipRamKB)
        memFastRamText.stringValue = String.init(format: "%d KB", fastRamKB)
        memSlowRamText.stringValue = String.init(format: "%d KB", slowRamKB)
        memRomText.stringValue = String.init(format: "%d KB", romKB)
        memWomText.stringValue = String.init(format: "%d KB", womKB)
        memExtText.stringValue = String.init(format: "%d KB", extKB)
    }
    
    var memLayoutImage: NSImage? {

        track("Computing layout image")

        guard let memory = amigaProxy?.mem else { return nil }

        // Create image representation in memory
        let size = CGSize.init(width: 256, height: 16)
        let cap = Int(size.width) * Int(size.height)
        let mask = calloc(cap, MemoryLayout<UInt32>.size)!
        let ptr = mask.bindMemory(to: UInt32.self, capacity: cap)

        // Create image data
        for x in 0...255 {

            let src = memory.memSrc(x << 16).rawValue
            var color: NSColor

            switch src {
            case MEM_UNMAPPED.rawValue: color = MemColors.unmapped
            case MEM_CHIP.rawValue:     color = MemColors.chipRam
            case MEM_FAST.rawValue:     color = MemColors.fastRam
            case MEM_SLOW.rawValue:     color = MemColors.slowRam
            case MEM_ROM.rawValue:      color = MemColors.rom
            case MEM_WOM.rawValue:      color = MemColors.wom
            case MEM_EXT.rawValue:      color = MemColors.ext
            case MEM_CIA.rawValue:      color = MemColors.cia
            case MEM_RTC.rawValue:      color = MemColors.rtc
            case MEM_OCS.rawValue:      color = MemColors.ocs
            case MEM_AUTOCONF.rawValue: color = MemColors.autoConf
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

    func setBank(src: MemorySource) {

        for bank in 0...255 {

            if amigaProxy?.mem.memSrc(bank << 16) == src {
                setBank(bank)
                return
            }
        }
    }

    func setBank(_ value: Int) {
        
        if value >= 0 && value <= 0xFF {
            
            bank = value
            memSrc = amigaProxy?.mem.memSrc(bank << 16) ?? MEM_UNMAPPED
            track("Switching to bank \(value)")
            memLayoutSlider.integerValue = bank
            memTableView.scrollRowToVisible(0)
            memBankTableView.scrollRowToVisible(value)
            memBankTableView.selectRowIndexes([value], byExtendingSelection: false)
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

    @IBAction func memSliderAction(_ sender: NSSlider!) {
        
        let value = sender.integerValue
        setBank(value)
    }

    @IBAction func memChipAction(_ sender: NSButton!) {
        setBank(src: MEM_CHIP)
    }

    @IBAction func memFastRamAction(_ sender: NSButton!) {
        setBank(src: MEM_FAST)
    }
    
    @IBAction func memSlowRamAction(_ sender: NSButton!) {
        setBank(src: MEM_SLOW)
    }

    @IBAction func memRomAction(_ sender: NSButton!) {
        setBank(src: MEM_ROM)
    }

    @IBAction func memWomAction(_ sender: NSButton!) {
        setBank(src: MEM_WOM)
    }

    @IBAction func memExtAction(_ sender: NSButton!) {
        setBank(src: MEM_EXT)
    }

    @IBAction func memCIAAction(_ sender: NSButton!) {
        setBank(src: MEM_CIA)
    }
 
    @IBAction func memRTCAction(_ sender: NSButton!) {
        setBank(src: MEM_RTC)
    }

    @IBAction func memOCSAction(_ sender: NSButton!) {
        setBank(src: MEM_OCS)
    }

    @IBAction func memAutoConfAction(_ sender: NSButton!) {
        setBank(src: MEM_AUTOCONF)
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
