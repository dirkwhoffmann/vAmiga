# Overview {#api_overview}

The whole public API hangs off a single object of class [vamiga::VAmiga](@ref vamiga::VAmiga). It has two kinds of members:

- **Functions** that act on the emulator as a whole: querying the version, launching the emulator thread, controlling the execution state, resetting, configuring, and synchronizing with the emulator thread.
- **Sub-APIs**, one per hardware component or subsystem. They are public data members, so a call reads like `amiga.cpu.getInfo()` or `amiga.df0.insertBlankDisk(...)`. Sub-APIs may contain further sub-APIs, for example `amiga.cpu.breakpoints` or `amiga.controlPort1.joystick`.

All API classes are derived from [vamiga::API](@ref vamiga::API), which provides [suspend()](@ref vamiga::API::suspend) and [resume()](@ref vamiga::API::resume).

## Sub-APIs

| Member | Class | Purpose |
|:-------|:------|:--------|
| `amiga` | [vamiga::AmigaAPI](@ref vamiga::AmigaAPI) | The machine as a whole: snapshots, ROMs, media files, auto-inspection. |
| `agnus` | [vamiga::AgnusAPI](@ref vamiga::AgnusAPI) | The Agnus chip. Its members `copper` ([vamiga::CopperAPI](@ref vamiga::CopperAPI)), `blitter` ([vamiga::BlitterAPI](@ref vamiga::BlitterAPI)), `xray` ([vamiga::XRayAPI](@ref vamiga::XRayAPI)), `logicAnalyzer` ([vamiga::LogicAnalyzerAPI](@ref vamiga::LogicAnalyzerAPI)) and `beamtraps` ([vamiga::GuardsAPI](@ref vamiga::GuardsAPI)) give access to the subcomponents. |
| `ciaA`, `ciaB` | [vamiga::CIAAPI](@ref vamiga::CIAAPI) | The two CIA chips. |
| `cpu` | [vamiga::CPUAPI](@ref vamiga::CPUAPI) | CPU state. Its members `debugger` ([vamiga::CPUDebuggerAPI](@ref vamiga::CPUDebuggerAPI)) disassembles code, `breakpoints`, `watchpoints` and `catchpoints` ([vamiga::GuardsAPI](@ref vamiga::GuardsAPI)) manage guards. |
| `denise` | [vamiga::DeniseAPI](@ref vamiga::DeniseAPI) | The Denise chip. |
| `mem` | [vamiga::MemoryAPI](@ref vamiga::MemoryAPI) | Memory configuration and state. Its member `debugger` ([vamiga::MemoryDebuggerAPI](@ref vamiga::MemoryDebuggerAPI)) dumps memory. |
| `paula` | [vamiga::PaulaAPI](@ref vamiga::PaulaAPI) | The Paula chip with its `diskController` ([vamiga::DiskControllerAPI](@ref vamiga::DiskControllerAPI)) and `uart` ([vamiga::UARTAPI](@ref vamiga::UARTAPI)). |
| `rtc` | [vamiga::RTCAPI](@ref vamiga::RTCAPI) | The real-time clock. |
| `audioPort`, `videoPort` | [vamiga::AudioPortAPI](@ref vamiga::AudioPortAPI), [vamiga::VideoPortAPI](@ref vamiga::VideoPortAPI) | Audio and video output. |
| `controlPort1`, `controlPort2` | [vamiga::ControlPortAPI](@ref vamiga::ControlPortAPI) | The control ports. Their members `joystick` ([vamiga::JoystickAPI](@ref vamiga::JoystickAPI)) and `mouse` ([vamiga::MouseAPI](@ref vamiga::MouseAPI)) control the connected device. |
| `serialPort` | [vamiga::SerialPortAPI](@ref vamiga::SerialPortAPI) | The serial port. |
| `df0` ... `df3` | [vamiga::FloppyDriveAPI](@ref vamiga::FloppyDriveAPI) | The floppy drives and their disks. |
| `hd0` ... `hd3` | [vamiga::HardDriveAPI](@ref vamiga::HardDriveAPI) | The hard drives. Their member `controller` ([vamiga::HdControllerAPI](@ref vamiga::HdControllerAPI)) is the drive controller. |
| `keyboard` | [vamiga::KeyboardAPI](@ref vamiga::KeyboardAPI) | Key presses. |
| `msgQueue` | [vamiga::MsgQueueAPI](@ref vamiga::MsgQueueAPI) | The message queue. |
| `remoteManager` | [vamiga::RemoteManagerAPI](@ref vamiga::RemoteManagerAPI) | Remote servers (RetroShell, RPC, DAP, ...). |
| `retroShell` | [vamiga::RetroShellAPI](@ref vamiga::RetroShellAPI) | The command console. |
| `defaults` (static) | [vamiga::DefaultsAPI](@ref vamiga::DefaultsAPI) | Persistent user defaults. Available without an emulator instance. |

## Conventions

| Pattern | Meaning |
|:--------|:--------|
| `getConfig()`, `get(Opt)` / `set(Opt, ...)` | Read and change the configuration. Options are identified by the enumeration [vamiga::Opt](@ref vamiga::Opt). |
| `getInfo()`, `getCachedInfo()`, `getMetrics()`, ... | Read the state of a component. |
| `foo()` / `fooAsync()` | Blocking and asynchronous flavor of a state-changing function. |

To be notified about events, read the messages from the message queue ([vamiga::MsgQueueAPI](@ref vamiga::MsgQueueAPI)). The messages are listed in the enumeration [vamiga::Msg](@ref vamiga::Msg).

## Errors

API functions report failures by throwing exceptions derived from [utl::Error](@ref utl::Error). Each function documents what it can throw with `@throw`.

| Exception | Meaning |
|:----------|:--------|
| [vamiga::CoreError](@ref vamiga::CoreError) | The request does not match the state of the emulator, e.g., an invalid option value or a missing ROM. |
| [utl::IOError](@ref utl::IOError) | A file or folder could not be accessed. |
| [vamiga::MediaError](@ref vamiga::MediaError) | A ROM, snapshot, or disk image is invalid or not supported. |
