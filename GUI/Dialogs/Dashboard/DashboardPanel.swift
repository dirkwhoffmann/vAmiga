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

class DashboardPanel: NSView {

    var model = DashboardDataProvider()

    // Title and sub title
    var heading = ""
    var subHeading = ""
    
    // Colors and gradients
    var graph1Color = Color(NSColor.init(r: 0x33, g: 0x99, b: 0xFF))
    var graph2Color = Color(NSColor.init(r: 0xFF, g: 0x33, b: 0x99))
    var lineColor = Color.gray
    var headingColor = Color.white
    var subheadingColor = Color.gray

    var themeColor: NSColor = .white {
        didSet {
            lineColor = Color(themeColor).opacity(0.6)
            graph1Color = Color(themeColor)
            graph2Color = Color(themeColor)
            headingColor = Color(themeColor)
        }
    }

    func configure(title: String, subtitle: String, range: ClosedRange<Double> = 0...1, logScale: Bool = false) {

        heading = title
        subHeading = subtitle
        model.logScale = logScale
        model.range = range
    }
    var background: Gradient {
        return Gradient(colors: [Color.black, Color.black])
    }
    var gradients: KeyValuePairs<Int, Gradient> {
        return [ 1: Gradient(colors: [graph1Color.opacity(0.75), graph1Color.opacity(0.25)]),
                 2: Gradient(colors: [graph2Color.opacity(0.75), graph2Color.opacity(0.25)])]
    }
    var gridLineColor: Color {
        return Color.white.opacity(0.6)
    }
}

class TimeLinePanel: DashboardPanel {
    
    var host: NSHostingView<ContentView>!
    
    struct ContentView: View {
        
        @ObservedObject var model: DashboardDataProvider
        var panel: DashboardPanel!
        
        var body: some View {
            
            VStack(alignment: .leading) {
                
                VStack(alignment: .leading) {
                    Text(panel.heading)
                        .font(.system(size: 14))
                        .fontWeight(.bold)
                        .foregroundColor(panel.headingColor)
                        .padding(.bottom, 1)
                    Text(panel.subHeading)
                        .font(.system(size: 8))
                        .fontWeight(.regular)
                        .foregroundColor(panel.subheadingColor)
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
                        .foregroundStyle(panel.lineColor)
                        .lineStyle(StrokeStyle(lineWidth: 1.25))
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
                .chartYScale(domain: model.range)
                .chartYAxis {
                    AxisMarks(values: model.gridLines) {
                        AxisGridLine()
                            .foregroundStyle(panel.gridLineColor)
                    }
                }
                .padding(EdgeInsets(top: 0.0, leading: 15.0, bottom: 15.0, trailing: 15.0))
                .chartLegend(.hidden)
                .chartForegroundStyleScale(panel.gradients)
            }
            .background(panel.background)
            .cornerRadius(10)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
    }
    
    required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        host = NSHostingView(rootView: ContentView( model: model, panel: self))
        self.addSubview(host)
        
        host.translatesAutoresizingMaskIntoConstraints = false
        host.leadingAnchor.constraint(equalTo: self.leadingAnchor).isActive = true
        host.trailingAnchor.constraint(equalTo: self.trailingAnchor).isActive = true
        host.topAnchor.constraint(equalTo: self.topAnchor).isActive = true
        host.bottomAnchor.constraint(equalTo: self.bottomAnchor).isActive = true
    }
}

//
// Gauge view
//

class GaugePanel: DashboardPanel {
    
    var host: NSHostingView<ContentView>!

    struct ContentView: View {
        
        @ObservedObject var model: DashboardDataProvider
        var panel: DashboardPanel
        
        private var themeColor: Color {
            return Color(nsColor: panel.themeColor)
        }
        private var graph1Color: Color {
            return panel.graph1Color
        }
        private var graph2Color: Color {
            return panel.graph2Color
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
                                
                                Gauge(value: model.latest(), in: model.range) {
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
                                
                                Text(panel.heading)
                                    .font(.system(size: 14))
                                    .fontWeight(.bold)
                                    .foregroundColor(themeColor.opacity(1.0))
                                    .padding(.bottom, 1)
                                Text(panel.subHeading)
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

        host = NSHostingView(rootView: ContentView(model: model, panel: self))
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

class ChipRamPanel: TimeLinePanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
       
        super.init(coder: aDecoder)
        
        configure(title: "Chip Ram",
                  subtitle: "Memory Accesses",
                  range: 0...Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4))
    }
}

class SlowRamPanel: TimeLinePanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Slow Ram",
                  subtitle: "Memory Accesses",
                  range: 0...Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4))
    }
}

class FastRamPanel: TimeLinePanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Fast Ram",
                  subtitle: "Memory Accesses",
                  range: 0...Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4))
        
    }
}

class RomPanel: TimeLinePanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Kickstart Rom",
                  subtitle: "Memory Accesses",
                  range: 0...Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4))
    }
}

class CopperDmaPanel: TimeLinePanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Copper",
                  subtitle: "DMA Accesses",
                  range: 0...(313 * 120),
                  logScale: true)
    }
}

class BlitterDmaPanel: TimeLinePanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Blitter",
                  subtitle: "DMA Accesses",
                  range: 0...(313 * 120),
                  logScale: true)
    }
}

class DiskDmaPanel: TimeLinePanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Disk", subtitle: "DMA Accesses", range: 0...(313 * 3))
    }
}

class AudioDmaPanel: TimeLinePanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Audio", subtitle: "DMA Accesses", range: 0...(313 * 4))
    }
}

class SpriteDmaPanel: TimeLinePanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Sprite", subtitle: "DMA Accesses", range: 0...(313 * 16))
    }
}

class BitplaneDmaPanel: TimeLinePanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Bitplane", subtitle: "DMA Accesses", range: 0...39330)
    }
}

class CpuLoadPanel: GaugePanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Host CPU", subtitle: "Load", range: 0...1.0)
    }
}

class GpuFpsPanel: GaugePanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Host GPU", subtitle: "Refresh Rate", range: 0...120)
    }
}

class AmigaFrequencyPanel: GaugePanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Amiga", subtitle: "CPU Frequency", range: 0...14)
    }
}

class AmigaFpsPanel: GaugePanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Amiga", subtitle: "Refresh Rate", range: 0...120)
    }
}

class CIAAPanel: GaugePanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "CIA A", subtitle: "Awakeness")
    }
}

class CIABPanel: GaugePanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "CIA B", subtitle: "Awakeness")
    }
}

class AudioFillLevelPanel: GaugePanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Audio Buffer", subtitle: "Fill Level")
    }
}
