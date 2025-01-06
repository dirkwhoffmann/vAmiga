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
    
    func add(_ value1: Double?, _ value2: Double? = nil) {
        
        add(value1, value2, timestamp: Date.now)
    }
    
    private func add(_ value1: Double?, _ value2: Double?, timestamp: Date) {
        
        var scaled1 = value1
        var scaled2 = value2
        if scaled1 != nil && logScale { scaled1 = log(1.0 + (19 * value1!)) / log(20) }
        if scaled2 != nil && logScale { scaled2 = log(1.0 + (19 * value2!)) / log(20) }
        
        if let scaled1 = scaled1 {
            data.append(DataPoint(series: 1, timestamp: timestamp, value: scaled1))
        }
        if let scaled2 = scaled2 {
            data.append(DataPoint(series: 2, timestamp: timestamp, value: scaled2))
        }
        if let scaled1 = scaled1, let scaled2 = scaled2 {
            data.append(DataPoint(series: 3, timestamp: timestamp, value: scaled1 + scaled2))
        }
        
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
        
        private var background: Gradient {
            return Gradient(colors: [model.bgColor.opacity(0.5), .clear])
        }
        private var gradient: Gradient {
            return Gradient(colors: [model.fgColor, model.fgColor.opacity(0.1)])
        }
        private var lineColor: Color {
            return model.fgColor
        }
        private var lineWidth: Double {
            return 1.0
        }
        
        var body: some View {
                            
            Chart(model.data.filter { $0.series == 1 }) { dataPoint in
                            
                AreaMark(
                    x: .value("Time", dataPoint.timestamp),
                    y: .value("Value", dataPoint.value)
                )
                .interpolationMethod(.catmullRom)
                .foregroundStyle(gradient)
                LineMark(
                    x: .value("Time", dataPoint.timestamp),
                    y: .value("Value", dataPoint.value)
                )
                .lineStyle(StrokeStyle(lineWidth: lineWidth))
                .foregroundStyle(lineColor)
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
        
        private var background: Gradient {
            return Gradient(colors: [model.bgColor.opacity(0.5), .clear])
        }
        private var gradients: KeyValuePairs<Int, Gradient> {
            return [ 1: Gradient(colors: [Color(red: 0.6, green: 0.0, blue: 0.0), Color.clear]),
                     2: Gradient(colors: [Color(red: 0.0, green: 0.6, blue: 0.0), Color.clear])]
        }
        private var lineColor: Color {
            return Color(nsColor: .green)
        }
        private var lineWidth: Double {
            return 1.0
        }

        var body: some View {
                        
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
                    .foregroundStyle(lineColor)
                    .lineStyle(StrokeStyle(lineWidth: lineWidth))
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
