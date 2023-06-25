// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
    
    struct MemColors {
        
        static let unmapped = NSColor.gray
        
        static let chip = NSColor(r: 0x80, g: 0xFF, b: 0x00, a: 0xFF)
        static let slow = NSColor(r: 0x66, g: 0xCC, b: 0x00, a: 0xFF)
        static let fast = NSColor(r: 0x4C, g: 0x99, b: 0x00, a: 0xFF)
        
        static let rom = NSColor(r: 0xFF, g: 0x00, b: 0x00, a: 0xFF)
        static let wom = NSColor(r: 0xCC, g: 0x00, b: 0x00, a: 0xFF)
        static let ext = NSColor(r: 0x99, g: 0x00, b: 0x00, a: 0xFF)
        
        static let cia = NSColor(r: 0x66, g: 0xB2, b: 0xFF, a: 0xFF)
        static let rtc = NSColor(r: 0xB2, g: 0x66, b: 0xFF, a: 0xFF)
        static let cust = NSColor(r: 0xFF, g: 0xFF, b: 0x66, a: 0xFF)
        static let auto = NSColor(r: 0xFF, g: 0x66, b: 0xB2, a: 0xFF)
    }

    var accessor: Accessor {
        return memBankMap.selectedTag() == 0 ? .CPU : .AGNUS
    }
    
    func memSrc(bank: Int) -> MemorySource {        
        return parent.amiga.mem.memSrc(accessor, addr: bank << 16)
    }
    
    var memLayoutImage: NSImage? {

        // Create image representation in memory
        let width = 512
        let height = 16
        let size = CGSize(width: width, height: height)
        let cap = Int(size.width) * Int(size.height)
        let mask = calloc(cap, MemoryLayout<UInt32>.size)!
        let ptr = mask.bindMemory(to: UInt32.self, capacity: cap)

        // Create image data
        for bank in 0...255 {

            var color: NSColor
            var mirror = false
            
            switch memSrc(bank: bank) {
            case .NONE:          color = MemColors.unmapped
            case .CHIP:          color = MemColors.chip
            case .CHIP_MIRROR:   color = MemColors.chip;     mirror = true
            case .SLOW:          color = MemColors.slow
            case .SLOW_MIRROR:   color = MemColors.slow;     mirror = true
            case .FAST:          color = MemColors.fast
            case .CIA:           color = MemColors.cia
            case .CIA_MIRROR:    color = MemColors.cia;      mirror = true
            case .RTC:           color = MemColors.rtc
            case .CUSTOM:        color = MemColors.cust
            case .CUSTOM_MIRROR: color = MemColors.cust;     mirror = true
            case .AUTOCONF:      color = MemColors.auto
            case .ZOR:           color = MemColors.auto
            case .ROM:           color = MemColors.rom
            case .ROM_MIRROR:    color = MemColors.rom;      mirror = true
            case .WOM:           color = MemColors.wom
            case .EXT:           color = MemColors.ext
            default:             fatalError()
            }
            let ciBgColor = CIColor(color: MemColors.unmapped)!
            let ciColor = CIColor(color: color)!
            
            for x in 0...1 {
                for y in 0...height-1 {
                    
                    let dx = 2*bank+x
                    let c = 2
                    var r, g, b, a: Int
                    
                    if mirror && (((dx - y) % 8) < 3) {
                        r = Int(ciBgColor.red * CGFloat(255 - y*c))
                        g = Int(ciBgColor.green * CGFloat(255 - y*c))
                        b = Int(ciBgColor.blue * CGFloat(255 - y*c))
                        a = Int(ciBgColor.alpha * CGFloat(255))
                    } else {
                        r = Int(ciColor.red * CGFloat(255 - y*c))
                        g = Int(ciColor.green * CGFloat(255 - y*c))
                        b = Int(ciColor.blue * CGFloat(255 - y*c))
                        a = Int(ciColor.alpha * CGFloat(255))
                    }
                    let abgr = UInt32(r | g << 8 | b << 16 | a << 24)
                    ptr[y*width + dx] = abgr
                }
            }
        }

        // Create image
        let image = NSImage.make(data: mask, rect: size)
        let resizedImage = image?.resizeSharp(width: CGFloat(width), height: CGFloat(height))
        return resizedImage
        // return image
    }

    private func refreshMemoryLayout() {

        let config = amiga.mem.config
        let size = NSSize(width: 16, height: 16)

        memLayoutButton.image   = memLayoutImage
        memChipRamButton.image  = NSImage(color: MemColors.chip, size: size)
        memFastRamButton.image  = NSImage(color: MemColors.fast, size: size)
        memSlowRamButton.image  = NSImage(color: MemColors.slow, size: size)
        memRomButton.image      = NSImage(color: MemColors.rom, size: size)
        memWomButton.image      = NSImage(color: MemColors.wom, size: size)
        memExtButton.image      = NSImage(color: MemColors.ext, size: size)
        memCIAButton.image      = NSImage(color: MemColors.cia, size: size)
        memRTCButton.image      = NSImage(color: MemColors.rtc, size: size)
        memOCSButton.image      = NSImage(color: MemColors.cust, size: size)
        memAutoConfButton.image = NSImage(color: MemColors.auto, size: size)

        let chipKB = config.chipSize / 1024
        let fastKB = config.fastSize / 1024
        let slowKB = config.slowSize / 1024
        let romKB = config.romSize / 1024
        let womKB = config.womSize / 1024
        let extKB = config.extSize / 1024
        memChipRamText.stringValue = String(format: "%d KB", chipKB)
        memFastRamText.stringValue = String(format: "%d KB", fastKB)
        memSlowRamText.stringValue = String(format: "%d KB", slowKB)
        memRomText.stringValue = String(format: "%d KB", romKB)
        memWomText.stringValue = String(format: "%d KB", womKB)
        memExtText.stringValue = String(format: "%d KB", extKB)
    }

    func refreshMemory(count: Int = 0, full: Bool = false) {

        if full { refreshMemoryLayout() }

        memTableView.refresh(count: count, full: full)
        memBankTableView.refresh(count: count, full: full)
    }

    func jumpTo(addr: Int) {
        
        if addr >= 0 && addr <= 0xFFFFFF {
            
            searchAddress = addr
            jumpTo(bank: addr >> 16)
            let row = (addr & 0xFFFF) / 16
            memTableView.scrollRowToVisible(row)
            memTableView.selectRowIndexes([row], byExtendingSelection: false)
        }
    }
    
    func jumpTo(source: MemorySource) {

        for bank in 0...255 {

            if memSrc(bank: bank) == source {
                jumpTo(bank: bank)
                return
            }
        }
    }

    func jumpTo(bank nr: Int) {
        
        if nr >= 0 && nr <= 0xFF {
            
            displayedBank = nr
            displayedBankType = memSrc(bank: nr) //  ?? .MEM_NONE
            memLayoutSlider.integerValue = displayedBank
            memTableView.scrollRowToVisible(0)
            memBankTableView.scrollRowToVisible(nr)
            memBankTableView.selectRowIndexes([nr], byExtendingSelection: false)
            fullRefresh()
        }
    }

    @IBAction func memSliderAction(_ sender: NSSlider!) {

        jumpTo(bank: sender.integerValue)
    }

    @IBAction func memChipAction(_ sender: NSButton!) {

        jumpTo(source: .CHIP)
    }

    @IBAction func memFastRamAction(_ sender: NSButton!) {

        jumpTo(source: .FAST)
    }
    
    @IBAction func memSlowRamAction(_ sender: NSButton!) {

        jumpTo(source: .SLOW)
    }

    @IBAction func memRomAction(_ sender: NSButton!) {

        jumpTo(source: .ROM)
    }

    @IBAction func memWomAction(_ sender: NSButton!) {

        jumpTo(source: .WOM)
    }

    @IBAction func memExtAction(_ sender: NSButton!) {

        jumpTo(source: .EXT)
    }

    @IBAction func memCIAAction(_ sender: NSButton!) {

        jumpTo(source: .CIA)
    }
 
    @IBAction func memRTCAction(_ sender: NSButton!) {

        jumpTo(source: .RTC)
    }

    @IBAction func memOCSAction(_ sender: NSButton!) {

        jumpTo(source: .CUSTOM)
    }

    @IBAction func memAutoConfAction(_ sender: NSButton!) {

        jumpTo(source: .AUTOCONF)
    }

    @IBAction func memBankMapAction(_ sender: NSPopUpButton!) {

        fullRefresh()
    }

    @IBAction func memSearchAction(_ sender: NSTextField!) {

        let input = sender.stringValue
        if let addr = Int(input, radix: 16), input != "" {
            sender.stringValue = String(format: "%06X", addr)
            jumpTo(addr: addr)
        } else {
            sender.stringValue = ""
            searchAddress = -1
        }
        fullRefresh()
    }
}
