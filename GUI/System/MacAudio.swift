// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import AVFoundation

public class MacAudio: NSObject {
    
    struct Sounds {
        
        static let insert = "insert"
        static let eject = "eject"
        static let step = "drive_head"
        static let move = "hdr_click"
    }
    
    var parent: MyController!
    var audiounit: AUAudioUnit!
    var amiga: EmulatorProxy!

    var prefs: Preferences { return parent.pref }
    
    // Indicates if the this emulator instance owns the audio unit
    var isRunning = false
    
    // Cached audio players
    var audioPlayers: [String: [AVAudioPlayer]] = [:]

    var queue = DispatchQueue(label: "vAmiga.audioplayer.queue")

    override init() {

        super.init()
    }
    
    convenience init?(with controller: MyController) {
    
        self.init()
        parent = controller
        amiga = controller.amiga
        
        // Setup component description for AudioUnit
        let compDesc = AudioComponentDescription(
            componentType: kAudioUnitType_Output,
            componentSubType: kAudioUnitSubType_DefaultOutput,
            componentManufacturer: kAudioUnitManufacturer_Apple,
            componentFlags: 0,
            componentFlagsMask: 0)

        // Create AudioUnit
        do { try audiounit = AUAudioUnit(componentDescription: compDesc) } catch {
            
            warn("Failed to create AUAudioUnit")
            return
        }
        
        // Query AudioUnit
        let hardwareFormat = audiounit.outputBusses[0].format
        let channels = hardwareFormat.channelCount
        let sampleRate = hardwareFormat.sampleRate
        let stereo = (channels > 1)
        
        // Make input bus compatible with output bus
        let renderFormat = AVAudioFormat(standardFormatWithSampleRate: sampleRate,
                                         channels: (stereo ? 2 : 1))
        do { try audiounit.inputBusses[0].setFormat(renderFormat!) } catch {
            
            warn("Failed to set render format on input bus")
            return
        }
        
        // Inform the emulator about the sample rate
        amiga.host.sampleRate = sampleRate

        // Register render callback
        if stereo {
            audiounit.outputProvider = { (
                actionFlags,
                timestamp,
                frameCount,
                inputBusNumber,
                inputDataList ) -> AUAudioUnitStatus in
                
                self.renderStereo(inputDataList: inputDataList, frameCount: frameCount)
                return 0
            }
        } else {
            audiounit.outputProvider = { (
                actionFlags,
                timestamp,
                frameCount,
                inputBusNumber,
                inputDataList ) -> AUAudioUnitStatus in
                
                self.renderMono(inputDataList: inputDataList, frameCount: frameCount)
                return 0
            }
        }
        
        // Allocate render resources
        do { try audiounit.allocateRenderResources() } catch {
            
            warn("Failed to allocate RenderResources")
            return nil
        }

        // Pre-allocate some audio players for playing sound effects
        initAudioPlayers(name: Sounds.insert)
        initAudioPlayers(name: Sounds.eject)
        initAudioPlayers(name: Sounds.step, count: 3)
        initAudioPlayers(name: Sounds.move, count: 3)
    }

    func initAudioPlayers(name: String, count: Int = 1) {

        let url = Bundle.main.url(forResource: name, withExtension: "aiff")!
        initAudioPlayers(name: name, url: url)
    }

    func initAudioPlayers(name: String, url: URL, count: Int = 1) {

        audioPlayers[name] = []

        do {
            for _ in 1 ... count {
                try audioPlayers[name]!.append(AVAudioPlayer(contentsOf: url))
            }
        } catch let error {
            print(error.localizedDescription)
        }
    }

    func shutDown() {

        stopPlayback()
        amiga = nil
    }

    private func renderMono(inputDataList: UnsafeMutablePointer<AudioBufferList>,
                            frameCount: UInt32) {

        let bufferList = UnsafeMutableAudioBufferListPointer(inputDataList)
        assert(bufferList.count == 1)
        
        let ptr = bufferList[0].mData!.assumingMemoryBound(to: Float.self)
        amiga.paula.readMonoSamples(ptr, size: Int(frameCount))
    }

    private func renderStereo(inputDataList: UnsafeMutablePointer<AudioBufferList>,
                              frameCount: UInt32) {

        // track("\(frameCount)")
        let bufferList = UnsafeMutableAudioBufferListPointer(inputDataList)
        assert(bufferList.count > 1)
        
        let ptr1 = bufferList[0].mData!.assumingMemoryBound(to: Float.self)
        let ptr2 = bufferList[1].mData!.assumingMemoryBound(to: Float.self)
        amiga.paula.readStereoSamples(ptr1, buffer2: ptr2, size: Int(frameCount))
    }
    
    // Connects Paula to the audio backend
    @discardableResult
    func startPlayback() -> Bool {

        if !isRunning {
            
            do { try audiounit.startHardware() } catch {
                
                warn("Failed to start audio hardware")
                return false
            }
        }
        
        isRunning = true
        return true
    }
    
    // Disconnects Paula from the audio backend
    func stopPlayback() {

        if isRunning {
            audiounit.stopHardware()
            isRunning = false
        }
    }
    
    // Plays a sound file
    func playSound(_ name: String, volume: Int, pan: Int) {
        
        let scaledVolume = Float(volume) / 100.0
        let p = 0.5 * (sin(Double(pan) * Double.pi / 200.0) + 1)
        
        playSound(name, volume: scaledVolume, pan: Float(p))
    }
    
    func playSound(_ name: String, volume: Float = 1.0, pan: Float = 0.0) {
        
        // Only proceed if the volume is greater 0
        if volume == 0.0 { return }

        // Play sound if a free player is available
        queue.async {

            for player in self.audioPlayers[name]! where !player.isPlaying {

                player.volume = volume
                player.pan = pan
                player.play()
                return
            }
        }
    }
}
