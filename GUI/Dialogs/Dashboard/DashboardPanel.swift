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
// SwiftUI Views
//

struct TimeSeriesView: View {
    
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
            .chartLegend(.hidden)
            .chartForegroundStyleScale(panel.gradients)
        }
        // .background(panel.background)
        .cornerRadius(10)
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding(panel.padding)
    }
}

struct GaugeView: View {
    
    @ObservedObject var model: DashboardDataProvider
    var panel: DashboardPanel
    
    let gradient = Gradient(colors: [.green, .yellow, .orange, .red])
    
    var body: some View {
        
        GeometryReader { geometry in
           
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

                if #available(macOS 14.0, *) {
                    
                    Gauge(value: model.latest(), in: model.range) {
                        Text("")
                    } currentValueLabel: {
                        Text(String(format: "%.2f", model.latest()))
                        //                                Text(Double(model.val()), format: .number)
                    }
                    .gaugeStyle(.accessoryCircular)
                    .tint(gradient)
                    .scaleEffect(1.25)
                    .padding(EdgeInsets(top: 3.0, leading: 0.0, bottom: 0.0, trailing: 0.0))
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                    // .background(.green)
                    
                } else { }
            }
        }
        .padding(panel.padding)
        // .background(panel.background)
        .cornerRadius(10)
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }
}

//
// Wrapper NSView
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
    var padding: EdgeInsets {
        return EdgeInsets(top: 4.0, leading: 4.0, bottom: 4.0, trailing: 4.0)
    }

    var host1: NSHostingView<TimeSeriesView>!
    var host2: NSHostingView<GaugeView>!
    var subview: NSView? { return subviews.isEmpty ? nil : subviews[0] }
    
    required init?(coder aDecoder: NSCoder) {

        super.init(coder: aDecoder)

        host1 = NSHostingView(rootView: TimeSeriesView(model: model, panel: self))
        host2 = NSHostingView(rootView: GaugeView(model: model, panel: self))

        switchStyle()
    }
    
    override func mouseDown(with event: NSEvent) {
        
        switchStyle()
    }
    
    func switchStyle() {
                    
        if subview == host1 {
            subview!.removeFromSuperview()
            addSubview(host2)
        } else if subview == host2 {
            subview!.removeFromSuperview()
            addSubview(host1)
        } else {
            addSubview(host1)
        }
        
        if let subview = subview {
            subview.translatesAutoresizingMaskIntoConstraints = false
            NSLayoutConstraint.activate([
                subview.leadingAnchor.constraint(equalTo: self.leadingAnchor),
                subview.trailingAnchor.constraint(equalTo: self.trailingAnchor),
                subview.topAnchor.constraint(equalTo: self.topAnchor),
                subview.bottomAnchor.constraint(equalTo: self.bottomAnchor)
            ])
        }
    }
}

//
// Custom panels
//

class ChipRamPanel: DashboardPanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
       
        super.init(coder: aDecoder)
        
        configure(title: "Chip Ram",
                  subtitle: "Memory Accesses",
                  range: 0...Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4))
    }
}

class SlowRamPanel: DashboardPanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Slow Ram",
                  subtitle: "Memory Accesses",
                  range: 0...Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4))
    }
}

class FastRamPanel: DashboardPanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Fast Ram",
                  subtitle: "Memory Accesses",
                  range: 0...Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4))
    }
}

class RomPanel: DashboardPanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Kickstart Rom",
                  subtitle: "Memory Accesses",
                  range: 0...Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4))
    }
}

class CopperDmaPanel: DashboardPanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Copper",
                  subtitle: "DMA Accesses",
                  range: 0...(313 * 120),
                  logScale: true)
    }
}

class BlitterDmaPanel: DashboardPanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        
        configure(title: "Blitter",
                  subtitle: "DMA Accesses",
                  range: 0...(313 * 120),
                  logScale: true)
    }
}

class DiskDmaPanel: DashboardPanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Disk", subtitle: "DMA Accesses", range: 0...(313 * 3))
    }
}

class AudioDmaPanel: DashboardPanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Audio", subtitle: "DMA Accesses", range: 0...(313 * 4))
    }
}

class SpriteDmaPanel: DashboardPanel {

    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Sprite", subtitle: "DMA Accesses", range: 0...(313 * 16))
    }
}

class BitplaneDmaPanel: DashboardPanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Bitplane", subtitle: "DMA Accesses", range: 0...39330)
    }
}

class CpuLoadPanel: DashboardPanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Host CPU", subtitle: "Load", range: 0...1.0)
    }
}

class GpuFpsPanel: DashboardPanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Host GPU", subtitle: "Refresh Rate", range: 0...120)
    }
}

class AmigaFrequencyPanel: DashboardPanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Amiga", subtitle: "CPU Frequency", range: 0...14)
    }
}

class AmigaFpsPanel: DashboardPanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Amiga", subtitle: "Refresh Rate", range: 0...120)
    }
}

class CIAAPanel: DashboardPanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "CIA A", subtitle: "Awakeness")
    }
}

class CIABPanel: DashboardPanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "CIA B", subtitle: "Awakeness")
    }
}

class AudioFillLevelPanel: DashboardPanel {
    
    @MainActor required init?(coder aDecoder: NSCoder) {
        
        super.init(coder: aDecoder)
        configure(title: "Audio Buffer", subtitle: "Fill Level")
    }
}
