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
    var value2: Double
}

class DataSource: ObservableObject {
    
    @Published var update = false

    static let maxTimeSpan = 5.0
   
    // The data to visualize
    var data: [DataPoint] = []

    // Appearance
    var fgColor: Color = .red
    var bgColor: Color = .white
    var gridLines = [0.0, 0.25, 0.5, 0.75, 1.0]
    var lines: Int { return logScale ? 10 : 5 }
    
    // Scaling type
    var logScale: Bool = false {
        
        didSet {
            if (logScale) {
                gridLines = []
                for i in 0 ... lines {
                    var y = Double(i) / Double(lines)
                    if logScale { y = log(1.0 + 19.0 * y) / log(20) }
                    gridLines.append(y)
                }
            } else {
                gridLines = [0.0, 0.25, 0.5, 0.75, 1.0]
            }
        }
    }
                
    // Base color
    var color: NSColor = .red {
        
        didSet {
            bgColor = Color(nsColor: color.adjust(brightness: 1.0, saturation: 0.5))
            fgColor = Color(nsColor: color)
        }
    }
    
    var timeSpan: TimeInterval {
        
        guard let first = data.first else { return 0.0 }
        guard let last = data.last else { return 0.0 }
        
        return last.timestamp.timeIntervalSince(first.timestamp)
    }
    
    func add(_ value1: Double, _ value2: Double = 0) {
        
        add(value1, value2, timestamp: Date.now)
    }
    
    private func add(_ value1: Double, _ value2: Double, timestamp: Date) {

        // Add data point
        let scaled1 = logScale ? log(1.0 + (19 * value1)) / log(20) : value1
        let scaled2 = logScale ? log(1.0 + (19 * value1)) / log(20) : value2
        data.append(DataPoint(series: 0, timestamp: timestamp, value: scaled1, value2: scaled2))
        data.append(DataPoint(series: 1, timestamp: timestamp, value: scaled2, value2: scaled1))

        // Delete outdated data
        while timeSpan > DataSource.maxTimeSpan { data.removeFirst() }

        // Force the view to update
        update.toggle()
    }
}


//
// Time series (one value per timestamp)
//

class TimeSeries: NSView {
    
    var model = DataSource()
    var host: NSHostingView<ContentView>!
    
    struct ContentView: View {
        
        @ObservedObject var model: DataSource
        
        private var areaBackground: Gradient {
            return Gradient(colors: [model.fgColor, model.fgColor.opacity(0.1)])
        }
        private var background: Gradient {
            return Gradient(colors: [model.bgColor.opacity(0.5), .clear])
        }
        
        var body: some View {
                                
            let data = model.data
            
            Chart(data) { dataPoint in
                            
                AreaMark(
                    x: .value("Time", dataPoint.timestamp),
                    y: .value("Value", dataPoint.value)
                )
                .interpolationMethod(.catmullRom)
                .foregroundStyle(areaBackground)
                LineMark(
                    x: .value("Time", dataPoint.timestamp),
                    y: .value("Value", dataPoint.value)
                )
                .lineStyle(StrokeStyle(lineWidth: 2))
                .foregroundStyle(model.fgColor)
            }
            .chartXScale(domain: Date() - 5...Date())
            .chartXAxis(.hidden)
            .chartYScale(domain: 0.0...1.0)
            .chartYAxis {
                AxisMarks(values: model.gridLines) { _ in AxisGridLine() }
            }
            .padding(EdgeInsets(top: 10.0, leading: 10.0, bottom: 10.0, trailing: 10.0))
            .background(background)
            .chartLegend(.hidden)
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
// Double time series (two values per timestamp)
//

class DoubleTimeSeries: NSView {
    
    var model = DataSource()
    var host: NSHostingView<ContentView>!

    struct ContentView: View {
        
        @ObservedObject var model: DataSource
        
        private var areaBackground: Gradient {
            return Gradient(colors: [model.fgColor, model.fgColor.opacity(0.1)])
        }
        private var background: Gradient {
            return Gradient(colors: [model.bgColor.opacity(0.5), .clear])
        }
                
        private var gradients: KeyValuePairs<Int, Gradient> {
            return [ 0: Gradient(colors: [Color.green, Color.clear]),
                     1: Gradient(colors: [Color.red, Color.clear])]
        }
            
        var body: some View {
            
            let data = model.data
            
            Chart {
                ForEach(data, id: \.id) { dataPoint in
                    AreaMark(
                        x: .value("Time", dataPoint.timestamp),
                        y: .value("Value", dataPoint.value),
                        series: .value("Series", dataPoint.series)
                    )
                    .foregroundStyle(by: .value("Series", dataPoint.series))
                }
                
                ForEach(data.filter { $0.series == 0 }, id: \.id) { dataPoint in
                    LineMark(
                        x: .value("Time", dataPoint.timestamp),
                        y: .value("Value", dataPoint.value + dataPoint.value2),
                        series: .value("Series", dataPoint.series)
                    )
                    .foregroundStyle(.gray) // Line color (black)
                    .lineStyle(StrokeStyle(lineWidth: 2))
                }
            }
            .chartXScale(domain: Date() - 5...Date())
            .chartXAxis(.hidden)
            .chartYScale(domain: 0...0.75)
            .chartYAxis {
                AxisMarks(values: model.gridLines) { _ in AxisGridLine() }
            }
            .padding(EdgeInsets(top: 10.0, leading: 10.0, bottom: 10.0, trailing: 10.0))
            .background(background)
            .chartLegend(.hidden)
            .chartForegroundStyleScale(gradients)
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
