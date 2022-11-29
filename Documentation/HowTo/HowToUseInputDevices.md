# How to use the delegation system

The Moira class declares various delegation functions that are called on certain events. By default, all delegation functions are declared as virtual and provided with an empty implementation. A subclassed CPU can react on the supported events by overrding the corresponding function.

## State delegates

- `void didReset()`

    Invoked at the end of the `reset()` routine. 

- `void didHalt()`

    Invoked when the CPU is halted. This state is entered, e.g., when a double-fault occurs. 

## Instruction delegates

- `void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode)`

    Invoked at the beginning of the instruction handlers of certain instructions. Due to speed reasons, the delegation function is only called for selected instructions. The instructions are selected by macro `WILL_EXECUTE` defined in `MoiraConfig.h`. The default implementation looks like this:
    ````c++
    #define WILL_EXECUTE    I == STOP || I == TAS || I == BKPT
    `````

- `void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode)`

    Invoked at the end of the instruction handlers of certain instructions. Due to speed reasons, the delegation function is only called for selected instructions. The instructions are selected by macro `DID_EXECUTE` defined in `MoiraConfig.h`. The default implementation looks like this:
    ````c++
    #define DID_EXECUTE     I == RESET
    `````

## Exception delegates

- `void willExecute(ExceptionType exc, u16 vector)`

    Invoked at the beginning of the exception processing code. 

- `void willInterrupt(u8 level)`

    Invoked at the beginning of the interrupt processing code. 

- `void didJumpToVector(int nr, u32 addr)`

    Invoked after the program counter has been redirected to the exception handler. 

- `void didExecute(ExceptionType exc, u16 vector)`

    Invoked at the end of the exception processing code. 

## Cache register delegated

- `void didChangeCACR(u32 value)`

    Invoked when the CACR register is modified (68020 only). 

- `void didChangeCAAR(u32 value)`

    Invoked when the CAAR register is modified (68020 only). 

## Debugger delegates

The following delegation methods are invoked by the debugger to signal a debug event.
- `void softstopReached(u32 addr)`
- `void breakpointReached(u32 addr)`
- `void watchpointReached(u32 addr)`
- `void catchpointReached(u8 vector)`
- `void softwareTrapReached(u32 addr)`