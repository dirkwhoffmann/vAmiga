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
        
        static let step = "drive_head"
        static let insert = "insert"
        static let eject = "eject"
        static let move = "hdr_click"
    }
    
    var parent: MyController!
    var audiounit: AUAudioUnit!
    var paula: PaulaProxy!

    var prefs: Preferences { return parent.pref }
    
    // Indicates if the this emulator instance owns the audio unit
    var isRunning = false
    
    // Cached audio players
    var audioPlayers: [String: [AVAudioPlayer]] = [:]
    
    override init() {

        super.init()
    }
    
    convenience init?(with controller: MyController) {
    
        self.init()
        parent = controller
        paula = controller.amiga.paula
        
        // Setup component description for AudioUnit
        let compDesc = AudioComponentDescription(
            componentType: kAudioUnitType_Output,
            componentSubType: kAudioUnitSubType_DefaultOutput,
            componentManufacturer: kAudioUnitManufacturer_Apple,
            componentFlags: 0,
            componentFlagsMask: 0)

        // Create AudioUnit
        do { try audiounit = AUAudioUnit(componentDescription: compDesc) } catch {
            
            log(warning: "Failed to create AUAudioUnit")
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
            
            log(warning: "Failed to set render format on input bus")
            return
        }
        
        // Inform Paula about the sample rate
        paula.setSampleRate(sampleRate)
        
        // Register render callback
        if stereo {
            audiounit.outputProvider = { ( // AURenderPullInputBlock
                actionFlags,
                timestamp,
                frameCount,
                inputBusNumber,
                inputDataList ) -> AUAudioUnitStatus in
                
                self.renderStereo(inputDataList: inputDataList, frameCount: frameCount)
                return 0
            }
        } else {
            audiounit.outputProvider = { ( // AURenderPullInputBlock
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
            
            log(warning: "Failed to allocate RenderResources")
            return nil
        }
    }

    func shutDown() {

        stopPlayback()
        paula = nil
    }

    private func renderMono(inputDataList: UnsafeMutablePointer<AudioBufferList>,
                            frameCount: UInt32) {

        let bufferList = UnsafeMutableAudioBufferListPointer(inputDataList)
        assert(bufferList.count == 1)
        
        let ptr = bufferList[0].mData!.assumingMemoryBound(to: Float.self)
        paula.readMonoSamples(ptr, size: Int(frameCount))
    }

    private func renderStereo(inputDataList: UnsafeMutablePointer<AudioBufferList>,
                              frameCount: UInt32) {

        // track("\(frameCount)")
        let bufferList = UnsafeMutableAudioBufferListPointer(inputDataList)
        assert(bufferList.count > 1)
        
        let ptr1 = bufferList[0].mData!.assumingMemoryBound(to: Float.self)
        let ptr2 = bufferList[1].mData!.assumingMemoryBound(to: Float.self)
        paula.readStereoSamples(ptr1, buffer2: ptr2, size: Int(frameCount))
    }
    
    // Connects Paula to the audio backend
    @discardableResult
    func startPlayback() -> Bool {

        if !isRunning {
            
            do { try audiounit.startHardware() } catch {
                
                log(warning: "Failed to start audio hardware")
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
        
        // Check for cached players for this sound file
        if audioPlayers[name] == nil {
            
            // Lookup sound file in bundle
            guard let url = Bundle.main.url(forResource: name, withExtension: "aiff") else {

                log(warning: "Cannot open sound file \(name)")
                return
            }
            
            // Create a couple of player instances for this sound file
            do {
                audioPlayers[name] = []
                try audioPlayers[name]!.append(AVAudioPlayer(contentsOf: url))
                try audioPlayers[name]!.append(AVAudioPlayer(contentsOf: url))
                try audioPlayers[name]!.append(AVAudioPlayer(contentsOf: url))
            } catch let error {
                print(error.localizedDescription)
            }
        }
        
        // Play sound if a free player is available
        for player in audioPlayers[name]! where !player.isPlaying {
            
            player.volume = volume
            player.pan = pan
            player.play()
            return
        }
    }
}
