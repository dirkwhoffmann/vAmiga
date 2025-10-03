// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class TraceTableView: NSTableView {

    @IBOutlet weak var inspector: Inspector!
    var amiga: EmulatorProxy? { return inspector.parent.emu }
    var cpu: CPUProxy? { return amiga?.cpu }

    // Data caches
    var numRows = 0
    var addrInRow: [Int: String] = [:]
    var flagsInRow: [Int: String] = [:]
    var instrInRow: [Int: String] = [:]

    override init(frame frameRect: NSRect) { super.init(frame: frameRect); commonInit() }
    required init?(coder: NSCoder) { super.init(coder: coder); commonInit() }
    
    func commonInit() {

        dataSource = self
        target = self
    }
        
    private func cache() {

        guard let cpu = cpu else { return }

        numRows = cpu.loggedInstructions

        for i in 0 ..< numRows {
            
            var len = 0
            addrInRow[i] = cpu.disassembleRecordedPC(i)
            instrInRow[i] = cpu.disassembleRecordedInstr(i, length: &len)
            flagsInRow[i] = cpu.disassembleRecordedFlags(i)
        }
    }

    func refresh(count: Int = 0, full: Bool = false) {

        cache()
        reloadData()
    }
}

extension TraceTableView: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {

        return numRows
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        switch tableColumn?.identifier.rawValue {
            
        case "addr":
            return addrInRow[row]
        case "flags":
            return flagsInRow[row]
        case "instr":
            return instrInRow[row]
        default:
            return "???"
        }
    }
}
