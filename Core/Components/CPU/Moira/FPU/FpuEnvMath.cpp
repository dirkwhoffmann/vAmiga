// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "FpuEnv.h"
#include "Wide.h"
#include <algorithm>
#include <bit>
#include <cmath>

namespace MOIRA_NAMESPACE {

namespace {


//
// Wide floating-point numbers
//

/* A floating-point number with a 128-bit significand and an exponent that is
 * not limited in practice. The value is (-1)^neg * m * 2^(e - 127). The
 * significand is normalized (bit 127 is set) unless the number is zero.
 *
 * The arithmetic truncates and is accurate to about 2^-126. All transcendental
 * functions are evaluated with this type and rounded to MFloat80 at the end.
 */
struct W {

    bool neg = false;
    i32 e = 0;
    u128 m = 0;

    bool zero() const { return m == 0; }
};

// Creates a number of value m * 2^(e - 127)
W norm(bool neg, i32 e, u128 m)
{
    W w;
    w.neg = neg;
    if (m) {
        int s = clz128(m);
        w.m = m << s;
        w.e = e - s;
    }
    return w;
}

W fromU64(u64 v) { return norm(false, 127, v); }
W fromI64(i64 v) { return norm(v < 0, 127, v < 0 ? u64(-v) : u64(v)); }
W fromMFloat80(const MFloat80 &x) { return norm(x.sign(), i32(x.biasedExponent()) - MFloat80::bias, u128(x.mantissa()) << 64); }

W fromDouble(double d)
{
    int k;
    double f = std::frexp(std::fabs(d), &k);
    return norm(d < 0, k + 63, u64(std::ldexp(f, 64)));
}

double toDouble(const W &w)
{
    return (w.neg ? -1.0 : 1.0) * std::ldexp(double(u64(w.m >> 64)), w.e - 63);
}

W operator-(W a) { a.neg = !a.neg; return a; }

W operator*(const W &a, const W &b)
{
    if (a.zero() || b.zero()) { W z; z.neg = a.neg != b.neg; return z; }
    return norm(a.neg != b.neg, a.e + b.e + 1, mulHigh128(a.m, b.m));
}

W operator+(W a, W b)
{
    if (a.zero()) return b;
    if (b.zero()) return a;
    if (a.e < b.e || (a.e == b.e && a.m < b.m)) std::swap(a, b);

    i32 d = a.e - b.e;
    u128 am = a.m >> 1;
    u128 bm = d >= 127 ? 0 : (b.m >> 1) >> d;
    u128 r = a.neg == b.neg ? am + bm : am - bm;
    return norm(a.neg, a.e + 1, r);
}

W operator-(const W &a, const W &b) { return a + (-b); }

W operator/(const W &a, const W &b)
{
    if (a.zero()) return a;

    bool ge = a.m >= b.m;
    u128 r = ge ? a.m - b.m : a.m;
    u128 q = ge ? 1 : 0;

    for (int i = 0; i < (ge ? 127 : 128); i++) {

        bool carry = (r >> 127) != 0;
        r <<= 1;
        q <<= 1;
        if (carry || r >= b.m) { r -= b.m; q |= 1; }
    }
    W w;
    w.neg = a.neg != b.neg;
    w.m = q;
    w.e = a.e - b.e - (ge ? 0 : 1);
    return w;
}

// Compares the magnitudes of two numbers
int cmpAbs(const W &a, const W &b)
{
    if (a.zero() || b.zero()) return a.zero() == b.zero() ? 0 : (a.zero() ? -1 : 1);
    if (a.e != b.e) return a.e < b.e ? -1 : 1;
    return a.m == b.m ? 0 : (a.m < b.m ? -1 : 1);
}

// Multiplies by 2^n
W scale(W a, i32 n) { if (!a.zero()) a.e += n; return a; }

W abs(W a) { a.neg = false; return a; }

W sqrt(const W &a)
{
    if (a.zero()) return a;

    // Write a = f * 4^k with f in [1,4)
    i32 k = a.e >> 1;
    W f = a;
    f.e = a.e & 1;

    // Newton iteration
    W x = fromDouble(std::sqrt(toDouble(f)));
    W half = scale(fromU64(1), -1);
    for (int i = 0; i < 4; i++) x = (x + f / x) * half;

    return scale(x, k);
}

const W &one() { static const W w = fromU64(1); return w; }
const W &two() { static const W w = fromU64(2); return w; }


//
// Series
//

// Computes atanh(z) = z + z^3/3 + z^5/5 + ... for small z
W atanhSeries(const W &z)
{
    if (z.zero()) return z;

    W z2 = z * z, term = z, sum = z;
    for (u64 k = 1; k < 200; k++) {

        term = term * z2;
        if (term.zero() || term.e < sum.e - 135) break;
        sum = sum + term / fromU64(2 * k + 1);
    }
    return sum;
}

// Computes atan(z) = z - z^3/3 + z^5/5 - ... for small z
W atanSeries(const W &z)
{
    if (z.zero()) return z;

    W z2 = z * z, term = z, sum = z;
    for (u64 k = 1; k < 200; k++) {

        term = term * z2;
        if (term.zero() || term.e < sum.e - 135) break;
        W t = term / fromU64(2 * k + 1);
        sum = (k & 1) ? sum - t : sum + t;
    }
    return sum;
}

// Computes sin(r) and cos(r) for |r| <= pi/4
void sinCosSeries(const W &r, W &s, W &c)
{
    W r2 = r * r;

    // Sine
    W term = r, sum = r;
    for (u64 k = 1; k < 100 && !r.zero(); k++) {

        term = -(term * r2) / fromU64((2 * k) * (2 * k + 1));
        if (term.zero() || term.e < sum.e - 135) break;
        sum = sum + term;
    }
    s = sum;

    // Cosine
    term = one(); sum = one();
    for (u64 k = 1; k < 100; k++) {

        term = -(term * r2) / fromU64((2 * k - 1) * (2 * k));
        if (term.zero() || term.e < -135) break;
        sum = sum + term;
    }
    c = sum;
}


//
// Constants
//

const W &ln2()
{
    static const W w = scale(atanhSeries(one() / fromU64(3)), 1);
    return w;
}

const W &ln10()
{
    // ln(10) = 3 ln(2) + ln(5/4)
    static const W w = fromU64(3) * ln2() + scale(atanhSeries(one() / fromU64(9)), 1);
    return w;
}

const W &invLn2() { static const W w = one() / ln2(); return w; }

const W &pi()
{
    // pi = 3.1415926... (the 128 most significant bits)
    static const W w = norm(false, 1, (u128(0xC90FDAA22168C234) << 64) | 0xC4C6628B80DC1CD1);
    return w;
}

const W &piOver2() { static const W w = scale(pi(), -1); return w; }

// The first 16896 fractional bits of 2/pi (most significant word first)
constexpr int twoOverPiWords = 528;
constexpr u32 twoOverPi[twoOverPiWords] = {

    0xA2F9836E, 0x4E441529, 0xFC2757D1, 0xF534DDC0, 0xDB629599, 0x3C439041,
    0xFE5163AB, 0xDEBBC561, 0xB7246E3A, 0x424DD2E0, 0x06492EEA, 0x09D1921C,
    0xFE1DEB1C, 0xB129A73E, 0xE88235F5, 0x2EBB4484, 0xE99C7026, 0xB45F7E41,
    0x3991D639, 0x835339F4, 0x9C845F8B, 0xBDF9283B, 0x1FF897FF, 0xDE05980F,
    0xEF2F118B, 0x5A0A6D1F, 0x6D367ECF, 0x27CB09B7, 0x4F463F66, 0x9E5FEA2D,
    0x7527BAC7, 0xEBE5F17B, 0x3D0739F7, 0x8A5292EA, 0x6BFB5FB1, 0x1F8D5D08,
    0x56033046, 0xFC7B6BAB, 0xF0CFBC20, 0x9AF4361D, 0xA9E39161, 0x5EE61B08,
    0x6599855F, 0x14A06840, 0x8DFFD880, 0x4D732731, 0x06061556, 0xCA73A8C9,
    0x60E27BC0, 0x8C6B47C4, 0x19C367CD, 0xDCE8092A, 0x8359C476, 0x8B961CA6,
    0xDDAF44D1, 0x5719053E, 0xA5FF0705, 0x3F7E33E8, 0x32C2DE4F, 0x98327DBB,
    0xC33D26EF, 0x6B1E5EF8, 0x9F3A1F35, 0xCAF27F1D, 0x87F12190, 0x7C7C246A,
    0xFA6ED577, 0x2D30433B, 0x15C614B5, 0x9D19C3C2, 0xC4AD414D, 0x2C5D000C,
    0x467D862D, 0x71E39AC6, 0x9B006233, 0x7CD2B497, 0xA7B4D555, 0x37F63ED7,
    0x1810A3FC, 0x764D2A9D, 0x64ABD770, 0xF87C6357, 0xB07AE715, 0x175649C0,
    0xD9D63B38, 0x84A7CB23, 0x24778AD6, 0x23545AB9, 0x1F001B0A, 0xF1DFCE19,
    0xFF319F6A, 0x1E666157, 0x9947FBAC, 0xD87F7EB7, 0x652289E8, 0x3260BFE6,
    0xCDC4EF09, 0x366CD43F, 0x5DD7DE16, 0xDE3B5892, 0x9BDE2822, 0xD2E88628,
    0x4D58E232, 0xCAC616E3, 0x08CB7DE0, 0x50C017A7, 0x1DF35BE0, 0x1834132E,
    0x62128301, 0x48835B8E, 0xF57FB0AD, 0xF2E91E43, 0x4A48D367, 0x10D8DDAA,
    0x425FAECE, 0x616AA428, 0x0AB499D3, 0xF2A6067F, 0x775C83C2, 0xA3883C61,
    0x78738A5A, 0x8CAFBDD7, 0x6F63A62D, 0xCBBFF4EF, 0x818D67C1, 0x2645CA55,
    0x36D9CAD2, 0xA8288D61, 0xC277C912, 0x1426049B, 0x4612C459, 0xC444C5C8,
    0x91B24DF3, 0x1700AD43, 0xD4E54929, 0x10D5FDFC, 0xBE00CC94, 0x1EEECE70,
    0xF53E1380, 0xF1ECC3E7, 0xB328F8C7, 0x9405933E, 0x71C1B309, 0x2EF3450B,
    0x9C12887B, 0x20AB9FB5, 0x2EC29247, 0x2F327B6D, 0x550C90A7, 0x721FE76B,
    0x96CB314A, 0x1679E279, 0x4189DFF4, 0x9794E884, 0xE6E29731, 0x996BED88,
    0x365F5F0E, 0xFDBBB49A, 0x486CA467, 0x42727132, 0x5D8DB815, 0x9F09E5BC,
    0x25318D39, 0x74F71C05, 0x30010C0D, 0x68084B58, 0xEE2C90AA, 0x4702E774,
    0x24D6BDA6, 0x7DF77248, 0x6EEF169F, 0xA6948EF6, 0x91B45153, 0xD1F20ACF,
    0x3398207E, 0x4BF56863, 0xB25F3EDD, 0x035D407F, 0x89852952, 0x55C06437,
    0x10D86D32, 0x4832754C, 0x5BD4714E, 0x6E5445C1, 0x090B69F5, 0x2AD56614,
    0x9D072750, 0x045DDB3B, 0xB4C576EA, 0x17F9877D, 0x6B49BA27, 0x1D296996,
    0xACCCC654, 0x14AD6AE2, 0x9089D988, 0x50722CBE, 0xA4049407, 0x777030F3,
    0x27FC00A8, 0x71EA49C2, 0x663DE064, 0x83DD9797, 0x3FA3FD94, 0x438C860D,
    0xDE41319D, 0x39928C70, 0xDDE7B717, 0x3BDF082B, 0x3715A080, 0x5C93805A,
    0x921110D8, 0xE80FAF80, 0x6C4BFFDB, 0x0F903876, 0x185915A5, 0x62BBCB61,
    0xB989C7BD, 0x401004F2, 0xD2277549, 0xF6B6EBBB, 0x22DBAA14, 0x0A2F2689,
    0x76836433, 0x3B091A94, 0x0EAA3A51, 0xC2A31DAE, 0xEDAF1226, 0x5C4DC26D,
    0x9C7A2D97, 0x56C0833F, 0x03F6F009, 0x8C402B99, 0x316D07B4, 0x3915200C,
    0x5BC3D8C4, 0x92F54BAD, 0xC6A5CA4E, 0xCD37A736, 0xA9E69492, 0xAB6842DD,
    0xDE6319EF, 0x8C76528B, 0x6837DBFC, 0xABA1AE31, 0x15DFA1AE, 0x00DAFB0C,
    0x664D64B7, 0x05ED3065, 0x29BF5657, 0x3AFF47B9, 0xF96AF3BE, 0x75DF9328,
    0x3080ABF6, 0x8C6615CB, 0x040622FA, 0x1DE4D9A4, 0xB33D8F1B, 0x5709CD36,
    0xE9424EA4, 0xBE13B523, 0x331AAAF0, 0xA8654FA5, 0xC1D20F3F, 0x0BCD785B,
    0x76F92304, 0x8B7B7217, 0x8953A6C6, 0xE26E6F00, 0xEBEF584A, 0x9BB7DAC4,
    0xBA66AACF, 0xCF761D02, 0xD12DF1B1, 0xC1998C77, 0xADC3DA48, 0x86A05DF7,
    0xF480C62F, 0xF0AC9AEC, 0xDDBC5C3F, 0x6DDED01F, 0xC790B6DB, 0x2A3A25A3,
    0x9AAF0093, 0x53AD0457, 0xB6B42D29, 0x7E804BA7, 0x07DA0EAA, 0x76A1597B,
    0x2A12162D, 0xB7DCFDE5, 0xFAFEDB89, 0xFDBE896C, 0x76E4FCA9, 0x0670803E,
    0x156E85FF, 0x87FD073E, 0x28336761, 0x86182AEA, 0xBD4DAFE7, 0xB36E6D8F,
    0x3967955B, 0xBF3148D7, 0x8416DF30, 0x432DC735, 0x6125CE70, 0xC9B8CB30,
    0xFD6CBFA2, 0x00A4E46C, 0x05A0DD5A, 0x476F21D2, 0x1262845C, 0xB9496170,
    0xE0566B01, 0x52993755, 0x50B7D51E, 0xC4F1335F, 0x6E13E430, 0x5DA92E85,
    0xC3B21D36, 0x32A1A4B7, 0x08D4B1EA, 0x21F716E4, 0x698F77FF, 0x2780030C,
    0x2D408DA0, 0xCD4F99A5, 0x20D3A2B3, 0x0A5D2F42, 0xF9B4CBDA, 0x11D0BE7D,
    0xC1DB9BBD, 0x17AB81A2, 0xCA5C6A08, 0x17552E55, 0x0027F014, 0x7F8607E1,
    0x640B148D, 0x4196DEBE, 0x872AFDDA, 0xB6256B34, 0x897BFEF3, 0x059EBFB9,
    0x4F6A68A8, 0x2A4A5AC4, 0x4FBCF82D, 0x985AD795, 0xC7F48D4D, 0x0DA63A20,
    0x5F57A4B1, 0x3F149538, 0x800120CC, 0x86DD71B6, 0xDEC9F560, 0xBF11654D,
    0x6B0701AC, 0xB08CD0C0, 0xB2485551, 0x0EFB1EC3, 0x72953B06, 0xA33540C0,
    0x7BDC06CC, 0x45E0FA29, 0x4EC8CAD6, 0x41F3E8DE, 0x647CD864, 0x9B31BED9,
    0xC397A4D4, 0x5877C5E3, 0x6913DAF0, 0x3C3ABA46, 0x18465F75, 0x55F5BDD2,
    0xC6926E5D, 0x2EACED44, 0x0E423E1C, 0x87C461E9, 0xFD29F3D6, 0xE7CA7C22,
    0x35916FC5, 0xE0088DD7, 0xFFE26A6E, 0xC6FDB0C1, 0x0893745D, 0x7CB2AD6B,
    0x9D6ECD7B, 0x723E6A11, 0xC6A9CFF7, 0xDF7329BA, 0xC9B55100, 0xB70DB2E2,
    0x24BA7460, 0x7DE58AD8, 0x742C150D, 0x0C188194, 0x667E1629, 0x01767A9F,
    0xBEFDFDEF, 0x4556367E, 0xD913D9EC, 0xB9BA8BFC, 0x97C427A8, 0x31C36EF1,
    0x36C59456, 0xA8D8B5A8, 0xB40ECCCF, 0x2D891234, 0x576F8956, 0x2CE3CE99,
    0xB920D6AA, 0x5E6B9C2A, 0x3ECC5F11, 0x4A0BFDFB, 0xF4E16D3B, 0x8E2C86E2,
    0x84D4E9A9, 0xB4FCD1EE, 0xEFC9352E, 0x61392F44, 0x2138C8D9, 0x1B0AFC81,
    0x6A4AFBD8, 0x1C2F84B4, 0x538C994E, 0xCC2254DC, 0x552AD6C6, 0xC096190B,
    0xB8701A64, 0x9569605A, 0x26EE523F, 0x0F117F11, 0xB5F4F5CB, 0xFC2DBC34,
    0xEEBC34CC, 0x5DE8605E, 0xDD9B8E67, 0xEF3392B8, 0x17C99B58, 0x61BC57E1,
    0xC6835110, 0x3ED84871, 0xDDDD1C2D, 0xA118AF46, 0x2C21D7F3, 0x59987AD9,
    0xC0549EFA, 0x864FFC06, 0x56AE79E5, 0x36228922, 0xAD38DC93, 0x67AAE855,
    0x3826829B, 0xE7CAA40D, 0x51B13399, 0x0ED7A948, 0x0569F0B2, 0x65A7887F,
    0x974C8836, 0xD1F9B392, 0x214A827B, 0x21CF98DC, 0x9F405547, 0xDC3A74E1,
    0x42EB67DF, 0x9DFE5FD4, 0x5EA4677B, 0x7AACBAA2, 0xF6552388, 0x2B55BA41,
    0x086E5986, 0x2A218347, 0x39E6E389, 0xD49EE540, 0xFB49E956, 0xFFCA0F1C,
    0x8A59C52B, 0xFA94C5C1, 0xD3CFC50F, 0xAE5ADB86, 0xC5476243, 0x853B8621,
    0x94792C87, 0x61107B4C, 0x2A1A2C80, 0x12BF4390, 0x2688893C, 0x78E4C4A8,
    0x7BDBE5C2, 0x3AC4EAF4, 0x268A67F7, 0xBF920D2B, 0xA365B193, 0x3D0B7CBD,
    0xDC51A463, 0xDD27DDE1, 0x6919949A, 0x9529A828, 0xCE68B4ED, 0x09209F44,
    0xCA984E63, 0x8270237C, 0x7E32B90F, 0x8EF5A7E7, 0x561408F1, 0x212A9DB5,
    0x4D7E6F51, 0x19A5ABF9, 0xB5D6DF82, 0x61DD9602, 0x36169F3A, 0xC4A1A283,
    0x6DED727A, 0x8D39A9B8, 0x825C326B, 0x5B2746ED, 0x34007700, 0xD255F4FC,
    0x4D590180, 0x71E0E13F, 0x89B295F3, 0x64A8F1AE, 0xA74B38FC, 0x4CEAB2BB,
    0x47270BAB, 0xC3A734BA, 0x6052DD34, 0xF8563AEB, 0x7E8A31BB, 0x365895B7,
};

// Returns word 'index' of the table (zero if out of range)
u32 twoOverPiWord(int index)
{
    return index >= 0 && index < twoOverPiWords ? twoOverPi[index] : 0;
}


//
// Argument reduction
//

/* Computes the remainder r of |x| modulo pi/2 with |r| <= pi/4 and the
 * quadrant (the quotient modulo 4). The reduction is exact, i.e., it takes as
 * many bits of pi into account as necessary for the magnitude of x.
 *
 * Let x = m * 2^(e - 127). The product x * 2/pi is only needed modulo 4.
 * Bits of 2/pi with a weight of 2^-i, i <= e - 129, contribute multiples of 4
 * and can be skipped. A window of 16 words (512 bits) starting at the first
 * relevant word leaves far more guard bits than any cancellation can consume.
 */
void reduce(const W &x, int &quadrant, W &r)
{
    quadrant = 0;

    if (x.e < -1) { r = abs(x); return; }

    constexpr int L = 16;               // Window size in words
    constexpr int N = 4 + L;            // Size of the product in words

    // The window holds bits a + 1 ... a + 32 L of 2/pi
    int a = std::max(0, x.e - 129) / 32 * 32;
    // Little-endian limbs of the significand and the window
    u32 t[L], m[4] = { u32(x.m), u32(x.m >> 32), u32(x.m >> 64), u32(x.m >> 96) };
    for (int j = 0; j < L; j++) t[j] = twoOverPiWord(a / 32 + L - 1 - j);

    // P = m * T represents |x| * 2/pi * 2^s
    u32 p[N + 1] = { };
    for (int i = 0; i < 4; i++) {
        u64 carry = 0;
        for (int j = 0; j < L; j++) {
            u64 v = u64(m[i]) * t[j] + p[i + j] + carry;
            p[i + j] = u32(v); carry = v >> 32;
        }
        p[i + L] = u32(carry);
    }
    int s = a + 32 * L - x.e + 127;

    auto bit = [&](int i) { return int(i >= 0 && i / 32 <= N && (p[i / 32] >> (i % 32) & 1)); };
    int n = bit(s) | bit(s + 1) << 1;
    
    // Isolate the fractional part (the lowest s bits)
    for (int i = 0; i <= N; i++) {
        int lo = i * 32;
        if (lo >= s) p[i] = 0;
        else if (lo + 32 > s) p[i] &= (1u << (s - lo)) - 1;
    }

    // Fold into the range [-1/2, 1/2)
    bool negative = false;
    if (bit(s - 1)) {

        u64 carry = 1;
        for (int i = 0; i <= N; i++) {
            u64 v = u64(u32(~p[i])) + carry;
            p[i] = u32(v); carry = v >> 32;
        }
        for (int i = 0; i <= N; i++) {
            int lo = i * 32;
            if (lo >= s) p[i] = 0;
            else if (lo + 32 > s) p[i] &= (1u << (s - lo)) - 1;
        }
        negative = true;
        n = (n + 1) & 3;
    }

    // Convert the fraction into a wide number and multiply by pi/2
    int len = 0;
    for (int i = N; i >= 0 && !len; i--) if (p[i]) len = i * 32 + 32 - std::countl_zero(p[i]);

    u128 mant = 0;
    for (int i = 0; i < 128; i++) mant = mant << 1 | bit(len - 1 - i);

    W frac = norm(negative, len - s - 1, mant);
    r = frac * piOver2();
    quadrant = n;
}


//
// Elementary functions
//

// Computes e^x. The argument is clamped to a range that overflows or underflows anyway.
W expW(W x)
{
    if (x.zero()) return one();
    if (x.e > 20) x = norm(x.neg, 20, u128(1) << 127);

    W t = x * invLn2();
    i64 k = std::llround(toDouble(t));
    W r = x - fromI64(k) * ln2();
    r = scale(r, -6);

    W sum = one(), term = one();
    for (u64 n = 1; n < 100; n++) {

        term = term * r / fromU64(n);
        if (term.zero() || term.e < -140) break;
        sum = sum + term;
    }
    for (int i = 0; i < 6; i++) sum = sum * sum;

    return scale(sum, i32(k));
}

// Computes e^x - 1
W expm1W(W x)
{
    if (x.zero()) return x;

    if (x.e < -1) {

        // Avoid cancellation
        W term = x, sum = x;
        for (u64 n = 2; n < 100; n++) {

            term = term * x / fromU64(n);
            if (term.zero() || term.e < sum.e - 135) break;
            sum = sum + term;
        }
        return sum;
    }
    return expW(x) - one();
}

// Computes ln(x) for x > 0
W lnW(const W &x)
{
    // Write x = f * 2^k with f in (0.7,1.4]
    i32 k = x.e;
    W f = x;
    f.e = 0;
    static const W sqrt2 = sqrt(two());
    if (cmpAbs(f, sqrt2) > 0) { f.e = -1; k++; }

    W z = (f - one()) / (f + one());
    W result = scale(atanhSeries(z), 1);
    if (k) result = result + fromI64(k) * ln2();
    return result;
}

// Computes ln(1 + x) for x > -1
W lognp1W(const W &x)
{
    if (x.zero()) return x;
    if (x.e < -1) return scale(atanhSeries(x / (two() + x)), 1);
    return lnW(one() + x);
}

// Computes atan(x)
W atanW(W x)
{
    if (x.zero()) return x;

    bool negative = x.neg;
    x.neg = false;

    bool invert = cmpAbs(x, one()) > 0;
    if (invert) x = one() / x;

    for (int i = 0; i < 3; i++) x = x / (one() + sqrt(one() + x * x));

    W r = scale(atanSeries(x), 3);
    if (invert) r = piOver2() - r;

    r.neg = negative;
    return r;
}

/* Rounds a wide number to a MFloat80. Like the reference, we report every
 * result of a transcendental function as inexact, even if it is exact (e.g.,
 * 2^n or ln(1)). Special operands that are resolved up front don't get here.
 */
MFloat80 finish(FpuEnv &env, const W &w)
{
    env.raiseFlags(FloatFlags::INEXACT);
    if (w.zero()) return MFloat80::zero(w.neg);
    return env.roundPack(w.neg, w.e + MFloat80::bias, u64(w.m >> 64), u64(w.m));
}

MFloat80 invalid(FpuEnv &env)
{
    env.raiseFlags(FloatFlags::INVALID);
    return MFloat80::defaultNaN();
}

MFloat80 divideByZero(FpuEnv &env, bool negative)
{
    env.raiseFlags(FloatFlags::DIV_BY_ZERO);
    return MFloat80::infinity(negative);
}

// Returns pi/2 or pi/4 etc. with a sign
MFloat80 signed_(FpuEnv &env, const W &w, bool negative) { W r = w; r.neg = negative; return finish(env, r); }

}


//
// Trigonometric functions
//

static void
sincos_(FpuEnv &env, const MFloat80 &x, MFloat80 &sine, MFloat80 &cosine)
{
    if (x.isNaN()) { sine = cosine = x; return; }
    if (x.isInfinite()) { sine = cosine = invalid(env); return; }
    if (x.isZero()) { sine = x; cosine = MFloat80::one(); return; }

    int n;
    W r, s, c;
    reduce(fromMFloat80(x), n, r);
    sinCosSeries(r, s, c);

    W sv = (n & 1) ? c : s;
    W cv = (n & 1) ? -s : c;
    if (n & 2) { sv = -sv; cv = -cv; }
    if (x.sign()) sv = -sv;

    sine = finish(env, sv);
    cosine = finish(env, cv);
}

void
FpuEnv::sincos(const MFloat80 &x, MFloat80 &sine, MFloat80 &cosine)
{
    sincos_(*this, x, sine, cosine);
}

namespace {

// Computes sin and cos as wide numbers (the argument must be finite and nonzero)
void sincosW(const MFloat80 &x, W &sv, W &cv)
{
    int n;
    W r, s, c;
    reduce(fromMFloat80(x), n, r);
    sinCosSeries(r, s, c);

    sv = (n & 1) ? c : s;
    cv = (n & 1) ? -s : c;
    if (n & 2) { sv = -sv; cv = -cv; }
    if (x.sign()) sv = -sv;
}

}

static MFloat80
sin_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN() || x.isZero()) return x;
    if (x.isInfinite()) return invalid(env);

    W sv, cv;
    sincosW(x, sv, cv);
    return finish(env, sv);
}

