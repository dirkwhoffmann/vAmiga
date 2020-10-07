// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

PaulaAudio::PaulaAudio(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("AudioUnit");
    
    subComponents = vector<HardwareComponent *> {

        &channel0,
        &channel1,
        &channel2,
        &channel3,
        &muxer,
        &filterL,
        &filterR
    };
}

void
PaulaAudio::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    muxer.clear();
}

long
PaulaAudio::getConfigItem(ConfigOption option)
{
    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            return config.samplingMethod;
            
        case OPT_FILTER_TYPE:
            assert(filterL.getFilterType() == config.filterType);
            assert(filterR.getFilterType() == config.filterType);
            return config.filterType;
            
        case OPT_FILTER_ALWAYS_ON:
            return config.filterAlwaysOn;
            
        case OPT_AUDVOLL:
            return (long)(exp2(config.volL) * 100.0);

        case OPT_AUDVOLR:
            return (long)(exp2(config.volR) * 100.0);

        case OPT_AUDVOL0:
            return (long)(exp2(config.vol[0] / 0.0000025) * 100.0);

        case OPT_AUDVOL1:
            return (long)(exp2(config.vol[1] / 0.0000025) * 100.0);
            
        case OPT_AUDVOL2:
            return (long)(exp2(config.vol[2] / 0.0000025) * 100.0);
            
        case OPT_AUDVOL3:
            return (long)(exp2(config.vol[3] / 0.0000025) * 100.0);

        case OPT_AUDPAN0:
            return (long)(config.pan[0] * 100.0);
            
        case OPT_AUDPAN1:
            return (long)(config.pan[1] * 100.0);
            
        case OPT_AUDPAN2:
            return (long)(config.pan[2] * 100.0);
            
        case OPT_AUDPAN3:
            return (long)(config.pan[3] * 100.0);

        default: assert(false);
    }
}

bool
PaulaAudio::setConfigItem(ConfigOption option, long value)
{
    bool wasMuted = isMuted();
    
    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            
            if (!isSamplingMethod(value)) {
                warn("Invalid filter activation: %d\n", value);
                return false;
            }
            break;
            
        case OPT_FILTER_TYPE:
            
            if (!isFilterType(value)) {
                warn("Invalid filter type: %d\n", value);
                warn("       Valid values: 0 ... %d\n", FILT_COUNT - 1);
                return false;
            }
            break;
            
        case OPT_AUDVOLL:
        case OPT_AUDVOLR:
        case OPT_AUDVOL0:
        case OPT_AUDVOL1:
        case OPT_AUDVOL2:
        case OPT_AUDVOL3:
            
            if (value < 100 || value > 400) {
                warn("Invalid volumne: %d\n", value);
                warn("       Valid values: 100 ... 400\n");
                return false;
            }
            break;
            
        case OPT_AUDPAN0:
        case OPT_AUDPAN1:
        case OPT_AUDPAN2:
        case OPT_AUDPAN3:
            
            if (value < 0 || value > 100) {
                warn("Invalid pan: %d\n", value);
                warn("       Valid values: 0 ... 100\n");
                return false;
            }
            break;
            
        default:
            break;
    }

    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            
            if (config.samplingMethod == value) {
                return false;
            }
            
            config.samplingMethod = (SamplingMethod)value;
            return true;
            
        case OPT_FILTER_TYPE:
            
            if (config.filterType == value) {
                return false;
            }

            config.filterType = (FilterType)value;
            filterL.setFilterType((FilterType)value);
            filterR.setFilterType((FilterType)value);
            return true;
            
        case OPT_FILTER_ALWAYS_ON:
            
            if (config.filterAlwaysOn == value) {
                return false;
            }
            
            config.filterAlwaysOn = value;
            return true;
            
        case OPT_AUDVOLL:
            
            config.volL = log2((double)value / 100.0);
            if (wasMuted != isMuted())
                messageQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return true;
            
        case OPT_AUDVOLR:            

            config.volR = log2((double)value / 100.0);
            if (wasMuted != isMuted())
                messageQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return true;
            
        case OPT_AUDVOL0:
            
            config.vol[0] = log2((double)value / 100.0) * 0.0000025;
            return true;
            
        case OPT_AUDVOL1:
            
            config.vol[1] = log2((double)value / 100.0) * 0.0000025;
            return true;

        case OPT_AUDVOL2:
            
            config.vol[2] = log2((double)value / 100.0) * 0.0000025;
            return true;

        case OPT_AUDVOL3:
            
            config.vol[3] = log2((double)value / 100.0) * 0.0000025;
            return true;

        case OPT_AUDPAN0:
            
            config.pan[0] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        case OPT_AUDPAN1:
            config.pan[1] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        case OPT_AUDPAN2:
            
            config.pan[2] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        case OPT_AUDPAN3:
            
            config.pan[3] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        default:
            return false;
    }
}

