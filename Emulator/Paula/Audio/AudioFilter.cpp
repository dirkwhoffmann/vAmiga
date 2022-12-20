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

    if (category == Category::Inspection) {

        os << tab("Active");
        os << bol(isEnabled()) << std::endl;
    }
    
    if (category == Category::Debug) {

        os << tab("Coefficient a1");
        os << flt(a1) << std::endl;
        os << tab("Coefficient a2");
        os << flt(a2) << std::endl;
        os << tab("Coefficient b0");
        os << flt(b0) << std::endl;
        os << tab("Coefficient b1");
        os << flt(b1) << std::endl;
        os << tab("Coefficient b2");
        os << flt(b2) << std::endl;
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
AudioFilter::setSampleRate(double sampleRate)
{
    trace(AUD_DEBUG, "Setting sample rate to %f Hz\n", sampleRate);
    
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

bool
AudioFilter::isEnabled() const
{
    switch (config.filterActivation) {

        case FILTER_AUTO_ENABLE:    return ciaa.powerLED();
        case FILTER_ALWAYS_ON:      return true;
        case FILTER_ALWAYS_OFF:     return false;

        default:
            fatalError;
    }
}

void
AudioFilter::clear()
{
    x1 = x2 = y1 = y2 = 0.0;
}

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

}
