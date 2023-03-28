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

//
// Butterworth filter (used in vAmiga up to v2.3)
//

struct ButterworthFilter : CoreObject {

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

    void setSampleRate(double sampleRate);

    const char *getDescription() const override { return "Butterworth"; }
    void _dump(Category category, std::ostream& os) const override;

    // Initializes the filter pipeline with zero elements
    void clear();

    // Inserts a sample into the filter pipeline
    float apply(float sample);
};

//
// OnePoleFilter (based on 8bitbubsy/pt2-clone)
//

struct OnePoleFilter : CoreObject {

    double tmpL, tmpR, a1, a2;

    const char *getDescription() const override { return "OnePoleFilter"; }
    void _dump(Category category, std::ostream& os) const override;

    // Initializes the filter coeeficients
    void setup(double sampleRate, double cutOff);

    // Initializes the filter pipeline with zero elements
    void clear();

    // Inserts a sample into the filter pipeline
    void apply(double &l, double &r);
};

struct TwoPoleFilter : CoreObject {

    double tmpL[4], tmpR[4], a1, a2, b1, b2;

    const char *getDescription() const override { return "TwoPoleFilter"; }
    void _dump(Category category, std::ostream& os) const override;

    // Initializes the filter coeeficients
    void setup(double sampleRate, double cutOff, double qFactor);

    // Initializes the filter pipeline with zero elements
    void clear();

    // Inserts a sample into the filter pipeline
    void apply(double &l, double &r);
};


class AudioFilter : public SubComponent {
    
    friend class Muxer;

public:

    static const double pi;

private:

    // Current configuration
    AudioFilterConfig config = {};

    // The filter pipeline
    OnePoleFilter loFilter;
    TwoPoleFilter ledFilter;
    OnePoleFilter hiFilter;

    // Legacy filters (used up to vAmiga 2.4b1)
    ButterworthFilter butterworthL;
    ButterworthFilter butterworthR;


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

        << config.filterType;
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

    // Sets up the filter based on the current filter type and sample rate
    void setup(double sampleRate);
    void setupLoFilter(double sampleRate);
    void setupLedFilter(double sampleRate);
    void setupHiFilter(double sampleRate);

    
    //
    // Querying
    //

    bool loFilterEnabled() const;
    bool ledFilterEnabled() const;
    bool hiFilterEnabled() const;


    //
    // Using
    //

public:
    
    // Initializes the filter pipeline with zero elements
    void clear();
};

}
