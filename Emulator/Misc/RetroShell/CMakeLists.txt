target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

Command.cpp
Console.cpp
ShellCmds.cpp
DebugCmds.cpp
RetroShell.cpp
TextStorage.cpp

)