MFloat80
FpuEnv::sin(const MFloat80 &x)
{
    return sin_(*this, x);
}

static MFloat80
cos_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN()) return x;
    if (x.isInfinite()) return invalid(env);
    if (x.isZero()) return MFloat80::one();

    W sv, cv;
    sincosW(x, sv, cv);
    return finish(env, cv);
}

MFloat80
FpuEnv::cos(const MFloat80 &x)
{
    return cos_(*this, x);
}

static MFloat80
tan_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN()) return x;
    if (x.isInfinite()) return invalid(env);
    if (x.isZero()) return x;

    int n;
    W r, s, c;
    reduce(fromMFloat80(x), n, r);
    sinCosSeries(r, s, c);

    W t = (n & 1) ? -(c / s) : s / c;
    if (x.sign()) t = -t;

    return finish(env, t);
}

MFloat80
FpuEnv::tan(const MFloat80 &x)
{
    return tan_(*this, x);
}


//
// Inverse trigonometric functions
//

static MFloat80
atan_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN() || x.isZero()) return x;
    if (x.isInfinite()) return signed_(env, piOver2(), x.sign());

    return finish(env, atanW(fromMFloat80(x)));
}

MFloat80
FpuEnv::atan(const MFloat80 &x)
{
    return atan_(*this, x);
}

