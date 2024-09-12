// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// Time and date
//

extension DispatchTime {

    static func diffNano(_ t: DispatchTime) -> UInt64 {
        return DispatchTime.now().uptimeNanoseconds - t.uptimeNanoseconds
    }

    static func diffMicroSec(_ t: DispatchTime) -> UInt64 { return diffNano(t) / 1_000 }
    static func diffMilliSec(_ t: DispatchTime) -> UInt64 { return diffNano(t) / 1_000_000 }
    static func diffSec(_ t: DispatchTime) -> UInt64 { return diffNano(t) / 1_000_000_000 }
}

extension Date {

    static func elapsed(time: time_t) -> String {

        return Date(timeIntervalSince1970: TimeInterval(time)).elapsed()
    }

    static func elapsed(interval: TimeInterval) -> String {

        let secPerMin = 60
        let secPerHour = secPerMin * 60
        let secPerDay = secPerHour * 24
        let secPerWeek = secPerDay * 7
        let secPerMonth = secPerWeek * 4
        let secPerYear = secPerWeek * 52

        let seconds = Int(interval)

        if seconds == 0 {
            return "Now"
        }
        if seconds < secPerMin {
            return "\(seconds) second" + (seconds == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerHour {
            let m = seconds / secPerMin
            return "\(m) minute" + (m == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerDay {
            let h = seconds / secPerHour
            return "\(h) hour" + (h == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerWeek {
            let d = seconds / secPerDay
            return "\(d) day" + (d == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerMonth {
            let w = seconds / secPerWeek
            return "\(w) week" + (w == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerYear {
            let m = seconds / secPerMonth
            return "\(m) month" + (m == 1 ? "" : "s") + " ago"
        } else {
            let y = seconds / secPerYear
            return "\(y) year" + (y == 1 ? "" : "s") + " ago"
        }
    }

    func elapsed() -> String {

        return Date.elapsed(interval: -self.timeIntervalSince(Date()))
    }
}
