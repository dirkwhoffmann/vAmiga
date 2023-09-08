// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraTypes.h"
#include "FpuFormats.h"

namespace vamiga::moira {

class FPUReg {

    // Reference to the FPU
    class FPU &fpu;

public:

    // Register value
    FpuExtended val;


    //
    // Constructing
    //

    FPUReg(FPU& fpu) : fpu(fpu) { }
    void reset() { val = FpuExtended::nan; }


    //
    // Getting and setting
    //

    FpuExtended asExtended();
    FpuExtended get() const { return val; }
    void set(const FpuExtended other);


    //
    // Classifying
    //


    int fpclassify() const { return val.fpclassify(); }
    bool isfinite() const { return val.isfinite(); }
    bool isinf() const { return val.isinf(); }
    bool isnan() const { return val.isnan(); }
    bool isnormal() const { return val.isnormal(); }
    bool issubnormal() const { return val.issubnormal(); }
    bool signbit() const { return val.signbit(); }

    bool isNegative() const { return val.isnegative(); }
    bool isZero() const  { return val.iszero(); }
    bool isSignalingNaN() const { return val.isSignalingNaN(); }
    bool isNonsignalingNaN() const { return val.isNonsignalingNaN(); }

};

class FPU {
    
    // Reference to the CPU
    class Moira &moira;
    
    // Emulated FPU model
    FPUModel model = FPU_NONE;
    
public:
    
    // Registers
    FPUReg fpr[8] = {
        FPUReg(*this), FPUReg(*this), FPUReg(*this), FPUReg(*this),
        FPUReg(*this), FPUReg(*this), FPUReg(*this), FPUReg(*this)
    };
    
    u32 fpiar;
    u32 fpsr;
    u32 fpcr;
    
    // Experimental
    int oldRoundingMode;
    
    
    //
    // Constructing
    //
    
public:
    
    FPU(Moira& ref);
    void reset();
    
    
    //
    // Configuring
    //
    
public:
    
    // Selects the emulated CPU model
    void setModel(FPUModel model);
    FPUModel getModel() const { return model; }
    
    // Returns the precision and rounding mode, as specified in the FPCR
    FpuPrecision getPrecision() const;
    FpuRoundingMode getRoundingMode() const;
    
    // Configures the rounding mode of the host FPU
    static FpuRoundingMode fesetround(FpuRoundingMode mode);
    
    
    //
    // Accessing registers
    //
    
public:
    
    // Accesses the control register
    u32 getFPCR() const { return fpcr & 0x0000FFF0; }
    void setFPCR(u32 value);
    
    // Accesses the status register
    void clearFPSR() { fpsr &= 0xFFFF00F8; }
    u32 getFPSR() const { return fpsr & 0x0FFFFFF8; }
    void setFPSR(u32 value);
    
    void setExcStatusBit(u32 mask);
    void clearExcStatusBit(u32 mask);
    
    void setConditionCodes(int reg);
    void setConditionCodes(const FpuExtended &value);
    
    // Accesses the instruction address register
    u32 getFPIAR() const { return fpiar; }
    void setFPIAR(u32 value);
    
    
    //
    // Accessing the constant Rom
    //
    
    // Reads a value from the constant Rom
    FpuExtended readCR(unsigned nr);
    
    
    //
    // Analyzing instructions
    //
    
public:
    
    // Returns true iff instruction I is supported by a certain FPU model
    template <Instr I> static bool isSupported(FPUModel model)
    {
        switch (I) {
                
            case FACOS:     case FASIN:     case FATANH:    case FCOS:
            case FCOSH:     case FETOX:     case FETOXM1:   case FGETEXP:
            case FGETMAN:   case FINTRZ:    case FLOG10:    case FLOG2:
            case FLOGN:     case FLOGNP1:   case FMOD:      case FREM:
            case FSCAL:     case FSIN:      case FSINCOS:   case FSINH:
            case FTAN:      case FTANH:     case FTENTOX:   case FTWOTOX:
                
                return model != FPU_68040;
                
            default:
                
                return true;
        }
    }
    template <Instr I> bool isSupported() { return isSupported<I>(model); }
    