static MFloat80
asin_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN() || x.isZero()) return x;
    if (x.isInfinite()) return invalid(env);

    W v = fromMFloat80(x);
    int c = cmpAbs(v, one());
    if (c > 0) return invalid(env);
    if (c == 0) return signed_(env, piOver2(), x.sign());

    // asin(x) = atan(x / sqrt((1 - x)(1 + x)))
    W root = sqrt((one() - abs(v)) * (one() + abs(v)));
    W r = atanW(abs(v) / root);
    r.neg = x.sign();
    return finish(env, r);
}

MFloat80
FpuEnv::asin(const MFloat80 &x)
{
    return asin_(*this, x);
}

static MFloat80
acos_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN()) return x;
    if (x.isInfinite()) return invalid(env);

    W v = fromMFloat80(x);
    int c = cmpAbs(v, one());
    if (c > 0) return invalid(env);
    if (c == 0) return x.sign() ? finish(env, pi()) : MFloat80::zero();

    // acos(x) = 2 atan(sqrt((1 - x) / (1 + x)))
    W r = scale(atanW(sqrt((one() - v) / (one() + v))), 1);
    return finish(env, r);
}

MFloat80
FpuEnv::acos(const MFloat80 &x)
{
    return acos_(*this, x);
}


//
// Hyperbolic functions
//

