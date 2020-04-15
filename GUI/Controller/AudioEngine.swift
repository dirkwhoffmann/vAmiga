// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import AVFoundation

public class AudioEngine: NSObject {

    // References
    var parent: MyController!
    var paula: PaulaProxy!
    var audiounit: AUAudioUnit!
    
    // Indicates if the this emulator instance owns the audio unit
    var isRunning = false

    // Reference storage used by playSound()
    var audioPlayers: [AVAudioPlayer] = []
    
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
    func playSound(name: String, volume: Float = 1.0) {
                
        guard let url = Bundle.main.url(forResource: name, withExtension: "aiff") else {
            track("Cannot open sound file \(name)")
            return
        }
        
        // Remove references to outdated players
        audioPlayers.removeAll(where: { !$0.isPlaying })

        // Bail out if the number of active players is still too high
        if audioPlayers.count >= 3 { return }

        // Play sound
        do {
            let player = try AVAudioPlayer(contentsOf: url)

            player.volume = volume
            player.pan = Float(parent.driveSoundPan)
            player.play()

            // Keep a reference to the player to avoid early deletion
            audioPlayers.append(player)

        } catch let error {
            print(error.localizedDescription)
        }
    }
}
