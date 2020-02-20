// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Double {
   
    func truncate(digits: Int) -> Double {
        let factor = Double(truncating: pow(10, digits) as NSNumber)
        return (self * factor).rounded() / factor
    }
}

class Speedometer {
    
    /// Current emulation speed in MHz
    private var _mhz = 0.0
    var mhz: Double { return _mhz.truncate(digits: 2); }
    
    /// Current GPU performance in frames per second
    private var _fps = 0.0
    var fps: Double { return _fps.truncate(digits: 0); }
    
    /// Smoothing factor
    private let alpha = 0.5

    /// Time of the previous update
    private var latchedTimestamp: Double
    
    /// Value of the master clock in the previous update
    private var latchedCycle: Int64 = Int64.max
    
    /// Frame count in the previous update
    private var latchedFrame: Int64 = Int64.max
    
    init() {

        latchedTimestamp = Date().timeIntervalSince1970
    }

    /* Updates speed, frame and jitter information.
     * This function needs to be invoked periodically to get meaningful
     * results.
     *   - cycles  Elapsed CPU cycles since power up
     *   - frames  Drawn frames since power up
     */
    func updateWith(cycle: Int64, frame: Int64) {
        
        let timestamp = Date().timeIntervalSince1970
        
        if cycle >= latchedCycle && frame >= latchedFrame {

            // Measure elapsed time in microseconds
            let elapsedTime = timestamp - latchedTimestamp
        
            // Measure clock frequency in MHz
            let elapsedCycles = Double(cycle - latchedCycle) / 1_000_000
            _mhz = alpha * (elapsedCycles / elapsedTime) + (1 - alpha) * _mhz

            // Measure frames per second
            let elapsedFrames = Double(frame - latchedFrame)
            _fps = alpha * (elapsedFrames / elapsedTime) + (1 - alpha) * _fps

            // Assign zero if values are completely out of range
            // mhz = (mhz >= 0.0 && mhz <= 100.0) ? mhz : 0.0
            // fps = (fps >= 0.0 && fps <= 200.0) ? fps : 0.
        }
        
        // Keep values
        latchedTimestamp = timestamp
        latchedCycle = cycle
        latchedFrame = frame
        
        // return (_mhz.truncate(digits: 2), _fps.truncate(digits: 0))
    }
}