static MFloat80
sinh_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN() || x.isZero() || x.isInfinite()) return x;

    // sinh(x) = (E + E / (E + 1)) / 2, where E = e^|x| - 1
    W v = abs(fromMFloat80(x));
    W r;

    if (cmpAbs(v, scale(one(), -32)) < 0) {

        // For tiny arguments, sinh(x) = x + x^3 / 6 is slightly larger than x. Don't compute it via
        // expm1, because rounding errors could push the result below x and signal a false underflow.
        r = v + scale(v, -65);

    } else {

        W e = expm1W(v);
        r = scale(e + e / (e + one()), -1);
    }
    r.neg = x.sign();
    return finish(env, r);
}

MFloat80
FpuEnv::sinh(const MFloat80 &x)
{
    return sinh_(*this, x);
}

static MFloat80
cosh_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN()) return x;
    if (x.isInfinite()) return MFloat80::infinity();
    if (x.isZero()) return MFloat80::one();

    W e = expW(abs(fromMFloat80(x)));
    return finish(env, scale(e + one() / e, -1));
}

MFloat80
FpuEnv::cosh(const MFloat80 &x)
{
    return cosh_(*this, x);
}

static MFloat80
tanh_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN() || x.isZero()) return x;
    if (x.isInfinite()) return MFloat80::one(x.sign());

    // tanh(x) = E / (E + 2), where E = e^(2|x|) - 1
    W e = expm1W(scale(abs(fromMFloat80(x)), 1));
    W r = e / (e + two());
    r.neg = x.sign();

    // For large arguments, the quotient rounds to 1, but the exact result is slightly smaller.
    // Use the closest value below 1 to let the rounding mode take effect.
    if (r.e == one().e && r.m == one().m) { r.e -= 1; r.m = ~u128(0); }

    return finish(env, r);
}

