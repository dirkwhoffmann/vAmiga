// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AudioFilterTypes.h"
#include "SubComponent.h"

namespace vamiga {

class AudioFilter : public SubComponent {
    
    friend class Muxer;

    // Current configuration
    AudioFilterConfig config = {};

    // Coefficients of the butterworth filter
    double a1 = 0.0;
    double a2 = 0.0;
    double b0 = 0.0;
    double b1 = 0.0;
    double b2 = 0.0;
    
    // The butterworth filter pipeline
    double x1 = 0.0;
    double x2 = 0.0;
    double y1 = 0.0;
    double y2 = 0.0;
    
    
    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;
    
    
    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override { return "AudioFilter"; }
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from CoreComponent
    //
    
private:
    
    void _reset(bool hard) override { RESET_SNAPSHOT_ITEMS(hard) }
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.filterType
        << config.filterActivation;
    }
    
    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    

    //
    // Configuring
    //

public:

    const AudioFilterConfig &getConfig() const { return config; }
    void resetConfig() override;

    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);

private:

    // Sets the sample rate (only to be called by the Muxer)
    void setSampleRate(double sampleRate);
    
    // Returns the activation status of this filter
    bool isEnabled() const;


    //
    // Using
    //

public:
    
    // Initializes the filter pipeline with zero elements
    void clear();

    // Inserts a sample into the filter pipeline
    float apply(float sample);
};

}
