// -----------------------------------------------------------------------------
// This file is part of utlib - A lightweight utility library
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "utl/support/Bytes.h"
#include "utl/support/Streams.h"

namespace utl::support {

std::ostream &
dec::operator()(std::ostream &os) const
{
    os << std::dec << value;
    return os;
};

std::ostream &
hex::operator()(std::ostream &os) const
{
    os << std::hex << "0x" << std::setw(int(digits)) << std::setfill('0') << value;
    return os;
};

std::ostream &
bin::operator()(std::ostream &os) const
{
    os << "%";

    for (isize i = 7; i >= 0; i--) {

        if ((digits == 64 && i < 8) ||
            (digits == 32 && i < 4) ||
            (digits == 16 && i < 2) ||
            (digits == 8  && i < 1)  ) {

            std::bitset<8> x(GET_BYTE(value, i));
            os << x << (i ? "." : "");
        }
    }
    return os;
};

std::ostream &
flt::operator()(std::ostream &os) const
{
    os << value;
    return os;
};

std::ostream &
tab::operator()(std::ostream &os) const {
    os << std::setw(int(pads)) << std::right << std::setfill(' ') << str;
    os << (str.empty() ? "   " : " : ");
    return os;
}

std::ostream &
bol::operator()(std::ostream &os) const {
    os << (value ? str1 : str2);
    return os;
}

std::ostream &
str::operator()(std::ostream &os) const
{
    auto c = [&](isize pos) {

        auto byte = GET_BYTE(value, pos);
        return std::isprint(byte) ? string{(char)byte} : string{'.'};
    };

    if (characters >= 8) os << c(7) << c(6) << c(5) << c(4);
    if (characters >= 4) os << c(3) << c(2);
    if (characters >= 2) os << c(1);
    if (characters >= 1) os << c(0);

    return os;
};

const string &bol::yes = "yes";
const string &bol::no = "no";

}
