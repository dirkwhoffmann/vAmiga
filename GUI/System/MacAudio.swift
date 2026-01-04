// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import AVFoundation

@MainActor
public class MacAudio: NSObject {
    
    struct Sounds {
        
        static let insert = "insert"
        static let eject = "eject"
        static let step = "drive_head"
        static let move = "hdr_click"
    }
    
    var parent: MyController!

    // Audio source
    var emu: EmulatorProxy? { return parent.emu }

    // Component state
    enum MacAudioState { case off, on, shutdown }
    var state = MacAudioState.off

    // Gateway to the host's audio unit
    var audiounit: AUAudioUnit!

    // Lock that is kept while the component is active
    var lock = NSLock()
    
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

        // Create AudioUnit
        let compDesc = AudioComponentDescription(
            componentType: kAudioUnitType_Output,
            componentSubType: kAudioUnitSubType_DefaultOutput,
            componentManufacturer: kAudioUnitManufacturer_Apple,
            componentFlags: 0,
            componentFlagsMask: 0)

        do { try audiounit = AUAudioUnit(componentDescription: compDesc) } catch {
            
            warn("Failed to create the audio unit.")
            return
        }
        
        // Query parameters
        let hardwareFormat = audiounit.outputBusses[0].format
        let channels = hardwareFormat.channelCount
        let sampleRate = hardwareFormat.sampleRate
        let stereo = (channels > 1)
        
        // Pass some host parameters to the emulator
        emu?.set(.HOST_SAMPLE_RATE, value: Int(sampleRate))

        // Make input bus compatible with output bus
        let renderFormat = AVAudioFormat(standardFormatWithSampleRate: sampleRate,
                                         channels: (stereo ? 2 : 1))
        do { try audiounit.inputBusses[0].setFormat(renderFormat!) } catch {
            
            warn("Failed to set render format on input bus")
            return
        }
        
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
        do { try audiounit.startHardware() } catch {

            warn("Failed to start the audio hardware.")
            return
        }

        // Pre-allocate some audio players for playing sound effects
        initAudioPlayers(name: Sounds.insert)
        initAudioPlayers(name: Sounds.eject)
        initAudioPlayers(name: Sounds.step, count: 3)
        initAudioPlayers(name: Sounds.move, count: 3)

        isRunning = true

        // The audio unit is up and running. Switch to 'on' state
        lock.lock()
        state = .on
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

        loginfo(.shutdown, "Initiating shutdown...")
        state = .shutdown

        loginfo(.shutdown, "Fading out...")
        lock.lock()
        precondition(state == .off)

        loginfo(.shutdown, "Stopping audio hardware...")
        audiounit.stopHardware()
        audiounit.outputProvider = nil
    }

    private func renderMono(inputDataList: UnsafeMutablePointer<AudioBufferList>,
                            frameCount: UInt32) {

        let bufferList = UnsafeMutableAudioBufferListPointer(inputDataList)
        assert(bufferList.count == 1)
        
        let ptr = bufferList[0].mData!.assumingMemoryBound(to: Float.self)
        let n = Int(frameCount)

        switch state {

        case .on:

            emu?.audioPort.copyMono(ptr, size: n)

        case .shutdown:

            if let cnt = emu?.audioPort.copyMono(ptr, size: n) {

                loginfo(.shutdown, "Copied \(cnt) mono samples.")
                if cnt == n { break }
            }

            loginfo(.shutdown, "Successfully faded out.")
            state = .off
            lock.unlock()

        case .off:

            memset(ptr, 0, 4 * n)
        }
    }

    private func renderStereo(inputDataList: UnsafeMutablePointer<AudioBufferList>,
                              frameCount: UInt32) {

        let bufferList = UnsafeMutableAudioBufferListPointer(inputDataList)
        assert(bufferList.count > 1)
        
        let ptr1 = bufferList[0].mData!.assumingMemoryBound(to: Float.self)
        let ptr2 = bufferList[1].mData!.assumingMemoryBound(to: Float.self)
        let n = Int(frameCount)

        switch state {

        case .on:

            emu?.audioPort.copyStereo(ptr1, buffer2: ptr2, size: n)

        case .shutdown:

            if let cnt = emu?.audioPort.copyStereo(ptr1, buffer2: ptr2, size: n) {

                loginfo(.shutdown, "Copied \(cnt) stereo samples.")
                if cnt == n { break }
            }

            loginfo(.shutdown, "Successfully faded out.")
            state = .off
            lock.unlock()

        case .off:

            memset(ptr1, 0, 4 * n)
            memset(ptr2, 0, 4 * n)
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
        Task { @MainActor in
        // queue.async {

            for player in self.audioPlayers[name]! where !player.isPlaying {

                player.volume = volume
                player.pan = pan
                player.play()
                return
            }
        }
    }
}
