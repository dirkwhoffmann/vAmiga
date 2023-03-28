// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Amiga.h"
#include "CIA.h"
#include <cmath>

namespace vamiga {

//
// Butterworth filter (DEPRECATED)
//

void
ButterworthFilter::setSampleRate(double sampleRate)
{
    // Compute butterworth filter coefficients based on
    // https://stackoverflow.com/questions/
    //  20924868/calculate-coefficients-of-2nd-order-butterworth-low-pass-filter

    // Cutoff frequency in Hz
    const double f_cutoff = 4500;

    // Frequency ratio
    const double ff = f_cutoff / sampleRate;

    // Compute coefficients
    const double ita = 1.0/ tan(M_PI * ff);
    const double q = sqrt(2.0);

    b0 = 1.0 / (1.0 + q * ita + ita * ita);
    b1 = 2 * b0;
    b2 = b0;
    a1 = 2.0 * (ita * ita - 1.0) * b0;
    a2 = -(1.0 - q * ita + ita * ita) * b0;
}

void
ButterworthFilter::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::State) {

        os << tab("a1");
        os << flt(a1) << std::endl;
        os << tab("a2");
        os << flt(a2) << std::endl;
        os << tab("b0");
        os << flt(b0) << std::endl;
        os << tab("b1");
        os << flt(b1) << std::endl;
        os << tab("b2");
        os << flt(b2) << std::endl;
    }
}

void
ButterworthFilter::clear()
{
    x1 = x2 = y1 = y2 = 0.0;
}

float
ButterworthFilter::apply(float sample)
{
    // Run pipeline
    double x0 = (double)sample;
    double y0 = (b0 * x0) + (b1 * x1) + (b2 * x2) + (a1 * y1) + (a2 * y2);

    // Shift pipeline
    x2 = x1; x1 = x0;
    y2 = y1; y1 = y0;

    return (float)y0;
}


//
// OnePoleFilter
//

void
OnePoleFilter::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::State) {

    }
}

void
OnePoleFilter::setup(double sampleRate, double cutOff)
{
    if (cutOff >= sampleRate / 2.0) cutOff = (sampleRate / 2.0) - 1e-4;

    const double a = 2.0 - std::cos((2.0 * AudioFilter::pi * cutOff) / sampleRate);
    const double b = a - std::sqrt((a * a) - 1.0);

    a1 = 1.0 - b;
    a2 = b;
}

void
OnePoleFilter::clear()
{
    tmpL = tmpR = 0.0;
}

void
OnePoleFilter::apply(double &l, double &r)
{
    tmpL = (a1 * l) + (a2 * tmpL);
    l = tmpL;

    tmpR = (a1 * r) + (a2 * tmpR);
    r = tmpR;
}


//
// TwoPoleFilter
//

void
TwoPoleFilter::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::State) {

    }
}

void
TwoPoleFilter::setup(double sampleRate, double cutOff, double qFactor)
{
    if (cutOff >= sampleRate / 2.0) cutOff = (sampleRate / 2.0) - 1e-4;

    const double a = 1.0 / std::tan((2.0 * AudioFilter::pi * cutOff) / sampleRate);
    const double b = 1.0 / qFactor;

    a1 = 1.0 / (1.0 + b * a + a * a);
    a2 = 2.0 * a1;
    b1 = 2.0 * (1.0 - a*a) * a1;
    b2 = (1.0 - b * a + a * a) * a1;
}

void
TwoPoleFilter::clear()
{
    tmpL[0] = tmpL[1] = tmpL[2] = tmpL[3] = 0.0;
    tmpR[0] = tmpR[1] = tmpR[2] = tmpR[3] = 0.0;
}

void
TwoPoleFilter::apply(double &l, double &r)
{
    auto inl = l;
    auto inr = r;

    l = (a1 * inl) + (a2 * tmpL[0]) + (a1 * tmpL[1]) - (b1 * tmpL[2]) - (b2 * tmpL[3]);
    r = (a1 * inr) + (a2 * tmpR[0]) + (a1 * tmpR[1]) - (b1 * tmpR[2]) - (b2 * tmpR[3]);

    tmpL[1] = tmpL[0];
    tmpL[0] = inl;
    tmpL[3] = tmpL[2];
    tmpL[2] = l;

    tmpR[1] = tmpR[0];
    tmpR[0] = inr;
    tmpR[3] = tmpR[2];
    tmpR[2] = r;
}


//
// AudioFilter (filter pipeline)
//


const double AudioFilter::pi = std::atan(1) * 4.0;

void
AudioFilter::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {

        os << tab("Filter type");
        os << FilterTypeEnum::key(config.filterType) << std::endl;
        os << tab("Filter activation");
        os << FilterActivationEnum::key(config.filterActivation) << std::endl;
    }

    if (category == Category::State) {

        os << "TODO" << std::endl;
    }
}

void
AudioFilter::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.defaults;

    std::vector <Option> options = {

        OPT_FILTER_TYPE,
        OPT_FILTER_ACTIVATION
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }
}

