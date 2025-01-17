target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

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
