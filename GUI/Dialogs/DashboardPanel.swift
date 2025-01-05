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
    var timestamp: Date
    var value: Double
}

class DataSource: ObservableObject {
    
    @Published var update = false

    static let maxTimeSpan = 5.0
    
    var data: [DataPoint] = []
    var yscale: ClosedRange<Double> = 0.0...1.0
    var fgColor: Color = .red
    var bgColor: Color = .white
    var scale = [0.0, 0.25, 0.5, 0.75, 1.0]
    var lines: Int { return logScale ? 10 : 5 }
    
    var logScale = false {
        didSet {
            if (logScale) {
                scale = []
                for i in 0 ... lines {
                    var y = Double(i) / Double(lines)
                    if logScale { y = log(1.0 + 19.0 * y) / log(20) }
                    scale.append(y)
                }
            } else {
                scale = [0.0, 0.25, 0.5, 0.75, 1.0]
            }
        }
    }
            
    init() {
        
    }
    
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
    
    func addLog(value: Double) {
                
        logScale = true
        add(value: log(1.0 + (19 * value)) / log(20), timestamp: Date.now)
    }

    func add(value: Double) {
        
        add(value: value, timestamp: Date.now)
    }
    
    func add(value: Double, timestamp: Date) {
    
        data.append(DataPoint(timestamp: timestamp, value: value))
        while timeSpan > DataSource.maxTimeSpan { data.removeFirst() }
        update.toggle()
    }
}

struct ContentView: View {
    
    @ObservedObject var model: DataSource
    
    private var domain: ClosedRange<Date> {
        return Date() - 5...Date()
    }
    private var areaBackground: Gradient {
        return Gradient(colors: [model.fgColor, model.fgColor.opacity(0.1)])
    }
    private var background: Gradient {
        return Gradient(colors: [model.bgColor.opacity(0.5), .clear])
    }
    
    var body: some View {
                    
        let logValues = stride(from: 0.0, through: 1.0, by: 0.1).map { pow(10, $0) }
        
        let data = model.data
        
        Chart(data) { dataPoint in
            AreaMark(
                x: .value("Time", dataPoint.timestamp),
                y: .value("Value", dataPoint.value)
            )
            .interpolationMethod(.catmullRom)
            .foregroundStyle(areaBackground)
            .lineStyle(StrokeStyle(lineWidth: 3))
            LineMark(
                x: .value("Time", dataPoint.timestamp),
                y: .value("Value", dataPoint.value)
            )
            .lineStyle(StrokeStyle(lineWidth: 2))
            .foregroundStyle(model.fgColor)
        }
        .chartXScale(domain: domain)
        .chartXAxis(.hidden)
        .chartYScale(domain: model.yscale)
        .chartYAxis {
            AxisMarks(values: model.scale) { _ in AxisGridLine() }
        }
        /*
        .chartYAxis {
            AxisMarks(values: Array(stride(from: 0.0, through: 1.0, by: 0.25))) { _ in
                        AxisGridLine() // Only draw grid lines
                    }
                }
         */
        /*
        .chartXAxis {
            AxisMarks(values: .automatic) { value in
                AxisValueLabel(format: .dateTime.minute().second(),
                               orientation: .horizontal)
            }
        }
        */
        .padding(EdgeInsets(top: 10.0, leading: 10.0, bottom: 10.0, trailing: 10.0))
        .background(background)
    }
    
    func FormattedDate(date: Date) -> String {
            let formatter = DateFormatter()
            formatter.dateFormat = "mm:ss" // Format as hour:minute
            return formatter.string(from: date)
        }
}

class MyChartView: NSView {

    @IBOutlet weak var monitor: Monitor!
    
    var model = DataSource()
    var timer: Timer?
    var hostingView: NSHostingView<ContentView>!
        
    required init?(coder aDecoder: NSCoder) {

        super.init(coder: aDecoder)
        
        hostingView = NSHostingView(rootView: ContentView( model: model) ) // MyChart())
        hostingView.translatesAutoresizingMaskIntoConstraints = false
        self.addSubview(hostingView)
        hostingView.leadingAnchor.constraint(equalTo: self.leadingAnchor).isActive = true
        hostingView.trailingAnchor.constraint(equalTo: self.trailingAnchor).isActive = true
        hostingView.topAnchor.constraint(equalTo: self.topAnchor).isActive = true
        hostingView.bottomAnchor.constraint(equalTo: self.bottomAnchor).isActive = true
    }
    
    func add(value: Double) {

        model.add(value: value)
    }
}
