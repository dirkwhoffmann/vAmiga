// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

// Accessors for the status register, the cache registers and the supervisor
// state. This file is included by the main translation unit (Moira.cpp) only.

u8
Moira::getCCR() const
{
    auto result =
    reg.sr.c << 0 |
    reg.sr.v << 1 |
    reg.sr.z << 2 |
    reg.sr.n << 3 |
    reg.sr.x << 4 ;

    return u8(result);
}

void
Moira::setCCR(u8 val)
{
    reg.sr.c = (val >> 0) & 1;
    reg.sr.v = (val >> 1) & 1;
    reg.sr.z = (val >> 2) & 1;
    reg.sr.n = (val >> 3) & 1;
    reg.sr.x = (val >> 4) & 1;
}

u16
Moira::getSR() const
{
    auto flags =
    reg.sr.t1  << 15 |
    reg.sr.t0  << 14 |
    reg.sr.s   << 13 |
    reg.sr.m   << 12 |
    reg.sr.ipl <<  8 ;

    return u16(flags | getCCR());
}

void
Moira::setSR(u16 val)
{
    bool t1  = (val >> 15) & 1;
    bool s   = (val >> 13) & 1;
    u8   ipl = (val >>  8) & 7;

    reg.sr.ipl = ipl;
    flags |= State::CHECK_IRQ;
    t1 ? setTraceFlag() : clearTraceFlag();

    setCCR(u8(val));
    setSupervisorMode(s);

    if (cpuModel > CPUModel::M68010) {

        bool t0 = (val >> 14) & 1;
        bool m = (val >> 12) & 1;

        t0 ? setTrace0Flag() : clearTrace0Flag();
        setMasterMode(m);
    }
}

void
Moira::setCACR(u32 val)
{
    // Setting the clear bits invalidates (parts of) the caches
    iCache.cacrDidChange(val);
    dCache.cacrDidChange(val);

    reg.cacr = val & getCacrMask();
    iCache.refresh();
    dCache.refresh();
    didChangeCACR(val);
}

void
Moira::setCAAR(u32 val)
{
    reg.caar = val;
    iCache.caarDidChange(val);
    dCache.caarDidChange(val);
    didChangeCAAR(val);
}

void
Moira::setSupervisorMode(bool s)
{
    if (s != reg.sr.s) setSupervisorFlags(s, reg.sr.m);
}

void
Moira::setMasterMode(bool m)
{
    if (m != reg.sr.m) setSupervisorFlags(reg.sr.s, m);
}

void
Moira::setSupervisorFlags(bool s, bool m)
{
    bool uspWasVisible = !reg.sr.s;
    bool ispWasVisible =  reg.sr.s && !reg.sr.m;
    bool mspWasVisible =  reg.sr.s &&  reg.sr.m;

    if (uspWasVisible) reg.usp = reg.sp;
    if (ispWasVisible) reg.isp = reg.sp;
    if (mspWasVisible) reg.msp = reg.sp;

    reg.sr.s = s;
    reg.sr.m = m;

    bool uspIsVisible  = !reg.sr.s;
    bool ispIsVisible  =  reg.sr.s && !reg.sr.m;
    bool mspIsVisible  =  reg.sr.s &&  reg.sr.m;

    if (uspIsVisible)  reg.sp = reg.usp;
    if (ispIsVisible)  reg.sp = reg.isp;
    if (mspIsVisible)  reg.sp = reg.msp;
}
