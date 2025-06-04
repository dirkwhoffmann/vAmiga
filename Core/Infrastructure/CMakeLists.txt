target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Emulator.cpp
Error.cpp
CmdQueue.cpp
Configurable.cpp
CoreObject.cpp
CoreComponent.cpp
Defaults.cpp
Dumpable.cpp
GuardList.cpp
Host.cpp
MsgQueue.cpp
Option.cpp
Serializable.cpp
SubComponent.cpp
Thread.cpp
Wakeable.cpp

)