MFloat80
FpuEnv::tanh(const MFloat80 &x)
{
    return tanh_(*this, x);
}

static MFloat80
atanh_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN() || x.isZero()) return x;
    if (x.isInfinite()) return invalid(env);

    W v = abs(fromMFloat80(x));
    int c = cmpAbs(v, one());
    if (c > 0) return invalid(env);
    if (c == 0) return divideByZero(env, x.sign());

    // atanh(x) = ln(1 + 2x / (1 - x)) / 2
    W r = scale(lognp1W(scale(v, 1) / (one() - v)), -1);
    r.neg = x.sign();
    return finish(env, r);
}

MFloat80
FpuEnv::atanh(const MFloat80 &x)
{
    return atanh_(*this, x);
}


//
// Exponential functions
//

static MFloat80
etox_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN()) return x;
    if (x.isInfinite()) return x.sign() ? MFloat80::zero() : x;
    if (x.isZero()) return MFloat80::one();

    return finish(env, expW(fromMFloat80(x)));
}

MFloat80
FpuEnv::etox(const MFloat80 &x)
{
    return etox_(*this, x);
}

static MFloat80
etoxm1_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN() || x.isZero()) return x;
    if (x.isInfinite()) {
        return x.sign() ? MFloat80::one(true) : x;
    }
    /* For tiny arguments, e^x - 1 equals x within the precision of the extended format. What
     * remains to be done is to raise the right flags and to round the result as requested.
     * The reference behavior is to add the smallest normal number with a negative sign (using
     * the precision and rounding mode of the instruction). This lets the result land just
     * below x in directed rounding modes and makes it tiny if x is a power of two at the lower
     * end of the destination range. The addend is only applied if it doesn't dominate x (if the
     * exponent field is at least 0x33). Otherwise, x is simply rounded to the destination format.
     * Note: The threshold and the addend can't be derived from the math. They reproduce the
     * observable behavior of the reference.
     */
    if ((x.signExp() & 0x7FFF) < MFloat80::bias - 65) {

        if ((x.signExp() & 0x7FFF) < 0x33) return finish(env, fromMFloat80(x));

        auto result = env.add(x, MFloat80::fromBits(0x8001, 1ULL << 63));
        env.raiseFlags(FloatFlags::INEXACT);
        return result;
    }

    W v = fromMFloat80(x);

    // Large negative arguments yield -1 + epsilon
    if (v.neg && v.e > 6) return finish(env, norm(true, -1, ~u128(0)));

    return finish(env, expm1W(v));
}

