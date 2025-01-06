// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import SwiftUI
import Charts

struct DataPoint: Identifiable {
    
    var id = UUID()
    var series: Int
    var timestamp: Date
    var value: Double
}

class DataSource: ObservableObject {
    
    @Published var update = false
    
    static let maxTimeSpan: Double = 5
    
    // Visualized data
    var data: [DataPoint] = []
    
    // Title and sub title
    var heading = ""
    var subHeading = ""
    
    // Colors
    var themeColor: NSColor = .white
    var graph1Color: NSColor?
    var graph2Color: NSColor?

    // Grid lines
    var gridLines = [0.0, 0.25, 0.5, 0.75, 1.0]
    
    // Scaling
    var scale = 1.0
    var logScale: Bool = false {
        
        didSet {
            if (logScale) {
                gridLines = []
                for i in 0...10 {
                    var y = Double(i) / Double(10)
                    if logScale { y = log(1.0 + 19.0 * y) / log(20) }
                    gridLines.append(y)
                }
            } else {
                gridLines = [0.0, 0.25, 0.5, 0.75, 1.0]
            }
        }
    }
    
    var timeSpan: TimeInterval {
        
        guard let first = data.first else { return 0.0 }
        guard let last = data.last else { return 0.0 }
        
        return last.timestamp.timeIntervalSince(first.timestamp)
    }
    
    func add(_ value1: Double?, _ value2: Double? = nil) {
        
        add(value1, value2, timestamp: Date.now)
    }
    
    private func add(_ value1: Double?, _ value2: Double?, timestamp: Date) {
        
        // Scale incoming values
        var scaled1 = (value1 ?? 0) * scale
        if logScale { scaled1 = log(1.0 + (19 * scaled1)) / log(20) }
        var scaled2 = (value2 ?? 0) * scale
        if logScale { scaled2 = log(1.0 + (19 * scaled2)) / log(20) }
        
        // Add the first provided value to time series 2
        if value2 != nil {
            data.append(DataPoint(series: 2, timestamp: timestamp, value: scaled2))
        }

        // Add the first provided value to time series 1
        if value1 != nil {
            data.append(DataPoint(series: 1, timestamp: timestamp, value: scaled1))
        }
                
        // Sum up both values in time series 3 (used for drawing the line)
        data.append(DataPoint(series: 3, timestamp: timestamp, value: scaled1 + scaled2))
        
        // Delete outdated data
        while timeSpan > DataSource.maxTimeSpan { data.removeFirst() }
        
        // Update the view
        update.toggle()
    }
}

//
// Time series view
//

class TimeSeries: NSView {
    
    var model = DataSource()
    var host: NSHostingView<ContentView>!

    struct ContentView: View {
        
        @ObservedObject var model: DataSource
        
        private var heading: String {
            return model.heading
        }
        private var subHeading: String {
            return model.subHeading
        }
        private var themeColor: Color {
            return Color(nsColor: model.themeColor)
        }
        private var graph1Color: Color {
            return Color(nsColor: (model.graph1Color != nil) ? model.graph1Color! : model.themeColor)
        }
        private var graph2Color: Color {
            return Color(nsColor: (model.graph2Color != nil) ? model.graph2Color! : model.themeColor)
        }
        private var background: Gradient {
            return Gradient(colors: [Color.black, Color.black])
        }
        private var gradients: KeyValuePairs<Int, Gradient> {
            return [ 1: Gradient(colors: [graph1Color.opacity(0.75), graph1Color.opacity(0.25)]),
                     2: Gradient(colors: [graph2Color.opacity(0.75), graph2Color.opacity(0.25)])]
        }
        private var lineColor: Color {
            
            if #available(macOS 15.0, *) {
                return graph1Color.mix(with: .white, by: 0.25)
            } else {
                return graph1Color
            }
        }
        private var gridLineColor: Color {
            return Color.white.opacity(0.6)
        }
        private var lineWidth: Double {
            return 1.25 // 0.75
        }