void
PaulaAudio::setSampleRate(double hz)
{
    debug(AUD_DEBUG, "setSampleRate(%f)\n", hz);

    config.sampleRate = hz;
    cyclesPerSample = MHz(masterClockFrequency) / hz;

    filterL.setSampleRate(hz);
    filterR.setSampleRate(hz);
}

size_t
PaulaAudio::didLoadFromBuffer(u8 *buffer)
{
    muxer.clear();
    return 0;
}

void
PaulaAudio::_inspect()
{
    synchronized {
        
        info.channel[0] = channel0.getInfo();
        info.channel[1] = channel1.getInfo();
        info.channel[2] = channel2.getInfo();
        info.channel[3] = channel3.getInfo();
    }
}

void
PaulaAudio::_dump()
{
}

void
PaulaAudio::_run()
{
    muxer.clear();
}

void
PaulaAudio::_pause()
{
    muxer.clear();
}

void
PaulaAudio::executeUntil(Cycle target)
{
    muxer.synthesize(clock, target);
    clock = target;
}

void
PaulaAudio::pokeAUDxPER(int nr, u16 value)
{
    switch (nr) {
        case 0: channel0.pokeAUDxPER(value); return;
        case 1: channel1.pokeAUDxPER(value); return;
        case 2: channel2.pokeAUDxPER(value); return;
        case 3: channel3.pokeAUDxPER(value); return;
    }
    assert(false);
}

void
PaulaAudio::pokeAUDxVOL(int nr, u16 value)
{
    switch (nr) {
         case 0: channel0.pokeAUDxVOL(value); return;
         case 1: channel1.pokeAUDxVOL(value); return;
         case 2: channel2.pokeAUDxVOL(value); return;
         case 3: channel3.pokeAUDxVOL(value); return;
     }
     assert(false);
}

void
PaulaAudio::rampUp()
{
    // Only proceed if the emulator is not running in warp mode
    if (warpMode) return;
    
    targetVolume = maxVolume;
    volumeDelta = 3;
    muxer.ignoreNextUnderOrOverflow();
}

void
PaulaAudio::rampUpFromZero()
{
    volume = 0;
    rampUp();
}
 
void
PaulaAudio::rampDown()
{    
    targetVolume = 0;
    volumeDelta = 50;
    muxer.ignoreNextUnderOrOverflow();
}

void
PaulaAudio::readMonoSamples(float *buffer, size_t n)
{
    muxer.copyMono(buffer, n, volume, targetVolume, volumeDelta);
}

void
PaulaAudio::readStereoSamples(float *left, float *right, size_t n)
{
    muxer.copy(left, right, n, volume, targetVolume, volumeDelta);
}

template<> u8 PaulaAudio::getState<0>() { return channel0.state; }
template<> u8 PaulaAudio::getState<1>() { return channel1.state; }
template<> u8 PaulaAudio::getState<2>() { return channel2.state; }
template<> u8 PaulaAudio::getState<3>() { return channel3.state; }