    // Returns true iff instruction I is a monadic arithmetic instruction
    template <Instr I> static bool isMonadic()
    {
        switch (I) {
                
            case FABS:      case FACOS:     case FASIN:     case FATAN:
            case FATANH:    case FCOS:      case FCOSH:     case FETOX:
            case FETOXM1:   case FGETEXP:   case FGETMAN:   case FINT:
            case FINTRZ:    case FLOG10:    case FLOG2:     case FLOGN:
            case FLOGNP1:   case FNEG:      case FSIN:      case FSINCOS:
            case FSINH:     case FSQRT:     case FTAN:      case FTANH:
            case FTENTOX:   case FTST:      case FTWOTOX:
                
                return true;
                
            default:
                
                return false;
        }
    }
    
    // Returns true iff instruction I is a dyadic arithmetic instruction
    template <Instr I> static bool isDyadic()
    {
        switch (I) {
                
            case FADD:      case FCMP:      case FDIV:      case FMOD:
            case FMUL:      case FREM:      case FSCAL:     case FSGLDIV:
            case FSGLMUL:   case FSUB:
                
                return true;
                
            default:
                
                return false;
        }
    }
    
    // Checks the validity of the extension words
    static bool isValidExt(Instr I, Mode M, u16 op, u32 ext);
    
    
    //
    // Managing the host FPU
    //
    
    // Clears the exception flags of the host FPU
    void clearHostFpuFlags();
    
    // Copies the exception flags of the host FPU into FPSR
    void copyHostFpuFlags();
    
    
    //
    // Handling special values
    //
    
    static FpuExtended makeNonsignalingNan(const FpuExtended &value); // DEPRECATED
    
    // Checks the arguments for NaNs and computes the result NaN if applicable
    std::optional<FpuExtended> resolveNan(const FpuExtended &op1, const FpuExtended &op2);
    std::optional<FpuExtended> resolveNan(const FpuExtended &op);
    
    
    //
    // Executing instructions
    //
    
    std::function<void(int)> exceptionHandler = [this](int flags) { setExcStatusBit(flags); };
    
    FpuExtended monadic(const FpuExtended &value, std::function<long double(long double)> func);
    
    // Monadic operations
    FpuExtended fabs(const FpuExtended &value);
    FpuExtended facos(const FpuExtended &value);
    FpuExtended fasin(const FpuExtended &value);
    FpuExtended fatan(const FpuExtended &value);
    FpuExtended fatanh(const FpuExtended &value);
    FpuExtended fcos(const FpuExtended &value);
    FpuExtended fcosh(const FpuExtended &value);
    FpuExtended fetox(const FpuExtended &value);
    FpuExtended fetoxm1(const FpuExtended &value);
    FpuExtended fgetexp(const FpuExtended &value);
    FpuExtended fgetman(const FpuExtended &value);
    FpuExtended fint(const FpuExtended &value);
    FpuExtended fintrz(const FpuExtended &value);
    FpuExtended flog10(const FpuExtended &value);
    FpuExtended flog2(const FpuExtended &value);
    FpuExtended flogn(const FpuExtended &value);
    FpuExtended flognp1(const FpuExtended &value);
    FpuExtended fneg(const FpuExtended &value);
    FpuExtended fsin(const FpuExtended &value);
    FpuExtended fsinh(const FpuExtended &value);
    FpuExtended fsqrt(const FpuExtended &value);
    FpuExtended ftan(const FpuExtended &value);
    FpuExtended ftanh(const FpuExtended &value);
    FpuExtended ftentox(const FpuExtended &value);
    FpuExtended ftwotox(const FpuExtended &value);

    // Dyadic operations
    FpuExtended fadd(const FpuExtended &op1, const FpuExtended &op2);
    FpuExtended fcmp(const FpuExtended &op1, const FpuExtended &op2);
    FpuExtended fdiv(const FpuExtended &op1, const FpuExtended &op2);
    FpuExtended fmod(const FpuExtended &op1, const FpuExtended &op2);
    FpuExtended fmul(const FpuExtended &op1, const FpuExtended &op2);
    FpuExtended frem(const FpuExtended &op1, const FpuExtended &op2);
    FpuExtended fscal(const FpuExtended &op1, const FpuExtended &op2);
    FpuExtended fsgldiv(const FpuExtended &op1, const FpuExtended &op2);
    FpuExtended fsglmul(const FpuExtended &op1, const FpuExtended &op2);
    FpuExtended fsub(const FpuExtended &op1, const FpuExtended &op2);
};

}