MFloat80
FpuEnv::etoxm1(const MFloat80 &x)
{
    return etoxm1_(*this, x);
}

static MFloat80
twotox_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN()) return x;
    if (x.isInfinite()) return x.sign() ? MFloat80::zero() : x;
    if (x.isZero()) return MFloat80::one();

    W v = fromMFloat80(x);

    // Integer exponents yield an exact result
    if (x.exponent() >= 0 && x.exponent() < 20 && (x.mantissa() << (x.exponent() + 1)) == 0) {

        i64 n = i64(x.mantissa() >> (63 - x.exponent()));
        return finish(env, norm(false, x.sign() ? -i32(n) : i32(n), u128(1) << 127));
    }
    if (v.e > 20) v = norm(v.neg, 20, u128(1) << 127);
    return finish(env, expW(v * ln2()));
}

MFloat80
FpuEnv::twotox(const MFloat80 &x)
{
    return twotox_(*this, x);
}

static MFloat80
tentox_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN()) return x;
    if (x.isInfinite()) return x.sign() ? MFloat80::zero() : x;
    if (x.isZero()) return MFloat80::one();

    W v = fromMFloat80(x);

    // 10^n is exact for 0 <= n <= 27
    if (!x.sign() && x.exponent() >= 0 && x.exponent() < 5 && (x.mantissa() << (x.exponent() + 1)) == 0) {

        i64 n = i64(x.mantissa() >> (63 - x.exponent()));
        if (n <= 27) {
            u128 p = 1;
            for (i64 i = 0; i < n; i++) p *= 10;
            return finish(env, norm(false, 127, p));
        }
    }
    if (v.e > 20) v = norm(v.neg, 20, u128(1) << 127);
    return finish(env, expW(v * ln10()));
}

