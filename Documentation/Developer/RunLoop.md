# The Run Loop

In this document we will examine how vAmiga calculates a single frame. If you've studied the document about the `Thread` class, you already know that the calculation of a single frame is triggered within the threads main execution function. E.g., when the thread is running in *pulsed* mode, the following function is executed:

```c++
template <> void
Thread::execute<THREAD_PULSED>()
{
    loadClock.go();
    execute();
    loadClock.stop();
}
```

The `loadClock` object is used to measure the CPU load. All the action takes place in function `execute`, which is declared as a pure virtual function inside the `Thread` class:

```c++
virtual void execute() = 0;
```

The `Amiga` class implements this function as follows: 

```c++
void
Amiga::execute()
{    
    while (1) {
        
        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (flags) {
                        
            ...
            
            // Are we requested to synchronize the thread?
            if (flags & RL::SYNC_THREAD) {
                clearFlag(RL::SYNC_THREAD);
                break;
            }
        }
    }
```

The function enters an infinite loop, which we refer to as the `run loop`. The inner working is quite simple. First the CPU is asked to execute a single instruction. After that, the variable `flags` is checked. This variable is a bit field storing several *action flags* whose purpose is to trigger specific actions within the run loop. The following list gives you an idea about what kind of actions we are talking about:

```c++
namespace RL
{
constexpr u32 STOP               = (1 << 0);
constexpr u32 SOFTSTOP_REACHED   = (1 << 1);
constexpr u32 BREAKPOINT_REACHED = (1 << 2);
constexpr u32 WATCHPOINT_REACHED = (1 << 3);
constexpr u32 CATCHPOINT_REACHED = (1 << 4);
constexpr u32 SWTRAP_REACHED     = (1 << 5);
constexpr u32 COPPERBP_REACHED   = (1 << 6);
constexpr u32 COPPERWP_REACHED   = (1 << 7);
constexpr u32 AUTO_SNAPSHOT      = (1 << 8);
constexpr u32 USER_SNAPSHOT      = (1 << 9);
constexpr u32 SYNC_THREAD        = (1 << 10);
};
```

In most cases the variable `flags` equals zero. In this case, the loop simply executes one CPU instruction after another.

But wait, didn't we say the function only calculates a single frame? The answer is yes, and this is where the `SYNC_THREAD` flag comes into play. The flag is set by `Agnus` in the following function:

```c++ 
void
Agnus::eofHandler()
{
    ...

    // Let the thread synchronize
    amiga.setFlag(RL::SYNC_THREAD);
}
```

The acronym `eof` refers to *end of frame*. As you may have guessed already, the function is executed at the end of each frame. Once the variable `SYNC_THREAD` is set, the run loop will be terminated with the next check of the variable `flags`.

Overall, it's pretty simple, isn't it? Well, the answer is yes and no. For instance, we just learned that the `SYNC_THREAD` flag is set by Agnus in its end-of-frame handler, but where is this function called? The run loop does nothing of that sort, it just calls the CPU and checks some flags. 

The sky clears up by taking a closer look at what's happening inside `CPU::execute()`. Let's assume that the next instruction to be executed is a `BRA` instruction. Inside `CPU::execute()` the CPU starts executing some internal actions. After that it calls the execution handler of the `BRA` instruction, which begins as follows:

```c++
template <Core C, Instr I, Mode M, Size S> void
Moira::execBra(u16 opcode)
{
    AVAILABILITY(S == Long ? C68020 : C68000)

    u32 oldpc = reg.pc;
    u32 disp = S == Byte ? (u8)opcode : queue.irc;

    SYNC(2);

    ...
}
```

The `SYNC` statement is the one we need to examine. It is a macro that expands to a call to the `sync` function, at least when emulating a M68000 or M68010. The CPU calls this function to signal to the surrounding logic that the CPU clock has advanced a certain number of cycles. In this particular example, it signals that 2 CPU cycles have elapsed.

Let's see how the `sync` function is implemented. An uncluttered version of this function looks like this: 

```c++ 
void
Moira::sync(int cycles)
{
    // Advance the CPU clock
    clock += cycles;

    // Emulate Agnus up to the same cycle
    agnus.execute(CPU_AS_DMA_CYCLES(cycles));
}
```

After advancing the internal clock, the function asks `Agnus` to emulate the surrounding logic up to the point in time where the CPU currently is. After that, control is returned to the CPU. Now, when the CPU performs a memory access, it is assured that the surrounding logic is up to date. For write accesses, this means that the operation is carried out at the proper point in time, and for read accesses, it means that correct values will be read.

Let's head over to `Agnus` and have a closer look at what the execute function does. The function that was actually called is a convenience wrapper around the actual execution function:

```c++ 
void
Agnus::execute(DMACycle cycles)
{
    for (DMACycle i = 0; i < cycles; i++) execute();
}
```

So let's dig a little deeper: 

```c++
void
Agnus::execute()
{
    // Advance the internal clock and the horizontal counter
    clock += DMA_CYCLES(1);
    pos.h += 1;

    // Process pending events
    if (nextTrigger <= clock) executeUntil(clock);
}
```

This function emulates `Agnus` for one bus cycle. First, it increments the color clock and the horizontal counter by one. After that, if a pending event is present, it calls the event scheduler.

The Event Scheduler can undoubtedly be considered the central component of vAmiga, as everything is built around it. All actions that have to be executed in a certain bus cycle are triggered by this component. Please always remember that the event scheduler, despite its central role, is never called directly inside the run loop of vAmiga. It is only called indirectly, before each memory access of the CPU.

Since the event scheduler is of such great importance, we'll examine it in more detail in a separate document. 