i64
AudioFilter::getConfigItem(Option option) const
{
    switch (option) {

        case OPT_FILTER_TYPE:       return config.filterType;
        case OPT_FILTER_ACTIVATION: return config.filterActivation;

        default:
            fatalError;
    }
}

void
AudioFilter::setConfigItem(Option option, i64 value)
{
    switch (option) {

        case OPT_FILTER_TYPE:

            if (!FilterTypeEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, FilterTypeEnum::keyList());
            }

            config.filterType = (FilterType)value;
            setupCoefficients(host.getSampleRate());
            return;

        case OPT_FILTER_ACTIVATION:

            if (!FilterActivationEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, FilterActivationEnum::keyList());
            }

            config.filterActivation = (FilterActivation)value;
            return;

        default:
            fatalError;
    }
}

void
AudioFilter::setupCoefficients(double sampleRate)
{
    trace(AUD_DEBUG, "Setting sample rate to %f Hz\n", sampleRate);

    setupLoFilter(sampleRate);
    setupLedFilter(sampleRate);
    setupHiFilter(sampleRate);

    // Setup the legacy filter (DEPRECATED)
    butterworthL.setSampleRate(sampleRate);
    butterworthR.setSampleRate(sampleRate);
}

void
AudioFilter::setupLoFilter(double sampleRate)
{
    double R, C, cutoff;

    R = 360.0;                                      // R321 (360 ohm)
    C = 1e-7;                                       // C321 (0.1uF)
    cutoff = 1.0 / (2 * AudioFilter::pi * R * C);   // ~4420.971Hz
    printf("Low-pass filter: cutoff = %f\n", cutoff);

    loFilter.clear();
    loFilter.setup(sampleRate, cutoff);
}

void
AudioFilter::setupLedFilter(double sampleRate)
{
    double R1 = 10000.0; // R322 (10K ohm)
    double R2 = 10000.0; // R323 (10K ohm)
    double C1 = 6.8e-9;  // C322 (6800pF)
    double C2 = 3.9e-9;  // C323 (3900pF)
    double cutoff = 1.0 / (2 * AudioFilter::pi * std::sqrt(R1 * R2 * C1 * C2)); // ~3090.533Hz
    double qfactor = std::sqrt(R1 * R2 * C1 * C2) / (C2 * (R1 + R2));           // ~0.660225

    printf("LED filter: cutoff = %f qFactor = %f\n", cutoff, qfactor);

    ledFilter.clear();
    ledFilter.setup(sampleRate, cutoff, qfactor);
}
void
AudioFilter::setupHiFilter(double sampleRate)
{
    double R, C, cutoff;

    if (config.filterType == FILTER_A1200) {

        R = 1360.0;     // R324 (1K ohm resistor) + R325 (360 ohm resistor)
        C = 2.2e-5;     // C334 (22uF capacitor)

    } else {

        R = 1390.0;     // R324 (1K ohm) + R325 (390 ohm)
        C = 2.233e-5;   // C334 (22uF) + C335 (0.33uF)
    }

    cutoff = 1.0 / (2 * AudioFilter::pi * R * C);
    printf("High-pass filter: cutoff = %f\n", cutoff);

    hiFilter.clear();
    hiFilter.setup(sampleRate, cutoff);
}

bool
AudioFilter::isEnabled() const
{
    if (config.filterType == FILTER_NONE) return false;
    
    switch (config.filterActivation) {

        case FILTER_AUTO_ENABLE:    return ciaa.powerLED();
        case FILTER_ALWAYS_ON:      return true;
        case FILTER_ALWAYS_OFF:     return false;

        default:
            fatalError;
    }
}

bool
AudioFilter::loFilterEnabled() const
{
    switch (config.filterType) {

        case FILTER_A500:
        case FILTER_LP:
        case FILTER_LP_HP:
        case FILTER_LP_LED_HP:  return true;
        default:                return false;
    }
}

bool
AudioFilter::ledFilterEnabled() const
{
    switch (config.filterType) {

        case FILTER_A500:
        case FILTER_A1200:      return ciaa.powerLED();
        case FILTER_LED:        
        case FILTER_LP_LED_HP:  return true;
        default:                return false;
    }
}

bool
AudioFilter::hiFilterEnabled() const
{
    switch (config.filterType) {

        case FILTER_A500:
        case FILTER_A1200:
        case FILTER_HP:
        case FILTER_LP_HP:
        case FILTER_LP_LED_HP:  return true;
        default:                return false;
    }
}

void
AudioFilter::clear()
{
    butterworthL.clear();
    butterworthR.clear();
}

/*
float
AudioFilter::apply(float sample)
{
    if (config.filterType == FILTER_NONE) return sample;
    
    // Apply butterworth filter
    assert(config.filterType == FILTER_BUTTERWORTH);
    
    // Run pipeline
    double x0 = (double)sample;
    double y0 = (b0 * x0) + (b1 * x1) + (b2 * x2) + (a1 * y1) + (a2 * y2);
    
    // Shift pipeline
    x2 = x1; x1 = x0;
    y2 = y1; y1 = y0;
    
    return (float)y0;
}
*/

}