MFloat80
FpuEnv::tentox(const MFloat80 &x)
{
    return tentox_(*this, x);
}


//
// Logarithms
//

namespace {

enum class Base { E, TWO, TEN };

MFloat80 logarithm(FpuEnv &env, const MFloat80 &x, Base base)
{
    if (x.isNaN()) return x;
    if (x.isZero()) return divideByZero(env, true);
    if (x.sign()) return invalid(env);
    if (x.isInfinite()) return x;

    W v = fromMFloat80(x);

    // Exact results
    if (base == Base::TWO && (x.mantissa() << 1) == 0) {
        env.raiseFlags(FloatFlags::INEXACT);
        return MFloat80::fromInt(x.exponent());
    }
    if (base == Base::TEN) {
        u128 p = 1;
        for (int n = 0; n <= 27; n++, p *= 10) {
            W t = norm(false, 127, p);
            if (v.e == t.e && v.m == t.m) { env.raiseFlags(FloatFlags::INEXACT); return MFloat80::fromInt(n); }
        }
    }
    W r = lnW(v);
    if (base == Base::TWO) r = r * invLn2();
    if (base == Base::TEN) r = r / ln10();
    return finish(env, r);
}

}

MFloat80 FpuEnv::logn(const MFloat80 &x) { return logarithm(*this, x, Base::E); }
MFloat80 FpuEnv::log2(const MFloat80 &x) { return logarithm(*this, x, Base::TWO); }
MFloat80 FpuEnv::log10(const MFloat80 &x) { return logarithm(*this, x, Base::TEN); }

static MFloat80
lognp1_(FpuEnv &env, const MFloat80 &x)
{
    if (x.isNaN() || x.isZero()) return x;
    if (x.isInfinite()) return x.sign() ? invalid(env) : x;

    W v = fromMFloat80(x);
    if (v.neg) {
        int c = cmpAbs(v, one());
        if (c > 0) return invalid(env);
        if (c == 0) return divideByZero(env, true);
    }
    return finish(env, lognp1W(v));
}

MFloat80
FpuEnv::lognp1(const MFloat80 &x)
{
    return lognp1_(*this, x);
}

}
