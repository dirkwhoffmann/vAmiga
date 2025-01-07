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

//
// Time series view
//

class TimeSeries: NSView {
    
    var model = DashboardDataProvider()
    var host: NSHostingView<ContentView>!
    
    struct ContentView: View {
        
        @ObservedObject var model: DashboardDataProvider
        
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
                .chartXScale(domain: Date() - DashboardDataProvider.maxTimeSpan...Date())
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
    
    func configure(range: ClosedRange<Double>, logScale: Bool = false) {
                
        model.range = range
        model.logScale = logScale
    }
}

//
// Pie chart view
//

class ActivityBars: NSView {
    
    var model = DashboardDataProvider()
    var host: NSHostingView<ContentView>!

    struct ContentView: View {
        
        @ObservedObject var model: DashboardDataProvider
        
        private var heading: String {
            return "Heading II" // model.heading
        }
        private var subHeading: String {
            return "Subheading" // model.subHeading
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

        let gradient = Gradient(colors: [.green, .yellow, .orange, .red])
        
        var body: some View {
            
            GeometryReader { geometry in
                
                ZStack() {
                
                    // Gauge
                    VStack {
                        HStack {
                            Spacer()
                            if #available(macOS 14.0, *) {
                                
                                Gauge(value: model.latest(), in: 0.0...1.0) {
                                    Text("")
                                } currentValueLabel: {
                                    Text(String(format: "%.2f", model.latest()))
                                    //                                Text(Double(model.val()), format: .number)
                                }
                                .gaugeStyle(.accessoryCircular)
                                .tint(gradient)
                                .scaleEffect(1.4)
                                .frame(width: 100, height: 100)
                                // .padding(0)
                                // .background(.green)
                                
                            } else { }
                        }
                    }
                    
                    VStack {
                        HStack {
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
                                    .padding(.bottom, 1)
                            }
                            Spacer()
                        }
                        Spacer()
                    }

                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
            .padding(EdgeInsets(top: 10, leading: 15, bottom: 5, trailing: 15))
            .background(background)
            .cornerRadius(10)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
    }
        
    required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)

        model.heading = "Activity"
        model.heading = "Running"

        host = NSHostingView(rootView: ContentView( model: model))
        self.addSubview(host)

        host.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            host.leadingAnchor.constraint(equalTo: self.leadingAnchor),
            host.trailingAnchor.constraint(equalTo: self.trailingAnchor),
            host.topAnchor.constraint(equalTo: self.topAnchor),
            host.bottomAnchor.constraint(equalTo: self.bottomAnchor)
        ])
         
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
