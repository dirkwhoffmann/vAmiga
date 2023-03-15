target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

Error.cpp
CoreObject.cpp
CoreComponent.cpp
SubComponent.cpp
Thread.cpp
MsgQueue.cpp
Defaults.cpp
Host.cpp
)