        var body: some View {
            
            VStack(alignment: .leading) {
                
                VStack(alignment: .leading) {
                    Text(heading)
                        .font(.system(size: 14))
                        .fontWeight(.bold)
                        .foregroundColor(themeColor.opacity(1.0))
                        .padding(.bottom, 1)
                    Text(subHeading)
                        .font(.system(size: 8))
                        .fontWeight(.regular)
                        .foregroundColor(Color.gray)
                }
                .padding(EdgeInsets(top: 10, leading: 15, bottom: 5, trailing: 15))
                
                Chart {
                    ForEach(model.data.filter { $0.series != 3 }, id: \.id) { dataPoint in
                        AreaMark(
                            x: .value("Time", dataPoint.timestamp),
                            y: .value("Value", dataPoint.value),
                            series: .value("Series", dataPoint.series)
                        )
                        .foregroundStyle(by: .value("Series", dataPoint.series))
                    }
                    
                    ForEach(model.data.filter { $0.series == 3 }, id: \.id) { dataPoint in
                        LineMark(
                            x: .value("Time", dataPoint.timestamp),
                            y: .value("Value", dataPoint.value),
                            series: .value("Series", dataPoint.series)
                        )
                        .interpolationMethod(.catmullRom)
                        .foregroundStyle(lineColor)
                        .lineStyle(StrokeStyle(lineWidth: lineWidth))
                        /*
                        .symbol {
                            if #available(macOS 15.0, *) {
                                Circle()
                                    .fill(graph1Color.mix(with: .white, by: 0.5))
                                    .frame(width: 2)
                            } else {
                                Circle()
                                    .fill(graph1Color)
                                    .frame(width: 2)
                            }
                        }
                        */
                    }
                }
                .chartXScale(domain: Date() - DataSource.maxTimeSpan...Date())
                .chartXAxis(.hidden)
                .chartYScale(domain: 0...1.0)
                .chartYAxis {
                    AxisMarks(values: model.gridLines) {
                        AxisGridLine()
                            .foregroundStyle(gridLineColor)
                    }
                }
                .padding(EdgeInsets(top: 0.0, leading: 15.0, bottom: 15.0, trailing: 15.0))
                .chartLegend(.hidden)
                .chartForegroundStyleScale(gradients)
            }
            .background(background)
            .cornerRadius(10)
        }
    }
    
    required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        host = NSHostingView(rootView: ContentView( model: model))
        self.addSubview(host)
        
        host.translatesAutoresizingMaskIntoConstraints = false
        host.leadingAnchor.constraint(equalTo: self.leadingAnchor).isActive = true
        host.trailingAnchor.constraint(equalTo: self.trailingAnchor).isActive = true
        host.topAnchor.constraint(equalTo: self.topAnchor).isActive = true
        host.bottomAnchor.constraint(equalTo: self.bottomAnchor).isActive = true
    }
}

//
// Custom panels
//

class ChipRamPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Chip Ram"
        model.subHeading = "Memory Accesses"
    }
}

class SlowRamPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Slow Ram"
        model.subHeading = "Memory Accesses"
    }
}

class FastRamPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Fast Ram"
        model.subHeading = "Memory Accesses"
    }
}

class RomPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Kickstart Rom"
        model.subHeading = "Memory Accesses"
    }
}

class CopperDmaPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Copper"
        model.subHeading = "DMA Accesses"
        model.logScale = true
    }
}

class BlitterDmaPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Blitter"
        model.subHeading = "DMA Accesses"
        model.logScale = true
    }
}

class DiskDmaPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Disk"
        model.subHeading = "DMA Accesses"
    }
}

class AudioDmaPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Audio"
        model.subHeading = "DMA Accesses"
    }
}

class SpriteDmaPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Sprite"
        model.subHeading = "DMA Accesses"
    }
}

class BitplaneDmaPanel: TimeSeries {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        model.heading = "Bitplane"
        model.subHeading = "DMA Accesses"
    }
}
