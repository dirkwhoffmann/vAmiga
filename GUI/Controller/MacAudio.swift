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

        track()
    
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
            track("Failed to create AUAudioUnit")
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
            track("Failed to set render format on input bus")
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
            track("Failed to allocate RenderResources")
            return nil
        }
    }

    func shutDown() {

        track()
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
                track("Failed to start audio hardware")
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
    func playStepSound(drive id: Int) {
        
        playSound(name: "drive_head",
                  volume: parent.amiga.getConfig(.STEP_VOLUME, drive: id),
                  pan: parent.amiga.getConfig(.DRIVE_PAN, drive: id))
    }

    func playPollSound(drive id: Int) {
                
        playSound(name: "drive_head",
                  volume: parent.amiga.getConfig(.POLL_VOLUME, drive: id),
                  pan: parent.amiga.getConfig(.DRIVE_PAN, drive: id))
    }

    func playInsertSound(drive id: Int) {
        
        playSound(name: "insert",
                  volume: parent.amiga.getConfig(.INSERT_VOLUME, drive: id),
                  pan: parent.amiga.getConfig(.DRIVE_PAN, drive: id))
    }
 
    func playEjectSound(drive id: Int) {
        
        playSound(name: "eject",
                  volume: parent.amiga.getConfig(.EJECT_VOLUME, drive: id),
                  pan: parent.amiga.getConfig(.DRIVE_PAN, drive: id))
    }
    
    func playSound(name: String, volume: Int, pan: Int) {
        
        let p = pan <= 100 ? pan : pan <= 300 ? 200 - pan : -400 + pan
        
        let scaledVolume = Float(volume) / 100.0
        let scaledPan = Float(p) / 100.0
        
        // track("\(name) \(scaledVolume) \(scaledPan)")
        
        playSound(name: name, volume: scaledVolume, pan: scaledPan)
    }
    
    func playSound(name: String, volume: Float = 1.0, pan: Float = 0.0) {
        
        // Only proceed if the volume is greater 0
        if volume == 0.0 { return }
        
        // Check for cached players for this sound file
        if audioPlayers[name] == nil {
            
            // Lookup sound file in bundle
            guard let url = Bundle.main.url(forResource: name, withExtension: "aiff") else {
                track("Cannot open sound file \(name)")
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
